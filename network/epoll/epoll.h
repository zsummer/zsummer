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

#ifndef _ZSUMMER_EPOLL_H_
#define _ZSUMMER_EPOLL_H_
#include "public.h"

/*
* epoll的 post使用socketpair做为挂靠在epoll_wait上的通知触发机制 采用EPOLLLT模式
*/

/* 此数据为3.0版本数据
*  virtualbox 虚拟机 8000个连接 每个连接平均1.5s发送一个200字节的数据 并对echo数据校验, 稳定.  40%CPU 其中us0.5%, %6hi %24si %4sy  大部分在软中断 怀疑虚拟网卡性能问题
*  virtualbox 虚拟机  28000个连接 每个连接平均10.5s发送一个200字节的数据 24%CPU占用 其中 us0.3%  17.2$si  大部分仍在软中断
*  宿主主机10%的CPU占用,  客户端6%的占用
*  vmware 虚拟机 8000个连接 每个连接平均1.5s发送一个200字节的数据 并对echo数据校验, 稳定.  8.0%CPU 其中us0.3%, %0.2hi %3.4si %0.7sy 
*  vmware 虚拟机 16000个连接 每个连接平均1.5s发送一个200字节的数据 并对echo数据校验, 稳定.  17.0%CPU 其中us0.5%, %0.2hi %7.1si %1.5sy 
*  vmware 虚拟机 23000个连接 每个连接平均1.5s发送一个200字节的数据 并对echo数据校验, 稳定.  27.0%CPU 其中us0.8%, %0.3hi %11.1si %2.1sy 
*
*/
namespace zsummer
{
	typedef std::vector<std::pair<int, void*> > MsgVct;
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
		void PostMsg(POST_COM_KEY pck, void * ptr); 
		void CheckTimer();
	public:
		int	m_epoll;
		IIOServerCallback	* m_cb;

		//! 网络消息
		epoll_event m_events[5000];

		//线程消息
		int		m_sockpair[2];
		tagRegister m_recv;
		MsgVct	m_msgs;
		CLock	m_msglock;

		//! 定时器
		std::map<unsigned long long, ITimerCallback*> m_queTimer;
		unsigned int m_queSeq; //! 用于生成定时器ID
		volatile unsigned long long m_nextExpire; //! 最快触发时间
		zsummer::thread4z::CLock m_lockTimer; //! 锁
	};

}





















#endif











