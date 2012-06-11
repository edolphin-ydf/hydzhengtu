#include <zebra/SessionServer.h>

ForbidTalkManager *ForbidTalkManager::ftm = 0;

ForbidTalkManager::ForbidTalkManager(){}
ForbidTalkManager::~ForbidTalkManager(){}

ForbidTalkManager& ForbidTalkManager::getMe()
{
  if (!ftm)
    ftm = new ForbidTalkManager();
  return *ftm;
}

void ForbidTalkManager::delMe()
{
  SAFE_DELETE(ftm);
}

void ForbidTalkManager::checkDB()
{
  const dbCol forbid_define[] = {
    { "NAME",    zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "STARTTIME",        zDBConnPool::DB_QWORD,sizeof(QWORD) },
    { "DELAY",            zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "OPERATION",        zDBConnPool::DB_WORD,sizeof(WORD) },
    { "REASON",    zDBConnPool::DB_STR,  sizeof(char[255+1]) },
    { "GM",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "ISVALID",          zDBConnPool::DB_WORD,sizeof(WORD) },
    { NULL,0,0}
  };
  forbidInfo *recordList=0,*tempPoint=0;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("checkDB: �õ����ݿ���ʧ��");
    return;
  }
  char where[128];
  bzero(where,sizeof(where));
  strncpy(where,"ISVALID = 1",sizeof(where)-1);

  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`FORBIDTALK`",forbid_define,where,NULL,(BYTE **)&recordList);

  if ((DWORD)-1 == retcode
      || 0 == retcode)
  {
    SessionService::dbConnPool->putHandle(handle);
    return;
  }
  if (recordList)
  {
    tempPoint = &recordList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      UserSession *pUser = UserSessionManager::getInstance()->getUserSessionByName(tempPoint->name);
      if (pUser)
      {
        if (pUser->scene)
        {
          bzero(where,sizeof(where));
          std::string escapeName;
          _snprintf(where,sizeof(where)-1,"NAME = '%s'",SessionService::dbConnPool->escapeString(handle,tempPoint->name,escapeName).c_str());
          tempPoint->isValid = 0;
          DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`FORBIDTALK`",forbid_define,(BYTE*)(tempPoint),where);
          if (1 != retcode)
            Zebra::logger->error("�޸Ĵ�����¼ʧ�� name=%s,retcode=%d",tempPoint->name,retcode);

          Cmd::Session::t_forbidTalk_SceneSession forbid;
          strncpy(forbid.name,tempPoint->name,sizeof(forbid.name));
          strncpy(forbid.reason,tempPoint->reason,sizeof(forbid.reason));
          forbid.delay = tempPoint->delay;
          forbid.operation = tempPoint->operation;
          pUser->scene->sendCmd(&forbid,sizeof(forbid));
        }
      }
      tempPoint++;
    }
    SAFE_DELETE_VEC(recordList);
  }
  SessionService::dbConnPool->putHandle(handle);
}
