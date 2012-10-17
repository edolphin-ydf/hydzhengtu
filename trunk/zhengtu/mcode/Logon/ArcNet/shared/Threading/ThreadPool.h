//////////////////////////////////////////////////
/// @file : threadpool.h
/// @brief : 
/// @date:  2012/10/17
/// @author : hyd
//////////////////////////////////////////////////

#include "../Common.h"
#include "shared/Singleton.h"
#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#ifdef WIN32

class SERVER_DECL ThreadController
{
	public:
		HANDLE hThread;
		uint32 thread_id;

		void Setup(HANDLE h)
		{
			hThread = h;
			// whoops! GetThreadId is for windows 2003 and up only! :<		 - Burlex
			//thread_id = (uint32)GetThreadId(h);
		}

		void Suspend()
		{
			// We can't be suspended by someone else. That is a big-no-no and will lead to crashes.
			ASSERT(GetCurrentThreadId() == thread_id);
			SuspendThread(hThread);
		}

		void Resume()
		{
			// This SHOULD be called by someone else.
			ASSERT(GetCurrentThreadId() != thread_id);
			if(ResumeThread(hThread) == DWORD(-1))
			{
				DWORD le = GetLastError();
				printf("lasterror: %u\n", le);
			}
		}

		void Join()
		{
			WaitForSingleObject(hThread, INFINITE);
		}

		uint32 GetId() { return thread_id; }
};

#else
#ifndef HAVE_DARWIN
#include <semaphore.h>
int GenerateThreadId();

class ThreadController
{
		sem_t sem;
		pthread_t handle;
		int thread_id;
	public:
		void Setup(pthread_t h)
		{
			handle = h;
			sem_init(&sem, PTHREAD_PROCESS_PRIVATE, 0);
			thread_id = GenerateThreadId();
		}
		~ThreadController()
		{
			sem_destroy(&sem);
		}

		void Suspend()
		{
			ASSERT(pthread_equal(pthread_self(), handle));
			sem_wait(&sem);
		}

		void Resume()
		{
			ASSERT(!pthread_equal(pthread_self(), handle));
			sem_post(&sem);
		}

		void Join()
		{
			// waits until the thread finishes then returns
			pthread_join(handle, NULL);
		}

		MNET_INLINE uint32 GetId() { return (uint32)thread_id; }
};

#else
int GenerateThreadId();
class ThreadController
{
		pthread_cond_t cond;
		pthread_mutex_t mutex;
		int thread_id;
		pthread_t handle;
	public:
		void Setup(pthread_t h)
		{
			handle = h;
			pthread_mutex_init(&mutex, NULL);
			pthread_cond_init(&cond, NULL);
			thread_id = GenerateThreadId();
		}
		~ThreadController()
		{
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&cond);
		}
		void Suspend()
		{
			pthread_cond_wait(&cond, &mutex);
		}
		void Resume()
		{
			pthread_cond_signal(&cond);
		}
		void Join()
		{
			pthread_join(handle, NULL);
		}
		MNET_INLINE uint32 GetId() { return (uint32)thread_id; }
};

#endif

#endif

struct SERVER_DECL Thread
{
	ThreadBase* ExecutionTarget;         /**< 线程基类,执行目标  */
	ThreadController ControlInterface;   /**< 线程管理器  */
	Mutex SetupMutex;
	bool DeleteAfterExit;
};

typedef std::set<Thread*> ThreadSet;

class SERVER_DECL CThreadPool
{
		int GetNumCpus();/**< 得到CPU的个数 */

		uint32 _threadsRequestedSinceLastCheck;
		uint32 _threadsFreedSinceLastCheck; /**< 从上回检查到现在变成空闲的线程数目  */
		uint32 _threadsExitedSinceLastCheck;/**< 从上回检查到现在调用退出函数的线程数目  */
		uint32 _threadsToExit;
		int32 _threadsEaten;
		Mutex _mutex;

		ThreadSet m_activeThreads;/**< 正在工作的线程队列 */
		ThreadSet m_freeThreads;  /**< 空闲的线程队列，使用的时候需要启动Resume() */

	public:
		CThreadPool();
		~CThreadPool()
		{
			m_activeThreads.clear();
			m_freeThreads.clear();
		}
		/** @brief 每两分钟调用一次 */ 
		void IntegrityCheck();

		/** @brief  开始调用 */ 
		void Startup();

		/** @brief 卸载所有的线程 */ 
		void Shutdown();
		/** @brief 退出线程
		// return true - suspend ourselves, and wait for a future task.
		// return false - exit, we're shutting down or no longer needed. */ 
		bool ThreadExit(Thread* t);

		/** @brief 建立线程 */ 
		Thread* StartThread(ThreadBase* ExecutionTarget);

		/** @brief 分配一个线程开始一个任务 */ 
		void ExecuteTask(ThreadBase* ExecutionTarget);

		/** @brief  stats打印调试消息 */ 
		void ShowStats();

		/** @brief 关闭指定数量的空闲线程 */ 
		void KillFreeThreads(uint32 count);

		/** @brief resets the gobble counter */ 
		MNET_INLINE void Gobble() { _threadsEaten = (int32)m_freeThreads.size(); }

		/** @brief 得到活动线程的数量 */ 
		MNET_INLINE uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }

		/** @brief 得到空闲线程的数量 */ 
		MNET_INLINE uint32 GetFreeThreadCount() { return (uint32)m_freeThreads.size(); }
};

extern SERVER_DECL CThreadPool ThreadPool;

#endif
