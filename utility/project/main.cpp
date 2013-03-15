
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


void TestUtilityColorText()
{
	{
		PUTN(zsummer::utility::GetVersion());
		PUTR(zsummer::utility::GetVersionLog());
		PUTG(zsummer::utility::GetUtilityVersion());
		PUTB(zsummer::utility::GetUtilityVersionLog());
		PUTY("YELLOW");
		PUTV("VIOLET");
	}
}

void TestUtilityTime()
{
	{
		unsigned int uCurMillisecond = zsummer::utility::GetTimeMillisecond();
		unsigned long long uCurMicrosecond = zsummer::utility::GetTimeMicrosecond();
		PUTN("sleep 1251ms\n");
		zsummer::utility::SleepMillisecond(1251);
		uCurMillisecond = zsummer::utility::GetTimeMillisecond() - uCurMillisecond;
		uCurMicrosecond = zsummer::utility::GetTimeMicrosecond() - uCurMicrosecond;
		PUTN("now sleep times : "<< uCurMillisecond <<" ms " <<(unsigned long) uCurMicrosecond << " us ");
	}

	{
		time_t now = time(NULL);
		PUTN( "time_t now =" << now );
		PUTN( "string time = "<< zsummer::utility::TimeToString(now) );
	}

	{
		struct tm tm1;
		struct tm tm2;
		memset(&tm1, 0, sizeof(tm1));
		tm1.tm_year = 2012-1900;
		tm1.tm_mon = 12-1;
		tm1.tm_mday = 31; //2012.12.31. monday1,
		tm1.tm_hour = 16;
		memcpy(&tm2, &tm1, sizeof(tm1));
		tm2.tm_hour=23;
		if (zsummer::utility::isTimeSameDay(zsummer::utility::TmToTime(&tm1), zsummer::utility::TmToTime(&tm2)))
		{
			PUTB("isTimeSameDay OK");
		}
		else
		{
			PUTR("isTimeSameDay ERR");
		}
	}
}

void TestUtilityXml()
{
	std::string str = "<!--   first -->\n"
		"<sre><!-- haha--> 324234               </sre>";
	int n = 0;
	if (zsummer::utility::GetXmlParam(str, "sre", n) && n == 324234)
	{
		PUTB("GetXmlParam OK");
	}
	else
	{
		PUTV("GetXmlParam ERROR");
	}
}

void TestUtilityOther()
{
	{
		PUTN( zsummer::utility::GetInstancePath() );
		PUTN( zsummer::utility::GetInstanceName() );
	}

	{
		char buf[100] = {"splite the  string by blank space, not ignore muti segmentation."};
		PUTN( buf );
		std::vector<std::string> v;
		zsummer::utility::SplitString(buf, " ", v);
		for (unsigned int i=0; i<v.size(); i++)
		{
			PUTN( v[i]);
		}
		PUTN("\n");

		char buff[100] = {"splite$$the string$$by$$blank space$$$$$ ignore muti segmentation."};
		PUTB( buff );
		zsummer::utility::SplitString(buff, "$$", v, true);
		for (unsigned int i=0; i<v.size(); i++)
		{
			PUTB(v[i]);
		}
		PUTB( "\n");
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
	unsigned long old;
	unsigned long now;

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


	TestUtilityColorText();
	TestUtilityTime();
	TestUtilityXml();
	TestUtilityOther();

	TestObjectPool();






	return 0;
}

