#include <windows.h>

#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H


class CCriticalSection
{
public:
	CCriticalSection() { InitializeCriticalSection(&m_csObject); }
	~CCriticalSection(){ DeleteCriticalSection(&m_csObject); }
public:
	void Lock()   { EnterCriticalSection(&m_csObject); }
	void Unlock() { LeaveCriticalSection(&m_csObject); }

private:
	CRITICAL_SECTION m_csObject;
};



class CAutoLock 
{ 
public: 
	CAutoLock(CCriticalSection& cs) 
	{ 
		m_pcs = &cs; 
		m_pcs->Lock(); 
	} 

	~CAutoLock()
	{ 
		m_pcs->Unlock(); 
	} 

private: 
	CCriticalSection* m_pcs; 
}; 



#endif