/**
 * \brief �����½�������ͻ���
 *
 * 
 */

#include "SuperServer.h"

/**
 * \brief ��ʱ��ŷ�����
 *
 */
WORD FLClient::tempidAllocator = 0;

/**
 * \brief ���캯��
 * \param ip ��������ַ
 * \param port �������˿�
 */
FLClient::FLClient(
    const std::string &ip,
    const WORD port) : zTCPClientTask(ip,port,true),tempid(++tempidAllocator),netType(NetType_near)
{
  Zebra::logger->debug("FLClient::FLClient(%s:%u)",ip.c_str(),port);
}

/**
 * \brief ��������
 *
 */
FLClient::~FLClient()
{
  Zebra::logger->debug("FLClient::~FLClient");
}

int FLClient::checkRebound()
{
  Zebra::logger->debug("FLClient::checkRebound");
  int retcode = pSocket->recvToBuf_NoPoll();
  if (retcode > 0)
  {
    BYTE pstrCmd[zSocket::MAX_DATASIZE];
    int nCmdLen = pSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
    if (nCmdLen <= 0)
      //����ֻ�Ǵӻ���ȡ���ݰ������Բ������û������ֱ�ӷ���
      return 0;
    else
    {
      using namespace Cmd::FL;

      t_LoginFL_OK *ptCmd = (t_LoginFL_OK *)pstrCmd;
      if (CMD_LOGIN == ptCmd->cmd
          && PARA_LOGIN_OK == ptCmd->para)
      {
        Zebra::logger->debug("��½FLServer�ɹ����յ����ı�ţ�zoneid=%u(gameid=%u,zone=%u),name=%s,nettype=%u",
            ptCmd->gameZone.id,
            ptCmd->gameZone.game,
            ptCmd->gameZone.zone,
            ptCmd->name,
            ptCmd->netType);
        netType = (ptCmd->netType == 0 ? NetType_near : NetType_far);
        SuperService::getInstance().setZoneID(ptCmd->gameZone);
        SuperService::getInstance().setZoneName(ptCmd->name);
        return 1;
      }
      else
      {
        Zebra::logger->error("��½FLServerʧ��");
        return -1;
      }
    }
  }
  else
    return retcode;
}

void FLClient::addToContainer()
{
  Zebra::logger->debug("FLClient::addToContainer");
  FLClientManager::getInstance().add(this);
}

void FLClient::removeFromContainer()
{
  Zebra::logger->debug("FLClient::removeFromContainer");
  FLClientManager::getInstance().remove(this);
}

bool FLClient::connect()
{
  Zebra::logger->debug("FLClient::connect");
  if (!zTCPClientTask::connect())
    return false;

  using namespace Cmd::FL;
  t_LoginFL cmd;
  strncpy(cmd.strIP,SuperService::getInstance().getIP(),sizeof(cmd.strIP));
  cmd.port = SuperService::getInstance().getPort();
  return sendCmd(&cmd,sizeof(cmd));
}

bool FLClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
  Zebra::logger->error("?? FLClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif

  using namespace Cmd::FL;

  switch(pNullCmd->cmd)
  {
    case CMD_GYLIST:
      if (msgParse_gyList(pNullCmd,nCmdLen))
        return true;
      break;
    case CMD_SESSION:
      if (msgParse_session(pNullCmd,nCmdLen))
        return true;
      break;
  }

  Zebra::logger->error("FLClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

bool FLClient::msgParse_gyList(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  Zebra::logger->debug("FLClient::msgParse_gyList");
  using namespace Cmd::FL;

  switch(pNullCmd->para)
  {
    case PARA_FL_RQGYLIST:
      {
        //t_RQGYList_FL *ptCmd = (t_RQGYList_FL *)pNullCmd;
        Cmd::Super::t_RQGYList_Gateway tCmd;

        return ServerManager::getInstance().broadcastByType(GATEWAYSERVER,&tCmd,sizeof(tCmd));
      }
      break;
  }

  Zebra::logger->error("FLClient::msgParse_gyList(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

bool FLClient::msgParse_session(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  Zebra::logger->debug("FLClient::msgParse_session");
  using namespace Cmd::FL;

  switch(pNullCmd->para)
  {
    case PARA_SESSION_NEWSESSION:
      {
        t_NewSession_Session *ptCmd = (t_NewSession_Session *)pNullCmd;
        Cmd::Super::t_NewSession_Bill tCmd;

        tCmd.session = ptCmd->session;
        tCmd.session.wdLoginID = tempid;
        //bcopy(&ptCmd->session,&tCmd.session,sizeof(tCmd.session));

        return ServerManager::getInstance().broadcastByType(BILLSERVER,&tCmd,sizeof(tCmd));
      }
      break;
  }

  Zebra::logger->error("FLClient::msgParse_session(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

