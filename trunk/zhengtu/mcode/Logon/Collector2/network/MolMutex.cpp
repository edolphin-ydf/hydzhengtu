#include "stdafx.h"
#include "MolMutex.h"

/** 
 * 构造函数
 */
Mutex::Mutex()
{
	InitializeCriticalSection(&cs);
}

/** 
 * 析构函数
 */
Mutex::~Mutex()
{
	DeleteCriticalSection(&cs);
}