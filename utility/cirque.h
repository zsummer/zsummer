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
模块:		环形队列
相关文件:	cirque.cpp
依赖模块:	-
作者:		张亚伟
版本:		
创建日期:	2011.3.29
最后更新日期:	2012.9.8
其他说明: 环形队列的封装, 包括内存队列  对象队列
*/

/*   更新日志
**********************************
* -
**********************************
*/






#pragma once

#ifndef _ZSUMMER_CIRQUE_H_
#define _ZSUMMER_CIRQUE_H_
#include <string.h>
#include <vector>

namespace zsummer
{
	class CCircularQue
	{
	public:
		CCircularQue()
		{
			m_maxlen = 0;
			m_rdlen = 0;
			m_wtlen = 0;
		}
		virtual ~CCircularQue()
		{

		}
		//KB
		bool Init(unsigned int size)
		{
			m_vct.resize(size*1024, '\0');
			m_maxlen = size*1024;
			m_rdlen = 0;
			m_wtlen = 0;
			return true;
		}
		bool PushData(const char * buf, unsigned int len)
		{
			//可写
			if (m_wtlen+len - m_rdlen < m_maxlen)
			{
				unsigned int curw = m_wtlen%m_maxlen;
				//不用回环 不可写完
				if (curw + len < m_maxlen)
				{
					memcpy(&m_vct[curw], buf, len);
					m_wtlen += len;
				}
				//回环
				else
				{
					unsigned int l = m_maxlen - curw;
					memcpy(&m_vct[curw], buf, l);
					memcpy(&m_vct[0], &buf[l], len-l);
					m_wtlen += len;
				}
				return true;
			}
			return false;
		}
		bool PopData(char *buf, unsigned int & len)
		{
			if (m_rdlen == m_wtlen)
			{
				len = 0;
				return false;
			}

			if (m_wtlen - m_rdlen < len)
			{
				len = m_wtlen - m_rdlen;
			}

			unsigned int currd = m_rdlen%m_maxlen;
			//可以读完
			if (currd +len <= m_maxlen)
			{
				memcpy(buf, &m_vct[currd], len);
				m_rdlen += len;
			}
			else
			{
				unsigned int l = m_maxlen - currd;
				memcpy(buf, &m_vct[currd], l);
				memcpy(&buf[l], &m_vct[0], len - l);
				m_rdlen += len;
			}
			return true;
		}
		bool IsEmpty()
		{
			return (m_rdlen == m_wtlen);
		}
		//
		bool Clear()
		{
			m_rdlen = 0;
			m_wtlen = 0;
			return true;
		}
	private:
		std::vector<char>	m_vct;
		volatile unsigned int		m_rdlen;
		volatile unsigned int		m_wtlen;
		volatile unsigned int		m_maxlen;
	};

	template <class T>
	class CCirObjQue
	{
	public:
		CCirObjQue();
		virtual ~CCirObjQue();
		//KB
		bool Init(unsigned int size, T t);
		bool PushData(T t);
		bool  PopData(T & t);
		bool IsEmpty();
		bool Clear();
	private:
		std::vector<T>		m_vct;
		unsigned int		m_rdCount;
		unsigned int		m_wtCount;
		unsigned int		m_maxCount;
	};
}

template <class T>
zsummer::CCirObjQue<T>::CCirObjQue()
{
	m_rdCount = 0;
	m_wtCount = 0;
	m_maxCount= 0;
}

template <class T>
zsummer::CCirObjQue<T>::~CCirObjQue()
{

}


//KB
template <class T>
bool zsummer::CCirObjQue<T>::Init(unsigned int size, T t)
{
	m_vct.resize(size, t);
	m_maxCount = size;
	m_rdCount = 0;
	m_wtCount = 0;
	return true;
}

template <class T>
bool zsummer::CCirObjQue<T>::PushData(T t)
{
	//可写
	if (m_wtCount+1 - m_rdCount < m_maxCount)
	{
		unsigned int next = m_wtCount%m_maxCount;
		m_vct[next] = t;
		m_wtCount ++;
		return true;
	}
	return false;
}


template <class T>
bool  zsummer::CCirObjQue<T>::PopData(T & t)
{
	if (m_rdCount == m_wtCount)
	{
		return false;
	}

	unsigned int next = m_rdCount%m_maxCount;
	t = m_vct[next];
	m_rdCount++;
	return true;
}


template <class T>
inline bool zsummer::CCirObjQue<T>::IsEmpty()
{
	return (m_rdCount == m_wtCount);
}

//
template <class T>
bool zsummer::CCirObjQue<T>::Clear()
{
	m_rdCount = 0;
	m_wtCount = 0;
	return true;
}



#endif


