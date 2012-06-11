#include <baseLib/logLib.h>
#include <baseLib/codeLib.h>
#include <baseLib/utilLib.h>
#include <baseLib/timeLib.h>

int SYSTEMTIME_Compare(SYSTEMTIME *pTM1,SYSTEMTIME *pTM2)
{
  if (pTM1->wYear   < pTM2->wYear)   return -1;
  if (pTM1->wYear   > pTM2->wYear)   return +1;
  if (pTM1->wMonth  < pTM2->wMonth)  return -1;
  if (pTM1->wMonth  > pTM2->wMonth)  return +1;
  if (pTM1->wDay    < pTM2->wDay)    return -1;
  if (pTM1->wDay    > pTM2->wDay)    return +1;
  if (pTM1->wHour   < pTM2->wHour)   return -1;
  if (pTM1->wHour   > pTM2->wHour)   return +1;
  if (pTM1->wMinute < pTM2->wMinute) return -1;
  if (pTM1->wMinute > pTM2->wMinute) return +1;  
  if (pTM1->wSecond < pTM2->wSecond) return -1;
  if (pTM1->wSecond > pTM2->wSecond) return +1;
  return 0;
}

/*
format0: Sun, 06 Nov 1994 08:49:37 GMT
format0: Sun, 06 Nov 1994 08:49:37
format1: Sunday, 06-Nov-94 08:49:37 GMT
format1: Sunday, 06-Nov-94 08:49:37
format2: Sun Nov  6 08:49:37 1994

2000/12/12 12:12:11
2000-12-12 12:12:11
2000年12月12日12:12:11
*/

static char szCKey[][2]={"年","月","日","时","分","秒",""};
static char __dsnames[]="SunMonTueWedThuFriSat";
static char *__dlnames[]={"Sunday","Monday","Wednesday","Thursday","Friday","Saturday"};
static char __mnames[]="JanFebMarAprMayJunJulAugSepOctNovDec";

