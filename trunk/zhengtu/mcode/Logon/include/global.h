//////////////////////////////////////////////////
/// @file : global.h
/// @brief : 
/// @date:  2012/10/11
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __global_H__
#define __global_H__

enum EServerType{
	EServerType_Login,
	EServerType_Cache,
	EServerType_Gate,
	EServerType_World,
	EServerType_Count,
	EServerType_None,
};

static string ServerName[EServerType_Count] =
{
	"��¼������",               // 0
	"���������",                // 1
	"���ط�����",			// 2
	"���������",		// 3
};

#define SAVE_DELETE(x) if(x){ delete x;x=NULL; }

/** @brief ����VLD�ڴ�й©��� */
#ifdef _DEBUG
#define WIN32_LEAN_AND_MEAN//���������ͻ
#include "vld/vld.h"
#pragma comment(lib,"vld")
#endif
/** @brief VSԭ������ڴ�й© */
//#ifdef _DEBUG
//#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
//#else
//#define DEBUG_CLIENTBLOCK
//#endif
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#ifdef _DEBUG
//#define new DEBUG_CLIENTBLOCK
//#endif

#endif