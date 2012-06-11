/**
 * \brief ����,���ڱ�֤��������ظ���½
 *
 * 
 */

#include "RecordServer.h"

RecordSessionManager *RecordSessionManager::instance = NULL;

/**
 * \brief ��ȡ������Ϣ��ʱ��,��Ҫ��Ӽ�¼,������֤�Ƿ�����ظ���¼
 *
 * \param accid �ʺ�
 * \param id ��ɫ���
 * \param wdServerID ���������
 * \return ����Ƿ�ɹ�
 */
bool RecordSessionManager::add(const DWORD accid,const DWORD id,const WORD wdServerID)
{
  Zebra::logger->debug("RecordSessionManager::add");
  bool retval = false;

  mlock.lock();
  RecordSessionHashmap_iterator it = sessionMap.find(accid);
  if (it == sessionMap.end())
  {
    //û���ҵ�,��Ҫ�����µļ�¼
    RecordSession session(accid,id,wdServerID);
    sessionMap.insert(RecordSessionHashmap_pair(accid,session));
    retval = true;
  }
  mlock.unlock();

  return retval;
}

/**
 * \brief ��д������Ҫ��֤�Ự��Ϣ�Ƿ����
 *
 * \param accid �ʺ�
 * \param id ��ɫ���
 * \param wdServerID ���������
 * \return ��֤�Ƿ�ɹ�
 */
bool RecordSessionManager::verify(const DWORD accid,const DWORD id,const WORD wdServerID)
{
  Zebra::logger->debug("RecordSessionManager::verify");
  bool retval = false;

  mlock.lock();
  RecordSessionHashmap_iterator it = sessionMap.find(accid);
  if (it != sessionMap.end()
      && it->second.accid == accid
      && it->second.id == id
      && it->second.wdServerID == wdServerID)
  {
    //�ҵ���
    retval = true;
    it->second.lastsavetime.now();
  }
  mlock.unlock();

  return retval;
}

/**
 * \brief ��ɫ�˳�ʱ��,��д��������Ժ���Ҫ�Ƴ��Ự��¼
 *
 * \param accid �ʺ�
 * \param id ��ɫ���
 * \param wdServerID ���������
 * \return �Ƴ��Ƿ�ɹ�
 */
bool RecordSessionManager::remove(const DWORD accid,const DWORD id,const WORD wdServerID)
{
  Zebra::logger->debug("RecordSessionManager::remove");
  bool retval = false;

  mlock.lock();
  RecordSessionHashmap_iterator it = sessionMap.find(accid);
  if (it != sessionMap.end()
      && it->second.accid == accid
      && it->second.id == id
      && it->second.wdServerID == wdServerID)
  {
    //�ҵ���
    retval = true;
    sessionMap.erase(it);
  }
  mlock.unlock();

  return retval;
}

/**
 * \brief �����������رյ�ʱ��,��Ҫ�����е��������������صĻỰ��¼���
 *
 * \param wdServerID ���������
 */
void RecordSessionManager::removeAllByServerID(const WORD wdServerID)
{
  Zebra::logger->debug("RecordSessionManager::removeAllByServerID");
  mlock.lock();
  if (!sessionMap.empty())
  {
    for(RecordSessionHashmap_iterator it = sessionMap.begin(); it != sessionMap.end();)
    {
      if (it->second.wdServerID == wdServerID)
      {
        RecordSessionHashmap_iterator tmp = it;
        it++;
        sessionMap.erase(tmp);
      }
      else
        it++;
    }
  }
  mlock.unlock();
}

