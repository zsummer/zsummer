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
#include "tcpaccept.h"
#include "tcpsocket.h"
#include "epoll.h"

using namespace zsummer;


zsummer::network::ITcpAccept * zsummer::network::CreateTcpAccept()
{
	return new CTcpAccept;
}
void zsummer::network::DestroyTcpAccept(zsummer::network::ITcpAccept *s)
{
	delete s;
}


CTcpAccept::CTcpAccept()
{
	m_port = 0;
	m_cb = NULL;
	memset(&m_handle, 0, sizeof(m_handle));
}


CTcpAccept::~CTcpAccept()
{

}

bool CTcpAccept::Initialize(IIOServer * ios, ITcpAcceptCallback * cb)
{
	m_ios =ios;
	m_cb = cb;
	return true;
}


bool CTcpAccept::OpenAccept(const char * ip, unsigned short port)
{
	if (m_cb == NULL)
	{
		LCF("ERR: callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCF("ERR: IIOServer not bind!");
		return false;
	}

	if (m_handle._fd != 0)
	{
		LCF("ERR: accept fd is aready used!");
		return false;
	}

	m_handle._fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_handle._fd == -1)
	{
		LCF("ERR: socket create err errno =" << strerror(errno));
		return false;
	}

	SetNonBlock(m_handle._fd);

	int bReuse = 1;
	if (setsockopt(m_handle._fd, SOL_SOCKET, SO_REUSEADDR,  &bReuse, sizeof(bReuse)) != 0)
	{
		LCW("WAR: socket set reuse fail! errno=" << strerror(errno));
	}


	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(ip);
	m_addr.sin_port = htons(port);
	if (bind(m_handle._fd, (sockaddr *) &m_addr, sizeof(m_addr)) != 0)
	{
		LCF("ERR: socket bind err, errno=" << strerror(errno));
		close(m_handle._fd);
		m_handle._fd = 0;
		return false;
	}

	if (listen(m_handle._fd, 200) != 0)
	{
		LCF("ERR: socket listen err, errno=" << strerror(errno));
		close(m_handle._fd);
		m_handle._fd = 0;
		return false;
	}

	m_handle._event.events = EPOLLIN;
	m_handle._event.data.ptr = &m_handle;
	m_handle._ptr = this;
	m_handle._type = tagRegister::REG_ACCEPT;
	if (epoll_ctl(((CIOServer *)m_ios)->m_epoll, EPOLL_CTL_ADD, m_handle._fd, &m_handle._event) != 0)
	{
		LCF("ERR: epoll ctl  accept err!  errno=" << strerror(errno));
		return false;
	}
	return true;
}

bool CTcpAccept::OnEPOLLMessage(bool bSuccess)
{
	if (!bSuccess)
	{
		LCF("ERR: accept EPOLLERR, errno=" << strerror(errno));
		Close();
		return false;
	}
	else
	{
		LCD("INFO: onaccept on link");
		sockaddr_in cltaddr;
		memset(&cltaddr, 0, sizeof(cltaddr));
		socklen_t len = sizeof(cltaddr);
		int s = accept(m_handle._fd, (sockaddr *)&cltaddr, &len);
		if (s == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				LCE("ERR: accept -1, errno=" << strerror(errno));
				Close();
				return false;
			}
			return true;
		}

		SetNonBlock(s);
		SetNoDelay(s);
	
		CTcpSocket * ps = (CTcpSocket *)CreateTcpSocket();
		ps->m_handle._fd = s;
		ps->m_handle._event.data.ptr = & ps->m_handle;
		ps->m_handle._ptr = ps;
		ps->m_handle._type = tagRegister::REG_ESTABLISHED;
		ps->m_bNeedDestroy = true;
		memcpy(&ps->m_addr, &cltaddr, sizeof(ps->m_addr));
		m_cb->OnAccept(ps);
	}
	return true;
}

bool CTcpAccept::Close()
{
	if (epoll_ctl(((CIOServer *)m_ios)->m_epoll, EPOLL_CTL_DEL, m_handle._fd, &m_handle._event) != 0)
	{
		LCE("ERR: epoll ctl  DEL err!  errno=" << strerror(errno));
	}
	shutdown(m_handle._fd, SHUT_RDWR);
	close(m_handle._fd);
	((CIOServer*)m_ios)->PostMsg(PCK_ACCEPT_CLOSE, this);
	return true;
}


void CTcpAccept::OnPostClose()
{
	m_cb->OnClose();
}
