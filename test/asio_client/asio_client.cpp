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

using namespace std;
using namespace boost;

#define _BUF_LEN	(5*1024)
#define _SEND_LEN   (1024)

static int g_recvmsgs = 0;
static char g_senddata[_BUF_LEN];

#define SEND_INTERVAL 10


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
		m_time.expires_from_now(boost::posix_time::milliseconds(1000+rand()%10000));
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
			cout << "ERR: " << boost::system::system_error(error).what() << endl;
			return;
		}
		else
		{
			Send();
			Recv();
		}
	}
	bool Recv()
	{
		m_socket.async_receive(asio::buffer(m_recvbuf, _SEND_LEN), boost::bind(&CClient::OnRecv, this, _1));
		return true;	
	}
	void OnRecv(const boost::system::error_code& error)
	{
		if (error)
		{
			cout <<"ERR: "  << boost::system::system_error(error).what() << endl;
			m_socket.close();
			return ;
		}
		if (memcmp(m_recvbuf, g_senddata, _SEND_LEN) != 0)
		{
			cout << "ERR: recv err:  datalen=" <<*((unsigned short*)m_recvbuf) << " data="  <<m_recvbuf+2 << endl;
			m_socket.close();
			return;
		}
		g_recvmsgs++;
		Recv();
	}
	bool Send()
	{
		m_socket.async_send(asio::buffer(g_senddata, _SEND_LEN), boost::bind(&CClient::OnSend, this, _1));
		return true;
	}
	void OnSend(const boost::system::error_code& error)
	{
		if (error)
		{
			m_socket.close();
			cout <<"ERR: "  << boost::system::system_error(error).what() << endl;
			return ;
		}
		m_time.expires_from_now(boost::posix_time::milliseconds(SEND_INTERVAL+rand()%600));
		m_time.async_wait(boost::bind(&CClient::TimeOut, this, _1));
	}

	void TimeOut(const boost::system::error_code& error )
	{
		if (error)
		{

			cout <<"ERR: "  << boost::system::system_error(error).what() << endl;
			return ;
		}
		if (m_type == 0)
		{
			m_type = 99;
			Connect(m_ip.c_str(), m_port);
		}
		else
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
	int m_type;
	char m_recvbuf[_BUF_LEN];
};


static asio::deadline_timer * g_ptimer = NULL;
void Moniter(const boost::system::error_code& error )
{
	if (error)
	{
		cout <<"ERR: "  << boost::system::system_error(error).what() << endl;
		return ;
	}
	cout << " MONITER:  recvs msg=" << g_recvmsgs << endl;

	g_ptimer->expires_from_now(boost::posix_time::milliseconds(1000));
	g_ptimer->async_wait(&Moniter);
}

int main(int argc, char* argv[])
{
	{
		memset(g_senddata, 0, sizeof(g_senddata));
		char tmpt[] = "001234567890abcdefghijklmnopqrstuvwxyz";

		int j = 0;
		for (int i=0; i<_BUF_LEN; i++)
		{
			g_senddata[i] = tmpt[j];
			j++;
			if (j == sizeof(tmpt)-1)
			{
				j = 0;
			}
		}
		unsigned short len = _SEND_LEN;
		memcpy(g_senddata, &len, 2);
	}


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
	cout << "press any key to continue.";
	if (!cin)
	{
		return -1;
	}
	getchar();
	getchar();
	for (int i=0; i<maxn; i++)
	{
		CClient * p = new CClient(ios, ip, port);
	}

	
	ios.run();
	
	return 0;
}

