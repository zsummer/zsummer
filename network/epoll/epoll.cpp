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

#include "epoll.h"
#include "tcpaccept.h"
#include "tcpsocket.h"
#include "udpsocket.h"
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
	m_cb = NULL;
	m_epoll = 0;
	m_nextExpire = (unsigned long long)-1;
	m_sockpair[0] = 0;
	m_sockpair[1] = 0;
}

CIOServer::~CIOServer()
{

}



bool CIOServer::Initialize(IIOServerCallback *cb)
{
	if (g_coreID <0)
	{
		g_coreID = zsummer::log4z::ILog4zManager::GetInstance()->FindLogger("NetWork");
	}
	//assert(g_coreID != -1);
	m_nextExpire = (unsigned long long)-1;
	m_cb = cb;
	if (m_epoll != 0)
	{
		LCF("ERR: epoll is create !");
		return false;
	}
	m_epoll = epoll_create(200);
	if (m_epoll == -1)
	{
		LCF("ERR: create epoll err errno=" << strerror(errno));
		return false;
	}
	{
		if (socketpair(AF_LOCAL, SOCK_STREAM, 0, m_sockpair) != 0)
		{
			LCF("ERR: create socketpair.  errno=" << strerror(errno));
			return false;
		}
		SetNonBlock(m_sockpair[0]);
		SetNonBlock(m_sockpair[1]);
		SetNoDelay(m_sockpair[0]);
		SetNoDelay(m_sockpair[1]);
		m_recv._ptr = this;
		m_recv._type = tagRegister::REG_THREAD;
		m_recv._fd = m_sockpair[1];
		m_recv._event.data.ptr = &m_recv;
		m_recv._event.events = EPOLLIN;
		if (epoll_ctl(m_epoll, EPOLL_CTL_ADD, m_sockpair[1], &m_recv._event) != 0)
		{
			LCF("epoll_ctl socketpair.  errno=" << errno);
			return false;
		}
	}

	return true;
}




void CIOServer::PostMsg(POST_COM_KEY pck, void * ptr)
{
	m_msglock.Lock();
	m_msgs.push_back(std::make_pair(pck, ptr));
	m_msglock.UnLock();
	char c='0';
	send(m_sockpair[0], &c, 1, 0);
}

void CIOServer::Post(void *pUser)
{
	PostMsg(PCK_USER_DATA, pUser);
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
void CIOServer::CheckTimer()
{
	if (!m_queTimer.empty())
	{
		unsigned long long nowMs = zsummer::utility::GetTimeMillisecond();
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
}
void CIOServer::RunOnce()
{
	int dwDelayMs =0;
	unsigned long long nowMs = zsummer::utility::GetTimeMillisecond();
	dwDelayMs = (int) (m_nextExpire -nowMs);
	if (dwDelayMs > 100)
	{
		dwDelayMs = 100;
	}
	else if (dwDelayMs < 10)
	{
		dwDelayMs = 10;
	}

	int retCount = epoll_wait(m_epoll, m_events, 1000,  dwDelayMs);
	if (retCount == -1)
	{
		if (errno != EINTR)
		{
			LCD("ERR: epoll_wait err!  errno=" <<strerror(errno));
			return; //! error
		}
		return;
	}
	//check timer
	//检查定时器超时状态
	{
		CheckTimer();
		if (retCount == 0) return;//timeout
	}


		
	for (int i=0; i<retCount; i++)
	{
		int eventflag = m_events[i].events;
		tagRegister * pReg = (tagRegister *)m_events[i].data.ptr;
		//tagHandle  type
		if (pReg->_type == tagRegister::REG_THREAD)
		{
			char buf[1000];
			while (recv(pReg->_fd, buf, 1000, 0) > 0);

			MsgVct msgs;
			m_msglock.Lock();
			msgs.swap(m_msgs);
			m_msglock.UnLock();

			for (MsgVct::iterator iter = msgs.begin(); iter != msgs.end(); ++iter)
			{
				if (iter->first == PCK_ACCEPT_CLOSE)
				{
					CTcpAccept * p = (CTcpAccept *) iter->second;
					p->OnPostClose();
				}
				else if (iter->first == PCK_SOCKET_CLOSE)
				{
					CTcpSocket *p = (CTcpSocket*)iter->second;
					p->OnPostClose();
				}
				else if (iter->first == PCK_USER_DATA)
				{
					m_cb->OnPost(iter->second);
				}
			}
		}
		else if (pReg->_type == tagRegister::REG_ACCEPT)
		{
			CTcpAccept *pKey = (CTcpAccept *) pReg->_ptr;
			if (eventflag & EPOLLIN)
			{
				pKey->OnEPOLLMessage(true);
			}
			if (eventflag & EPOLLERR || eventflag & EPOLLHUP)
			{
				pKey->OnEPOLLMessage(false);
			}
		}
		else if (pReg->_type == tagRegister::REG_ESTABLISHED || pReg->_type == tagRegister::REG_CONNECT)
		{
			CTcpSocket *pKey = (CTcpSocket *) pReg->_ptr;
			pKey->OnEPOLLMessage(pReg->_type, eventflag);
		}
		else if (pReg->_type == tagRegister::REG_RECVFROM)
		{
			CUdpSocket *pKey = (CUdpSocket *) pReg->_ptr;
			pKey->OnEPOLLMessage(pReg->_type, eventflag);
		}
		else
		{
			LCE("check register event type failed !!  type=" << pReg->_type);
		}
			
	}
}


