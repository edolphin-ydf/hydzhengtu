/**
 * \brief zebra项目Gateway服务器,负责用户指令检查转发、加密解密等
 */

#include "GatewayServer.h"

GatewayService *GatewayService::instance = NULL;
zTCPTaskPool * GatewayService::taskPool = NULL;
bool GatewayService::service_gold=true;
bool GatewayService::service_stock=true;
DWORD merge_version = 0;

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool GatewayService::init()
{
  Zebra::logger->debug("GatewayService::init");
  verify_client_version = ZEBRA_CLIENT_VERSION;
  Zebra::logger->info("服务器版本号:%d",verify_client_version);
  
  //加载国家名称(地图)信息
  if (!country_info.init())
  {
    Zebra::logger->error("加载地图名称失败!");
  }

  //初始化连接线程池
  int state = state_none;
  to_lower(Zebra::global["threadPoolState"]);
  if ("repair" == Zebra::global["threadPoolState"]
      || "maintain" == Zebra::global["threadPoolState"])
    state = state_maintain;

  taskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state,65000);
  if (NULL == taskPool
      || !taskPool->init())
    return false;

  strncpy(pstrIP,zSocket::getIPByIfName(Zebra::global["ifname"].c_str()),MAX_IP_LENGTH - 1);
  Zebra::logger->info("GatewayService::init(%s)",pstrIP);

  if (!zSubNetService::init())
  {
    return false;
  }

  const Cmd::Super::ServerEntry *serverEntry = NULL;
  
  //连接会话服务器
  serverEntry = getServerEntryByType(SESSIONSERVER);
  if (NULL == serverEntry)
  {
    Zebra::logger->error("不能找到会话服务器相关信息,不能连接会话服务器");
    return false;
  }
  sessionClient = new SessionClient("会话服务器客户端",serverEntry->pstrIP,serverEntry->wdPort);
  if (NULL == sessionClient)
  {
    Zebra::logger->error("没有足够内存,不能建立会话服务器客户端实例");
    return false;
  }
  if (!sessionClient->connectToSessionServer())
  {
    Zebra::logger->error("GatewayService::init 连接会话服务器失败");
    //return false;
  }
  sessionClient->start();

  //连接计费服务器
  serverEntry = getServerEntryByType(BILLSERVER);
  if (NULL == serverEntry)
  {
    Zebra::logger->error("不能找到计费服务器相关信息,不能连接计费服务器");
    return false;
  }
  accountClient = new BillClient("计费服务器客户端",serverEntry->pstrIP,serverEntry->wdPort,serverEntry->wdServerID);
  if (NULL == accountClient)
  {
    Zebra::logger->error("没有足够内存,不能建立计费服务器客户端实例");
    return false;
  }
  if (!accountClient->connectToBillServer())
  {
    Zebra::logger->error("GatewayService::init 连接计费服务器失败");
    return false;
  }
  accountClient->start();

  //连接所有的场景服务器
  serverEntry = getServerEntryByType(SCENESSERVER);
  if (serverEntry)
  {
    if (!SceneClientManager::getInstance().init())
      return false;
  }  

  //连接所有的档案服务器
  serverEntry = getServerEntryByType(RECORDSERVER);
  if (NULL == serverEntry)
  {
    Zebra::logger->error("不能找到档案服务器相关信息,不能连接档案服务器");
    return false;
  }
  recordClient = new RecordClient("档案服务器客户端",serverEntry->pstrIP,serverEntry->wdPort);
  if (NULL == recordClient)
  {
    Zebra::logger->error("没有足够内存,不能建立档案服务器客户端实例");
    return false;
  }
  if (!recordClient->connectToRecordServer())
  {
    Zebra::logger->error("GatewayService::init 连接档案服务器失败");
    return false;
  }
  recordClient->start();

  //连接小游戏服务器
  serverEntry = getServerEntryByType(MINISERVER);
  if (NULL == serverEntry)
  {
    Zebra::logger->error("不能找到小游戏服务器相关信息,不能连接小游戏服务器");
    return false;
  }
  miniClient = new MiniClient("小游戏服务器客户端",serverEntry->pstrIP,serverEntry->wdPort,serverEntry->wdServerID);
  if (NULL == miniClient)
  {
    Zebra::logger->error("没有足够内存,不能建立小游戏服务器客户端实例");
    return false;
  }
  if (!miniClient->connectToMiniServer())
  {
    Zebra::logger->error("GatewayService::init 连接小游戏服务器失败");
    return false;
  }
  miniClient->start();

  if (!GateUserManager::getInstance()->init())
    return false;

  GatewayTimeTick::getInstance().start();

  Zebra::logger->debug("初始化成功完成！");
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
void GatewayService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
  Zebra::logger->debug("GatewayService::newTCPTask");
  GatewayTask *tcpTask = new GatewayTask(taskPool,sock,addr);
  if (NULL == tcpTask)
    //内存不足,直接关闭连接
    ::closesocket(sock);
  else if (!taskPool->addVerify(tcpTask))
  {
    //得到了一个正确连接,添加到验证队列中
    SAFE_DELETE(tcpTask);
  }
}

