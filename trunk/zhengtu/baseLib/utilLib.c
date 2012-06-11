#include <stdarg.h>
#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <baseLib/memLib.h>
#include <baseLib/utilLib.h>
#include <baseLib/logLib.h>
#include <baseLib/codeLib.h>
#include <baseLib/strLib.h>
#include <baseLib/fileLib.h>

#ifdef WIN32

#pragma comment(lib,"ws2_32")
#pragma comment(lib,"shlwapi")

#include <commctrl.h>
#include <conio.h>

HGLOBAL GlobalDuplicate(PBYTE pData,DWORD dwSize)
{
  HGLOBAL hGlobal;
  
  if (NULL == (hGlobal=GlobalAlloc(GHND,dwSize))) return NULL;
  memcpy((PBYTE)GlobalLock(hGlobal),pData,dwSize);
  GlobalUnlock(hGlobal);
  return hGlobal;
}

#endif //WIN32

static CRITICAL_SECTION g_nCS;

void appStartup(void)
{
  InitializeCriticalSection(&g_nCS);
}

void appCleanup(void)
{
  DeleteCriticalSection(&g_nCS);
}

void appLock(void)
{
  EnterCriticalSection(&g_nCS);
}

void appUnlock(void)
{
  LeaveCriticalSection(&g_nCS);
}

#ifdef HAVE_SYSTEM
BOOL system_ex(PSTR szResult,size_t cbResult,PSTR szFMT,...)
{
  char    szName[256];
  char    szCmdLine[1024];
  FILE    *fp;
  va_list vp;

#ifdef HAVE_MKSTEMP
  if (NULL == mkstemp(szName)) return FALSE;
#else //!HAVE_MKSTEMP
  if (NULL == tmpnam(szName)) return FALSE;
#endif //!HAVE_MKSTEMP
  va_start(vp,szFMT);
  vsnprintf(szCmdLine,sizeof(szCmdLine),szFMT,vp);
  va_end(vp);
  strcatV(szCmdLine,sizeof(szCmdLine)," > %s 2> /dev/null",szName);
  if (system(szCmdLine) < 0) return FALSE;
  if (NULL == (fp=fopen(szName,"rt"))) return FALSE;
  cbResult = fread(szResult,sizeof(char),cbResult-1,fp);
  szResult[cbResult] = 0;
  fclose(fp);
  unlink(szName);
  return TRUE;
}
#endif //HAVE_SYSTEM

FILE *OpenTempFile(PSTR szName,DWORD dwName)
{
  FILE *fp;

  if (!GetTempFile(szName,dwName)) return NULL;
  if (NULL == (fp=fopen(szName,"wt")))
  {
    logMessage1("!fopen(%s)",szName);
    return NULL;
  }
  return fp;
}

