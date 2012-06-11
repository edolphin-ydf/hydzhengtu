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
BillClient *accountClient = NULL;

/**
 * \brief ������Bill������������
 *
 * \return �����Ƿ�ɹ�
 */
bool BillClient::connectToBillServer()
{
  if (!connect())
  {
    Zebra::logger->error("����Bill������ʧ��");
    return false;
  }

  using namespace Cmd::Bill;
  t_LoginBill tCmd;
  tCmd.wdServerID = GatewayService::getInstance().getServerID();
  tCmd.wdServerType = GatewayService::getInstance().getServerType();

  return sendCmd(&tCmd,sizeof(tCmd));
}

/**
 * \brief ����zThread�еĴ��麯��,���̵߳����ص�����,���ڴ�����յ���ָ��
 *
 */
void BillClient::run()
{
  zTCPBufferClient::run();

  while(!GatewayService::getInstance().isTerminate())
  {
    while(!connect())
    {
      Zebra::logger->error("���ӼƷѷ�����ʧ��");
      zThread::msleep(1000);
    }
    Cmd::Super::t_restart_ServerEntry_NotifyOther notify;
    notify.srcID=GatewayService::getInstance().getServerID();
    notify.dstID=this->getServerID();
    GatewayService::getInstance().sendCmdToSuperServer(&notify,sizeof(notify));
    zThread::msleep(2000);
    connect();
    using namespace Cmd::Bill;
    t_LoginBill tCmd;
    tCmd.wdServerID = GatewayService::getInstance().getServerID();
    tCmd.wdServerType = GatewayService::getInstance().getServerType();

    if (sendCmd(&tCmd,sizeof(tCmd)))
    {
      zTCPBufferClient::run();
    }
      // */
    zThread::msleep(1000);
  }
  //��Bill֮������ӶϿ�,��Ҫ�رշ�����
  GatewayService::getInstance().Terminate();
}

/**
 * \brief ��������Bill������������ָ��
 *
 * \param pNullCmd ��������ָ��
 * \param nCmdLen ��������ָ���
 * \return �����Ƿ�ɹ�
 */
