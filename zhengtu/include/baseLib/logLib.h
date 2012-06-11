#ifndef _INC_LOGLIB_H_
#define _INC_LOGLIB_H_

#include <baseLib/platForm.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#ifdef WIN32

typedef HANDLE (WINAPI *PFN_CreateFileW)(LPCWSTR a0,
                               DWORD a1,
                               DWORD a2,
                               LPSECURITY_ATTRIBUTES a3,
                               DWORD a4,
                               DWORD a5,
                               HANDLE a6);

typedef DWORD (WINAPI *PFN_GetFileSize)(
  HANDLE hFile,           // handle to file
  LPDWORD lpFileSizeHigh  // high-order word of file size
);

typedef DWORD (WINAPI *PFN_SetFilePointer)(
  HANDLE hFile,
  LONG lDistanceToMove,
  PLONG lpDistanceToMoveHigh,
  DWORD dwMoveMethod
);

typedef BOOL (WINAPI *PFN_SetEndOfFile)(
  HANDLE hFile
);

typedef BOOL (WINAPI *PFN_WriteFile)(HANDLE hFile,
                           LPCVOID lpBuffer,
                           DWORD nNumberOfBytesToWrite,
                           LPDWORD lpNumberOfBytesWritten,
                           LPOVERLAPPED lpOverlapped);

typedef BOOL (WINAPI *PFN_CloseHandle)(HANDLE hObject);

typedef DWORD (WINAPI *PFN_WaitForSingleObjectEx)(
  HANDLE hHandle,        // handle to object
  DWORD dwMilliseconds,  // time-out interval,
  BOOL bAlertable        // alertable option
);

typedef BOOL (WINAPI *PFN_ReleaseMutex)(
  HANDLE hMutex   // handle to mutex
);

typedef struct{
 PFN_CreateFileW           CreateFileW;
 PFN_WriteFile             WriteFile;
 PFN_GetFileSize	       GetFileSize;
 PFN_SetFilePointer        SetFilePointer;
 PFN_SetEndOfFile          SetEndOfFile;
 PFN_CloseHandle           CloseHandle;
 PFN_WaitForSingleObjectEx WaitForSingleObjectEx;
 PFN_ReleaseMutex          ReleaseMutex;
}NLogDetours,*PLogDetours;

#endif //!WIN32

#define LOGFMT_NONE		0x0000
#define LOGFMT_APP		0x0001
#define LOGFMT_DATE		0x0002
#define LOGFMT_TIME		0x0004
#define LOGFMT_PATH		0x0008
#define LOGFMT_NAME		0x0010
#define LOGFMT_THREAD	0x0020

#define LOGFMT_ALL		(LOGFMT_TIME|LOGFMT_PATH|LOGFMT_NAME|LOGFMT_THREAD|LOGFMT_APP)

void logInit(PSTR szApp,PSTR szLog,DWORD dwLimits,PSTR szServer,short sPort);
void logTerm(void);

void logFormat(DWORD dwFormat);

#ifdef WIN32
void logEnter(void);
void logLeave(void);

DWORD logLevel(void);

PLogDetours logDetours(void);

typedef void (*PFN_logEnter)(void);
typedef void (*PFN_logLeave)(void);

typedef DWORD (*PFN_logLevel)(void);

typedef struct{
  PFN_logEnter logEnter;
  PFN_logLeave logLeave;
  PFN_logLevel logLevel;
}NLogIndent,*PLogIndent;

PLogIndent logGetIndent(void);

void odprintf(PSTR szFMT,...);

#else //!WIN32
#define logEnter()
#define logLeave()
#define logLevel() 0
#endif //!WIN32

void logMessage(PSTR szFMT,...);
void logError_FL(const char *szFile,int nLine,PSTR szFMT,...);
void logMessage_FL(const char *szFile,int nLine,PSTR szFMT,...);
void logCode_FL(const char *szFile,int nLine,int nError,PSTR szFMT,...);

void logMessage_VP(PSTR szFMT,va_list vp);
void logError_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp);
void logMessage_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp);
void logCode_FL_VP(const char *szFile,int nLine,int nError,PSTR szFMT,va_list vp);

#ifdef WIN32
#define LOG_INIT(sApp)			logInit(sApp,"c:\\ldsec.log",0,NULL,0)
#else //!WIN32
#define LOG_INIT(sApp)			logInit(sApp,"/var/tmp/ldsec.log",0,NULL,0)
#endif //!WIN32

#define logCode0(code,msg)									logCode_FL(__FILE__,__LINE__,code,msg)
#define logCode1(code,msg,a0)								logCode_FL(__FILE__,__LINE__,code,msg,a0)
#define logCode2(code,msg,a0,a1)							logCode_FL(__FILE__,__LINE__,code,msg,a0,a1)
#define logCode3(code,msg,a0,a1,a2)							logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2)
#define logCode4(code,msg,a0,a1,a2,a3)						logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2,a3)
#define logCode5(code,msg,a0,a1,a2,a3,a4)					logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2,a3,a4)
#define logCode6(code,msg,a0,a1,a2,a3,a4,a5)				logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2,a3,a4,a5)
#define logCode7(code,msg,a0,a1,a2,a3,a4,a5,a6)				logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2,a3,a4,a5,a6)
#define logCode8(code,msg,a0,a1,a2,a3,a4,a5,a6,a7)			logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2,a3,a4,a5,a6,a7)
#define logCode9(code,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8)		logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8)
#define logCode10(code,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)	logCode_FL(__FILE__,__LINE__,code,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)