#ifdef WIN32
BOOL GetTempFile(PSTR szName,DWORD dwName)
{
  char szPath[_MAX_PATH];
  char szTemp[_MAX_PATH];

  GetTempPath(sizeof(szPath),szPath);
  GetTempFileName(szPath,"tmp",0,szTemp);
  strcpyN(szName,dwName,szTemp);
  return TRUE;
}
#else //!WIN32
BOOL GetTempFile(PSTR szName,DWORD dwName)
{
  PSTR szTemp;

#ifdef HAVE_MKSTEMP
  if (NULL == (szTemp=mkstemp(NULL)))
  {
    logError0("!mkstemp");
#else //!HAVE_MKSTEMP
  if (NULL == (szTemp=tmpnam(NULL)))
  {
    logError0("!tmpnam");
#endif //!HAVE_MKSTEMP  
    return FALSE;
  }
  strcpyN(szName,dwName,szTemp);
  return TRUE;
}
#endif //WIN32

void SleepUS(DWORD dwUS)
{
#ifdef WIN32
  Sleep(dwUS/1000);
#else //!WIN32
  usleep(dwUS);
#endif //!WIN32
}

void SleepMS(DWORD dwMS)
{
#ifdef WIN32
  Sleep(dwMS);
#else //!WIN32
  usleep(dwMS*1000);
#endif //!WIN32
}

void SleepSecond(DWORD nSeconds)
{
#ifdef WIN32
  Sleep(nSeconds*1000);
#else //!WIN32
  sleep(nSeconds);
#endif //!WIN32
}

int VPUpdateValue(PValuePair ptVP,DWORD dwVP,PFNValueNotify ValueNotify,PVOID userData,PSTR szName,PSTR szValue)
{
  DWORD i;

  for(i=0;i<dwVP;i++)
  {
    if (FLAGON(ptVP[i].nStyle,VS_ASSIGNED)) continue;
	if (0 == stricmp(ptVP[i].szName,szName)) break;
  }
  if (i<dwVP)
  {
    switch(ptVP[i].eVT)
    {
	  case EVT_BYTE:	       
	       *(ptVP[i].u.bValue) = (BYTE)atoi(szValue);
		   break;
	  case EVT_WORD:
		   *(ptVP[i].u.wValue) = (WORD)atoi(szValue);
		   break;
	  case EVT_DWORD:
		   *(ptVP[i].u.dwValue) = (DWORD)atol(szValue);
		   break;
	  case EVT_SHORT:
		   *(ptVP[i].u.sValue) = (short)atoi(szValue);
		   break;
	  case EVT_LONG:
		   *(ptVP[i].u.lValue) = atol(szValue);
		   break;
	  case EVT_STRING:
	       strcpyN(ptVP[i].u.szValue,ptVP[i].dwSize,szValue);
		   break;
	  case EVT_PATH:
		   strcpyN(ptVP[i].u.szValue,ptVP[i].dwSize,szValue);
		   PathAddBackslash(ptVP[i].u.szValue);
		   break;
	}
	ptVP[i].nStyle |= VS_ASSIGNED;
  }
  else{
	if (NULL != ValueNotify) return ValueNotify(userData,szName,szValue);
  }
  return 1;
}

void BIN2HEX(PBYTE pbData,size_t size,PSTR szSpace,PSTR szText,size_t cbText)
{
  size_t i,cbCurr,cbSpace;

  szText[0] = 0;
  cbSpace = NULL == szSpace ? 0 : strlen(szSpace);
  for(i=cbCurr=0;i<size;i++)
  {
    if (cbText <= cbCurr+cbSpace+2) break;
    strcpyV(&szText[cbCurr],cbText - cbCurr,"%02X",pbData[i]);
    cbCurr += 2;
	if (NULL != szSpace)
    {
      strcpyN(&szText[cbCurr],cbText - cbCurr,szSpace);
      cbCurr += cbSpace;
    }
  }
}

void BIN2HEX_CRLF(PBYTE pbData,size_t size,PSTR szSpace,PSTR szText,size_t cbText)
{
  size_t i,cbCurr,cbSpace;

  szText[0] = 0;
  cbSpace = strlen(szSpace);
  for(i=cbCurr=0;i<size;i++)
  {
#ifdef WIN32
    if (cbText <= cbCurr+cbSpace+4+2) break;
#else
    if (cbText <= cbCurr+cbSpace+4+1) break;
#endif //WIN32
    strcpyV(&szText[cbCurr],cbText - cbCurr,"0x%02X%s",pbData[i],szSpace);
    cbCurr += 4;
	if (15 == (i%16))
    {
#ifdef WIN32
      strcpyN(&szText[cbCurr],cbText - cbCurr,"\r\n");
#else
      strcpyN(&szText[cbCurr],cbText - cbCurr,"\n");
#endif //WIN32
	}
  }
}

void BIN2HEX_DEBUG(PBYTE pData,size_t count,PSTR szText,size_t cbText)
{
  char   szZero[2];
  size_t i,j,lineNum;
  size_t ulen;

  szText[0] = 0;
  szZero[1] = 0;
  lineNum = (count + 15) / 16;
  for(i=0;i<lineNum;i++,pData+=16)
  {
#ifdef WIN32
    if (strlen(szText) + 8 + 2 + 16 * 3 + 16 + 2 >= cbText) break;
#else //!WIN32
    if (strlen(szText) + 8 + 2 + 16 * 3 + 16 + 1 >= cbText) break;
#endif //!WIN32
    strcatV(szText,cbText,"%08x: ",i*16);
	
    ulen   = (count > 16) ? 16 : count;
    count -= ulen;
	for(j=0;j<ulen;j++)
    {
      strcatV(szText,cbText,"%02X ",pData[j]);
    }
	for(;j<16;j++)
    {
      strcatN(szText,cbText,"   ");
	}
    for(j=0;j<ulen;j++)
    {
      szZero[0] = isprint(pData[j]) ? pData[j] : '.';
      strcatN(szText,cbText,szZero);
	}
#ifdef WIN32
      strcatN(szText,cbText,"\r\n");
#else
      strcatN(szText,cbText,"\n");
#endif //WIN32
  }
}

void ReverseBytes(PBYTE pData,UINT uData)
{
  UINT i;
  
  for(i=0;i<uData/2;i++)
  {
    swap(&pData[i],&pData[uData-i-1],sizeof(BYTE));
  }
}

int CachedOutput_Output(PBYTE pCachedData,DWORD dwMaxSize,PDWORD pdwSize,PBYTE pData,DWORD dwData,PFNOutput Output,PVOID userData)
{
  DWORD dwCopy;

  __API_ENTER("CachedOutput_Output",int,0);
  if (NULL == pData)
  {
    if (0 != *pdwSize)
    {
      if (0 != (retCode=Output(userData,pCachedData,*pdwSize))) __API_FINISH();
	}
	*pdwSize = 0;
    __API_FINISH();
  }
  if (-1 == dwData) dwData = strlen(pData);
  if (*pdwSize + dwData < dwMaxSize)
  {
    //全部数据小于一个cache
    memcpy(&pCachedData[*pdwSize],pData,dwData);
    *pdwSize += dwData;
  }
  else{
    //全部数据大于等于cache
    if (0 == *pdwSize)
    {
	  //cache为空
	  if (0 != (retCode=Output(userData,pData,dwData))) __API_FINISH();
    }
    else{
	  //计算cache空间
	  dwCopy = dwMaxSize - *pdwSize;
      memcpy(&pCachedData[*pdwSize],pData,dwCopy);
	  if (0 != (retCode=Output(userData,pCachedData,dwMaxSize))) __API_FINISH();
      pData  += dwCopy;
      dwData -= dwCopy;
      dwCopy  = dwData / dwMaxSize * dwMaxSize;
      if (0 != dwCopy)
      {
        if (0 != (retCode=Output(userData,pData,dwCopy))) __API_FINISH();
	    pData += dwCopy;
        dwData -= dwCopy;
	  }
      memcpy(pCachedData,pData,dwData);
      *pdwSize = dwData;
	}
  }
__API_END_POINT:
  __API_LEAVE("CachedOutput_Output");
}

/*
static PSTR szHZText[]={"零","一","二","三","四","五","六","七","八","九"};
static PSTR szHZName0[]={"十","百","千","万","十万","百万","千万","十亿","亿"};
static PSTR szHZName1[]={"十","百","千","万","十万","兆","十兆","百兆","吉"};

#define BHZDT_NONAME    0
#define BHZDT_NAME00    1
#define BHZDT_NAME      2

int gridBuildHZDigitText(PSTR szVars,DWORD dwValue,int iUseName)
{
  int   i,iVars,iName;
  DWORD dwName;
  BOOL  bZeroed;
  char  szDigit[32];

  if (0 == dwValue)
  {
	strcpy(szVars,szHZText[0]);
	return 2;
  }
  sprintf(szDigit,"%u",dwValue);
  if (BHZDT_NONAME == iUseName)
  {
    for(i=0;0 != szDigit[i];i++) strcpy(&szVars[i*2],szHZText[szDigit[i]-'0']);
    szVars[i*2] = 0;
    return i * 2;
  }
  iVars   = 0;
  bZeroed = FALSE;
  for(i=8,dwName=1000000000;0 != dwName;dwName/=10,i--)
  {
    if (dwValue < dwName)
    {
	  if (0 == iVars || bZeroed) continue;
	  if (-1 == i) goto SKIP_DIGIT;
      strcpy(szVars,szHZText[0]);
	  iVars  += 2;
	  bZeroed = TRUE;
	}
	else{
	  bZeroed = FALSE;
	  iName   = (int)(dwValue / dwName);
	  if (iName == 1)
      {
		if (BHZDT_NAME00 == iUseName)
        {
		  if (0 == i || 4 == i || 7 == i) goto SKIP_DIGIT;
		}
		else{
		  if (0 == i || 4 == i || 6 == i) goto SKIP_DIGIT;
		}
	  }
	  else{
		if (0 == iName && -1 == i) goto SKIP_DIGIT;
	  }
	  strcpy(szVars,szHZText[iName]);
	  iVars += 2;
SKIP_DIGIT:
	  if (i >= 0)
      {
		if (BHZDT_NAME00 == iUseName)
        {
	      strcpy(szVars,szHZName0[i]);
		  iVars += strlen(szHZName0[i]);
		}
		else{
		  strcpy(szVars,szHZName1[i]);
		  iVars += strlen(szHZName1[i]);
		}
	  }
	  dwValue %= dwName;
	}
  }
  szVars[iVars] = 0;
  return iVars;
}
*/

#ifdef _X86

#ifdef _MSC_VER
static void cpuid(DWORD op,PDWORD peax,PDWORD pebx,PDWORD pecx,PDWORD pedx)
{
  __asm
  {
	push esi;
	mov eax,op;
    cpuid;
    mov esi,peax;
    mov [esi],eax;
	mov esi,pebx;
    mov [esi],ebx;
	mov esi,pecx;
    mov [esi],ecx;
	mov esi,pedx;
    mov [esi],edx;
	pop esi;
  }
}

void x86_DisableCpuid(void)
{
  __asm
  {
    mov ecx,119h
    rdmsr
    or eax,00200000h
    wrmsr
  }
}

static char g_szVendorAMD[]={"AuthenticAMD"};

PSTR x86_CPUName(PSTR vendor,WORD family,BYTE model,WORD family_ex,BYTE model_ex)
{
  if (0 == strcmp(vendor,"GenuineIntel"))
  {
	switch(family)
    {
	  case 4:
		   switch(model)
           {
			 MAP_TEXT2(0,"Intel 486 DX-25/33")
			 MAP_TEXT2(1,"Intel 486 DX-50")
			 MAP_TEXT2(2,"Intel 486 SX")
			 MAP_TEXT2(3,"Intel 486 DX2")
			 MAP_TEXT2(4,"Intel 486 SL")
			 MAP_TEXT2(5,"Intel 486 SX2")
			 MAP_TEXT2(7,"Intel 486 DX2-WB")
			 MAP_TEXT2(8,"Intel 486 DX4")
			 MAP_TEXT2(9,"Intel 486 DX4-WB")
		   }
		   break;
	  case 5:
		   switch(model)
           {
			 MAP_TEXT2(0,"Intel Pentium 60/66 A-step")
			 MAP_TEXT2(1,"Intel Pentium 60/66")
			 MAP_TEXT2(2,"Intel Pentium 75-200")
			 MAP_TEXT2(3,"Intel OverDrive PODP5V83")
			 MAP_TEXT2(4,"Intel Pentium MMX")
			 MAP_TEXT2(7,"Intel Mobile Pentium 75-200")
			 MAP_TEXT2(8,"Intel Mobile Pentium MMX")
		   }
		   break;
	  case 6:
		   switch(model)
           {
			 MAP_TEXT2(1,"Intel Pentium Pro A-Step")
			 MAP_TEXT2(2,"Intel Pentium Pro")
			 MAP_TEXT2(3,"Intel Pentium II Klamath")
			 MAP_TEXT2(5,"Intel Pentium II Deschutes")
			 MAP_TEXT2(6,"Intel Celeron Mendocino")
			 MAP_TEXT2(7,"Intel Pentium III Katmai")
			 MAP_TEXT2(8,"Intel Pentium III Coppermine")
			 MAP_TEXT2(9,"Intel Mobile Pentium III")
			 MAP_TEXT2(10,"Intel Pentium III (0.18um)")
			 MAP_TEXT2(11,"Intel Pentium III (0.13um)")
             //?
			 MAP_TEXT2(13,"Intel Mobile Pentium III")
		   }
		   break;
	  case 7:
	       return "Intel Itanium";
	  case 15:
           switch(family_ex)
           {
			 case 0:
			      switch(model_ex)
                  {
					case 0:
					case 1:
					     return "Intel Pentium IV (0.18um)";
					MAP_TEXT2(2,"Intel Pentium IV (0.13um)")
					MAP_TEXT2(3,"Intel Pentium IV (0.09um)")
				  }
				  break;
			 MAP_TEXT2(1,"Intel Itanium 2 (IA-64)")
		   }
		   break;			
    }
  }
  else if (0 == strcmp(vendor,g_szVendorAMD))
  {
	switch(family)
    {
	  case 4:
		   switch(model)
           {
			 MAP_TEXT2(3,"AMD 486 DX2")
			 MAP_TEXT2(7,"AMD 486 DX2-WB")
			 MAP_TEXT2(8,"AMD 486 DX4")
			 MAP_TEXT2(9,"AMD 486 DX4-WB")
			 MAP_TEXT2(14,"AMD Am5x86-WT")
			 MAP_TEXT2(15,"AMD Am5x86-WB")
		   }
		   break;
	  case 5:
		   switch(model)
           {
			 MAP_TEXT2(0,"AMD K5 SSA5")
			 case 1:
			 case 2:
			 case 3:
			      return "AMD K5";
             case 6:
			 case 7:
			      return "AMD K6";
			 MAP_TEXT2(8,"AMD K6-2")
			 MAP_TEXT2(9,"AMD K6-3")
			 MAP_TEXT2(13,"AMD K6-3+")
		   }
		   break;
	  case 6:
		   switch(model)
           {
			 case 0:
			 case 1:
			      return "AMD Athlon (25um)";
			 MAP_TEXT2(2,"AMD Athlon (18um)")
			 MAP_TEXT2(3,"AMD Duron")
			 MAP_TEXT2(4,"AMD Athlon Thunderbird")
			 MAP_TEXT2(6,"AMD Athlon Palamino")
			 MAP_TEXT2(7,"AMD Duron Morgan")
			 MAP_TEXT2(8,"AMD Athlon Thoroughbred")
			 MAP_TEXT2(10,"AMD Athlon Barton")
		   }
		   break;
	  case 15:
		   switch(family_ex)
           {
			 case 0:
				  switch(model_ex)
                  {
					MAP_TEXT2(4,"AMD Athlon 64")
					MAP_TEXT2(5,"AMD Athlon 64FX Operon")
				  }
                  break;
		   }
           break;
    }
  }
  else if (0 == strcmp(vendor,"CyrixInstead")) return "Cyrix";
  else if (0 == strcmp(vendor,"CentaurHauls")) return "Centaur";
  return "unknown";
}

DWORD x86_GetCpuFeature(PProcessorInfo pPI) 
{
  union{
    char vendor_name[13];// vendor name
    struct{
      DWORD ebx;
      DWORD edx;
      DWORD ecx;
    };
  }u;
  DWORD standard;
  DWORD feature;
  DWORD extended;
  DWORD cpu_support;
  DWORD os_support;

  memset(&u,0,sizeof(u));
  standard    = 0;
  feature     = 0;
  extended    = 0;
  cpu_support = 0;
  os_support  = 0;
  if (NULL != pPI) memset(pPI,0,sizeof(NProcessorInfo));
  /*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Step 1: Check if processor has CPUID support. Processor faults
    ;; with an illegal instruction exception if the instruction is not
    ;; supported. This step catches the exception and immediately returns
    ;; with feature string bits with all 0s, if the exception occurs.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/
  __try
  {
   __asm
   {
     xor eax,eax
     cpuid 
    }
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    return 0;
  }
  cpu_support |= FEATURE_CPUID;
  __asm
  {
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    //;; Check if CPUID supports function 1 (signature/std features)
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor     eax, eax                       //; CPUID function #0
    cpuid                                  //; largest std func/vendor string
    mov     u.ebx,ebx
    mov     u.edx,edx
    mov     u.ecx,ecx
  
    test    eax, eax                       //; largest standard function==0?
    jz      $all_done          //; yes, no standard features func
  
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    //;; Get standard feature flags and signature
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov     eax, 1                        //; CPUID function #1
    cpuid                                 //; get signature/std feature flgs
    mov     [standard], eax              //; save processor signature
	mov     [feature], edx

    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    //;; Check for CPUID extended functions
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov     eax, 0x80000000                //; extended function 0x80000000
    cpuid                                  //; largest extended function
    cmp     eax, 0x80000000                //; no function > 0x80000000 ?
    jbe     $all_done          //; yes, no extended feature flags

    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    //;; Get extended feature flags
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov     eax, 0x80000001               //; CPUID ext. function 0x80000001
    cpuid                                 //; EDX = extended feature flags
	mov     [extended],edx

    //;; Check for 3DNow! support (vendor independent)
    mov     ecx, CPUID_EXT_3DNOW          //; bit 31 indicates 3DNow! supprt
    and     ecx, edx                      //; supp 3DNow! ?CPUID_EXT_3DNOW:0
    neg     ecx                           //; supports 3DNow! ? CY : NC
    sbb     ecx, ecx                      //; supports 3DNow! ? 0x0Ffffffff:0
    and     ecx, FEATURE_3DNOW            //; support 3DNow!?FEATURE_3DNOW:0
    or      cpu_support, ecx                 //; merge into feature flags

    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    //;; Determine CPU vendor
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    lea     esi, g_szVendorAMD                //; AMD's vendor string
    lea     edi, u.vendor_name          //; this CPU's vendor string
    mov     ecx, 12                       //; strings are 12 characters
    cld                                   //; compare lowest to highest
    repe    cmpsb                         //; current vendor str == AMD's ?
    jnz     $all_done                      //; no, CPU vendor is not AMD

    //;; Check support for AMD-K6 processor-style MTRRs
    mov     eax, [standard] 	//; get processor signature
    and     eax, 0x0FFF 		//; extract family/model/stepping
    cmp     eax, 0x0588 		//; CPU < AMD-K6-2/CXT ? CY : NC
    sbb     edx, edx 		//; CPU < AMD-K6-2/CXT ? 0x0Ffffffff:0
    not     edx 			//; CPU < AMD-K6-2/CXT ? 0:0x0Ffffffff
    cmp     eax, 0x0600 	//; CPU < AMD Athlon ? CY : NC
    sbb     ecx, ecx 		//; CPU < AMD-K6 ? 0x0Ffffffff:0
    and     ecx, edx 		//; (CPU>=AMD-K6-2/CXT)&&
				//; (CPU<AMD Athlon) ? 0x0Ffffffff:0
    and     ecx, FEATURE_K6_MTRR 	//; (CPU>=AMD-K6-2/CXT)&&
				//; (CPU<AMD Athlon) ? FEATURE_K6_MTRR:0
    or      cpu_support, ecx 		//; merge into feature flags

$all_done:
  }
   /* The FP part of SSE introduces a new architectural state and therefore
      requires support from the operating system. So even if CPUID indicates
      support for SSE FP, the application might not be able to use it. If
      CPUID indicates support for SSE FP, check here whether it is also
      supported by the OS, and turn off the SSE FP feature bit if there
      is no OS support for SSE FP.

      Operating systems that do not support SSE FP return an illegal
      instruction exception if execution of an SSE FP instruction is performed.
      Here, a sample SSE FP instruction is executed, and is checked for an
      exception using the (non-standard) __try/__except mechanism
      of Microsoft Visual C.
  */
  //;; Check for FPU support
  if (feature & CPUID_STD_FPU)
  {
    cpu_support |= FEATURE_FPU;
  }
  //;; Check for time stamp counter support
  if (feature & CPUID_STD_TSC)
  {
    cpu_support |= FEATURE_TSC;
  }
  //;; Check for MMX support
  if (feature & CPUID_STD_MMX)
  {
    cpu_support |= FEATURE_MMX;
  }
  //;; Check for CMOV support
  if (feature & CPUID_STD_CMOV)
  {
    cpu_support |= FEATURE_CMOV;
  }
  //;; Check support for P6-style MTRRs
  if (feature & CPUID_STD_MTRR)
  {
    cpu_support |= FEATURE_P6_MTRR;
  }
  //;; Check for initial SSE support. There can still be partial SSE
  //;; support. Step 9 will check for partial support.
  if (feature & CPUID_STD_SSE)
  {
    cpu_support |= FEATURE_MMXEXT | FEATURE_SSE;
  }
  if (feature & CPUID_STD_SSE2)
  {
    cpu_support |= FEATURE_SSE2;
  }
  //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  //;; Check AMD specific extended features
  //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  if (extended & CPUID_EXT_AMD_3DNOWEXT)
  {
    cpu_support |= FEATURE_3DNOWEXT;
  }
  //;; Check support for AMD's multimedia instruction set additions
  if (extended & CPUID_EXT_AMD_MMXEXT)
  {
    cpu_support |= FEATURE_MMXEXT;
  }
  os_support = cpu_support;
  if (os_support & CPUID_STD_MMX)
  {
    __try
    {
      __asm
      {
        //pxor mm0,mm0 // MMX Sets the 64-bit value to zero 
		_emit 0x0F
		_emit 0xEF
		_emit 0xC0
		emms
      }
	}
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      os_support &= ~FEATURE_MMX;
	}
  }
  if (os_support & FEATURE_SSE)
  {
	__try
    {
      __asm
      {
        //xorps xmm0,xmm0 // SSE Bitwise Exclusive OR 
		_emit 0x0F
		_emit 0x57
		_emit 0xC0
      }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      os_support &= ~FEATURE_SSE;
    }
  }
  if (os_support & FEATURE_SSE2)
  {
    __try
    {
      __asm
      {
        //xorpd xmm0,xmm0 // SSE2 Sets the 2 double-precision, floating-point values to zero 
		_emit 0x66
		_emit 0x0F
		_emit 0x57
		_emit 0xC0
      }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      os_support &= ~FEATURE_SSE2;
    }
  }
  if (os_support & FEATURE_3DNOW)
  {
	__try
    {
      __asm
      {
        //pfrcp mm0,mm0 // 3DNow! Packed floating-point reciprocal approximation 
        _emit 0x0f
		_emit 0x0f
		_emit 0xc0
		_emit 0x96
        emms
      }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
	  os_support &= ~FEATURE_3DNOW;
	}
  }
  if (NULL != pPI)
  {
    pPI->standard    = standard;
	pPI->feature     = feature;
	pPI->extended    = extended;
	pPI->cpu_support = cpu_support;
	pPI->os_support  = os_support;
    strncpy(pPI->vendor_name,u.vendor_name,sizeof(pPI->vendor_name));

	pPI->family = (WORD)((pPI->standard >> 8) & 0x0F);// retrieve family 
    if (0x0F == pPI->family)
    {// retrieve extended family 
      pPI->family_ex = (pPI->standard >> 16) & 0x0FF0;
    }
    pPI->model = (BYTE)((pPI->standard >> 4) & 0x0F);// retrieve model 
    if (0x0F == pPI->model)
    {// retrieve extended model 
      pPI->model_ex = (pPI->standard >> 12) & 0x0F;
    }
    pPI->stepping = (BYTE)(pPI->standard & 0x0F);// retrieve stepping
    pPI->model_name = x86_CPUName(pPI->vendor_name,pPI->family,pPI->model,pPI->family_ex,pPI->model_ex);
  
	pPI->nSpeed = x86_GetCpuSpeed();
  }
  return os_support;
}

