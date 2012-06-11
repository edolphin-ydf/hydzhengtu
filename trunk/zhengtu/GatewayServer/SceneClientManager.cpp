/**
 * \brief ���ص��������ݻ��巢��
 *
 * 
 */

#include "GatewayServer.h"

/**
 ** \brief ���Ψһʵ��ָ��
 **/
SceneClientManager *SceneClientManager::instance = NULL;

/**
 ** \brief ���캯��
 **/
SceneClientManager::SceneClientManager()
{
  sceneClientPool = NULL;
}

/**
 ** \brief ��������
 **/
SceneClientManager::~SceneClientManager()
{
  SAFE_DELETE(sceneClientPool);
}

/**
 ** \brief ��ʼ��������
 ** \return ��ʼ���Ƿ�ɹ�
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
    SceneClient *sceneClient = new SceneClient("�����������ͻ���",serverEntry);
    if (NULL == sceneClient)
    {
      Zebra::logger->error("û���㹻�ڴ�,���ܽ��������������ͻ���ʵ��");
      return false;
    }
    sceneClientPool->put(sceneClient);
    serverEntry = GatewayService::getInstance().getNextServerEntryByType(SCENESSERVER,&serverEntry);
  }
  return true;
}

/**
 ** \brief ���ڼ���������ӵĶ�����������
 ** \param ct ��ǰʱ��
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
 ** \brief ������������Ѿ��ɹ�������
 ** \param sceneClient ����ӵ�����
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
 ** \brief ���������Ƴ��Ͽ�������
 ** \param sceneClient ���Ƴ�������
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
 ** \brief ��ɹ����������ӹ㲥ָ��
 ** \param pstrCmd ���㲥��ָ��
 ** \param nCmdLen ���㲥ָ��ĳ���
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

