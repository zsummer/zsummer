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

#include "IOServer.h"
#include "Client.h"

CIOServer::CIOServer()
{
	m_ios = NULL;
	m_nTotalRecvLen = 0;
	m_nTotalSendLen = 0;
	m_nTotalRecvCount = 0;
	m_nTotalSendCount = 0;
}

bool CIOServer::Start()
{
	m_ios = CreateIOServer();
	if (!m_ios->Start(this))
	{
		LOGE("process start fail!");
		return false;
	}
	return CThread::Start();
}

void CIOServer::Stop()
{
	m_ios->Stop();
}

void CIOServer::Run()
{
	m_ios->Run();
}

void CIOServer::Post(void * pUser)
{
	m_ios->Post(pUser);
}

bool CIOServer::OnStop()
{
	return true;
}
//来自Schedule的消息
bool CIOServer::OnMsg(void *pUser)
{
	ITcpSocket * s = (ITcpSocket *) pUser;
	CClient * p = new CClient;
	s->BindIOServer(m_ios);
	p->InitSocket(this, s);
	return true;
}

