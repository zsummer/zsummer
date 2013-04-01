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

#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#ifndef zs_ut_s
#define zs_ut_s zsummer::utility
#endif
#pragma warning(disable:4996)

//-------------------------------------------------------------------
void zs_ut_s::SleepMillisecond(unsigned int ms)
{
#ifdef WIN32
	Sleep(ms);
#else
	usleep(1000*ms);
#endif
}

unsigned int zs_ut_s::GetTimeMillisecond()
{
#ifdef WIN32
	return GetTickCount();
#else
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return (tm.tv_sec * 1000 + (tm.tv_usec/1000));
#endif
}


//获得一个时间值 精确到微秒
unsigned long long zs_ut_s::GetTimeMicrosecond()
{
#ifdef WIN32
	LARGE_INTEGER lip;
	LARGE_INTEGER lic;
	QueryPerformanceFrequency(&lip);
	QueryPerformanceCounter(&lic);
	return (unsigned long long)(lic.QuadPart*1000/(lip.QuadPart/1000));
#else
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return ((unsigned long long )tm.tv_sec) * 1000 * 1000 + (unsigned long long)tm.tv_usec;
#endif
}




//转换time_t tm
bool zs_ut_s::TimeToTm(time_t t, tm * tt)
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

// 转换错误返回-1
time_t zs_ut_s::TmToTime(tm * tt)
{
	return mktime(tt);
}

std::string zs_ut_s::TimeToString(time_t t)
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

//[范围)
int zs_ut_s::Rand(int begin, int end)
{
	return begin + rand()%(end - begin);
}
//[0-*)
int zs_ut_s::Rand(int end)
{
	return Rand(0, end);
}