unsigned __int64 x86_GetCpuSpeed(void) 
{
  LARGE_INTEGER start,stop;
  LARGE_INTEGER nCtr,nFreq,nCtrStop;
			  
  QueryPerformanceFrequency(&nFreq);
  __asm _emit 0x0F
  __asm _emit 0x31
  __asm mov DWORD PTR start, eax
  __asm mov DWORD PTR [start+4], edx
  QueryPerformanceCounter(&nCtrStop);
  nCtrStop.QuadPart += nFreq.QuadPart;
  do{
	QueryPerformanceCounter(&nCtr);
  }while(nCtr.QuadPart < nCtrStop.QuadPart);

  __asm _emit 0x0F
  __asm _emit 0x31
  __asm mov DWORD PTR stop, eax
  __asm mov DWORD PTR [stop+4], edx

  return (stop.QuadPart-start.QuadPart);
}

#else //!_MSC_VER

static inline void cpuid(DWORD op,PDWORD eax,PDWORD ebx,PDWORD ecx,PDWORD edx)
{
  __asm__("cpuid"
          : "=a" (*eax),
            "=b" (*ebx),
            "=c" (*ecx),
            "=d" (*edx)
          : "0" (op));
}

#endif //!_MSC_VER
#endif //!_X86

/*
用CPUID指令，首先你可以确定你用的CPU是Intel的。
mov eax,0
cpuid
然后执行：
mov eax,1
cpuid
如果返回的EDX中，低18位为1，那么这个CPU就是支持序列号的。
此时EAX就是序列号的高32位。这32位对同一型号的CPU是一样的。
再执行：
mov eax,3
cpuid
此时的EDX:ECX就是序列号的第64位。
*/

