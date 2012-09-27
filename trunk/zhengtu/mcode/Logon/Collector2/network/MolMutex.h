#ifndef _MOL_MUTEX_H_INCLUDE
#define _MOL_MUTEX_H_INCLUDE

/** 
* MolNet��������
*
* ����:������
* ����:akinggw
* ����:2010.2.11
*/

#include "MolCommon.h"

class Mutex
{
public:
	/// ���캯��
	Mutex();
	/// ��������
	virtual ~Mutex();

	/// ȡ���ٽ���
	inline void Acquire()
	{
		EnterCriticalSection(&cs);
	}
	/// �ͷ��ٽ���
	inline void Release()
	{
		LeaveCriticalSection(&cs);
	}
	/// ��ͼȡ���ٽ���
	inline bool AttemptAcquire()
	{
		return (TryEnterCriticalSection(&cs) == TRUE) ? true : false;
	}

protected:
	CRITICAL_SECTION cs;
};

#endif