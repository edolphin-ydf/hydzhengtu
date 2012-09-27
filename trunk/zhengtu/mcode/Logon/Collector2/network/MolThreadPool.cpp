#include "stdafx.h"
#include "MolThreadPool.h"

CThreadPool ThreadPool;

/** 
 * 构造函数
 */
CThreadPool::CThreadPool()
{
	_threadsExitedSinceLastCheck = 0;
	_threadsRequestedSinceLastCheck = 0;
	_threadsEaten = 0;
	_threadsFreedSinceLastCheck = 0;
}

/** 
 * 退出线程
 *
 * @param t 要退出的线程
 *
 * @return
 */
bool CThreadPool::ThreadExit(Thread * t)
{
	_mutex.Acquire();

	// we're definitely no longer active
	m_activeThreads.erase(t);

	// do we have to kill off some threads?
	if(_threadsToExit > 0)
	{
		// kill us.
		--_threadsToExit;
		++_threadsExitedSinceLastCheck;
		if(t->DeleteAfterExit)
			m_freeThreads.erase(t);

		_mutex.Release();		
		delete t;
		return false;
	}

	// enter the "suspended" pool
	++_threadsExitedSinceLastCheck;
	++_threadsEaten;
	std::set<Thread*>::iterator itr = m_freeThreads.find(t);

	if(itr != m_freeThreads.end())
	{
		TRACE("Thread %u duplicated with thread %u\n", (*itr)->ControlInterface.GetId(), t->ControlInterface.GetId());
	}
	m_freeThreads.insert(t);

	TRACE("Thread %u entered the free pool.", t->ControlInterface.GetId());
	_mutex.Release();
	return true;
}

/** 
 * 分配一个线程开始一个任务
 *
 * @param ExecutionTarget 要执行的任务
 */
void CThreadPool::ExecuteTask(ThreadBase * ExecutionTarget)
{
	Thread * t;
	_mutex.Acquire();
	++_threadsRequestedSinceLastCheck;
	--_threadsEaten;

	// grab one from the pool, if we have any.
	if(m_freeThreads.size())
	{
		t = *m_freeThreads.begin();
		m_freeThreads.erase(m_freeThreads.begin());

		// execute the task on this thread.
		t->ExecutionTarget = ExecutionTarget;

		// resume the thread, and it should start working.
		t->ControlInterface.Resume();
		TRACE("Thread %u left the thread pool.", t->ControlInterface.GetId());
	}
	else
	{
		// creating a new thread means it heads straight to its task.
		// no need to resume it :)
		t = StartThread(ExecutionTarget);
	}

	// add the thread to the active set
	TRACE("Thread %u is now executing task at 0x%p.", t->ControlInterface.GetId(), ExecutionTarget);

	m_activeThreads.insert(t);
	_mutex.Release();
}

/** 
 * 开始调用
 */
void CThreadPool::Startup()
{
	int i;
	int tcount = THREAD_RESERVE;

	for(i=0; i < tcount; ++i)
		StartThread(NULL);

	TRACE("Startup, launched %u threads.", tcount);
}

/** 
 * 打印调试消息
 */
void CThreadPool::ShutStats()
{
	_mutex.Acquire();
	TRACE("============ ThreadPool Status =============");
	TRACE("Active Threads: %u", m_activeThreads.size());
	TRACE("Suspended Threads: %u", m_freeThreads.size());
	TRACE("Requested-To-Freed Ratio: %.3f%% (%u/%u)", float( float(_threadsRequestedSinceLastCheck+1) / float(_threadsExitedSinceLastCheck+1) * 100.0f ), _threadsRequestedSinceLastCheck, _threadsExitedSinceLastCheck);
	TRACE("Eaten Count: %d (negative is bad!)", _threadsEaten);
	TRACE("============================================");
	_mutex.Release();
}

/** 
 * 每两分钟调用一次
 */
void CThreadPool::IntegrityCheck()
{
	_mutex.Acquire();
	int32 gobbled = _threadsEaten;

	if(gobbled < 0)
	{
		// this means we requested more threads than we had in the pool last time.
		// spawn "gobbled" + THREAD_RESERVE extra threads.
		uint32 new_threads = abs(gobbled) + THREAD_RESERVE;
		_threadsEaten=0;

		for(uint32 i = 0; i < new_threads; ++i)
			StartThread(NULL);

		TRACE("IntegrityCheck: (gobbled < 0) Spawning %u threads.", new_threads);
	}
	else if(gobbled < THREAD_RESERVE)
	{
		// this means while we didn't run out of threads, we were getting damn low.
		// spawn enough threads to keep the reserve amount up.
		uint32 new_threads = (THREAD_RESERVE - gobbled);
		for(uint32 i = 0; i < new_threads; ++i)
			StartThread(NULL);

		TRACE("IntegrityCheck: (gobbled <= 5) Spawning %u threads.", new_threads);
	}
	else if(gobbled > THREAD_RESERVE)
	{
		// this means we had "excess" threads sitting around doing nothing.
		// lets kill some of them off.
		uint32 kill_count = (gobbled - THREAD_RESERVE);
		KillFreeThreads(kill_count);
		_threadsEaten -= kill_count;
		TRACE("IntegrityCheck: (gobbled > 5) Killing %u threads.", kill_count);
	}
	else
	{
		// perfect! we have the ideal number of free threads.
		TRACE("IntegrityCheck: Perfect!");
	}
	/*if(m_freeThreads.size() < 5)
	{
	uint32 j = 5 - m_freeThreads.size();
	Log.Debug("ThreadPool", "Spawning %u threads.", j);
	for(uint32 i = 0; i < j; ++i)
	StartThread(NULL);
	}*/

	_threadsExitedSinceLastCheck = 0;
	_threadsRequestedSinceLastCheck = 0;
	_threadsFreedSinceLastCheck = 0;

	_mutex.Release();
}

