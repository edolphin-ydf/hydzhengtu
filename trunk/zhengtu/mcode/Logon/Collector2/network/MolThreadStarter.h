#ifndef _MOL_THREAD_STARTER_H_INCLUDE
#define _MOL_THREAD_STARTER_H_INCLUDE

#include "MolCommon.h"

/** 
* MolNet��������
*
* ����:�߳�ִ�еĻ���
* ����:akinggw
* ����:2010.2.11
*/

class ThreadBase
{
public:
	/// ���캯��
	ThreadBase() {}
	/// ��������
	virtual ~ThreadBase() {}

	virtual bool run() = 0;
	virtual void OnShutdown() {}

	HANDLE THREAD_HANDLE;
};

#endif
