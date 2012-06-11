#ifndef _INC_DATETIME_H_
#define _INC_DATETIME_H_

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#include "baseLib/platForm.h"

#ifndef WIN32
typedef struct{
  WORD wYear;
  WORD wMonth;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
}SYSTEMTIME;
#endif //!WIN32

#pragma pack(1)
typedef struct{
  WORD wYear;  //0-65535
  WORD wMonth  : 4;  //0-15 1-12
  WORD wDay    : 5;  //0-31 1-31
  WORD wHour   : 5;  //0-31 0-23
  WORD wMinute : 6;  //0-63 0-59
  WORD wSecond : 6;  //0-63 0-59
}PACKEDTIME;
#pragma pack()

#ifndef WIN32
BOOL GetSystemTime(SYSTEMTIME *pST);
BOOL GetLocalTime(SYSTEMTIME *pST);
#else
int gettimeofday(struct timeval *tp,void *tzp);
#endif //!WIN32

int SYSTEMTIME_Compare(SYSTEMTIME *pFT1,SYSTEMTIME *pFT2);

BOOL SYSTEMTIME_Parse(PSTR IN szText,SYSTEMTIME OUT *pST,BOOL OUT *pbGMT);
BOOL TIME_T_Parse(PSTR IN szText,time_t OUT *pST,BOOL OUT *pbGMT);

BOOL SYSTEMTIME_Valid(SYSTEMTIME IN *pST);

void TM_SYSTEMTIME(struct tm *pTM,SYSTEMTIME IN OUT *pST);
BOOL TIME_T_SYSTEMTIME(time_t pTM,SYSTEMTIME IN OUT *pST);

BOOL SYSTEMTIME_TM(SYSTEMTIME IN *pST,struct tm OUT *pTM);
BOOL SYSTEMTIME_TIME_T(SYSTEMTIME IN *pST,time_t OUT *pTM);

size_t strftime_SYSTEMTIME(PSTR szText,size_t cbText,PSTR szFormat,SYSTEMTIME *pST);
size_t strftime_TIME_T(PSTR szText,size_t cbText,PSTR szFormat,time_t time);

void SYSTEMTIME2PACKEDTIME(SYSTEMTIME IN *pST,PACKEDTIME OUT *pPT);
void PACKEDTIME2SYSTEMTIME(PACKEDTIME IN *pPT,SYSTEMTIME OUT *pST);

/*
time_t 
BOOL GetTimeText(time_t time,PSTR szFormat,PSTR szText,size_t cbText);
BOOL GetLocalTimeText(time_t gmttime,PSTR szFormat,PSTR szText,size_t cbText);
*/

#ifndef HAVE_STRPTIME
char *strptime(const char *buf,const char * format,struct tm * timeptr);
#endif //HAVE_STRPTIME

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_DATETIME_H_
