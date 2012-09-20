#include "staticFun.h"

#define _W32_FT_OFFSET (116444736000000000)
int gettimeofday(struct timeval *tp,void *tzp)
{
  union{
    __int64  ns100; /*time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  }_now;

  GetSystemTimeAsFileTime(&_now.ft);
  tp->tv_usec = (long)((_now.ns100 / 10) % 1000000);
  tp->tv_sec  = (long)((_now.ns100 - _W32_FT_OFFSET) / 10000000);
  /* Always return 0 as per Open Group Base Specifications Issue 6.
     Do not set errno on error.  */
  return 0;
}

void sockStartup(void)
{
	WSADATA WSAData;
	WSAStartup(0x0202,&WSAData);
}

void sockCleanup(void)
{
	WSACleanup();
}