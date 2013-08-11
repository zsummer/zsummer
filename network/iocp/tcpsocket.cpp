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


	//send
	memset(&m_sendHandle, 0, sizeof(m_sendHandle));
	m_sendHandle._type = tagReqHandle::HANDLE_SEND;
	m_sendWsaBuf.buf = NULL;
	m_sendWsaBuf.len = 0;

	//connect

	memset(&m_connectHandle, 0, sizeof(m_connectHandle));
	m_connectHandle._type = tagReqHandle::HANDLE_CONNECT;

	//status
	m_isRecving = false;
	m_isSending = false;
	m_isConnecting = false;
	m_nLinkStatus = LS_UNINITIALIZE;
	m_nNeedDelete = 0;
}


CTcpSocket::~CTcpSocket()
{
	if (m_socket != INVALID_SOCKET)
	{
		LCF("Destruct CTcpSocket Error. socket handle not invalid, socket=" << (unsigned int)m_socket);
		m_socket = INVALID_SOCKET;
	}
}

bool CTcpSocket::Initialize(IIOServer * ios, ITcpSocketCallback * cb)
{
	m_ios = ios;
	m_cb = cb;
	if (m_socket == INVALID_SOCKET)
	{
		m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_socket == INVALID_SOCKET)
		{
			LCE("connecter create socket  error! ERRCODE=" << WSAGetLastError());
			return false;
		}
		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(SOCKADDR_IN));
		addr.sin_family = AF_INET;
		if (bind(m_socket, (sockaddr *) &addr, sizeof(SOCKADDR_IN)) != 0)
		{
			LCE("connecter bind local addr error!  socket=" << (unsigned int)m_socket << ", ERRCODE=" << WSAGetLastError());
			return false;
		}
	}

	if (CreateIoCompletionPort((HANDLE)m_socket, ((CIOServer *)m_ios)->m_io, (ULONG_PTR)this, 1) == NULL)
	{
		LCE("bind socket to IOCP error. socket="<< (unsigned int) m_socket << ", ERRCODE=" << GetLastError());
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	return true;
}





typedef  BOOL (PASCAL  *ConnectEx)(  SOCKET s,  const struct sockaddr* name,  int namelen,
			 PVOID lpSendBuffer,  DWORD dwSendDataLength,  LPDWORD lpdwBytesSent,
			 LPOVERLAPPED lpOverlapped);

bool CTcpSocket::DoConnect(const char *ip, unsigned short port)
{
	if (m_cb == NULL)
	{
		LCF("CTcpSocket::DoConnect callback pointer uninitialize. socket=" << (unsigned int)m_socket);
		return false;
	}
	if (m_ios == NULL)
	{
		LCF("CTcpSocket::DoConnect IIOServer pointer uninitialize.socket=" << (unsigned int) m_socket);
		return false;
	}
	if (m_nLinkStatus != LS_UNINITIALIZE)
	{
		LCF("CTcpSocket::DoConnect socket already used. socket="<<(unsigned int) m_socket);
		return false;
	}
	if (m_isConnecting)
	{
		LCF("CTcpSocket::DoConnect socket is connecting. socket="<<(unsigned int) m_socket);
		return false;
	}

	GUID gid = WSAID_CONNECTEX;
	ConnectEx lpConnectEx = NULL;
	DWORD dwSize = 0;
	if (WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &gid, sizeof(gid), &lpConnectEx, sizeof(lpConnectEx), &dwSize, NULL, NULL) != 0)
	{
		LCE("Get ConnectEx pointer err!  socket="<< (unsigned int) m_socket <<", ERRCODE= "<< WSAGetLastError());
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
			LCE("CTcpSocket::DoConnect DoConnect failed and ERRCODE!=ERROR_IO_PENDING, socket="<< (unsigned int) m_socket << ", ERRCODE=" << WSAGetLastError());
			return false;
		}

	}
	m_isConnecting = true;
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
		LCF("CTcpSocket::DoSend callback pointer uninitialize. socket=" << (unsigned int)m_socket);
		return false;
	}
	if (m_ios == NULL)
	{
		LCF("CTcpSocket::DoSend IIOServer pointer uninitialize.socket=" << (unsigned int) m_socket);
		return false;
	}
	if (m_nLinkStatus != LS_ESTABLISHED)
	{
		LCF("CTcpSocket::DoSend socket status != LS_ESTABLISHED. socket="<<(unsigned int) m_socket);
		return false;
	}
	if (m_isSending)
	{
		LCF("CTcpSocket::DoSend socket is sending. socket="<<(unsigned int) m_socket);
		return false;
	}
	if (len == 0)
	{
		LCF("CTcpSocket::DoSend length is 0. socket="<<(unsigned int) m_socket);
		return false;
	}

	m_sendWsaBuf.buf = buf;
	m_sendWsaBuf.len = len;

	DWORD dwTemp1=0;
	if (WSASend(m_socket, &m_sendWsaBuf, 1, &dwTemp1, 0, &m_sendHandle._overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LCE("CTcpSocket::DoSend DoSend failed and ERRCODE!=ERROR_IO_PENDING, socket="<< (unsigned int) m_socket << ", ERRCODE=" << WSAGetLastError());
			return false;
		}
	}
	m_isSending = true;
	return true;
}


