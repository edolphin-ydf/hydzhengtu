#include "MemoryPools.h"


CMemoryPools* CMemoryPools::m_pMemoryPools = NULL;
CMemoryPools::CMemoryPools(void)
{
	Init();
}

CMemoryPools::~CMemoryPools(void)
{
	Close();
}

void CMemoryPools::Init()
{
	m_pMemoryList     = NULL;
	m_pMemoryListLast = NULL;
	m_ThreadLock.Init();
}

void CMemoryPools::Close()
{
	/** @brief 添加线程安全 */
	CAutoLock autolock(&m_ThreadLock);

	/** @brief 删除所有已经分配的文件块 */
	_MemoryList* pCurrMemoryList = m_pMemoryList;
	while(NULL != pCurrMemoryList)
	{
		_MemoryBlock* pMemoryUsed     = pCurrMemoryList->m_pMemoryUsed;
		while(NULL != pMemoryUsed)
		{
			if(NULL != pMemoryUsed->m_pBrick)
			{
				free(pMemoryUsed->m_pBrick);
				pMemoryUsed->m_pBrick = NULL;
			}
			_MemoryBlock* toDel = pMemoryUsed;
			pMemoryUsed     = pMemoryUsed->m_pNext;
			free(toDel);
		}

		_MemoryBlock* pMemoryFree     = pCurrMemoryList->m_pMemoryFree;
		while(NULL != pMemoryFree)
		{
			if(NULL != pMemoryFree->m_pBrick)
			{
				free(pMemoryFree->m_pBrick);
				pMemoryFree->m_pBrick = NULL;
			}

			_MemoryBlock* toDel = pMemoryFree;
			pMemoryFree     = pMemoryFree->m_pNext;
			free(toDel);
		}
		m_pMemoryList = pCurrMemoryList;
		pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
		free(m_pMemoryList);
	}
}

void* CMemoryPools::SetMemoryHead(void* pBuff, _MemoryList* pList, _MemoryBlock* pBlock,int aindex ,int line )
{
	/**< 组成内存包头  */
	if(NULL == pBuff)
	{
		return NULL;
	}

	/** @brief 因为一个long是4个字节，在linux和windows下都是一样的。所以加起来是12个 */
	UINT32* plData = (UINT32*)pBuff;
	int i = 0;
	plData[i++] = (UINT32)pList;         /**< 内存链表首地址  */
	plData[i++] = (UINT32)pBlock;        /**< 所在链表的地址  */
#ifdef _DEBUG
	plData[i++] = (UINT32)aindex;
	plData[i++] = (UINT32)line;
#endif
	plData[i++] = (UINT32)MAGIC_CODE;    /**< 验证码  */

	return &plData[i];
}

void* CMemoryPools::GetMemoryHead(void* pBuff)
{
	if(NULL == pBuff)
	{
		return NULL;
	}

	long* plData = (long*)pBuff;

	return &plData[MAX_MEMORYHEAD_SIZE/4];
}

bool CMemoryPools::GetHeadMemoryBlock(void* pBuff, _MemoryList*& pList, _MemoryBlock*& pBlock, int& map_inex)
{
	if (pBuff == NULL)
	{
		return false;
	}
	char* szbuf = (char*)pBuff;
	UINT32* plData = (UINT32*)(szbuf - MAX_MEMORYHEAD_SIZE);
	if(plData[MAX_MEMORYHEAD_SIZE/4-1] != (long)MAGIC_CODE)
	{
		return false;
	}
	else
	{
		pList  = (_MemoryList*)plData[0];   //内存链表首地址
		pBlock = (_MemoryBlock*)plData[1];  //所在链表的地址
#ifdef _DEBUG
		map_inex = plData[2];//返回MAP地址
#endif

		return true;
	}

}