bool GatewayService::notifyLoginServer()
{
  Zebra::logger->debug("GatewayService::notifyLoginServer");
  using namespace Cmd::Super;
  t_GYList_Gateway tCmd;

  tCmd.wdServerID = wdServerID;
  tCmd.wdPort     = wdPort;
  strncpy(tCmd.pstrIP,pstrIP,sizeof(tCmd.pstrIP));
  if (!GatewayService::getInstance().isTerminate())
  {
    tCmd.wdNumOnline = getPoolSize();
    printf("网关目前在线人数:%d\n",tCmd.wdNumOnline);
  }
  else
  {
    tCmd.wdNumOnline = 0;
  }
  tCmd.state = getPoolState();
  tCmd.zoneGameVersion = verify_client_version;

  return sendCmdToSuperServer(&tCmd,sizeof(tCmd));
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
bool GatewayService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  using namespace Cmd::Super;

  if (CMD_GATEWAY == pNullCmd->cmd)
  {
    switch(pNullCmd->para)
    {
      case PARA_GATEWAY_RQGYLIST:
         Zebra::logger->info("PARA_GATEWAY_RQGYLIST");
                 return notifyLoginServer();
	  case PARA_NOTIFYGATE_FINISH:
		  {
				if(!startUpFinish)
					startUpFinish = true;
		  }
		  break;
      case PARA_CHARNAME_GATEWAY:
        {
          t_Charname_Gateway *rev = (t_Charname_Gateway *)pNullCmd;

                    Zebra::logger->info("PARA_CHARNAME_GATEWAY");
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(rev->accid);
          if (pUser
              && pUser->isCreateState()
              && rev->state & ROLEREG_STATE_TEST)
          {
            if (rev->state & ROLEREG_STATE_HAS)
            {
              //创建角色失败,角色名称重复
              pUser->nameRepeat();
              Zebra::logger->warn("角色名重复 GatewayService::msgParse_SuperService");
            }
            else
            {
              if (!recordClient->sendCmd(&pUser->createCharCmd,sizeof(pUser->createCharCmd)))
                return false;
            }
          }

          return true;
        }
        break;
    }
  }

  Zebra::logger->error("GatewayService::msgParse_SuperService(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

/**
 * \brief 结束网络服务器
 *
 * 实现了纯虚函数<code>zService::final</code>
 *
 */
void GatewayService::final()
{
  Zebra::logger->debug("GatewayService::final");
  GatewayTimeTick::getInstance().final();
  GatewayTimeTick::getInstance().join();
  GatewayTimeTick::delInstance();
  GateUserManager::getInstance()->removeAllUser(); 

  if (taskPool)
  {
    taskPool->final();
    SAFE_DELETE(taskPool);
  }
  //必须放再taskPool之后处理,否则会down机
  //SceneClientManager::getInstance().final();
  SceneClientManager::delInstance();
  // */
  if (sessionClient)
  {
    sessionClient->final();
    sessionClient->join();
    SAFE_DELETE(sessionClient);
  }
  if (recordClient)
  {
    recordClient->final();
    recordClient->join();
    SAFE_DELETE(recordClient);
  }
  zSubNetService::final();

  GatewayTaskManager::delInstance();

  LoginSessionManager::delInstance();

  GateUserManager::delInstance();

}

/**
 * \brief 读取配置文件
 *
 */
class GatewayConfile:public zConfile
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
void GatewayService::reloadConfig()
{
  Zebra::logger->debug("GatewayService::reloadConfig");
  GatewayConfile gc;
  gc.parse("GatewayServer");
  if ("true" == Zebra::global["rolereg_verify"])
    GatewayService::getInstance().rolereg_verify = true;
  else
    GatewayService::getInstance().rolereg_verify = false;
  
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
  
  if (!country_info.reload())
  {
    Zebra::logger->error("重新加载国家配置!");
  }

  merge_version = atoi(Zebra::global["merge_version"].c_str());
#ifdef _DEBUG
  Zebra::logger->debug("[合区]: 重新加载合区版本号",merge_version);
#endif  
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
  Zebra::logger=new zLogger("GatewayServer");

  //设置缺省参数
  Zebra::global["countryorder"] = "0";

  //解析配置文件参数
  GatewayConfile gc;
  if (!gc.parse("GatewayServer"))
    return EXIT_FAILURE;

  //设置日志级别
  Zebra::logger->setLevel(Zebra::global["log"]);
  //设置写本地日志文件
  if ("" != Zebra::global["logfilename"]){
    Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
    Zebra::logger->removeConsoleLog();
    }

  //Zebra::logger->debug("%s",Zebra::global["rolereg_verify"].c_str());
  if ("true" == Zebra::global["rolereg_verify"])
    GatewayService::getInstance().rolereg_verify = true;
  else
    GatewayService::getInstance().rolereg_verify = false;
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

  merge_version = atoi(Zebra::global["merge_version"].c_str());

  Zebra_Startup();
  
  GatewayService::getInstance().main();
  //GatewayService::delInstance();

  return EXIT_SUCCESS;
}

