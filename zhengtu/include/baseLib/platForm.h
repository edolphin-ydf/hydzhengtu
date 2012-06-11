#ifndef _INC_PLATFORM_H
#define _INC_PLATFORM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#else //!HAVE_CONFIG_H

//config.h begin
#ifdef _MSC_VER
#define _X86
#define X86

#define SERIAL_PORT_FORMAT   "COM%d"
#define SERIAL_PORT_ADJUST   0

#define HAVE_WINDOWS_H	1
#define HAVE_COMMCTRL_H	1
#define HAVE_WINDOWSX_H	1
#define HAVE_WINSOCK_H	1
#define HAVE_ODBCINST_H	1

#undef HAVE_ARPA_INET_H
#undef HAVE_DOPRNT

#define HAVE_DUP2	1
#define HAVE_FCNTL_H	1

#undef HAVE_FORK
#define HAVE_FTIME	1
#define HAVE_FTRUNCATE	1
#define HAVE_GETHOSTBYNAME	1
#define HAVE_GETHOSTNAME	1
#define HAVE_GETPAGESIZE	1
#define HAVE_INET_NTOA	1
#undef HAVE_INTTYPES_H

#undef HAVE_LIBPTHREAD

#define HAVE_LIBZ	1
#define HAVE_LIMITS_H	1
#define HAVE_MALLOC_H	1
#define HAVE_MEMMOVE	1
#define HAVE_MEMORY_H	1
#define HAVE_MEMSET	1
#define HAVE_MKDIR	1
#define HAVE_MEMICMP	1

#undef HAVE_MMAP

#undef HAVE_MUNMAP

#undef HAVE_NETDB_H

#undef HAVE_NETINET_IN_H

#define HAVE_REALLOC	1
#define HAVE_SELECT	1

#define HAVE_SOCKET	1

#undef HAVE_STDINT_H

#define HAVE_STDLIB_H	1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR	1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP	1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR	1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME	1

#define HAVE_SHELLEXECUTE	1
#define HAVE_SYSTEMTIME	1
#define HAVE_GETLOCALTIME	1
#define HAVE_HANDLE	1
#define HAVE_STRLWR	1
#define HAVE_STRUPR	1
#define HAVE_STRICMP	1
#define HAVE_WCSICMP	1
#define HAVE_GETPRIVATEPROFILESTRING	1
#define HAVE_BSTR	1
#define HAVE_MULTIBYTETOWIDECHAR	1
#define HAVE_WIDECHARTOMULTIBYTE	1

#define HAVE_STRINGS_H	1
#define HAVE_STRING_H	1
#define HAVE_STRRCHR	1
#define HAVE_STRSTR	1

#undef HAVE_SYS_SELECT_H
#undef HAVE_SYS_SOCKET_H

#define HAVE_SYS_STAT_H

#define HAVE_SYS_TIMEB_H

#undef HAVE_SYS_TIME_H

#define HAVE_SYS_TYPES_H	1

#undef HAVE_SYS_WAIT_H

#undef HAVE_TERMIOS_H

#undef HAVE_UNISTD_H

#undef HAVE_VFORK

/* Define to 1 if you have the <io.h> header file. */
#define HAVE_IO_H		1

/* Define to if you have the <vfork.h> header file. */
/* #undef HAVE_VFORK_H */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF	1

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H	1

/* Define to 1 if `fork' works. */
/* #undef HAVE_WORKING_FORK */

/* Define to 1 if `vfork' works. */
//#define HAVE_WORKING_VFORK
#undef HAVE_WORKING_VFORK

/* Define to 1 if you have the `__pthread_initialize' function. */
#undef HAVE___PTHREAD_INITIALIZE

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to the type of arg 1 for `select'. */
#define SELECT_TYPE_ARG1 int

/* Define to the type of args 2, 3 and 4 for `select'. */
#define SELECT_TYPE_ARG234 (fd_set *)

/* Define to the type of arg 5 for `select'. */
#define SELECT_TYPE_ARG5 (struct timeval *)

/* Define to 1 if you have the ANSI C header files. */
/* #undef STDC_HEADERS */

//#define TIME_WITH_SYS_TIME	1

/* #undef TM_IN_SYS_TIME */

/* #undef const */

/* #undef inline */

/* #undef off_t */


/* #undef pid_t */

/* #undef size_t */

/* Define as `fork' if `vfork' does not work. */
/* #undef vfork */

#ifndef IMPORT
#define IMPORT				__declspec(dllimport)
#endif //IMPORT

#ifndef EXPORT
#define EXPORT				__declspec(dllexport)
#endif //EXPORT

#endif //_MSC_VER
//config.h end

#endif //HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif //HAVE_WINDOWS_H

#ifdef HAVE_WINDOWSX_H
#ifndef __cplusplus
#include <windowsx.h>
#endif //__cplusplus
#endif //HAVE_WINDOWSX_H

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif //HAVE_SYS_TYPES_H

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif //HAVE_UNISTD_H

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif //HAVE_WCHAR_H

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>

typedef void *HINSTANCE;

#define LoadLibrary(szName) dlopen(szName,RTLD_NOW)
#define GetProcAddress      dlsym
#define FreeLibrary         dlclose

#endif //HAVE_DLFCN_H

#ifdef _MSC_VER
#define vsnprintf                             _vsnprintf
#endif //_MSC_VER

