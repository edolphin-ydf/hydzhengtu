#ifndef _INC_UTILLIB_H_
#define _INC_UTILLIB_H_

#include <baseLib/platForm.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#define CH_TOKEN_SPLIT	'!'

#define VS_ASSIGNED     1

#define VS_MUSTEXIST    0
#define VS_OPTION       2

#define MAX_PASSWD		16

typedef enum{
  EVT_BYTE,
  EVT_WORD,
  EVT_SHORT,
  EVT_DWORD,
  EVT_LONG,
  EVT_INT  = EVT_LONG,
  EVT_BOOL = EVT_LONG,
  EVT_STRING,
  EVT_PATH
}EValueType;

typedef struct{
  PSTR       szName;
  EValueType eVT;
  union{
    PVOID  pv;
	PBYTE  bValue;
	PWORD  wValue;
	PDWORD dwValue;
	PSHORT sValue;
	PLONG  lValue;
	PSTR   szValue;
  }u;
  DWORD dwSize;
  int   nStyle;
}NValuePair,*PValuePair;

typedef struct{
  BYTE chPasswd[16];
}NEachPasswd,*PEachPasswd;

typedef int (*PFNValueNotify)(PVOID userData,PSTR szName,PSTR szValue);

int VPUpdateValue(PValuePair ptVP,DWORD dwVP,PFNValueNotify ValueNotify,PVOID userData,PSTR szName,PSTR szValue);

typedef struct{
  DWORD dwData;
  PSTR  szData;
}NLongTextItem,*PLongTextItem;

PSTR LTI_Data2Text(PLongTextItem pLTI,DWORD dwLTI,DWORD dwData);
PSTR LTI_Data2TextHex(PLongTextItem pLTI,DWORD dwLTI,DWORD dwData,PSTR szText,DWORD dwText);
PSTR LTI_Data2TextInt(PLongTextItem pLTI,DWORD dwLTI,DWORD dwData,PSTR szText,DWORD dwText);

BOOL LTI_Text2Data(PLongTextItem pLTI,DWORD dwLTI,PSTR szData,PDWORD pdwData);

#define LTI_ITEM(d)     {d,#d}
#define LTI_ITEM2(d,s)  {d,s}

#define LTI_DATA2TEXT(anLTI,dwData) LTI_Data2Text(anLTI,SIZEOF_ARRAY(anLTI),dwData)
#define LTI_DATA2TEXTHEX(anLTI,dwData,szText,dwText) LTI_Data2TextHex(anLTI,SIZEOF_ARRAY(anLTI),dwData,szText,dwText)
#define LTI_DATA2TEXTINT(anLTI,dwData,szText,dwText) LTI_Data2TextInt(anLTI,SIZEOF_ARRAY(anLTI),dwData,szText,dwText)
#define LTI_TEXT2DATA(anLTI,szData,pdwData) LTI_Text2Data(anLTI,SIZEOF_ARRAY(anLTI),szData,pdwData)

typedef struct{
  DWORD dwData;
  DWORD dwLong;
}NLongLongItem,*PLongLongItem;

BOOL LLI_Data2Long(PLongLongItem pLLI,DWORD dwLLI,DWORD dwData,PDWORD pdwLong);
BOOL LLI_Long2Data(PLongLongItem pLLI,DWORD dwLLI,DWORD dwLong,PDWORD pdwData);

#define LLI_ITEM(d,l)     {d,l}

#define LLI_DATA2LONG(anLLI,dwData,pdwLong) LLI_Data2Long(anLLI,SIZEOF_ARRAY(anLLI),dwData,pdwLong)
#define LLI_LONG2DATA(anLLI,szData,pdwData) LLI_Long2Data(anLLI,SIZEOF_ARRAY(anLLI),dwLong,pdwData)

#ifdef WIN32
HGLOBAL GlobalDuplicate(PBYTE pData,DWORD dwSize);
#endif //WIN32

void appStartup(void);
void appCleanup(void);
void appLock(void);
void appUnlock(void);

#ifdef HAVE_SYSTEM
BOOL system_ex(PSTR szResult,size_t cbResult,PSTR szFMT,...);
#endif //HAVE_SYSTEM

BOOL GetTempFile(PSTR szName,DWORD dwName);

FILE *OpenTempFile(PSTR szName,DWORD dwName);

void BIN2HEX(PBYTE pbData,size_t size,PSTR szSpace,PSTR szText,size_t cbText);
void BIN2HEX_CRLF(PBYTE pbData,size_t size,PSTR szSpace,PSTR szText,size_t cbText);
void BIN2HEX_DEBUG(PBYTE pData,size_t count,PSTR szText,size_t cbText);

void SleepUS(DWORD dwUS);
void SleepMS(DWORD dwMS);
void SleepSecond(DWORD nSeconds);

typedef int (*PFNOutput)(PVOID userData,PVOID pData,DWORD dwData);

int CachedOutput_Output(PBYTE pCachedData,DWORD dwSize,PDWORD pdwSize,PBYTE pData,DWORD dwData,PFNOutput Output,PVOID userData);

PSTR x86_CPUName(PSTR vendor,WORD family,BYTE model,WORD family_ex,BYTE model_ex);

BOOL x86_Cpuid(PSTR vendor_id/*[13]*/,PDWORD serial_no/*[4]*/);

BOOL GetMachineFinger(PSTR szText,size_t cbText);

void x86_DisableCpuid(void);

DWORD atoix(PSTR sText,int nText);

void ftoa(PSTR szText,DWORD cbText,double fValue);

