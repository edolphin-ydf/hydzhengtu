//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      hyd
///  Create:      2012/10/17
///
/// @file         ThreadPool.cpp
/// @brief        �̳߳�
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

#include "ThreadPool.h"
#include "../Log.h"

#ifdef WIN32
#include <process.h>
#else
volatile int threadid_count = 0;
int GenerateThreadId()
{
	int i = ++threadid_count;
	return i;
}
#endif

#define THREAD_RESERVE 10//Ԥ�����߳���

SERVER_DECL CThreadPool ThreadPool;

CThreadPool::CThreadPool()
{
	_threadsExitedSinceLastCheck = 0;
	_threadsRequestedSinceLastCheck = 0;
	_threadsEaten = 0;
	_threadsFreedSinceLastCheck = 0;
}

/** @brief �̴߳ӵ�ǰ״̬�˳� */
bool CThreadPool::ThreadExit(Thread* t)
{
	_mutex.Acquire();

	/**< �Ӽ���Ķ����Ƴ�  */
	m_activeThreads.erase(t);
	++_threadsExitedSinceLastCheck;
	/** @brief �ǲ��ǻ��д�ɱ�����߳���Ŀ */
	if(_threadsToExit > 0)
	{
		// kill us.
		if(t->DeleteAfterExit)
		{
			--_threadsToExit;
			Log.Debug("ThreadPool", "Thread %u exit the free pool.", t->ControlInterface.GetId());
			m_freeThreads.erase(t);
			Macro_Delete(t);
		}

		_mutex.Release();
		return false;
	}

	// enter the "suspended" pool
	++_threadsEaten;
	ThreadSet::iterator itr = m_freeThreads.find(t);

	if(itr != m_freeThreads.end())
	{
		//sLog.outError("Thread %u duplicated with thread %u", (*itr)->ControlInterface.GetId(), t->ControlInterface.GetId());
	}
	else
	{
		++_threadsFreedSinceLastCheck;
		m_freeThreads.insert(t);
		Log.Debug("ThreadPool", "Thread %u entered the free pool.", t->ControlInterface.GetId());
	}
	
	_mutex.Release();
	return true;
}

/** @brief �߳�ִ������*/
void CThreadPool::ExecuteTask(ThreadBase* ExecutionTarget)
{
	if (ExecutionTarget == NULL)
	{
		Log.Error("ThreadPool", "ExecuteTask ExecutionTarget==NULL");
		return;
	}
	
	Thread* t;
	_mutex.Acquire();
	++_threadsRequestedSinceLastCheck;
	--_threadsEaten;

	// ����̳߳��л��п����߳���ȡ����һ��ʹ��
	if(m_freeThreads.size())
	{
		t = *m_freeThreads.begin();
		m_freeThreads.erase(m_freeThreads.begin());

		// execute the task on this thread.
		t->ExecutionTarget = ExecutionTarget;

		// resume the thread, and it should start working.
		t->ControlInterface.Resume();
		Log.Debug("ThreadPool", "Thread %u left the thread pool.", t->ControlInterface.GetId());
	}
	else//û�п����߳��򴴽�һ���µ��߳�
	{
		// creating a new thread means it heads straight to its task.
		// no need to resume it :)
		t = StartThread(ExecutionTarget);
	}

	// add the thread to the active set
#ifdef WIN32
	Log.Debug("ThreadPool", "Thread %u is now executing task at 0x%p.", t->ControlInterface.GetId(), ExecutionTarget);
#else
	Log.Debug("ThreadPool", "Thread %u is now executing task at %p.", t->ControlInterface.GetId(), ExecutionTarget);
#endif
	m_activeThreads.insert(t);
	_mutex.Release();
}

void CThreadPool::Startup()
{
	int i;
	int tcount = THREAD_RESERVE;//Ԥ���������߳���

	for(i = 0; i < tcount; ++i)
		StartThread(NULL);

	Log.Debug("ThreadPool", "�̳߳�����, ���� %u ���߳�.", tcount);
}

