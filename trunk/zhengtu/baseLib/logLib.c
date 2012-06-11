#include <stdio.h>
#include <stdarg.h>
#include <baseLib/logLib.h>
#include <baseLib/strLib.h>
#include <baseLib/memLib.h>
#include <baseLib/fileLib.h>
#include <baseLib/timeLib.h>
#include <baseLib/sockLib.h>

static void filelogCode_FL_VP(const char *szFile,int nLine,int nError,PSTR szFMT,va_list vp);
static void filelogMessage_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp);

static void netlogCode_FL_VP(const char *szFile,int nLine,int nError,PSTR szFMT,va_list vp);
static void netlogMessage_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp);

#ifdef WIN32

#pragma comment(lib,"shlwapi")

static BOOL filelogDeviceOpen_FL(const char *szFile,int nLine,HANDLE *phFile);
static void filelogDeviceClose(HANDLE fp);

void vfprintf_H(HANDLE fp,PSTR szFMT,va_list vp);
void fprintf_H(HANDLE fp,PSTR szFMT,...);

#else //!WIN32

static BOOL filelogDeviceOpen_FL(const char *szFile,int nLine,FILE **phFile);
static void filelogDeviceClose(FILE *fp);

#define vfprintf_H	vfprintf
#define fprintf_H	fprintf

#endif //!WIN32

static BOOL netlogDeviceOpen_FL(const char *szFile,int nLine,PSTR szText,size_t dwText,size_t *pdwSize);

#define logDeviceOpen()		logDeviceOpen_FL(__FILE__,__LINE__)
#define netlogDeviceOpen()	netlogDeviceOpen_FL(__FILE__,__LINE__)

static DWORD g_dwFormat = LOGFMT_APP|LOGFMT_DATE|LOGFMT_TIME;
static SOCKET g_sock = INVALID_SOCKET;
static struct sockaddr_in g_remote;

#ifdef WIN32
static WCHAR g_szName[_MAX_PATH];
static HANDLE g_hMutex=NULL;
static NLogIndent g_nIndent={NULL,NULL,NULL};
 
static NLogDetours g_nDetours={
  CreateFileW,
  WriteFile,
  GetFileSize,
  SetFilePointer,
  SetEndOfFile,
  CloseHandle,
  WaitForSingleObjectEx,
  ReleaseMutex
};

#else //!WIN32

static char g_szName[_MAX_PATH];

#endif //!WIN32

static char g_szApp[33];

void logInit(PSTR szApp,PSTR szLog,DWORD dwLimits,PSTR szServer,short sPort)
{
  UINT   i,size;
#ifdef WIN32
  //char   szPath[_MAX_PATH];
  char   szName[_MAX_PATH];
  HANDLE fp;
#else //!WIN32
  long   nSize;
  FILE   *fp;
#endif //!WIN32

  sockStartup();
#ifdef WIN32
  if (NULL == szLog){
	GetModuleFileName(NULL,szName,sizeof(szName));
	PathRenameExtension(szName,".log");
	szLog = szName;
  }
  MultiByteToWideChar(CP_ACP,0,szLog,-1,g_szName,SIZEOF_ARRAY(g_szName));
  if (0 != szLog[0]){
    strcpyN(szName,sizeof(szName),szLog);
    strlwr(szName);
	for(i=0;0 != szName[i];i++){
	  switch(szName[i]){
	    case ':':
		case '\\':
		case '/':
		     szName[i] = '_';
			 break;
	  }
	}
    g_hMutex = CreateMutex(NULL,FALSE,szName);
  }
#else //WIN32
  strncpy(g_szName,szLog,sizeof(g_szName));
#endif //WIN32
  if (NULL == szApp){
#ifdef WIN32
    GetModuleFileName(NULL,szName,sizeof(szName));
    szApp = PathFindFileName(szName);
#else //!WIN32
    szApp = "";
#endif //!WIN32  
  }
  size = strlen(szApp);
  if (size > sizeof(g_szApp)-1) size = sizeof(g_szApp)-1;
  for(i=0;i<sizeof(g_szApp)-1-size;i++){
    g_szApp[i] = ' ';
  }
  for(;i<sizeof(g_szApp)-1;i++){
    g_szApp[i] = *szApp++;
  }
  g_szApp[sizeof(g_szApp)-1] = 0;
  //check size
  if (0 != dwLimits){
#ifdef WIN32
    g_nDetours.WaitForSingleObjectEx(g_hMutex,INFINITE,FALSE);
    if (INVALID_HANDLE_VALUE != (fp=g_nDetours.CreateFileW(g_szName,GENERIC_READ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL))){
	  if (g_nDetours.GetFileSize(fp,NULL) >= dwLimits){
	    g_nDetours.SetFilePointer(fp,0,NULL,FILE_BEGIN);
        g_nDetours.SetEndOfFile(fp);
      }
	  g_nDetours.CloseHandle(fp);
    }
    g_nDetours.ReleaseMutex(g_hMutex);
#else //!WIN32
    if (NULL != (fp=fopen(g_szName,"rt"))){
      fseek(fp,0,SEEK_END);
      nSize = ftell(fp);
	  fclose(fp);
	  if (nSize >= dwLimits){
	    unlink(g_szName);
	  }
    }
#endif //!WIN32
  }
  if (NULL != szServer && 0 != sPort){
    g_sock = socket(AF_INET,SOCK_DGRAM,0);

    memset(&g_remote,0,sizeof(g_remote));
    g_remote.sin_family = AF_INET;
    g_remote.sin_port   = htons(sPort);    
    sockTranslateAddr(szServer,TRUE,&g_remote.sin_addr);
  }
}

