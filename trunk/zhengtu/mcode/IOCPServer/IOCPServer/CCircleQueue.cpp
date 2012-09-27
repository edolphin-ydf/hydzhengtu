#include <assert.h>
#include "CCircleQueue.h"

CCircleQueue::CCircleQueue()
{
	m_Id=(WORD)-1;
	m_Max=0;
	m_Front=(UINT)-1;
	m_Back=(UINT)-1;
	m_cData=0;
	m_pBuffer=NULL;
}

CCircleQueue::CCircleQueue(UINT size)
{
	m_Id=(WORD)-1;
	m_Max=size+1;
	m_Front=0;
	m_Back=0;
	m_cData=0;

	// use heap memory
	m_pBuffer=(BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,sizeof(BYTE)*m_Max);
}

CCircleQueue::~CCircleQueue()
{
	if (m_pBuffer!=NULL)
	{
		// use heap memory
		HeapFree(GetProcessHeap(),0,m_pBuffer);

		m_pBuffer=NULL;
	}
}

void CCircleQueue::SetId(WORD id)
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	m_Id=id;
}

WORD CCircleQueue::GetId()
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	WORD id=m_Id;
	return id;
}

BOOL CCircleQueue::CreateCircleQueue(UINT size)
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	assert(size!=0);

	if (m_Max!=0) return FALSE;

	m_Max=size+1;
	m_Front=(UINT)-1;
	m_Back=(UINT)-1;
	m_cData=0;

	// use heap memory
	m_pBuffer=(BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,sizeof(BYTE)*m_Max);

	return TRUE;
}

UINT CCircleQueue::GetCount()
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	UINT cData=m_cData;
	return cData;
}

void CCircleQueue::SetEmpty()
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	m_Front=(UINT)-1;
	m_Back=(UINT)-1;
	m_cData=0;
	if(m_pBuffer)
		ZeroMemory(m_pBuffer,sizeof(BYTE)*m_Max);
}

UINT CCircleQueue::GetMax()
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	UINT max=m_Max;
	return max;
}

UINT CCircleQueue::GetData(void* pBuffer, UINT size)
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue))); 

	//error check
	if (pBuffer==NULL || m_cData==0 || size==0)
		return 0;

	UINT index=(m_Front+1)%(m_Max-1);
	if (index <= m_Back)
	{
		if (m_cData<=size)
		{
			BYTE* p=m_pBuffer;
			p+=index;
			memcpy(pBuffer, p, m_cData);
			return m_cData;
		}
		else
		{
			BYTE* p=m_pBuffer;
			p+=index;
			memcpy(pBuffer, p, size);
			return size;
		}
	}
	else
	{
		if (m_cData<=size)
		{
			UINT size2=m_Max-2-m_Front;
			BYTE* p=m_pBuffer;
			p+=index;
			memcpy(pBuffer, p, size2);

			BYTE* p2=(BYTE*)pBuffer;
			p2+=size2;
			memcpy(p2, m_pBuffer, m_cData-size2);
			return m_cData;
		}
		else
		{
			UINT size2=m_Max-2-m_Front;
			if (size<=size2)
			{
				BYTE* p=m_pBuffer;
				p+=index;
				memcpy(pBuffer, p, size);
				return size;
			}
			else
			{
				BYTE* p=m_pBuffer;
				p+=index;
				memcpy(pBuffer, p, size2);

				BYTE* p2=(BYTE*)pBuffer;
				p2+=size2;
				memcpy(p2, m_pBuffer, size-size2);
				return size;
			}
		}
	}
}

BOOL CCircleQueue::Add(void* pBuffer, UINT size)
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	//error check
	if (m_pBuffer==NULL || size==0 || (m_cData+size)>(m_Max-1))
		return FALSE;

	UINT index=(m_Back+1)%(m_Max-1);
	if (m_Front <= m_Back)
	{
		if ((index+size-1)<=(m_Max-2))
		{
			//                .
			BYTE* p=m_pBuffer;
			p+=index;

			memcpy(p, pBuffer, size);
			m_Back=index+size-1;
		}
		else
		{
			UINT size2=m_Max-2-m_Back;
			BYTE* p=m_pBuffer;
			p+=index;

			memcpy(p, pBuffer, size2);
			BYTE* p2=(BYTE*)pBuffer;
			p2+=size2;

			memcpy(m_pBuffer, p2, size-size2);
			m_Back=size-size2-1;
		}
	}
	else
	{
		BYTE* p=m_pBuffer;
		p+=index;

		memcpy(p, pBuffer, size);
		m_Back=index+size-1;
	}

	m_cData+=size;

	return TRUE;
}

BOOL CCircleQueue::Delete(UINT size)
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	if (m_pBuffer==NULL || size==0 || m_cData<size)
		return FALSE;

	if (m_Front+size<=m_Max-1)
	{
		m_Front+=size;
	}
	else
	{
		m_Front=((m_Front+size)-(m_Max-1));
	}

	m_cData-=size;

	return TRUE;
}

BOOL CCircleQueue::DeleteForBack(UINT size)
{
	assert(FALSE==IsBadReadPtr(this, sizeof(CCircleQueue)));

	if (m_pBuffer==NULL || size==0 || m_cData<size)
		return FALSE;

	if(m_Back>=size)
	{
		m_Back-=size;
	}
	else
	{
		m_Back=((m_Max-1)-(size-m_Back));
	}

	m_cData-=size;

	return TRUE;
}
