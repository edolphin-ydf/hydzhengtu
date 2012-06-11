#ifdef ZEBRA_RELEASE
#include <epesdk.h>
#endif //ZEBRA_RELEASE

#include <zebra/srvEngine.h>
#include <shlwapi.h>
#include <stdlib.h>

#include <iostream>

namespace DareDef
{
  /// ״̬����
  char str_state[9][20]={"DARE_READY","DARE_READY_QUESTION","DARE_READY_ACTIVE","DARE_RETURN_GOLD",
    "DARE_ACTIVE","DARE_DATAPROCESS","DARE_READY_OVER","DARE_WAIT_BOUNTY","DARE_OVER"};

  char str_type[7][30] = {"UNION_DARE","SCHOOL_DARE","SEPT_DARE","SEPT_NPC_DARE","UNION_CITY_DARE","COUNTRY_FORMAL_DARE","COUNTRY_FORMAL_ANTI_DARE"};

}

namespace QuizDef
{
  /// ״̬����
  char str_state[9][30]={"QUIZ_READY","QUIZ_READY_QUESTION","QUIZ_SEND_QUESTION","QUIZ_ACTIVE_QUESTION",
    "QUIZ_END_QUESTION","QUIZ_READY_OVER","QUIZ_READ_SORT","QUIZ_RETURN_GOLD","QUIZ_OVER"};
  char str_type[2][20] = {"WORLD_QUIZ","PERSONAL_QUIZ"};

}

namespace Zebra
{
  volatile QWORD qwGameTime = 0;

  zLogger *logger = NULL;

  zProperties global;

  /**
   * \brief ��ʼ��һЩȫ�ֱ���
   *
   */
  //static void initGlobal()  __attribute__ ((constructor));
  static void initGlobal();
  void initGlobal()
  {
    global["threadPoolClient"] = "512";
    global["threadPoolServer"] = "2048";
    global["server"]           = "127.0.0.1";
    global["port"]             = "10000";
    global["ifname"]           = "127.0.0.1";
    global["mysql"]            = "mysql://zebra:zebra@127.0.0.1:3306/zebra";
    global["log"]              = "error";
  }
  /**
   * \brief �ͷ�һЩȫ�ֱ���
   *
   */
  //static void finalGlobal() __attribute__ ((destructor));
  static void finalGlobal();
  void finalGlobal()
  {
    SAFE_DELETE(logger);
  }
};

//�������min~max֮������֣�����min��max
int randBetween(int min,int max)
{
  if (min==max)
    return min;
  else if (min > max)
    return max + (int) (((double) min - (double)max + 1.0) * rand() / (RAND_MAX + 1.0));
  else
    return min + (int) (((double) max - (double)min + 1.0) * rand() / (RAND_MAX + 1.0));
}

//��ȡ����֮�ļ���
bool selectByOdds(const DWORD upNum,const DWORD downNum)
{
  DWORD m_rand;
  if (downNum < 1) return false;
  if (upNum < 1) return false;
  if (upNum > downNum - 1) return true;
  m_rand = 1 + (DWORD) ((double)downNum * rand() / (RAND_MAX + 1.0));
  return (m_rand <= upNum);
}

//��ȡ����֮���ļ���
bool selectByt_Odds(const odds_t &odds)
{
  return selectByOdds(odds.upNum,odds.downNum);
}

//��ȡ�ٷ�֮�ļ���
bool selectByPercent(const DWORD percent)
{
  return selectByOdds(percent,100);
}

//��ȡ���֮�ļ���
bool selectByTenTh(const DWORD tenth)
{
  return selectByOdds(tenth,10000);
}

//��ȡʮ���֮�ļ���
bool selectByLakh(const DWORD lakh)
{
  return selectByOdds(lakh,100000);
}

//��ȡ�ڷ�֮֮�ļ���
bool selectByOneHM(const DWORD lakh)
{
  return selectByOdds(lakh,100000000);
}

