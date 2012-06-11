/**
 * \brief ����Ʒѷ��������ӿͻ���
 *
 */

#include "GatewayServer.h"

/**
 * \brief �Ʒѷ��������ӿͻ���
 *
 * һ������ֻ��һ���Ʒѷ�����,��������ֻ��Ҫ����һ��ָ��,����Ҫ���ӹ�����֮��Ķ���
 *
 */
MiniClient *miniClient = NULL;

/**
 * \brief ������Mini������������
 *
 * \return �����Ƿ�ɹ�
 */
bool MiniClient::connectToMiniServer()
{
  if (!connect())
  {
    Zebra::logger->error("����Mini������ʧ��");
    return false;
  }

  using namespace Cmd::Mini;
  t_LoginMini tCmd;
  tCmd.wdServerID = GatewayService::getInstance().getServerID();
  tCmd.wdServerType = GatewayService::getInstance().getServerType();

  return sendCmd(&tCmd,sizeof(tCmd));
}

/**
 * \brief ����zThread�еĴ��麯��,���̵߳����ص�����,���ڴ�����յ���ָ��
 *
 */
void MiniClient::run()
{
  zTCPBufferClient::run();

  while(!GatewayService::getInstance().isTerminate())
  {
    while(!connect())
    {
      Zebra::logger->error("����С��Ϸ������ʧ��");
      zThread::msleep(1000);
    }
    Cmd::Super::t_restart_ServerEntry_NotifyOther notify;
    notify.srcID=GatewayService::getInstance().getServerID();
    notify.dstID=this->getServerID();
    GatewayService::getInstance().sendCmdToSuperServer(&notify,sizeof(notify));
    zThread::msleep(2000);
    connect();
    using namespace Cmd::Mini;
    t_LoginMini tCmd;
    tCmd.wdServerID = GatewayService::getInstance().getServerID();
    tCmd.wdServerType = GatewayService::getInstance().getServerType();

    if (sendCmd(&tCmd,sizeof(tCmd)))
    {
      zTCPBufferClient::run();
    }
      // */
    zThread::msleep(1000);
  }
  //��Mini֮������ӶϿ�,����Ҫ�رշ�����
  //GatewayService::getInstance().Terminate();
}

/**
 * \brief ��������Mini������������ָ��
 *
 * \param pNullCmd ��������ָ��
 * \param nCmdLen ��������ָ���
 * \return �����Ƿ�ɹ�
 */
bool MiniClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
  Zebra::logger->error("?? MiniClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif

  using namespace Cmd::Mini;
  using namespace Cmd;

  if (CMD_GATE == pNullCmd->cmd)
  {
    switch(pNullCmd->para)
    {
      default:
        break;
    }
    return false;
  }

  if (CMD_FORWARD == pNullCmd->cmd)
  {
    switch(pNullCmd->para)
    {
      case PARA_MINI_FORWARD_USER:
        {
          t_Mini_ForwardUser *rev = (t_Mini_ForwardUser *)pNullCmd;
          GateUser *pUser=(GateUser *)GateUserManager::getInstance()->getUserByID(rev->id);
          if (!pUser || !pUser->sendCmd(rev->data,rev->size))
            Zebra::logger->debug("ת��MINI������ %u��%u %u��Ϣʧ��",rev->id,((Cmd::stNullUserCmd *)rev->data)->byCmd,((Cmd::stNullUserCmd *)rev->data)->byParam);
          return true;
        }
        break;
        /*
           case PARA_FORWARD_MINI_TO_SCENE:
           {
           t_Mini_ForwardMiniToScene *rev =(t_Mini_ForwardMiniToScene*)pNullCmd;
           GateUser *pUser=(GateUser *)GateUserManager::getInstance()->getUserByAccID(rev->id);
           if (!pUser || !pUser->forwardSceneMini((const Cmd::stNullUserCmd*)rev->data,(DWORD)rev->size))
           {
           Zebra::logger->debug("ת��MINI�������ʺ�%ld�ĳ���%u %u��Ϣʧ��",rev->id,
           ((Cmd::stNullUserCmd *)rev->data)->byCmd,((Cmd::stNullUserCmd *)rev->data)->byParam);
           }
           }
           break;
           */
    }
  }

  Zebra::logger->error("MiniClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

