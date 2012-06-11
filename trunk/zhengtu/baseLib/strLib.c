#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <baseLib/strLib.h>
#include <baseLib/memLib.h>

PBYTE memmem(PBYTE pData,size_t dwData,PBYTE pMask,size_t dwMask)
{
  PBYTE pHead;

  for(pHead=pData;pHead<pData+dwData-dwMask;pHead++){
    if (0 == memcmp(pHead,pMask,dwMask)) return pHead;
  }
  return NULL;
}

BOOL ISSPACE(char c)
{
  if ((BYTE)c > 0xA0) return FALSE;
  return isspace(c);
}

void strcpyN(PSTR szDst,size_t cbDst,PSTR szSrc)
{
  cbDst--;
  strncpy(szDst,szSrc,cbDst);
  szDst[cbDst] = 0;
}

void strcatN(PSTR szDst,size_t cbDst,PSTR szSrc)
{
  size_t cbSize;

  cbSize = strlen(szDst);
  cbDst--;
  strncpy(&szDst[cbSize],szSrc,cbDst - cbSize);
  szDst[cbDst] = 0;
}

void strcpyNN(PSTR szDst,size_t cbDst,PSTR szSrc,size_t cbSrc)
{
  size_t cbSize;

  cbDst--;
  cbSize = min(cbDst,cbSrc);
  strncpy(szDst,szSrc,cbSize);
  szDst[cbDst] = 0;
}

void strcpyV(PSTR szDst,size_t cbDst,PSTR szFMT,...)
{
  va_list vp;

  cbDst--;
  va_start(vp,szFMT);
  vsnprintf(szDst,cbDst,szFMT,vp);
  va_end(vp);
}

void strcatV(PSTR szDst,size_t cbDst,PSTR szFMT,...)
{
  size_t  cbSize;
  va_list vp;

  cbDst--;
  cbSize = strlen(szDst);
  va_start(vp,szFMT);
  vsnprintf(&szDst[cbSize],cbDst - cbSize,szFMT,vp);
  va_end(vp);
}

void strescape(PSTR szData,size_t cbData,PSTR szOut,size_t cbOut,BOOL asRC)
{
  size_t i,j;

  if (-1 == cbData) cbData = strlen(szData);
  cbOut--;
  for(i=j=0;i < cbData && j < cbOut;i++)
  {
    switch(szData[i])
    {
      case '%':
           szOut[j++] = '%';
           if (asRC) break;
           szOut[j++] = '%';
           break;
      case '\r':
           szOut[j++] = '\\';
           szOut[j++] = 'r';
           break;
      case '\n':
           szOut[j++] = '\\';
           szOut[j++] = 'n';
           break;
      case '\t':
           szOut[j++] = '\\';
           szOut[j++] = 't';
           break;
      case '\\':
           szOut[j++] = '\\';
           szOut[j++] = '\\';
           break;
      case '\"':
           szOut[j++] = asRC ? '\"' : '\\';
           szOut[j++] = '\"';
           break;
      default:
           szOut[j++] = szData[i];
           break;
    }
  }
  szOut[j] = 0;
}

void wcsescape(PWSTR szData,size_t cbData,PWSTR szOut,size_t cbOut,BOOL asRC)
{
  size_t i,j;

  if (-1 == cbData) cbData = wcslen(szData);
  cbOut--;
  for(i=j=0;i < cbData && j < cbOut;i++)
  {
    switch(szData[i])
    {
      case L'%':
           szOut[j++] = L'%';
           if (asRC) break;
           szOut[j++] = L'%';
           break;
      case L'\r':
           szOut[j++] = L'\\';
           szOut[j++] = L'r';
           break;
      case L'\n':
           szOut[j++] = L'\\';
           szOut[j++] = L'n';
           break;
      case L'\t':
           szOut[j++] = L'\\';
           szOut[j++] = L't';
           break;
      case L'\\':
           szOut[j++] = L'\\';
           szOut[j++] = L'\\';
           break;
      case L'\"':
           szOut[j++] = asRC ? L'\"' : L'\\';
           szOut[j++] = L'\"';
           break;
      default:
           szOut[j++] = szData[i];
           break;
    }
  }
  szOut[j] = 0;
}

PSTR strtokC0Ex(PSTR szLine,PPSTR pchTemp,char chSplit,PSTR endC)
{
  if (NULL == (*pchTemp=szLine)) return NULL;
  for(;0 != **pchTemp && chSplit != **pchTemp;(*pchTemp)++);
  if (NULL != endC) *endC = **pchTemp;
  if (0 == **pchTemp) *pchTemp = NULL;
  else *((*pchTemp)++) = 0;
  return szLine;
}

