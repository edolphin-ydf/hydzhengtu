#include <baseLib/logLib.h>
#include <baseLib/threadLib.h>

BOOL TLCreateThread(HTHREAD *phThread,LPTHREAD_START_ROUTINE ThreadProc,PVOID pvParam)
{
#ifdef WIN32
  DWORD  dwThread;
  HANDLE hThread;

  if (NULL == (hThread=CreateThread(NULL,0,ThreadProc,pvParam,0,&dwThread))){
    logError0("!CreateThread");
	return FALSE;
  }
#endif //WIN32
#ifdef HAVE_LIBPTHREAD
  int       errNo;
  pthread_t hThread;

  if (0 != (errNo=pthread_create(&hThread,NULL,ThreadProc,pvParam))){
    logMessage1("!pthread_create %d",errNo);
	return FALSE;
  }
#endif //HAVE_LIBPTHREAD
  if (NULL != phThread) *phThread = hThread;
  else{
#ifdef WIN32
    CloseHandle(hThread);
#endif //WIN32
#ifdef HAVE_LIBPTHREAD
    pthread_detach(hThread);
#endif //HAVE_LIBPTHREAD
  }
  return TRUE;
}

void TLWaitThread(HTHREAD hThread,PDWORD pdwCode)
{
#ifdef WIN32
  WaitForSingleObject(hThread,INFINITE);
  if (NULL!=pdwCode) GetExitCodeThread(hThread,pdwCode);
  CloseHandle(hThread);
#endif //WIN32
#ifdef HAVE_LIBPTHREAD
  int errNo;

  if (0 != (errNo=pthread_join(hThread,(PVOID*)pdwCode))){
    logMessage1("!pthread_join %d",errNo);
  }
#endif //HAVE_LIBPTHREAD
}

#ifndef MAXIMUM_WAIT_OBJECTS
#define MAXIMUM_WAIT_OBJECTS   32
#endif //MAXIMUM_WAIT_OBJECTS

#ifdef HAVE_LIBPTHREAD
HMUTEX TLCreateMutex(void)
{
  int                 errNo;
  HMUTEX              hMutex;
  pthread_mutexattr_t attr;

  if (NULL == (hMutex=(HMUTEX)MemCalloc(1,sizeof(pthread_mutex_t)))) return NULL;
  if (0 != (errNo=pthread_mutexattr_init(&attr))){
    logMessage1("!pthread_mutexattr_init %d",errNo);
    __API_FINISH();
  }
#ifdef PTHREAD_MUTEX_RECURSIVE_NP
  //WIN32 mutexes are recursive, so match that behavior.
  if (0 != (errNo=pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP))){
    logMessage1("!pthread_mutexattr_settype %d",errNo);
    pthread_mutexattr_destroy(&attr);
    __API_FINISH();
  }
#endif //PTHREAD_MUTEX_RECURSIVE_NP
  if (0 != (errNo=pthread_mutex_init(hMutex,&attr))){
    logMessage1("!pthread_mutex_init %d",errNo);
  }
  pthread_mutexattr_destroy(&attr);
__API_END_POINT:
  if (0 != errNo){
    MemFree(hMutex);
	hMutex = NULL;
  }
  return hMutex;
}

BOOL TLDestroyMutex(HMUTEX hMutex)
{
  int errNo;

  if (NULL == hMutex) return FALSE;
  if (0 != (errNo=pthread_mutex_destroy(hMutex))) {
    logMessage1("!pthread_mutex_destroy %d",errNo);
  }
  MemFree(hMutex);
  return 0 == errNo;
}

BOOL TLLockMutex(HMUTEX hMutex)
{
  int errNo;

  if (NULL == hMutex) return FALSE;
  if (0 == (errNo=pthread_mutex_lock(hMutex))) return TRUE;
  logMessage1("!pthread_mutex_lock %d",errNo);
  return FALSE;
}

BOOL TLTryLockMutex(HMUTEX hMutex)
{
  int errNo;

  if (NULL == hMutex) return FALSE;
  if (0 == (errNo=pthread_mutex_trylock(hMutex))) return TRUE;
  logMessage1("!pthread_mutex_trylock %d",errNo);
  return FALSE;
}

BOOL TLUnlockMutex(HMUTEX hMutex)
{
  int errNo;

  if (NULL == hMutex) return FALSE;
  if (0 == (errNo=pthread_mutex_unlock(hMutex))) return TRUE;
  logMessage1("!pthread_mutex_unlock %d",errNo);
  return FALSE;
}
#endif //HAVE_LIBPTHREAD

#ifdef WIN32
BOOL TLInvokeAndWaitMultiThread(int needThread,LPTHREAD_START_ROUTINE ThreadProc,PVOID pvParam)
{
  DWORD   i,numThread;
  HTHREAD hThreads[MAXIMUM_WAIT_OBJECTS];

  numThread = min(needThread,SIZEOF_ARRAY(hThreads));
  for(i=0;i<numThread;i++){
    if (!TLCreateThread(&hThreads[i],ThreadProc,pvParam)) break;
  }
  numThread = i;
  if (0 != numThread){
    WaitForMultipleObjects(numThread,hThreads,TRUE,INFINITE);
	for(i=0;i<numThread;i++){
      CloseHandle(hThreads[i]);
    }
    for(i=0;i<numThread;i++){
      TLWaitThread(hThreads[i],NULL);
	}
  }
  return 0 != numThread;
}
#endif //WIN32
