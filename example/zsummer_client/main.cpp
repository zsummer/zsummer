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

//! zsummer的测试客户端模块
//! main文件


#include "../../utility/utility.h"
#include "../../depends/thread4z/thread.h"
#include "../../depends/log4z/log4z.h"
#include "../../network/SocketInterface.h"
#include "../../depends/protocol4z/protocol4z.h"
#include <iostream>
#include <queue>
#include <iomanip>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
using namespace std;


//! 消息包缓冲区大小
#define _MSG_BUF_LEN	(2*1024)
std::string g_fillString;
int g_ping_pong = 1;

//! 消息包 
struct Packet
{
	unsigned short _len;
	unsigned long long   _senddelay;
	char		   _orgdata[_MSG_BUF_LEN];
};


//统计

enum TYPE_DELAY
{
	TD_SEND,
	TD_RECV,
	TD_TOTAL,
	TD_END,
};
enum TIME_DELAY
{
	TM_1MS, // < 1 ms
	TM_5MS, // < 5 ms
	TM_10MS,
	TM_20MS,
	TM_40MS,
	TM_60MS,
	TM_100MS,
	TM_1000MS,
	TM_LOWMS,// error dalay time
	TM_END,
};
unsigned long long g_delay[TD_END][TM_END] = {{0}};
inline void addDelayData(TYPE_DELAY td, unsigned long long usedtime)
{
	if (usedtime <= 1)
	{
		g_delay[td][TM_1MS]++;
	}
	else if (usedtime <= 5)
	{
		g_delay[td][TM_5MS]++;
	}
	else if (usedtime <= 10)
	{
		g_delay[td][TM_10MS]++;
	}
	else if (usedtime <= 20)
	{
		g_delay[td][TM_20MS]++;
	}
	else if (usedtime <= 40)
	{
		g_delay[td][TM_40MS]++;
	}
	else if (usedtime <= 60)
	{
		g_delay[td][TM_60MS]++;
	}
	else if (usedtime <= 100)
	{
		g_delay[td][TM_100MS]++;
	}
	else if (usedtime <= 1000)
	{
		g_delay[td][TM_1000MS]++;	
	}
	else
	{
		g_delay[td][TM_LOWMS]++;
	}
}
zsummer::network::IIOServer * g_pios;
//! 上层Socekt Client的二次封装
class CClient :public zsummer::network::ITcpSocketCallback, public zsummer::network::ITimerCallback
{
public:
	CClient()
	{
		m_socket = NULL;
		m_establish = 0;
		memset(&m_recving, 0, sizeof(m_recving));
		m_curRecvLen = 0;
		memset(&m_sending, 0, sizeof(m_sending));
		m_curSendLen = 0;
	}
	~CClient()
	{

	}
	void OnTimer(unsigned long long timerID)
	{
		if (m_establish)
		{
			SendOnce();
		}
	}
	virtual bool OnRecv(unsigned int nRecvedLen)
	{
		m_curRecvLen += nRecvedLen;
		int needRecv = zsummer::protocol4z::CheckBuffIntegrity(m_recving._orgdata, m_curRecvLen, _MSG_BUF_LEN);
		if ( needRecv == -1)
		{
			LOGE("killed socket: CheckBuffIntegrity error ");
			m_socket->Close();
			return false;
		}
		if (needRecv > 0)
		{
			m_socket->DoRecv(m_recving._orgdata+m_curRecvLen, needRecv);
			return true;
		}

		//! 解包完成 进行消息处理
		unsigned long long testTimeUsed = zsummer::utility::GetTimeMillisecond();
		zsummer::protocol4z::ReadStream rs(m_recving._orgdata, m_curRecvLen);
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
		//! 继续收包
		m_recving._len = 0;
		m_recving._senddelay = 0;
		m_curRecvLen = 0;
		testTimeUsed = zsummer::utility::GetTimeMillisecond()-testTimeUsed;
		addDelayData(TD_RECV, testTimeUsed);
		m_socket->DoRecv(m_recving._orgdata, 2);
		if (g_ping_pong)
		{
			SendOnce();
		}
		else
		{
			g_pios->CreateTimer(9000+rand()%1500, this);
		}
		return true;
	}
	virtual bool OnConnect(bool bConnected)
	{
		if (bConnected)
		{
			m_establish = 1;
			OnRecv(0);
			LOGI("establish connect");
			g_pios->CreateTimer(1000+rand()%1000, this);
		}
		else
		{
			LOGI("connect false!");
		}
		return true;
	}
	virtual bool OnSend(unsigned int nSentLen)
	{
		m_curSendLen += nSentLen;
		if (m_curSendLen < m_sending._len)
		{
			m_socket->DoSend(&m_sending._orgdata[m_curSendLen], m_sending._len - m_curSendLen);
		}
		else if (m_curSendLen == m_sending._len)
		{
			unsigned long long senduse = zsummer::utility::GetTimeMillisecond();
			senduse = senduse - m_sending._senddelay;
			addDelayData(TD_SEND, senduse);
			
			m_curSendLen = 0;
			if (m_sendque.empty())
			{
				m_sending._len = 0;
				m_sending._senddelay = 0;
			}
			else
			{
				Packet *p = m_sendque.front();
				m_sendque.pop();
				memcpy(m_sending._orgdata, p->_orgdata, p->_len);
				m_sending._len = p->_len;
				m_sending._senddelay = p->_senddelay;
				delete p;
				m_socket->DoSend(m_sending._orgdata, m_sending._len);
			}
		}
		return true;
	}
	virtual bool OnClose()
	{
		LOGI("Client Closed!");
		m_establish = 0;
		return true;
	}
	void MessageEntry(zsummer::protocol4z::ReadStream & rs)
	{
		//协议流异常会被上层捕获并关闭连接
		unsigned short protocolID = 0;
		rs >> protocolID;
		switch (protocolID)
		{
		case 1:
			{
				unsigned long long localTick = 0;
				std::string text;
				rs >> localTick >> text;
				
				unsigned long long curTick = zsummer::utility::GetTimeMillisecond();
				curTick = curTick - localTick;
				addDelayData(TD_TOTAL, curTick);
				
				
			}
			break;
		default:
			{
				LOGI("unknown protocol id = " << protocolID);
			}
			break;
		}
	}
	void SendOnce()
	{

		Packet *p=NULL;
		if (m_sending._len != 0)
		{
			p = new Packet;
		}
		else
		{
			p = &m_sending;
		}
		p->_senddelay = zsummer::utility::GetTimeMillisecond();
		zsummer::protocol4z::WriteStream ws(p->_orgdata, _MSG_BUF_LEN);
		ws << (unsigned short) 1; //protocol id
		ws << p->_senddelay; // local tick count
		ws << g_fillString; // append text, fill the length protocol.
		p->_len = ws.GetWriteLen();
		if (p == &m_sending)
		{
			m_socket->DoSend(p->_orgdata, p->_len);
		}
		else
		{
			//LOGW("m_sendque.push(p)" <<p);
			m_sendque.push(p);
		}
	}
	zsummer::network::ITcpSocket * m_socket;
	unsigned char m_establish;

