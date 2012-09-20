/*
 �ļ��� : Mutex.h
 ����ʱ�� : 2012/9/18
 ���� : hyd
 ���� : 
*/
#ifndef __Mutex_H__
#define __Mutex_H__
#include "Common.h"
#include "Node.h"
/**
* \brief �ٽ�������װ��ϵͳ�ٽ�����������ʹ��ϵͳ�ٽ���ʱ����Ҫ�ֹ���ʼ���������ٽ�������Ĳ���
*
*/
class Mutex : private CNode
{
	friend class CCond;
public:
/**
* \brief ���캯��������һ�����������
*
*/
	Mutex() 
	{
		m_hMutex = CreateMutex(NULL,FALSE,NULL);
	}

	/**
	* \brief ��������������һ�����������
	*
	*/
	~Mutex()
	{
		CloseHandle(m_hMutex);
	}

	/**
	* \brief ����һ��������
	*
	*/
	inline void lock()
	{
		if( WaitForSingleObject(m_hMutex,10000) == WAIT_TIMEOUT )
		{
			char szName[MAX_PATH];
			GetModuleFileName(NULL,szName,sizeof(szName));
			::MessageBox(NULL,"����������", szName, MB_ICONERROR);
		}
	}

	/**
	* \brief ����һ��������
	*
	*/
	inline void unlock()
	{
		ReleaseMutex(m_hMutex);
	}

private:

	HANDLE m_hMutex;    /**< ϵͳ������ */
	
};


/**
* \brief Wrapper
* �����ڸ��Ӻ���������ʹ��
*/
class Mutex_scope_lock : private CNode
{
public:
	/**
	* \brief ���캯��
	* ��������lock����
	* \param m ��������
	*/
	Mutex_scope_lock(Mutex &m) : mlock(m)
	{
		mlock.lock();
	}

	/**
	* \brief ��������
	* ��������unlock����
	*/
	~Mutex_scope_lock()
	{
		mlock.unlock();
	}

private:

	/**
	* \brief ��������
	*/
	Mutex &mlock;

};

/**
* \brief ��װ��ϵͳ����������ʹ����Ҫ�򵥣�ʡȥ���ֹ���ʼ��������ϵͳ���������Ĺ�������Щ�����������ɹ��캯���������������Զ����
*
*/
class CCond : private CNode
{

public:

	/**
	* \brief ���캯�������ڴ���һ����������
	*
	*/
	CCond()
	{
		m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	}

	/**
	* \brief ������������������һ����������
	*
	*/
	~CCond()
	{
		CloseHandle(m_hEvent);
	}

	/**
	* \brief �����еȴ���������������̹߳㲥�����źţ�ʹ��Щ�߳��ܹ���������ִ��
	*
	*/
	void broadcast()
	{
		SetEvent(m_hEvent);
	}

	/**
	* \brief �����еȴ���������������̷߳����źţ�ʹ��Щ�߳��ܹ���������ִ��
	*
	*/
	void signal()
	{
		SetEvent(m_hEvent);
	}

	/**
	* \brief �ȴ��ض���������������
	*
	*
	* \param m_hMutex ��Ҫ�ȴ��Ļ�����
	*/
	void wait(Mutex &mutex)
	{
		WaitForSingleObject(m_hEvent,INFINITE);
	}

private:

	HANDLE m_hEvent;    /**< ϵͳ�������� */

};

/**
* \brief ��װ��ϵͳ��д����ʹ����Ҫ�򵥣�ʡȥ���ֹ���ʼ��������ϵͳ��д���Ĺ�������Щ�����������ɹ��캯���������������Զ����
*
*/
class RWLock : private CNode
{

public:
	/**
	* \brief ���캯�������ڴ���һ����д��
	*
	*/
	RWLock()
	{
		m_hMutex = CreateMutex(NULL,FALSE,NULL);
	}

	/**
	* \brief ������������������һ����д��
	*
	*/
	~RWLock()
	{
		CloseHandle(m_hMutex);
	}

	/**
	* \brief �Զ�д�����ж���������
	*
	*/
	inline void rdlock()
	{
		WaitForSingleObject(m_hMutex,INFINITE);
	};

	/**
	* \brief �Զ�д������д��������
	*
	*/
	inline void wrlock()
	{
		WaitForSingleObject(m_hMutex,INFINITE);
	}

	/**
	* \brief �Զ�д�����н�������
	*
	*/
	inline void unlock()
	{
		ReleaseMutex(m_hMutex);
	}

private:

	HANDLE m_hMutex;    /**< ϵͳ��д�� */

};

/**
* \brief rdlock Wrapper
* �����ڸ��Ӻ����ж�д����ʹ��
*/
class RWLock_scope_rdlock : private CNode
{

public:

	/**
	* \brief ���캯��
	* ��������rdlock����
	* \param m ��������
	*/
	RWLock_scope_rdlock(RWLock &m) : rwlock(m)
	{
		rwlock.rdlock();
	}

	/**
	* \brief ��������
	* ��������unlock����
	*/
	~RWLock_scope_rdlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief ��������
	*/
	RWLock &rwlock;

};

/**
* \brief wrlock Wrapper
* �����ڸ��Ӻ����ж�д����ʹ��
*/
class RWLock_scope_wrlock : private CNode
{

public:

	/**
	* \brief ���캯��
	* ��������wrlock����
	* \param m ��������
	*/
	RWLock_scope_wrlock(RWLock &m) : rwlock(m)
	{
		rwlock.wrlock();
	}

	/**
	* \brief ��������
	* ��������unlock����
	*/
	~RWLock_scope_wrlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief ��������
	*/
	RWLock &rwlock;

};

#endif
