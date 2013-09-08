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
 * VERSION:  4.0.0
 * PURPOSE:  A lightweight C++ library for network.
 * CREATION: 2010.9.6
 * LCHANGE:  -
 * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
 */


#pragma once
#ifndef _ZSUMMER_UTILITY_H_
#define _ZSUMMER_UTILITY_H_

#ifndef _ZSUMMER_BEGIN
#define _ZSUMMER_BEGIN namespace zsummer {
#endif
#ifndef _ZSUMMER_UTILITY_BEGIN
#define _ZSUMMER_UTILITY_BEGIN namespace utility {
#endif

#include <string>
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <iostream>
#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif
_ZSUMMER_BEGIN
_ZSUMMER_UTILITY_BEGIN



inline void SleepMillisecond(unsigned int ms)
{
#ifdef WIN32
	Sleep(ms);
#else
	usleep(1000*ms);
#endif
}



inline unsigned long long GetTimeMicrosecond()
{
#ifdef WIN32
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	unsigned long long now = ft.dwHighDateTime;
	now <<= 32;
	now |= ft.dwLowDateTime;
	now /=10;
	now -=11644473600000000Ui64;
	return now;
#else
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return ((unsigned long long) tm.tv_sec * 1000 *1000 + (tm.tv_usec));
#endif
}

inline unsigned long long GetTimeMillisecond()
{
	return GetTimeMicrosecond()/1000;
}

inline unsigned long long GetTickMillisecond()
{
#ifdef WIN32
	return GetTickCount();
#else
	timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
	{
		return ts.tv_sec*1000 + ts.tv_nsec/1000/1000;
	}
	assert(0);
	return 0;
#endif
}

inline unsigned long long GetTickMicrosecond()
{
#ifdef WIN32
	LARGE_INTEGER lip;
	LARGE_INTEGER lic;
	QueryPerformanceFrequency(&lip);
	QueryPerformanceCounter(&lic);
	return (unsigned long long)(lic.QuadPart*1000/(lip.QuadPart/1000));
#else
	timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
	{
		return ts.tv_sec*1000*1000 + ts.tv_nsec/1000;
	}
	assert(0);
	return 0;
#endif
}

inline bool TimeToTm(time_t t, tm * tt)
{
#ifdef WIN32
	if (localtime_s(tt, &t) == 0)
	{
		return true;
	}
	return false;
#else
	if (localtime_r(&t, tt) != NULL)
	{
		return true;
	}
	return false;
#endif
}

inline time_t TmToTime(tm * tt)
{
	return mktime(tt);
}

inline std::string TimeToString(time_t t)
{
	char m[128] = {0};
	tm tt;
	if (TimeToTm(t, &tt))
		{
		sprintf(m, "%d-%02d-%02d %02d:%02d:%02d", tt.tm_year+1900, tt.tm_mon+1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec);
	}
	else
	{
		sprintf(m, "0000-00-00 00:00:00");
	}
	return m;
}



//Ëæ»ú: [begin-end)
inline int Rand(int begin, int end)
{
	return begin + rand()%(end - begin);
}
//[0-end)
inline int Rand(int end)
{
	return Rand(0, end);
}






#ifndef _ZSUMMER_END
#define _ZSUMMER_END }
#endif
#ifndef _ZSUMMER_UTILITY_END
#define _ZSUMMER_UTILITY_END }
#endif

_ZSUMMER_UTILITY_END
_ZSUMMER_END

#ifdef WIN32
#pragma warning(pop)
#endif
#endif