PSTR strtokC1Ex(PPSTR pchTemp,char chSplit,PSTR endC)
{
  return strtokC0Ex(*pchTemp,pchTemp,chSplit,endC);
}

PSTR strtokS0Ex(PSTR szLineIn,PPSTR szTmpPtr,PSTR szSpace,PSTR endC)
{
  BOOL bHead;
  PSTR szLine;

  if (NULL == (*szTmpPtr=(szLine=szLineIn))) return NULL;
  for(bHead=TRUE;0 != **szTmpPtr;(*szTmpPtr)++){
    if (NULL != strchr(szSpace,**szTmpPtr)){
	  if (!bHead) break;
	  if (!ISSPACE(**szTmpPtr)) break;
	}
	else{
	  if (bHead){
	    szLine = *szTmpPtr;
	    bHead = FALSE;
	  }
	}
  }
  if (NULL != endC) *endC = **szTmpPtr;
  if (0 == **szTmpPtr) *szTmpPtr = NULL;
  else *((*szTmpPtr)++) = 0;
  return szLine;
}

PSTR strtokS1Ex(PPSTR szTmpPtr,PSTR szSpace,PSTR endC)
{
  return strtokS0Ex(*szTmpPtr,szTmpPtr,szSpace,endC);
}

PSTR strtokC0(PSTR szLine,PPSTR pchTemp,char chSplit)
{
  return strtokC0Ex(szLine,pchTemp,chSplit,NULL);
}

PSTR strtokC1(PPSTR pchTemp,char chSplit)
{
  return strtokC1Ex(pchTemp,chSplit,NULL);
}

PSTR strtokS0(PSTR szLine,PPSTR szTmpPtr,PSTR szSpace)
{
  return strtokS0Ex(szLine,szTmpPtr,szSpace,NULL);
}

PSTR strtokS1(PPSTR szTmpPtr,PSTR szSpace)
{
  return strtokS1Ex(szTmpPtr,szSpace,NULL);
}

void strtrim(PSTR szText)
{
  UINT i,j,k;
      
  for(j=0;ISSPACE(szText[j]);j++);
  if (j==strlen(szText)){
    szText[0] = 0;
	return;
  }
  for(k=strlen(szText)-1;ISSPACE(szText[k]);k--);
  for(i=0;j<=k;j++) szText[i++] = szText[j];
  szText[i] = 0;
}

void strtrimEx(PSTR szText,PSTR szSpace)
{
  int i,j,k;
      
  for(j=0;strchr(szSpace,szText[j]);j++);
  for(k=strlen(szText)-1;strchr(szSpace,szText[k]);k--);
  for(i=0;j<=k;j++) szText[i++] = szText[j];
  szText[i] = 0;
}

BOOL strmatch(PSTR szText,PSTR szPattern)
{
  int i;
  
  for(i=0;;i++){
    switch(szPattern[i]){
      case 0:
           return 0 == szText[i];
      case '?':  
           if (0 == szText[i]) return FALSE;
           break;
      case '*':
           return TRUE;
      default:
           if (szPattern[i] != szText[i]) return FALSE;
           break;
    }
  }
  return TRUE;
}

BOOL strprefix(PSTR szData,PSTR szPrefix)
{
  int size1,size2;

  size1 = strlen(szData);
  size2 = strlen(szPrefix);
  if (size1 < size2) return FALSE;
  return 0 == memcmp(szData,szPrefix,size2);
}

BOOL striprefix(PSTR szData,PSTR szPrefix)
{
  int i,size1,size2;

  size1 = strlen(szData);
  size2 = strlen(szPrefix);
  if (size1 < size2) return FALSE;
  for(i=0;i<size2;i++){
    if (toupper(szData[i]) != toupper(szPrefix[i])) return FALSE;
  }
  return TRUE;
}


BOOL wcsprefix(PWSTR szData,PWSTR szPrefix)
{
  int size1,size2;

  size1 = wcslen(szData);
  size2 = wcslen(szPrefix);
  if (size1 < size2) return FALSE;
  return 0 == memcmp(szData,szPrefix,sizeof(WCHAR)*size2);
}

#ifndef HAVE_STRLWR
void _strlwr(PSTR szText)
{
  int i;

  for(i=0;0 != szText[i];i++) szText[i] = tolower(szText[i]);
}
#endif //!HAVE_STRLWR