	//! 读包
	Packet m_recving;
	unsigned short m_curRecvLen;
	//! 写包队列
	std::queue<Packet *> m_sendque;

	//! 当前写包
	Packet m_sending;
	unsigned short m_curSendLen;
};

class CZSummer : public zsummer::network::IIOServerCallback , public zsummer::network::ITimerCallback, public zsummer::network::IUdpSocketCallback
{
public:
	CZSummer()
	{
		m_udp = NULL;
		m_clientMax = 0;
	}
	~CZSummer()
	{
		if (g_pios)
		{
			zsummer::network::DestroyIOServer(g_pios);
			g_pios = NULL;
		}
	}
	bool Run(std::string ip, unsigned short port, int clients)
	{
		m_ip = ip;
		m_port = port;
		m_clientMax = clients;
		g_pios =zsummer::network::CreateIOServer();
		if (!g_pios)
		{
			return false;
		}
		if (!g_pios->Initialize(this))
		{
			zsummer::network::DestroyIOServer(g_pios);
			g_pios = NULL;
			return false;
		}
		m_udp = zsummer::network::CreateUdpSocket();
		m_udp->Initialize(g_pios, this, "0.0.0.0", 82);
		m_udp->DoRecv(m_udpbuf, 1500);
		g_pios->CreateTimer(1000, this);
		while(true)
		{
			g_pios->RunOnce();
			
		}
		return true;
	}

