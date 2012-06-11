/**
 * \brief ����,���ڱ����Ҷһ���Ϣ
 *
 * 
 */

#include "BillServer.h"


BillSessionManager *BillSessionManager::instance = NULL;

/**
 * \brief ��ӽ��׼�¼,������֤�Ƿ�����ظ���¼
 *
 * \param tid ��������ҽ�����ˮ��
 * \param cmd �������������
 * \param task ����ý��׵�TASK
 * \return ����Ƿ�ɹ�
 */
bool BillSessionManager::add(BillSession &bs)
{
  bool retval = false;
  std::string key = bs.tid;

  mlock.lock();
  BillSessionHashmap_iterator it = sessionMap.find(key);

  if (it == sessionMap.end())
  {
    //û���ҵ�,��Ҫ�����µļ�¼
    sessionMap.insert(BillSessionHashmap_pair(bs.tid,bs));
    retval = true;
  }
  mlock.unlock();

  return retval;
}

/**
 * \brief ���״�����ɺ�,�ӻỰ��������ɾ����Ӧ��¼
 *
 * \param tid �������к�
 *
 * \return �Ƴ��Ƿ�ɹ�
 */
bool BillSessionManager::remove(const std::string& tid)
{
  bool retval = false;

  mlock.lock();
  BillSessionHashmap_iterator it = sessionMap.find(tid);
  
  if (it != sessionMap.end())
  {
    //�ҵ���
    retval = true;
    sessionMap.erase(it);
  }
  mlock.unlock();

  return retval;
}

/**
 * \brief ����TID��Ӧ�Ľ��׼�¼
 *
 * \param tid �������к�
 *
 * \return ����ʧ��,���صĶ�����TID=0,���򷵻ض�Ӧ�Ķ���
 */

BillSession BillSessionManager::get(const std::string& tid)
{
  BillSession ret;
  
  mlock.lock();
  BillSessionHashmap_iterator it = sessionMap.find(tid);
  
  if (it != sessionMap.end())
  {
    //�ҵ���
    ret = it->second;
    sessionMap.erase(it);
  }
  mlock.unlock();
  return ret;

}