//��ȡ��ǰʱ���ַ�������Ҫ������ʽ
void getCurrentTimeString(char *buffer,const int bufferlen,const char *format)
{
  time_t now;
  time(&now);
  strftime(buffer,bufferlen,format,localtime(&now));
}

char *getTimeString(time_t t,char *buffer,const int bufferlen,const char *format)
{
  strftime(buffer,bufferlen,format,localtime(&t));
  return buffer;
}

char *getTimeString(time_t t,char *buffer,const int bufferlen)
{
  return getTimeString(t,buffer,bufferlen,"%C/%m/%d %T");
}

void Zebra_Startup(void)
{
  char szName[_MAX_PATH*100];
  char szCode[8192];
#ifdef ZEBRA_RELEASE
  BOOL bRegistered;
  FILE *fp;
  char szUser[_MAX_PATH];
  char *token;

  EPE_GetRegisterHandle(NULL);
  EPE_GetRegisterInfo(EPEGRI_IS_REGISTERED,szName,sizeof(szName));
  bRegistered = 0 == strcmp(szName,"Y");
  if (!bRegistered)
  {
    GetModuleFileName(NULL,szName,sizeof(szName));
    strcpy(PathFindFileName(szName),"server.reg");
    if (NULL != (fp=fopen(szName,"rt")))
    {
      if (NULL != fgets(szUser,sizeof(szUser),fp))
      {
        if (NULL != fgets(szCode,sizeof(szCode),fp))
        {
          if (NULL != (token=strchr(szUser,'\r'))) *token = 0;
          if (NULL != (token=strchr(szUser,'\n'))) *token = 0;
          if (NULL != (token=strchr(szCode,'\r'))) *token = 0;
          if (NULL != (token=strchr(szCode,'\n'))) *token = 0;
          if (EPE_SetRegisterInfo(szUser,szCode))
          {
            EPE_GetRegisterInfo(EPEGRI_IS_REGISTERED,szName,sizeof(szName));
            bRegistered = 0 == strcmp(szName,"Y");
          }
        }
      }
      fclose(fp);
    }
  }
  if (!bRegistered)
  {
    EPE_ShowRegisterDlg();
  }
#endif //ZEBRA_RELEASE

  GetModuleFileName(NULL,szName,sizeof(szName));
  _snprintf(szCode,sizeof(szCode),"Ӣ����˫ - %s",PathFindFileName(szName));
  SetConsoleTitle(szCode);
}

void loadFilter(NFilterModuleArray & nFMA,PSTR szPattern)
{
  char                               szPath[_MAX_PATH];
  char                               szName[_MAX_PATH];
  HANDLE                             hFind;
  NFilterModule                      nFM;
  WIN32_FIND_DATA                    nWFD;
  
  GetModuleFileName(NULL,szPath,sizeof(szPath));
  *PathFindFileName(szPath) = 0;
  sprintf(szName,"%s%s",szPath,szPattern);
  if (INVALID_HANDLE_VALUE != (hFind=FindFirstFile(szName,&nWFD))){
    for(;;)
	{
      if (!(nWFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	  {
	    sprintf(szName,"%s%s",szPath,nWFD.cFileName);
		memset(&nFM,0,sizeof(nFM));
		if (NULL != (nFM.hInstance=LoadLibrary(szName)))
		{
		  nFM.filter_init    = (PFN_filter_init)GetProcAddress(nFM.hInstance,"filter_init");
		  nFM.filter_command = (PFN_filter_command)GetProcAddress(nFM.hInstance,"filter_command");
		  nFM.filter_term    = (PFN_filter_term)GetProcAddress(nFM.hInstance,"filter_term");
		  if (NULL != nFM.filter_command)
		  {
            nFMA.push_back(nFM);
		  }
		  else
		  {
		    FreeLibrary(nFM.hInstance);
		  }
		}
	  }
	  if (!FindNextFile(hFind,&nWFD)) break;
	}
  }
}
