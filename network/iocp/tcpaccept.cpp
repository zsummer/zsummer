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
#include "public.h"
#include "tcpaccept.h"
#include "tcpsocket.h"
#include "iocp.h"
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
	//config
	m_ios = NULL;
	m_cb = NULL;

	//listen
	m_server = INVALID_SOCKET;
	memset(&m_addr, 0, sizeof(m_addr));
	memset(&m_handle, 0, sizeof(m_handle));
	m_handle._type = tagReqHandle::HANDLE_ACCEPT;

	//client
	m_socket = INVALID_SOCKET;
	memset(m_recvBuf, 0, sizeof(m_recvBuf));
	
	m_recvLen = 0;
	memset(&m_handle, 0, sizeof(m_handle));

	//status
	m_nAcceptCount = 0;
	m_nLinkStatus = LS_UNINITIALIZE;
}
CTcpAccept::~CTcpAccept()
{
	if (m_server != INVALID_SOCKET)
	{
		closesocket(m_server);
		m_server = INVALID_SOCKET;
	}
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

bool CTcpAccept::Initialize(IIOServer * ios, ITcpAcceptCallback * cb)
{
	m_ios =ios;
	m_cb = cb;
	return true;
}

bool CTcpAccept::OpenAccept(const char * ip, unsigned short port)
{
	if (m_ios == NULL)
	{
		LCF("CTcpAccept IIOServer not bind!  ip=" << ip << ", port=" << port);
		assert(0);
		return false;
	}
	if (m_cb == NULL)
	{
		LCF("CTcpAccept callback not set!  ip=" << ip << ", port=" << port);
		assert(0);
		return false;
	}

	if (m_server != INVALID_SOCKET)
	{
		LCF("CTcpAccept socket is arealy used!  ip=" << ip << ", port=" << port);
		assert(0);
		return false;
	}

	m_server = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_server == INVALID_SOCKET)
	{
		LCF("CTcpAccept server can't create socket!   ip=" << ip << ", port=" << port);
		assert(0);
		return false;
	}

	BOOL bReUseAddr = TRUE;
	if (setsockopt(m_server, SOL_SOCKET,SO_REUSEADDR, (char*)&bReUseAddr, sizeof(BOOL)) != 0)
	{
		LCW("setsockopt  SO_REUSEADDR ERROR! ERRCODE=" << WSAGetLastError() << "    ip=" << ip << ", port=" << port);
	}

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(ip);
	m_addr.sin_port = htons(port);
	if (bind(m_server, (sockaddr *) &m_addr, sizeof(m_addr)) != 0)
	{
		LCF("server bind err, ERRCODE=" << WSAGetLastError() << "   ip=" << ip << ", port=" << port);
		closesocket(m_server);
		m_server = INVALID_SOCKET;
		return false;
	}

	if (listen(m_server, SOMAXCONN) != 0)
	{
		LCF("server listen err, ERRCODE=" << WSAGetLastError() << "   ip=" << ip << ", port=" << port);
		closesocket(m_server);
		m_server = INVALID_SOCKET;
		return false;
	}

	if (CreateIoCompletionPort((HANDLE)m_server, ((CIOServer *)m_ios)->m_io, (ULONG_PTR)this, 1) == NULL)
	{
		LCF("server bind iocp err, ERRCODE=" << WSAGetLastError() << "   ip=" << ip << ", port=" << port);
		closesocket(m_server);
		m_server = INVALID_SOCKET;
		return false;
	}
	m_nLinkStatus = LS_ESTABLISHED;



	memset(&m_handle, 0, sizeof(m_handle));
	m_handle._type = tagReqHandle::HANDLE_ACCEPT;
	m_socket = INVALID_SOCKET;
	memset(m_recvBuf, 0, sizeof(m_recvBuf));
	m_recvLen = 0;

	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,  NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_socket == INVALID_SOCKET)
	{
		LCF("create client socket err! ERRCODE=" << WSAGetLastError() << " ip=" << m_ip << ", port=" << m_port);
		return false;
	}

	if (!AcceptEx(m_server, m_socket, m_recvBuf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &m_recvLen, &m_handle._overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			LCE("do AcceptEx err, ERRCODE=" << WSAGetLastError() << " ip=" << m_ip << ", port=" << m_port);
			return false;
		}
	}
	m_nAcceptCount++;
	return true;
}


