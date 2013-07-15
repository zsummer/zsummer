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
* Copyright (C) 2012-2013 YaweiZhang <yawei_zhang@foxmail.com>.
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

#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN	


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "../../tools/protocol4z/protocol4z.h"
#include "../../utility/utility.h"
using namespace std;
using namespace boost;

#define _BUF_LEN	(8*1024)


static int g_recvmsgs = 0;
static char g_senddata[_BUF_LEN];

static int g_usetime1[10]; // 0~9ms
static int g_usetime2[10]; // (0~10)*10ms

#define SEND_INTERVAL 5000


class CClient
{
public:
	CClient(asio::io_service & ios, std::string ip, unsigned short port): 
	  m_ios(ios),
	  m_ip(ip),
	  m_port(port),
	  m_socket(m_ios),
	  m_time(m_ios)
	{
		memset(m_recvbuf, 0 , sizeof(m_recvbuf));
		m_type = 0;
		m_time.expires_from_now(boost::posix_time::milliseconds(1000+rand()%1000));
		m_time.async_wait(boost::bind(&CClient::TimeOut, this, _1));
	}
	bool Connect(const char * ip, unsigned short port)
	{
		m_socket.async_connect(asio::ip::tcp::endpoint(asio::ip::address_v4::from_string(ip), port), boost::bind(&CClient::connect_handler, this, _1));
		return true;
	}
	void connect_handler(const boost::system::error_code& error)
	{
		if (error)
		{
			cout << "connect error: " << boost::system::system_error(error).what() << endl;
			m_type = 0;
			m_time.expires_from_now(boost::posix_time::milliseconds(2000));
			m_time.async_wait(boost::bind(&CClient::TimeOut, this, _1));
			return;
		}
		else
		{
			m_type = 2;
			m_time.expires_from_now(boost::posix_time::milliseconds(2000));
			m_time.async_wait(boost::bind(&CClient::TimeOut, this, _1));
			Recv();
		}
	}
	bool Recv()
	{
		if (m_type == 2)
		{
			m_socket.async_receive(asio::buffer(m_recvbuf, 2), boost::bind(&CClient::OnRecv, this, _1));
		}
		else if (m_type == 3)
		{
			m_socket.async_receive(asio::buffer(m_recvbuf+2, *(unsigned short *)(m_recvbuf) - 2), boost::bind(&CClient::OnRecv, this, _1));
		}
		
		return true;	
	}
	void OnRecv(const boost::system::error_code& error)
	{
		if (error)
		{
			cout <<"recv error: "  << boost::system::system_error(error).what() << endl;
			m_socket.close();
			return ;
		}
		if (m_type == 2)
		{
			if (*(unsigned short *)(m_recvbuf) > _BUF_LEN)
			{
				cout <<"recv error, type=2: recv error header len:" <<*(unsigned short *)(m_recvbuf) << endl;
				m_socket.close();
				return;
			}
			else
			{
				m_type = 3;
				Recv();
			}
		}
		else if (m_type == 3)
		{
			zsummer::protocol4z::ReadStream rs(m_recvbuf, *(unsigned short *)(m_recvbuf));
			unsigned short protocolID = 0;
			unsigned short requestID = 0;
			unsigned int sendtime =0;
			unsigned long long counter = 0;
			std::string text;
			try
			{
				rs >> protocolID >> requestID >> counter>> sendtime >> text;
				unsigned int now = zsummer::utility::GetTimeMillisecond();
				now = now - sendtime;

				if (now >= 100)
				{
					cout << " recv warning: protocolID=" << protocolID << ", requestID=" << requestID << ", counter=" << counter << "used time=" << now << ", text=" << text << endl;
				}
				else
				{
					if (now < 10)
					{
						g_usetime1[now] ++;
					}
					else
					{
						g_usetime2[now/10] ++;
					}
					
					
				}
				//cout << " recv: protocolID=" << protocolID << ", requestID=" << requestID << ", counter=" << counter << "used time=" << now << ", text=" << text << endl;
			}
			catch (std::runtime_error e)
			{
				cout <<"protocol error: " << e.what() << endl;
				m_socket.close();
				return;
			}
			m_type =2;
			Recv();
		}

		g_recvmsgs++;
		
	}
	bool Send()
	{
		unsigned short protocolID = 1;
		unsigned short requestID = 32;
		unsigned long long counter = 123;
		unsigned int sendtime = zsummer::utility::GetTimeMillisecond();
		std::string text = "ffffffffffffffffffffffff";
		zsummer::protocol4z::WriteStream ws(g_senddata, _BUF_LEN);
		ws << protocolID << requestID << counter << sendtime << text;
		m_socket.async_send(asio::buffer(g_senddata, *(unsigned short *)(g_senddata) ), boost::bind(&CClient::OnSend, this, _1));
		return true;
	}
	void OnSend(const boost::system::error_code& error)
	{
		if (error)
		{
			m_socket.close();
			cout <<"send error: "  << boost::system::system_error(error).what() << endl;
			return ;
		}
		m_time.expires_from_now(boost::posix_time::milliseconds(SEND_INTERVAL+rand()%600));
		m_time.async_wait(boost::bind(&CClient::TimeOut, this, _1));
	}

