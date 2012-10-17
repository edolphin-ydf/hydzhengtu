//////////////////////////////////////////////////
/// @file : define.h
/// @brief : 
/// @date:  2012/10/9
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __define_H__
#define __define_H__

// 引用任何所需的附加头文件，而不是在此文件中引用
//#include "MemoryPools.h"
//
////重载New和Delete操作符
//inline void* operator new(size_t szBuff)
//{
//	void* pBuff = CMemoryPools::Instance().GetBuff(szBuff);
//	//OUR_DEBUG((LM_ERROR, "[New] Size = %d Address = [0x%08x].!\n", (int)szBuff, pBuff));
//	return pBuff;
//}
//
//inline void operator delete(void* p)
//{
//	if(false == CMemoryPools::Instance().DelBuff(p))
//	{
//		//	OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] false!\n", p));
//		//CMemoryPools::Instance().DisplayMemoryList(p);
//	}
//	else
//	{
//		//OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] OK!\n", p));
//	}
//}
//
//inline void operator  delete[]( void * p )
//{
//	if(false == CMemoryPools::Instance().DelBuff(p))
//	{
//		//	OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] false!\n", p));
//		//CMemoryPools::Instance().DisplayMemoryList(p);
//	}
//	else
//	{
//		//OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] OK!\n", p));
//	}
//}
#endif