/**
 * \brief ʵ�ַ�������������
 *
 * �����������ȫ��������Ψһ����֤����
 * 
 */

#include "SuperServer.h"

ServerManager *ServerManager::instance = NULL;

/**
 * \brief ���һ����������������������
 *
 * \param task ��������������
 */
void ServerManager::addServer(ServerTask *task)
{
  Zebra::logger->debug("ServerManager::addServer()");
  if (task)
  {
    mutex.lock();    
    container.push_front(task);
    mutex.unlock();
  }
}

/**
 * \brief ��������ɾ��һ����������������
 *
 * \param task ��������������
 */
void ServerManager::removeServer(ServerTask *task)
{
  Zebra::logger->debug("ServerManager::removeServer()");
  if (task)
  {
    mutex.lock();    
    container.remove(task);
    mutex.unlock();
  }
}

/**
 * \brief ���ݱ�Ų���һ����������������
 *
 * \param wdServerID ���������
 * \return �����ʺϵķ�������������
 */
ServerTask *ServerManager::getServer(WORD wdServerID)
{
  Zebra::logger->info("ServerManager::getServer(serverid=%u)",wdServerID);
  Containter_const_iterator it;
  ServerTask *retval = NULL;

  mutex.lock();
  for(it = container.begin(); it != container.end(); it++)
  {
    if ((*it)->getID() == wdServerID)
    {
      retval = *it;
      break;
    }
  }
  mutex.unlock();

  return retval;
}

/**
 * \brief ��һ������������������ӵ�Ψһ��������
 *
 * \param task ��������������
 * \return ����Ƿ�ɹ���Ҳ����Ψһ����֤�Ƿ�ɹ�
 */
bool ServerManager::uniqueAdd(ServerTask *task)
{
  Zebra::logger->info("ServerManager::uniqueVerify id=%u",task->getID());
  ServerTaskHashmap_const_iterator it;
  mutex.lock();
  it = taskUniqueContainer.find(task->getID());
  if (it != taskUniqueContainer.end())
  {
    Zebra::logger->error("ServerManager::uniqueAdd");
    mutex.unlock();
    return false;
  }
  taskUniqueContainer.insert(ServerTaskHashmap_pair(task->getID(),task));
  mutex.unlock();
  return true;
}

/**
 * \brief ��֤����������Ƿ��Ѿ�����
 *
 * \param wdServerID ���������
 * \return ��֤�Ƿ�ɹ�
 */
bool ServerManager::uniqueVerify(const WORD wdServerID)
{
  Zebra::logger->info("ServerManager::uniqueVerify wdServerID=%u",wdServerID);
  ServerTaskHashmap_const_iterator it;
  mutex.lock();
  it = taskUniqueContainer.find(wdServerID);
  if (it != taskUniqueContainer.end())
  {
    mutex.unlock();
    return false;
  }
  mutex.unlock();
  return true;
}

/**
 * \brief ��Ψһ��������ɾ��һ����������
 *
 * \param task ��������������
 * \return ɾ���Ƿ�ɹ�
 */
bool ServerManager::uniqueRemove(ServerTask *task)
{
  Zebra::logger->info("ServerManager::uniqueRemove id=%u",task->getID());
  ServerTaskHashmap_iterator it;
  mutex.lock();
  it = taskUniqueContainer.find(task->getID());
  if (it != taskUniqueContainer.end())
  {
    taskUniqueContainer.erase(it);
  }
  else
    Zebra::logger->warn("ServerManager::uniqueRemove");
  mutex.unlock();
  return true;
}

/**
 * \brief �����������еķ������㲥ָ��
 *
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool ServerManager::broadcast(const void *pstrCmd,int nCmdLen)
{
  bool retval = true;

  mutex.lock();
  for(Containter_const_iterator it = container.begin(); it != container.end(); it++)
  {
    if (!(*it)->sendCmd(pstrCmd,nCmdLen))
      retval = false;
  }
  mutex.unlock();

  return retval;
}

/**
 * \brief ���ݷ�������Ź㲥ָ��
 *
 * \param wdServerID ���㲥ָ��ķ��������
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool ServerManager::broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen)
{
  Zebra::logger->info("ServerManager::broadcastByID(wdServerID=%u)",wdServerID);
  bool retval = false;

  mutex.lock();
  for(Containter_const_iterator it = container.begin(); it != container.end(); it++)
  {
    if ((*it)->getID() == wdServerID)
    {
      retval = (*it)->sendCmd(pstrCmd,nCmdLen);
      break;
    }
  }
  mutex.unlock();

  return retval;
}

/**
 * \brief ���ݷ��������͹㲥ָ��
 *
 * \param wdType ���㲥ָ��ķ���������
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool ServerManager::broadcastByType(const WORD wdType,const void *pstrCmd,int nCmdLen)
{
  bool retval = true;

  mutex.lock();
  for(Containter_const_iterator it = container.begin(); it != container.end(); it++)
  {    
    if ((*it)->getType() == wdType
        && !(*it)->sendCmd(pstrCmd,nCmdLen))
      retval = false;
  }
  mutex.unlock();

  return retval;
}

/**
 * \brief ͳ��һ��������������
 * \return �õ�һ�����ĵ�ǰ����������
 */
const DWORD ServerManager::caculateOnlineNum()
{
  DWORD retval = 0;

  mutex.lock();
  for(Containter_const_iterator it = container.begin(); it != container.end(); it++)
  {
    if ((*it)->getType() == GATEWAYSERVER)
      retval += (*it)->getOnlineNum();
  }
  mutex.unlock();

  return retval;
}

/**
 * \brief �յ�notifyOther�ظ�
 * \param srcID Դ���������
 * \param wdServerID Ŀ�ķ��������
 */
void ServerManager::responseOther(const WORD srcID,const WORD wdServerID)
{
  Zebra::logger->info("ServerManager::responseOther(srcid=%u,wdServerID=%u)",srcID,wdServerID);
  ServerTaskHashmap_const_iterator it;
  mutex.lock();
  it = taskUniqueContainer.find(srcID);//taskUniqueContainer.find(srcID);
  if (it != taskUniqueContainer.end())
  {
    if (it->second)
      it->second->responseOther(wdServerID);
  }else
  {
    Zebra::logger->info("ServerManager::responseOther find srcid=%u",srcID);
  }
  mutex.unlock();
}