void CThreadPool::ShowStats()
{
	_mutex.Acquire();
	Log.Debug("ThreadPool", "============ ThreadPool Status =============");
	Log.Debug("ThreadPool", "Active Threads: %u", m_activeThreads.size());
	Log.Debug("ThreadPool", "Suspended Threads: %u", m_freeThreads.size());
	Log.Debug("ThreadPool", "Requested-To-Freed Ratio: %.3f%% (%u/%u)", float(float(_threadsRequestedSinceLastCheck + 1) / float(_threadsExitedSinceLastCheck + 1) * 100.0f), _threadsRequestedSinceLastCheck, _threadsExitedSinceLastCheck);
	Log.Debug("ThreadPool", "Eaten Count: %d (negative is bad!)", _threadsEaten);
	Log.Debug("ThreadPool", "============================================");
	_mutex.Release();
}
// �̳߳��߳������
void CThreadPool::IntegrityCheck()
{
	_mutex.Acquire();
	int32 gobbled = _threadsEaten;

	if(gobbled < 0)
	{
		// ����ζ��������Ҫ������߳�
		uint32 new_threads = abs(gobbled) + THREAD_RESERVE;
		_threadsEaten = 0;

		for(uint32 i = 0; i < new_threads; ++i)
			StartThread(NULL);

		Log.Debug("ThreadPool", "IntegrityCheck: (gobbled < 0) Spawning %u threads.", new_threads);
	}
	else if(gobbled < THREAD_RESERVE)
	{
		// ����ζ������û�кľ��̣߳�����Ҫ�����߳���
		uint32 new_threads = (THREAD_RESERVE - gobbled);
		for(uint32 i = 0; i < new_threads; ++i)
			StartThread(NULL);

		Log.Debug("ThreadPool", "IntegrityCheck: (gobbled <= 5) Spawning %u threads.", new_threads);
	}
	else if(gobbled > THREAD_RESERVE)
	{
		//����Ҫ���߳�̫�࣬����һЩû��Ҫ��
		uint32 kill_count = (gobbled - THREAD_RESERVE);
		KillFreeThreads(kill_count);
		_threadsEaten -= kill_count;
		Log.Debug("ThreadPool", "IntegrityCheck: (gobbled > 5) Killing %u threads.", kill_count);
	}
	else
	{
		// ����״̬��
		Log.Debug("ThreadPool", "IntegrityCheck: Perfect!");
	}

	_threadsExitedSinceLastCheck = 0;
	_threadsRequestedSinceLastCheck = 0;
	_threadsFreedSinceLastCheck = 0;

	_mutex.Release();
}

void CThreadPool::KillFreeThreads(uint32 count)
{
	Log.Debug("ThreadPool", "Killing %u excess threads.", count);
	_mutex.Acquire();
	Thread* t;
	ThreadSet::iterator itr;
	uint32 i;
	for(i = 0, itr = m_freeThreads.begin(); i < count && itr != m_freeThreads.end(); ++i, ++itr)
	{
		t = *itr;
		t->ExecutionTarget = NULL;
		t->DeleteAfterExit = true;
		++_threadsToExit;/**< �˳��߳�+1  */
		t->ControlInterface.Resume();/**< ˯���ָ̻߳�����  */
	}

	_mutex.Release();
}

void CThreadPool::Shutdown()
{
	_mutex.Acquire();
	size_t tcount = m_activeThreads.size() + m_freeThreads.size();		// exit all
	Log.Debug("ThreadPool", "���������̣߳� %u ��.", tcount);
	KillFreeThreads((uint32)m_freeThreads.size());
	_threadsToExit += (uint32)m_activeThreads.size();

	for(ThreadSet::iterator itr = m_activeThreads.begin(); itr != m_activeThreads.end(); ++itr)
	{
		Thread* t = *itr;
		t->DeleteAfterExit = true;
		if(t->ExecutionTarget)
			t->ExecutionTarget->OnShutdown();
		else
			t->ControlInterface.Resume();
	}
	_mutex.Release();

	for(int i = 0;; i++)
	{
		_mutex.Acquire();
		// ѭ���ȴ������߳��˳�
		if(m_activeThreads.size() || m_freeThreads.size() || _threadsToExit>0)
		{
			if(i != 0 && m_freeThreads.size() != 0)
			{
				/*if we are here then a thread in the free pool checked if it was being shut down just before CThreadPool::Shutdown() was called,
				but called Suspend() just after KillFreeThreads(). All we need to do is to resume it.*/
				Thread* t;
				ThreadSet::iterator itr;
				for(itr = m_freeThreads.begin(); itr != m_freeThreads.end(); ++itr)
				{
					t = *itr;
					t->DeleteAfterExit = true;
					t->ControlInterface.Resume();
				}
			}
			Log.Debug("ThreadPool", "%u ������״̬ and %u ������״̬�߳� remaining...", m_activeThreads.size(), m_freeThreads.size());
			_mutex.Release();
			MNet::Sleep(1000);
			continue;
		}

		_mutex.Release();
		break;
	}
}

