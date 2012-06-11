/**
 * \brief zebra项目档案服务器,用于创建、储存和读取档案
 *
 */

#include "RecordServer.h"

zDBConnPool *RecordService::dbConnPool = NULL;

RecordService *RecordService::instance = NULL;

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool RecordService::init()
{
  Zebra::logger->debug("RecordService::init");

  dbConnPool = zDBConnPool::newInstance(NULL);
  if (NULL == dbConnPool
      || !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
  {
   MessageBox(NULL,"连接数据库失败","RecordServer",MB_ICONERROR);
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
  //Zebra::logger->debug("%s",pstrIP);

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
void RecordService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
  Zebra::logger->debug("RecordService::newTCPTask");
  RecordTask *tcpTask = new RecordTask(taskPool,sock,addr);
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
bool RecordService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  Zebra::logger->error("RecordService::msgParse_SuperService(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

/**
 * \brief 结束网络服务器
 *
 * 实现了纯虚函数<code>zService::final</code>
 *
 */
void RecordService::final()
{
  Zebra::logger->debug("RecordService::final");
  while(!RecordSessionManager::getInstance().empty())
  {
    zThread::msleep(10);
  }
  if (taskPool)
  {
    taskPool->final();
    SAFE_DELETE(taskPool);
  }
  zSubNetService::final();

  RecordSessionManager::delInstance();

  zDBConnPool::delInstance(&dbConnPool);

  Zebra::logger->debug("RecordService::final");
}

/**
 * \brief 读取配置文件
 *
 */
class RecordConfile:public zConfile
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
void RecordService::reloadConfig()
{
  Zebra::logger->debug("RecordService::reloadConfig");
  RecordConfile rc;
  rc.parse("RecordServer");
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

  fprintf(stderr,"here\n");

  Zebra::logger=new zLogger("RecordServer");

  fprintf(stderr,"here2\n");

  //设置缺省参数

  //解析配置文件参数
  fprintf(stderr,"here3\n");
  RecordConfile rc;
  if (!rc.parse("RecordServer"))
  {
	fprintf(stderr,"here4\n");
    return EXIT_FAILURE;
  }

  fprintf(stderr,"here5\n");

  //指令检测开关
  if (Zebra::global[std::string("cmdswitch")] == "true")
  {
    fprintf(stderr,"here6\n");
    zTCPTask::analysis._switch = true;
    zTCPClient::analysis._switch=true;
  }
  else
  {
    fprintf(stderr,"here7\n");
    zTCPTask::analysis._switch = false;
    zTCPClient::analysis._switch=false;
  }

  fprintf(stderr,"here8\n");

  //设置日志级别
  Zebra::logger->setLevel(Zebra::global["log"]);
  //设置写本地日志文件
  if ("" != Zebra::global["logfilename"]){
    Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
        Zebra::logger->removeConsoleLog();
    }

  fprintf(stderr,"here9\n");

  Zebra_Startup();

  fprintf(stderr,"here10\n");
  
  RecordService::getInstance().main();
  fprintf(stderr,"here11\n");
  RecordService::delInstance();
  fprintf(stderr,"here12\n");

  return EXIT_SUCCESS;
}

int __stdcall WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	return 1;
}