void* CMemoryPools::GetBuff(size_t szBuffSize,const char *file, int line)
{
	/** @brief 添加线程安全 */
	CAutoLock autolock(&m_ThreadLock);

	void* pBuff = NULL;

	//判断这个内存块大小是否存在。
	if(NULL == m_pMemoryList)
	{
		//第一次使用内存管理器
#ifdef _DEBUG
		int aindex = map_file.size();
		map_file.insert(MAP_FILE::value_type(aindex,file));
#endif
		pBuff = malloc(szBuffSize + MAX_MEMORYHEAD_SIZE);
		if(NULL == pBuff)
		{
			//printf_s("[CMemoryPools::GetBuff] pBuff malloc = NULL.\n");
			return NULL;
		}

		m_pMemoryList = (_MemoryList* )malloc(sizeof(_MemoryList));
		if(NULL == m_pMemoryList)
		{
			//printf_s("[CMemoryPools::GetBuff] m_pMemoryList new = NULL.\n");
			free(pBuff);
			return NULL;
		}
		m_pMemoryList->Init();

		//新建一个内存块链表
		//_MemoryBlock* pMemoryUsed = new _MemoryBlock();
		_MemoryBlock* pMemoryUsed = (_MemoryBlock* )malloc(sizeof(_MemoryBlock));
		if(NULL == pMemoryUsed)
		{
			//printf_s("[CMemoryPools::GetBuff] pMemoryBrick new = NULL.\n");
			free(pBuff);
			return NULL;
		}
		pMemoryUsed->Init();

		pMemoryUsed->m_pBrick = pBuff;

		m_pMemoryList->m_nSize           = (int)szBuffSize;
		m_pMemoryList->m_pMemoryUsed     = pMemoryUsed;
		m_pMemoryList->m_pMemoryUsedLast = pMemoryUsed;

		m_pMemoryListLast = m_pMemoryList;

		//return pBuff;
#ifdef _DEBUG
		return SetMemoryHead(pBuff, m_pMemoryList, pMemoryUsed,aindex,line);
#else
		return SetMemoryHead(pBuff, m_pMemoryList, pMemoryUsed);
#endif
	}

	//查找已有的链表中是否存在空余内存块
	_MemoryList* pCurrMemoryList = m_pMemoryList;
	while(NULL != pCurrMemoryList)
	{
		if(pCurrMemoryList->m_nSize == (int)szBuffSize)
		{
			//如果找到存在链表，则在这样的链表中寻找自由的内存块
			_MemoryBlock* pMemoryFree = pCurrMemoryList->m_pMemoryFree;
			if(NULL == pMemoryFree)
			{
				//没有剩余的自由内存块，新建内存块。
#ifdef _DEBUG
				int aindex = map_file.size();
				map_file.insert(MAP_FILE::value_type(aindex,file));
#endif
				pBuff = malloc(szBuffSize + MAX_MEMORYHEAD_SIZE);
				if(NULL == pBuff)
				{
					//printf_s("[CMemoryPools::GetBuff] (pMemoryFree) pBuff malloc = NULL.\n");
					return NULL;
				}

				//新建一个内存块链表
				//_MemoryBlock* pMemoryUsed = new _MemoryBlock();
				_MemoryBlock* pMemoryUsed = (_MemoryBlock* )malloc(sizeof(_MemoryBlock));
				if(NULL == pMemoryUsed)
				{
					//printf_s("[CMemoryPools::GetBuff] pMemoryBrick new = NULL.\n");
					free(pBuff);
					return NULL;
				}
				pMemoryUsed->Init();

				pMemoryUsed->m_pBrick = pBuff;
				_MemoryBlock* pMemoryUsedLast = m_pMemoryList->m_pMemoryUsedLast;
				if(NULL == pMemoryUsedLast)
				{
					//printf_s("[CMemoryPools::GetBuff] 没有最后使用的内存 pBuff = 0x%08x.\n", pBuff);
					pCurrMemoryList->m_nSize           = (int)szBuffSize;
					pCurrMemoryList->m_pMemoryUsed     = pMemoryUsed;
					pCurrMemoryList->m_pMemoryUsedLast = pMemoryUsed;
					//return pBuff;
#ifdef _DEBUG
					return SetMemoryHead(pBuff, pCurrMemoryList, pMemoryUsed,aindex,line);
#else
					return SetMemoryHead(pBuff, pCurrMemoryList, pMemoryUsed);
#endif
				}
				else
				{
					pMemoryUsed->m_pPrev                        = pCurrMemoryList->m_pMemoryUsedLast;
					pCurrMemoryList->m_pMemoryUsedLast->m_pNext = pMemoryUsed;
					pCurrMemoryList->m_pMemoryUsedLast          = pMemoryUsed;
					//return pBuff;
#ifdef _DEBUG
					return SetMemoryHead(pBuff, pCurrMemoryList, pMemoryUsed,aindex,line);
#else
					return SetMemoryHead(pBuff, pCurrMemoryList, pMemoryUsed);
#endif
				}
			}
			else
			{
				//将空余内存弹出来
				_MemoryBlock* pMemoryTemp      = pMemoryFree;
				pCurrMemoryList->m_pMemoryFree = pMemoryFree->m_pNext;
				pBuff                          = pMemoryTemp->m_pBrick;

				pMemoryTemp->m_pPrev                        = pCurrMemoryList->m_pMemoryUsedLast;
				pMemoryFree->m_pNext                        = NULL;

				if(NULL == pCurrMemoryList->m_pMemoryUsedLast)
				{
					pCurrMemoryList->m_pMemoryUsedLast          = pMemoryTemp;
					pCurrMemoryList->m_pMemoryUsed              = pMemoryTemp;
				}
				else
				{
					pCurrMemoryList->m_pMemoryUsedLast->m_pNext = pMemoryTemp;
					pCurrMemoryList->m_pMemoryUsedLast          = pMemoryTemp;
				}

				//printf_s("[CMemoryPools::GetBuff] 空余内存弹出来 pBuff = 0x%08x.\n", pBuff);
				//return pBuff;
				return GetMemoryHead(pBuff);
			}
		}
		else
		{
			pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
		}
	}

	//如果已有内存中不存在以上内存块，则新建一个Memorylist
	{
		//没有剩余的自由内存块，新建内存块。
#ifdef _DEBUG
		int aindex = map_file.size();
		map_file.insert(MAP_FILE::value_type(aindex,file));
#endif
		pBuff = malloc(szBuffSize + MAX_MEMORYHEAD_SIZE);
		if(NULL == pBuff)
		{
			//printf_s("[CMemoryPools::GetBuff] (m_pMemoryList) pBuff malloc = NULL.\n");
			return NULL;
		}

		//_MemoryList* pMemoryList = new _MemoryList();
		_MemoryList* pMemoryList = (_MemoryList* )malloc(sizeof(_MemoryList));
		if(NULL == pMemoryList)
		{
			//printf_s("[CMemoryPools::GetBuff] (m_pMemoryList) m_pMemoryList new = NULL.\n");
			free(pBuff);
			return NULL;
		}
		pMemoryList->Init();

		//新建一个内存块链表
		//_MemoryBlock* pMemoryUsed = new _MemoryBlock();
		_MemoryBlock* pMemoryUsed = (_MemoryBlock* )malloc(sizeof(_MemoryBlock));
		if(NULL == pMemoryUsed)
		{
			//printf_s("[CMemoryPools::GetBuff] (m_pMemoryList) pMemoryBrick new = NULL.\n");
			free(pBuff);
			return NULL;
		}
		pMemoryUsed->Init();

		pMemoryUsed->m_pBrick = pBuff;

		pMemoryList->m_nSize           = (int)szBuffSize;
		pMemoryList->m_pMemoryUsed     = pMemoryUsed;
		pMemoryList->m_pMemoryUsedLast = pMemoryUsed;

		m_pMemoryListLast->m_pMemLNext = pMemoryList;
		m_pMemoryListLast = pMemoryList;

		//return pBuff;
#ifdef _DEBUG
		return SetMemoryHead(pBuff, m_pMemoryList, pMemoryUsed,aindex,line);
#else
		return SetMemoryHead(pBuff, m_pMemoryList, pMemoryUsed);
#endif
	}
}

