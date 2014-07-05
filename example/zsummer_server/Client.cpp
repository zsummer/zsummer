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
	m_socket->DoRecv(m_recving._orgdata, 2);
}

bool CClient::OnRecv(unsigned int nRecvedLen)
{
	m_curRecvLen += nRecvedLen;
	

	std::pair<zsummer::protocol4z::INTEGRITY_RET_TYPE, zsummer::protocol4z::DefaultStreamHeadTraits::Integer> ret = zsummer::protocol4z::CheckBuffIntegrity<zsummer::protocol4z::DefaultStreamHeadTraits>(m_recving._orgdata, m_curRecvLen, _MSG_BUF_LEN);

	if (ret.first == zsummer::protocol4z::IRT_CORRUPTION)
	{
		LOGE("killed socket: CheckBuffIntegrity error ");
		m_socket->Close();
		return false;
	}
	if (ret.first == zsummer::protocol4z::IRT_SHORTAGE)
	{
		m_socket->DoRecv(m_recving._orgdata+m_curRecvLen, ret.second);
		return true;
	}

	//! 解包完成 进行消息处理
	zsummer::protocol4z::ReadStream<zsummer::protocol4z::DefaultStreamHeadTraits> rs(m_recving._orgdata, m_curRecvLen);
	try
	{
		MessageEntry(rs);
	}
	catch (std::runtime_error e)
	{
		LOGE("MessageEntry catch one exception: "<< e.what() );
		m_socket->Close();
		return false;
	}

	m_process->AddTotalRecvCount(1);
	m_process->AddTotalRecvLen(m_curRecvLen);
	//! 继续收包
	m_recving._len = 0;
	m_curRecvLen = 0;
	m_socket->DoRecv(m_recving._orgdata, 2);
	return true;
}
void CClient::MessageEntry(zsummer::protocol4z::ReadStream<zsummer::protocol4z::DefaultStreamHeadTraits> & rs)
{
	//协议流异常会被上层捕获并关闭连接
	unsigned short protocolID = 0;
	rs >> protocolID;
	switch (protocolID)
	{
	case 1:
		{
			unsigned long long clientTick = 0;
			m_textCache.clear();
			rs >> clientTick >> m_textCache;
			char buf[_MSG_BUF_LEN];
			zsummer::protocol4z::WriteStream<zsummer::protocol4z::DefaultStreamHeadTraits> ws(buf, _MSG_BUF_LEN);
			ws << protocolID << clientTick << m_textCache;
			Send(buf, ws.GetStreamLen());
		}
		break;
	default:
		{
			LOGI("unknown protocol id = " << protocolID);
		}
		break;
	}
}
void CClient::Send(char * buf, unsigned int len)
{
	if (m_sending._len != 0)
	{
		Packet *p = new Packet;
		memcpy(p->_orgdata, buf, len);
		p->_len = len;
		m_sendque.push(p);
	}
	else
	{
		memcpy(m_sending._orgdata, buf, len);
		m_sending._len= len;
		m_socket->DoSend(m_sending._orgdata, m_sending._len);
	}
}

bool CClient::OnConnect(bool bConnected)
{
	return true;
}

bool CClient::OnSend(unsigned int nSentLen)
{
	m_curSendLen += nSentLen;
	if (m_curSendLen < m_sending._len)
	{
		m_socket->DoSend(&m_sending._orgdata[m_curSendLen], m_sending._len - m_curSendLen);
	}
	else if (m_curSendLen == m_sending._len)
	{
		m_process->AddTotalSendCount(1);
		m_process->AddTotalSendLen(m_curSendLen);
		m_curSendLen = 0;
		if (m_sendque.empty())
		{
			m_sending._len = 0;
		}
		else
		{
			Packet *p = m_sendque.front();
			m_sendque.pop();
			memcpy(m_sending._orgdata, p->_orgdata, p->_len);
			delete p;
			m_socket->DoSend(m_sending._orgdata, m_sending._len);
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