#ifndef HAVE_STRUPR
void strupr(PSTR szText)
{
  int i;

  for(i=0;0 != szText[i];i++) szText[i] = toupper(szText[i]);
}
#endif //!HAVE_STRUPR

#ifndef HAVE_STRICMP
int stricmp(PSTR szL,PSTR szR)
{
  char cL,cR;

  for(;;){
    if (0 == (cL=*szL++)){
      if (0 == (cR=*szR++)) break;
      return -1;
    }
    else{
      if (0 == (cR=*szR++)) return 1;
    }
    cL = tolower(cL);
    cR = tolower(cR);
    if (cL < cR) return -1;
    if (cL > cR) return 1;
  }
  return 0;
}
#endif //!HAVE_STRICMP

#ifndef HAVE_MEMICMP
int memicmp(PSTR szL,PSTR szR,size_t size)
{
  char cL,cR;

  for(;size>0;size--){
    if (0 == (cL=*szL++)){
      if (0 == (cR=*szR++)) break;
      return -1;
    }
    else{
      if (0 == (cR=*szR++)) return 1;
    }
    cL = tolower(cL);
    cR = tolower(cR);
    if (cL < cR) return -1;
    if (cL > cR) return 1;
  }
  return 0;
}
#endif //!HAVE_MEMICMP

void mstrcpyV(PSTR szText,PSTR szFMT,...)
{
  va_list vp;

  va_start(vp,szFMT);
  vsprintf(szText,szFMT,vp);
  va_end(vp);
  szText[1+strlen(szText)] = 0;
}

void mstrcatV(PSTR szText,PSTR szFMT,...)
{
  PSTR    token;
  va_list vp;

  for(token=szText;0 != *token;){
	token += 1+strlen(token);
  }
  va_start(vp,szFMT);
  vsprintf(token,szFMT,vp);
  va_end(vp);
  token[1+strlen(token)] = 0;
}

PSTR astrcpy(PSTR szData)
{
  if (NULL == szData) return NULL;
  return strdup(szData);
}

PSTR astrcat(PSTR szText,PSTR szData)
{
  int  sizeOld,sizeNew;
  PSTR szNew;

  if (NULL == szText) return astrcpy(szData);
  sizeOld = strlen(szText);
  sizeNew = strlen(szData);
  if (NULL == (szNew=MemRealloc(szText,sizeOld+sizeNew+1))) return NULL;
  strncpy(&szNew[sizeOld],szData,szText,sizeOld+sizeNew+1);
  return szNew;
}

PSTR astrcpyV(PSTR szFMT,...)
{
  char    szData[8192];
  va_list vp;

  va_start(vp,szFMT);
  vsnprintf(szData,sizeof(szData),szFMT,vp);
  va_end(vp);
  return strdup(szData);
}

PSTR astrcatV(PSTR szText,PSTR szFMT,...)
{
  char    szData[8192];
  va_list vp;

  va_start(vp,szFMT);
  vsnprintf(szData,sizeof(szData),szFMT,vp);
  va_end(vp);
  return astrcat(szText,szData);
}

void _calcSize(PSTR szWord,size_t *pdwWord)
{
  if (-1 != *pdwWord) return;
  if (NULL == szWord) *pdwWord = 0;
  else *pdwWord = strlen(szWord);
}

BOOL _calcSizeEx(PSTR szWord,size_t *pdwWord,PSTR szCheck,size_t *pdwCheck)
{
  _calcSize(szWord,pdwWord);
  _calcSize(szCheck,pdwCheck);
  return *pdwCheck <= *pdwWord;
}

PSTR _wordchr(PSTR szWord,size_t dwWord,char chCheck)
{
  PSTR pHead,pTail;

  _calcSize(szWord,&dwWord);
  pHead = szWord;
  pTail = szWord + dwWord - 1;
  for(;pHead < pTail && chCheck != *pHead;pHead++);
  return pHead < pTail? pHead : NULL;
}

PSTR wordchr(PPerWord pWord,char chCheck,size_t *pdwPos)
{
  PSTR pPos;

  if (NULL != (pPos=_wordchr(pWord->szData0,pWord->dwData0,chCheck))){
    if (NULL != pdwPos) *pdwPos = pPos - pWord->szData0;
    return pPos;
  }
  if (NULL != (pPos=_wordchr(pWord->szData1,pWord->dwData1,chCheck))){
    if (NULL != pdwPos) *pdwPos = pPos - pWord->szData1 + pWord->dwData0;
    return pPos;
  }
  return NULL;
}

