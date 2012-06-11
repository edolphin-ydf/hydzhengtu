#include <baseLib/regexLib.h>
#include <baseLib/strLib.h>

#ifdef WIN32
#ifdef _DLL
#ifdef _DEBUG
#pragma comment(lib,"regexD")
#else //!_DEBUG
#pragma comment(lib,"regexR")
#endif //!_DEBUG
#else //!_DLL
#ifdef _DEBUG
#pragma comment(lib,"regexSD")
#else //!_DEBUG
#pragma comment(lib,"regexSR")
#endif //!_DEBUG
#endif //!_DLL
#endif //WIN32

BOOL regex_Replace(regex_t *reg,PSTR szVar[],regmatch_t *pmatch,PSTR pSrc,PSTR pDst,DWORD dwDst)
{
  PSTR  pDstEnd;
  DWORD i;

  pDstEnd = pDst + dwDst - 1;
  for(;pDst<pDstEnd;){
    if (0 != regexec(reg,pSrc,reg->re_nsub,pmatch,0)){
	  strcpyN(pDst,pDstEnd - pDst,pSrc);
	  break;
    }
	memcpy(pDst,pSrc,pmatch[0].rm_so);
	pDst += pmatch[0].rm_so;
	for(i=1;i<reg->re_nsub;i++){
	  if (-1 != pmatch[i].rm_so) break;
	}
    strncpy(pDst,szVar[i-1],sizeof(pDst));
    pDst += strlen(pDst);
	pSrc += pmatch[0].rm_eo;
  }
  return TRUE;
}

BOOL regex_ReplaceEx(PSTR szPattern,PSTR szVar[],regmatch_t *pmatch,PSTR pSrc,PSTR pDst,DWORD dwDst)
{
  BOOL    retCode;
  regex_t reg;

  if (0 != regcomp(&reg,szPattern,REG_EXTENDED | REG_ICASE)) return FALSE;
  retCode = regex_Replace(&reg,szVar,pmatch,pSrc,pDst,dwDst);
  regfree(&reg);
  return TRUE;
}

BOOL regex_ReplaceExEx(DWORD isCount,PSTR szName[],PSTR szVar[],regmatch_t *pmatch,PSTR pSrc,PSTR pDst,DWORD dwDst)
{
  char  szPattern[2048];
  DWORD i;

  szPattern[0] = 0;
  for(i=0;i<isCount;i++){
    if (0 != i){
      strcatN(szPattern,sizeof(szPattern),"|");
    }
    strcatV(szPattern,sizeof(szPattern),"(%s)",szName[i]);
  }
  return regex_ReplaceEx(szPattern,szVar,pmatch,pSrc,pDst,dwDst);
}

void regex_Escape(PSTR pSrc,PSTR pDst,DWORD dwDst)
{
  PSTR  pDstEnd;

  pDstEnd = pDst + dwDst - 1;
  for(;0 != *pSrc && pDst<pDstEnd;){
    switch(*pSrc){
	  case '\\':
	  case '$':
	  case '^':
	  case '.':
	  case '[':
	  case ']':
	  case '(':
	  case ')':
	  case '{':
	  case '}':
	       *pDst++ = '\\';
	  default:
		   *pDst++ = *pSrc++;
		   break;
    }
  }
  *pDst = 0;
}

BOOL regex_Build(DWORD isCount,PSTR szName[],PSTR szVar[],PSTR pSrc,PSTR pDst,DWORD dwDst)
{
  PSTR  pSrcHead,pDstEnd;
  DWORD i;

  pSrcHead = pSrc;
  pDstEnd  = pDst + dwDst - 1;
  for(;0 != *pSrc && pDst<pDstEnd;){
    for(i=0;i<isCount;i++){
      if (striprefix(pSrc,szVar[i])) break;
    }
    if (i==isCount){
      *pDst++ = *pSrc++;
      *pDst   = 0;
    }
    else{
      strncpy(pDst,szName[i],sizeof(pDst));
      pDst += strlen(pDst);
      pSrc += strlen(szVar[i]);
    }
  }
  return TRUE;
}
