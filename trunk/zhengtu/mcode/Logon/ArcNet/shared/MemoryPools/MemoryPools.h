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

/** @brief 4���ֽڵ�ָ����ַ + 4���ֽڵ������׵�ַ + 4λ��֤��*/
#define MAX_MEMORYHEAD_SIZE 12    
/** @brief ��֤��*/
#define MAGIC_CODE          0x123456

#define UINT32 unsigned int

////////////////////////////////////////////////////////////////
/// @struct _MemoryBlock
/// @brief  �ڴ��Ľṹ��˫������
////////////////////////////////////////////////////////////////
struct _MemoryBlock
{
	_MemoryBlock *m_pNext; /**< ��ǰָ��  */
	_MemoryBlock *m_pPrev; /**< ���ָ��  */
	void*         m_pBrick;/**< �ڴ��ָ��  */

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
/// @brief  �ڴ�����б�
////////////////////////////////////////////////////////////////
struct _MemoryList    
{
	_MemoryList*   m_pMemLNext;
	_MemoryBlock*  m_pMemoryFree;    /**< ���ɵ��ڴ��  */
	_MemoryBlock*  m_pMemoryFreeLast;/**< ���ɵ��ڴ������ĩβ  */
	_MemoryBlock*  m_pMemoryUsed;    /**< ʹ�õ��ڴ��  */
	_MemoryBlock*  m_pMemoryUsedLast;/**< ʹ�õ��ڴ������ĩβ  */

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
/// @brief �ڴ����
///
/// @note  �����ķ���ֻ����������Ӧ��new��delete������
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
	/** @brief ��Ӧ�Ŵ�ӡ���ڴ�ش�ʱ�˿�����ʹ�õ��ڴ����������
	ÿ���������ж����ڴ����ʹ�ã��ж����������ڴ棬�����ڴ�й¶�ĸ���*/
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
	_MemoryList*         m_pMemoryListLast;    /**< ���һ���ڴ��������ָ��  */
	CThreadLock          m_ThreadLock;
};

#endif