PSTR worddump(PPerWord pWord)
{
  PSTR  szData,pHead;
  size_t dwSize;

  dwSize = wordlen(pWord);
  if (NULL == (szData=MemCalloc(dwSize+1,1))) return NULL;
  pHead = szData;
  memcpy(pHead,pWord->szData0,pWord->dwData0);
  pHead += pWord->dwData0;
  memcpy(pHead,pWord->szData1,pWord->dwData1);
  pHead += pWord->dwData1;
  return szData;
}

void wordcopy(PPerWord pWord,PPSTR pszData)
{
  memcpy(*pszData,pWord->szData0,pWord->dwData0);
  (*pszData) += pWord->dwData0;
  memcpy(*pszData,pWord->szData1,pWord->dwData1);
  (*pszData) += pWord->dwData1;
}

BOOL wordcpy(PPerWord pWord,PSTR szData,size_t dwData)
{
  if (wordlen(pWord) >= dwData) return FALSE;
  memcpy(szData,pWord->szData0,pWord->dwData0);
  szData += pWord->dwData0;
  memcpy(szData,pWord->szData1,pWord->dwData1);
  szData[pWord->dwData1] = 0;
  return TRUE;
}

BOOL wordhead(PPerWord pWord,size_t dwHead,PPerWord pHead)
{
  memset(pHead,0,sizeof(*pHead));
  if (dwHead > wordlen(pWord)) return FALSE;
  if (dwHead > pWord->dwData0){
	pHead->szData0 = pWord->szData0;
	pHead->dwData0 = pWord->dwData0;
	pHead->szData1 = pWord->szData1;
	pHead->dwData1 = dwHead - pWord->dwData0;
  }
  else{
    pHead->szData0 = pWord->szData0;
	pHead->dwData0 = dwHead;
  }
  return TRUE;
}

BOOL wordtail(PPerWord pWord,size_t dwHead,PPerWord pTail)
{
  memset(pTail,0,sizeof(*pTail));
  if (dwHead <= pWord->dwData0){
	pTail->szData0 = &pWord->szData0[dwHead];
	pTail->dwData0 = pWord->dwData0 - dwHead;
	pTail->szData1 = pWord->szData1;
	pTail->dwData1 = pWord->dwData1;
  }
  else{
    dwHead -= pWord->dwData0;
    if (dwHead < pWord->dwData1){
	  pTail->szData0 = &pWord->szData1[dwHead];
	  pTail->dwData0 = pWord->dwData1 - dwHead;
	}
	else{
	  return FALSE;
	}
  }
  return TRUE;
}

PSTR _wordword(PSTR szWord,size_t dwWord,PSTR szCheck,size_t dwCheck)
{
  PSTR pHead,pTail;

  if (!_calcSizeEx(szWord,&dwWord,szCheck,&dwCheck)) return NULL;
  if (0 == dwCheck) return szWord;
  pTail = szWord + (dwWord - dwCheck);
  for(pHead=szWord;pHead<pTail;pHead++){
	if (0 == memcmp(pHead,szCheck,dwCheck)) return pHead;
  }
  return NULL;
}

BOOL _wordtokC(PSTR szData,size_t dwData,size_t dwBase,size_t *pdwCurr,char chSplit,PPSTR pszData,size_t *pdwData,PPSTR ppchEnd)
{
  PSTR pHead,pTail;
  
  if (*pdwCurr >= dwData+dwBase) return FALSE;
  pHead = szData + (*pdwCurr) - dwBase;
  pTail = szData + dwData;
  *pszData = pHead;
  for(;pHead < pTail && chSplit != *pHead;pHead++);
  *pdwData = pHead - (*pszData);
  if (NULL != ppchEnd) *ppchEnd = pHead;
  if (pHead < pTail) pHead++;
  *pdwCurr = pHead - szData + dwBase;
  return TRUE;
}