char V2H(int value);
BYTE H2V(char value);
BYTE H2B(PSTR value);
WORD H2W(PSTR value);
DWORD H2DW(PSTR value);

DWORD HEX2BIN(PSTR szText,size_t cbText,PBYTE pData,DWORD dwData);
PBYTE HEX2BIN_Ex(PSTR szText,size_t cbText,PDWORD pdwData);

void swap(PVOID pt0,PVOID pt1,UINT uSize);
void wswap(PWORD pt0,PWORD pt1);
void dwswap(PDWORD pt0,PDWORD pt1);

void ReverseBytes(PBYTE pData,UINT uData);

#ifdef WIN32

PVOID get_argcargv(int *argc,char ***argv);

/************************************************************
rundll32.exe支持的接口

//rundll32 DllName,FunctionName [Arguments]
************************************************************/
typedef void (CALLBACK *PFNEntryPoint)(HWND HWindow,HINSTANCE hInstance,LPTSTR lpCmdLine,int nCmdShow);

BOOL CreateProcessEx(PSTR szProg,PSTR szCmdLine,BOOL bWait);

DWORD Rundll32(HWND HWindow,PSTR szModule,PSTR szFunction,PSTR lpCmdLine,int nCmdShow);
DWORD Rundll32h(HWND HWindow,HINSTANCE hInstance,PSTR szFunction,PSTR lpCmdLine,int nCmdShow);

PVOID LoadResourceData(HINSTANCE hInstance,LPCSTR lpTemplateName,PDWORD pdwSize);
BOOL SaveResourceFile(HINSTANCE hInstance,LPCSTR lpTemplateName,PSTR szName);

//#define emms  _emit 0x0F _emit 0x77
//#define cpuid _emit 0x0F _emit 0xA2

#define FEATURE_CPUID		0x00000001
#define FEATURE_FPU			0x00000008
#define FEATURE_TSC			0x00000010
#define FEATURE_MMX			0x00000020
#define FEATURE_CMOV		0x00000040
#define FEATURE_3DNOW		0x00000080
#define FEATURE_3DNOWEXT	0x00000100
#define FEATURE_MMXEXT		0x00000200
#define FEATURE_SSE			0x00000400
#define FEATURE_K6_MTRR		0x00000800
#define FEATURE_P6_MTRR		0x00001000
#define FEATURE_SSE2		0x00002000 

/* Symbolic constants for feature flags in CPUID standard feature flags */
#define CPUID_STD_FPU		0x00000001
#define CPUID_STD_VME		0x00000002
#define CPUID_STD_DEBUGEXT	0x00000004
#define CPUID_STD_4MPAGE	0x00000008
#define CPUID_STD_TSC		0x00000010
#define CPUID_STD_MSR		0x00000020
#define CPUID_STD_PAE		0x00000040
#define CPUID_STD_MCHKXCP	0x00000080
#define CPUID_STD_CMPXCHG8B	0x00000100
#define CPUID_STD_APIC		0x00000200
#define CPUID_STD_SYSENTER	0x00000800
#define CPUID_STD_MTRR		0x00001000
#define CPUID_STD_GPE		0x00002000
#define CPUID_STD_MCHKARCH	0x00004000
#define CPUID_STD_CMOV		0x00008000
#define CPUID_STD_PAT		0x00010000
#define CPUID_STD_PSE36		0x00020000
#define CPUID_STD_MMX		0x00800000
#define CPUID_STD_FXSAVE	0x01000000
#define CPUID_STD_SSE		0x02000000
#define CPUID_STD_SSE2		0x04000000

/* Symbolic constants for feature flags in CPUID extended feature flags */
#define CPUID_EXT_3DNOW        0x80000000
#define CPUID_EXT_AMD_3DNOWEXT 0x40000000
#define CPUID_EXT_AMD_MMXEXT   0x00400000

typedef struct{
  char  vendor_name[13];// vendor name
  PSTR  model_name;// name of model e.g. Intel Pentium-Pro 
  WORD  family;// family of the processor  e.g. 6 = Pentium-Pro architecture 
  WORD  family_ex;
  BYTE  model;// model of processor  e.g. 1 = Pentium-Pro for family = 6 
  BYTE  model_ex;
  BYTE  stepping;// processor revision number 
  DWORD standard;
  DWORD feature;// processor feature 
  DWORD extended;
  unsigned __int64 nSpeed;
  //(same as return value from _cpuid)
  DWORD cpu_support;
  DWORD os_support;// does OS Support the feature? 
}NProcessorInfo,*PProcessorInfo;

DWORD x86_GetCpuFeature(PProcessorInfo pPI);

unsigned __int64 x86_GetCpuSpeed(void);

#ifdef _MSC_VER
#pragma warning(disable:4035)
__forceinline __declspec(naked) PVOID GetEIP(void)
{
  __asm{
    call NEXT
NEXT:
    pop eax
	ret
  }
}
#pragma warning(default:4035)
#endif //_MSC_VER

void GetCallerEx(PVOID eip,PSTR szName,DWORD cbName);

#define GetCaller(pName,cName)	GetCallerEx(GetEIP(),pName,cName)

size_t getPWD(PSTR szText,size_t cbText,PSTR szPrompt);

#endif //WIN32

int VersionCompare(WORD wMSH0,WORD wMSL0,WORD wLSH0,WORD wLSL0,WORD wMSH1,WORD wMSL1,WORD wLSH1,WORD wLSL1);

BOOL GetStringVersion(PSTR szVersion,PWORD pwMSH,PWORD pwMSL,PWORD pwLSH,PWORD pwLSL);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_UTILLIB_H_
