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


//! zsummer的测试服务模块(对应zsummer底层网络封装的上层设计测试服务) 可视为服务端架构中的 gateway服务/agent服务/前端服务, 特点是高并发高吞吐量
//! IOServer头文件 该类负责处理Schedule抛过来的client socket. 每个IOServer独立一个线程.

#ifndef ZSUMMER_PROCESS_H_
#define ZSUMMER_PROCESS_H_

#include "header.h"

class CProcess : public CThread, public IIOServerCallback
{
public:
	CProcess();
	//! 启动与停止
	bool Start();
	void Stop();
	//! 线程
	virtual void Run();
	//! 投递线程间消息 当前设计仅供Schedule投递client socket用
	virtual void Post(void * pUser);
	//!IOServer被安全停止
	virtual bool OnStop();
	//！收到线程消息 
	virtual bool OnPost(void *pUser);
	virtual bool OnTimer();

	//! 一些状态统计用接口
	inline unsigned long long GetTotalRecvLen() {return m_nTotalRecvLen;}
	inline void AddTotalRecvLen(unsigned long long len) { m_nTotalRecvLen += len;}

	inline unsigned long long GetTotalSendLen() {return m_nTotalSendLen;}
	inline void AddTotalSendLen(unsigned long long len) { m_nTotalSendLen += len;}

	inline unsigned long long GetTotalRecvCount() {return m_nTotalRecvCount;}
	inline void AddTotalRecvCount(unsigned long long len) { m_nTotalRecvCount += len;}

	inline unsigned long long GetTotalSendCount() {return m_nTotalSendCount;}
	inline void AddTotalSendCount(unsigned long long len) { m_nTotalSendCount += len;}
	
private:
	IIOServer * m_ios;
	bool	m_bRunning;
	unsigned long long  m_nTotalRecvLen;
	unsigned long long  m_nTotalSendLen;
	unsigned long long  m_nTotalRecvCount;
	unsigned long long  m_nTotalSendCount;
};

#endif