#ifdef _X86
BOOL x86_Cpuid(PSTR vendor_id,PDWORD serial_no)
{
  DWORD dummy;

  //取生产商
  cpuid(0,&dummy,(PDWORD)&vendor_id[0],(PDWORD)&vendor_id[8],(PDWORD)&vendor_id[4]);
  vendor_id[12] = 0;
  //取低32位
  cpuid(1,&serial_no[1],&dummy,&dummy,&serial_no[0]);
  if (!FLAGON_ALL(serial_no[0],0x3FF)) return FALSE;
  //取高64位
  cpuid(3,&dummy,&dummy,&serial_no[3],&serial_no[2]);
  return TRUE;
}
#endif //_X86

BOOL GetMachineFinger(PSTR szResult,size_t cbResult)
{
#ifdef _X86
  char  vendor_id[13];
  DWORD serial_no[4];

  if (!x86_Cpuid(vendor_id,serial_no)) return FALSE;
  strcpyV(szResult,cbResult,"%s-%08X-%08X-%08X-%08X",vendor_id,serial_no[0],serial_no[1],serial_no[2],serial_no[3]);
  return TRUE;
#else //!_X86
  return system_ex(szResult,cbResult,"uname -a");
#endif //!_X86
}

char V2H(int value)
{
  if (value < 0x0A) return '0' + value;
  if (value < 0x10) return 'A' + value - 0x0A;
  return '0';
}

