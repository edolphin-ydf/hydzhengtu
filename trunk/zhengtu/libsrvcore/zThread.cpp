/**
 * \brief ʵ����zThread
 */
#include <zebra/srvEngine.h>
#include <mysql.h>

/**
 * \brief �̺߳���
 *
 * �ں��������������߳������ʵ�ֵĻص�����
 *
 * \param arg �����̵߳Ĳ���
 * \return �����߳̽�����Ϣ
 */
DWORD WINAPI zThread::threadFunc(void *arg)
{
  zThread *thread = (zThread *)arg;

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
bool zThread::start()
{
  DWORD dwThread;

  //�߳��Ѿ���������,ֱ�ӷ���
  if (alive)
  {
    Zebra::logger->warn("�߳� %s �Ѿ���������,���ڳ��������߳�",getThreadName().c_str());
    return true;
  }

  if (NULL == (m_hThread=CreateThread(NULL,0,zThread::threadFunc,this,0,&dwThread))) 
  {
    Zebra::logger->error("�����߳� %s ʧ��",getThreadName().c_str());
    return false;
  }

  //Zebra::logger->debug("�����߳� %s �ɹ�",getThreadName().c_str());

  return true;
}

/**
 * \brief �ȴ�һ���߳̽���
 *
 */
void zThread::join()
{
  //Zebra::logger->debug("zThread::join");
  WaitForSingleObject(m_hThread,INFINITE);
}

/**
 * \brief ���캯��
 *
 */
zThreadGroup::zThreadGroup() : vts(),rwlock()
{
}

/**
 * \brief ��������
 *
 */
zThreadGroup::~zThreadGroup()
{
  joinAll();
}

/**
 * \brief ���һ���̵߳�������
 * \param thread ����ӵ��߳�
 */
void zThreadGroup::add(zThread *thread)
{
  zRWLock_scope_wrlock scope_wrlock(rwlock);
  Container::iterator it = std::find(vts.begin(),vts.end(),thread);
  if (it == vts.end())
    vts.push_back(thread);
}

/**
 * \brief ����index�±��ȡ�߳�
 * \param index �±���
 * \return �߳�
 */
zThread *zThreadGroup::getByIndex(const Container::size_type index)
{
  zRWLock_scope_rdlock scope_rdlock(rwlock);
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
zThread *zThreadGroup::operator[] (const Container::size_type index)
{
  zRWLock_scope_rdlock scope_rdlock(rwlock);
  if (index >= vts.size())
    return NULL;
  else
    return vts[index];
}

/**
 * \brief �ȴ������е������߳̽���
 */
void zThreadGroup::joinAll()
{
  zRWLock_scope_wrlock scope_wrlock(rwlock);
  while(!vts.empty())
  {
    zThread *pThread = vts.back();
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
void zThreadGroup::execAll(Callback &cb)
{
  zRWLock_scope_rdlock scope_rdlock(rwlock);
  for(Container::iterator it = vts.begin(); it != vts.end(); ++it)
  {
    cb.exec(*it);
  }
}

