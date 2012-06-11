/**
 * \brief zebra项目计费服务器
 *
 */

#include "BillServer.h"

zDBConnPool *BillService::dbConnPool = NULL;

BillService *BillService::instance = NULL;

DBMetaData* BillService::metaData = NULL;

zLogger* BillService::tradelog = NULL;

bool action(const BillData *bd);

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool BillService::init()
{
  Zebra::logger->debug("BillService::init");

  dbConnPool = zDBConnPool::newInstance(NULL);
  if (NULL == dbConnPool
      || !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
  {
    MessageBox(NULL,"连接数据库失败","BillServer",MB_ICONERROR);
    return false;
  }

  metaData = DBMetaData::newInstance("");

  if (NULL == metaData
      || !metaData->init(Zebra::global["mysql"]))
  {
    MessageBox(NULL,"连接数据库失败","BillServer",MB_ICONERROR);
    return false;
  }

  tradelog = new zLogger("tradelog");

  //设置日志级别
  tradelog->setLevel(Zebra::global["log"]);
  //设置写本地日志文件
  if ("" != Zebra::global["goldtradelogfilename"])
  {
    tradelog->addLocalFileLog(Zebra::global["goldtradelogfilename"]);
    tradelog->removeConsoleLog();
  }

  //初始化连接线程池
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
    Zebra::logger->error("连接BILL服务器失败");
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
 * \brief 新建立一个连接任务
 *
 * 实现纯虚函数<code>zNetService::newTCPTask</code>
 *
 * \param sock TCP/IP连接
 * \param addr 地址
 */
void BillService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
  //Zebra::logger->debug("BillService::newTCPTask");
  BillTask *tcpTask = new BillTask(taskPool,sock,addr);
  if (NULL == tcpTask)
    //内存不足,直接关闭连接
    ::closesocket(sock);
  else if (!taskPool->addVerify(tcpTask))
  {
    //得到了一个正确连接,添加到验证队列中
    SAFE_DELETE(tcpTask);
  }
}

/**
 * \brief 解析来自管理服务器的指令
 *
 * 这些指令是网关和管理服务器交互的指令<br>
 * 实现了虚函数<code>zSubNetService::msgParse_SuperService</code>
 *
 * \param pNullCmd 待解析的指令
 * \param nCmdLen 待解析的指令长度
 * \return 解析是否成功
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
          Zebra::logger->error("账号%d登陆时网关已经关闭",ptCmd->session.accid);
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
          //重复登陆验证
          Zebra::logger->error("账号已经登陆%s(%d,%d)",ptCmd->session.account,ptCmd->session.accid,ptCmd->session.loginTempID);
          t_idinuse_Bill ret;
          ret.accid = ptCmd->session.accid;
          ret.loginTempID = ptCmd->session.loginTempID;
          ret.wdLoginID = ptCmd->session.wdLoginID;
          bcopy(ptCmd->session.name,ret.name,sizeof(ret.name),sizeof(ret.name));
          return sendCmdToSuperServer(&ret,sizeof(ret));
        }
        Zebra::logger->info("帐号登陆%s(%d,%d)",pUser->account,pUser->id,pUser->tempid);
        tCmd.session = ptCmd->session;

        return BillTaskManager::getInstance().broadcastByID(ptCmd->session.wdGatewayID,&tCmd,sizeof(tCmd));
      }
      break;
  }

  Zebra::logger->error("BillService::msgParse_SuperService(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

/**
 * \brief 结束网络服务器
 *
 * 实现了纯虚函数<code>zService::final</code>
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
 * \brief 读取配置文件
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
 * \brief 重新读取配置文件,为HUP信号的处理函数
 *
 */
void BillService::reloadConfig()
{
  Zebra::logger->debug("BillService::reloadConfig");
  BillConfile rc;
  rc.parse("BillServer");
  //指令检测开关
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
 * \brief 主程序入口
 *
 * \param argc 参数个数
 * \param argv 参数列表
 * \return 运行结果
 */
int main(int argc,char **argv)
{
  Zebra::logger=new zLogger("BillServer");

  //设置缺省参数
  Zebra::global["goldtradelogfilename"] = "/tmp/goldtradebillserver.log";
  Zebra::global["tradelogfilename"]     = "/tmp/tradebillserver.log";

  //解析配置文件参数
  BillConfile rc;
  if (!rc.parse("BillServer"))
    return EXIT_FAILURE;

  //指令检测开关
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

  //设置日志级别
  Zebra::logger->setLevel(Zebra::global["log"]);
  //设置写本地日志文件
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
    Zebra::logger->debug("%s兑换金币返回时没有正确的BillSession,可能该玩家已经退出",bd->tid);
    return false;
  }
  strncpy(send.account,bs.account,Cmd::UserServer::ID_MAX_LENGTH);
  send.accid = bs.accid;              /// 账号编号
  send.charid = bs.charid;        /// 角色ID

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
          {//更新已有金币记录
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
          {// 没有兑换记录,插入新的记录
            
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

    // 金币服务器操作失败,记录兑换日志
     // 帐号,TID,交易结果,点数余额,金币余额
    BillService::tradelog->info("点数换月卡:----------------------------------------");
    BillService::tradelog->info("点数换月卡:%d,%s,%d,%d,%d",
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
    Zebra::logger->debug("%s兑换金币返回时没有正确的BillSession,可能该玩家已经退出",bd->tid);
    return false;
  }
  strncpy(send.account,bs.account,Cmd::UserServer::ID_MAX_LENGTH);
  send.accid = bs.accid;              /// 账号编号
  send.charid = bs.charid;        /// 角色ID

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
  int rate = REDEEM_RATE_GOLD;  // 金币与点数兑换比率
  double gold = 0.0;  // 待充金币数
  double last_gold = 0.0; // 上次金币余额
  BillSession bs = BillSessionManager::getInstance().get(bd->tid);
  if (!bs.accid)
  {
    Zebra::logger->debug("%s兑换金币返回时没有正确的BillSession,可能该玩家已经退出",bd->tid);
    return false;
  }
  strncpy(send.account,bs.account,Cmd::UserServer::ID_MAX_LENGTH);
  send.accid = bs.accid;              /// 账号编号
  send.charid = bs.charid;        /// 角色ID

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
          {//更新已有金币记录
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
          {// 没有兑换记录,插入新的记录
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

    // 金币服务器操作失败,记录兑换日志
     // 帐号,TID,交易结果,点数余额,金币余额
    //BillService::tradelog->info("点数换金币:----------------------------------------");
    BillService::tradelog->info("点数换金币:%d,%s,%d,%d,%f",
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
