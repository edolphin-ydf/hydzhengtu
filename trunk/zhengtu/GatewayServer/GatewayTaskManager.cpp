/**
 * \brief ���������ӵ�����
 *
 * 
 */


#include "GatewayServer.h"

///�������ӹ�����ʵ��
GatewayTaskManager *GatewayTaskManager::instance = NULL;

GatewayTaskManager::GatewayTaskManager()
{
}

GatewayTaskManager::~GatewayTaskManager()
{
}

/**
 * \brief �����������һ��������
 *
 * \param task ����������
 * \return ����Ƿ�ɹ�
 */
bool GatewayTaskManager::uniqueAdd(GatewayTask *task)
{
  GatewayTaskHashmap_const_iterator it;
  rwlock.wrlock();
  it = gatewayTaskSet.find(task->getACCID());
  if (it != gatewayTaskSet.end())
  {
    rwlock.unlock();
    return false;
  }
  gatewayTaskSet.insert(GatewayTaskHashmap_pair(task->getACCID(),task));
  rwlock.unlock();
  return true;
}

/**
 * \brief ��������ɾ��һ��������
 *
 * \param task ����������
 * \return ɾ���Ƿ�ɹ�
 */
bool GatewayTaskManager::uniqueRemove(GatewayTask *task)
{
  GatewayTaskHashmap_iterator it;
  rwlock.wrlock();
  it = gatewayTaskSet.find(task->getACCID());
  if (it != gatewayTaskSet.end())
  {
    gatewayTaskSet.erase(it);
  }
  else
    Zebra::logger->warn("GatewayTaskManager::uniqueRemove");
  rwlock.unlock();
  return true;
}

/**
 * \brief ����ĳһ�����ӼƷ���֤�Ƿ�ͨ��
 * \param accid �˺�
 * \param ok �Ʒ���֢�Ƿ�ͨ��
 */
void GatewayTaskManager::accountVerifyOK(const DWORD accid,const bool ok)
{
  GatewayTaskHashmap_iterator it;
  rwlock.rdlock();
  it = gatewayTaskSet.find(accid);
  if (it != gatewayTaskSet.end())
  {
    it->second->accountVerifyOK(ok);
  }
  rwlock.unlock();
}

/**
 * \brief ���������е�����Ԫ�أ�ִ��ĳһ������
 * \param callback ��ִ�еĻص�����
 */
void GatewayTaskManager::execAll(GatewayTaskCallback &callback)
{
  rwlock.rdlock();
  for(GatewayTaskHashmap_iterator it = gatewayTaskSet.begin(); it != gatewayTaskSet.end(); ++it)
  {
    if (!callback.exec(it->second))
      break;
  }
  rwlock.unlock();
}

