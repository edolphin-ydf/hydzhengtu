/**
 * \brief ʱ��ص�����
 *
 * 
 */

#include "SuperServer.h"

zRTime SuperTimeTick::currentTime;
SuperTimeTick *SuperTimeTick::instance = NULL;

void SuperTimeTick::run()
{
  readTime();

  while(!isFinal())
  {
    zThread::sleep(1);

    currentTime.now();
    Zebra::qwGameTime = qwStartGameTime + startTime.elapse(currentTime) / 1000;
    
    if (0 != Zebra::qwGameTime
        && 0 == Zebra::qwGameTime % 600)
    {
      saveTime();
    }

    Cmd::Super::t_GameTime tCmd;
    tCmd.qwGameTime = Zebra::qwGameTime;
    ServerManager::getInstance().broadcast(&tCmd,sizeof(tCmd));
  }

  saveTime();
}

static const dbCol gametime_define[] =
{
  {"`GAMETIME`",zDBConnPool::DB_QWORD,sizeof(QWORD)},
  {NULL,0,0}
};

bool SuperTimeTick::readTime()
{
  connHandleID handle = SuperService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܴ����ݿ����ӳػ�ȡ���Ӿ��");
    return false;
  }

  DWORD retcode = SuperService::dbConnPool->exeSelectLimit(handle,"`GAMETIME`",gametime_define,NULL,NULL,1,(BYTE*)(&qwStartGameTime));
  if ((DWORD)-1 == retcode)
  {
    SuperService::dbConnPool->putHandle(handle);
    Zebra::logger->error("��ȡ��Ϸʱ�����");
    return false;
  }

  if (0 == retcode)
  {
    qwStartGameTime = 0;
    retcode = SuperService::dbConnPool->exeInsert(handle,"`GAMETIME`",gametime_define,(const BYTE *)(&qwStartGameTime));
    if ((DWORD)-1 == retcode)
    {
      SuperService::dbConnPool->putHandle(handle);
      Zebra::logger->error("��ʼ����Ϸʱ�����");
      return false;
    }
  }
  SuperService::dbConnPool->putHandle(handle);

  Zebra::logger->debug("��ȡ��Ϸʱ��ɹ�");
  Zebra::qwGameTime = qwStartGameTime;
  startTime.now();

  return true;
}

bool SuperTimeTick::saveTime()
{
  connHandleID handle = SuperService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܴ����ݿ����ӳػ�ȡ���Ӿ��");
    return false;
  }

  DWORD retcode = SuperService::dbConnPool->exeUpdate(handle,"`GAMETIME`",gametime_define,(BYTE*)(&Zebra::qwGameTime),NULL);
  SuperService::dbConnPool->putHandle(handle);

  if (1 == retcode)
  {
    Zebra::logger->debug("������Ϸʱ��ɹ�");
  }
  else
  {
    Zebra::logger->error("������Ϸʱ��ʧ��");
  }

  return true;
}

