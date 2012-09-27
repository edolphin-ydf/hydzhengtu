#include <windows.h>
#include "CriticalSection.h"

#ifndef CCIRCLEQUEUE_H
#define CCIRCLEQUEUE_H



// ѭ������
class CCircleQueue : public CCriticalSection
{
public:
	CCircleQueue();
	CCircleQueue(UINT size);
	~CCircleQueue();

	// ����ID
public:
	void SetId(WORD id);
	WORD GetId();

	// ���в����ӿ�     
public:
	BOOL CreateCircleQueue(UINT size);
	UINT GetCount();
	void SetEmpty();
	UINT GetMax();
	UINT GetData(void* pBuffer, UINT size);

	BOOL Add(void* pBuffer, UINT size);
	BOOL Delete(UINT size);
	BOOL DeleteForBack(UINT size);

	// ����
private:
	WORD m_Id;
	UINT m_Max;
	UINT m_Front;
	UINT m_Back;
	UINT m_cData;

	BYTE* m_pBuffer;
};

#endif