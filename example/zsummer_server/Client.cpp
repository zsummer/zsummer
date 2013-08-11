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

#include "Client.h"
#include "Process.h"
#include "../../depends/protocol4z/protocol4z.h"
CClient::CClient()
{
	m_process = NULL;
	m_socket = NULL;

	m_type = 1;
	memset(&m_recving, 0, sizeof(m_recving));
	m_curRecvLen = 0;
	memset(&m_sending, 0, sizeof(m_sending));
	m_curSendLen = 0;
}

CClient::~CClient()
{
}

void CClient::InitSocket(CProcess *proc, ITcpSocket *s)
{
	m_process = proc;
	m_socket = s;
	m_socket->DoRecv(m_recving._body, 2);
}

bool CClient::OnRecv(unsigned int nRecvedLen)
{
	m_curRecvLen += nRecvedLen;
	m_process->AddTotalRecvCount(1);
	int needRecv = zsummer::protocol4z::CheckBuffIntegrity(m_recving._body, m_curRecvLen, _MSG_BUF_LEN);
	if ( needRecv == -1)
	{
		LOGE("killed socket: CheckBuffIntegrity error ");
		m_socket->Close();
		return false;
	}
	if (needRecv > 0)
	{
		m_socket->DoRecv(m_recving._body+m_curRecvLen, needRecv);
		return true;
	}



	//! 解包
	unsigned short protocolID = 0;
	unsigned short requestID = 0;
	unsigned long long counter = 0;
	unsigned int sendtime = 0;
	std::string text;
	zsummer::protocol4z::ReadStream rs(m_recving._body, m_curRecvLen);
	try
	{
		rs >> protocolID >> requestID >>counter >> sendtime >> text;
	}
	catch (std::runtime_error e)
	{
		LOGE("recv msg catch one exception: "<< e.what() );
		m_socket->Close();
		return false;
	}
	//begin logic
	counter++;
	//end logic

	if (m_sending._head != 0)
	{
		Packet *p = new Packet;
		zsummer::protocol4z::WriteStream ws(p->_body, _MSG_BUF_LEN);
		try
		{
			ws << protocolID << requestID << counter << sendtime << text;
			m_sendque.push(p);
		}
		catch (std::runtime_error e)
		{
			delete p;
			LOGE("send msg catch one exception: "<< e.what() );
			m_socket->Close();
			return false;
		}

	}
	else
	{
		zsummer::protocol4z::WriteStream ws(m_sending._body, _MSG_BUF_LEN);
		try
		{
			ws << protocolID << requestID << counter << sendtime << text;

		}
		catch (std::runtime_error e)
		{
			LOGE("send msg catch one exception: "<< e.what() );
			m_socket->Close();
			return false;
		}
		m_socket->DoSend(m_sending._body, m_sending._head);
	}

	m_process->AddTotalRecvLen(m_recving._head);
	memset(&m_recving, 0, sizeof(m_recving));
	m_curRecvLen = 0;
	m_socket->DoRecv(m_recving._body, 2);
	
	return true;
}

bool CClient::OnConnect(bool bConnected)
{
	return true;
}

bool CClient::OnSend(unsigned int nSentLen)
{
	m_process->AddTotalSendCount(1);
	m_process->AddTotalSendLen(nSentLen);
	m_curSendLen += nSentLen;
	if (m_curSendLen < m_sending._head)
	{
		m_socket->DoSend(&m_sending._body[m_curSendLen], m_sending._head - m_curSendLen);
	}
	else if (m_curSendLen == m_sending._head)
	{
		m_curSendLen = 0;
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

