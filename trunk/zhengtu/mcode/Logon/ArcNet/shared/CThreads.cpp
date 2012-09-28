

// Class CThread - Base class for all threads in the
// server, and allows for easy management by ThreadMgr.

#include "Common.h"
#include "CThreads.h"

CThread::CThread(std::string name) : ThreadBase(name)
{
	ThreadState.SetVal(THREADSTATE_AWAITING);
	start_time  = 0;
}

CThread::~CThread()
{

}

bool CThread::run()
{
	return false;
}

void CThread::OnShutdown()
{
	SetThreadState(THREADSTATE_TERMINATE);
}