BYTE H2V(char value)
{
  if ('A' <= value && value <= 'Z') return (BYTE)(value - 'A' + 10);
  if ('a' <= value && value <= 'z') return (BYTE)(value - 'a' + 10);
  return (BYTE)(value - '0');
}

BYTE H2B(PSTR value)
{
  return (H2V(value[0]) << 4) + H2V(value[1]);
}

WORD H2W(PSTR value)
{
  return (((WORD)H2B(&value[0])) << 8) + H2B(&value[2]);
}

DWORD H2DW(PSTR value)
{
  return (((DWORD)H2W(&value[0])) << 16) + H2W(&value[4]);
}

DWORD HEX2BIN(PSTR szText,size_t cbText,PBYTE pData,DWORD dwData)
{
  DWORD i;

  if (-1 == cbText) cbText = strlen(szText);
  cbText /= 2;
  if (dwData > cbText) dwData = cbText;
  for(i=0;i<dwData;i++)
  {
    if (!isxdigit(szText[i*2]) || !isxdigit(szText[i*2+1])) break;
    pData[i] = H2B(&szText[i*2]);
  }
  return i;
}

PBYTE HEX2BIN_Ex(PSTR szText,size_t cbText,PDWORD pdwData)
{
  DWORD i;
  PBYTE pData;

  if (-1 == cbText) cbText = strlen(szText);
  if (NULL == (pData=(PBYTE)MemCalloc(*pdwData,1))) return NULL;
  for(i=0;i<*pdwData;i++)
  {
    if (!isxdigit(szText[i*2]) || !isxdigit(szText[i*2+1])) break;
    pData[i] = H2B(&szText[i*2]);
  }
  *pdwData = i;
  return pData;
}

DWORD atoix(PSTR sText,int nText)
{
  int   i,nSize;
  char  szBuff[32];
  DWORD dwValue;

  nSize = strlen(sText);
  if (-1 == nText || nSize < nText) nText = nSize;
  if (nText > sizeof(szBuff)-1) nText = sizeof(szBuff)-1;
  memcpy(szBuff,sText,nText);
  szBuff[nText] = 0;
  for(i=0;i<nText;i++)
  {
    if (isalpha(szBuff[i])) break;
  }
  if (i==nText) return atoi(szBuff);
  if ('x'==szBuff[i] || 'X'==szBuff[i]) i++;
  else i = 0;
  for(dwValue=0;i<nText;i++)
  {
    dwValue *= 16;
    dwValue += H2V(szBuff[i]);
  }
  return dwValue;
}

void ftoa(PSTR szText,DWORD cbText,double fValue)
{
  size_t i;

  strcpyV(szText,cbText,"%lf",fValue);
  i = strlen(szText) - 1;
  for(;i>0;i--)
  {
	if ('0' != szText[i])
    {
	  if ('.' == szText[i])
      {
		szText[i] = 0;
	  }
	  else{
		szText[i+1] = 0;
	  }
	  break;
	}
  }
}

BOOL STDIN_GetTextVP(PSTR szName,int cbName,PSTR szFMT,va_list vp)
{
  PSTR token;

  vfprintf(stdout,szFMT,vp);
  fgets(szName,cbName,stdin);
  token = &szName[strlen(szName)-1];
  if ('\n' == *token) *token = 0;
  return TRUE;
}

BOOL STDIN_GetValueVP(PDWORD pdwValue,PSTR szFMT,va_list vp)
{
  char szText[256];

  vfprintf(stdout,szFMT,vp);
  fgets(szText,sizeof(szText),stdin);
  if (!isdigit(szText[0])) return FALSE;
  *pdwValue = atoi(szText);
  return TRUE;
}

void swap(PVOID pt0,PVOID pt1,UINT uSize)
{
  UINT i;
  char ch;
  
  for(i=0;i<uSize;i++)
  {
    ch = ((PSTR)pt0)[i];
    ((PSTR)pt0)[i] = ((PSTR)pt1)[i];
    ((PSTR)pt1)[i] = ch;
  }
}

