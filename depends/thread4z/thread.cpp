
/*
 * Thread4z License
 * -----------
 * 
 * Thread4z is licensed under the terms of the MIT license reproduced below.
 * This means that Thread4z is free software and can be used for both academic
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


#include "thread.h"

#include <time.h>
#include <string.h>

#ifndef WIN32
#include <string.h>
#include <fcntl.h>
#else
#include <process.h>
#endif

#ifndef zs_th_s
#define zs_th_s zsummer::thread4z
#endif


zs_th_s::CThread::CThread()
{
	m_hThread = 0;
}
zs_th_s::CThread::~CThread()
{

}
#ifdef WIN32

static unsigned int WINAPI  ThreadProc(LPVOID lpParam)
{
	zs_th_s::CThread * p = (zs_th_s::CThread *) lpParam;
	p->Run();
	_endthreadex(0);
	return 0;
}
#else

static void * ThreadProc(void * pParam)
{
	zs_th_s::CThread * p = (zs_th_s::CThread *) pParam;
	p->Run();
	return NULL;
}
#endif


bool zs_th_s::CThread::Start()
{
#ifdef WIN32
	unsigned long long ret = _beginthreadex(NULL, 0, ThreadProc, (void *) this, 0, NULL);

	if (ret == -1 || ret == 1  || ret == 0)
	{
		return false;
	}
	m_hThread = ret;
#else
	pthread_t ptid = 0;
	int ret = pthread_create(&ptid, NULL, ThreadProc, (void*)this);
	if (ret != 0)
	{
		return false;
	}
	m_hThread = ptid;

#endif
	return true;
}



bool zs_th_s::CThread::Wait()
{
#ifdef WIN32
	if (WaitForSingleObject((HANDLE)m_hThread, INFINITE) != WAIT_OBJECT_0)
	{
		return false;
	}
#else
	if (pthread_join((pthread_t)m_hThread, NULL) != 0)
	{
		return false;
	}
#endif
	return true;
}

bool zs_th_s::CThread::Terminate()
{
#ifdef WIN32
	return TerminateThread((HANDLE)m_hThread, 1) ? true :false;
#else
	return pthread_cancel((pthread_t)m_hThread) == 0 ? true : false;
#endif
}



zs_th_s::CSem::CSem()
{
#ifndef WIN32
	memset(m_name, 0, sizeof(m_name));
#endif
}

zs_th_s::CSem::~CSem()
{
#ifdef WIN32
	if (m_hSem != NULL)
	{
		CloseHandle(m_hSem);
		m_hSem = NULL;
	}
#else
	if (strlen(m_name) == 0)
	{
		sem_destroy(&m_semid);
	}
	else
	{
		sem_close(m_psid);
		sem_unlink(m_name);
		memset(m_name, 0, sizeof(m_name));
	}
#endif
}

bool zs_th_s::CSem::Create(int initcount, const char *  name)
{
	if (initcount < 0)
	{
		initcount = 0;
	}
#ifdef WIN32
	if (initcount > 64)
	{
		return false;
	}
	m_hSem = CreateSemaphoreA(NULL, initcount, 64, name);
	if (m_hSem == NULL)
	{
		return false;
	}
#else
	if (name == NULL || strlen(name) == 0)
	{
		if (sem_init(&m_semid, 0, initcount) != 0)
		{
			return false;
		}
	}
	else
	{
		if (strlen(name) > 256)
		{
			return false;
		}
		strcpy(m_name, name);
		m_psid = sem_open(name, O_CREAT, 0644, initcount);
		if (m_psid == SEM_FAILED)
		{
			return false;
		}
	}
#endif
	return true;
}


bool zs_th_s::CSem::Open(const char * name)
{
	if ( name == NULL)
	{
		return false;
	}
	if (strlen(name) == 0)
	{
		return false;
	}
#ifdef WIN32
	m_hSem = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, name);
	if (m_hSem == NULL)
	{
		return false;
	}
#else
	if (strlen(name) > 256)
	{
		return false;
	}
	strcpy(m_name, name);
	m_psid = sem_open(name, O_EXCL);
	if (m_psid == SEM_FAILED)
	{
		return false;
	}
#endif

	return true;
}
bool zs_th_s::CSem::Wait(int timeout)
{
#ifdef WIN32
	if (timeout <= 0)
	{
		timeout = INFINITE;
	}
	if (WaitForSingleObject(m_hSem, timeout) != WAIT_OBJECT_0)
	{
		return false;
	}
#else

	if (strlen(m_name) == 0)
	{
		if (timeout <= 0)
		{
			return (sem_wait(&m_semid) == 0);
		}
		else
		{
			timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += timeout/1000;
			ts.tv_nsec += (timeout%1000)*1000000;
			return (sem_timedwait(&m_semid, &ts) == 0);
		}
	}
	else
	{
		if (timeout <= 0)
		{
			return (sem_wait(m_psid) == 0);
		}
		else
		{
			timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += timeout/1000;
			ts.tv_nsec += (timeout%1000)*1000000;
			return (sem_timedwait(m_psid, &ts) == 0);
		}
	}
#endif
	return true;
}
bool zs_th_s::CSem::Post()
{
#ifdef WIN32
	return ReleaseSemaphore(m_hSem, 1, NULL) ? true : false;
#else
	if (strlen(m_name) == 0)
	{
		return (sem_post(&m_semid) == 0);
	}
	else
	{
		return (sem_post(m_psid) == 0);
	}
#endif
}

zs_th_s::CLock::CLock()
{
#ifdef WIN32
	InitializeCriticalSection(&m_crit);
#else
	//m_crit = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_crit, &attr);
	pthread_mutexattr_destroy(&attr);
#endif
}


zs_th_s::CLock::~CLock()
{
#ifdef WIN32
	DeleteCriticalSection(&m_crit);
#else
	pthread_mutex_destroy(&m_crit);
#endif
}


void zs_th_s::CLock::Lock()
{
#ifdef WIN32
	EnterCriticalSection(&m_crit);
#else
	pthread_mutex_lock(&m_crit);
#endif
}


void zs_th_s::CLock::UnLock()
{
#ifdef WIN32
	LeaveCriticalSection(&m_crit);
#else
	pthread_mutex_unlock(&m_crit);
#endif
}

//------------------------------------------------------------------
int zs_th_s::AtomicAdd(volatile int * pt, int t)
{
#ifdef WIN32
	return (int)InterlockedExchangeAdd((long *) pt, (long)t);
#else
	//	return __gnu_cxx::__exchange_and_add(pt, t);
	return __sync_add_and_fetch(pt, t);
	//return *++pt;
#endif
}


int zs_th_s::AtomicInc(volatile int * pt)
{
#ifdef WIN32
	return (int)InterlockedIncrement((long *)pt);
#else
	//return __gnu_cxx::__exchange_and_add(pt, 1);
	return __sync_add_and_fetch(pt, 1);
#endif
}
int zs_th_s::AtomicDec(volatile int * pt)
{
#ifdef WIN32
	return (int)InterlockedDecrement((long*)pt);
#else
	//return __gnu_cxx::__exchange_and_add(pt, -1);
	return __sync_add_and_fetch(pt, -1);
#endif
}


