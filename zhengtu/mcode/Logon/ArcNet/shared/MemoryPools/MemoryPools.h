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
#include "ThreadLock.h"

/** @brief 4个字节的指针块地址 + 4个字节的链表首地址 + 4位验证码*/
#define MAX_MEMORYHEAD_SIZE 12    
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
			m_pMemoryPools = (CMemoryPools* )malloc(sizeof(CMemoryPools));
			m_pMemoryPools->Init();
		}

		return *m_pMemoryPools;
	}
	static void release()
	{
		if(m_pMemoryPools)
		{
			free(m_pMemoryPools);
			m_pMemoryPools = NULL;
		}
	}

public:
	~CMemoryPools(void);

	void* GetBuff(size_t szBuffSize);
	bool DelBuff(size_t szBuffSize, void* pBuff);
	bool DelBuff(void* pBuff);
	/** @brief 对应着打印出内存池此时此刻正在使用的内存链表个数，
	每个链表中有多少内存块在使用，有多少是自由内存，用于内存泄露的跟踪*/
	void DisplayMemoryList();

private:
	CMemoryPools(void);
	void Close();
	void Init();
	void* SetMemoryHead(void* pBuff, _MemoryList* pList, _MemoryBlock* pBlock);
	void* GetMemoryHead(void* pBuff);
	bool GetHeadMemoryBlock(void* pBuff, _MemoryList*& pList, _MemoryBlock*& pBlock);
	

private:
	static CMemoryPools* m_pMemoryPools;
	_MemoryList*         m_pMemoryList;
	_MemoryList*         m_pMemoryListLast;    /**< 最后一个内存管理链表指针  */
	CThreadLock          m_ThreadLock;
};

#endif