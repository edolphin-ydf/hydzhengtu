/*
 文件名 : Mutex.h
 创建时间 : 2012/9/18
 作者 : hyd
 功能 : 
*/
#ifndef __Mutex_H__
#define __Mutex_H__
#include "Common.h"
#include "Node.h"
/**
* \brief 临界区，封装了系统临界区，避免了使用系统临界区时候需要手工初始化和销毁临界区对象的操作
*
*/
class Mutex : private CNode
{
	friend class CCond;
public:
/**
* \brief 构造函数，构造一个互斥体对象
*
*/
	Mutex() 
	{
		m_hMutex = CreateMutex(NULL,FALSE,NULL);
	}

	/**
	* \brief 析构函数，销毁一个互斥体对象
	*
	*/
	~Mutex()
	{
		CloseHandle(m_hMutex);
	}

	/**
	* \brief 加锁一个互斥体
	*
	*/
	inline void lock()
	{
		if( WaitForSingleObject(m_hMutex,10000) == WAIT_TIMEOUT )
		{
			char szName[MAX_PATH];
			GetModuleFileName(NULL,szName,sizeof(szName));
			::MessageBox(NULL,"发生死锁！", szName, MB_ICONERROR);
		}
	}

	/**
	* \brief 解锁一个互斥体
	*
	*/
	inline void unlock()
	{
		ReleaseMutex(m_hMutex);
	}

private:

	HANDLE m_hMutex;    /**< 系统互斥体 */
	
};


/**
* \brief Wrapper
* 方便在复杂函数中锁的使用
*/
class Mutex_scope_lock : private CNode
{
public:
	/**
	* \brief 构造函数
	* 对锁进行lock操作
	* \param m 锁的引用
	*/
	Mutex_scope_lock(Mutex &m) : mlock(m)
	{
		mlock.lock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~Mutex_scope_lock()
	{
		mlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	Mutex &mlock;

};

/**
* \brief 封装了系统条件变量，使用上要简单，省去了手工初始化和销毁系统条件变量的工作，这些工作都可以由构造函数和析构函数来自动完成
*
*/
class CCond : private CNode
{

public:

	/**
	* \brief 构造函数，用于创建一个条件变量
	*
	*/
	CCond()
	{
		m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	}

	/**
	* \brief 析构函数，用于销毁一个条件变量
	*
	*/
	~CCond()
	{
		CloseHandle(m_hEvent);
	}

	/**
	* \brief 对所有等待这个条件变量的线程广播发送信号，使这些线程能够继续往下执行
	*
	*/
	void broadcast()
	{
		SetEvent(m_hEvent);
	}

	/**
	* \brief 对所有等待这个条件变量的线程发送信号，使这些线程能够继续往下执行
	*
	*/
	void signal()
	{
		SetEvent(m_hEvent);
	}

	/**
	* \brief 等待特定的条件变量满足
	*
	*
	* \param m_hMutex 需要等待的互斥体
	*/
	void wait(Mutex &mutex)
	{
		WaitForSingleObject(m_hEvent,INFINITE);
	}

private:

	HANDLE m_hEvent;    /**< 系统条件变量 */

};

/**
* \brief 封装了系统读写锁，使用上要简单，省去了手工初始化和销毁系统读写锁的工作，这些工作都可以由构造函数和析构函数来自动完成
*
*/
class RWLock : private CNode
{

public:
	/**
	* \brief 构造函数，用于创建一个读写锁
	*
	*/
	RWLock()
	{
		m_hMutex = CreateMutex(NULL,FALSE,NULL);
	}

	/**
	* \brief 析构函数，用于销毁一个读写锁
	*
	*/
	~RWLock()
	{
		CloseHandle(m_hMutex);
	}

	/**
	* \brief 对读写锁进行读加锁操作
	*
	*/
	inline void rdlock()
	{
		WaitForSingleObject(m_hMutex,INFINITE);
	};

	/**
	* \brief 对读写锁进行写加锁操作
	*
	*/
	inline void wrlock()
	{
		WaitForSingleObject(m_hMutex,INFINITE);
	}

	/**
	* \brief 对读写锁进行解锁操作
	*
	*/
	inline void unlock()
	{
		ReleaseMutex(m_hMutex);
	}

private:

	HANDLE m_hMutex;    /**< 系统读写锁 */

};

/**
* \brief rdlock Wrapper
* 方便在复杂函数中读写锁的使用
*/
class RWLock_scope_rdlock : private CNode
{

public:

	/**
	* \brief 构造函数
	* 对锁进行rdlock操作
	* \param m 锁的引用
	*/
	RWLock_scope_rdlock(RWLock &m) : rwlock(m)
	{
		rwlock.rdlock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~RWLock_scope_rdlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	RWLock &rwlock;

};

/**
* \brief wrlock Wrapper
* 方便在复杂函数中读写锁的使用
*/
class RWLock_scope_wrlock : private CNode
{

public:

	/**
	* \brief 构造函数
	* 对锁进行wrlock操作
	* \param m 锁的引用
	*/
	RWLock_scope_wrlock(RWLock &m) : rwlock(m)
	{
		rwlock.wrlock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~RWLock_scope_wrlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	RWLock &rwlock;

};

#endif
