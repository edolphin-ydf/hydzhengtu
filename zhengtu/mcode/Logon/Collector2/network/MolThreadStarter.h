#ifndef _MOL_THREAD_STARTER_H_INCLUDE
#define _MOL_THREAD_STARTER_H_INCLUDE

#include "MolCommon.h"

/** 
* MolNet网络引擎
*
* 描述:线程执行的基类
* 作者:akinggw
* 日期:2010.2.11
*/

class ThreadBase
{
public:
	/// 构造函数
	ThreadBase() {}
	/// 析构函数
	virtual ~ThreadBase() {}

	virtual bool run() = 0;
	virtual void OnShutdown() {}

	HANDLE THREAD_HANDLE;
};

#endif