bool BillClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE
  Zebra::logger->error("BillClient::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif

  using namespace Cmd::Bill;
  using namespace Cmd;

  switch(pNullCmd->cmd)
  {
  case CMD_GATE:
    switch(pNullCmd->para)
    {
      case PARA_GATE_LOGINVERIFY_RETURN:
        {
          t_LoginVerify_Gateway_Return *ptCmd = (t_LoginVerify_Gateway_Return *)pNullCmd;

                    GatewayTaskManager::getInstance().accountVerifyOK(ptCmd->accid,ptCmd->retcode);
            Zebra::logger->info("PARA_GATE_LOGINVERIFY_RETURN %d,%d",ptCmd->accid,ptCmd->retcode);            
          return true;
        }
        break;
      case PARA_GATE_NEWSESSION:
        {
          t_NewSession_Gateway *ptCmd = (t_NewSession_Gateway *)pNullCmd;
          Cmd::Super::t_NewSession_Gateway tCmd;

          Zebra::logger->info("PARA_GATE_NEWSESSION %d,%d,%s:%d",ptCmd->session.accid,ptCmd->session.loginTempID,ptCmd->session.pstrIP,ptCmd->session.wdPort);
          LoginSessionManager::getInstance().put(ptCmd->session);
          tCmd.session = ptCmd->session;
          return GatewayService::getInstance().sendCmdToSuperServer(&tCmd,sizeof(tCmd));
        }
        break;
     }
         break;
    case CMD_REDEEM:
     switch(pNullCmd->para)
     {
      case PARA_GATE_REDEEM_GOLD:
        {
          t_Redeem_Gold_Gateway *ptCmd=(t_Redeem_Gold_Gateway *)pNullCmd; 

                    //Zebra::logger->debug("Billת���������Ķһ����ָ��");
                    Zebra::logger->info("PARA_GATE_REDEEM_GOLD");
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(ptCmd->accid);
          if (pUser)
          {
            stRedeemGold rd;
            rd.dwNum = ptCmd->dwGold;
            rd.dwBalance = ptCmd->dwBalance;
            rd.byReturn = ptCmd->byReturn;
            pUser->forwardScene(&rd,sizeof(rd));
          }
          return true;
        }
        break;
      case PARA_GATE_REDEEM_MONTH_CARD:
        {
          t_Redeem_MonthCard_Gateway *ptCmd=(t_Redeem_MonthCard_Gateway *)pNullCmd;

                    //Zebra::logger->debug("Billת���������Ķһ��¿�ָ��");
                    Zebra::logger->info("PARA_GATE_REDEEM_MONTH_CARD");
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(ptCmd->accid);
          if (pUser)
          {
            stRedeemMonthCard rd;
            rd.byReturn = ptCmd->byReturn;
            rd.dwNum = ptCmd->dwNum;
            rd.dwBalance = ptCmd->dwBalance;
            pUser->forwardScene(&rd,sizeof(rd));
            if (ptCmd->byReturn ==Cmd::REDEEM_SUCCESS) 
            {
              pUser->setVip(ptCmd->dwNum>0?true:false);
            }
          }
          return true;
        }
        break;
      case PARA_GATE_RETURN_CARD_GOLD:
        {
          t_Return_Card_Gold *ptCmd=(t_Return_Card_Gold *)pNullCmd;

                    //Zebra::logger->debug("Billת���������Ĳ�ѯ����¿�ָ��");
                    Zebra::logger->info("PARA_GATE_RETURN_CARD_GOLD");
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(ptCmd->accid);
          if (pUser)
          {
            stReturnCardAndGold rc;
            rc.byReturn = ptCmd->byReturn;
            rc.dwGold = ptCmd->dwGold;
            rc.dwMonthCard = ptCmd->dwMonthCard;
            pUser->forwardScene(&rc,sizeof(rc));
            if (ptCmd->byReturn ==Cmd::REDEEM_SUCCESS) 
            {
              pUser->setVip(ptCmd->dwMonthCard>0?true:false);
            }
          }
          return true;
        }
        break;
      case PARA_GATE_RETURN_POINT:
        {
          t_Return_Point *ptCmd=(t_Return_Point *)pNullCmd;

                    Zebra::logger->info("PARA_GATE_RETURN_POINT");
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(ptCmd->accid);
          if (pUser)
          {
            stReturnRequestPoint rc;
            rc.byReturn = ptCmd->byReturn;
            rc.dwPoint = ptCmd->dwPoint;
            pUser->forwardScene(&rc,sizeof(rc));
          }
          return true;
        }
        break;
      case PARA_GATE_RETURN_CARD:
        {
          t_Return_ObjCard *ptCmd=(t_Return_ObjCard *)pNullCmd;

                    Zebra::logger->info("PARA_GATE_RETURN_CARD");
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(ptCmd->accid);
          if (pUser)
          {
            stReturnConSumeCardCard  ccc;
            ccc.byReturn = ptCmd->byReturn;
            ccc.byType = ptCmd->subatt;
            ccc.balance = ptCmd->balance;
            pUser->forwardScene(&ccc,sizeof(ccc));
          }
                    return true;
        }
        break;
     }
         break;
    case CMD_FORWARD:
     switch(pNullCmd->para)
     {
      case PARA_FORWARD_USER:
        {
          t_Bill_ForwardUser *rev=(t_Bill_ForwardUser *)pNullCmd;

                    Zebra::logger->info("PARA_FORWARD_USER %ld��%d %d��Ϣ",rev->dwAccid,
                ((Cmd::stNullUserCmd *)rev->data)->byCmd,((Cmd::stNullUserCmd *)rev->data)->byParam);

          GateUser *pUser=(GateUser *)GateUserManager::getInstance()->getUserByAccID(rev->dwAccid);
          if (!pUser ||  !pUser->sendCmd(rev->data,rev->size))
            Zebra::logger->debug("ת��BILL�������ʺ�%ld��%d %d��Ϣʧ��",rev->dwAccid,
                ((Cmd::stNullUserCmd *)rev->data)->byCmd,((Cmd::stNullUserCmd *)rev->data)->byParam);
          return true;
        }
        break;
      case PARA_FORWARD_BILL_TO_SCENE:
        {
          t_Bill_ForwardBillToScene *rev =(t_Bill_ForwardBillToScene*)pNullCmd;

                    Zebra::logger->debug("PARA_FORWARD_BILL_TO_SCENE %ld�ĳ���%d %d��Ϣ",rev->id,
                ((Cmd::stNullUserCmd *)rev->data)->byCmd,((Cmd::stNullUserCmd *)rev->data)->byParam);
          GateUser *pUser=(GateUser *)GateUserManager::getInstance()->getUserByAccID(rev->id);
          if (!pUser || !pUser->forwardSceneBill((const Cmd::stNullUserCmd*)rev->data,(DWORD)rev->size))
          {
            Zebra::logger->debug("ת��BILL�������ʺ�%ld�ĳ���%d %d��Ϣʧ��",rev->id,
                ((Cmd::stNullUserCmd *)rev->data)->byCmd,((Cmd::stNullUserCmd *)rev->data)->byParam);
          }
                    return true;
        }
        break;

     }
       break;
  }
  Zebra::logger->error("BillClient::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}
