#ifndef _INC_STRLIB_
#define _INC_STRLIB_

#include <baseLib/platForm.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

PBYTE memmem(PBYTE pData,size_t dwData,PBYTE pMask,size_t dwMask);

void strcpyNN(PSTR szDst,size_t cbDst,PSTR szSrc,size_t cbSrc);

void strcpyN(PSTR szText,size_t cbText,PSTR szSrc);
void strcpyV(PSTR szText,size_t cbText,PSTR szFMT,...);

void strcatN(PSTR szText,size_t cbText,PSTR szSrc);
void strcatV(PSTR szText,size_t cbText,PSTR szFMT,...);

void strtrim(PSTR szText);
void strtrimEx(PSTR szText,PSTR szSpace);

void strescape(PSTR szData,size_t cbData,PSTR szOut,size_t cbOut,BOOL asRC);
void wcsescape(PWSTR szData,size_t cbData,PWSTR szOut,size_t cbOut,BOOL asRC);

PSTR strtokC0(PSTR szLine,PPSTR pchTemp,char chSplit);
PSTR strtokC1(PPSTR pchTemp,char chSplit);
PSTR strtokS0(PSTR szLine,PPSTR szTmpPtr,PSTR szSpace);
PSTR strtokS1(PPSTR szTmpPtr,PSTR szSpace);

PSTR strtokC0Ex(PSTR szLine,PPSTR pchTemp,char chSplit,PSTR endC);
PSTR strtokC1Ex(PPSTR pchTemp,char chSplit,PSTR endC);
PSTR strtokS0Ex(PSTR szLine,PPSTR szTmpPtr,PSTR szSpace,PSTR endC);
PSTR strtokS1Ex(PPSTR szTmpPtr,PSTR szSpace,PSTR endC);

PSTR strlf2crlf(PSTR szIn);
void strcrlf2lf(PSTR szIn);

BOOL strmatch(PSTR szText,PSTR szPattern);

BOOL strprefix(PSTR szData,PSTR szPrefix);
BOOL striprefix(PSTR szData,PSTR szPrefix);

BOOL wcsprefix(PWSTR szData,PWSTR szPrefix);

#ifndef HAVE_STRLWR
void strlwr(PSTR szText);
#endif //HAVE_STRLWR

#ifndef HAVE_STRUPR
void strupr(PSTR szText);
#endif //HAVE_STRUPR

#ifndef HAVE_STRICMP
int stricmp(PSTR p0,PSTR p1);
#endif //HAVE_STRICMP

#ifndef HAVE_MEMICMP
int memicmp(PSTR p0,PSTR p1,size_t size);
#endif //HAVE_MEMICMP

#ifndef HAVE___STRCHRNUL
char *__strchrnul(const char *s,char c);
#endif //HAVE___STRCHRNUL

#ifndef HAVE_STRNDUP
char *strndup(const char *s,size_t l);
#endif //HAVE_STRNDUP

void mstrcpyV(PSTR szText,PSTR szFMT,...);
void mstrcatV(PSTR szText,PSTR szFMT,...);

PSTR astrcpy(PSTR szData);
PSTR astrcat(PSTR szText,PSTR szData);

PSTR astrcpyV(PSTR szFMT,...);
PSTR astrcatV(PSTR szText,PSTR szFMT,...);

typedef struct{
  PSTR  szData0;
  size_t dwData0;
  PSTR  szData1;
  size_t dwData1;
}NPerWord,*PPerWord;

void _calcSize(PSTR szWord,size_t *pdwWord);

BOOL _calcSizeEx(PSTR szWord,size_t *pdwWord,PSTR szCheck,size_t *pdwCheck);

BOOL _wordtokC(PSTR szData,size_t dwData,size_t dwBase,size_t *pdwCurr,char chSplit,PPSTR pszData,size_t *pdwData,PPSTR pchEnd);
BOOL wordtokC(PPerWord pLine,size_t *pdwCurr,char chSplit,PPerWord pWord,PPSTR pchEnd);

PSTR _wordchr(PSTR szWord,size_t dwWord,char chCheck);
PSTR wordchr(PPerWord pWord,char chCheck,size_t *pdwPos);

PSTR _wordword(PSTR szWord,size_t dwWord,PSTR szCheck,size_t dwCheck);

void wordinit(PPerWord pWord,PSTR szData0,size_t dwData0,PSTR szData1,size_t dwData1);

void wordtrim(PPerWord pWord);

PSTR worddump(PPerWord pWord);

BOOL wordcpy(PPerWord pWord,PSTR szData,size_t dwData);

void wordcopy(PPerWord pWord,PPSTR pszData);

BOOL wordhead(PPerWord pWord,size_t dwHead,PPerWord pHead);
BOOL wordtail(PPerWord pWord,size_t dwHead,PPerWord pTail);

BOOL _wordiequ(PSTR szWord,size_t dwWord,PSTR szCheck,size_t dwCheck);
BOOL wordiequ(PPerWord pWord,PPerWord pCheck);

BOOL _wordiequn(PSTR szWord,size_t dwWord,PSTR szCheck,size_t dwCheck);
BOOL wordiequn(PPerWord pWord,PPerWord pCheck);

size_t wordlen(PPerWord pWord);

BOOL wordicheck(PPerWord pWord,size_t dwHead,PPerWord pCheck,PPerWord pTail);

BOOL strbuildEx(PSTR *pszDest,size_t *pdwDest,...);
size_t strbuild(PSTR szDest,size_t dwDest,...);
size_t strbuildVP(PSTR szDest,size_t dwDest,va_list vpS);

void UrlAddPathSplit(PSTR szUrl);

BOOL strcpyex(PSTR szBuffer,size_t dwBuffer,size_t *pdwSize,PSTR szText);
BOOL strcpyexv(PSTR szBuffer,size_t dwBuffer,size_t *pdwSize,PSTR szFMT,...);
BOOL strcpyexvp(PSTR szBuffer,size_t dwBuffer,size_t *pdwSize,PSTR szFMT,va_list vp);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_STRLIB_