void logTerm(void)
{
#ifdef WIN32
  if (NULL != g_hMutex){
    CloseHandle(g_hMutex);
    g_hMutex = NULL;
  }
#endif //WIN32
  if (INVALID_SOCKET != g_sock){
    sockClose(g_sock);
    g_sock = INVALID_SOCKET;
  }
  sockCleanup();
}

void logFormat(DWORD dwFormat)
{
  g_dwFormat = dwFormat;
}

#ifdef WIN32
void odprintf(PSTR szFMT,...)
{
  char    buf[4096];
  PSTR    p;
  va_list vp;

  va_start(vp,szFMT);
  vsnprintf(buf,sizeof(buf),szFMT,vp);
  va_end(vp);
  p = &buf[strlen(buf)];
  while(p > buf && isspace(p[-1])) *--p = 0;
  *p++ = '\r';
  *p++ = '\n';
  *p   = 0;
  OutputDebugString(buf);
}

void vfprintf_H(HANDLE fp,PSTR szFMT,va_list vp)
{
  char  szText[32768];
  DWORD dwDone;

  vsnprintf(szText,sizeof(szText),szFMT,vp);
  szText[sizeof(szText)-1] = 0;
  g_nDetours.WriteFile(fp,szText,strlen(szText),&dwDone,NULL);
}

void fprintf_H(HANDLE fp,PSTR szFMT,...)
{
  va_list vp;

  va_start(vp,szFMT);
  vfprintf_H(fp,szFMT,vp);
  va_end(vp);
}

void logEnter(void)
{
  if (NULL == g_nIndent.logEnter) return;
  g_nIndent.logEnter();
}

void logLeave(void)
{
  if (NULL == g_nIndent.logLeave) return;
  g_nIndent.logLeave();
}

PLogDetours logDetours(void)
{
  return &g_nDetours;
}

PLogIndent logGetIndent(void)
{
  return &g_nIndent;
}
#endif //WIN32

#ifdef WIN32
BOOL filelogDeviceOpen_FL(const char *szFile,int nLine,HANDLE *pfp)
#else //!WIN32
BOOL filelogDeviceOpen_FL(const char *szFile,int nLine,FILE **pfp)
#endif //!WIN32
{
  PSTR       token;
  SYSTEMTIME now;
#ifdef WIN32
  DWORD      dwSize,dwIndent;
#endif //!WIN32

#ifdef WIN32
  *pfp = INVALID_HANDLE_VALUE;
  if (0 == g_szName[0]) return FALSE;
  g_nDetours.WaitForSingleObjectEx(g_hMutex,INFINITE,FALSE);
  if (INVALID_HANDLE_VALUE == (*pfp=g_nDetours.CreateFileW(g_szName,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL))){
    g_nDetours.ReleaseMutex(g_hMutex);
	return FALSE;
  }
  g_nDetours.SetFilePointer(*pfp,0,NULL,FILE_END);
#else //!WIN32
  *pfp = NULL;
  if (0 == g_szName[0]) return FALSE;
  if (NULL == (*pfp=fopen(g_szName,"at"))) return FALSE;
#endif //!WIN32
  if (0 != g_szApp[0]){
    if (FLAGON(g_dwFormat,LOGFMT_APP)){
      fprintf_H(*pfp,"[%s] ",g_szApp);
	}
  }
  if (FLAGON(g_dwFormat,LOGFMT_DATE | LOGFMT_TIME)){
    GetLocalTime(&now);
	if (FLAGON(g_dwFormat,LOGFMT_DATE)){
      fprintf_H(*pfp,"%04d/%02d/%02d ",now.wYear,now.wMonth,now.wDay);
	}
	if (FLAGON(g_dwFormat,LOGFMT_TIME)){
      fprintf_H(*pfp,"%02d:%02d:%02d ",now.wHour,now.wMinute,now.wSecond);
	}
  }
  if (NULL != szFile){
    if (FLAGON(g_dwFormat,LOGFMT_NAME)){
      if (FLAGON(g_dwFormat,LOGFMT_PATH)){
        fprintf_H(*pfp,"%s:%d ",szFile,nLine);
	  }
      else{
	    if (NULL == (token=strrchr(szFile,'/'))){
		  token = strrchr(szFile,'\\');
		}
		if (NULL != token){
          token++;
		}
		else{
		  token = szFile;
		}
        fprintf_H(*pfp,"%s:%d ",token,nLine);
	  }
	}
  }
  if (FLAGON(g_dwFormat,LOGFMT_THREAD)){
#ifdef WIN32
	fprintf_H(*pfp,"0x%08X:0x%08X ",GetCurrentProcessId(),GetCurrentThreadId());
#else //!WIN32
  	fprintf(*pfp,"0x%08X:0x%08X ",getppid(),getpid());
#endif //!WIN32
  }
#ifdef WIN32
  if (NULL != g_nIndent.logLevel){
    dwIndent = g_nIndent.logLevel();
	while(dwIndent--){
	  g_nDetours.WriteFile(*pfp,"  ",2,&dwSize,NULL);
	}
  }
#endif //WIN32
  return TRUE;
}