/** 
 * 关闭指定数量的线程
 *
 * @param count 要关闭的线程的数量
 */
void CThreadPool::KillFreeThreads(uint32 count)
{
	TRACE("Killing %u excess threads.", count);
	_mutex.Acquire();
	Thread * t;
	ThreadSet::iterator itr;
	uint32 i;
	for(i = 0, itr = m_freeThreads.begin(); i < count && itr != m_freeThreads.end(); ++i, ++itr)
	{
		t = *itr;
		t->ExecutionTarget = NULL; 
		t->DeleteAfterExit = true;
		++_threadsToExit;
		t->ControlInterface.Resume();
	}
	_mutex.Release();
}

/** 
 * 卸载所有的线程
 */
void CThreadPool::Shutdown()
{
	_mutex.Acquire();
	size_t tcount = m_activeThreads.size() + m_freeThreads.size();		// exit all
//	Log.Debug("ThreadPool", "Shutting down %u threads.", tcount);
	KillFreeThreads((uint32)m_freeThreads.size());
	_threadsToExit += (uint32)m_activeThreads.size();

    for( std::set< Thread* >::iterator itr = m_activeThreads.begin(); itr != m_activeThreads.end(); ++itr)
	{

        Thread *t = *itr;

		if(t == NULL) continue;

		if( t->ExecutionTarget )
			t->ExecutionTarget->OnShutdown();
        else         
            t->ControlInterface.Resume();
	}
	_mutex.Release();

	int i = 0;
	for(;i<3; i++)
	{
		_mutex.Acquire();
		if(m_activeThreads.size() || m_freeThreads.size())
		{
			if( i != 0 && m_freeThreads.size() != 0 )
			{
				/*if we are here then a thread in the free pool checked if it was being shut down just before CThreadPool::Shutdown() was called,
				but called Suspend() just after KillFreeThreads(). All we need to do is to resume it.*/
				Thread * t;
				ThreadSet::iterator itr;
				for(itr = m_freeThreads.begin(); itr != m_freeThreads.end(); ++itr)
				{
					t = *itr;
					t->ControlInterface.Resume();
				}
			}
			//Log.Debug("ThreadPool", "%u active and %u free threads remaining...",m_activeThreads.size(), m_freeThreads.size() );
			_mutex.Release();
			Sleep(1000);
			continue;
		}

		_mutex.Release();
		break;
	}

	if(i>=3)
	{
		_mutex.Acquire();
		while(!m_activeThreads.empty())
		{
			Thread * t = *(m_activeThreads.begin());
			t->ControlInterface.Resume();
			if(t->ExecutionTarget) 
				delete t->ExecutionTarget;
			//CloseHandle(t->ControlInterface.hThread);
			t->ExecutionTarget = NULL;
			
			delete t;
			t = NULL;
			m_activeThreads.erase(m_activeThreads.begin());
		}
		_mutex.Release();
	}
}

static unsigned long WINAPI thread_proc(void* param)
{
	Thread * t = (Thread*)param;
	t->SetupMutex.Acquire();
	uint32 tid = t->ControlInterface.GetId();
	bool ht = (t->ExecutionTarget != NULL);
	t->SetupMutex.Release();
	//Log.Debug("ThreadPool", "Thread %u started.", t->ControlInterface.GetId());

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
			TRACE("Thread %u exiting.", tid);
			break;
		}
		else
		{
			if(ht)
				TRACE("Thread %u waiting for a new task.", tid);
			// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
			t->ControlInterface.Suspend();
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.
		}
	}

	// at this point the t pointer has already been freed, so we can just cleanly exit.
	ExitThread(0);
}

/** 
 * 建立线程
 *
 * @param ExecutionTarget 线程执行的任务
 *
 * @return 如果线程建立成功返回这个线程，否则返回NULL
 */
Thread * CThreadPool::StartThread(ThreadBase * ExecutionTarget)
{
	HANDLE h;
	Thread * t = new Thread;

	t->DeleteAfterExit = false;
	t->ExecutionTarget = ExecutionTarget;
	//h = (HANDLE)_beginthreadex(NULL, 0, &thread_proc, (void*)t, 0, NULL);
	t->SetupMutex.Acquire();
	h = CreateThread(NULL, 0, &thread_proc, (LPVOID)t, 0, (LPDWORD)&t->ControlInterface.thread_id);
	Sleep(1);
	t->ControlInterface.Setup(h);
	t->SetupMutex.Release();

	return t;
}
