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
模块:		对象池
相关文件:	-
依赖模块:	-
作者:		张亚伟
版本:		
创建日期:	2012.4.23
最后更新日期:	2012.9.8
其他说明: 
*/

/*   更新日志
**********************************
* -
**********************************
*/







#ifndef _ZSUMMER_OBJECTPOOL_H_
#define _ZSUMMER_OBJECTPOOL_H_
#include <list>
namespace zsummer
{
	//对象池

/*
i3-350M, 5gDRRIII, VS2005 release 1000000 禁止优化,
	CObject1 size: 204800
	CObject2 size: 50
	new  CObject1 : 422 ms,   per : 2369000 n/s
	pool CObject1 : 46 ms,   per : 21739000 n/s
	new  CObject2 : 125 ms,   per : 8000000 n/s
	pool CObject2 : 47 ms,   per : 21276000 n/s

i3-350M, 5gDRRIII, g++4.4.4  1000000
	CObject1 size: 204800
	CObject2 size: 50
	new  CObject1 : 108 ms,   per : 9259000 n/s 
	pool CObject1 : 45 ms,   per : 22222000 n/s 
	new  CObject2 : 62 ms,   per : 16129000 n/s 
	pool CObject2 : 46 ms,   per : 21739000 n/s
*/
	template<class T>
	class CObjectPool
	{
	public:
		CObjectPool()
		{
			m_pHead = new char[8+sizeof(T)];
			m_pTail = new char[8+sizeof(T)];
			*((unsigned long long *)m_pHead) = (unsigned long long)m_pTail;
			*((unsigned long long *)m_pTail) = 0;
			
			m_nUseSize = 0;
			m_nFreSize = 0;
			m_nTotSize = 0;
		}
		virtual ~CObjectPool(){}
		void AddToPool(void *p = NULL)
		{
			if (p == NULL)
			{
				p = new char[8+sizeof(T)];
				m_nTotSize++;
			}

			*(unsigned long long *)p = 0;
			*(unsigned long long *)m_pTail = (unsigned long long)p;
			m_pTail = p;
			m_nFreSize++;
			
		}
		bool Empty()
		{
			return  m_nUseSize == 0;
		}
		size_t Size()
		{
			return m_nUseSize;
		}

		T * CreateObject()
		{
			if ( m_nFreSize == 0)
			{
				AddToPool();
			}
			char *p = (char *)m_pHead;
			m_pHead = (void *)*((unsigned long long *)m_pHead);
			m_nUseSize++;
			m_nFreSize--;
			return new (p+8) T;
		}

		template <class tr1>
		T * CreateObject(tr1 t1)
		{
			if ( m_nFreSize == 0)
			{
				AddToPool();
			}
			char *p = (char *)m_pHead;
			m_pHead = (void *)*((unsigned long long *)m_pHead);
			m_nUseSize++;
			m_nFreSize--;
			return new (p+8) T(t1);
		}
		template <class tr1, class tr2>
		T * CreateObject(tr1 t1, tr2 t2)
		{
			if ( m_nFreSize == 0)
			{
				AddToPool();
			}
			char *p = (char *)m_pHead;
			m_pHead = (void *)*((unsigned long long *)m_pHead);
			m_nUseSize++;
			m_nFreSize--;
			return new (p+8) T(t1, t2);
		}
		template <class tr1, class tr2, class tr3>
		T * CreateObject(tr1 t1, tr2 t2, tr3 t3)
		{
			if ( m_nFreSize == 0)
			{
				AddToPool();
			}
			char *p = (char *)m_pHead;
			m_pHead = (void *)*((unsigned long long *)m_pHead);
			m_nUseSize++;
			m_nFreSize--;
			return new (p+8) T(t1, t2, t3);
		}
		template <class tr1, class tr2, class tr3, class tr4>
		T * CreateObject(tr1 t1, tr2 t2, tr3 t3, tr4 t4)
		{
			if ( m_nFreSize == 0)
			{
				AddToPool();
			}
			char *p = (char *)m_pHead;
			m_pHead = (void *)*((unsigned long long *)m_pHead);
			m_nUseSize++;
			m_nFreSize--;
			return new (p+8) T(t1, t2, t3, t4);
		}
		template <class tr1, class tr2, class tr3, class tr4, class tr5>
		T * CreateObject(tr1 t1, tr2 t2, tr3 t3, tr4 t4, tr5 t5)
		{
			if ( m_nFreSize == 0)
			{
				AddToPool();
			}
			char *p = (char *)m_pHead;
			m_pHead = (void *)*((unsigned long long *)m_pHead);
			m_nUseSize++;
			m_nFreSize--;
			return new (p+8) T(t1, t2, t3, t4, t5);
		}
		void DealObject(T *p)
		{
			p->~T();
			m_nUseSize--;
			AddToPool((char *)p-8);
		}
	protected:
	public:
		void *	m_pHead;
		void * m_pTail;
		size_t m_nUseSize;
		size_t m_nFreSize;
		size_t m_nTotSize;
	};






