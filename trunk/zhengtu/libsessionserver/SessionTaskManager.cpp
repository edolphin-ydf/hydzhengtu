/**
 * \brief ���������ӵ�����
 *
 * 
 */

#include <zebra/SessionServer.h>

SessionTaskManager *SessionTaskManager::instance = NULL;

/**
 * \brief ���һ�������ӵ�������
 *
 * ��������������أ���Ҫ��ӵ���������������
 * ����������ǳ�������������Ҫ��ӵ���������������
 *
 * \param task ����������
 */
void SessionTaskManager::addSessionTask(SessionTask *task)
{
  rwlock.wrlock();
  if (GATEWAYSERVER == task->getType())
  {
    gatewayTaskList.push_back(task);
  }
  else if (SCENESSERVER == task->getType())
  {
    sceneTaskList.push_back(task);
  }
  rwlock.unlock();
}

/**
 * \brief ���������Ƴ�����������
 *
 * ��������������أ���Ҫ�������������Ƴ�
 * ����������ǳ�������������Ҫ�ӳ��������������Ƴ�
 *
 * \param task ����������
 */
void SessionTaskManager::removeSessionTask(SessionTask *task)
{
  rwlock.wrlock();
  if (GATEWAYSERVER == task->getType())
  {
    gatewayTaskList.remove(task);
  }
  else if (SCENESSERVER == task->getType())
  {
    sceneTaskList.remove(task);
  }
  rwlock.unlock();
}

/**
 * \brief ��Ψһ����֤���������һ������������
 *
 * \param task ����������
 * \return ��������Ƿ�ɹ�
 */
bool SessionTaskManager::uniqueAdd(SessionTask *task)
{
  SessionTaskHashmap_const_iterator it;
  rwlock.wrlock();
  it = sessionTaskSet.find(task->getID());
  if (it != sessionTaskSet.end())
  {
    Zebra::logger->error("SessionTaskManager::uniqueAdd");
    rwlock.unlock();
    return false;
  }
  sessionTaskSet.insert(SessionTaskHashmap_pair(task->getID(),task));
  rwlock.unlock();
  return true;
}

/**
 * \brief ��Ψһ���������Ƴ�һ������������
 *
 * \param task ����������
 * \return �Ƴ��Ƿ�ɹ�
 */
bool SessionTaskManager::uniqueRemove(SessionTask *task)
{
  SessionTaskHashmap_iterator it;
  rwlock.wrlock();
  it = sessionTaskSet.find(task->getID());
  if (it != sessionTaskSet.end())
  {
    sessionTaskSet.erase(it);
  }
  else
    Zebra::logger->warn("SessionTaskManager::uniqueRemove");
  rwlock.unlock();
  return true;
}

/**
 * \brief �����е��������ӹ㲥ָ��
 *
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool SessionTaskManager::broadcastGateway(const void *pstrCmd,int nCmdLen)
{
  bool retval = true;
  TaskContainer_iterator it;
  rwlock.rdlock();
  for(it = gatewayTaskList.begin(); it != gatewayTaskList.end(); it++)
  {
    retval = (retval && (*it)->sendCmd(pstrCmd,nCmdLen));
  }
  rwlock.unlock();
  return retval;
}
/**
 * \brief �����е����ط�������ָ��
 *
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool SessionTaskManager::sendCmdToWorld(const void *pstrCmd,int nCmdLen)
{
  using namespace Cmd::Session;
  char buf[zSocket::MAX_DATASIZE];
  t_Session_ForwardWorld * sfw =(t_Session_ForwardWorld*)buf; 
  constructInPlace(sfw);
  sfw->size=nCmdLen;
  bcopy(pstrCmd,sfw->data,nCmdLen,sizeof(buf) - sizeof(t_Session_ForwardWorld));
  return broadcastGateway(sfw,sizeof(t_Session_ForwardWorld) + sfw->size);
}
/**
 * \brief �����е����ط��͹���ָ��
 *
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool SessionTaskManager::sendCmdToCountry(DWORD country,const void *pstrCmd,int nCmdLen)
{
  using namespace Cmd::Session;
  char buf[zSocket::MAX_DATASIZE];
  t_Session_ForwardCountry * sfc =(t_Session_ForwardCountry*)buf; 
  constructInPlace(sfc);
  sfc->size=nCmdLen;
  sfc->dwCountry=country;
  bcopy(pstrCmd,sfc->data,nCmdLen,sizeof(buf) - sizeof(t_Session_ForwardCountry));
  return broadcastGateway(sfc,sizeof(t_Session_ForwardCountry) + sfc->size);
}

/**
 * \brief �����еĳ������������ӹ㲥ָ��
 *
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ָ���
 * \return �㲥�Ƿ�ɹ�
 */
bool SessionTaskManager::broadcastScene(const void *pstrCmd,int nCmdLen)
{
  bool retval = true;
  TaskContainer_iterator it;
  rwlock.rdlock();
  for(it = sceneTaskList.begin(); it != sceneTaskList.end(); it++)
  {
    retval = (retval && (*it)->sendCmd(pstrCmd,nCmdLen));
  }
  rwlock.unlock();
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
bool SessionTaskManager::broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen)
{
  bool retval = true;
  SessionTaskHashmap_iterator it;
  rwlock.rdlock();
  it = sessionTaskSet.find(wdServerID);
  if (it != sessionTaskSet.end())
  {
    retval = it->second->sendCmd(pstrCmd,nCmdLen);
  }
  rwlock.unlock();
  return retval;
}

void SessionTaskManager::execEvery()
{
  SessionTaskHashmap_iterator it;
  SessionTask *task=NULL;
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
