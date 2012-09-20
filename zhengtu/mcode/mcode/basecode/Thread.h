/*
 文件名 : Thread.h
 创建时间 : 2012/9/18
 作者 : hyd
 功能 : 
*/
#ifndef __Thread_H__
#define __Thread_H__
#include "Common.h"
#include "Node.h"
#include "Mutex.h"
#include <vector>
/**
* \brief 封装了线程操作，所有使用线程的基类
*
*/
class CThread : private CNode
{
public:

	/**
	* \brief 构造函数，创建一个对象
	*
	* \param name 线程名称
	* \param joinable 标明这个线程退出的时候是否保存状态，如果为true表示线程退出保存状态，否则将不保存退出状态
	*/
	CThread(const std::string &name = std::string("CThread"),const bool joinable = true) 
		: threadName(name),alive(false),complete(false),joinable(joinable) { m_hThread = NULL; };

	/**
	* \brief 析构函数，用于销毁一个对象，回收对象空间
	*
	*/
	virtual ~CThread()
	{
		if (NULL != m_hThread)
		{
			CloseHandle(m_hThread);
		}
	};

	/**
	* \brief 使当前线程睡眠指定的时间，秒
	*
	*
	* \param sec 指定的时间，秒
	*/
	static void sleep(const long sec)
	{
		::Sleep(1000 * sec);
	}

	/**
	* \brief 使当前线程睡眠指定的时间，毫秒
	*
	*
	* \param msec 指定的时间，毫秒
	*/
	static void msleep(const long msec)
	{
		::Sleep(msec);
	}

	/**
	* \brief 使当前线程睡眠指定的时间，微秒
	*
	*
	* \param usec 指定的时间，微秒
	*/
	static void usleep(const long usec)
	{
		::Sleep(usec / 1000);
	}

	/**
	* \brief 线程是否是joinable的
	*
	*
	* \return joinable
	*/
	const bool isJoinable() const
	{
		return joinable;
	}

	/**
	* \brief 检查线程是否在运行状态
	*
	* \return 线程是否在运行状态
	*/
	const bool isAlive() const
	{
		return alive;
	}

	static DWORD WINAPI threadFunc(void *arg);
	bool start();
	void join();

	/**
	* \brief 主动结束线程
	*
	* 其实只是设置标记，那么线程的run主回调循环回检查这个标记，如果这个标记已经设置，就退出循环
	*
	*/
	void final()
	{
		complete = true;
	}

	/**
	* \brief 判断线程是否继续运行下去
	*
	* 主要用在run()函数循环中，判断循环是否继续执行下去
	*
	* \return 线程主回调是否继续执行
	*/
	const bool isFinal() const 
	{
		return complete;
	}

	/**
	* \brief 纯虚构函数，线程主回调函数，每个需要实例华的派生类需要重载这个函数
	*
	* 如果是无限循环需要在每个循环检查线程退出标记isFinal()，这样能够保证线程安全退出
	* <pre>
	*   while(!isFinal())
	*   {
	*     ...
	*   }
	*   </pre>
	*
	*/
	virtual void run() = 0;

	/**
	* \brief 判断两个线程是否是同一个线程
	* \param other 待比较的线程
	* \return 是否是同一个线程
	*/
	//bool operator==(const CThread& other) const
	//{
	// return pthread_equal(thread,other.thread) != 0;
	//}

	/**
	* \brief 判断两个线程是否不是同一个线程
	* \param other 待比较的线程
	* \return 是否不是同一个线程
	*/
	//bool operator!=(const CThread& other) const
	//{
	//return !operator==(other);
	//}

	/**
	* \brief 返回线程名称
	*
	* \return 线程名称
	*/
	const std::string &getThreadName() const
	{
		return threadName;
	}

public:

	std::string threadName;      /**< 线程名称 */
	Mutex mlock;                /**< 互斥锁 */
	volatile bool alive;         /**< 线程是否在运行 */
	volatile bool complete;
	HANDLE m_hThread;            /**< 线程编号 */
	bool joinable;               /**< 线程属性，是否设置joinable标记 */

}; 

/**
* \brief 对线程进行分组管理的类
*
*/
class CThreadGroup : private CNode
{

public:

	struct Callback
	{
		virtual void exec(CThread *e)=0;
		virtual ~Callback(){};
	};

	typedef std::vector<CThread *> Container;  /**< 容器类型 */

	CThreadGroup();
	~CThreadGroup();
	void add(CThread *thread);
	CThread *getByIndex(const Container::size_type index);
	CThread *operator[] (const Container::size_type index);
	void joinAll();
	void execAll(Callback &cb);

	const Container::size_type size()
	{
		RWLock_scope_rdlock scope_rdlock(rwlock);
		return vts.size();
	}

private:

	Container vts;                /**< 线程向量 */
	RWLock rwlock;                /**< 读写锁 */

};

#endif


