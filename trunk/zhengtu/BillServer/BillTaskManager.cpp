/**
 * \brief ���������ӵ�����
 *
 * 
 */

#include "BillServer.h"

BillTaskManager *BillTaskManager::instance = NULL;

/**
 * \brief ��Ψһ����֤���������һ������������
 *
 * \param task ����������
 * \return ��������Ƿ�ɹ�
 */
bool BillTaskManager::uniqueAdd(BillTask *task)
{
  BillTaskHashmap_const_iterator it;
  rwlock.wrlock();
  it = sessionTaskSet.find(task->getID());
  if (it != sessionTaskSet.end())
  {
    Zebra::logger->error("BillTaskManager::uniqueAdd");
    rwlock.unlock();
    return false;
  }
  sessionTaskSet.insert(BillTaskHashmap_pair(task->getID(),task));
  rwlock.unlock();
  return true;
}

/**
 * \brief ��Ψһ���������Ƴ�һ������������
 *
 * \param task ����������
 * \return �Ƴ��Ƿ�ɹ�
 */
bool BillTaskManager::uniqueRemove(BillTask *task)
{
  BillTaskHashmap_iterator it;
  rwlock.wrlock();
  it = sessionTaskSet.find(task->getID());
  if (it != sessionTaskSet.end())
  {
    sessionTaskSet.erase(it);
  }
  else
    Zebra::logger->warn("BillTaskManager::uniqueRemove");
  rwlock.unlock();
  return true;
}

/**
 * \brief ���ݷ�������Ź㲥ָ��
 *
 * \param wdServerID ���㲥ָ��ķ��������
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool BillTaskManager::broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen)
{
  bool retval = true;
  BillTaskHashmap_iterator it;
  rwlock.rdlock();
  it = sessionTaskSet.find(wdServerID);
  if (it != sessionTaskSet.end())
  {
    retval = it->second->sendCmd(pstrCmd,nCmdLen);
  }
  rwlock.unlock();
  return retval;
}

/**
 * \brief ���ݷ�������Ų���task
 *
 * \param wdServerID �����ҵķ��������
 * \return �㲥�Ƿ�ɹ�
 */
BillTask *BillTaskManager::getTaskByID(const WORD wdServerID)
{
  BillTask *ret=NULL;
  BillTaskHashmap_iterator it;
  rwlock.rdlock();
  it = sessionTaskSet.find(wdServerID);
  if (it != sessionTaskSet.end())
  {
    ret = it->second;
  }
  rwlock.unlock();
  return ret;
}
void BillTaskManager::execEvery()
{
  BillTaskHashmap_iterator it;
  BillTask *task=NULL;
  rwlock.rdlock();
  it = sessionTaskSet.begin();
  for (; it != sessionTaskSet.end() ; it ++)
  {
    task=it->second;
    if (!task->checkRecycle())
    {
      task->doCmd();
    }
  }
  rwlock.unlock();

}