#define logError0(msg)									logError_FL(__FILE__,__LINE__,msg)
#define logError1(msg,a0)								logError_FL(__FILE__,__LINE__,msg,a0)
#define logError2(msg,a0,a1)							logError_FL(__FILE__,__LINE__,msg,a0,a1)
#define logError3(msg,a0,a1,a2)							logError_FL(__FILE__,__LINE__,msg,a0,a1,a2)
#define logError4(msg,a0,a1,a2,a3)						logError_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3)
#define logError5(msg,a0,a1,a2,a3,a4)					logError_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4)
#define logError6(msg,a0,a1,a2,a3,a4,a5)				logError_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5)
#define logError7(msg,a0,a1,a2,a3,a4,a5,a6)				logError_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6)
#define logError8(msg,a0,a1,a2,a3,a4,a5,a6,a7)			logError_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6,a7)
#define logError9(msg,a0,a1,a2,a3,a4,a5,a6,a7,a8)		logError_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8)
#define logError10(msg,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)	logError_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)

#define logMessage0(msg)								logMessage_FL(__FILE__,__LINE__,msg)
#define logMessage1(msg,a0)								logMessage_FL(__FILE__,__LINE__,msg,a0)
#define logMessage2(msg,a0,a1)							logMessage_FL(__FILE__,__LINE__,msg,a0,a1)
#define logMessage3(msg,a0,a1,a2)						logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2)
#define logMessage4(msg,a0,a1,a2,a3)					logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3)
#define logMessage5(msg,a0,a1,a2,a3,a4)					logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4)
#define logMessage6(msg,a0,a1,a2,a3,a4,a5)				logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5)
#define logMessage7(msg,a0,a1,a2,a3,a4,a5,a6)			logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6)
#define logMessage8(msg,a0,a1,a2,a3,a4,a5,a6,a7)		logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6,a7)
#define logMessage9(msg,a0,a1,a2,a3,a4,a5,a6,a7,a8)		logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8)
#define logMessage10(msg,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)	logMessage_FL(__FILE__,__LINE__,msg,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)

#define __API_CHECK(expr)					{ if (0 != (retCode=expr)) goto __API_END_POINT; }
#define __API_CHECK_N(expr)					{ if ((retCode=expr) < 0) goto __API_END_POINT; }
#define __API_CHECK_EC(sMsg,expr)			{ if (0 != (retCode=expr)) { logError_FL(__FILE__,__LINE__,"%s retCode=%d(0x%08x)",sMsg,retCode,retCode); goto __API_END_POINT; }  }
#define __API_CHECK_RC(sMsg,expr)			{ if (0 != (retCode=expr)) { logCode_FL(__FILE__,__LINE__,retCode,"%s",sMsg); goto __API_END_POINT; }  }
#define __API_CHECK_TC(sMsg,pFunc,expr)		{ if (0 != (retCode=expr)) { logMessage_FL(__FILE__,__LINE__,"%s %s",sMsg,pFunc(retCode)); goto __API_END_POINT; }  }

#define __HR_CHECK(expr)					{ if (FAILED((retCode=expr))) goto __API_END_POINT; }
#define __HR_CHECK_EC(sMsg,expr)			{ if (FAILED((retCode=expr))) { logError_FL(__FILE__,__LINE__,"%s retCode=%d(0x%08x)",sMsg,retCode,retCode); goto __API_END_POINT; }  }
#define __HR_CHECK_RC(sMsg,expr)			{ if (FAILED((retCode=expr))) { logCode_FL(__FILE__,__LINE__,retCode,"%s",sMsg); goto __API_END_POINT; }  }
#define __HR_CHECK_TC(sMsg,pFunc,expr)		{ if (FAILED((retCode=expr))) { logMessage_FL(__FILE__,__LINE__,"%s %s",sMsg,pFunc(retCode)); goto __API_END_POINT; }  }

#define __API_CANCEL(code)					{ retCode = code; goto __API_END_POINT; }
#define __API_CANCEL_EC(sMsg,code)			{ retCode = code; { logError_FL(__FILE__,__LINE__,"%s retCode=%d(0x%08x)",sMsg,retCode,retCode); goto __API_END_POINT; } }
#define __API_CANCEL_RC(sMsg,code)			{ retCode = code; { logCode_FL(__FILE__,__LINE__,retCode,"%s",sMsg); goto __API_END_POINT; } }
#define __API_CANCEL_TC(sMsg,pFunc,code)	{ retCode = code; { logMessage_FL(__FILE__,__LINE__,"%s %s",sMsg,pFunc(retCode)); goto __API_END_POINT; } }