BOOL SYSTEMTIME_Parse(PSTR IN szDateTime,SYSTEMTIME OUT *pST,BOOL OUT *pbGMT)
{
  size_t i,j,k,len,ord,value;

  __API_ENTER("SYSTEMTIME_Parse",BOOL,FALSE);
  memset(pST,0,sizeof(SYSTEMTIME));
  if (NULL != pbGMT) *pbGMT = FALSE;
  len = strlen(szDateTime);
  if (isalpha(szDateTime[0]))
  {
	for(i=0;i<7;i++)
    {
      if (0 == memcmp(szDateTime,&__dsnames[i*3],3)) break;
    }
	if (i<7)
    {
      switch(szDateTime[3])
      {
        case ',': //format0
             if (len < 25) __API_FINISH();
             pST->wDay = atoix(&szDateTime[5],2);
             for(i=0;i<12;i++)
             {
               if (0 == memcmp(&szDateTime[8],&__mnames[i*3],3)) break;
             }
             if (i == 12) __API_FINISH();
             pST->wMonth = i + 1;
             pST->wYear = atoix(&szDateTime[12],4);

             pST->wHour   = atoix(&szDateTime[17],2);
             pST->wMinute = atoix(&szDateTime[20],2);
             pST->wSecond = atoix(&szDateTime[23],2);
             if (len >= 29) *pbGMT = 0 == memcmp(&szDateTime[26],"GMT",3);
             break;
        case ' ': //format1
             if (len < 24) __API_FINISH();
             for(i=0;i<12;i++)
             {
               if (0 == memcmp(&szDateTime[4],&__mnames[i*3],3)) break;
             }
             if (i == 12) __API_FINISH();
             pST->wMonth = i + 1;
             pST->wDay   = atoix(&szDateTime[8],2);

             pST->wHour   = atoix(&szDateTime[11],2);
             pST->wMinute = atoix(&szDateTime[14],2);
             pST->wSecond = atoix(&szDateTime[17],2);
             pST->wYear   = atoix(&szDateTime[20],4);
             break;
        default: //format2
             j = strlen(__dlnames[i]);
             if (0 != memcmp(szDateTime,__dlnames[i],j)) __API_FINISH();
             if (len < j+20) __API_FINISH();
             pST->wDay = atoix(&szDateTime[j+2],2);
             for(i=0;i<12;i++)
             {
               if (0 == memcmp(&szDateTime[j+5],&__mnames[i*3],3)) break;
             }
             if (i == 12) __API_FINISH();
             pST->wMonth = i + 1;
             pST->wYear  = 1900 + atoix(&szDateTime[j+9],2);

             pST->wHour   = atoix(&szDateTime[j+12],2);
             pST->wMinute = atoix(&szDateTime[j+15],2);
             pST->wSecond = atoix(&szDateTime[j+18],2);
             if (len >= j+24) *pbGMT = 0 == memcmp(&szDateTime[j+21],"GMT",3);
             break;
      }
    }
  }
  else
  {  
    for(ord=i=0;i<len;i=j,ord++){
      for(j=i;j<len;j++){
	    if (!isdigit((BYTE)szDateTime[j])) break;
	  }
      k = IsValidGb2312((BYTE)szDateTime[j],(BYTE)szDateTime[j+1]) ? 2 : 1;
      if (1 == k){
        switch(ord){
          case 0:
          case 1:
               if (NULL == strchr("./-",szDateTime[j])) __API_FINISH();
               break;
          case 2:
               if (NULL == strchr("./- ",szDateTime[j])) __API_FINISH();
               break;
          case 3:
               if (NULL == strchr("./-:",szDateTime[j])) __API_FINISH();
               break;
          case 4:
	  		   if (NULL == strchr("./-:'",szDateTime[j])) __API_FINISH();
               break;
          case 5:
          case 6:
			   if (0 != szDateTime[j] && (NULL == strchr("./-:\"",szDateTime[j]))) __API_FINISH();
               break;
        }
      }
      else{
        if (memcmp(szCKey[ord],&szDateTime[j],k)) __API_FINISH();
      }
      value = atoix(&szDateTime[i],j-i);
      switch(ord){
        case 0:
		     if (value <= 0) __API_FINISH();
             pST->wYear = value;
             break;
        case 1:
             if (value < 1 || value > 12) __API_FINISH();
             pST->wMonth = value;
             break;
        case 2:
             if (value < 1 || value > 31) __API_FINISH();
             pST->wDay = (WORD)value;
             break;
        case 3:
             if (value < 0 || value > 23) __API_FINISH();
             pST->wHour = (WORD)value;
             break;
        case 4:
             if (value < 0 || value > 59) __API_FINISH();
             pST->wMinute = (WORD)value;
             break;
        case 5:
             if (value < 0 || value > 59) __API_FINISH();
             pST->wSecond = (WORD)value;
             break;
        case 6:
             break;
      }
      j += k;
    }
    if (i < len) __API_FINISH();
    switch(ord){
      case 1:
           pST->wMonth = 1;
      case 2:
           pST->wDay = 1;
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
           break;
      default:
           __API_FINISH();
    }
  }
  retCode = TRUE;
__API_END_POINT:
  if (!retCode)
  {
    logMessage1("!SYSTEMTIME_Parse(%s)",szDateTime);
#ifdef _DEBUG
    __asm int 3;
#endif //_DEBUG
  }
  __API_LEAVE("SYSTEMTIME_Parse");
}

BOOL TIME_T_Parse(PSTR IN szText,time_t OUT *pTM,BOOL OUT *pbGMT)
{
  SYSTEMTIME nST;

  if (!SYSTEMTIME_Parse(szText,&nST,pbGMT)) return FALSE;
  return SYSTEMTIME_TIME_T(&nST,pTM);
}

BOOL SYSTEMTIME_Valid(SYSTEMTIME IN *pST)
{
  return pST->wMonth >= 1 && pST->wMonth <= 12 &&
         pST->wDay >= 1 && pST->wDay <= 31 &&
		 pST->wHour < 23 &&
		 pST->wMinute < 60 &&
		 pST->wSecond < 60;
}