#ifdef WIN32
void filelogDeviceClose(HANDLE fp)
#else //!WIN32
void filelogDeviceClose(FILE *fp)
#endif //!WIN32
{
#ifdef WIN32
  g_nDetours.CloseHandle(fp);
  g_nDetours.ReleaseMutex(g_hMutex);
#else //!WIN32
  fclose(fp);
#endif //!WIN32
}

void filelogMessage_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp)
{
#ifdef WIN32
  HANDLE fp;
#else //!WIN32
  FILE *fp;
#endif //!WIN32

  if (!filelogDeviceOpen_FL(szFile,nLine,&fp)) return;
  vfprintf_H(fp,szFMT,vp);
#ifdef WIN32
  fprintf_H(fp,"\r\n");
#else //!WIN32
  fprintf(fp,"\n");
#endif //!WIN32
  filelogDeviceClose(fp);
}

void filelogCode_FL_VP(const char *szFile,int nLine,int nError,PSTR szFMT,va_list vp)
{
#ifdef WIN32
  PSTR   pText;
  char   szMsg[1024];
  HANDLE fp;
#else //!WIN32
  FILE *fp;
#endif //!WIN32

  if (!filelogDeviceOpen_FL(szFile,nLine,&fp)) return;
  vfprintf_H(fp,szFMT,vp);
#ifdef WIN32
  if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,nError,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                         szMsg,sizeof(szMsg),NULL)){
    fprintf_H(fp," 0x%08X(%d)\r\n",nError,nError);
  }
  else{
    pText = &szMsg[strlen(szMsg)-1];
    for(pText=szMsg;0 != *pText;pText++){
	  if ('\r' == *pText || '\n' == *pText){
	    *pText = 0;
		break;
	  }
    }
	fprintf_H(fp," 0x%08X(%d) %s\r\n",nError,nError,szMsg);
  }
#else //!WIN32
  fprintf(fp," 0x%08X(%d) %s\n",nError,nError,strerror(nError));
#endif //!WIN32
  filelogDeviceClose(fp);
}

BOOL netlogDeviceOpen_FL(const char *szFile,int nLine,PSTR szText,size_t dwMax,size_t *pdwSize)
{
  PSTR       token;
  SYSTEMTIME now;

  if (INVALID_SOCKET == g_sock) return FALSE;
  *pdwSize = 0;
  szText[0] = 0;
  if (0 != g_szApp[0]){
    if (FLAGON(g_dwFormat,LOGFMT_APP)){
      strcpyexv(szText,dwMax,pdwSize,"[%s] ",g_szApp);
	}
  }
  if (FLAGON(g_dwFormat,LOGFMT_DATE | LOGFMT_TIME)){
    GetLocalTime(&now);
	if (FLAGON(g_dwFormat,LOGFMT_DATE)){
      strcpyexv(szText,dwMax,pdwSize,"%04d/%02d/%02d ",now.wYear,now.wMonth,now.wDay);
	}
	if (FLAGON(g_dwFormat,LOGFMT_TIME)){
      strcpyexv(szText,dwMax,pdwSize,"%02d:%02d:%02d ",now.wHour,now.wMinute,now.wSecond);
	}
  }
  if (NULL != szFile){
    if (FLAGON(g_dwFormat,LOGFMT_NAME)){
      if (FLAGON(g_dwFormat,LOGFMT_PATH)){
        strcpyexv(szText,dwMax,pdwSize,"%s:%d ",szFile,nLine);
	  }
      else{
	    if (NULL == (token=strrchr(szFile,'/'))){
		  token = strrchr(szFile,'\\');
		}
		if (NULL != token){
          token++;
		}
		else{
		  token = szFile;
		}
        strcpyexv(szText,dwMax,pdwSize,"%s:%d ",token,nLine);
	  }
	}
  }
  if (FLAGON(g_dwFormat,LOGFMT_THREAD)){
#ifdef WIN32
	strcpyexv(szText,dwMax,pdwSize,"0x%08X:0x%08X ",GetCurrentProcessId(),GetCurrentThreadId());
#else //!WIN32
  	strcpyexv(szText,dwMax,pdwSize,"0x%08X:0x%08X ",getppid(),getpid());
#endif //!WIN32
  }
  return TRUE;
}

