
#include "../utility.h"

#include "../cirque.h"
#include "../objectpool.h"

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include <string>
#include <vector>
#include <time.h>

using namespace std;

#define TEMPMACRO(x)\
	{std::stringstream os;os << x;std::cout << os.str() << endl;}

#define PUTN(x) TEMPMACRO(x)
#define PUTB(x) TEMPMACRO(x)
#define PUTR(x) TEMPMACRO(x)



void TestUtilityTime()
{
	{
		PUTN("sleep 1251ms");
		unsigned long long uBeginMillisecond = zsummer::utility::GetTimeMillisecond();
		unsigned long long uBeginMicrosecond = zsummer::utility::GetTimeMicrosecond();
		unsigned long long  uBeginTickMillisecond = zsummer::utility::GetTickMillisecond();
		unsigned long long  uBeginTickMicrosecond = zsummer::utility::GetTickMicrosecond();
		zsummer::utility::SleepMillisecond(1251);
		unsigned long long uEndMillisecond = zsummer::utility::GetTimeMillisecond();
		unsigned long long uEndMicrosecond = zsummer::utility::GetTimeMicrosecond();
		unsigned long long  uEndTickMillisecond = zsummer::utility::GetTickMillisecond();
		unsigned long long  uEndTickMicrosecond = zsummer::utility::GetTickMicrosecond();
		PUTN("need sleep times is 1251, now sleep="<< uEndMillisecond - uBeginMillisecond <<"ms, " <<uEndMicrosecond - uBeginMicrosecond << "us.");
		PUTN("need sleep ticks is 1251, now sleep="<< uEndTickMillisecond - uBeginTickMillisecond <<"ms, " <<uEndTickMicrosecond - uBeginTickMicrosecond << "us.");

		PUTN("uBeginMillisecond=" <<uBeginMillisecond);
		PUTN("uEndMillisecond=" <<uEndMillisecond);
		PUTN("uBeginMicrosecond=" <<uBeginMicrosecond);
		PUTN("uEndMicrosecond=" <<uEndMicrosecond);
		PUTN("uBeginTickMillisecond=" <<uBeginTickMillisecond);
		PUTN("uEndTickMillisecond=" <<uEndTickMillisecond);
		PUTN("uBeginTickMicrosecond=" <<uBeginTickMicrosecond);
		PUTN("uEndTickMicrosecond=" <<uEndTickMicrosecond);


		unsigned long long  now = zsummer::utility::GetTickMillisecond();
		for (int i=0; i<1000000; i++)
		{
			uEndTickMicrosecond = zsummer::utility::GetTickMillisecond();
		}
		PUTN("100Íò GetTickMillisecond use time=" << zsummer::utility::GetTickMillisecond() - now << "ms");
		now = zsummer::utility::GetTimeMillisecond();
		for (int i=0; i<1000000; i++)
		{
			uEndTickMicrosecond = zsummer::utility::GetTimeMillisecond();
		}
		PUTN("100Íò GetTimeMillisecond use time=" << zsummer::utility::GetTimeMillisecond() - now<< "ms");
	}

	{
		time_t now = time(NULL);
		PUTN( "time_t now =" << now );
		PUTN( "string time = "<< zsummer::utility::TimeToString(now) );
	}
}


class CObject1
{
public:
	CObject1()
	{
	}
	char t[200*1024];
};

class CObject2
{
public:
	CObject2()
	{
	}
	char t[50];
};

void TestObjectPool()
{
	zsummer::CObjectPool<CObject1> obj1;
	zsummer::CObjectPool<CObject2> obj2;
	PUTN(" CObject1 size: " << sizeof(CObject1));
	PUTN(" CObject2 size: " << sizeof(CObject2));

	unsigned long max=10*10000;
	unsigned long long old;
	unsigned long long now;

	old = zsummer::utility::GetTimeMillisecond();
	for (unsigned int i=0; i<max; i++)
	{
		CObject1 *p = new CObject1;
		delete p;
	}
	now = zsummer::utility::GetTimeMillisecond();
	PUTN("new  CObject1 200K : " << now - old << " ms,   per : " << (int)(max*1.0/(now-old)*1000)  << " n/s " );


	old = zsummer::utility::GetTimeMillisecond();
	for (unsigned int i=0; i<max; i++)
	{
		CObject1 * p = obj1.CreateObject();
		obj1.DealObject(p);
	}
	now = zsummer::utility::GetTimeMillisecond();
	PUTN("pool CObject1 200K : " << now - old << " ms,   per : " << (int)(max*1.0/(now-old)*1000)  << " n/s " );

	old = zsummer::utility::GetTimeMillisecond();
	for (unsigned int i=0; i<max; i++)
	{
		CObject2 *p = new CObject2;
		delete p;
	}
	now = zsummer::utility::GetTimeMillisecond();
	PUTN("new  CObject2 : " << now - old << " ms,   per : " << (int)(max*1.0/(now-old)*1000)  << " n/s ");


	old = zsummer::utility::GetTimeMillisecond();
	for (unsigned int i=0; i<max; i++)
	{
		CObject2 * p = obj2.CreateObject();
		obj2.DealObject(p);
	}
	now = zsummer::utility::GetTimeMillisecond();
	PUTN("pool CObject2 : " << now - old << " ms,   per : " << (int)(max*1.0/(now-old)*1000)  << " n/s ");


}
int main(int argc, char* argv[])
{
	TestUtilityTime();
	TestObjectPool();
	return 0;
}

