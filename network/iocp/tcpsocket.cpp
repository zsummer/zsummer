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

#include "public.h"
#include "tcpsocket.h"
#include "iocp.h"
using namespace zsummer;

ITcpSocket * zsummer::network::CreateTcpSocket()
{
	return new CTcpSocket;
}
void zsummer::network::DestroyTcpSocket(ITcpSocket * s)
{
	delete s;
}


CTcpSocket::CTcpSocket()
{
	m_ios=NULL;
	m_cb=NULL;
	m_socket=INVALID_SOCKET;
	memset(&m_addr, 0, sizeof(m_addr));

	//recv
	memset(&m_recvHandle, 0, sizeof(m_recvHandle));
	m_recvHandle._type = tagReqHandle::HANDLE_RECV;
	m_recvWSABuf.buf = NULL;
	m_recvWSABuf.len = 0;
	m_recvUserBuf = NULL;
	m_recvOffset = 0;
	m_recvTotal = 0;

	//send
	memset(&m_sendHandle, 0, sizeof(m_sendHandle));
	m_sendHandle._type = tagReqHandle::HANDLE_SEND;
	m_sendWsaBuf.buf = NULL;
	m_sendWsaBuf.len = 0;
	m_sendUserBuf = NULL;
	m_sendOffset = 0;
	m_sendTotal = 0;

	//connect

	memset(&m_connectHandle, 0, sizeof(m_connectHandle));
	m_connectHandle._type = tagReqHandle::HANDLE_CONNECT;

	//status
	m_nReqCount = 0;
	m_nLinkStatus = LS_NORMAL;
	m_nNeedDelete = 0;
}


CTcpSocket::~CTcpSocket()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

bool CTcpSocket::BindIOServer(IIOServer * ios)
{
	m_ios = ios;
	if (m_socket == INVALID_SOCKET)
	{
		m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_socket == INVALID_SOCKET)
		{
			LCE("WSASocket create ERROR! ERRCODE=" << WSAGetLastError());
			return false;
		}
		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(SOCKADDR_IN));
		addr.sin_family = AF_INET;
		if (bind(m_socket, (sockaddr *) &addr, sizeof(SOCKADDR_IN)) != 0)
		{
			LCE("bind any addr for connect err!  last err=" << WSAGetLastError());
			return false;
		}
	}

	if (CreateIoCompletionPort((HANDLE)m_socket, ((CIOServer *)m_ios)->m_io, (ULONG_PTR)this, 1) == NULL)
	{
		LCE("bind to IOCP err!");
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	return true;
}


bool CTcpSocket::SetCallback(ITcpSocketCallback * cb)
{
	m_cb = cb;
	return true;
}


typedef  BOOL (PASCAL  *ConnectEx)(  SOCKET s,  const struct sockaddr* name,  int namelen,
			 PVOID lpSendBuffer,  DWORD dwSendDataLength,  LPDWORD lpdwBytesSent,
			 LPOVERLAPPED lpOverlapped);

bool CTcpSocket::DoConnect(const char *ip, unsigned short port)
{
	if (m_cb == NULL)
	{
		LCF("callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCF("IIOServer not bind!");
		return false;
	}

	GUID gid = WSAID_CONNECTEX;
	ConnectEx lpConnectEx = NULL;
	DWORD dwSize = 0;
	if (WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &gid, sizeof(gid), &lpConnectEx, sizeof(lpConnectEx), &dwSize, NULL, NULL) != 0)
	{
		LCE("Get ConnectEx pointer err!  last err= "<< WSAGetLastError());
		return false;
	}

	m_addr.sin_addr.s_addr = inet_addr(ip);
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);

	char buf[1];
	DWORD dwLenth = 0;
	if (!lpConnectEx(m_socket, (sockaddr *)&m_addr, sizeof(m_addr), buf, 0, &dwLenth, &m_connectHandle._overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			LCE(" do ConnectEx err! ERRCODE=" << WSAGetLastError());
			return false;
		}

	}
	m_nReqCount ++;
	return true;
}


bool CTcpSocket::GetPeerInfo(unsigned int * addr, unsigned short *port)
{
	*addr = (int)m_addr.sin_addr.S_un.S_addr;
	*port = ntohs(m_addr.sin_port);
	return true;
}


