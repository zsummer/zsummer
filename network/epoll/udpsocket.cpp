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

#include "udpsocket.h"
#include "epoll.h"
using namespace zsummer;

zsummer::network::IUdpSocket * zsummer::network::CreateUdpSocket()
{
	return new CUdpSocket;
}


void zsummer::network::DestroyUdpSocket(zsummer::network::IUdpSocket * s)
{
	delete s;
}


CUdpSocket::CUdpSocket()
{
	m_cb = NULL;
	m_ios = NULL;
	memset(&m_handle, 0, sizeof(m_handle));
	m_iRecvLen  = 0;
	m_pRecvBuf = NULL;
}


CUdpSocket::~CUdpSocket()
{
	if (m_handle._fd != 0)
	{
		close(m_handle._fd);
		m_handle._fd = 0;
	}
}

 bool CUdpSocket::Initialize(IIOServer * ios, IUdpSocketCallback * cb, const char *ip, unsigned short port)
{
	if (m_cb != NULL)
	{
		LCE("CUdpSocket socket is aready used. m_cb not is NULL. this=" << this);
		return false;
	}
	if (m_ios != NULL)
	{
		LCE("CUdpSocket socket is aready used, m_ios not is NULL. this=" << this);
		return false;
	}
	if (m_handle._fd != 0)
	{
		LCE("CUdpSocket socket is aready used, _fd not is 0. this=" << this << ", fd=" << m_handle._fd);
		return false;
	}
	m_ios = ios;
	m_cb = cb;
	m_handle._fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_handle._fd == -1)
	{
		LCE("CUdpSocket create socket fail. this=" << this << ", errno=" << strerror(errno));
		return false;
	}

	SetNonBlock(m_handle._fd);
	m_handle._event.events = 0;
	m_handle._event.data.ptr = &m_handle;
	m_handle._ptr = this;
	m_handle._type = tagRegister::REG_RECVFROM;
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(ip);
	m_addr.sin_port = htons(port);
	if (bind(m_handle._fd, (sockaddr *) &m_addr, sizeof(m_addr)) != 0)
	{
		LCE("CUdpSocket: socket bind err, errno=" << strerror(errno));
		close(m_handle._fd);
		m_handle._fd = 0;
		return false;
	}
	if (epoll_ctl(((CIOServer *)m_ios)->m_epoll, EPOLL_CTL_ADD, m_handle._fd, &m_handle._event) != 0)
	{
		LCE("CUdpSocket::Initialize()" << this << " EPOLL_CTL_ADD error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
		return false;
	}
	return true;
}

bool CUdpSocket::DoSend(char * buf, unsigned int len, const char *dstip, unsigned short dstport)
{
	if (m_ios == NULL)
	{
		LCE("CUdpSocket::DoSend()" << this << " IIOServer not bind!");
		return false;
	}
	if (len == 0 || len >1500)
	{
		LCE("CUdpSocket::DoSend()" << this << " argument err! len=" << len);
		return false;
	}
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(dstip);
	addr.sin_port = htons(dstport);
	int sl = sendto(m_handle._fd, buf, len, 0, (sockaddr*)&addr, sizeof(addr));
	if (sl<= 0)
	{
		LCE("CUdpSocket sendto error, sentlen=" << sl);
		return false;
  }
  return true;
}


bool CUdpSocket::DoRecv(char * buf, unsigned int len)
{
	if (m_cb == NULL)
	{
		LCE("CUdpSocket::DoRecv()" << this << "  callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCE("CUdpSocket::DoRecv()" << this << "  IIOServer not bind!");
		return false;
	}
	if (len == 0 )
	{
		LCE("CUdpSocket::DoRecv()" << this << "  argument err !!!  len==0");
		return false;
	}
	if (m_pRecvBuf != NULL || m_iRecvLen != 0)
	{
		LCE("CUdpSocket::DoRecv()" << this << "    (m_pRecvBuf != NULL || m_iRecvLen != 0) == TRUE");
		return false;
	}
	if (m_handle._event.events & EPOLLIN)
	{
		LCE("CUdpSocket::DoRecv()" << this << "  (m_handle._event.events & EPOLLIN) == TRUE");
		return false;
	}
	
	m_pRecvBuf = buf;
	m_iRecvLen = len;

	m_handle._event.events = m_handle._event.events|EPOLLIN;
	if (!EPOLLMod(((CIOServer *)m_ios)->m_epoll, m_handle._fd, &m_handle._event) != 0)
	{
		LCE("CUdpSocket::DoRecv()" << this << " EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
		return false;
	}
	return true;
}


bool CUdpSocket::OnEPOLLMessage(int type, int flag)
{
	if (flag & EPOLLHUP)
	{
		LCE("CUdpSocket::OnEPOLLMessage()" << this << " EPOLLHUP  error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", events=" << flag);
		return false;
	}
	if (flag & EPOLLERR)
	{
		LCE("CUdpSocket::OnEPOLLMessage()" << this << "  EPOLLERR error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", events=" << flag);
		return false;
	}
	if (flag & EPOLLIN)
	{
		if (m_pRecvBuf == NULL || m_iRecvLen == 0)
		{
			LCE("CUdpSocket::OnEPOLLMessage()" << this << " recv error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", m_pRecvBuf=" <<(void*)m_pRecvBuf << ", m_iRecvLen=" << m_iRecvLen);
		}
		else
		{
			sockaddr_in raddr;
			memset(&raddr, 0, sizeof(raddr));
			socklen_t len = sizeof(raddr);
			int ret = recvfrom(m_handle._fd, m_pRecvBuf, m_iRecvLen, 0, (sockaddr*)&raddr, &len);
			if (ret == 0 || (ret ==-1 && (errno !=EAGAIN && errno != EWOULDBLOCK)) )
			{
				LCE("CUdpSocket::OnEPOLLMessage()" << this << " recv error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", ret=" << ret << ", errno=" << strerror(errno));
				return false;
			}

			if (ret != -1)
			{
				m_pRecvBuf = NULL;
				m_iRecvLen = 0;
				m_handle._event.events = m_handle._event.events& ~EPOLLIN;
				if (!EPOLLMod(((CIOServer *)m_ios)->m_epoll, m_handle._fd, &m_handle._event))
				{
					LCE("CUdpSocket::OnEPOLLMessage()" << this << "recv EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
					return false;
				}
				m_cb->OnRecvFrom(ret, inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
			}
		}
	}

	

	return true;
}

