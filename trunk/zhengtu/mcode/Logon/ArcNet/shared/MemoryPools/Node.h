//////////////////////////////////////////////////
/// @file : Node.h
/// @brief : 所有对象的基类，申请的对象都在内存池中
/// @date:  2012/10/15
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __Node_H__
#define __Node_H__
#include <xiosbase>
#include "MemoryPools.h"

class CNode
{
public:
	explicit CNode()
	{

	}
public:
#ifdef _DEBUG
	inline void* operator new(size_t szBuff,const char *file, int line)
	{
		void* pBuff = CMemoryPools::Instance().GetBuff(szBuff,file,line);
		//OUR_DEBUG((LM_ERROR, "[New] Size = %d Address = [0x%08x].!\n", (int)szBuff, pBuff));
		return pBuff;
	}
	inline void* operator new[](size_t szBuff,const char *file, int line)
	{
		void* pBuff = CMemoryPools::Instance().GetBuff(szBuff,file,line);
		//OUR_DEBUG((LM_ERROR, "[New] Size = %d Address = [0x%08x].!\n", (int)szBuff, pBuff));
		return pBuff;
	}
#else
	inline void* operator new(size_t szBuff)
	{
		void* pBuff = CMemoryPools::Instance().GetBuff(szBuff);
		//OUR_DEBUG((LM_ERROR, "[New] Size = %d Address = [0x%08x].!\n", (int)szBuff, pBuff));
		return pBuff;
	}
	inline void* operator new[](size_t szBuff)
	{
		void* pBuff = CMemoryPools::Instance().GetBuff(szBuff);
		//OUR_DEBUG((LM_ERROR, "[New] Size = %d Address = [0x%08x].!\n", (int)szBuff, pBuff));
		return pBuff;
	}
#endif
	
	inline void operator delete(void* p)
	{
		if(false == CMemoryPools::Instance().DelBuff(p))
		{
			//	OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] false!\n", p));
			//CMemoryPools::Instance().DisplayMemoryList(p);
		}
		else
		{
			//OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] OK!\n", p));
		}
	}
	
	inline void operator  delete[]( void * p )
	{
		if(false == CMemoryPools::Instance().DelBuff(p))
		{
			//	OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] false!\n", p));
			//CMemoryPools::Instance().DisplayMemoryList(p);
		}
		else
		{
			//OUR_DEBUG((LM_ERROR, "[Delete]*p = [0x%08x] OK!\n", p));
		}
	}
};

#endif