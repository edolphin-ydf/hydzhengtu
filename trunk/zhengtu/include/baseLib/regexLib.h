#ifndef _INC_REGEXLIB_H_
#define _INC_REGEXLIB_H_

#include <baseLib/platForm.h>
#include <baseLib/regex.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

void regex_Escape(PSTR pSrc,PSTR pDst,DWORD dwDst);

BOOL regex_Replace(regex_t *reg,PSTR szVar[],regmatch_t *pmatch,PSTR pSrc,PSTR pDst,DWORD dwDst);
BOOL regex_ReplaceEx(PSTR szPattern,PSTR szVar[],regmatch_t *pmatch,PSTR pSrc,PSTR pDst,DWORD dwDst);
BOOL regex_ReplaceExEx(DWORD isCount,PSTR szName[],PSTR szVar[],regmatch_t *pmatch,PSTR pSrc,PSTR pDst,DWORD dwDst);
BOOL regex_Build(DWORD isCount,PSTR szName[],PSTR szVar[],PSTR pSrc,PSTR pDst,DWORD dwDst);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_REGEXLIB_H_
