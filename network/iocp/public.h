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
#pragma once

#ifndef _ZSUMMER_PUBLIC_H_
#define _ZSUMMER_PUBLIC_H_
#define _WIN32_WINNT 0x501

#include "../../utility/utility.h"
#include "../../tools/thread4z/thread.h"
#include "../../tools/log4z/log4z.h"
#include "../../network/SocketInterface.h"
#include <assert.h>
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <queue>
#include <Mswsock.h>
#include <WinSock2.h>
#include <Windows.h>

namespace zsummer
{
	struct tagReqHandle 
	{
		OVERLAPPED	 _overlapped;
		unsigned char _type;
		enum
		{
			HANDLE_ACCEPT, 
			HANDLE_RECV, 
			HANDLE_SEND,
			HANDLE_CONNECT, 
		};
	};

	enum LINK_STATUS
	{
		LS_NORMAL,
		LS_ESTABLISHED,
		LS_WAITCLOSE,
		LS_WAITCLEAR,
	};
	enum POST_COM_KEY
	{
		PCK_IOCP_EXIT = 1,
		PCK_ACCEPT_CLOSE,
		PCK_SOCKET_CLOSE,
		PCK_USER_DATA,
	};

	class CInitWSASocketEnv
	{
	public:
		CInitWSASocketEnv();
		~CInitWSASocketEnv();
	};
}
extern LoggerId g_coreID;
extern zsummer::CInitWSASocketEnv appInitSocket;


#define LCD( log ) LOG_DEBUG( g_coreID, log )
#define LCI( log ) LOG_INFO( g_coreID, log )
#define LCW( log ) LOG_WARN( g_coreID, log )
#define LCE( log ) LOG_ERROR( g_coreID, log )
#define LCA( log ) LOG_ALARM( g_coreID, log )
#define LCF( log ) LOG_FATAL( g_coreID, log )




















#endif











