#ifndef _MOL_THREAD_POOL_H_INCLUDE
#define _MOL_THREAD_POOL_H_INCLUDE

/** 
* MolNet��������
*
* ����:�̳߳�
* ����:akinggw
* ����:2010.2.11
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
	/// ���캯��
	CThreadPool();
	
	/// ÿ�����ӵ���һ��
	void IntegrityCheck();
	/// ��ʼ����
	void Startup();
	/// ж�����е��߳�
	void Shutdown();
	/// �˳��߳�
	bool ThreadExit(Thread * t);
	/// �����߳�
	Thread * StartThread(ThreadBase * ExecutionTarget);
	/// ����һ���߳̿�ʼһ������
	void ExecuteTask(ThreadBase * ExecutionTarget);
	/// ��ӡ������Ϣ
	void ShutStats();
	/// �ر�ָ���������߳�
	void KillFreeThreads(uint32 count);

	inline void Gobble() { _threadsEaten = (int)m_freeThreads.size(); }
	/// �õ���̵߳�����
	inline uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }
	/// �õ������̵߳�����
	inline uint32 GetFreeThreadCount() { return (uint32)m_freeThreads.size(); }
private:
	/// �õ�CPU�ĸ���
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