bool CMemoryPools::DelBuff(size_t szBuffSize, void* pBuff)
{
	/** @brief 添加线程安全 */
	CAutoLock autolock(&m_ThreadLock);

	//在内存块中寻找指定的地址似是否存在，如果存在标记为已经释放。
	_MemoryList* pCurrMemoryList = m_pMemoryList;
	while(NULL != pCurrMemoryList)
	{
		if(pCurrMemoryList->m_nSize == (int)szBuffSize)
		{
			_MemoryBlock* pMemoryUsed     = pCurrMemoryList->m_pMemoryUsed;

			bool blFirst = true;
			while(NULL != pMemoryUsed)
			{
				if(pBuff == pMemoryUsed->m_pBrick)
				{
					if(NULL != pMemoryUsed)
					{
						//如果是列表的第一个，则直接把下一个链表的地址复制到前一个
						if(true == blFirst)
						{
							//把指向前端的地址赋值为当前指针之前的指针
							if(NULL != pMemoryUsed->m_pNext)
							{
								pMemoryUsed->m_pNext->m_pPrev = pMemoryUsed->m_pPrev;
							}

							pCurrMemoryList->m_pMemoryUsed = pMemoryUsed->m_pNext;

							if(pMemoryUsed == pCurrMemoryList->m_pMemoryUsedLast)
							{
								pCurrMemoryList->m_pMemoryUsedLast = pCurrMemoryList->m_pMemoryUsedLast->m_pPrev;
							}

							blFirst= false;
						}
						else
						{
							//把指向前端的地址赋值为当前指针之前的指针
							if(NULL != pMemoryUsed->m_pNext)
							{
								pMemoryUsed->m_pNext->m_pPrev = pMemoryUsed->m_pPrev;
							}

							if(pMemoryUsed == pCurrMemoryList->m_pMemoryUsedLast)
							{
								pCurrMemoryList->m_pMemoryUsedLast = pCurrMemoryList->m_pMemoryUsedLast->m_pPrev;
							}
							else
							{
								pMemoryUsed->m_pPrev->m_pNext = pMemoryUsed->m_pNext;
								//printf_s("[CMemoryPools::DelBuff] 内存指针指向下一个 pMemoryUsed->m_pPrev->m_pNext = 0x%08x.\n", pMemoryUsed->m_pPrev->m_pNext->m_pBrick);
							}
						}

						if(pCurrMemoryList->m_pMemoryFree == NULL)
						{
							//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
							pMemoryUsed->m_pPrev               = NULL;
							pMemoryUsed->m_pNext               = NULL;
							pCurrMemoryList->m_pMemoryFree     = pMemoryUsed;
							pCurrMemoryList->m_pMemoryFreeLast = pMemoryUsed;

						}
						else
						{
							//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
							pMemoryUsed->m_pPrev                        = pCurrMemoryList->m_pMemoryFreeLast;
							pMemoryUsed->m_pNext                        = NULL;
							pCurrMemoryList->m_pMemoryFreeLast->m_pNext = pMemoryUsed;
							pCurrMemoryList->m_pMemoryFreeLast          = pMemoryUsed;
						}

						return true;
					}
					else
					{
						//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x pMemoryUsedProv is NULL.\n", pBuff);
						return false;
					}
				}

				pMemoryUsed     = pMemoryUsed->m_pNext;
				blFirst         = false;
			}
		}

		pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
	}

	//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x is not memoryPool.\n", pBuff);
	return false;
}

