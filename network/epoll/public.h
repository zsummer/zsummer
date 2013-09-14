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
#ifndef _ZSUMMER_PUBLIC_H_
#define _ZSUMMER_PUBLIC_H_

#include "../../utility/utility.h"
#include "../../depends/thread4z/thread.h"
#include "../../depends/log4z/log4z.h"
#include "../SocketInterface.h"
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <string>
#include <iostream>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <utility>

namespace zsummer
{
	struct tagRegister
	{
		epoll_event   _event;//event
		unsigned char _type;//register type
		int			  _fd; //file descriptor
		void *		  _ptr; //user pointer
		enum REGISTER_TYPE
		{
			REG_ACCEPT, // listen
			REG_ESTABLISHED, //socket write & read
			REG_RECVFROM,
			REG_CONNECT, // connect
			REG_THREAD, // user message router
			REG_INVALIDE,
		};
	};


	enum POST_COM_KEY
	{
		PCK_ACCEPT_CLOSE = 1, // user thread message type
		PCK_SOCKET_CLOSE,
		PCK_USER_DATA,
	};

	class CApp
	{
	public:
		CApp();
	};

	inline bool SetNonBlock(int fd) 
	{
		return fcntl((fd), F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK) == 0;
	}
	inline bool SetNoDelay(int fd)
	{
		int bTrue = true?1:0;
		return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&bTrue, sizeof(bTrue)) == 0;
	}
	inline bool EPOLLMod(int epfd, int fd,  struct epoll_event *event)
	{
		if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event) != 0)
		{
			epoll_ctl(epfd, EPOLL_CTL_DEL, fd, event);
			return false;
		}
		return true;
	}
}

extern LoggerId g_coreID;
extern zsummer::CApp app;

#define LCD( log ) LOG_DEBUG( g_coreID, log )
#define LCI( log ) LOG_INFO( g_coreID, log )
#define LCW( log ) LOG_WARN( g_coreID, log )
#define LCE( log ) LOG_ERROR( g_coreID, log )
#define LCA( log ) LOG_ALARM( g_coreID, log )
#define LCF( log ) LOG_FATAL( g_coreID, log )

using namespace zsummer::network;
using namespace zsummer::thread4z;
using namespace zsummer::utility;











#endif