BOOL wordtokC(PPerWord pLine,size_t *pdwCurr,char chSplit,PPerWord pWord,PPSTR ppchEnd)
{
  PSTR pchEnd;

  memset(pWord,0,sizeof(*pWord));
  if (NULL != ppchEnd) *ppchEnd = NULL;
  pchEnd = NULL;
  if (*pdwCurr >= pLine->dwData0 + pLine->dwData1) return FALSE;
  if (*pdwCurr < pLine->dwData0){
    if (_wordtokC(pLine->szData0,pLine->dwData0,0,pdwCurr,chSplit,&pWord->szData0,&pWord->dwData0,&pchEnd)){
	  if (NULL != pchEnd){
	    if (NULL != ppchEnd) *ppchEnd = pchEnd;
	    return TRUE;
	  }
	}
	return _wordtokC(pLine->szData1,pLine->dwData1,pLine->dwData0,pdwCurr,chSplit,&pWord->szData1,&pWord->dwData1,ppchEnd);
  }
  else{
    return _wordtokC(pLine->szData1,pLine->dwData1,pLine->dwData0,pdwCurr,chSplit,&pWord->szData0,&pWord->dwData0,ppchEnd);
  }
}

void wordinit(PPerWord pWord,PSTR szData0,size_t dwData0,PSTR szData1,size_t dwData1)
{
  memset(pWord,0,sizeof(*pWord));
  pWord->szData0 = szData0;
  pWord->dwData0 = dwData0;
  pWord->szData1 = szData1;
  pWord->dwData1 = dwData1;
  _calcSize(pWord->szData0,&pWord->dwData0);
  if (NULL != szData1){
    _calcSize(pWord->szData1,&pWord->dwData1);
  }
}

void wordtrim(PPerWord pWord)
{
  size_t i;

  for(i=0;i<pWord->dwData0;i++){
    if (!ISSPACE(pWord->szData0[i])) break;
  }
  pWord->szData0 += i;
  pWord->dwData0 -= i;
  if (0 == pWord->dwData1){
    if (0 != pWord->dwData0){
      for(i=pWord->dwData0-1;i>0;i--){
        if (!ISSPACE(pWord->szData0[i])){
	      pWord->dwData0 = i + 1;
	      break;
		}
	  }
	}
  }
  else{
	for(i=pWord->dwData1-1;i>0;i--){
      if (!ISSPACE(pWord->szData1[i])){
	    pWord->dwData1 = i + 1;
	    break;
	  }
	}
  }
}

BOOL _wordiequ(PSTR szWord,size_t dwWord,PSTR szCheck,size_t dwCheck)
{
  size_t i;

  _calcSize(szWord,&dwWord);
  _calcSize(szCheck,&dwCheck);
  if (dwWord != dwCheck) return FALSE;
  for(i=0;i<dwCheck;i++){
    if (0 != toupper(szWord[i]) - toupper(szCheck[i])) return FALSE;
  }
  return TRUE;
}

size_t wordlen(PPerWord pWord)
{
  return pWord->dwData0 + pWord->dwData1;
}

BOOL wordiequ(PPerWord pWord,PPerWord pCheck)
{
  PSTR  pNext;
  size_t dwSize;

  if (wordlen(pWord) != wordlen(pCheck)) return FALSE;
  if (pWord->dwData0 <= pCheck->dwData0){
    dwSize = pWord->dwData0;
    if (!_wordiequ(pWord->szData0,dwSize,pCheck->szData0,dwSize)) return FALSE;
	pNext  = pCheck->szData0 + dwSize;
	dwSize = pCheck->dwData0 - pWord->dwData0;
	if (!_wordiequ(pWord->szData1,dwSize,pNext,dwSize)) return FALSE;
	pNext  = pWord->szData1 + dwSize;
	dwSize = pCheck->dwData1 - (pCheck->dwData0 - pWord->dwData0);
	if (!_wordiequ(pNext,dwSize,pCheck->szData1,dwSize)) return FALSE;
  }
  else{
    return wordiequ(pCheck,pWord);
  }
  return TRUE;
}

BOOL _wordiequn(PSTR szWord,size_t dwWord,PSTR szCheck,size_t dwCheck)
{
  size_t i;

  if (!_calcSizeEx(szWord,&dwWord,szCheck,&dwCheck)) return FALSE;
  for(i=0;i<dwCheck;i++){
    if (0 != toupper(szWord[i]) - toupper(szCheck[i])) return FALSE;
  }
  return TRUE;
}

BOOL wordiequn(PPerWord pWord,PPerWord pCheck)
{
  size_t    dwCheck;
  NPerWord nWord;

  if (wordlen(pWord) < (dwCheck=wordlen(pCheck))) return FALSE;
  memset(&nWord,0,sizeof(nWord));
  nWord.szData0 = pWord->szData0;
  if (pWord->dwData0 < dwCheck){
    nWord.dwData0 = pWord->dwData0;
	nWord.szData1 = pWord->szData1;
	nWord.dwData1 = dwCheck - nWord.dwData0;
  }
  else{
    nWord.dwData0 = dwCheck;
  }
  return wordiequ(&nWord,pCheck);
}

