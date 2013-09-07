/*
 * ZSUMMER License
 * -----------
 * 
 * ZSUMMER is licensed under the terms of the MIT license reproduced below.
 * This means that ZSUMMER is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2010-2013 YaweiZhang <yawei_zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */

#include "iocp.h"
#include "tcpaccept.h"
#include "tcpsocket.h"
using namespace zsummer;




zsummer::network::IIOServer * zsummer::network::CreateIOServer()
{
	return new CIOServer();
}
void zsummer::network::DestroyIOServer(zsummer::network::IIOServer * ios)
{
	delete ios;
}

CIOServer::CIOServer()
{
	m_io = NULL;
	m_cb = NULL;
	m_queSeq = 0;
}
CIOServer::~CIOServer()
{

}


bool CIOServer::Initialize(IIOServerCallback * cb)
{
	if (g_coreID <0)
	{
		g_coreID = zsummer::log4z::ILog4zManager::GetInstance()->FindLogger("NetWork");
	}
	//assert(g_coreID != -1);

	if (m_io != NULL)
	{
		LCF("iocp is craeted !");
		return false;
	}
	m_cb = cb;

	m_io = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
	if (!m_io)
	{
		LCF("CreateIoCompletionPort False!");
		return false;
	}
	m_nextExpire = (unsigned long long)-1;
	return true;
}


void CIOServer::Post(void *pUser)
{
	PostMsg(PCK_USER_DATA, (ULONG_PTR)pUser);
}

unsigned long long CIOServer::CreateTimer(unsigned int delayms, ITimerCallback * cb)
{
	unsigned long long now = zsummer::utility::GetTimeMillisecond();
	unsigned long long expire = now+delayms;
	assert(!(expire&0xfffff00000000000));
	expire <<= 20;
	{
		CAutoLock l(m_lockTimer);
		m_queSeq++;
		expire |= (m_queSeq&0xfffff);
		m_queTimer.insert(std::make_pair(expire, cb));
		if (m_nextExpire > now+delayms || m_nextExpire < now)
		{
			m_nextExpire = now+delayms;
		}
	}
	//LCI("create timerID=" << expire << " expire time= " << delayms );
	return expire;
}

bool CIOServer::CancelTimer(unsigned long long timerID)
{
	bool bRet = false;
	{
		CAutoLock l(m_lockTimer);
		std::map<unsigned long long, ITimerCallback*>::iterator iter = m_queTimer.find(timerID);
		if (iter != m_queTimer.end())
		{
			m_queTimer.erase(iter);
			bRet = true;
		}
	}
	return bRet;
}

void CIOServer::PostMsg(POST_COM_KEY pck, ULONG_PTR ptr)
{
	PostQueuedCompletionStatus(m_io, 0, pck,(LPOVERLAPPED)(ptr));
}


void CIOServer::RunOnce()
{
	if (m_io == NULL || m_cb == NULL)
	{
		LCF("Can't Run Once. server not initialize or initialize false.");
		return;
	}

	DWORD dwTranceCount = 0;
	ULONG_PTR uComKey = NULL;
	LPOVERLAPPED pOverlapped = NULL;
	unsigned long long nowMs = zsummer::utility::GetTimeMillisecond();
	DWORD dwDelayMs = 0;
	dwDelayMs = (DWORD) (m_nextExpire -nowMs);
	if (dwDelayMs > 100)
	{
		dwDelayMs = 100;
	}
	else if (dwDelayMs < 10)
	{
		dwDelayMs = 10;
	}
	BOOL bRet = GetQueuedCompletionStatus(m_io, &dwTranceCount, &uComKey, &pOverlapped, dwDelayMs/*INFINITE*/);

	//检查定时器超时状态
	{
		if (!m_queTimer.empty())
		{
			nowMs = zsummer::utility::GetTimeMillisecond();
			unsigned long long expire = m_nextExpire;
			if (expire <= nowMs)
			{
				std::vector<std::pair<unsigned long long, ITimerCallback*> > allexpire;
				{
					CAutoLock l(m_lockTimer);
					while(1)
					{
						if (m_queTimer.empty())
						{
							m_nextExpire = (unsigned long long)-1;
							//LCI("m_queTimer.empty()");
							break;
						}
						std::map<unsigned long long, ITimerCallback*>::iterator iter = m_queTimer.begin();
						unsigned long long nextexpire = (iter->first)>>20;
						if (nowMs < nextexpire)
						{
							m_nextExpire = nextexpire;
							//LCI("next timerID=" << iter->first << ", next expire=" << nextexpire-cur);
							break;
						}
						//LCI("timer expire, timeID=" << iter->first);
						allexpire.push_back(*iter);
						m_queTimer.erase(iter);
					}
				}
				//LCI("allexpire size=" << allexpire.size());
				for (std::vector<std::pair<unsigned long long, ITimerCallback*> >::iterator iter = allexpire.begin(); iter != allexpire.end(); ++iter)
				{
					iter->second->OnTimer(iter->first);
				}
			}

		}
		
		if (!bRet && !pOverlapped)
		{
			//TIMEOUT
			return;
		}
	}

	//! 检查自定义通知
	if (uComKey == PCK_ACCEPT_CLOSE)
	{
		CTcpAccept *pp = (CTcpAccept *) (ULONG_PTR)pOverlapped;
		pp->OnClear();
		return;
	}
	else if (uComKey == PCK_SOCKET_CLOSE)
	{
		CTcpSocket *pp = (CTcpSocket*) (ULONG_PTR)pOverlapped;
		pp->OnClear();
		return;
	}
	else if (uComKey == PCK_USER_DATA)
	{
		m_cb->OnPost((void *)pOverlapped);
		return;
	}
	
	//! 处理来自网络的通知
	unsigned char type = ((tagReqHandle*)pOverlapped)->_type;
	switch (type)
	{
	case tagReqHandle::HANDLE_ACCEPT:
		{
			CTcpAccept *pKey = (CTcpAccept *) uComKey;
			pKey->OnIOCPMessage(bRet);
		}
		break;
	case tagReqHandle::HANDLE_RECV:
	case tagReqHandle::HANDLE_SEND:
	case tagReqHandle::HANDLE_CONNECT:
		{
			CTcpSocket *pKey = (CTcpSocket *) uComKey;
			pKey->OnIOCPMessage(bRet, dwTranceCount, type);
		}
		break;
	default:
		LCE("GetQueuedCompletionStatus undefined type=" << type);
	}

}