void wswap(PWORD pw0,PWORD pw1)
{
  WORD wtmp;

  wtmp = *pw0;
  *pw0 = *pw1;
  *pw1 = wtmp;
}

void dwswap(PDWORD pdw0,PDWORD pdw1)
{
  DWORD dwtmp;

  dwtmp = *pdw0;
  *pdw0 = *pdw1;
  *pdw1 = dwtmp;
}

#ifdef WIN32
BOOL CreateProcessEx(PSTR szProg,PSTR szCmdLine,BOOL bWait)
{
  STARTUPINFO         nSI;
  PROCESS_INFORMATION nPI;

  __API_ENTER("CreateProcessEx",BOOL,TRUE);
  memset(&nSI,0,sizeof(nSI));
  nSI.cb          = sizeof(nSI);
  nSI.dwFlags     = STARTF_USESHOWWINDOW;
  nSI.wShowWindow = SW_NORMAL;
  if (!CreateProcess(szProg,szCmdLine,NULL,NULL,TRUE,0,NULL,NULL,&nSI,&nPI))
  {
    logError2("!CreateProcess(%s,%s)",szProg,szCmdLine);
    __API_CANCEL(FALSE);
  }
  if (bWait)
  {
    WaitForSingleObject(nPI.hProcess,INFINITE);
  }
  else
  {
    WaitForInputIdle(nPI.hProcess,INFINITE);
  }
  CloseHandle(nPI.hProcess);
  CloseHandle(nPI.hThread);
__API_END_POINT:
  __API_LEAVE("CreateProcessEx");
}

DWORD Rundll32(HWND HWindow,PSTR szModule,PSTR szFunction,PSTR lpCmdLine,int nCmdShow)
{
  BOOL      retCode;
  HINSTANCE hInstance;

  if (NULL == (hInstance=LoadLibrary(szModule))) return GetLastError();
  retCode = Rundll32h(HWindow,hInstance,szFunction,lpCmdLine,nCmdShow);
  FreeLibrary(hInstance);
  return retCode;
}

DWORD Rundll32h(HWND HWindow,HINSTANCE hInstance,PSTR szFunction,PSTR lpCmdLine,int nCmdShow)
{
  PFNEntryPoint EntryPoint;

  if (NULL == (EntryPoint=(PFNEntryPoint)GetProcAddress(hInstance,szFunction))) return GetLastError();
  EntryPoint(HWindow,hInstance,lpCmdLine,nCmdShow);
  return 0;
}

PVOID LoadResourceData(HINSTANCE hInstance,LPCSTR lpTemplateName,PDWORD pdwSize)
{
  HRSRC hResource;
	
  if (NULL == (hResource=FindResource(hInstance,lpTemplateName,RT_RCDATA))) return NULL;
  if (NULL != pdwSize) *pdwSize = SizeofResource(hInstance,hResource);
  return LoadResource(hInstance,hResource);
}

BOOL SaveResourceFile(HINSTANCE hInstance,LPCSTR lpTemplateName,PSTR szName)
{
  PVOID pData;
  DWORD dwSize;

  if (NULL == (pData=LoadResourceData(hInstance,lpTemplateName,&dwSize))) return FALSE;
  return fileSave(szName,pData,dwSize);
}
#endif //WIN32

/*
#define PAGE_ZERO	0
#define MEM_ZERO	0

PSTR PAGE_toString(DWORD dwPage)
{
  switch(dwPage)
  {
    MAP_TEXT(PAGE_ZERO)
    MAP_TEXT(PAGE_NOACCESS)
    MAP_TEXT(PAGE_READONLY)
    MAP_TEXT(PAGE_READWRITE)
    MAP_TEXT(PAGE_WRITECOPY)  
    MAP_TEXT(PAGE_EXECUTE)
    MAP_TEXT(PAGE_EXECUTE_READ)
    MAP_TEXT(PAGE_EXECUTE_READWRITE)
    MAP_TEXT(PAGE_EXECUTE_WRITECOPY)
    MAP_TEXT(PAGE_GUARD)
    MAP_TEXT(PAGE_NOCACHE)
    MAP_TEXT(PAGE_WRITECOMBINE)
  }
  return NULL;
}

PSTR MEM_toString(DWORD dwMem)
{
  switch(dwMem)
  {
    MAP_TEXT(MEM_ZERO)
    MAP_TEXT(MEM_COMMIT)
    MAP_TEXT(MEM_RESERVE)    
    MAP_TEXT(MEM_DECOMMIT)
    MAP_TEXT(MEM_RELEASE)
    MAP_TEXT(MEM_FREE)
    MAP_TEXT(MEM_PRIVATE)     
    MAP_TEXT(MEM_MAPPED)
    MAP_TEXT(MEM_RESET)
    MAP_TEXT(MEM_TOP_DOWN)
	MAP_TEXT(MEM_WRITE_WATCH)   
    MAP_TEXT(MEM_PHYSICAL)
    MAP_TEXT(MEM_LARGE_PAGES)
    MAP_TEXT(MEM_4MB_PAGES)
    MAP_TEXT(MEM_IMAGE)
  }
  return NULL;
}

void module_dump(HINSTANCE hInstance)
{
  char              szPath[_MAX_PATH];
  DWORD             i,dwRVA;
  PIMAGE_NT_HEADERS pINH;

  if (NULL == hInstance) hInstance = GetModuleHandle(NULL);
  if (NULL == (pINH=FindHeader((PVOID)hInstance))) return;
  GetModuleFileName(hInstance,szPath,sizeof(szPath));
  logMessage2("0x%08X is image %s",hInstance,PathFindFileName(szPath));
  for(i=0;i<IMAGE_NUMBEROF_DIRECTORY_ENTRIES;i++)
  {
    if (0 == (dwRVA=pINH->OptionalHeader.DataDirectory[i].VirtualAddress)) continue;
    dwRVA += (DWORD)hInstance;
	logMessage3("%d: VirtualAddress=0x%08X Size=0x%08X",i,dwRVA,pINH->OptionalHeader.DataDirectory[i].Size);
  }
}

void memory_search(PBYTE pAddr,DWORD dwSize,DWORD State,DWORD Type)
{
return;
  if (MEM_COMMIT != State) return;
  if (MEM_PRIVATE != Type) return;
  for(;;)
  {
    if (dwSize < 11) break;
	if (0 == memcmp("Message-ID:",pAddr,11))
    {
	  logMessage1("Message-ID found at 0x%08X",pAddr);
	  break;
	}
	pAddr++;
	dwSize--;
  }
//Message-ID: <reuiqxgkqknw57c.220620041606@lordor.n
}

void memory_dump(void)
{
  DWORD                    dwAddr;
  MEMORY_BASIC_INFORMATION nMBI;

  for(dwAddr=0;;)
  {
    if (0 == VirtualQuery((PVOID)dwAddr,&nMBI,sizeof(nMBI)))
    {
	  logError1("!VirtualQuery(0x%08X)",dwAddr);
	  for(;dwAddr<0xFFFE0000;dwAddr+=0x00001000)
      {
	    if (0 != VirtualQuery((PVOID)dwAddr,&nMBI,sizeof(nMBI))) break;
	  }
	  if (dwAddr > 0xFFFE0000) break;
	}
	logMessage6("0x%08X: RegionSize=0x%08X AllocationProtect=%s Protect=%s State=%s Type=%s",
	            nMBI.BaseAddress,nMBI.RegionSize,
	            PAGE_toString(nMBI.AllocationProtect),PAGE_toString(nMBI.Protect),MEM_toString(nMBI.State),MEM_toString(nMBI.Type));
	memory_search((PBYTE)nMBI.BaseAddress,nMBI.RegionSize,nMBI.State,nMBI.Type);
	
	dwAddr += nMBI.RegionSize;

	if (MEM_IMAGE == nMBI.Type)
    {
	  module_dump((HINSTANCE)nMBI.BaseAddress);
	}
  }
}
static BOOL bStarted = FALSE;

  {
    FILE *fp;
	char szName[_MAX_PATH];

    strcpyV(szName,sizeof(szName),"c:\\0x%08X_encrypt.dat",hKey);
	if (NULL != (fp=fopen(szName,"ab")))
    {
	  fwrite(pbData,*pdwDataLen,1,fp);
	  fclose(fp);
	}
  }
  if (!bStarted)
  {
    MEMORY_BASIC_INFORMATION nMBI;

    memory_dump();

    bStarted = TRUE;
    if (0 != VirtualQuery(pbData,&nMBI,sizeof(nMBI)))
    {
	  logMessage7("Mine_CryptEncrypt 0x%08X,0x%08X: RegionSize=0x%08X AllocationProtect=%s Protect=%s State=%s Type=%s",
	              pbData,nMBI.BaseAddress,nMBI.RegionSize,
	              PAGE_toString(nMBI.AllocationProtect),PAGE_toString(nMBI.Protect),MEM_toString(nMBI.State),MEM_toString(nMBI.Type));
      if (MEM_COMMIT == nMBI.State)
      {
	    char szPath[_MAX_PATH];

        strcpyV(szPath,sizeof(szPath),"c:\\0x%08X_0x%08X_encrypt.dat",pbData,nMBI.BaseAddress);
	    fileSave(szPath,nMBI.BaseAddress,nMBI.RegionSize);
	  }
    }
  }
  else{
    if (Final && NULL != pbData) bStarted = FALSE;
  }
*/