	//--------------弃用 ------------------------------
	//对象池

/*
i3-350, 5gDRRIII, VS2005 release 1000000 禁止优化,
	CObject1 size: 204800
	CObject2 size: 50
	new  CObject1 : 374 ms,   per : 2673000 n/s
	pool CObject1 : 452 ms,   per : 2212000 n/s
	new  CObject2 : 109 ms,   per : 9174000 n/s
	pool CObject2 : 468 ms,   per : 2136000 n/s

i3-350, 5gDRRIII, g++4.4.4  1000000
	CObject1 size: 204800
	CObject2 size: 50
	new  CObject1 : 99 ms,   per : 10101000 n/s 
	pool CObject1 : 238 ms,   per : 4201000 n/s 
	new  CObject2 : 62 ms,   per : 16129000 n/s 
	pool CObject2 : 228 ms,   per : 4385000 n/s 
*/
	template<class T>
	class CObjectPool2
	{
	public:
		CObjectPool2(){}
		virtual ~CObjectPool2(){}
		T * CreateObject()
		{		
			char *p = NULL;
			if (!m_objs.empty())
			{
				p = m_objs.front();
				m_objs.pop_front();
			}
			else
			{
				p = new char[sizeof(T)];
			}

			return new (p) T;
		}

		template <class tr1>
		T * CreateObject(tr1 t1)
		{		
			char *p = NULL;
			if (!m_objs.empty())
			{
				p = m_objs.front();
				m_objs.pop_front();
			}
			else
			{
				p = new char[sizeof(T)];
			}
			return new (p) T(t1);
		}

		template <class tr1, class tr2>
		T * CreateObject(tr1 t1, tr2 t2)
		{		
			char *p = NULL;
			if (!m_objs.empty())
			{
				p = m_objs.front();
				m_objs.pop_front();
			}
			else
			{
				p = new char[sizeof(T)];
			}
			return new (p) T(t1, t2);
		}


		template <class tr1, class tr2, class tr3>
		T * CreateObject(tr1 t1, tr2 t2, tr3 t3)
		{		
			char *p = NULL;
			if (!m_objs.empty())
			{
				p = m_objs.front();
				m_objs.pop_front();
			}
			else
			{
				p = new char[sizeof(T)];
			}
			return new (p) T(t1, t2, t3);
		}


		template <class tr1, class tr2, class tr3, class tr4>
		T * CreateObject(tr1 t1, tr2 t2, tr3 t3, tr4 t4)
		{		
			char *p = NULL;
			if (!m_objs.empty())
			{
				p = m_objs.front();
				m_objs.pop_front();
			}
			else
			{
				p = new char[sizeof(T)];
			}
			return new (p) T(t1, t2, t3, t4);
		}

		template <class tr1, class tr2, class tr3, class tr4, class tr5>
		T * CreateObject(tr1 t1, tr2 t2, tr3 t3, tr4 t4, tr5 t5)
		{		
			char *p = NULL;
			if (!m_objs.empty())
			{
				p = m_objs.front();
				m_objs.pop_front();
			}
			else
			{
				p = new char[sizeof(T)];
			}
			return new (p) T(t1, t2, t3, t4, t5);
		}

		void DealObject(T *p)
		{
			p->~T();
			m_objs.push_back((char *)p);
		}
	protected:
	private:
		std::list<char *> m_objs;
	};











}


#endif



