//////////////////////////////////////////////////
/// @file : global.h
/// @brief : 
/// @date:  2012/10/11
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __global_H__
#define __global_H__

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