#ifdef WIN32
#define Off2Ptr(type,head,pos)		(type*)((PSTR)head + pos)
PIMAGE_NT_HEADERS FindHeader(PVOID pAddr)
{
  PIMAGE_DOS_HEADER pIDH;
  PIMAGE_NT_HEADERS pINH;

  if (NULL == (pIDH=(PIMAGE_DOS_HEADER)pAddr)) return NULL;
  if (IsBadReadPtr(pIDH,sizeof(*pIDH))) return NULL;
  if (IMAGE_DOS_SIGNATURE != pIDH->e_magic) return NULL;
  pINH = Off2Ptr(IMAGE_NT_HEADERS,pIDH,pIDH->e_lfanew);
  if (IsBadReadPtr(pINH,sizeof(*pINH))) return NULL;
  if (IMAGE_NT_SIGNATURE != pINH->Signature) return NULL;
  return pINH;
}

void GetCallerEx(PVOID eip,PSTR szName,DWORD cbName)
{
  PBYTE pAddr;

  szName[0] = 0;
  pAddr = (PBYTE)((DWORD)eip & 0xFFFFF000);
  for(;;pAddr-=0x00001000)
  {
    if (0 == pAddr) break;
    if (NULL == FindHeader(pAddr)) continue;
	GetModuleFileName((HINSTANCE)pAddr,szName,cbName);
	break;
  }
}

#endif //WIN32

PSTR LTI_Data2Text(PLongTextItem pLTI,DWORD dwLTI,DWORD dwData)
{
  DWORD i;

  for(i=0;i<dwLTI;i++)
  {
    if (pLTI[i].dwData == dwData) return pLTI[i].szData;
  }
  return NULL;
}

PSTR LTI_Data2TextHex(PLongTextItem pLTI,DWORD dwLTI,DWORD dwData,PSTR szText,DWORD dwText)
{
  PSTR pText;

  if (NULL != (pText=LTI_Data2Text(pLTI,dwLTI,dwData))) return pText;
  strcpyV(szText,dwText,"0x%08X",dwData);
  return szText;
}

PSTR LTI_Data2TextInt(PLongTextItem pLTI,DWORD dwLTI,DWORD dwData,PSTR szText,DWORD dwText)
{
  PSTR pText;

  if (NULL != (pText=LTI_Data2Text(pLTI,dwLTI,dwData))) return pText;
  strcpyV(szText,dwText,"%u",dwData);
  return szText;
}

BOOL LTI_Text2Data(PLongTextItem pLTI,DWORD dwLTI,PSTR szData,PDWORD pdwData)
{
  DWORD i;

  for(i=0;i<dwLTI;i++)
  {
    if (0 == strcmp(pLTI[i].szData,szData))
    { 
      *pdwData = pLTI[i].dwData;
      return TRUE;
    }
  }
  *pdwData = atoix(szData,-1);
  return NULL != LTI_Data2Text(pLTI,dwLTI,*pdwData);
}

BOOL LLI_Data2Long(PLongLongItem pLLI,DWORD dwLLI,DWORD dwData,PDWORD pdwLong)
{
  DWORD i;

  for(i=0;i<dwLLI;i++)
  {
    if (pLLI[i].dwData == dwData)
    { 
      *pdwLong = pLLI[i].dwLong;
      return TRUE;
    }
  }
  return FALSE;
}

BOOL LLI_Long2Data(PLongLongItem pLLI,DWORD dwLLI,DWORD dwLong,PDWORD pdwData)
{
  DWORD i;

  for(i=0;i<dwLLI;i++)
  {
    if (pLLI[i].dwLong == dwLong)
    { 
      *pdwData = pLLI[i].dwData;
      return TRUE;
    }
  }
  return FALSE;
}

#ifdef WIN32

