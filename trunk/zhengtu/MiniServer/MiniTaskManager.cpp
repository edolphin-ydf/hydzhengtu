/**
 * \brief ���������ӵ�����
 *
 * 
 */

#include "MiniServer.h"

MiniTaskManager *MiniTaskManager::instance = NULL;

/**
 * \brief ��Ψһ����֤���������һ������������
 *
 * \param task ����������
 * \return ��������Ƿ�ɹ�
 */
bool MiniTaskManager::uniqueAdd(MiniTask *task)
{
  MiniTaskHashmap_const_iterator it;
  rwlock.wrlock();
  it = sessionTaskSet.find(task->getID());
  if (it != sessionTaskSet.end())
  {
    Zebra::logger->error("MiniTaskManager::uniqueAdd");
    rwlock.unlock();
    return false;
  }
  sessionTaskSet.insert(MiniTaskHashmap_pair(task->getID(),task));
  rwlock.unlock();
  return true;
}

/**
 * \brief ��Ψһ���������Ƴ�һ������������
 *
 * \param task ����������
 * \return �Ƴ��Ƿ�ɹ�
 */
bool MiniTaskManager::uniqueRemove(MiniTask *task)
{
  MiniTaskHashmap_iterator it;
  rwlock.wrlock();
  it = sessionTaskSet.find(task->getID());
  if (it != sessionTaskSet.end())
  {
    sessionTaskSet.erase(it);
  }
  else
    Zebra::logger->warn("MiniTaskManager::uniqueRemove");
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
bool MiniTaskManager::broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen)
{
  bool retval = true;
  MiniTaskHashmap_iterator it;
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
MiniTask *MiniTaskManager::getTaskByID(const WORD wdServerID)
{
  MiniTask *ret=NULL;
  MiniTaskHashmap_iterator it;
  rwlock.rdlock();
  it = sessionTaskSet.find(wdServerID);
  if (it != sessionTaskSet.end())
  {
    ret = it->second;
  }
  rwlock.unlock();
  return ret;
}

void MiniTaskManager::execEvery()
{
  MiniTaskHashmap_iterator it;
  MiniTask *task=NULL;
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
