#include "Thread.h"
#include <queue>
/**
 * \brief �̺߳���
 *
 * �ں��������������߳������ʵ�ֵĻص�����
 *
 * \param arg �����̵߳Ĳ���
 * \return �����߳̽�����Ϣ
 */
DWORD WINAPI CThread::threadFunc(void *arg)
{
  CThread *thread = (CThread *)arg;

  thread->mlock.lock();
  thread->alive = true;
  thread->mlock.unlock();

  //mysql_thread_init();

  //�����̵߳����ص�����
  thread->run();

  //mysql_thread_end();

  thread->mlock.lock();
  thread->alive = false;
  thread->mlock.unlock();

  //�������joinable,��Ҫ�����߳���Դ
  if (!thread->isJoinable())
  {
    SAFE_DELETE(thread);
  }
  else
  {
    CloseHandle(thread->m_hThread);
    thread->m_hThread = NULL;
  }

  return 0;
}

/**
 * \brief �����߳�,�����߳�
 *
 * \return �����߳��Ƿ�ɹ�
 */
bool CThread::start()
{
  DWORD dwThread;

  //�߳��Ѿ���������,ֱ�ӷ���
  if (alive)
  {
    printf("�߳� %s �Ѿ���������,���ڳ��������߳�\n",getThreadName().c_str());
    return true;
  }

  if (NULL == (m_hThread=CreateThread(NULL,0,CThread::threadFunc,this,0,&dwThread))) 
  {
    printf("�����߳� %s ʧ��\n",getThreadName().c_str());
    return false;
  }

  //Zebra::logger->debug("�����߳� %s �ɹ�",getThreadName().c_str());

  return true;
}

/**
 * \brief �ȴ�һ���߳̽���
 *
 */
void CThread::join()
{
  //Zebra::logger->debug("zThread::join");
  WaitForSingleObject(m_hThread,INFINITE);
}

/**
 * \brief ���캯��
 *
 */
CThreadGroup::CThreadGroup() : vts(),rwlock()
{
}

/**
 * \brief ��������
 *
 */
CThreadGroup::~CThreadGroup()
{
  joinAll();
}

/**
 * \brief ���һ���̵߳�������
 * \param thread ����ӵ��߳�
 */
void CThreadGroup::add(CThread *thread)
{
  RWLock_scope_wrlock scope_wrlock(rwlock);
  Container::iterator it = std::find(vts.begin(),vts.end(),thread);
  if (it == vts.end())
    vts.push_back(thread);
}

/**
 * \brief ����index�±��ȡ�߳�
 * \param index �±���
 * \return �߳�
 */
CThread *CThreadGroup::getByIndex(const Container::size_type index)
{
  RWLock_scope_rdlock scope_rdlock(rwlock);
  if (index >= vts.size())
    return NULL;
  else
    return vts[index];
}

/**
 * \brief ����[]�����,����index�±��ȡ�߳�
 * \param index �±���
 * \return �߳�
 */
CThread *CThreadGroup::operator[] (const Container::size_type index)
{
  RWLock_scope_rdlock scope_rdlock(rwlock);
  if (index >= vts.size())
    return NULL;
  else
    return vts[index];
}

/**
 * \brief �ȴ������е������߳̽���
 */
void CThreadGroup::joinAll()
{
  RWLock_scope_wrlock scope_wrlock(rwlock);
  while(!vts.empty())
  {
    CThread *pThread = vts.back();
    vts.pop_back();
    if (pThread)
    {
      pThread->final();
      pThread->join();
      SAFE_DELETE(pThread);
    }
  }
}

/**
 * \brief �������е�����Ԫ�ص��ûص�����
 * \param cb �ص�����ʵ��
 */
void CThreadGroup::execAll(Callback &cb)
{
  RWLock_scope_rdlock scope_rdlock(rwlock);
  for(Container::iterator it = vts.begin(); it != vts.end(); ++it)
  {
    cb.exec(*it);
  }
}

