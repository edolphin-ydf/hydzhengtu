/**
 * \brief zebra��Ŀ�Ʒѷ�����
 *
 */

#include "BillServer.h"

zDBConnPool *BillService::dbConnPool = NULL;

BillService *BillService::instance = NULL;

DBMetaData* BillService::metaData = NULL;

zLogger* BillService::tradelog = NULL;

bool action(const BillData *bd);

/**
 * \brief ��ʼ���������������
 *
 * ʵ�����麯��<code>zService::init</code>
 *
 * \return �Ƿ�ɹ�
 */
bool BillService::init()
{
  Zebra::logger->debug("BillService::init");

  dbConnPool = zDBConnPool::newInstance(NULL);
  if (NULL == dbConnPool
      || !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
  {
    MessageBox(NULL,"�������ݿ�ʧ��","BillServer",MB_ICONERROR);
    return false;
  }

  metaData = DBMetaData::newInstance("");

  if (NULL == metaData
      || !metaData->init(Zebra::global["mysql"]))
  {
    MessageBox(NULL,"�������ݿ�ʧ��","BillServer",MB_ICONERROR);
    return false;
  }

  tradelog = new zLogger("tradelog");

  //������־����
  tradelog->setLevel(Zebra::global["log"]);
  //����д������־�ļ�
  if ("" != Zebra::global["goldtradelogfilename"])
  {
    tradelog->addLocalFileLog(Zebra::global["goldtradelogfilename"]);
    tradelog->removeConsoleLog();
  }

  //��ʼ�������̳߳�
  int state = state_none;
  to_lower(Zebra::global["threadPoolState"]);
  if ("repair" == Zebra::global["threadPoolState"]
      || "maintain" == Zebra::global["threadPoolState"])
    state = state_maintain;
  taskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
  if (NULL == taskPool
      || !taskPool->init())
    return false;

  strncpy(pstrIP,zSocket::getIPByIfName(Zebra::global["ifname"].c_str()),MAX_IP_LENGTH - 1);
  
  BillCallback bc;
  bc.action = action;

  /*
  GameZone_t gameZone;
  gameZone.game = 0;
  gameZone.zone = atoi(Zebra::global["zone"].c_str());
  // */

  if (!::Bill_init(Zebra::global["confdir"] + "billServerList.xml",Zebra::global["tradelogfilename"].c_str(),&bc) )
  //  || !::Bill_addserver(Zebra::global["BillServerIP"].c_str(),  atoi(Zebra::global["BillServerPort"].c_str())))
  {
    Zebra::logger->error("����BILL������ʧ��");
    return false;
  }
  ConsignGoldManager::getInstance()->init();
  ConsignMoneyManager::getInstance()->init();
  ConsignHistoryManager::getInstance()->init();
  BillTimeTick::getInstance().start();

  if (!zSubNetService::init())
  {
    return false;
  }

  
  return true;
}

/**
 * \brief �½���һ����������
 *
 * ʵ�ִ��麯��<code>zNetService::newTCPTask</code>
 *
 * \param sock TCP/IP����
 * \param addr ��ַ
 */
void BillService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
  //Zebra::logger->debug("BillService::newTCPTask");
  BillTask *tcpTask = new BillTask(taskPool,sock,addr);
  if (NULL == tcpTask)
    //�ڴ治��,ֱ�ӹر�����
    ::closesocket(sock);
  else if (!taskPool->addVerify(tcpTask))
  {
    //�õ���һ����ȷ����,��ӵ���֤������
    SAFE_DELETE(tcpTask);
  }
}

/**
 * \brief �������Թ����������ָ��
 *
 * ��Щָ�������غ͹��������������ָ��<br>
 * ʵ�����麯��<code>zSubNetService::msgParse_SuperService</code>
 *
 * \param pNullCmd ��������ָ��
 * \param nCmdLen ��������ָ���
 * \return �����Ƿ�ɹ�
 */
