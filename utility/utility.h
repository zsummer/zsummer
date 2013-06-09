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
 * CREATION: 2010.8.29
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

_ZSUMMER_BEGIN
_ZSUMMER_UTILITY_BEGIN



void SleepMillisecond(unsigned int ms);
unsigned int GetTimeMillisecond();
unsigned long long GetTimeMicrosecond();
bool TimeToTm(time_t t, tm * tt);
time_t TmToTime(tm * tt);
std::string TimeToString(time_t t);



//Ëæ»ú: [begin-end)
int Rand(int begin, int end);
//[0-end)
int Rand(int end);





#ifndef _ZSUMMER_END
#define _ZSUMMER_END }
#endif
#ifndef _ZSUMMER_UTILITY_END
#define _ZSUMMER_UTILITY_END }
#endif

_ZSUMMER_UTILITY_END
_ZSUMMER_END

#endif
