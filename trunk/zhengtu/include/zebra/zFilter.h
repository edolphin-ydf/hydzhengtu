#ifndef _INC_ZFILTER_H_
#define _INC_ZFILTER_H_

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#ifndef ZFILTER_EXPORTS
#define FILTERAPI DECLSPEC_IMPORT
#else //ZFILTER_EXPORTS
#define FILTERAPI
#endif //ZFILTER_EXPORTS

#ifndef _INC_SRVENGINE_H_
typedef enum
    {
      /**
       * \brief ��zLogger�ȼ�����ΪOFF�����򲻻�����κ���־
       */
      LEVEL_OFF   = INT_MAX,
      
      /**
       * \brief ��zLogger�ȼ�����ΪFATAL��ֻ���FATAL�ȼ�����־
       *
       * �������������Ѿ��޷��ṩ�����ķ����ܡ�
       */
      LEVEL_FATAL = 50000,
      
      /**
       * \brief ��zLogger�ȼ�����ΪERROR��ֻ������ڵ��ڴ˵ȼ�����־
       *
       * ���󣬿��ܲ����ṩĳ�ַ��񣬵����Ա�֤������ȷ���С�
       */
      LEVEL_ERROR = 40000,
      
      /**
       * \brief ��zLogger�ȼ�����ΪWARN��ֻ������ڵ��ڴ˵ȼ�����־
       *
       * ���棬ĳЩ�ط���Ҫ����ע�⣬����û�������ļ�����������Ĭ��ѡ�����ʹ�á�
       */
      LEVEL_WARN  = 30000,
      
      /**
       * \brief ��zLogger�ȼ�����ΪINFO��ֻ������ڵ��ڴ˵ȼ�����־
       *
       * ��Ϣ���ṩһ����Ϣ��¼��������һЩ����״̬�ļ�¼��
       */
      LEVEL_INFO  = 20000,
      
      /**
       * \brief ��zLogger�ȼ�����ΪDEBUG��������еȼ�����־
       */
      LEVEL_DEBUG = 10000,
      
      /**
       * \brief ��zLogger�ȼ�����ΪALL��������еȼ�����־
       */
      LEVEL_ALL   = INT_MIN
    }zLevel;

typedef enum
{
  //UNKNOWNSERVER  =  0, // δ֪���������� 
  //SUPERSERVER      =  1, // ��������� 
  //LOGINSERVER     =  10, // ��½������ 
  //RECORDSERVER  =  11, // ���������� 
  //BILLSERVER      =  12, // �Ʒѷ����� 
  SESSIONSERVER  =  20, // �Ự������ 
  SCENESSERVER  =  21, // ���������� 
  //GATEWAYSERVER  =  22, // ���ط����� 
  //MINISERVER      =  23    // С��Ϸ������ 
}ServerType;
#endif //_INC_SRVENGINE_H_

typedef PSTR (*PFN_GetConfigValue)(PSTR szName,PSTR szDefault);

#ifdef _INC_SRVENGINE_H_
typedef void (*PFN_WriteLogMsg)(zLogger::zLevel level,char * pattern,...);
#else //_INC_SRVENGINE_H_
typedef void (*PFN_WriteLogMsg)(zLevel level,char * pattern,...);
#endif //_INC_SRVENGINE_H_

typedef struct NZebraUser *PZebraUser;

typedef PZebraUser (*PFN_GetUserByName)(PSTR szName);
typedef PZebraUser (*PFN_GetUserByTempID)(DWORD tempid);
typedef PZebraUser (*PFN_GetUserByID)(DWORD id);

typedef BOOL (*PFN_SendCommand)(PZebraUser pUser,PBYTE pCmd,DWORD dwCmd);

typedef struct
{
  ServerType          eST; //server����
  PFN_GetConfigValue  GetConfigValue; //��ȡ�����ļ����������Ϣ,û�еĻ�����szDefault
  PFN_WriteLogMsg     WriteLogMsg;    //д��־��Ϣ
  PFN_GetUserByName   GetUserByName;  //�����û����õ��û���ʵ��,SESSIONSERVER
  PFN_GetUserByTempID GetUserByTempID;//�����û���ʱID�õ��û���ʵ��,SESSIONSERVER
  PFN_GetUserByID     GetUserByID;    //�����û�ID�õ��û���ʵ��,SESSIONSERVER
  PFN_SendCommand     SendCommand;    //���û���������,SESSIONSERVER
}NZTFilterContext,*PZTFilterContext;

/*
server����ʱ����,���������г�ʼ���Ѿ����.
*/
void FILTERAPI FilterStartup(PZTFilterContext pZTFC);

/*
serverÿ�ν��յ�һ������ʱ����,����TRUE˵��filter�Ѿ����������.
ԭ����filter���Ը�дserver�ڲ��Ĵ�������,����Ŀǰ��������������������Ĺ��ܵ�.
*/
BOOL FILTERAPI FilterProcess(PZTFilterContext pZTFC,PBYTE pCmd,DWORD dwCmd);

/*
server�˳�ʱ����,���������������δ��ʼ.
*/
void FILTERAPI FilterCleanup(PZTFilterContext pZTFC);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_ZFILTER_H_
