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

#include "Client.h"

CClient::CClient()
{
	m_socket = NULL;
	m_type = 1;
	memset(&m_send, 0, sizeof(m_send));
	m_sending = false;
	memset(&m_recv, 0, sizeof(m_recv));
}
CClient::~CClient()
{
}

void CClient::SetSocket(ITcpSocket *s)
{
	m_socket = s;
	memset(&m_send, 0, sizeof(m_send));
	m_sending = false;
	memset(&m_recv, 0, sizeof(m_recv));
	m_type = 1;
	m_socket->SetCallback(this);
	m_socket->DoRecv((char*)&m_recv._head, 2);
}

bool CClient::OnRecv()
{
	//先收取2字节包头
	if (m_type == 1)
	{
		m_type = 2;
		if (m_recv._head <= 2 || m_recv._head >= _MSG_LEN)
		{
			LOGE("recv body len = " << m_recv._head);
			m_socket->Close();
			return false;
		}
		m_socket->DoRecv(m_recv._body, m_recv._head-2);
	}
	//收取包体内容
	else if (m_type == 2)
	{
		m_recv._body[m_recv._head-2] = '\0';
		AtomicAdd(&g_nTotalRecvLen, m_recv._head);
		LOGD("recv msg: " << m_recv._body);
		if (m_sending)
		{
			tagPacket * p =new tagPacket;
			memcpy(p, &m_recv, sizeof(m_recv));
			m_sendque.push(p);
		}
		else
		{
			memcpy(m_send, &m_recv._head, 2);
			memcpy(m_send+2, m_recv._body, m_recv._head-2);
			m_sending = true;
			m_socket->DoSend(m_send, m_recv._head);
			AtomicAdd(&g_nTotalSendLen, m_recv._head);
		}
		m_type = 1;
		m_socket->DoRecv((char*)&m_recv, 2);
	}
	return true;
}

bool CClient::OnConnect(bool bConnected)
{
	return true;
}

bool CClient::OnSend()
{
	if (m_sendque.empty())
	{
		m_sending = false;
	}
	else
	{
		tagPacket *p = m_sendque.front();
		m_sendque.pop();
		memcpy(m_send, (const void *)p->_head, 2);
		memcpy(m_send+2, p->_body, p->_head-2);
		m_socket->DoSend(m_send, p->_head);
		AtomicAdd(&g_nTotalSendLen, m_recv._head);
		delete p;
	}
	return true;
}

bool CClient::OnClose()
{
	LOGI("Client Closed!");
	delete this;
	AtomicAdd(&g_nTotalCloesed, 1);
	return true;
}

