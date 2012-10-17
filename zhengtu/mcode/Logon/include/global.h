//////////////////////////////////////////////////
/// @file : global.h
/// @brief : 
/// @date:  2012/10/11
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __global_H__
#define __global_H__

/** @brief 加入VLD内存泄漏检查 */
#ifdef _DEBUG
#define WIN32_LEAN_AND_MEAN//避免包含冲突
#include "vld/vld.h"
#pragma comment(lib,"vld")
#endif
/** @brief VS原生检查内存泄漏 */
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