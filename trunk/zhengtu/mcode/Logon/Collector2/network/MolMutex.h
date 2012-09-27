#ifndef _MOL_MUTEX_H_INCLUDE
#define _MOL_MUTEX_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:互斥类
* 作者:akinggw
* 日期:2010.2.11
*/

#include "MolCommon.h"

class Mutex
{
public:
	/// 构造函数
	Mutex();
	/// 析构函数
	virtual ~Mutex();

	/// 取得临界区
	inline void Acquire()
	{
		EnterCriticalSection(&cs);
	}
	/// 释放临界区
	inline void Release()
	{
		LeaveCriticalSection(&cs);
	}
	/// 试图取得临界区
	inline bool AttemptAcquire()
	{
		return (TryEnterCriticalSection(&cs) == TRUE) ? true : false;
	}

protected:
	CRITICAL_SECTION cs;
};

#endif