/* this is the only platform-specific code. neat, huh! */
#ifdef WIN32

static unsigned long WINAPI thread_proc(void* param)
{
	Thread* t = (Thread*)param;
	t->SetupMutex.Acquire();
	uint32 tid = t->ControlInterface.GetId();
	bool ht = (t->ExecutionTarget != NULL);
	t->SetupMutex.Release();
	Log.Debug("ThreadPool", "Thread %u started.", t->ControlInterface.GetId());

	for(;;)
	{
		if(t->ExecutionTarget != NULL)
		{
			if(t->ExecutionTarget->run())
				delete t->ExecutionTarget;

			t->ExecutionTarget = NULL;
		}

		if(!ThreadPool.ThreadExit(t))
		{
			Log.Debug("ThreadPool", "�߳� %u �˳�.", tid);
			break;
		}
		else
		{
			if(ht)
				Log.Debug("ThreadPool", "Thread %u waiting for a new task.", tid);
			// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
			t->ControlInterface.Suspend();
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.
		}
	}

	// at this point the t pointer has already been freed, so we can just cleanly exit.
	ExitThread(0);
}

/**
* @brief ����һ�����߳�
* @param �߳�ִ��Ŀ��
* @return �����߳���Ϣ�ṹ��
*/
Thread* CThreadPool::StartThread(ThreadBase* ExecutionTarget)
{
	HANDLE h;
	Thread* t = NULL;
	Macro_NewClass(t,Thread);

	t->DeleteAfterExit = false;
	t->ExecutionTarget = ExecutionTarget;
	if (ExecutionTarget == NULL)
	{
		m_freeThreads.insert(t);
	}
	//h = (HANDLE)_beginthreadex(NULL, 0, &thread_proc, (void*)t, 0, NULL);
	t->SetupMutex.Acquire();
	h = CreateThread(NULL, 0, &thread_proc, (LPVOID)t, 0, (LPDWORD)&t->ControlInterface.thread_id);
	t->ControlInterface.Setup(h);
	t->SetupMutex.Release();

	return t;
}

#else

static void* thread_proc(void* param)
{
	Thread* t = (Thread*)param;
	t->SetupMutex.Acquire();
	Log.Debug("ThreadPool", "Thread %u started.", t->ControlInterface.GetId());
	t->SetupMutex.Release();

	for(;;)
	{
		if(t->ExecutionTarget != NULL)
		{
			if(t->ExecutionTarget->run())
				delete t->ExecutionTarget;

			t->ExecutionTarget = NULL;
		}

		if(!ThreadPool.ThreadExit(t))
			break;
		else
		{
			// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
			t->ControlInterface.Suspend();
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.
		}
	}

	pthread_exit(0);
}

Thread* CThreadPool::StartThread(ThreadBase* ExecutionTarget)
{
	pthread_t target;
	Thread* t = NULL;
	Macro_NewClass(t,Thread);
	t->ExecutionTarget = ExecutionTarget;
	if (ExecutionTarget == NULL)
	{
		m_freeThreads.insert(t);
	}
	t->DeleteAfterExit = false;

	// lock the main mutex, to make sure id generation doesn't get messed up
	_mutex.Acquire();
	t->SetupMutex.Acquire();
	pthread_create(&target, NULL, &thread_proc, (void*)t);
	t->ControlInterface.Setup(target);
	pthread_detach(target);
	t->SetupMutex.Release();
	_mutex.Release();
	return t;
}

#endif
