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


#include "Schedule.h"

#include "Process.h"

CSchedule::CSchedule()
{
	m_iCurProcess = 0;
	m_bRunning = false;
}

void CSchedule::Start()
{
	for (int i=0; i< 1; i++)
	{
		CProcess * p = new CProcess;
		if (p->Start())
		{
			m_process.push_back(p);
		}
	}
	m_ios = CreateIOServer();
	if (!m_ios->Initialize(this))
	{
		LOGE("manager start fail!");
		return;
	}

	m_accept = CreateTcpAccept();
	m_accept->Initialize(m_ios, this);
	if (m_accept->OpenAccept("0.0.0.0", 81))
	{
		LOGI("OpenAccept 81 success");
	}
	else
	{
		LOGF("OpenAccept 81 failed");
		return;
	}
	m_bRunning = true;
	CThread::Start();
}
void CSchedule::Stop()
{
	m_bRunning = false;
}
void CSchedule::Run()
{
	while (m_bRunning)
	{
		m_ios->RunOnce();
	}
}
bool CSchedule::OnStop()
{
	return true;
}

bool CSchedule::OnPost(void *pUser)
{
	return true;
}
bool CSchedule::OnTimer()
{
	//LOGD("CSchedule::OnTimer():"<<GetTimeMillisecond());
	return true;
}


bool CSchedule::OnAccept(ITcpSocket * s)
{
	//LOGI("OnAccept one link ");
	m_process[m_iCurProcess]->Post((void *) s);
	m_iCurProcess++;
	m_iCurProcess = m_iCurProcess%(int)m_process.size();
	g_nTotalLinked ++;
	return true;
}

bool CSchedule::OnClose()
{
	LOGD("accept socket do close");
	DestroyTcpAccept(m_accept);
	Stop();
	return true;
}

