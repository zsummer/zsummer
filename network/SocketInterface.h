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


/*
 * AUTHORS:  YaweiZhang <yawei_zhang@foxmail.com>
 * VERSION:  1.0.0
 * PURPOSE:  A lightweight C++ library for network.
 * CREATION: 2010.8.29
 * LCHANGE:  -
 * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
 */


/*
 *
 * QQ Group: 19811947
 * Web Site: www.zsummer.net
 * mail: yawei_zhang@foxmail.com
 */


/*
 * UPDATES
 *
 "* 2010.8.29  iocp test\n"
 "* 2010.9.24	实现windows下临界区的封装\n"
 "* 2010.9.26	实现windows下多线程的封装\n"
 "* 2010.9.29	解决iocp socket close 资源释放\n"
 "* 2010.10.4  抽象出IOCP高级封装接口\n"
 "* 2010.12.7  重构IOCP的设计封装, 服务器趋于稳定\n"
 "* -------------- *\n"

 "* 2011.3.15  iocp / epoll project build\n"
 "* 2011.3.19	实现跨平台的互斥锁, 信号量, 线程包装类的封装,  跨平台的时间获取 休眠 转换小函数\n"
 "* 2011.3.22	增加跨平台的网络封装接口雏形, CLog 封装, createdir函数, splitstring, selfmemqueue等\n"
 "* 2011.3.23	修正代码风格, 去掉linux c的全小写风格\n"
 "* 2011.4.1	epoll实现经过功能测试, iocp移植到新的网络接口下, CLog采用环形队列\n"
 "* 2011.4.9   功能测试完成\n"
 "* -------------- *\n"

 "* 2012.4.21 新的封装, 全新编写\n"
 "* 2012.5.7 增加跨平台的系统进程监控记录模块\n"
 "* 2012.5.8  增加minidump模块\n"
 "* 2012.5.28 IOCP实现完成\n"
 "* 2012.6.6 EPOLL实现完成\n"
 "* 2012.6.7 代码整理, 测试服务编写\n"
 "* 2012.6.8 asio测试客户端编写完成\n"
 "* 2012.6.9 调测\n"
 "* 2012.7.18 多种环境下的压力测试完成\n"
 "* 2012.12.16 日志类由单日志输出改为多日志输出 并可灵活配置 屏显上色等\n"
 "*--------------- *\n"
 *
 */


#ifndef _ZSUMMER_SOCKETINTERFACE_H_
#define _ZSUMMER_SOCKETINTERFACE_H_

namespace zsummer
{
	namespace network
	{
		class ITcpAccept;
		class ITcpSocket;

		class IIOServerCallback
		{
		public:
			virtual bool OnStop() = 0;
			virtual bool OnMsg(void *pUser) = 0;
		};
		class IIOServer
		{
		public:
			IIOServer(){}
			virtual ~IIOServer() {}
			virtual bool Start(IIOServerCallback *cb) = 0;
			virtual void Run() = 0;
			virtual bool Stop() = 0;
			virtual void Post(void *pUser) = 0;
		};


		class ITcpSocketCallback
		{
		public:
			virtual bool OnConnect(bool bConnected) = 0;
			virtual bool OnRecv() = 0;
			virtual bool OnSend() = 0;
			virtual bool OnClose() = 0;
		};

		class ITcpSocket
		{
		public:
			ITcpSocket(){}
			virtual ~ITcpSocket(){}
			virtual bool BindIOServer(IIOServer * ios) = 0;
			virtual bool SetCallback(ITcpSocketCallback * cb) = 0;
			virtual bool DoConnect(const char *ip, unsigned short port) = 0;
			virtual bool DoSend(char * buf, unsigned int len) = 0;
			virtual bool DoRecv(char * buf, unsigned int len) =0;
			virtual bool GetPeerInfo(unsigned int * addr, unsigned short *port)=0;
			virtual bool Close() = 0;
		};

		class ITcpAcceptCallback
		{
		public:
			virtual bool OnAccept(ITcpSocket * s) = 0;
			virtual bool OnClose() = 0;
		};

		class ITcpAccept
		{
		public:
			ITcpAccept(){}
			virtual ~ITcpAccept(){}
			virtual bool BindIOServer(IIOServer * ios) = 0;
			virtual bool SetCallbck(ITcpAcceptCallback * cb) = 0;
			virtual bool OpenAccept(const char * ip, unsigned short port) = 0;
		};


		IIOServer * CreateIOServer();
		void DestroyIOServer(IIOServer * ios);

		ITcpSocket * CreateTcpSocket();
		void DestroyTcpSocket(ITcpSocket * s);

		ITcpAccept * CreateTcpAccept();
		void DestroyTcpAccept(ITcpAccept *s);
	};
};























#endif











