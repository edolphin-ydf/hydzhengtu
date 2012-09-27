#ifndef _DEFINE_H
#define _DEFINE_H

#include <windows.h>
#include <math.h>
#include <stdio.h>

#define MAX_BUFF_9    9
#define MAX_BUFF_20   20
#define MAX_BUFF_100  100
#define MAX_BUFF_200  200
#define MAX_BUFF_500  500
#define MAX_BUFF_1000 1000
#define MAX_BUFF_1024 1024

#define MAX_PACKET_SIZE     1024*1024

#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef uint64
typedef unsigned long long uint64;
#endif

#ifndef int8
typedef char int8;
#endif

#ifndef int16
typedef short int16;
#endif

#ifndef int32
typedef int int32;
#endif

#ifndef float32
typedef float float32;
#endif

#ifndef float64
typedef double float64;
#endif

#ifndef VCHARS_STR
typedef  struct _VCHARS_STR {
	const char *text;
	uint8 u1Len;
}VCHARS_STR;
#endif

#ifndef VCHARM_STR
typedef  struct _VCHARM_STR {
	const char *text;
	uint16 u2Len;
}VCHARM_STR;
#endif

#ifndef VCHARB_STR
typedef  struct _VCHARB_STR {
	const char *text;
	uint32 u4Len;
}VCHARB_STR;
#endif


///////////////////////////////定义一些函数///////////////////////////////////////
//定义一个函数，可以支持内存越界检查
inline void sprintf_safe(char* szText, int nLen, const char* fmt ...)
{
	if(szText == NULL)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	int result = vsnprintf(szText, nLen, fmt, ap);

	if (result == -1)
	{
		result = static_cast <int> (nLen + 1);
	}

	va_end(ap);
};
#endif