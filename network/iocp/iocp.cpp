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
 * Copyright (C) 2012 YaweiZhang <yawei_zhang@foxmail.com>.
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
}
CIOServer::~CIOServer()
{

}


bool CIOServer::Start(IIOServerCallback * cb)
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

	return true;
}

bool CIOServer::Stop()
{
	PostMsg(PCK_IOCP_EXIT, NULL);
	return true;
}
void CIOServer::Post(void *pUser)
{
	PostMsg(PCK_USER_DATA, (ULONG_PTR)pUser);
}

void CIOServer::PostMsg(POST_COM_KEY pck, ULONG_PTR ptr)
{
	PostQueuedCompletionStatus(m_io, 0, pck,(LPOVERLAPPED)(ptr));
}


void CIOServer::Run()
{
	DWORD dwTranceCount = 0;
	ULONG_PTR uComKey = NULL;
	LPOVERLAPPED pOverlapped = NULL;
	unsigned int timerTime = GetTimeMillisecond();
	while (1)
	{
		dwTranceCount = 0;
		uComKey = 0;
		pOverlapped = NULL;

		BOOL bRet = GetQueuedCompletionStatus(m_io, &dwTranceCount, &uComKey, &pOverlapped, 1000/*INFINITE*/);

		//check timer
		{
			unsigned int curTime = GetTimeMillisecond();
			if (curTime - timerTime > 1000)
			{
				timerTime = curTime;
				m_cb->OnTimer();
			}
			if (!bRet && !pOverlapped)
			{
				//TIMEOUT
				continue;
			}
		}

		if (uComKey == PCK_IOCP_EXIT && pOverlapped == NULL)
		{
			LCI("PCK_IOCP_EXIT ...");
			break;
		}
		else if (uComKey == PCK_ACCEPT_CLOSE)
		{
			CTcpAccept *pp = (CTcpAccept *) (ULONG_PTR)pOverlapped;
			pp->OnClear();
			continue;
		}
		else if (uComKey == PCK_SOCKET_CLOSE)
		{
			CTcpSocket *pp = (CTcpSocket*) (ULONG_PTR)pOverlapped;
			pp->OnClear();
			continue;
		}
		else if (uComKey == PCK_USER_DATA)
		{
			m_cb->OnMsg((void *)pOverlapped);
			continue;
		}
		
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

	
}


