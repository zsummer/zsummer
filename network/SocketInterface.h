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


/*
 * AUTHORS:  YaweiZhang <yawei_zhang@foxmail.com>
 * VERSION:  4.2.0
 * PURPOSE:  A lightweight C++ library for network.
 * CREATION: 2010.9.14
 * LCHANGE:  -
 * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
 */


/*
 * Web Site: www.zsummer.net
 * mail: yawei_zhang@foxmail.com
 */


/*
 * UPDATES
 *
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
		class ITimerCallback
		{
		public:
			ITimerCallback(){}
			virtual ~ITimerCallback(){}
			virtual void OnTimer(unsigned long long timerID) =0;
		};
		
		//! IO处理器回调
		class IIOServerCallback
		{
		public:
			IIOServerCallback(){}
			virtual ~IIOServerCallback(){}
			//! IIOServer.post(pUser) callback.
			virtual bool OnPost(void *pUser) = 0;
		};
		//! IO处理器
		class IIOServer
		{
		public:
			IIOServer(){}
			virtual ~IIOServer() {}
			//! some initialization for runing.
			virtual bool Initialize(IIOServerCallback *cb) = 0;
			//! user block call.
			virtual void RunOnce() = 0;
			//! provide Thread's message queue.
			virtual void Post(void *pUser) = 0;
			//! create timer
			virtual unsigned long long CreateTimer(unsigned int delayms, ITimerCallback * cb) = 0;
			virtual bool CancelTimer(unsigned long long timerID) = 0;
		};


		class ITcpSocketCallback
		{
		public:
			ITcpSocketCallback(){}
			virtual ~ITcpSocketCallback(){}
			virtual bool OnConnect(bool bConnected) = 0;
			virtual bool OnRecv(unsigned int nRecvedLen) = 0;
			virtual bool OnSend(unsigned int nSentLen) = 0;
			virtual bool OnClose() = 0;
		};

		class ITcpSocket
		{
		public:
			ITcpSocket(){}
			virtual ~ITcpSocket(){}
			virtual bool Initialize(IIOServer * ios, ITcpSocketCallback * cb) = 0;
			virtual bool DoConnect(const char *ip, unsigned short port) = 0;
			virtual bool DoSend(char * buf, unsigned int len) = 0;
			virtual bool DoRecv(char * buf, unsigned int len) =0;
			virtual bool GetPeerInfo(unsigned int * addr, unsigned short *port)=0;
			virtual bool Close() = 0;
		};

		class ITcpAcceptCallback
		{
		public:
			ITcpAcceptCallback(){}
			virtual ~ITcpAcceptCallback(){}
			virtual bool OnAccept(ITcpSocket * s) = 0;
			virtual bool OnClose() = 0;
		};

		class ITcpAccept
		{
		public:
			ITcpAccept(){}
			virtual ~ITcpAccept(){}
			virtual bool Initialize(IIOServer * ios, ITcpAcceptCallback * cb) = 0;
			virtual bool OpenAccept(const char * ip, unsigned short port) = 0;
		};

		class IUdpSocketCallback
		{
		public:
			IUdpSocketCallback(){}
			virtual ~IUdpSocketCallback(){}
			virtual bool OnRecvFrom(unsigned int nRecvlen, const char *ip, unsigned short port) = 0;
//			virtual bool OnSendTo(unsigned int nSendlen) = 0;
		};

		class IUdpSocket
		{
		public:
			IUdpSocket(){}
			virtual ~IUdpSocket(){}
			virtual bool Initialize(IIOServer * ios, IUdpSocketCallback * cb, const char *ip, unsigned short port = 0) = 0;
			virtual bool DoRecv(char * buf, unsigned int len) = 0;
			virtual bool DoSend(char * buf, unsigned int len, const char *dstip, unsigned short dstport) = 0;
		};

		IIOServer * CreateIOServer();
		void DestroyIOServer(IIOServer * ios);

		ITcpSocket * CreateTcpSocket();
		void DestroyTcpSocket(ITcpSocket * s);

		ITcpAccept * CreateTcpAccept();
		void DestroyTcpAccept(ITcpAccept *s);

		IUdpSocket * CreateUdpSocket();
		void DestroyUdpSocket(IUdpSocket * s);

	};
};























#endif