bool CMemoryPools::DelBuff(void* pBuff)
{
	/** @brief 添加线程安全 */
	CAutoLock autolock(&m_ThreadLock);

	_MemoryBlock* pMemoryUsed     = NULL;
	_MemoryList*  pCurrMemoryList = NULL;
	int map_index = 0;

	if(false == GetHeadMemoryBlock(pBuff, pCurrMemoryList, pMemoryUsed,map_index))
	{
		return false;
	}

#ifdef _DEBUG
	map_file.erase(map_index);
#endif

	if(NULL != pMemoryUsed && NULL != pCurrMemoryList)
	{
		//如果是列表的第一个，则直接把下一个链表的地址复制到前一个
		if(pCurrMemoryList->m_pMemoryUsed == pMemoryUsed)
		{
			pCurrMemoryList->m_pMemoryUsed = pMemoryUsed->m_pNext;
		}
		else
		{
			pMemoryUsed->m_pPrev->m_pNext = pMemoryUsed->m_pNext;
		}

		if(NULL != pMemoryUsed->m_pNext)
		{
			pMemoryUsed->m_pNext->m_pPrev = pMemoryUsed->m_pPrev;
		}

		if(pMemoryUsed == pCurrMemoryList->m_pMemoryUsedLast)
		{
			pCurrMemoryList->m_pMemoryUsedLast = pCurrMemoryList->m_pMemoryUsedLast->m_pPrev;
		}

		if(pCurrMemoryList->m_pMemoryFree == NULL)
		{
			//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
			pMemoryUsed->m_pPrev               = NULL;
			pMemoryUsed->m_pNext               = NULL;
			pCurrMemoryList->m_pMemoryFree     = pMemoryUsed;
			pCurrMemoryList->m_pMemoryFreeLast = pMemoryUsed;
			//printf_s("[CMemoryPools::DelBuff] 内存列表为空 m_pMemoryFree.m_pBrick = 0x%08x.\n", pCurrMemoryList->m_pMemoryFreeLast->m_pBrick);
		}
		else
		{
			//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
			pMemoryUsed->m_pPrev                        = pCurrMemoryList->m_pMemoryFreeLast;
			pMemoryUsed->m_pNext                        = NULL;
			pCurrMemoryList->m_pMemoryFreeLast->m_pNext = pMemoryUsed;
			pCurrMemoryList->m_pMemoryFreeLast          = pMemoryUsed;
			//printf_s("[CMemoryPools::DelBuff] 内存列表非空 m_pMemoryFree.m_pBrick = 0x%08x.\n", pCurrMemoryList->m_pMemoryFreeLast->m_pBrick);
		}

		return true;
	}
	else
	{
		//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x pMemoryUsedProv is NULL.\n", pBuff);
		return false;
	}


	//printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x is not memoryPool.\n", pBuff);
	return false;
}

