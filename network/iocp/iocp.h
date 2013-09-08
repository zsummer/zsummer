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



#ifndef _ZSUMMER_IOCP_H_
#define _ZSUMMER_IOCP_H_


#include "public.h"
using namespace zsummer::network;
using namespace zsummer::thread4z;
using namespace zsummer::utility;
using namespace zsummer::network;

namespace zsummer
{
	//! 消息泵, message loop.
	class CIOServer: public IIOServer
	{
	public:
		CIOServer();
		virtual ~CIOServer();
		virtual bool Initialize(IIOServerCallback *cb);
		virtual void RunOnce();
		virtual void Post(void *pUser);
		virtual unsigned long long CreateTimer(unsigned int delayms, ITimerCallback * cb);
		virtual bool CancelTimer(unsigned long long timerID);
	public:
		void PostMsg(POST_COM_KEY pck, ULONG_PTR ptr); 
		void CheckTimer();
	public:
		//! IOCP句柄
		HANDLE m_io;
		//! IO处理器回调指针
		IIOServerCallback * m_cb;

		//! 定时器
		std::map<unsigned long long, ITimerCallback*> m_queTimer;
		unsigned int m_queSeq; //! 用于生成定时器ID
		volatile unsigned long long m_nextExpire; //! 最快触发时间
		zsummer::thread4z::CLock m_lockTimer; //! 锁
		
		
	};

}





















#endif