bool BillService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  using namespace Cmd::Super;

  switch(pNullCmd->para)
  {
    case PARA_BILL_NEWSESSION:
      {
        t_NewSession_Bill *ptCmd = (t_NewSession_Bill *)pNullCmd;
        Cmd::Bill::t_NewSession_Gateway tCmd;
        BillTask *task=BillTaskManager::getInstance().getTaskByID(ptCmd->session.wdGatewayID);
        if (!task)
        {
          Zebra::logger->error("�˺�%d��½ʱ�����Ѿ��ر�",ptCmd->session.accid);
          t_idinuse_Bill ret;
          ret.accid = ptCmd->session.accid;
          ret.loginTempID = ptCmd->session.loginTempID;
          ret.wdLoginID = ptCmd->session.wdLoginID;
          bcopy(ptCmd->session.name,ret.name,sizeof(ret.name),sizeof(ret.name));
          return sendCmdToSuperServer(&ret,sizeof(ret));
        }
        BillUser *pUser=new BillUser(ptCmd->session.accid,ptCmd->session.loginTempID,ptCmd->session.account,ptCmd->session.client_ip,task);
        if (!pUser || !BillUserManager::getInstance()->addUser(pUser))
        {
          //�ظ���½��֤
          Zebra::logger->error("�˺��Ѿ���½%s(%d,%d)",ptCmd->session.account,ptCmd->session.accid,ptCmd->session.loginTempID);
          t_idinuse_Bill ret;
          ret.accid = ptCmd->session.accid;
          ret.loginTempID = ptCmd->session.loginTempID;
          ret.wdLoginID = ptCmd->session.wdLoginID;
          bcopy(ptCmd->session.name,ret.name,sizeof(ret.name),sizeof(ret.name));
          return sendCmdToSuperServer(&ret,sizeof(ret));
        }
        Zebra::logger->info("�ʺŵ�½%s(%d,%d)",pUser->account,pUser->id,pUser->tempid);
        tCmd.session = ptCmd->session;

        return BillTaskManager::getInstance().broadcastByID(ptCmd->session.wdGatewayID,&tCmd,sizeof(tCmd));
      }
      break;
  }

  Zebra::logger->error("BillService::msgParse_SuperService(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

/**
 * \brief �������������
 *
 * ʵ���˴��麯��<code>zService::final</code>
 *
 */
void BillService::final()
{
  BillTimeTick::getInstance().final();
  BillTimeTick::getInstance().join();
  BillTimeTick::delInstance();

  if (taskPool)
  {
    SAFE_DELETE(taskPool);
  }
  BillManager::delInstance();

  BillTaskManager::delInstance();

  zSubNetService::final();

  ::Bill_final();
  zDBConnPool::delInstance(&dbConnPool);

  Zebra::logger->debug("BillService::final");
}

/**
 * \brief ��ȡ�����ļ�
 *
 */
class BillConfile:public zConfile
{
  bool parseYour(const xmlNodePtr node)
  {
    if (node)
    {
      xmlNodePtr child=parser.getChildNode(node,NULL);
      while(child)
      {
        parseNormal(child);
        child=parser.getNextNode(child,NULL);
      }
      return true;
    }
    else
      return false;
  }
};

/**
 * \brief ���¶�ȡ�����ļ�,ΪHUP�źŵĴ�����
 *
 */
void BillService::reloadConfig()
{
  Zebra::logger->debug("BillService::reloadConfig");
  BillConfile rc;
  rc.parse("BillServer");
  //ָ���⿪��
  if (Zebra::global["cmdswitch"] == "true")
  {
    zTCPTask::analysis._switch = true;
    zTCPClient::analysis._switch=true;
  }
  else
  {
    zTCPTask::analysis._switch = false;
    zTCPClient::analysis._switch=false;
  }
}

/**
 * \brief ���������
 *
 * \param argc ��������
 * \param argv �����б�
 * \return ���н��
 */
int main(int argc,char **argv)
{
  Zebra::logger=new zLogger("BillServer");

  //����ȱʡ����
  Zebra::global["goldtradelogfilename"] = "/tmp/goldtradebillserver.log";
  Zebra::global["tradelogfilename"]     = "/tmp/tradebillserver.log";

  //���������ļ�����
  BillConfile rc;
  if (!rc.parse("BillServer"))
    return EXIT_FAILURE;

  //ָ���⿪��
  if (Zebra::global["cmdswitch"] == "true")
  {
    zTCPTask::analysis._switch = true;
    zTCPClient::analysis._switch=true;
  }
  else
  {
    zTCPTask::analysis._switch = false;
    zTCPClient::analysis._switch=false;
  }

  //������־����
  Zebra::logger->setLevel(Zebra::global["log"]);
  //����д������־�ļ�
  if ("" != Zebra::global["logfilename"]){
    Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
        Zebra::logger->removeConsoleLog();
  }

  Zebra_Startup();
  
  BillService::getInstance().main();
  BillService::delInstance();

  return EXIT_SUCCESS;
}

bool action(const BillData *bd)
{
  //Zebra::logger->debug("action");
  using namespace Cmd::UserServer;
  
  switch(bd->at)
  {
    case AT_CONSUME:
      {
        if (bd)
        {
          Zebra::logger->debug("consume(uid = %d,tid = %s) ... %s",
              bd->uid,bd->tid,(bd->result == Cmd::UserServer::RET_OK)? "success" : "failure");
          BillUser *pUser = BillUserManager::getInstance()->getUserByID(bd->uid);
          if (pUser)
          {
            pUser->redeem_gold(bd);
          }
          else
          {
            BillUser::redeem_gold_err(bd);
          }
        }
        return true;
      }
      break;
    case AT_MCARD:
      {
        if (bd)
        {
          Zebra::logger->debug("consume(uid = %d,tid = %s) ... %s",
              bd->uid,bd->tid,(bd->result == Cmd::UserServer::RET_OK)? "success" : "failure");
          BillUser *pUser = BillUserManager::getInstance()->getUserByID(bd->uid);
          if (pUser)
          {
            pUser->redeem_moth_card(bd);
          }
        }
        return true;
      }
      break;
    case AT_PCARD:
    case AT_SCARD:
      {
        if (bd)
        {
          BillUser *pUser = BillUserManager::getInstance()->getUserByID(bd->uid);
          if (pUser)
          {
            pUser->redeem_object_card(bd);
          }
          else
          {
            BillUser::redeem_object_card_err(bd);
          }
        }
        return true;
      }
    case AT_FILLIN:
      {
        Zebra::logger->debug("fillin(uid = %d,tid = %s) ... %s",
          bd->uid,bd->tid,bd->result==Cmd::UserServer::RET_OK ? "success" : "failure"); 
        return true;
      }
      break;
    case AT_QBALANCE:
      {
        if (bd)
        {
          Zebra::logger->debug("qbanlance(uid = %d,tid = %s) ... %s",bd->uid,bd->tid,!bd->result ? "success" : "failure");
          BillUser *pUser = BillUserManager::getInstance()->getUserByID(bd->uid);
          if (pUser)
          {
            pUser->query_point(bd);
          }
        }
        return true;
      }
      break;
    default:
      return false;
  }
  
  return false;
}
/*
bool redeem_moth_card(const BillData* bd)
{
  Cmd::Bill::t_Redeem_MonthCard_Gateway send;
  DWORD old_vip_time=0;
  
  DBRecord column,where;                           
  std::ostringstream oss;         
  BillSession bs = BillSessionManager::getInstance().get(bd->tid);
  if (!bs.accid)
  {
    Zebra::logger->debug("%s�һ���ҷ���ʱû����ȷ��BillSession,���ܸ�����Ѿ��˳�",bd->tid);
    return false;
  }
  strncpy(send.account,bs.account,Cmd::UserServer::ID_MAX_LENGTH);
  send.accid = bs.accid;              /// �˺ű��
  send.charid = bs.charid;        /// ��ɫID

  //send.type = Cmd::TYPE_QUERY;

  if (bd->result == Cmd::UserServer::RET_OK)
  {
    DBRecordSet* recordset = NULL;
    DBFieldSet* balance = BillService::metaData->getFields("BALANCE");

      
    if (bs.accid != 0)
    {
      oss << "accid=" << bs.accid;
      where.put("accid",oss.str());
      oss.str("");

      oss << "charid=" << bs.charid;
      where.put("charid",oss.str());
      
      if (balance)
      {
        connHandleID handle = BillService::dbConnPool->getHandle();

        if ((connHandleID)-1 != handle)
        {
          recordset = BillService::dbConnPool->exeSelect(handle,balance,NULL,&where);

          if (recordset && !recordset->empty())
          {//�������н�Ҽ�¼
            oss.str("");

            old_vip_time = recordset->get(0)->get("monthcard");
            old_vip_time +=  30 * 24 * 60 * 60;
            column.put("monthcard",old_vip_time);

            if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,
                  balance,&column,&where))
            {
              send.byReturn = Cmd::REDEEM_FAIL;
            }      
            else
            {
              BillManager::getInstance().updateVipTime(bs.accid,old_vip_time);
              send.byReturn = Cmd::REDEEM_SUCCESS;
            }  
          }
          else
          {// û�жһ���¼,�����µļ�¼
            
            old_vip_time = time((time_t)NULL);
            old_vip_time +=  30 * 24 * 60 * 60;
            column.clear();
            column.put("account",bs.account);
            column.put("accid",bs.accid);
            column.put("charid",bs.charid);
            column.put("gold",0);
            column.put("allgold",0);
            column.put("monthcard",old_vip_time);
            column.put("allconsum",(int)0);

            if ((DWORD)-1 == BillService::dbConnPool->exeInsert(handle,balance,&column))
            {
              send.byReturn = Cmd::REDEEM_FAIL;
            }
            else
            {
              BillManager::getInstance().updateVipTime(bs.accid,old_vip_time);
              send.byReturn = Cmd::REDEEM_SUCCESS;
            }
          }
        }
        else
        {
          send.byReturn = Cmd::REDEEM_BUSY;
        }
      }
    }
    else
    {
      send.byReturn = Cmd::REDEEM_FAIL;
    }

    // ��ҷ���������ʧ��,��¼�һ���־
     // �ʺ�,TID,���׽��,�������,������
    BillService::tradelog->info("�������¿�:----------------------------------------");
    BillService::tradelog->info("�������¿�:%d,%s,%d,%d,%d",
        bd->uid,
        bd->tid,
        bd->result,
        bd->balance,
        old_vip_time  
        );  

  }
  else
  {
    send.byReturn = Cmd::REDEEM_FAIL;
  }


  if (bs.task)
  {
    bs.task->sendCmd(&send,sizeof(send));
  }

  return true;

}
bool query_point(const BillData* bd)
{
  Cmd::Bill::t_Return_Point send;
  
  DBRecord column,where;                           
  std::ostringstream oss;         
  BillSession bs = BillSessionManager::getInstance().get(bd->tid);
  if (!bs.accid)
  {
    Zebra::logger->debug("%s�һ���ҷ���ʱû����ȷ��BillSession,���ܸ�����Ѿ��˳�",bd->tid);
    return false;
  }
  strncpy(send.account,bs.account,Cmd::UserServer::ID_MAX_LENGTH);
  send.accid = bs.accid;              /// �˺ű��
  send.charid = bs.charid;        /// ��ɫID

  //send.type = Cmd::TYPE_QUERY;

  if (bd->result == Cmd::UserServer::RET_OK)
  {
    send.dwPoint = bd->balance;
    send.byReturn = Cmd::REDEEM_SUCCESS;
  }
  else
  {
    send.byReturn = Cmd::REDEEM_FAIL;
  }
  if (bs.task)
  {
    bs.task->sendCmd(&send,sizeof(send));
  }
  return true;
}
bool redeem_gold(const BillData* bd)
{
  Cmd::Bill::t_Redeem_Gold_Gateway send;
  
  DBRecord column,where;                           
  std::ostringstream oss;         
  int rate = REDEEM_RATE_GOLD;  // ���������һ�����
  double gold = 0.0;  // ��������
  double last_gold = 0.0; // �ϴν�����
  BillSession bs = BillSessionManager::getInstance().get(bd->tid);
  if (!bs.accid)
  {
    Zebra::logger->debug("%s�һ���ҷ���ʱû����ȷ��BillSession,���ܸ�����Ѿ��˳�",bd->tid);
    return false;
  }
  strncpy(send.account,bs.account,Cmd::UserServer::ID_MAX_LENGTH);
  send.accid = bs.accid;              /// �˺ű��
  send.charid = bs.charid;        /// ��ɫID

  //send.type = Cmd::TYPE_QUERY;

  if (bd->result == Cmd::UserServer::RET_OK)
  {
    DBRecordSet* recordset = NULL;
    DBFieldSet* balance = BillService::metaData->getFields("BALANCE");

      
    if (bs.accid != 0)
    {
      oss << "accid=" << bs.accid;
      where.put("accid",oss.str());
      oss.str("");

      oss << "charid=" << bs.charid;
      where.put("charid",oss.str());
      
      gold = bs.point /(double) rate;

      if (balance)
      {
        connHandleID handle = BillService::dbConnPool->getHandle();

        if ((connHandleID)-1 != handle)
        {
          recordset = BillService::dbConnPool->exeSelect(handle,balance,NULL,&where);

          if (recordset && !recordset->empty())
          {//�������н�Ҽ�¼
            oss.str("");

            last_gold = recordset->get(0)->get("gold");
            
            oss << "gold+" << gold;
            column.put("gold",oss.str());

            oss.str("");
            oss << "allgold+" << gold;
            column.put("allgold",oss.str());

            if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,
                  balance,&column,&where))
            {
              send.byReturn = Cmd::REDEEM_FAIL;
            }      
            else
            {
              last_gold = last_gold + gold;
              BillManager::getInstance().updateGold(bs.accid,last_gold);
              send.byReturn = Cmd::REDEEM_SUCCESS;
            }  
          }
          else
          {// û�жһ���¼,�����µļ�¼
            last_gold = 0;
            
            column.clear();
            column.put("account",bs.account);
            column.put("accid",bs.accid);
            column.put("charid",bs.charid);
            column.put("gold",gold);
            column.put("allgold",gold);
            column.put("monthcard",0);
            column.put("allconsum",(int)0);

            if ((DWORD)-1 == BillService::dbConnPool->exeInsert(handle,balance,&column))
            {
              send.byReturn = Cmd::REDEEM_FAIL;
            }
            else
            {
              last_gold = gold;
              BillManager::getInstance().updateGold(bs.accid,last_gold);
              send.byReturn = Cmd::REDEEM_SUCCESS;
            }
          }
        }
        else
        {
          send.byReturn = Cmd::REDEEM_BUSY;
        }
      }
    }
    else
    {
      send.byReturn = Cmd::REDEEM_FAIL;
    }

    // ��ҷ���������ʧ��,��¼�һ���־
     // �ʺ�,TID,���׽��,�������,������
    //BillService::tradelog->info("���������:----------------------------------------");
    BillService::tradelog->info("���������:%d,%s,%d,%d,%f",
        bd->uid,
        bd->tid,
        bd->result,
        bd->balance,
        last_gold
        );  

  }
  else
  {
    send.byReturn = Cmd::REDEEM_FAIL;
  }

  send.dwGold = (DWORD)last_gold;

  if (bs.task)
  {
    bs.task->sendCmd(&send,sizeof(send));
  }

  return true;

}


  // */
