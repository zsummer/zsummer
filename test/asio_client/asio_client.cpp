// asio_client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
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


class CClient
{
public:
	CClient(asio::io_service & ios): m_ios(ios), m_socket(m_ios), m_time(m_ios)
	{
		memset(m_recvbuf, 0 , sizeof(m_recvbuf));

		m_type = 0;

		//m_time.expires_from_now(boost::posix_time::milliseconds(1000));
		m_time.expires_from_now(boost::posix_time::milliseconds(2000+rand()%90000));
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
			//cout <<" connect ok" << endl;
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

 		//cout << " recv data: " << m_recvbuf+2 << endl;

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
		m_time.expires_from_now(boost::posix_time::milliseconds(5000+rand()%1000));
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
			//Connect("192.168.138.58", 81);
			//Connect("192.168.138.146", 81);
			//Connect("192.168.204.128", 81);
			Connect("127.0.0.1", 81);
		}
		else
		{
			Send();
		}
		
	}
private:
	asio::io_service & m_ios;
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

int _tmain(int argc, _TCHAR* argv[])
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

	srand(time(NULL));

	cout << " enter a num for clients" << endl;
	int maxn =0;
	cin >> maxn;
	cout <<" will create " << maxn << " clts" << endl;
	for (int i=0; i<maxn; i++)
	{
		CClient * p = new CClient(ios);
	}

	
	ios.run();
	
	return 0;
}

