//////////////////////////////////////////////////
/// @file : MemoryPools.h
/// @brief : 
/// @date:  2012/10/9
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __MemoryPools_H__
#define __MemoryPools_H__

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "ThreadLock.h"

#ifdef _DEBUG
/** @brief 4个字节的指针块地址 + 4个字节的链表首地址 + 4位验证码 + 4个字节的map索引号 + 4个字节的行号 */
#define MAX_MEMORYHEAD_SIZE 20  
#else
/** @brief 4个字节的指针块地址 + 4个字节的链表首地址 + 4位验证码*/
#define MAX_MEMORYHEAD_SIZE 12    
#endif
/** @brief 验证码*/
#define MAGIC_CODE          0x123456

#define UINT32 unsigned int

////////////////////////////////////////////////////////////////
/// @struct _MemoryBlock
/// @brief  内存块的结构，双向链表
////////////////////////////////////////////////////////////////
struct _MemoryBlock
{
	_MemoryBlock *m_pNext; /**< 向前指针  */
	_MemoryBlock *m_pPrev; /**< 向后指针  */
	void*         m_pBrick;/**< 内存块指针  */

	void Init()
	{
		m_pNext   = NULL;
		m_pPrev   = NULL;
		m_pBrick  = NULL;
	};

	_MemoryBlock()
	{
		Init();
	};
};
////////////////////////////////////////////////////////////////
/// @struct _MemoryList
/// @brief  内存管理列表
////////////////////////////////////////////////////////////////
struct _MemoryList    
{
	_MemoryList*   m_pMemLNext;
	_MemoryBlock*  m_pMemoryFree;    /**< 自由的内存块  */
	_MemoryBlock*  m_pMemoryFreeLast;/**< 自由的内存块链表末尾  */
	_MemoryBlock*  m_pMemoryUsed;    /**< 使用的内存块  */
	_MemoryBlock*  m_pMemoryUsedLast;/**< 使用的内存块链表末尾  */

	int m_nSize;

	void Init()
	{
		m_pMemLNext       = NULL;
		m_pMemoryFree     = NULL;
		m_pMemoryUsed     = NULL;
		m_pMemoryUsedLast = NULL;
		m_pMemoryFreeLast = NULL;
		m_nSize           = 0;
	};

	_MemoryList()
	{
		Init();
	};
};

////////////////////////////////////////////////////////////////
/// @struct 
/// @brief
////////////////////////////////////////////////////////////////
struct _MemoryInfo
{
	int nUsedCount;
	int nUsedSize;
	int nFreeCount;
	int nFreeSize;
};

////////////////////////////////////////////////////////////////
/// @class CMemoryPools
/// @brief 内存池类
///
/// @note  公开的方法只有三个，对应着new，delete的重载
class CMemoryPools
{
public:
	static CMemoryPools& Instance()
	{
		if(m_pMemoryPools == NULL)
		{
			m_pMemoryPools = new CMemoryPools();
			//m_pMemoryPools->Init();
		}

		return *m_pMemoryPools;
	}
	static void release()
	{
		if(m_pMemoryPools)
		{
			//m_pMemoryPools->~CMemoryPools();
			//free(m_pMemoryPools);
			delete m_pMemoryPools;
			m_pMemoryPools = NULL;
		}
	}

public:
	~CMemoryPools(void);

	void* GetBuff(size_t szBuffSize,const char *file = NULL, int line = 0);
	bool DelBuff(size_t szBuffSize, void* pBuff); /**< 基本上不用这个接口  */
	bool DelBuff(void* pBuff);
	/** @brief 对应着打印出内存池此时此刻正在使用的内存链表个数，\n
	每个链表中有多少内存块在使用，有多少是自由内存，用于内存泄露的跟踪
	*/
	_MemoryInfo GetAndPlayMemoryList();
#ifdef _DEBUG
	/** @brief 打印map中保存的文件地址 */
	void DisPlayMap();
#endif
	template< class T >	
#ifdef _DEBUG
	T * Alloc(const char *file, int line)
	{
		unsigned long lSize = sizeof(T);
		void* ptMem = GetBuff(lSize, file, line);
#else
	T * Alloc()
	{
		unsigned long lSize = sizeof(T);
		void* ptMem = GetBuff(lSize);
#endif
		if( !ptMem) return NULL;
		return (T*)ptMem;
	}
private:
	CMemoryPools(void);
	void Close();
	void Init();
	void* SetMemoryHead(void* pBuff, _MemoryList* pList, _MemoryBlock* pBlock,int aindex = -1,int line = 0);
	void* GetMemoryHead(void* pBuff);
	bool GetHeadMemoryBlock(void* pBuff, _MemoryList*& pList, _MemoryBlock*& pBlock,int& map_inex);
	

private:
	static CMemoryPools* m_pMemoryPools;
	_MemoryList*         m_pMemoryList;
	_MemoryList*         m_pMemoryListLast;    /**< 最后一个内存管理链表指针  */
	CThreadLock          m_ThreadLock;

#ifdef _DEBUG
	typedef std::map<unsigned int,std::string> MAP_FILE;
	MAP_FILE map_file;
#endif
};

#define MemPools CMemoryPools::Instance()
#define Macro_New(szBuffSize) \
	do {\
	MemPools.GetBuff(szBuffSize,__FILE__,__LINE__);\
	}while (0)

#define Macro_Delete(pBuff) \
	do {\
	if(MemPools.DelBuff(pBuff))\
		pBuff=NULL;\
	}while (0)

#define Macro_NewClass(pObject, ClassName) \
do {\
	pObject = (ClassName*)MemPools.Alloc<ClassName>(__FILE__,__LINE__);\
	pObject->ClassName::ClassName();\
}while (0)

#define Macro_DeleteClass(pObject, ClassName) \
do {\
	if(pObject!=NULL)\
{\
	void *ptr = pObject;\
	pObject->ClassName::~ClassName();\
	MemPools.DelBuff(ptr);\
}\
}while (0)

#define Macro_SaveDeleteClass(pObject, ClassName) \
	do {\
	if(pObject!=NULL)\
{\
	void *ptr = pObject;\
	pObject->ClassName::~ClassName();\
	if(MemPools.DelBuff(ptr))\
	pObject=NULL;\
}\
	}while (0)
#endif