_MemoryInfo CMemoryPools::GetAndPlayMemoryList()
{
	_MemoryInfo _info;
	_info.nUsedSize = 0;
	_info.nFreeSize = 0;
	int nUsedCount = 0;
	int nFreeCount = 0;

	_MemoryList* pCurrMemoryList = m_pMemoryList;
	while(NULL != pCurrMemoryList)
	{
		_MemoryBlock* pMemoryUsed     = pCurrMemoryList->m_pMemoryUsed;
		_MemoryBlock* pMemoryFree     = pCurrMemoryList->m_pMemoryFree;

		nUsedCount = 0;
		nFreeCount = 0;

		while(NULL != pMemoryUsed)
		{
			nUsedCount++;
			pMemoryUsed = pMemoryUsed->m_pNext;
		}
		_info.nUsedSize = pCurrMemoryList->m_nSize * nUsedCount;
		printf_s("[CMemoryPools::DisplayMemoryList] pMemoryUsed nUsedCount = %d, Size = %d.\n", nUsedCount, _info.nUsedSize);

		while(NULL != pMemoryFree)
		{
			nFreeCount++;
			pMemoryFree = pMemoryFree->m_pNext;
		}
		_info.nFreeSize = pCurrMemoryList->m_nSize * nFreeCount;
		printf_s("[CMemoryPools::DisplayMemoryList] pMemoryFree nFreeCount = %d, Size = %d.\n", nFreeCount, _info.nFreeSize);

		pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
	}
	_info.nUsedCount = nUsedCount;
	_info.nFreeCount = nFreeCount;
	return _info;
}

