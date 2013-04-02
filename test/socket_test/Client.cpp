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
#include "IOServer.h"

CClient::CClient()
{
	m_ios = NULL;
	m_socket = NULL;

	m_type = 1;
	memset(&m_recving, 0, sizeof(m_recving));
	memset(&m_sending, 0, sizeof(m_sending));
}

CClient::~CClient()
{
}

void CClient::InitSocket(CIOServer *ios, ITcpSocket *s)
{
	m_ios = ios;
	m_socket = s;
	m_socket->SetCallback(this);

	m_socket->DoRecv((char*)&m_recving._head, 2);
}

bool CClient::OnRecv()
{
	//! 先收包头
	m_ios->AddTotalRecvCount(1);
	if (m_type == 1)
	{
		m_type = 2;
		if (m_recving._head <= 2 || m_recving._head >= _MSG_LEN)
		{
			LOGE("recv body len = " << m_recving._head);
			m_socket->Close();
			return false;
		}
		m_socket->DoRecv(m_recving._body+2, m_recving._head-2);
	}
	//! 收包体
	else if (m_type == 2)
	{
		m_recving._body[m_recving._head] = '\0';
		m_ios->AddTotalRecvLen(m_recving._head);
		//LOGD("recv msg: " << m_recving._body+2);
		if (m_sending._head != 0)
		{
			Packet * p =new Packet;
			memcpy(p, &m_recving, sizeof(Packet));
			m_sendque.push(p);
		}
		else
		{
			memcpy(&m_sending, &m_recving, sizeof(Packet));
			m_socket->DoSend((char*)&m_sending._body, m_sending._head);
		}
		m_type = 1;
		m_socket->DoRecv((char*)&m_recving._head, 2);
	}
	return true;
}

bool CClient::OnConnect(bool bConnected)
{
	return true;
}

bool CClient::OnSend()
{
	m_ios->AddTotalSendCount(1);
	m_ios->AddTotalSendLen(m_sending._head);
	if (m_sendque.empty())
	{
		m_sending._head = 0;
	}
	else
	{
		Packet *p = m_sendque.front();
		m_sendque.pop();
		memcpy((char*)&m_sending, p, sizeof(m_sending));
		m_socket->DoSend(m_sending._body, p->_head);
		delete p;
	}
	return true;
}

bool CClient::OnClose()
{
	LOGI("Client Closed!");
	delete this; //! 安全的自删除源于底层的彻底的异步分离
	AtomicAdd(&g_nTotalCloesed, 1);
	return true;
}

