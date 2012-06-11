#ifndef _INC_THREADLIB_
#define _INC_THREADLIB_

#include <baseLib/platForm.h>

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif //HAVE_LIBPTHREAD

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#ifdef WIN32
typedef HANDLE HTHREAD;
typedef HANDLE HMUTEX;
#define TLDeclThread(name,param)  DWORD WINAPI name(PVOID param)
#endif //WIN32

#ifndef INFINITE
#define INFINITE -1
#endif //INFINITE

#ifdef HAVE_LIBPTHREAD
typedef pthread_t HTHREAD;
typedef pthread_mutex_t *HMUTEX;

typedef PVOID (*LPTHREAD_START_ROUTINE)(PVOID pvParam);
#define TLDeclThread(name,param)  PVOID name(PVOID param)
#endif //HAVE_LIBPTHREAD

#ifdef HAVE_PTHREAD_INIT
#define TLStartup()  pthread_init()
#else //!HAVE_PTHREAD_INIT
#define TLStartup()
#endif //HAVE_PTHREAD_INIT

BOOL TLCreateThread(HTHREAD *phThred,LPTHREAD_START_ROUTINE ThreadProc,PVOID pvParam);

void TLWaitThread(HTHREAD hThread,PDWORD pdwCode);

#ifdef WIN32
#define TLEndCurrThread(retCode) \
  ExitThread((DWORD)retCode); \
  return (DWORD)retCode;
#endif //WIN32

#ifdef HAVE_LIBPTHREAD
#define TLEndCurrThread(retCode) { pthread_exit((PVOID)retCode); return (PVOID)retCode; }
#endif //HAVE_LIBPTHREAD

#ifdef WIN32
#define TLCreateMutex()						CreateMutex(NULL,FALSE,NULL)
#define TLDestroyMutex(hMutex)				CloseHandle(hMutex)
#define TLTryLockMutex(hMutex)				WaitForSingleObject(hMutex,0)
#define TLLockMutex(hMutex)					WaitForSingleObject(hMutex,INFINITE)
#define TLUnlockMutex(hMutex)				ReleaseMutex(hMutex)
#endif //WIN32

#ifdef HAVE_LIBPTHREAD
typedef pthread_mutex_t CRITICAL_SECTION,*LPCRITICAL_SECTION;

#define InitializeCriticalSection(pCS) pthread_mutex_init(pCS,NULL)
#define DeleteCriticalSection(pCS)     pthread_mutex_destroy(pCS)
#define EnterCriticalSection(pCS)      pthread_mutex_lock(pCS)
#define LeaveCriticalSection(pCS)      pthread_mutex_unlock(pCS)

HMUTEX TLCreateMutex(void);
BOOL TLDestroyMutex(HMUTEX hMutex);
BOOL TLTryLockMutex(HMUTEX hMutex);
BOOL TLLockMutex(HMUTEX hMutex);
BOOL TLUnlockMutex(HMUTEX hMutex);
#endif //HAVE_LIBPTHREAD

#ifdef WIN32
BOOL TLInvokeAndWaitMultiThread(int needThread,LPTHREAD_START_ROUTINE ThreadProc,PVOID pvParam);
#endif //WIN32

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_THREADLIB_