BOOL SYSTEMTIME_TM(SYSTEMTIME IN *pST,struct tm OUT *pTM)
{
  memset(pTM,0,sizeof(struct tm));
  pTM->tm_year = pST->wYear - 1900;
  pTM->tm_mon  = pST->wMonth - 1;
  pTM->tm_mday = pST->wDay;
  pTM->tm_hour = pST->wHour;
  pTM->tm_min  = pST->wMinute;
  pTM->tm_sec  = pST->wSecond;
  return TRUE;
}

void TM_SYSTEMTIME(struct tm *pTM,SYSTEMTIME IN OUT *pST)
{
  memset(pST,0,sizeof(SYSTEMTIME));
  pST->wYear   = pTM->tm_year + 1900;
  pST->wMonth  = pTM->tm_mon + 1;
  pST->wDay    = pTM->tm_mday;
  pST->wHour   = pTM->tm_hour;
  pST->wMinute = pTM->tm_min;
  pST->wSecond = pTM->tm_sec;
}

BOOL SYSTEMTIME_TIME_T(SYSTEMTIME IN *pST,time_t OUT *pTM)
{
  struct tm now;

  if (!SYSTEMTIME_TM(pST,&now)) return FALSE;
  *pTM = mktime(&now);
  return TRUE;
}

BOOL TIME_T_SYSTEMTIME(time_t ltime,SYSTEMTIME IN OUT *pST)
{
  struct tm *now;

  memset(pST,0,sizeof(SYSTEMTIME));
  if (NULL == (now=gmtime(&ltime))) return FALSE;
  TM_SYSTEMTIME(now,pST);
  return TRUE;
}

size_t strftime_SYSTEMTIME(PSTR szText,size_t cbText,PSTR szFormat,SYSTEMTIME *time)
{
  struct tm now;

  if (!SYSTEMTIME_TM(time,&now)) return 0;
  return strftime(szText,cbText,szFormat,&now);
}

size_t strftime_TIME_T(PSTR szText,size_t cbText,PSTR szFormat,time_t time)
{
  struct tm *now;

  if (NULL == (now=gmtime(&time))) return 0;
  return strftime(szText,cbText,szFormat,now);
}

/*
BOOL GetLocalTimeText(time_t gmttime,PSTR szFormat,PSTR szText,size_t cbText)
{
  struct tm *now;

  szText[0] = 0;
#ifdef WIN32
  gmttime -= _timezone;
#else //WIN32
  gmttime -= __timezone;
#endif //WIN32
  if (NULL == (now=localtime(&gmttime))) return FALSE;
  strftime(szText,cbText,szFormat,now);
  return TRUE;
}
*/

#ifdef WIN32
/* Offset between 1/1/1601 and 1/1/1970 in 100 nanosec units */
#define _W32_FT_OFFSET (116444736000000000)

