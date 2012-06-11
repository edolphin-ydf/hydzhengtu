#ifndef _INC_CODELIB_
#define _INC_CODELIB_

#include <baseLib/platForm.h>

//#define USE_BUILDIN_UNICODE

#ifndef WIN32
#ifndef USE_BUILDIN_UNICODE
#define USE_BUILDIN_UNICODE
#endif //USE_BUILDIN_UNICODE
#endif //!WIN32

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

BOOL IsValidGb2312(BYTE b0,BYTE b1);

#ifdef USE_BUILDIN_UNICODE

size_t Utf8ToUnicode(PSTR szUtf8,size_t nUtf8,PWSTR pwWC,size_t nWC);
size_t Gb2312ToUnicode(PSTR szGb2312,size_t nGb2312,PWSTR pwWC,size_t nWC);

size_t UnicodeToUtf8(PWSTR pwWC,size_t nWC,PSTR szUtf8,size_t nUtf8);
size_t UnicodeToGb2312(PWSTR pwWC,size_t nWC,PSTR szGb2312,size_t nGb2312);

#ifdef WIN32

void buildTable(void);
void checkTable(void);

size_t IMultiByteToWideChar(UINT CodePage,DWORD dwFlags,PSTR szMB,size_t nMB,PWSTR pwWC,size_t nWC);
size_t IWideCharToMultiByte(UINT CodePage,DWORD dwFlags,PWSTR pwWC,size_t nWC,PSTR szMB,size_t nMB,LPCSTR lpDefaultChar,BOOL *lpUsedDefaultChar);

#else //!WIN32

#define CP_ACP      0
#define CP_UTF8     1

size_t MultiByteToWideChar(UINT CodePage,DWORD dwFlags,PSTR szMB,size_t nMB,PWSTR pwWC,size_t nWC);
size_t WideCharToMultiByte(UINT CodePage,DWORD dwFlags,PWSTR pwWC,size_t nWC,PSTR szMB,size_t nMB,LPCSTR lpDefaultChar,BOOL *lpUsedDefaultChar);

#endif //!WIN32

#endif //USE_BUILDIN_UNICODE

#ifdef WIN32
BSTR SysAllocStringA(PSTR szText,size_t cbText);

#define BSTR2PSTR(strptr)        WideCharToMultiByteEx(CP_ACP,strptr,SysStringLen(strptr))
#define PSTR2BSTR(strptr,size)   SysAllocStringA(strptr,size)
#endif //WIN32

#ifndef HAVE_WCSICMP
int wcsicmp(PWSTR sz0,PWSTR sz1);
#endif //!HAVE_WCSICMP

PWSTR MultiByteToWideCharEx(UINT CodePage,PSTR szMB,size_t nMB);
PSTR WideCharToMultiByteEx(UINT CodePage,PWSTR pwWC,size_t nWC);

size_t MultiByteToMultiByte(UINT cpMPI,PSTR szMBI,size_t nMBI,UINT cpMPO,PSTR szMBO,size_t nMBO);
PSTR MultiByteToMultiByteEx(UINT cpMPI,PSTR szMBI,size_t nMBI,UINT cpMPO);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_CODELIB_
