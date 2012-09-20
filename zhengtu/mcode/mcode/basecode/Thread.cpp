#include "Thread.h"
#include <queue>
/**
 * \brief 线程函数
 *
 * 在函数体里面会调用线程类对象实现的回调函数
 *
 * \param arg 传入线程的参数
 * \return 返回线程结束信息
 */
DWORD WINAPI CThread::threadFunc(void *arg)
{
  CThread *thread = (CThread *)arg;

  thread->mlock.lock();
  thread->alive = true;
  thread->mlock.unlock();

  //mysql_thread_init();

  //运行线程的主回调函数
  thread->run();

  //mysql_thread_end();

  thread->mlock.lock();
  thread->alive = false;
  thread->mlock.unlock();

  //如果不是joinable,需要回收线程资源
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
 * \brief 创建线程,启动线程
 *
 * \return 创建线程是否成功
 */
bool CThread::start()
{
  DWORD dwThread;

  //线程已经创建运行,直接返回
  if (alive)
  {
    printf("线程 %s 已经创建运行,还在尝试运行线程\n",getThreadName().c_str());
    return true;
  }

  if (NULL == (m_hThread=CreateThread(NULL,0,CThread::threadFunc,this,0,&dwThread))) 
  {
    printf("创建线程 %s 失败\n",getThreadName().c_str());
    return false;
  }

  //Zebra::logger->debug("创建线程 %s 成功",getThreadName().c_str());

  return true;
}

/**
 * \brief 等待一个线程结束
 *
 */
void CThread::join()
{
  //Zebra::logger->debug("zThread::join");
  WaitForSingleObject(m_hThread,INFINITE);
}

/**
 * \brief 构造函数
 *
 */
CThreadGroup::CThreadGroup() : vts(),rwlock()
{
}

/**
 * \brief 析构函数
 *
 */
CThreadGroup::~CThreadGroup()
{
  joinAll();
}

/**
 * \brief 添加一个线程到分组中
 * \param thread 待添加的线程
 */
void CThreadGroup::add(CThread *thread)
{
  RWLock_scope_wrlock scope_wrlock(rwlock);
  Container::iterator it = std::find(vts.begin(),vts.end(),thread);
  if (it == vts.end())
    vts.push_back(thread);
}

/**
 * \brief 按照index下标获取线程
 * \param index 下标编号
 * \return 线程
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
 * \brief 重载[]运算符,按照index下标获取线程
 * \param index 下标编号
 * \return 线程
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
 * \brief 等待分组中的所有线程结束
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
 * \brief 对容器中的所有元素调用回调函数
 * \param cb 回调函数实例
 */
void CThreadGroup::execAll(Callback &cb)
{
  RWLock_scope_rdlock scope_rdlock(rwlock);
  for(Container::iterator it = vts.begin(); it != vts.end(); ++it)
  {
    cb.exec(*it);
  }
}

