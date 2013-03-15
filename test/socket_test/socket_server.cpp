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

#include "../../utility/utility.h"
#include "../../tools/thread4z/thread.h"
#include "../../utility/objectpool.h"
#include "../../network/SocketInterface.h"
#include <queue>
#include "../../tools/log4z/log4z.h"
#include <iostream>
#include <string.h>
#include <signal.h>
using namespace std;
using namespace zsummer::log4z;
using namespace zsummer::utility;
using namespace zsummer::thread4z;
using namespace zsummer::network;
class CIOProcess;


static int g_cloesdclt = 0;


#define _MSG_BUF_LEN	(5*1024)
#define _MSG_LEN   (5000)
struct tagPacket
{
	unsigned short _head;
	char		   _body[_MSG_BUF_LEN];
};


class CClient :public ITcpSocketCallback
{
public:
	CClient()
	{
		m_socket = NULL;
		m_type = 1;
		memset(&m_send, 0, sizeof(m_send));
		m_sending = false;
		memset(&m_recv, 0, sizeof(m_recv));
	}
	~CClient()
	{
	}
	void SetSocket(ITcpSocket *s)
	{
		m_socket = s;
		memset(&m_send, 0, sizeof(m_send));
		m_sending = false;
		memset(&m_recv, 0, sizeof(m_recv));
		m_type = 1;
		s->SetCallback(this);
		s->DoRecv((char*)&m_recv._head, 2);
	}

	virtual bool OnRecv()
	{
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
		else if (m_type == 2)
		{
			m_recv._body[m_recv._head-2] = '\0';
//			LOGD("recv msg: " << m_recv._body);

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
			}


			m_type = 1;
			m_socket->DoRecv((char*)&m_recv, 2);
		}
		return true;
	}

	virtual bool OnConnect(bool bConnected)
	{
		return true;
	}

	virtual bool OnSend()
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
		}
		return true;
	}

	virtual bool OnClose()
	{
		LOGA("onclose");
		delete this;
		AtomicAdd(&g_cloesdclt, 1);
		return true;
	}

	ITcpSocket * m_socket;
	tagPacket m_recv;
	unsigned char m_type; //1,2  header, body

	std::queue<tagPacket *> m_sendque;
	char m_send[_MSG_BUF_LEN];
	bool m_sending;
};


// io 追随者
class CIOProcess : public CThread, public IIOServerCallback
{
public:
	CIOProcess()
	{
		m_ios = NULL;
	}
	bool Start()
	{
		m_ios = CreateIOServer();
		if (!m_ios->Start(this))
		{
			LOGE("process start fail!");
			return false;
		}
		return CThread::Start();
	}
	void Stop()
	{
		m_ios->Stop();
	}

	virtual void Run()
	{
		m_ios->Run();
	}
	virtual void Post(void * pUser)
	{
		m_ios->Post(pUser);
	}
	virtual bool OnStop(){return true;}
	virtual bool OnMsg(void *pUser)
	{
		ITcpSocket * s = (ITcpSocket *) pUser;
		CClient * p = new CClient;
		s->BindIOServer(m_ios);
		p->SetSocket(s);
		return true;
	}
private:
	IIOServer * m_ios;
};



// IO 监听 调度
class CIOManager :public ITcpAcceptCallback, public IIOServerCallback, public CThread
{
public:
	CIOManager()
	{
		m_iCurProcess = 0;
		m_clts = 0;
	}
	void Start()
	{
		

		for (int i=0; i< 2; i++)
		{
			CIOProcess * p = new CIOProcess;
			if (p->Start())
			{
				m_process.push_back(p);
			}
		}

		m_ios = CreateIOServer();
		if (!m_ios->Start(this))
		{
			LOGE("manager start fail!");
			return;
		}
		m_accept = CreateTcpAccept();
		m_accept->SetCallbck(this);
		m_accept->BindIOServer(m_ios);
		m_accept->OpenAccept("0.0.0.0", 81);
		CThread::Start();
	}
	void Stop()
	{
		m_ios->Stop();
	}
	void Run()
	{
		m_ios->Run();
	}
	virtual bool OnStop(){return true;}
	virtual bool OnMsg(void *pUser){return true;}
	virtual bool OnAccept(ITcpSocket * s)
	{
		//LOGI("OnAccept one link ");
		m_process[m_iCurProcess]->Post((void *) s);
		m_iCurProcess++;
		m_iCurProcess = m_iCurProcess%(int)m_process.size();
		m_clts ++;
		return true;
	}

	virtual bool OnClose()
	{
		LOGD("accept socket do close");
		DestroyTcpAccept(m_accept);
		Stop();
		return true;
	}

	ITcpAccept * m_accept;
	IIOServer * m_ios;
	std::vector<CIOProcess *> m_process;
	int						m_iCurProcess;
public:

	int m_clts;
};



int main(int argc, char* argv[])
{
#ifndef _WIN32
	signal( SIGHUP, SIG_IGN );
	signal( SIGALRM, SIG_IGN ); 
	signal( SIGPIPE, SIG_IGN );
	signal( SIGXCPU, SIG_IGN );
	signal( SIGXFSZ, SIG_IGN );
	signal( SIGPROF, SIG_IGN ); 
	signal( SIGVTALRM, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	signal( SIGCHLD, SIG_IGN);
#endif
	ILog4zManager::GetInstance()->Start();
// 	ILog4zManager::GetInstance()->ChangeLoggerDisplay(ILog4zManager::GetInstance()->GetMainLogger(), false);
	//ILog4zManager::GetInstance()->ChangeLoggerLevel(ILog4zManager::GetInstance()->GetMainLogger(), LOG_LEVEL_INFO);

	CIOManager m;
	m.Start();  //启动主IO

	for (;;)
	{
		SleepMillisecond(2000);
		LOGI("accept clients:" << m.m_clts <<"   closed clients:" << g_cloesdclt);
	}
	m.Stop();


	//InterfaceLogger::GetInstance()->Stop();
	return 0;
}

