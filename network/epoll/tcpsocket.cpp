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

#include "tcpsocket.h"
#include "epoll.h"
using namespace zsummer;

zsummer::network::ITcpSocket * zsummer::network::CreateTcpSocket()
{
	return new CTcpSocket;
}


void zsummer::network::DestroyTcpSocket(zsummer::network::ITcpSocket * s)
{
	delete s;
}


CTcpSocket::CTcpSocket()
{
	m_cb = NULL;
	m_ios = NULL;
	memset(&m_handle, 0, sizeof(m_handle));

	m_bNeedDestroy = false;

	m_iRecvLen  = 0;
	m_pRecvBuf = NULL;
	m_iSendLen = 0;
	m_pSendBuf = NULL;

}


CTcpSocket::~CTcpSocket()
{

}

 bool CTcpSocket::Initialize(IIOServer * ios, ITcpSocketCallback * cb)
{
	m_ios = ios;
	m_cb = cb;
	if (m_handle._fd != 0)
	{
		m_handle._event.events = 0;
		if (epoll_ctl(((CIOServer *)m_ios)->m_epoll, EPOLL_CTL_ADD, m_handle._fd, &m_handle._event) != 0)
		{
			LCE("CTcpSocket::Initialize()" << this << " EPOLL_CTL_ADD error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
			return false;
		}
	}
	return true;
}




bool CTcpSocket::DoConnect(const char *ip, unsigned short port)
{
	if (m_cb == NULL)
	{
		LCE("CTcpSocket::DoConnect()" << this << " callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCE("CTcpSocket::DoConnect()" << this << " IIOServer not bind!");
		return false;
	}
	if (m_handle._fd != 0)
	{
		LCE("CTcpSocket::DoConnect()" << this << " DoConnect ERR:  fd aready used!");
		return false;
	}
	
	m_handle._fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_handle._fd == -1)
	{
		LCE("CTcpSocket::DoConnect()" << this << " socket create err errno =" << strerror(errno));
		return false;
	}

	SetNonBlock(m_handle._fd);
	SetNoDelay(m_handle._fd);
	m_handle._event.events = EPOLLOUT;
	m_handle._event.data.ptr = &m_handle;
	m_handle._ptr = this;
	m_handle._type = tagRegister::REG_CONNECT;

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(ip);
	m_addr.sin_port = htons(port);
	
	int ret = connect(m_handle._fd, (sockaddr *) &m_addr, sizeof(m_addr));
	if (ret!=0 && errno != EINPROGRESS)
	{
		LCE("CTcpSocket::DoConnect()" << this << " ::connect error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
		return false;
	}
	
	if (epoll_ctl(((CIOServer *)m_ios)->m_epoll, EPOLL_CTL_ADD, m_handle._fd, &m_handle._event) != 0)
	{
		LCE("CTcpSocket::DoConnect()" << this << " EPOLL_CTL_ADD error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
		return false;
	}

	return true;
}


bool CTcpSocket::GetPeerInfo(unsigned int * addr, unsigned short *port)
{
	*addr = (unsigned int)m_addr.sin_addr.s_addr;
	*port = ntohs(m_addr.sin_port);
	return true;
}


bool CTcpSocket::DoSend(char * buf, unsigned int len)
{
	if (m_cb == NULL)
	{
		LCE("CTcpSocket::DoSend()" << this << " callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCE("CTcpSocket::DoSend()" << this << " IIOServer not bind!");
		return false;
	}
	if (len == 0)
	{
		LCE("CTcpSocket::DoSend()" << this << " argument err! len ==0");
		return false;
	}
	if (m_pSendBuf != NULL || m_iSendLen != 0)
	{
		LCE("CTcpSocket::DoSend()" << this << " (m_pSendBuf != NULL || m_iSendLen != 0) == TRUE");
		return false;
	}
	if (m_handle._event.events & EPOLLOUT)
	{
		LCE("CTcpSocket::DoSend()" << this << " (m_handle._event.events & EPOLLOUT) == TRUE");
		return false;
	}
	
	
	m_pSendBuf = buf;
	m_iSendLen = len;

	m_handle._event.events = m_handle._event.events|EPOLLOUT;
	if (!EPOLLMod(((CIOServer *)m_ios)->m_epoll, m_handle._fd, &m_handle._event) != 0)
	{
		LCE("CTcpSocket::DoSend()" << this << " EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
		Close();
		return false;
	}
	//LOGI("do send");
	return true;
}


bool CTcpSocket::DoRecv(char * buf, unsigned int len)
{
	if (m_cb == NULL)
	{
		LCE("CTcpSocket::DoRecv()" << this << "  callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCE("CTcpSocket::DoRecv()" << this << "  IIOServer not bind!");
		return false;
	}
	if (len == 0 )
	{
		LCE("CTcpSocket::DoRecv()" << this << "  argument err !!!  len==0");
		return false;
	}
	if (m_pRecvBuf != NULL || m_iRecvLen != 0)
	{
		LCE("CTcpSocket::DoRecv()" << this << "    (m_pRecvBuf != NULL || m_iRecvLen != 0) == TRUE");
		return false;
	}
	if (m_handle._event.events & EPOLLIN)
	{
		LCE("CTcpSocket::DoRecv()" << this << "  (m_handle._event.events & EPOLLIN) == TRUE");
		return false;
	}
	
	m_pRecvBuf = buf;
	m_iRecvLen = len;

	m_handle._event.events = m_handle._event.events|EPOLLIN;
	if (!EPOLLMod(((CIOServer *)m_ios)->m_epoll, m_handle._fd, &m_handle._event) != 0)
	{
		LCE("CTcpSocket::DoRecv()" << this << " EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
		Close();
		return false;
	}
	return true;
}


bool CTcpSocket::OnEPOLLMessage(int type, int flag)
{
	if (m_handle._type == tagRegister::REG_CONNECT)
	{
		if (flag & EPOLLOUT)
		{
			m_handle._event.events = /*EPOLLONESHOT*/ 0;
			if (!EPOLLMod(((CIOServer *)m_ios)->m_epoll, m_handle._fd, &m_handle._event))
			{
				LCE("CTcpSocket::OnEPOLLMessage()" << this << " connect true & EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
				m_cb->OnConnect(false);
				return false;
			}
			m_handle._type = tagRegister::REG_ESTABLISHED;
			m_cb->OnConnect(true);
		}
		else 
		{
			if (epoll_ctl(((CIOServer *)m_ios)->m_epoll, EPOLL_CTL_DEL, m_handle._fd, &m_handle._event) != 0)
			{
				LCE("CTcpSocket::OnEPOLLMessage()" << this << " connect false & EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
			}
			close(m_handle._fd);
			m_handle._fd = 0;
			m_cb->OnConnect(false);
		}
		return true;
	}


	if (flag & EPOLLHUP)
	{
		//LCI("CTcpSocket::OnEPOLLMessage()" << this << " EPOLLHUP  error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", events=" << flag);
		Close();
		return false;
	}

	if (flag & EPOLLERR)
	{
		//LCI("CTcpSocket::OnEPOLLMessage()" << this << "  EPOLLERR error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", events=" << flag);
		Close();
		return false;
	}
	if (flag & EPOLLIN)
	{
		if (m_pRecvBuf == NULL || m_iRecvLen == 0)
		{
			LCE("CTcpSocket::OnEPOLLMessage()" << this << " recv error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", m_pRecvBuf=" <<(void*)m_pRecvBuf << ", m_iRecvLen=" << m_iRecvLen);
		}
		else
		{
			int ret = recv(m_handle._fd, m_pRecvBuf, m_iRecvLen, 0);
			if (ret == 0 || (ret ==-1 && (errno !=EAGAIN && errno != EWOULDBLOCK)) )
			{
				LCE("CTcpSocket::OnEPOLLMessage()" << this << " recv error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", ret=" << ret << ", errno=" << strerror(errno));
				Close();
				return false;
			}

			if (ret != -1)
			{
				m_pRecvBuf = NULL;
				m_iRecvLen = 0;
				m_handle._event.events = m_handle._event.events& ~EPOLLIN;
				if (!EPOLLMod(((CIOServer *)m_ios)->m_epoll, m_handle._fd, &m_handle._event))
				{
					LCE("CTcpSocket::OnEPOLLMessage()" << this << "recv EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
					Close();
					return false;
				}
				m_cb->OnRecv(ret);
			}
		}
	}

	if (flag & EPOLLOUT)
	{
		if (m_pSendBuf == NULL || m_iSendLen == 0)
		{
			LCE("CTcpSocket::OnEPOLLMessage()" << this << " send error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", m_pSendBuf=" <<(void*)m_pSendBuf << ", m_iSendLen=" << m_iSendLen);
		}
		else
		{
			int ret = send(m_handle._fd, m_pSendBuf, m_iSendLen, 0);
			if (ret == -1 && (errno != EAGAIN && errno != EWOULDBLOCK))
			{
				LCE("CTcpSocket::OnEPOLLMessage()" << this << " send error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", ret=" << ret << ", errno=" << strerror(errno));
				Close();
				return false;
			}

			if (ret != -1)
			{
				m_pSendBuf = NULL;
				m_iSendLen = 0;
				m_handle._event.events = m_handle._event.events& ~EPOLLOUT;
				if (!EPOLLMod(((CIOServer *)m_ios)->m_epoll, m_handle._fd, &m_handle._event))
				{
					LCE("CTcpSocket::OnEPOLLMessage()" << this << "send EPOLLMod error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
					Close();
					return false;
				}
				m_cb->OnSend(ret);
			}
		}
	}

	return true;
}


bool CTcpSocket::Close()
{
	if (m_handle._type == tagRegister::REG_ESTABLISHED)
	{
		m_handle._type = tagRegister::REG_INVALIDE;
		if (epoll_ctl(((CIOServer *)m_ios)->m_epoll, EPOLL_CTL_DEL, m_handle._fd, &m_handle._event) != 0)
		{
			LCE("CTcpSocket::Close()" << this << " EPOLL_CTL_DEL error. epfd="<<((CIOServer *)m_ios)->m_epoll << ", handle fd=" << m_handle._fd << ", errno=" << strerror(errno));
		}
		shutdown(m_handle._fd, SHUT_RDWR);
		close(m_handle._fd);
		((CIOServer*)m_ios)->PostMsg(PCK_SOCKET_CLOSE, this);
	}
	return true;
}

void CTcpSocket::OnPostClose()
{
	m_cb->OnClose();
	if (m_bNeedDestroy)
	{
		DestroyTcpSocket(this);
	}
}