	void TimeOut(const boost::system::error_code& error )
	{
		if (error)
		{
			cout <<"timmer error: "  << boost::system::system_error(error).what() << endl;
			return ;
		}
		if (m_type == 0)
		{
			m_type = 1;
			Connect(m_ip.c_str(), m_port);
		}
		else if (m_type > 1)
		{
			Send();
		}
	}
private:
	asio::io_service & m_ios;
	std::string m_ip;
	unsigned short m_port;
	asio::ip::tcp::socket m_socket;
	asio::deadline_timer m_time;
	int m_type;// 0 init, 1 connect, 2 recved header, 3 recved body.
	char m_recvbuf[_BUF_LEN];
};


static asio::deadline_timer * g_ptimer = NULL;
void Moniter(const boost::system::error_code& error )
{
	if (error)
	{
		cout <<"moniter error: "  << boost::system::system_error(error).what() << endl;
		return ;
	}
	cout << " MONITER:  recvs msg=" << g_recvmsgs << ", use time[";
	for (int i=0; i<10; i++)
	{
		if (g_usetime1[i] > 0)
		{
			cout <<i << "ms:"<<g_usetime1[i] << "  ";
		}
	}
	for (int i=0; i<10; i++)
	{
		if (g_usetime2[i] > 0)
		{
			cout <<i*10 << "ms:"<<g_usetime2[i] << "  ";
		}
	}
	cout << "] " << endl;

	g_ptimer->expires_from_now(boost::posix_time::milliseconds(5000));
	g_ptimer->async_wait(&Moniter);
}

int main(int argc, char* argv[])
{



	asio::io_service ios;
	asio::io_service::work w(ios);
	asio::deadline_timer timer(ios);
	g_ptimer = &timer;
	timer.expires_from_now(boost::posix_time::milliseconds(1000));
	timer.async_wait(&Moniter);

	srand((unsigned int)time(NULL));

	std::string ip;
	unsigned short port = 0;
	int maxn = 0;
	cout << "zsummer's server ip:" << endl;
	cin >> ip;
	cout << "zsummer's server port:" << endl;
	cin >> port;
	cout << "stress client num:" << endl;
	cin >> maxn;
	cout <<" zsummer's server info: [" << ip << ":" << port << "]  stress clients:" << maxn << endl;
	if (!cin)
	{
		return -1;
	}
	memset(g_usetime1, 0, sizeof(g_usetime1));
	memset(g_usetime2, 0, sizeof(g_usetime2));
	for (int i=0; i<maxn; i++)
	{
		CClient * p = new CClient(ios, ip, port);
	}


	
	ios.run();
	
	return 0;
}

