

#include "../thread.h"
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>

using namespace std;

//! 测试次数
#define MAX_NUM (10*1000*1000)

//! 测试线程锁
class CTestLock: public zsummer::thread4z::CThread
{
public:
	static  int ms_ask;
	static  int ms_data[MAX_NUM];
	static zsummer::thread4z::CLock ms_lock;
	virtual void Run()
	{
		while (1)
		{
			int index =0;
			ms_lock.Lock();
			index = ms_ask++;
			ms_lock.UnLock();
			if (index >= MAX_NUM)
			{
				break;
			}
			ms_data[index] = index;
		}
		printf("test lock thread:%d exit\n", (int)GetThread());
	}
};
int CTestLock::ms_ask = 0;
int CTestLock::ms_data[MAX_NUM]={0};
zsummer::thread4z::CLock CTestLock::ms_lock;


//! 测试原子操作
class CTestAtom: public zsummer::thread4z::CThread
{
public:
	static  int ms_ask;
	static  int ms_data[MAX_NUM];
	virtual void Run()
	{
		while (1)
		{
			int index = zsummer::thread4z::AtomicAdd(&ms_ask, 1);
			if (index >= MAX_NUM)
			{
				break;
			}
			ms_data[index] = index;
		}
		printf("test atom thread:%x exit\n", (int)GetThread());
	}
};
int CTestAtom::ms_ask = 0;
int CTestAtom::ms_data[MAX_NUM] = {0};

//! 测试信号量
class CTestSem: public zsummer::thread4z::CThread
{
public:
	static int ms_ask;
	static zsummer::thread4z::CSem ms_sem;
	static void Init()
	{
		ms_sem.Create(1, NULL);
		printf("1000~0 seq print:\n");
	}

	virtual void Run()
	{
		while (1)
		{
			ms_sem.Wait();
			if (ms_ask >= 0)
			{
				printf("%d,", ms_ask);
				ms_ask--;
			}
			ms_sem.Post();
			if (ms_ask < 0)
			{
				break;
			}
		}
		printf("\ntest CSem thread:%x exit\n", (int)GetThread());
	}
};
int CTestSem::ms_ask = 1000;
zsummer::thread4z::CSem CTestSem::ms_sem;

//! 测试具名信号量
class CTestNamedSem: public zsummer::thread4z::CThread
{
public:
	static int ms_ask;
	zsummer::thread4z::CSem ms_sem;
	void CreateNamed(const char * name)
	{
		printf("1000~0 seq print:\n");
		ms_sem.Create(1, name);
	}
	void OpenNamed(const char * name)
	{
		ms_sem.Open(name);
	}

	virtual void Run()
	{

		while (1)
		{
			ms_sem.Wait();
			if (ms_ask >= 0)
			{
				printf("%d,", ms_ask);
				ms_ask--;
			}
			ms_sem.Post();
			if (ms_ask < 0)
			{
				break;
			}
		}
		printf("\ntest named CSem thread:%x exit\n", (int)GetThread());
	}
};
int CTestNamedSem::ms_ask = 1000;




int main(int argc, char* argv[])
{
	//test lock
	{
		CTestLock test1;
		CTestLock test2;
		CTestLock test3;
		test1.Start();
		test2.Start();
		test3.Start();
		test1.Wait();
		test2.Wait();
		test3.Wait();

		bool checkvalues = true;
		for (int i=0; i<MAX_NUM; i++)
		{
			if (CTestLock::ms_data[i] != i)
			{
				checkvalues = false;
				printf("lock test error!  ms_data[%d] is %d .\n", i, CTestLock::ms_data[i]);
				break;
			}
		}
		if (checkvalues)
		{
			printf("lock test success!\n");
		}
	}

	//atom test
	{
		CTestAtom test1;
		CTestAtom test2;
		CTestAtom test3;
		test1.Start();
		test2.Start();
		test3.Start();
		test1.Wait();
		test2.Wait();
		test3.Wait();

		bool checkvalues = true;
		for (int i=0; i<MAX_NUM; i++)
		{
			if (CTestAtom::ms_data[i] != i)
			{
				checkvalues = false;
				printf("atom test error!  ms_data[%d] is %d .\n", i, CTestAtom::ms_data[i]);
				break;
			}
		}
		if (checkvalues)
		{
			printf("atom test success!\n");
		}
	}


	//CSem test
	{
		CTestSem test1;
		CTestSem test2;
		CTestSem test3;
		CTestSem::Init();
		test1.Start();
		test2.Start();
		test3.Start();
		test1.Wait();
		test2.Wait();
		test3.Wait();
	}
	//CSem Named test
	{
		CTestNamedSem test1;
		CTestNamedSem test2;
		test1.CreateNamed("testsem");
		test2.OpenNamed("testsem");
		test1.Start();
		test2.Start();
		test1.Wait();
		test2.Wait();
	}

	cout <<"press any key to exit ..." << endl;
	getchar();
	return 0;
}