int gettimeofday(struct timeval *tp,void *tzp)
{
  union{
    __int64  ns100; /*time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  }_now;

  GetSystemTimeAsFileTime(&_now.ft);
  tp->tv_usec = (long)((_now.ns100 / 10) % 1000000);
  tp->tv_sec  = (long)((_now.ns100 - _W32_FT_OFFSET) / 10000000);
  /* Always return 0 as per Open Group Base Specifications Issue 6.
     Do not set errno on error.  */
  return 0;
}
#else
BOOL GetSystemTime(SYSTEMTIME *pST)
{
  time_t ltime;

  time(&ltime);
  return TIME_T_SYSTEMTIME(ltime,pST);
}

BOOL GetLocalTime(SYSTEMTIME *pST)
{
  time_t ltime;

  time(&ltime);
  ltime -= __timezone;
  return TIME_T_SYSTEMTIME(ltime,pST);
}
#endif //!WIN32

void SYSTEMTIME2PACKEDTIME(SYSTEMTIME IN *pIN,PACKEDTIME OUT *pOUT)
{
  pOUT->wYear   = pIN->wYear;
  pOUT->wMonth  = pIN->wMonth;
  pOUT->wDay    = pIN->wDay;
  pOUT->wHour   = pIN->wHour;
  pOUT->wMinute = pIN->wMinute;
  pOUT->wSecond = pIN->wSecond;
}

void PACKEDTIME2SYSTEMTIME(PACKEDTIME IN *pIN,SYSTEMTIME OUT *pOUT)
{
  memset(pIN,0,sizeof(*pIN));
  pOUT->wYear   = pIN->wYear;
  pOUT->wMonth  = pIN->wMonth;
  pOUT->wDay    = pIN->wDay;
  pOUT->wHour   = pIN->wHour;
  pOUT->wMinute = pIN->wMinute;
  pOUT->wSecond = pIN->wSecond;
}

#ifndef HAVE_STRPTIME

static const char *abb_weekdays[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    NULL
};

static const char *full_weekdays[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    NULL
};

static const char *abb_month[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL
};

static const char *full_month[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    NULL,
};

static const char *ampm[] = {
    "am",
    "pm",
    NULL
};

/*
 * tm_year is relative this year 
 */
const int tm_year_base = 1900;

/*
 * Return TRUE iff `year' was a leap year.
 * Needed for strptime.
 */
static int is_leap_year (int year)
{
    return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

/* Needed for strptime. */
static int match_string (const char **buf,const char **strs)
{
    int i = 0;

    for (i = 0; strs[i] != NULL; ++i) {
  int len = strlen (strs[i]);

  if (strncmp (*buf,strs[i],len) == 0) {
      *buf += len;
      return i;
  }
    }
    return -1;
}

/* Needed for strptime. */
static int first_day (int year)
{
    int ret = 4;

    for (; year > 1970; --year)
  ret = (ret + 365 + is_leap_year (year) ? 1 : 0) % 7;
    return ret;
}

/*
 * Set `timeptr' given `wnum' (week number [0,53])
 * Needed for strptime
 */

static void set_week_number_sun (struct tm *timeptr,int wnum)
{
    int fday = first_day (timeptr->tm_year + tm_year_base);

    timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
  timeptr->tm_wday = fday;
  timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0,53])
 * Needed for strptime
 */

static void
set_week_number_mon (struct tm *timeptr,int wnum)
{
    int fday = (first_day (timeptr->tm_year + tm_year_base) + 6) % 7;

    timeptr->tm_yday = wnum * 7 + (timeptr->tm_wday + 6) % 7 - fday;
    if (timeptr->tm_yday < 0) {
  timeptr->tm_wday = (fday + 1) % 7;
  timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0,53])
 * Needed for strptime
 */
static void
set_week_number_mon4 (struct tm *timeptr,int wnum)
{
    int fday = (first_day (timeptr->tm_year + tm_year_base) + 6) % 7;
    int offset = 0;

    if (fday < 4)
  offset += 7;

    timeptr->tm_yday = offset + (wnum - 1) * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
  timeptr->tm_wday = fday;
  timeptr->tm_yday = 0;
    }
}