bool CTcpSocket::DoRecv(char * buf, unsigned int len)
{
	if (m_cb == NULL)
	{
		LCF("CTcpSocket::DoRecv callback pointer uninitialize. socket=" << (unsigned int)m_socket);
		return false;
	}
	if (m_ios == NULL)
	{
		LCF("CTcpSocket::DoRecv IIOServer pointer uninitialize.socket=" << (unsigned int) m_socket);
		return false;
	}
	if (m_nLinkStatus != LS_ESTABLISHED)
	{
		LCF("CTcpSocket::DoRecv socket status != LS_ESTABLISHED. socket="<<(unsigned int) m_socket);
		return false;
	}
	if (m_isRecving)
	{
		LCF("CTcpSocket::DoRecv socket is recving. socket="<<(unsigned int) m_socket);
		return false;
	}
	if (len == 0)
	{
		LCF("CTcpSocket::DoRecv length is 0. socket="<<(unsigned int) m_socket);
		return false;
	}


	m_recvWSABuf.buf = buf;
	m_recvWSABuf.len = len;

	DWORD dwRecv = 0;
	DWORD dwFlag = 0;
	if (WSARecv(m_socket, &m_recvWSABuf, 1, &dwRecv, &dwFlag, &m_recvHandle._overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LCE("CTcpSocket::DoRecv DoRecv failed and ERRCODE!=ERROR_IO_PENDING, socket="<< (unsigned int) m_socket << ", ERRCODE=" << WSAGetLastError());
			return false;
		}
	}
	m_isRecving = true;
	return true;
}

bool CTcpSocket::OnIOCPMessage(BOOL bSuccess, DWORD dwTranceCount, unsigned char cType)
{
	if (cType == tagReqHandle::HANDLE_CONNECT)
	{
		m_isConnecting = false;
		if (bSuccess)
		{
			m_nLinkStatus = LS_ESTABLISHED;
		}
		else
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		m_cb->OnConnect(!!bSuccess);
		return true;
	}



	if (cType == tagReqHandle::HANDLE_RECV)
	{
		//server side active close.
		m_isRecving = false;
		if (!bSuccess)
		{
			CloseImlp();
			return true;
		}
		// client side active closed
		if (dwTranceCount == 0)
		{
			CloseImlp();
			return true;
		}
		m_cb->OnRecv((unsigned int)dwTranceCount);
	}
	else if (cType == tagReqHandle::HANDLE_SEND)
	{
		//server side active close.
		m_isSending = false;
		if (!bSuccess)
		{
			CloseImlp();
			return true;
		}
		m_cb->OnSend((unsigned int)dwTranceCount);
	}
	return true;
}


bool CTcpSocket::Close()
{
	if (m_nLinkStatus == LS_ESTABLISHED)
	{
		CloseImlp();
		return true;
	}
	else if (m_nLinkStatus == LS_WAITCLEAR)
	{
		LCW("CTcpSocket::Close(),  socket status = LS_WAITCLEAR, socket=" << (unsigned int) m_socket);
	}
	else if (m_nLinkStatus == LS_UNINITIALIZE)
	{
		LCW("CTcpSocket::Close(),  socket status = LS_UNINITIALIZE, socket=" << (unsigned int) m_socket);
	}
	else if (m_nLinkStatus == LS_WAITCLOSE)
	{
		LCW("CTcpSocket::Close(),  socket status = LS_WAITCLOSE, socket=" << (unsigned int) m_socket);
	}
	return true;
}

bool CTcpSocket::CloseImlp()
{
	if (m_nLinkStatus == LS_ESTABLISHED)
	{
		m_nLinkStatus = LS_WAITCLOSE;
		closesocket(m_socket);
		if (!m_isRecving && !m_isSending)
		{
			goto postClear;
		}
		return true;
	}
	else if (m_nLinkStatus == LS_WAITCLOSE)
	{
		if (!m_isRecving && !m_isSending)
		{
			goto postClear;
		}
		LCI("CTcpSocket::CloseImlp(),  socket status = LS_WAITCLOSE, socket=" << (unsigned int) m_socket);
	}
	else if (m_nLinkStatus == LS_WAITCLEAR)
	{
		LCW("CTcpSocket::CloseImlp(),  socket status = LS_WAITCLEAR, socket=" << (unsigned int) m_socket);
	}
	else if (m_nLinkStatus == LS_UNINITIALIZE)
	{
		LCW("CTcpSocket::CloseImlp(),  socket status = LS_UNINITIALIZE, socket=" << (unsigned int) m_socket);
	}

	return true;
postClear:
	m_nLinkStatus = LS_WAITCLEAR;
	if (m_ios)
	{
		((CIOServer*)m_ios)->PostMsg(PCK_SOCKET_CLOSE, (ULONG_PTR)this);
	}
	else
	{
		LCW("CTcpSocket::Close() IOServer uninitialize.  socket=" << (unsigned int) m_socket);
		m_nLinkStatus = LS_UNINITIALIZE;
		m_socket = INVALID_SOCKET;
		if (m_nNeedDelete != 0)
		{
			DestroyTcpSocket(this);
		}
		m_nNeedDelete = 0;
	}
	return true;
}

void CTcpSocket::OnClear()
{
	m_cb->OnClose();
	m_nLinkStatus = LS_UNINITIALIZE;
	m_socket = INVALID_SOCKET;
	if (m_nNeedDelete != 0)
	{
		DestroyTcpSocket(this);
	}
}