//from M$ CRT source
static void parse_cmdline(char * cmdstart,char * *argv,char * args,int *numargs,int *numchars)
{
        char *p;
        char c;
        int inquote;                    /* 1 = inside quotes */
        int copychar;                   /* 1 = copy char to *args */
        unsigned numslash;              /* num of backslashes seen */

        *numchars = 0;
        *numargs = 1;                   /* the program name at least */

        /* first scan the program name, copy it, and count the bytes */
        p = cmdstart;
        if (argv)
            *argv++ = args;

        /* A quoted program name is handled here. The handling is much
           simpler than for other arguments. Basically, whatever lies
           between the leading double-quote and next one, or a terminal null
           character is simply accepted. Fancier handling is not required
           because the program name must be a legal NTFS/HPFS file name.
           Note that the double-quote characters are not copied, nor do they
           contribute to numchars. */
        if ( *p == '\"' ) {
            /* scan from just past the first double-quote through the next
               double-quote, or up to a null, whichever comes first */
            while ( (*(++p) != '\"') && (*p != 0) ) {

#ifdef _MBCS
                if (IsDBCSLeadByte(*p)) {
                    ++*numchars;
                    if ( args )
                        *args++ = *p++;
                }
#endif  /* _MBCS */
                ++*numchars;
                if ( args )
                    *args++ = *p;
            }
            /* append the terminating null */
            ++*numchars;
            if ( args )
                *args++ = 0;

            /* if we stopped on a double-quote (usual case), skip over it */
            if ( *p == '\"' )
                p++;
        }
        else {
            /* Not a quoted program name */
            do {
                ++*numchars;
                if (args)
                    *args++ = *p;

                c = *p++;
#ifdef _MBCS
                if (IsDBCSLeadByte(c)) {
                    ++*numchars;
                    if (args)
                        *args++ = *p;   /* copy 2nd byte too */
                    p++;  /* skip over trail byte */
                }
#endif  /* _MBCS */

            } while ( c != ' ' && c != 0 && c != '\t' );

            if ( c == 0 ) {
                p--;
            } else {
                if (args)
                    *(args-1) = 0;
            }
        }

        inquote = 0;

        /* loop on each argument */
        for(;;) {

            if ( *p ) {
                while (*p == ' ' || *p == '\t')
                    ++p;
            }

            if (*p == 0)
                break;              /* end of args */

            /* scan an argument */
            if (argv)
                *argv++ = args;     /* store ptr to arg */
            ++*numargs;


        /* loop through scanning one argument */
        for (;;) {
            copychar = 1;
            /* Rules: 2N backslashes + " ==> N backslashes and begin/end quote
               2N+1 backslashes + " ==> N backslashes + literal "
               N backslashes ==> N backslashes */
            numslash = 0;
            while (*p == '\\') {
                /* count number of backslashes for use below */
                ++p;
                ++numslash;
            }
            if (*p == '\"') {
                /* if 2N backslashes before, start/end quote, otherwise
                    copy literally */
                if (numslash % 2 == 0) {
                    if (inquote) {
                        if (p[1] == '\"')
                            p++;    /* Double quote inside quoted string */
                        else        /* skip first quote char and copy second */
                            copychar = 0;
                    } else
                        copychar = 0;       /* don't copy quote */

                    inquote = !inquote;
                }
                numslash /= 2;          /* divide numslash by two */
            }

            /* copy slashes */
            while (numslash--) {
                if (args)
                    *args++ = '\\';
                ++*numchars;
            }

            /* if at end of arg, break loop */
            if (*p == 0 || (!inquote && (*p == ' ' || *p == '\t')))
                break;

            /* copy character into argument */
#ifdef _MBCS
            if (copychar) {
                if (args) {
                    if (IsDBCSLeadByte(*p)) {
                        *args++ = *p++;
                        ++*numchars;
                    }
                    *args++ = *p;
                } else {
                    if (IsDBCSLeadByte(*p)) {
                        ++p;
                        ++*numchars;
                    }
                }
                ++*numchars;
            }
            ++p;
#else  /* _MBCS */
            if (copychar) {
                if (args)
                    *args++ = *p;
                ++*numchars;
            }
            ++p;
#endif  /* _MBCS */
            }

            /* null-terminate the argument */

            if (args)
                *args++ = 0;          /* terminate string */
            ++*numchars;
        }

        /* We put one last argument in -- a null ptr */
        if (argv)
            *argv++ = NULL;
        ++*numargs;
}

PVOID get_argcargv(int *argc,char ***argv)
{
  int  numargs,numchars;
  char *p,*szCmdLine;

  szCmdLine = GetCommandLine();
  parse_cmdline(szCmdLine,NULL,NULL,&numargs,&numchars);
  if (NULL == (p=(char*)MemMalloc(numargs * sizeof(char*) + numchars))) return NULL;
  parse_cmdline(szCmdLine,(char**)p,p + numargs * sizeof(char *),&numargs,&numchars);
  *argc  = numargs - 1;
  *argv = (char**)p;
  return p;
}

size_t getPWD(PSTR szText,size_t cbText,PSTR szPrompt)
{
  size_t i,j;

  fprintf(stdout,"%s",szPrompt);
  cbText--;
  for(i=0;i<cbText;)
  {
    if ('\r' == (szText[i]=_getch())) break;
    switch(szText[i])
    {
      case '\b':
           if (0 == i) break;
           fprintf(stdout,"\r%s",szPrompt);
           for(j=0;j<i;j++)
           {
             fprintf(stdout," ");
           }
           i--;
           fprintf(stdout,"\r%s",szPrompt);
           for(j=0;j<i;j++)
           {
             fprintf(stdout,"*");
           }
           break;
      default:
           fprintf(stdout,"*");
           i++;
           break;
    }
  }
  szText[i] = 0;
  fprintf(stdout,"\n");
  return i;
}

#endif //WIN32

BOOL GetStringVersion(PSTR szVersion,PWORD pwMSH,PWORD pwMSL,PWORD pwLSH,PWORD pwLSL)
{
  char szLine[256];
  PSTR token,chTemp;

  __API_ENTER("GetStringVersion",BOOL,TRUE);
  *pwMSH = 0;
  *pwMSL = 0;
  *pwLSH = 0;
  *pwLSL = 0;
  strcpyN(szLine,sizeof(szLine),szVersion);
  if (NULL == (token=strtokS0(szLine,&chTemp,".,:-"))) __API_CANCEL(FALSE);
  *pwMSH  = atoi(token);
  if (NULL == (token=strtokS1(&chTemp,".,:-"))) __API_FINISH();
  *pwMSL = atoi(token);
  if (NULL == (token=strtokS1(&chTemp,".,:-"))) __API_FINISH();
  *pwLSH = atoi(token);
  if (NULL == (token=strtokS1(&chTemp,".,:-"))) __API_FINISH();
  *pwLSL = atoi(token);
__API_END_POINT:
  __API_LEAVE("GetStringVersion");
}

int VersionCompare(WORD wMSH0,WORD wMSL0,WORD wLSH0,WORD wLSL0,WORD wMSH1,WORD wMSL1,WORD wLSH1,WORD wLSL1)
{
  __API_ENTER("VersionCompare",int,0);
  if (wMSH0 > wMSH1) __API_CANCEL(1);
  if (wMSH0 < wMSH1) __API_CANCEL(-1);
  if (wMSL0 > wMSL1) __API_CANCEL(1);
  if (wMSL0 < wMSL1) __API_CANCEL(-1);
  if (wLSH0 > wLSH1) __API_CANCEL(1);
  if (wLSH0 < wLSH1) __API_CANCEL(-1);
  if (wLSL0 > wLSL1) __API_CANCEL(1);
  if (wLSL0 < wLSL1) __API_CANCEL(-1);
__API_END_POINT:
  __API_LEAVE("VersionCompare");
}
