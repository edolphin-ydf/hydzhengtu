/**
 * \brief 网关到场景数据缓冲发送
 *
 * 
 */

#include "GatewayServer.h"

/**
 ** \brief 类的唯一实例指针
 **/
SceneClientManager *SceneClientManager::instance = NULL;

/**
 ** \brief 构造函数
 **/
SceneClientManager::SceneClientManager()
{
  sceneClientPool = NULL;
}

/**
 ** \brief 析构函数
 **/
SceneClientManager::~SceneClientManager()
{
  SAFE_DELETE(sceneClientPool);
}

/**
 ** \brief 初始化管理器
 ** \return 初始化是否成功
 **/
bool SceneClientManager::init()
{
  Zebra::logger->debug("SceneClientManager::init");
  const Cmd::Super::ServerEntry *serverEntry = GatewayService::getInstance().getServerEntryByType(SCENESSERVER);
  sceneClientPool = new zTCPClientTaskPool(atoi(Zebra::global["threadPoolClient"].c_str()),8000);
  if (NULL == sceneClientPool
      || !sceneClientPool->init())
    return false;

  while(serverEntry)
  {
    SceneClient *sceneClient = new SceneClient("场景服务器客户端",serverEntry);
    if (NULL == sceneClient)
    {
      Zebra::logger->error("没有足够内存,不能建立场景服务器客户端实例");
      return false;
    }
    sceneClientPool->put(sceneClient);
    serverEntry = GatewayService::getInstance().getNextServerEntryByType(SCENESSERVER,&serverEntry);
  }
  return true;
}

/**
 ** \brief 周期间隔进行连接的断线重连工作
 ** \param ct 当前时间
 **/
void SceneClientManager::timeAction(const zTime &ct)
{
  if (actionTimer.elapse(ct) > 4)
  {
    if (sceneClientPool)
      sceneClientPool->timeAction(ct);
    actionTimer = ct;
  }
}

/**
 ** \brief 向容器中添加已经成功的连接
 ** \param sceneClient 待添加的连接
 **/
void SceneClientManager::add(SceneClient *sceneClient)
{
  if (sceneClient)
  {
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    allClients.insert(value_type(sceneClient->getServerID(),sceneClient));
  }
}

/**
 ** \brief 从容器中移除断开的连接
 ** \param sceneClient 待移除的连接
 **/
void SceneClientManager::remove(SceneClient *sceneClient)
{
  if (sceneClient)
  {
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    iter it = allClients.find(sceneClient->getServerID());
    if (it != allClients.end())
    {
      allClients.erase(it);
    }
  }
}

/**
 ** \brief 向成功的所有连接广播指令
 ** \param pstrCmd 待广播的指令
 ** \param nCmdLen 待广播指令的长度
 **/
bool SceneClientManager::broadcastOne(const void *pstrCmd,int nCmdLen)
{
  return false;
}

bool SceneClientManager::sendTo(const DWORD tempid,const void *pstrCmd,int nCmdLen)
{
  zRWLock_scope_rdlock scope_rdlock(rwlock);
  iter it = allClients.find(tempid);
  if (it == allClients.end())
    return false;
  else
    return it->second->sendCmd(pstrCmd,nCmdLen);
}