bool CTcpAccept::OnIOCPMessage(BOOL bSuccess)
{
	m_nAcceptCount--;
	if (m_nAcceptCount < 0)
	{
		LCF("m_nAcceptCount=" <<m_nAcceptCount  << " ip=" << m_ip << ", port=" << m_port);
		return false;
	}
	if (m_nLinkStatus == LS_WAITCLOSE)
	{
		LCF("safe close server ip=" << m_ip << ", port=" << m_port);
		m_nLinkStatus = LS_WAITCLEAR;
		((CIOServer*)m_ios)->PostMsg(PCK_ACCEPT_CLOSE, NULL);
		return true;
	}

	if (bSuccess)
	{

		{
			if (setsockopt(m_socket,SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_server, sizeof(m_server)) != 0)
			{
				LCW("SO_UPDATE_ACCEPT_CONTEXT fail!  last err=" << WSAGetLastError()  << " ip=" << m_ip << ", port=" << m_port);
			}
			BOOL bTrue = TRUE;
			if (setsockopt(m_socket,IPPROTO_TCP, TCP_NODELAY, (char*)&bTrue, sizeof(bTrue)) != 0)
			{
				LCW("setsockopt TCP_NODELAY fail!  last err=" << WSAGetLastError()  << " ip=" << m_ip << ", port=" << m_port);
			}
		}

		sockaddr * paddr1 = NULL;
		sockaddr * paddr2 = NULL;
		int tmp1 = 0;
		int tmp2 = 0;
		GetAcceptExSockaddrs(m_recvBuf, m_recvLen, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &paddr1, &tmp1, &paddr2, &tmp2);
		
		CTcpSocket * ps = (CTcpSocket*)CreateTcpSocket();
		ps->m_nNeedDelete = 1;
		ps->m_socket = m_socket;
		ps->m_nLinkStatus = LS_ESTABLISHED;
		memcpy(&ps->m_addr, paddr2, sizeof(ps->m_addr));
		m_socket = NULL;
		m_cb->OnAccept(ps);

		memset(&m_handle, 0, sizeof(m_handle));
		m_handle._type = tagReqHandle::HANDLE_ACCEPT;
		m_socket = INVALID_SOCKET;
		memset(m_recvBuf, 0, sizeof(m_recvBuf));
		m_recvLen = 0;

		m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,  NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_socket == INVALID_SOCKET)
		{
			LCF("create client socket err! ERRCODE=" << WSAGetLastError() << " ip=" << m_ip << ", port=" << m_port);
			return false;
		}

		if (!AcceptEx(m_server, m_socket, m_recvBuf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &m_recvLen, &m_handle._overlapped))
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				LCE("do AcceptEx err, ERRCODE=" << WSAGetLastError() << " ip=" << m_ip << ", port=" << m_port);
				return false;
			}
		}
		m_nAcceptCount++;
	}
	else
	{
		LCW("Accept Fail,  retry doAccept ... ip=" << m_ip << ", port=" << m_port);
	}
	return true;
}


bool CTcpAccept::Close()
{
	if (m_nLinkStatus == LS_ESTABLISHED)
	{
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
		if (m_nAcceptCount > 0)
		{
			m_nLinkStatus = LS_WAITCLOSE;
		}
		else
		{
			m_nLinkStatus = LS_WAITCLEAR;
			((CIOServer*)m_ios)->PostMsg(PCK_ACCEPT_CLOSE, (ULONG_PTR)this);
		}
		return true;
	}
	return false;
}

void CTcpAccept::OnClear()
{
	m_cb->OnClose();
}
