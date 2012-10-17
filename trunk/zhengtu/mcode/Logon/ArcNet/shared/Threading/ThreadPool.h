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
	ThreadBase* ExecutionTarget;         /**< �̻߳���,ִ��Ŀ��  */
	ThreadController ControlInterface;   /**< �̹߳�����  */
	Mutex SetupMutex;
	bool DeleteAfterExit;
};

typedef std::set<Thread*> ThreadSet;

class SERVER_DECL CThreadPool
{
		int GetNumCpus();/**< �õ�CPU�ĸ��� */

		uint32 _threadsRequestedSinceLastCheck;
		uint32 _threadsFreedSinceLastCheck; /**< ���ϻؼ�鵽���ڱ�ɿ��е��߳���Ŀ  */
		uint32 _threadsExitedSinceLastCheck;/**< ���ϻؼ�鵽���ڵ����˳��������߳���Ŀ  */
		uint32 _threadsToExit;
		int32 _threadsEaten;
		Mutex _mutex;

		ThreadSet m_activeThreads;/**< ���ڹ������̶߳��� */
		ThreadSet m_freeThreads;  /**< ���е��̶߳��У�ʹ�õ�ʱ����Ҫ����Resume() */

	public:
		CThreadPool();
		~CThreadPool()
		{
			m_activeThreads.clear();
			m_freeThreads.clear();
		}
		/** @brief ÿ�����ӵ���һ�� */ 
		void IntegrityCheck();

		/** @brief  ��ʼ���� */ 
		void Startup();

		/** @brief ж�����е��߳� */ 
		void Shutdown();
		/** @brief �˳��߳�
		// return true - suspend ourselves, and wait for a future task.
		// return false - exit, we're shutting down or no longer needed. */ 
		bool ThreadExit(Thread* t);

		/** @brief �����߳� */ 
		Thread* StartThread(ThreadBase* ExecutionTarget);

		/** @brief ����һ���߳̿�ʼһ������ */ 
		void ExecuteTask(ThreadBase* ExecutionTarget);

		/** @brief  stats��ӡ������Ϣ */ 
		void ShowStats();

		/** @brief �ر�ָ�������Ŀ����߳� */ 
		void KillFreeThreads(uint32 count);

		/** @brief resets the gobble counter */ 
		MNET_INLINE void Gobble() { _threadsEaten = (int32)m_freeThreads.size(); }

		/** @brief �õ���̵߳����� */ 
		MNET_INLINE uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }

		/** @brief �õ������̵߳����� */ 
		MNET_INLINE uint32 GetFreeThreadCount() { return (uint32)m_freeThreads.size(); }
};

extern SERVER_DECL CThreadPool ThreadPool;

#endif
