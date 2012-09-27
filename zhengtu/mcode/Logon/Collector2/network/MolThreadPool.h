#ifndef _MOL_THREAD_POOL_H_INCLUDE
#define _MOL_THREAD_POOL_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:线程池
* 作者:akinggw
* 日期:2010.2.11
*/

#include "MolCommon.h"
#include "MolMutex.h"
#include "MolThreadStarter.h"

class ThreadController
{
public:
	HANDLE hThread;
	uint32 thread_id;

	void Setup(HANDLE h)
	{
		hThread = h;
	}

	void Suspend()
	{
		assert(GetCurrentThreadId() == thread_id);
		SuspendThread(hThread);
	}

	 void Resume()
	 {
		 assert(GetCurrentThreadId() != thread_id);
		 if(ResumeThread(hThread) == DWORD(-1))
		 {
			 DWORD le = GetLastError();
			 TRACE("lasterror: %u\n",le);
		 }
	 }

	 void Join()
	 {
		 WaitForSingleObject(hThread,INFINITE);
	 }

	 uint32 GetId() { return thread_id; }
};

struct Thread 
{
	ThreadBase * ExecutionTarget;
	ThreadController ControlInterface;
	Mutex SetupMutex;
	bool DeleteAfterExit;
};

typedef std::set<Thread*> ThreadSet;

class CThreadPool
{
public:
	/// 构造函数
	CThreadPool();
	
	/// 每两分钟调用一次
	void IntegrityCheck();
	/// 开始调用
	void Startup();
	/// 卸载所有的线程
	void Shutdown();
	/// 退出线程
	bool ThreadExit(Thread * t);
	/// 建立线程
	Thread * StartThread(ThreadBase * ExecutionTarget);
	/// 分配一个线程开始一个任务
	void ExecuteTask(ThreadBase * ExecutionTarget);
	/// 打印调试消息
	void ShutStats();
	/// 关闭指定数量的线程
	void KillFreeThreads(uint32 count);

	inline void Gobble() { _threadsEaten = (int)m_freeThreads.size(); }
	/// 得到活动线程的数量
	inline uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }
	/// 得到空闲线程的数量
	inline uint32 GetFreeThreadCount() { return (uint32)m_freeThreads.size(); }
private:
	/// 得到CPU的个数
	int GetNumCpus();

	uint32 _threadsRequestedSinceLastCheck;
	uint32 _threadsFreedSinceLastCheck;
	uint32 _threadsExitedSinceLastCheck;
	uint32 _threadsToExit;
	int32 _threadsEaten;
	Mutex _mutex;

	ThreadSet m_activeThreads;
	ThreadSet m_freeThreads;
};

extern CThreadPool ThreadPool;

#endif
