#include "CriticalSection.h"

/*
�����ô���
CCriticalSection::CCriticalSection()
{
	InitializeCriticalSection(&m_csObject);
	m_RefCount = 0;
}

CCriticalSection::~CCriticalSection() 
{ 
	DeleteCriticalSection(&m_csObject);
	// 	if (m_RefCount != 0)
	// 	{
	// 		int a = 0;
	// 	}
	m_RefCount = 0xFF;
}

void CCriticalSection::Lock() 
{ 
	EnterCriticalSection(&m_csObject); 
	m_RefCount++;
}

void CCriticalSection::Unlock()
{ 
	// 	if (m_csObject.LockCount < 0)
	// 	{
	// 		int a = 0;
	// 	}
	LeaveCriticalSection(&m_csObject);
	m_RefCount--;
	// 	if (m_RefCount < 0)
	// 	{
	// 		g_Log.OutputDebugString(INFO, "[�߳�����]");
	// 	}
}
*/