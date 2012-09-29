
#ifndef _UTIL_H
#define _UTIL_H

#include "Common.h"

///////////////////////////////////////////////////////////////////////////////
// String Functions ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> StrSplit(const std::string & src, const std::string & sep);

// This HAS to be called outside the threads __try / __except block!
void SetThreadName(const char* format, ...);
time_t convTimePeriod(uint32 dLength, char dType);

inline uint32 secsToTimeBitFields(time_t secs)
{
	tm* lt = localtime(&secs);
	return (lt->tm_year - 100) << 24 | lt->tm_mon  << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min;
}


extern SERVER_DECL const char* _StringToUTF8(const char* pASCIIBuf);
extern SERVER_DECL const char* _StringToANSI(const char* pUtf8Buf);
extern SERVER_DECL bool _IsStringUTF8(const char* str);

volatile long Sync_Add(volatile long* value);

volatile long Sync_Sub(volatile long* value);

#ifdef WIN32

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

#endif

////////////////////////////////////////////////////////////////
/// @namespace MNet
/// @brief M网络库
namespace MNet
{
	SERVER_DECL float round(float f);
	SERVER_DECL double round(double d);
	SERVER_DECL long double round(long double ld);

	/////////////////////////////////////////////////////////////////////////
	//void Sleep( unsigned long timems );
	//  Puts the calling thread to sleep for the specified miliseconds
	//
	//Parameter(s)
	//  unsigned long timemes  -  time interval to put the thread to sleep for
	//
	//Return Value
	//  None
	//
	//
	/////////////////////////////////////////////////////////////////////////
	void Sleep(unsigned long timems);
}


/////////////////////////////////////////////////////////
//uint32 getMSTime()
//  Returns the time elapsed in milliseconds
//
//Parameter(s)
//  None
//
//Return Value
//  Returns the time elapsed in milliseconds
//
//
/////////////////////////////////////////////////////////
MNET_INLINE uint32 getMSTime()//返回以毫秒为单位时间间隔
{
	uint32 MSTime = 0;
#ifdef WIN32
	MSTime = GetTickCount();
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	MSTime = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
	return MSTime;
}

#endif