	virtual bool OnPost(void *pUser)
	{
		return true;
	}
	virtual bool OnRecvFrom(unsigned int nRecvlen, const char *ip, unsigned short port)
	{
		m_udpbuf[nRecvlen] ='\0';
		LOGI("UDP recv data: " << m_udpbuf << ", len=" << nRecvlen << ", ip=" << ip << ", port=" << port);
		m_udp->DoSend(m_udpbuf, nRecvlen, ip, port);
		m_udp->DoRecv(m_udpbuf, 1500);
		return true;
	}
	//! IIOServer's Timerr. per 1 seconds trigger. Don't spend too much time in here.
	virtual void OnTimer(unsigned long long timerID)
	{

		//限定每秒200 connecter
		size_t clientCount=m_clients.size();
		if (clientCount < m_clientMax)
		{
			int n=0;

			while (clientCount++ < m_clientMax && n++ < 200)
			{
				CClient * p = new CClient;
				p->m_socket = zsummer::network::CreateTcpSocket();
				p->m_socket->Initialize(g_pios, p);
				p->m_socket->DoConnect(m_ip.c_str(), m_port);
				m_clients.push_back(p);
			}
		}

		//触发统计
		{
			static unsigned int count = 0;
			if (count%5 == 0)
			{
				LOGI("-- type -- 1MS -- 5MS -- 10MS -- 20MS -- 40MS -- 60MS -- 100MS -- 1S -- LOW MS --");
				for (int i=0; i<TD_END; i++)
				{
					LOGI("--  " << i << " "
					<< " -- " << g_delay[i][TM_1MS]
					<< " -- " << g_delay[i][TM_5MS]
					<< " -- " << g_delay[i][TM_10MS]
					<< " -- " << g_delay[i][TM_20MS]
					<< " -- " << g_delay[i][TM_40MS]
					<< " -- " << g_delay[i][TM_60MS]
					<< " -- " << g_delay[i][TM_100MS]
					<< " -- " << g_delay[i][TM_1000MS]
					<< " -- " << g_delay[i][TM_LOWMS] << " --");
				}

				//test udp
				//m_udp->DoSend("123456789", 9, "127.0.0.1", 82);
			}
			count++;
		}
		g_pios->CreateTimer(1000, this);
	}
	
private:
	zsummer::network::IUdpSocket * m_udp;
	char m_udpbuf[1500];
	std::string m_ip;
	unsigned short m_port;
	size_t m_clientMax;
	std::vector<CClient*> m_clients;
};
 

int main(int argc, char* argv[])
{

#ifndef _WIN32
	//! linux下需要屏蔽的一些信号
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
	//! 启动日志服务
	zsummer::log4z::ILog4zManager::GetInstance()->Config("client.cfg");
	zsummer::log4z::ILog4zManager::GetInstance()->Start();
	
	zsummer::utility::SleepMillisecond(500);
	g_fillString.resize(1000, 'z');
	cout <<"please entry ip" <<endl;
	std::string ip;
	cin >> ip;
	unsigned short port =0;
	cout << "please entry port" << endl;
	cin >> port;
	cout << "please entry create link count" << endl;
	unsigned short count = 0;
	cin >> count;
	cout << "please entry ping-pong" << endl;
	cin >> g_ping_pong;
	if (port == 0 || count <=0)
	{
		LOGF("user entry is error. ip=" << ip << ", port=" << port << ", count=" << count);
		return 0;
	}
	else
	{
		LOGI("user entry is ip=" << ip << ", port=" << port << ", count=" << count);
	}

	g_pios = NULL;
	CZSummer summer;
	summer.Run(ip, port, count);
	


	return 0;
}

