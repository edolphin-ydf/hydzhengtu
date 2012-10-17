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
/** @brief 4���ֽڵ�ָ����ַ + 4���ֽڵ������׵�ַ + 4λ��֤�� + 4���ֽڵ�map������ + 4���ֽڵ��к� */
#define MAX_MEMORYHEAD_SIZE 20  
#else
/** @brief 4���ֽڵ�ָ����ַ + 4���ֽڵ������׵�ַ + 4λ��֤��*/
#define MAX_MEMORYHEAD_SIZE 12    
#endif
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
	bool DelBuff(size_t szBuffSize, void* pBuff); /**< �����ϲ�������ӿ�  */
	bool DelBuff(void* pBuff);
	/** @brief ��Ӧ�Ŵ�ӡ���ڴ�ش�ʱ�˿�����ʹ�õ��ڴ����������\n
	ÿ���������ж����ڴ����ʹ�ã��ж����������ڴ棬�����ڴ�й¶�ĸ���
	*/
	_MemoryInfo GetAndPlayMemoryList();
#ifdef _DEBUG
	/** @brief ��ӡmap�б�����ļ���ַ */
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
	_MemoryList*         m_pMemoryListLast;    /**< ���һ���ڴ��������ָ��  */
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