#ifndef WIN32

#ifndef IMPORT
#define IMPORT
#endif //IMPORT

#ifndef EXPORT
#define EXPORT
#endif //EXPORT

#if defined(HAVE_IODBC) && defined(ODBCVER)
#include <sql.h>
#else //! HAVE_IODBC && ODBCVER
typedef int                                    BOOL;
typedef unsigned short                         WORD;
typedef unsigned long                          DWORD;
typedef char                                   *LPSTR;
typedef const char							   *LPCSTR;
#endif //HAVE_IODBC && ODBCVER

typedef unsigned char                          BYTE;
typedef long                                   LONG;
typedef unsigned int                           UINT;
typedef BYTE                                   *PBYTE;
typedef char                                   *PSTR;
typedef int                                    *PINT;
typedef long                                   *PLONG;
typedef void                                   *PVOID;
typedef WORD                                   *PWORD;
typedef short                                  *PSHORT;
typedef DWORD                                  *PDWORD;

typedef wchar_t                                WCHAR;
typedef WCHAR                                  *PWSTR;

#define WINAPI
#define IN
#define OUT

#ifndef FALSE
#define FALSE                                  0
#endif //!FALSE

#ifndef TRUE
#define TRUE                                   1
#endif //!TRUE

#ifndef min
#define min(a,b)                               ((a)<(b)?(a):(b))
#endif //!min

#ifndef max
#define max(a,b)                               ((a)>(b)?(a):(b))
#endif //!max

#define LOBYTE(w)                              ((BYTE)w)
#define HIBYTE(w)                              ((BYTE)(w>>8))

typedef struct{
  long x;
  long y;
}POINT,*PPOINT;

typedef int	HANDLE;

#endif //!WIN32

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define BIT_TEXT(uMask,fProc,fData,uBit) if (FLAGON(uMask,uBit)) fProc(fData,# uBit);
#define BIT_TEXT2(uMask,sBit,fProc,fData,uBit) if (FLAGON(uMask,uBit)) fProc(fData,sBit);

#define MAP_BIT(uMask,szText,cbText,cbMax,uBit)\
  if (FLAGON(uMask,uBit)){\
    if (0 != cbText) szText[cbText++] = ' ';\
    strcpyN(&szText[cbText],cbMax - cbText, # uBit);\
	cbText += strlen(&szText[cbText]);\
  }

#define MAP_BIT2(uMask,sBit,szText,cbText,cbMax,uBit)\
  if (FLAGON(uMask,uBit)){\
    if (0 != cbText) szText[cbText++] = ' ';\
    strcpyN(&szText[cbText],cbMax - cbText,sBit);\
	cbText += strlen(&szText[cbText]);\
  }

#define MAP_TEXT(m)								case m: return # m;
#define MAP_TEXT2(m,s)							case m: return s;

#define FLAGON(b,f)								(0!=((b)&(f)))
#define FLAGON_ALL(b,f)							((f)==((b)&(f)))
#define SIZEOF_ARRAY(a)							(sizeof(a)/sizeof((a)[0]))
#define AssignPoint(p,ix,iy)					{ (p).x = ix; (p).y = iy; }

typedef PSTR		*PPSTR;
typedef PBYTE		*PPBYTE;
typedef PVOID		*PPVOID;
typedef double		*PDOUBLE;

typedef const char *PCSTR;

typedef wchar_t const *LPCWSTR;
typedef wchar_t const *PCWSTR;

typedef int (*PFNCompare_CLIB)(const void *elem1,const void *elem2);

#ifndef _MAX_PATH
#define _MAX_PATH							1024
#endif //_MAX_PATH

#define _MAX_FEXT							32

#ifndef _API_DESC_
#define _API_DESC_
#endif //_API_DESC_

#define T2HH(time)			(time/3600)
#define T2MM(time)			((time/60)%60)
#define T2SS(time)			(time%60)
#define T2HHMMSS(time)		T2HH(time),T2MM(time),T2SS(time)

#define HHMMSS2T(hh,mm,ss)	(hh*3600+mm*60+ss)
#define HHMMSS_FMT			"%02u:%02u:%02u"

#define FT2HH(time)			(time/3600000)
#define FT2MM(time)			((time/60000)%60)
#define FT2SS(time)			((time/1000)%60)
#define FT2MS(time)			(time%1000)
#define FT2HHMMSS(time)	    FT2HH(time),FT2MM(time),FT2SS(time)
#define FT2HHMMSSMS(time)	FT2HH(time),FT2MM(time),FT2SS(time),FT2MS(time)

#define HHMMSSMS2FT(hh,mm,ss,ms)    (hh*3600000+mm*60000+ss*1000+ms)
#define HHMMSS2FT(hh,mm,ss)	        (hh*3600000+mm*60000+ss*1000)
#define HHMMSSMS_FMT		        "%02u:%02u:%02u.%03u"

#define MB2B(b)				((b)*(1024*1024))
#define B2MB(b)				((b)/(1024*1024))

#define MM2SS(m)			((m)*60)
#define SS2MM(m)			((m)/60)

typedef PBYTE (*PFN_EncryptDecrypt)(PVOID pKey,PBYTE pData,DWORD cbData,PDWORD pcbOut);

#endif //_INC_PLATFORM_H