void netlogMessage_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp)
{
  char   szText[8192];
  size_t dwSize;

  if (!netlogDeviceOpen_FL(szFile,nLine,szText,sizeof(szText),&dwSize)) return;
  strcpyexvp(szText,sizeof(szText),&dwSize,szFMT,vp);
  sendto(g_sock,szText,dwSize,MSG_DONTROUTE,(PSOCKADDR)&g_remote,sizeof(g_remote));
}

void netlogCode_FL_VP(const char *szFile,int nLine,int nError,PSTR szFMT,va_list vp)
{
  char   szText[8192];
  size_t dwSize;
#ifdef WIN32
  PSTR   pText;
  char   szMsg[1024];
#endif //!WIN32

  if (!netlogDeviceOpen_FL(szFile,nLine,szText,sizeof(szText),&dwSize)) return;
  strcpyexvp(szText,sizeof(szText),&dwSize,szFMT,vp);
#ifdef WIN32
  if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,nError,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                         szMsg,sizeof(szMsg),NULL)){
    strcpyexv(szText,sizeof(szText),&dwSize," 0x%08X(%d)\r\n",nError,nError);
  }
  else{
    pText = &szMsg[strlen(szMsg)-1];
    for(pText=szMsg;0 != *pText;pText++){
	  if ('\r' == *pText || '\n' == *pText){
	    *pText = 0;
		break;
	  }
    }
	strcpyexv(szText,sizeof(szText),&dwSize," 0x%08X(%d) %s\r\n",nError,nError,szMsg);
  }
#else //!WIN32
  strcpyexv(szText,sizeof(szText),&dwSize," 0x%08X(%d) %s\n",nError,nError,strerror(nError));
#endif //!WIN32
  sendto(g_sock,szText,dwSize,MSG_DONTROUTE,(PSOCKADDR)&g_remote,sizeof(g_remote));
}

void logMessage(PSTR szFMT,...)
{
  va_list vp;
  
  va_start(vp,szFMT);
  netlogMessage_FL_VP(NULL,0,szFMT,vp);
  filelogMessage_FL_VP(NULL,0,szFMT,vp);
  va_end(vp);
}

void logError_FL(const char *szFile,int nLine,PSTR szFMT,...)
{
  int     nError;
  va_list vp;

#ifdef WIN32
  nError = GetLastError();
#else //!WIN32
  nError = errno;
#endif //!WIN32
  va_start(vp,szFMT);
  netlogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
  filelogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
  va_end(vp);
}

void logMessage_FL(const char *szFile,int nLine,PSTR szFMT,...)
{
  va_list vp;

  va_start(vp,szFMT);
  netlogMessage_FL_VP(szFile,nLine,szFMT,vp);
  filelogMessage_FL_VP(szFile,nLine,szFMT,vp);
  va_end(vp);
}

void logCode_FL(const char *szFile,int nLine,int nError,PSTR szFMT,...)
{
  va_list vp;

  va_start(vp,szFMT);
  netlogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
  filelogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
  va_end(vp);
}

void logMessage_VP(PSTR szFMT,va_list vp)
{
  netlogMessage_FL_VP(NULL,0,szFMT,vp);
  filelogMessage_FL_VP(NULL,0,szFMT,vp);
}

void logError_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp)
{
  int nError;

#ifdef WIN32
  nError = GetLastError();
#else //!WIN32
  nError = errno;
#endif //!WIN32
  netlogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
  filelogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
}

void logMessage_FL_VP(const char *szFile,int nLine,PSTR szFMT,va_list vp)
{
  netlogMessage_FL_VP(szFile,nLine,szFMT,vp);
  filelogMessage_FL_VP(szFile,nLine,szFMT,vp);
}

void logCode_FL_VP(const char *szFile,int nLine,int nError,PSTR szFMT,va_list vp)
{
  netlogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
  filelogCode_FL_VP(szFile,nLine,nError,szFMT,vp);
}
