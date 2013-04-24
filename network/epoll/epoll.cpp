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

#include "epoll.h"
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
	m_cb = NULL;
	m_epoll = 0;
	m_sockpair[0] = 0;
	m_sockpair[1] = 0;
}

CIOServer::~CIOServer()
{

}



bool CIOServer::Start(IIOServerCallback *cb)
{
	if (g_coreID <0)
	{
		g_coreID = zsummer::log4z::ILog4zManager::GetInstance()->FindLogger("NetWork");
	}
	//assert(g_coreID != -1);
	
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
		m_recv._ptr = this;
		m_recv._type = tagRegister::REG_THREAD;
		m_recv._socket = m_sockpair[1];
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

bool CIOServer::Stop()
{
	PostMsg(PCK_EPOLL_EXIT, NULL);
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

void CIOServer::Run()
{
	epoll_event events[10000];
	int retCount = 0;
	bool bRuning = true;
	unsigned int timerTime = GetTimeMillisecond();
	while (true)
	{
		retCount = epoll_wait(m_epoll, events, 10000,  1000);
		if (retCount == -1)
		{
			if (errno != EINTR)
			{
				LCD("ERR: epoll_wait err!  errno=" <<strerror(errno));
				break;
			}
			continue;
		}
		//check timer
		{
			unsigned int curTime = GetTimeMillisecond();
			if (curTime - timerTime > 1000)
			{
				timerTime = curTime;
				m_cb->OnTimer();
			}
			if (retCount == 0) continue;//timeout
		}

		
		for (int i=0; i<retCount; i++)
		{
			int eventflag = events[i].events;
			tagRegister * pReg = (tagRegister *)events[i].data.ptr;
			//tagHandle  type
			if (pReg->_type == tagRegister::REG_THREAD)
			{
				char buf[1000];
				while (recv(pReg->_socket, buf, 1000, 0) > 0){}

				MsgVct msgs;
				m_msglock.Lock();
				msgs = m_msgs;
				m_msgs.clear();
				m_msglock.UnLock();

				for (MsgVct::iterator iter = msgs.begin(); iter != msgs.end(); ++iter)
				{
					if (iter->first == PCK_EPOLL_EXIT)
					{
						LCD("INFO: epoll recv exit event");
						bRuning = false;
						continue;
					}
					else if (iter->first == PCK_ACCEPT_CLOSE)
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
						m_cb->OnMsg(iter->second);
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
			else
			{
				if    (pReg->_type != tagRegister::REG_RECV 
					&& pReg->_type != tagRegister::REG_SEND
					&& pReg->_type != tagRegister::REG_CONNECT)
				{
					LCE("check register event type failed !!  type=" << pReg->_type);
					continue;
				}

				CTcpSocket *pKey = (CTcpSocket *) pReg->_ptr;
				pKey->OnEPOLLMessage(pReg->_type, eventflag);
			}
		}

		if (!bRuning)
		{
			break;
		}
	}

	LCD("INFO: epoll loop exit");
}