bool CTcpSocket::DoSend(char * buf, unsigned int len)
{
	if (m_cb == NULL)
	{
		LCE("callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCE("IIOServer not bind!");
		return false;
	}

	if (len == 0)
	{
		LCE("argument err! len ==0");
		return false;
	}
	m_sendUserBuf = buf;
	m_sendTotal = len;
	m_sendOffset = 0;
	m_sendWsaBuf.buf = m_sendUserBuf;
	m_sendWsaBuf.len = m_sendTotal;

	DWORD dwTemp1=0;
	if (WSASend(m_socket, &m_sendWsaBuf, 1, &dwTemp1, 0, &m_sendHandle._overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LCD("ERR: WSASend err!");
			return false;
		}
	}
	m_nReqCount++;
	return true;
}


bool CTcpSocket::DoRecv(char * buf, unsigned int len)
{
	if (m_cb == NULL)
	{
		LCF("callback not set!");
		return false;
	}
	if (m_ios == NULL)
	{
		LCF("IIOServer not bind!");
		return false;
	}

	m_recvUserBuf = buf;
	m_recvTotal = len;
	m_recvOffset = 0;
	m_recvWSABuf.buf = m_recvUserBuf;
	m_recvWSABuf.len = m_recvTotal;

	DWORD dwRecv = 0;
	DWORD dwFlag = 0;
	if (WSARecv(m_socket, &m_recvWSABuf, 1, &dwRecv, &dwFlag, &m_recvHandle._overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LCD("ERR: WSARecv err!  last err=" << WSAGetLastError());
			return false;
		}
	}
	m_nReqCount++;
	return true;
}

bool CTcpSocket::OnIOCPMessage(BOOL bSuccess, DWORD dwTranceCount, unsigned char cType)
{
	m_nReqCount--;
	if (cType == tagReqHandle::HANDLE_CONNECT)
	{
		if (bSuccess)
		{
			m_nLinkStatus = LS_ESTABLISHED;
		}
		else
		{
			m_nLinkStatus = LS_NORMAL;
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		m_cb->OnConnect(!!bSuccess);
		return true;
	}

	if (m_nLinkStatus == LS_WAITCLOSE && m_nReqCount == 0)
	{
		m_nLinkStatus = LS_WAITCLEAR;
		((CIOServer *)m_ios)->PostMsg(PCK_SOCKET_CLOSE, NULL);
		return true;
	}

	if (cType == tagReqHandle::HANDLE_RECV)
	{
		if (dwTranceCount == m_recvTotal - m_recvOffset)
		{
			m_cb->OnRecv();
		}
		else
		{
			m_recvOffset += dwTranceCount;
			m_recvWSABuf.buf = m_recvUserBuf+m_recvOffset;
			m_recvWSABuf.len = m_recvTotal - m_recvOffset;

			DWORD dwRecv = 0;
			DWORD dwFlag = 0;
			if (WSARecv(m_socket, &m_recvWSABuf, 1, &dwRecv, &dwFlag, &m_recvHandle._overlapped, NULL) != 0)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					LCD("WSARecv err!  last err=" << WSAGetLastError());
					goto doclose;
				}
			}
			m_nReqCount++;
		}
	}
	else if (cType == tagReqHandle::HANDLE_SEND)
	{
		if (dwTranceCount == m_sendTotal - m_sendOffset)
		{
			m_cb->OnSend();
		}
		else
		{
			m_sendOffset += dwTranceCount;
			m_sendWsaBuf.buf = m_sendUserBuf + m_sendOffset;
			m_sendWsaBuf.len = m_sendTotal - m_sendOffset;

			DWORD dwTemp1=0;
			if (WSASend(m_socket, &m_sendWsaBuf, 1, &dwTemp1, 0, &m_sendHandle._overlapped, NULL) != 0)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					LCD("WSASend err!");
					goto doclose;
				}
			}
			m_nReqCount++;
		}
	}
	return true;
doclose:
	Close();
	return true;
}


bool CTcpSocket::Close()
{
	if (m_nLinkStatus == LS_NORMAL || m_nLinkStatus == LS_ESTABLISHED)
	{
		m_nLinkStatus = LS_WAITCLOSE;
		closesocket(m_socket);
		if (m_nReqCount == 0)
		{
			m_nLinkStatus = LS_WAITCLEAR;
			((CIOServer*)m_ios)->PostMsg(PCK_SOCKET_CLOSE, (ULONG_PTR)this);
		}
		return true;
	}
	return true;
}

void CTcpSocket::OnClear()
{
	m_cb->OnClose();
	if (m_nNeedDelete != 0)
	{
		DestroyTcpSocket(this);
	}
}