BOOL wordicheck(PPerWord pWord,size_t dwHead,PPerWord pCheck,PPerWord pTail)
{
  NPerWord nHead;
  
  wordhead(pWord,dwHead,&nHead);
  if (!wordiequ(&nHead,pCheck)) return FALSE;
  wordtail(pWord,dwHead+1,pTail);
  return TRUE;
}

PSTR strlf2crlf(PSTR szIn)
{
  PSTR szOut,pOld,pNew;

  if (NULL == (szOut=MemCalloc(2*strlen(szIn)+1,1))) return NULL;
  for(pNew=szOut,pOld=szIn;*pOld;pOld++){
    switch(*pOld){
	  case '\r':
	       break;
	  case '\n':
	       *pNew++ = '\r';
	  default:
		   *pNew++ = *pOld;
		   break;
    }
  }
  *pNew = 0;
  return szOut;
}

void strcrlf2lf(PSTR szIn)
{
  PSTR pOld,pNew;

  for(pOld=pNew=szIn;*pOld;pOld++){
    switch(*pOld){
	  case '\r':
	       break;
	  default:
		   *pNew++ = *pOld;
		   break;
    }
  }
  *pNew = 0;
}

size_t strbuild(PSTR szDest,size_t dwDest,...)
{
  size_t   retCode;
  va_list vp;

  va_start(vp,dwDest);
  retCode = strbuildVP(szDest,dwDest,vp);
  va_end(vp);
  return retCode;
}

size_t strbuildVP(PSTR szDest,size_t dwDest,va_list vp)
{
  size_t dwTotal,dwData;
  PSTR  szData;

  for(dwTotal=0;;){
    if (NULL == (szData=va_arg(vp,PSTR))) break;
    dwData = strlen(szData);
    if (NULL != szDest){
      if (dwDest <= dwTotal + dwData) return -1;
      memcpy(&szDest[dwTotal],szData,dwData);
    }
    dwTotal += dwData;
  }
  return dwTotal + 1;
}

BOOL strbuildEx(PSTR *pszDest,size_t *pdwDest,...)
{
  BOOL    retCode;
  size_t   dwSize;
  va_list vp;

  va_start(vp,pdwDest);
  dwSize = strbuildVP(NULL,0,vp);
  if (NULL == *pszDest || *pdwDest < dwSize){
    retCode = MemCheckSize(pszDest,dwSize);
  }
  else{
    retCode = TRUE;
  }
  if (retCode){
    retCode = -1 != strbuildVP(*pszDest,*pdwDest,vp);
  }
  va_end(vp);
  return retCode;
}

BOOL strcpyex(PSTR szBuffer,size_t dwBuffer,size_t *pdwSize,PSTR szText)
{
  size_t dwText;

  dwText = strlen(szText);
  if (NULL != szBuffer){
    if (*pdwSize + dwText >= dwBuffer) return FALSE;
	memcpy(&szBuffer[*pdwSize],szText,dwText);
	szBuffer[*pdwSize + dwText]=0;
  }
  *pdwSize += dwText;
  return TRUE;
}

BOOL strcpyexv(PSTR szBuffer,size_t dwBuffer,size_t *pdwSize,PSTR szFMT,...)
{
  char    szText[8192];
  va_list vp;

  va_start(vp,szFMT);
  vsnprintf(szText,sizeof(szText),szFMT,vp);
  va_end(vp);
  return strcpyex(szBuffer,dwBuffer,pdwSize,szText);
}

BOOL strcpyexvp(PSTR szBuffer,size_t dwBuffer,size_t *pdwSize,PSTR szFMT,va_list vp)
{
  char szText[8192];

  vsnprintf(szText,sizeof(szText),szFMT,vp);
  return strcpyex(szBuffer,dwBuffer,pdwSize,szText);
}

#ifndef HAVE___STRCHRNUL
char *__strchrnul(const char *s,char c)
{
  for(;*s && *s!=c;s++);
  return s;
}
#endif //HAVE___STRCHRNUL

#ifndef HAVE_STRNDUP
char *strndup(const char *s,size_t l)
{
  char *d;

  d = malloc(l+1);
  memcpy(d,s,l);
  d[l]=0;
  return d;
}
#endif //HAVE_STRNDUP
