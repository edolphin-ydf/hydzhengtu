#include <windows.h>
#include "CriticalSection.h"

#ifndef CCIRCLEQUEUE_H
#define CCIRCLEQUEUE_H



// 循环队列
class CCircleQueue : public CCriticalSection
{
public:
	CCircleQueue();
	CCircleQueue(UINT size);
	~CCircleQueue();

	// 设置ID
public:
	void SetId(WORD id);
	WORD GetId();

	// 队列操作接口     
public:
	BOOL CreateCircleQueue(UINT size);
	UINT GetCount();
	void SetEmpty();
	UINT GetMax();
	UINT GetData(void* pBuffer, UINT size);

	BOOL Add(void* pBuffer, UINT size);
	BOOL Delete(UINT size);
	BOOL DeleteForBack(UINT size);

	// 属性
private:
	WORD m_Id;
	UINT m_Max;
	UINT m_Front;
	UINT m_Back;
	UINT m_cData;

	BYTE* m_pBuffer;
};

#endif