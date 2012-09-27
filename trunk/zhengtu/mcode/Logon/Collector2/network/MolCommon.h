#ifndef _MOL_COMMON_H_INCLUDE
#define _MOL_COMMON_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:网络引擎用到的一些共用的部分
* 作者:akinggw
* 日期:2010.2.11
*/
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <process.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <climits>

#pragma warning(disable:4251)
#pragma warning(disable:4290)

typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;

#define THREAD_RESERVE 10
#define MOL_REV_BUFFER_SIZE 20480
#define MOL_REV_BUFFER_SIZE_TWO 10240
#define MOL_STR_BUFFER_SIZE 5000
#define MOL_CONN_POOL_MAX 10                        // 数据库连接池中最大连接
#define MOL_SEND_FILE 9000                          // 用于文件传输

#define MOL_HOST_TO_NET_16(value) (htons (value))
#define MOL_HOST_TO_NET_32(value) (htonl (value))

#define MOL_NET_TO_HOST_16(value) (ntohs (value))
#define MOL_NET_TO_HOST_32(value) (ntohl (value))
//
//#ifdef MOLE2D_NETWORK_DLL
//#define MOLNETEXPORT  __declspec(dllexport)
//#else
//#define MOLNETEXPORT
//#endif

#endif