#define __API_FINISH()				{ goto __API_END_POINT; }

#define __API_VAL_TRANS(a,b,c)		{ a = b; b = c; };
#define __API_PTR_TRANS(a,b)		{ a = b; b = NULL; };
#define __API_PTR_TRANS_CAST(a,b,t)	{ a = (t)b; b = NULL; };

#ifdef _LOG_FULL_

#define __API_ENTER0(sFunc,type)		type retCode; logMessage_FL(__FILE__,__LINE__,"%s enter",sFunc); logEnter();
#define __API_ENTER(sFunc,type,code)	type retCode=code; logMessage_FL(__FILE__,__LINE__,"%s enter",sFunc); logEnter();
#define __API_ENTER_(sFunc)				logMessage_FL(__FILE__,__LINE__,"%s enter",sFunc); logEnter();
#define __API_LEAVEEX(sFunc,code)		logLeave(); logMessage_FL(__FILE__,__LINE__,"%s leave",sFunc); return code;
#define __API_LEAVE(sFunc)				logLeave(); logMessage_FL(__FILE__,__LINE__,"%s leave,retCode=0x%08x",sFunc,retCode); return retCode;
#define __API_LEAVE_(sFunc)				logLeave(); logMessage_FL(__FILE__,__LINE__,"%s leave",sFunc); return;

#define __WINAPI_ENTER(sFunc,type,code)	type retCode=code; DWORD errCode = GetLastError(); logMessage_FL(__FILE__,__LINE__,"%s enter",sFunc); logEnter();
#define __WINAPI_ENTER_(sFunc)			DWORD errCode = GetLastError(); logMessage_FL(__FILE__,__LINE__,"%s enter",sFunc); logEnter();
#define __WINAPI_LEAVE(sFunc)			logLeave(); logMessage_FL(__FILE__,__LINE__,"%s leave",sFunc); SetLastError(errCode); return retCode;
#define __WINAPI_LEAVE_(sFunc)			logLeave(); logMessage_FL(__FILE__,__LINE__,"%s leave",sFunc); SetLastError(errCode); return;

#define __SOCKAPI_ENTER(sFunc,type,code)	type retCode=code; int errCode = WSAGetLastError(); logMessage_FL(__FILE__,__LINE__,"%s enter",sFunc); logEnter();
#define __SOCKAPI_ENTER_(sFunc)				int errCode = WSAGetLastError(); logMessage_FL(__FILE__,__LINE__,"%s enter",sFunc); logEnter();
#define __SOCKAPI_LEAVE(sFunc)				logLeave(); logMessage_FL(__FILE__,__LINE__,"%s leave",sFunc); WSASetLastError(errCode); return retCode;
#define __SOCKAPI_LEAVE_(sFunc)				logLeave(); logMessage_FL(__FILE__,__LINE__,"%s leave",sFunc); WSASetLastError(errCode); return;

#else //!_LOG_FULL_

#define __API_ENTER0(sFunc,type)		type retCode; logEnter();
#define __API_ENTER(sFunc,type,code)	type retCode=code; logEnter();
#define __API_ENTER_(sFunc)				logEnter();
#define __API_LEAVEEX(sFunc,code)		logLeave(); return code;
#define __API_LEAVE(sFunc)				logLeave(); return retCode;
#define __API_LEAVE_(sFunc)				logLeave(); return;

#define __WINAPI_ENTER(sFunc,type,code)	type retCode=code; DWORD errCode = GetLastError(); logEnter();
#define __WINAPI_ENTER_(sFunc)			DWORD errCode = GetLastError(); logEnter();
#define __WINAPI_LEAVE(sFunc)			logLeave(); SetLastError(errCode); return retCode;
#define __WINAPI_LEAVE_(sFunc)			logLeave(); SetLastError(errCode); return;

#define __SOCKAPI_ENTER(sFunc,type,code)	type retCode=code; int errCode = WSAGetLastError(); logEnter();
#define __SOCKAPI_ENTER_(sFunc)				int errCode = WSAGetLastError(); logEnter();
#define __SOCKAPI_LEAVE(sFunc)				logLeave(); WSASetLastError(errCode); return retCode;
#define __SOCKAPI_LEAVE_(sFunc)				logLeave(); WSASetLastError(errCode); return;

#endif //!_LOG_FULL_

#ifdef STEP_BY_STEP
#define logStep(s)		logMessage_FL(__FILE__,__LINE__,s)
#else //!STEP_BY_STEP
#define logStep(s)
#endif //STEP_BY_STEP

#ifdef __cplusplus
}

#ifdef WIN32
class CWaiter
{
private:
  HCURSOR m_hCursor;
public: 
  CWaiter(void)
  {
    m_hCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
  }
  ~CWaiter(void)
  {
    SetCursor(m_hCursor);
  }
};

#endif //WIN32

#endif //__cplusplus

#endif //_INC_LOGLIB_H_