char *strptime(const char *buf,const char * format,struct tm * timeptr)
{
    char c;

    for (; (c = *format) != '\0'; ++format) {
  char *s;
  int ret;

  if (isspace (c)) {
      while (isspace (*buf))
    ++buf;
  } else if (c == '%' && format[1] != '\0') {
      c = *++format;
      if (c == 'E' || c == 'O')
    c = *++format;
      switch (c) {
      case 'A' :
    ret = match_string (&buf,full_weekdays);
    if (ret < 0)
        return NULL;
    timeptr->tm_wday = ret;
    break;
      case 'a' :
    ret = match_string (&buf,abb_weekdays);
    if (ret < 0)
        return NULL;
    timeptr->tm_wday = ret;
    break;
      case 'B' :
    ret = match_string (&buf,full_month);
    if (ret < 0)
        return NULL;
    timeptr->tm_mon = ret;
    break;
      case 'b' :
      case 'h' :
    ret = match_string (&buf,abb_month);
    if (ret < 0)
        return NULL;
    timeptr->tm_mon = ret;
    break;
      case 'C' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_year = (ret * 100) - tm_year_base;
    buf = s;
    break;
      case 'c' :    /* %a %b %e %H:%M:%S %Y */
    s = strptime (buf,"%a %b %e %H:%M:%S %Y",timeptr);
    if (s == NULL)
        return NULL;
    buf = s;
    break;
      case 'D' :    /* %m/%d/%y */
    s = strptime (buf,"%m/%d/%y",timeptr);
    if (s == NULL)
        return NULL;
    buf = s;
    break;
      case 'd' :
      case 'e' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_mday = ret;
    buf = s;
    break;
      case 'H' :
      case 'k' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_hour = ret;
    buf = s;
    break;
      case 'I' :
      case 'l' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    if (ret == 12)
        timeptr->tm_hour = 0;
    else
        timeptr->tm_hour = ret;
    buf = s;
    break;
      case 'j' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_yday = ret - 1;
    buf = s;
    break;
      case 'm' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_mon = ret - 1;
    buf = s;
    break;
      case 'M' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_min = ret;
    buf = s;
    break;
      case 'n' :
    if (*buf == '\n')
        ++buf;
    else
        return NULL;
    break;
      case 'p' :
    ret = match_string (&buf,ampm);
    if (ret < 0)
        return NULL;
    if (timeptr->tm_hour == 0) {
        if (ret == 1)
      timeptr->tm_hour = 12;
    } else
        timeptr->tm_hour += 12;
    break;
      case 'r' :    /* %I:%M:%S %p */
    s = strptime (buf,"%I:%M:%S %p",timeptr);
    if (s == NULL)
        return NULL;
    buf = s;
    break;
      case 'R' :    /* %H:%M */
    s = strptime (buf,"%H:%M",timeptr);
    if (s == NULL)
        return NULL;
    buf = s;
    break;
      case 'S' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_sec = ret;
    buf = s;
    break;
      case 't' :
    if (*buf == '\t')
        ++buf;
    else
        return NULL;
    break;
      case 'T' :    /* %H:%M:%S */
      case 'X' :
    s = strptime (buf,"%H:%M:%S",timeptr);
    if (s == NULL)
        return NULL;
    buf = s;
    break;
      case 'u' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_wday = ret - 1;
    buf = s;
    break;
      case 'w' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_wday = ret;
    buf = s;
    break;
      case 'U' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    set_week_number_sun (timeptr,ret);
    buf = s;
    break;
      case 'V' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    set_week_number_mon4 (timeptr,ret);
    buf = s;
    break;
      case 'W' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    set_week_number_mon (timeptr,ret);
    buf = s;
    break;
      case 'x' :
    s = strptime (buf,"%Y:%m:%d",timeptr);
    if (s == NULL)
        return NULL;
    buf = s;
    break;
      case 'y' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    if (ret < 70)
        timeptr->tm_year = 100 + ret;
    else
        timeptr->tm_year = ret;
    buf = s;
    break;
      case 'Y' :
    ret = strtol (buf,&s,10);
    if (s == buf)
        return NULL;
    timeptr->tm_year = ret - tm_year_base;
    buf = s;
    break;
      case 'Z' :
    /* Unsupported. Just ignore.  */
    break;
      case '\0' :
    --format;
    /* FALLTHROUGH */
      case '%' :
    if (*buf == '%')
        ++buf;
    else
        return NULL;
    break;
      default :
    if (*buf == '%' || *++buf == c)
        ++buf;
    else
        return NULL;
    break;
      }
  } else {
      if (*buf == c)
    ++buf;
      else
    return NULL;
  }
    }
    return (char *)buf;
}

#endif //HAVE_STRPTIME
