
#include "MiniDump.h"


#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <time.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp")
#pragma warning(disable:4996)
LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	std::string str;
	{
		char buf[260];
		memset(buf,0, sizeof(buf));
		GetModuleFileName(NULL, buf, 259);
		std::string s;
		s = buf;
		std::string::size_type pos = s.rfind("\\");
		if (pos != std::string::npos)
		{
			s.substr(pos, std::string::npos);
		}
		str = s;

		tm t;
		time_t tt=time(NULL);
		localtime_s(&t, &tt);
		sprintf(buf, "_%04d_%02d_%02d_%02d_%02d_%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		str += buf;
	}
	str += ".dmp";


	HANDLE lhDumpFile = CreateFile(str.c_str(), GENERIC_WRITE, 0, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
	CloseHandle(lhDumpFile);
	return EXCEPTION_EXECUTE_HANDLER;
}

zsummer::CMiniDump::CMiniDump()
{
	SetUnhandledExceptionFilter(&MyUnhandledExceptionFilter);
}
zsummer::CMiniDump * zsummer::CMiniDump::GetInstance()
{
	static zsummer::CMiniDump m;
	return &m;
}



#endif




