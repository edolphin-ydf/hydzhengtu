/**
 * \brief zebra项目计费服务器
 *
 */

#include "MiniServer.h"

zDBConnPool *MiniService::dbConnPool = NULL;
MiniService *MiniService::instance = NULL;
DBMetaData* MiniService::metaData = NULL;

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool MiniService::init()
{
  dbConnPool = zDBConnPool::newInstance(NULL);
  if (NULL == dbConnPool
      || !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
  {
    MessageBox(NULL,"连接数据库失败","MiniServer",MB_ICONERROR);
    return false;
  }

  metaData = DBMetaData::newInstance("");

  if (NULL == metaData
      || !metaData->init(Zebra::global["mysql"]))
  {
    MessageBox(NULL,"连接数据库失败","MiniServer",MB_ICONERROR);
    return false;
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

  MiniTimeTick::getInstance().start();
  if (!MiniHall::getMe().init()) return false;

  if (!zSubNetService::init())
    return false;

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
void MiniService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
  MiniTask *tcpTask = new MiniTask(taskPool,sock,addr);
  if (NULL == tcpTask)
    //内存不足，直接关闭连接
    ::closesocket(sock);
  else if (!taskPool->addVerify(tcpTask))
  {
    //得到了一个正确连接，添加到验证队列中
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
bool MiniService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  using namespace Cmd::Super;

  return true;
}

/**
 * \brief 结束网络服务器
 *
 * 实现了纯虚函数<code>zService::final</code>
 *
 */
void MiniService::final()
{
  MiniTimeTick::getInstance().final();
  MiniTimeTick::getInstance().join();
  MiniTimeTick::delInstance();

  if (taskPool)
  {
    SAFE_DELETE(taskPool);
  }

  MiniTaskManager::delInstance();
  MiniUserManager::delInstance();

  zSubNetService::final();

  zDBConnPool::delInstance(&dbConnPool);
  Zebra::logger->debug("MiniService::final");
}

/**
 * \brief 读取配置文件
 *
 */
class MiniConfile:public zConfile
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
 * \brief 重新读取配置文件，为HUP信号的处理函数
 *
 */
void MiniService::reloadConfig()
{
  Zebra::logger->debug("MiniService::reloadConfig");
  MiniConfile rc;
  rc.parse("MiniServer");
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
  Zebra::logger=new zLogger("MiniServer");

  //设置缺省参数

  //解析配置文件参数
  MiniConfile rc;
  if (!rc.parse("MiniServer"))
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
  
  MiniService::getInstance().main();
  MiniService::delInstance();

  return EXIT_SUCCESS;
}
