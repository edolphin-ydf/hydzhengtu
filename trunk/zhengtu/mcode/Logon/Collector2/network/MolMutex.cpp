#include "stdafx.h"
#include "MolMutex.h"

/** 
 * ���캯��
 */
Mutex::Mutex()
{
	InitializeCriticalSection(&cs);
}

/** 
 * ��������
 */
Mutex::~Mutex()
{
	DeleteCriticalSection(&cs);
}