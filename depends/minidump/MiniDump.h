#pragma once

#ifndef _SUMMER_MINIDUMP_H_
#define _SUMMER_MINIDUMP_H_

#ifdef WIN32

namespace zsummer
{
	class CMiniDump
	{
	public:
		CMiniDump();
		static CMiniDump * GetInstance();
	};
}
#else
namespace zsummer
{
	class CMiniDump
	{
	public:
		CMiniDump(){}
		static CMiniDump * GetInstance(){return 0;}
	};
}
#endif


#endif
