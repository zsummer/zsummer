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

#ifndef ZSUMMER_HEADER_H_
#define ZSUMMER_HEADER_H_

#include "../../utility/utility.h"
#include "../../tools/thread4z/thread.h"
#include "../../tools/log4z/log4z.h"
#include "../../network/SocketInterface.h"

#include <iostream>
#include <queue>
#include <iomanip>
#include <string.h>
#include <signal.h>
using namespace std;

using namespace zsummer::utility;
using namespace zsummer::thread4z;
using namespace zsummer::network;

#define _MSG_BUF_LEN	(5*1024)
#define _MSG_LEN   (5000)
struct tagPacket
{
	unsigned short _head;
	char		   _body[_MSG_BUF_LEN];
};

//服务端状态信息
extern int g_nTotalLinked;
extern int g_nTotalCloesed;
extern int g_nTotalRecvLen;
extern int g_nTotalSendLen;


#endif

