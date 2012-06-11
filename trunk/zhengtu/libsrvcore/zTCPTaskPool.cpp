/**
* \brief 实现线程池类,用于处理多连接服务器
*/
#include <zebra/srvEngine.h>

#include <assert.h>
//#include <ext/pool_allocator.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

zTCPTask* g_DeleteLog = NULL;

zMutex g_DeleteLock;

int zTCPTaskPool::usleep_time=50000;                    /**< 循环等待时间 */
/**
* \brief 连接任务链表
*
*/
//typedef std::list<zTCPTask *,__gnu_cxx::__pool_alloc<zTCPTask *> > zTCPTaskContainer;
typedef std::vector<zTCPTask *> zTCPTaskContainer;

/**
* \brief 连接任务链表叠代器
*
*/
typedef zTCPTaskContainer::iterator zTCPTask_IT;

typedef std::vector<struct pollfd> pollfdContainer;

class zTCPTaskQueue
{
public:
	zTCPTaskQueue() :_size(0) {}
	virtual ~zTCPTaskQueue() {}
	inline void add(zTCPTask *task)
	{
		mlock.lock();
		_queue.push_back(task);
		_size++;
		mlock.unlock();
	}
	inline void check_queue()
	{
		mlock.lock();
		while(!_queue.empty())
		{
			zTCPTask *task = _queue.back();
			_queue.pop_back();
			_add(task);
		}
		_size = 0;
		mlock.unlock();
	}
protected:
	virtual void _add(zTCPTask *task) = 0;
	DWORD _size;
private:
	zMutex mlock;
	//std::queue<zTCPTask *,std::deque<zTCPTask *,__gnu_cxx::__pool_alloc<zTCPTask *> > > _queue;
	std::vector<zTCPTask *> _queue;
};

/**
* \brief 处理TCP连接的验证,如果验证不通过,需要回收这个连接
*
*/
class zVerifyThread : public zThread,public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< 任务列表 */
	zTCPTaskContainer::size_type task_count;      /**< tasks计数(保证线程安全*/
//	pollfdContainer pfds;

	zMutex m_Lock;
	/**
	* \brief 添加一个连接任务
	* \param task 连接任务
	*/
	void _add(zTCPTask *task)
	{
		Zebra::logger->debug("zVerifyThread::_add");
		m_Lock.lock();
		//struct pollfd pfd;
		//task->fillPollFD(pfd,POLLIN | POLLPRI);
		tasks.push_back(task);
		task_count = tasks.size();
	//	pfds.push_back(pfd);
		m_Lock.unlock();
		Zebra::logger->error("zVerifyThread::_add_end");
	}


	void remove(zTCPTask_IT &it,int p)
	{
		//int i;
		//pollfdContainer::iterator iter;
		//bool bDeleted = false;
		//m_Lock.lock();
		//for(iter = pfds.begin(),i = 0; iter != pfds.end(); iter++,i++)
		//{
		//	if (i == p)
		//	{
		//		pfds.erase(iter);
		//		it = tasks.erase(it);
		//		task_count = tasks.size();
		//		g_CsizeV = task_count;
		//		bDeleted = true;
		//		break;
		//	}
		//}
		//m_Lock.unlock();
		//if( !bDeleted )
		//{
		//	int iii = 0;
		//}
	}

public:

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	zVerifyThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zVerifyThread"))
		: zThread(name),pool(pool)
	{
		task_count = 0;
	}

	/**
	* \brief 析构函数
	*
	*/
	~zVerifyThread()
	{
	}

	void run();

};

/**
* \brief 等待接受验证指令,并进行验证
*
*/
void zVerifyThread::run()
{
	Zebra::logger->debug("zVerifyThread::run");

	zRTime currentTime;
	zTCPTask_IT it,next;
	pollfdContainer::size_type i;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{

		//fprintf(stderr,"zVerifyThread::run\n");

		//if( dwBeginTime != 0 )
		//{
		//	Zebra::logger->debug("zVerifyThread循环时间：%d ms", GetTickCount() - dwBeginTime);
		//}

		//dwBeginTime = GetTickCount();


		currentTime.now();

		check_queue();
		//if (!pfds.empty())
		{
			m_Lock.lock();
			if(!tasks.empty())
			{
				for(i = 0,it = tasks.begin();  it != tasks.end();)
				{
					zTCPTask *task = *it;
					if (task->checkVerifyTimeout(currentTime))
					{
						//超过指定时间验证还没有通过,需要回收连接
						it = tasks.erase(it);
						task_count = tasks.size();
						task->resetState();
						pool->addRecycle(task);
					}
					else
					{
						i ++;
						it++;
					}
				}
				if(!tasks.empty())
				{
					int i;
					bool status = false;

					for(i = 0,it = tasks.begin(); it != tasks.end();)
					{						
						Zebra::logger->error("verify2 %d",tasks.size());
						zTCPTask *task = *it;
						int ret = task->WaitRecv( false );
						if ( ret == -1 )
						{
							//套接口出现错误
							it = tasks.erase(it);
							task_count = tasks.size();
							task->resetState();
							printf("套接口错误回收\n");
							pool->addRecycle(task);
						}
						else if( ret > 0 )
						{
							switch(task->verifyConn())
							{
							case 1:
								//验证成功
								it = tasks.erase(it);
								task_count = tasks.size();
								//再做唯一性验证
								if (task->uniqueAdd())
								{
									//唯一性验证成功,获取下一个状态
									Zebra::logger->debug("客户端唯一性验证成功");
									task->setUnique();
									pool->addSync(task);
								}
								else
								{
									//唯一性验证失败,回收连接任务
									Zebra::logger->debug("客户端唯一性验证失败");
									task->resetState();
									printf("唯一性验证失败回收\n");
									pool->addRecycle(task);
								}
								break;

							case -1:
								//验证失败,回收任务
								it = tasks.erase(it);
								task_count = tasks.size();
								task->resetState();
								printf("验证失败回收\n");
								pool->addRecycle(task);
								break;	
							default:
								//超时,下面会处理
								i++;
								it++;
								break;
							}
						}
						else
						{
							i++;
							it++;
						}
					}
				}
			}
			m_Lock.unlock();
		}

		zThread::msleep(50);
	}

	//把所有等待验证队列中的连接加入到回收队列中,回收这些连接

	fprintf(stderr,"zVerifyThread::final\n");

	if(tasks.size() == 0)
		return;
	for(i = 0,it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		remove(it,i);
		task->resetState();
		pool->addRecycle(task);
	}
}

/**
* \brief 等待其它线程同步验证这个连接,如果失败或者超时,都需要回收连接
*
*/
class zSyncThread : public zThread,public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< 任务列表 */

	zMutex m_Lock;
	void _add(zTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	zSyncThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zSyncThread"))
		: zThread(name),pool(pool)
	{}

	/**
	* \brief 析构函数
	*
	*/
	~zSyncThread() {};

	void run();

};

/**
* \brief 等待其它线程同步验证这个连接
*
*/
void zSyncThread::run()
{
	Zebra::logger->debug("zSyncThread::run");
	zTCPTask_IT it;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{

		//fprintf(stderr,"zVerifyThread::run\n");

		//if( dwBeginTime != 0 )
		//{
		//	Zebra::logger->debug("zSyncThread循环时间：%d ms", GetTickCount() - dwBeginTime);
		//}

		//dwBeginTime = GetTickCount();

		//fprintf(stderr,"zSyncThread::run\n");
		check_queue();

		m_Lock.lock();
		if (!tasks.empty())
		{
			for(it = tasks.begin(); it != tasks.end();)
			{
				zTCPTask *task = (*it);
				switch(task->waitSync())
				{
				case 1:
					//等待其它线程同步验证成功
					it = tasks.erase(it);
					if (!pool->addOkay(task))
					{
						task->resetState();
						pool->addRecycle(task);
					}
					break;
				case 0:
					it++;
					break;
				case -1:
					//等待其它线程同步验证失败,需要回收连接
					it = tasks.erase(it);
					task->resetState();
					pool->addRecycle(task);
					break;
				}
			}
		}
		m_Lock.unlock();
		zThread::msleep(200);
	}

	fprintf(stderr,"zSyncThread::final\n");
	//把所有等待同步验证队列中的连接加入到回收队列中,回收这些连接
	for(it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		it = tasks.erase(it);
		task->resetState();
		pool->addRecycle(task);
	}
}

/**
* \brief TCP连接的主处理线程,一般一个线程带几个TCP连接,这样可以显著提高效率
*
*/
class zOkayThread : public zThread,public zTCPTaskQueue
{

private:

	Timer  _one_sec_; // 秒定时器
	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< 任务列表 */
	zTCPTaskContainer::size_type task_count;      /**< tasks计数(保证线程安全*/

	//pollfdContainer pfds;

	zMutex m_Lock;

	void _add(zTCPTask *task)
	{
		m_Lock.lock();
	//	struct pollfd pfd;
	//	task->fillPollFD(pfd,POLLIN | POLLOUT | POLLPRI);
		tasks.push_back(task);
		task_count = tasks.size();
		//pfds.push_back(pfd);
		task->ListeningRecv(false);
		m_Lock.unlock();
	}


	void remove(zTCPTask_IT &it,int p)
	{
		//int i;
		//pollfdContainer::iterator iter;
		//bool bDeleted = false;
		//m_Lock.lock();
		//for(iter = pfds.begin(),i = 0; iter != pfds.end(); iter++,i++)
		//{
		//	if (i == p)
		//	{
		//		pfds.erase(iter);
		//		it = tasks.erase(it);
		//		task_count = tasks.size();
		//		g_CsizeO = task_count;
		//		bDeleted = true;
		//		break;
		//	}
		//}
		//m_Lock.unlock();
		//if( !bDeleted )
		//{
		//	int iii = 0;
		//}
	}

public:

	static const zTCPTaskContainer::size_type connPerThread = 512;  /**< 每个线程带的连接数量 */

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	zOkayThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zOkayThread"))
		: zThread(name),pool(pool),_one_sec_(1)
	{
		task_count = 0;
	}

	/**
	* \brief 析构函数
	*
	*/
	~zOkayThread()
	{
	}

	void run();

	/**
	* \brief 返回连接任务的个数
	* \return 这个线程处理的连接任务数
	*/
	const zTCPTaskContainer::size_type size() const
	{
		return task_count + _size;
	}

};

/**
* \brief 主处理线程,回调处理连接的输入输出指令
*
*/
void zOkayThread::run()
{
	Zebra::logger->debug("zOkayThread::run");

	zRTime currentTime;
	zTCPTask_IT it,next;
	pollfdContainer::size_type i;

	int time = pool->usleep_time;
	pollfdContainer::iterator iter_r;
	pollfdContainer pfds_r;
	zTCPTaskContainer tasks_r;    
	bool check=false;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{

		//fprintf(stderr,"zVerifyThread::run\n");

		//if( dwBeginTime != 0 )
		//{
		//	Zebra::logger->debug("zOkayThread循环时间：%d ms", GetTickCount() - dwBeginTime);
		//}

		//dwBeginTime = GetTickCount();
		currentTime.now();
		check_queue();
		if (check)
		{
			m_Lock.lock();
			if (!tasks.empty())
			{
				for(i = 0,it = tasks.begin(); it != tasks.end(); )
				{
					zTCPTask *task = *it;
					//检查测试信号指令
					task->checkSignal(currentTime);

					if (task->isTerminateWait())
					{
						task->Terminate();
					}
					if (task->isTerminate())
					{
						it = tasks.erase(it);
						task_count = tasks.size();
						// state_sync -> state_okay
						/*
						* whj
						* 先设置状态再添加容器,
						* 否则会导致一个task同时在两个线程中的危险情况
						*/
						task->getNextState();
						pool->addRecycle(task);
					}
					else
					{
						i ++;
						it ++;
					}
					//else
					//{
					//	pfds[i].revents = 0;
					//}
				}
			}
			m_Lock.unlock();
			check=false;
		}
		zThread::usleep(time);
		time = 0;
		if (check)
		{
			if (time <=0)
			{
				time = 0;
			}
			continue;
		}
		if (time <=0)
		{
			m_Lock.lock();
			if (!tasks.empty())
			{
				for(i = 0,it = tasks.begin(); it != tasks.end(); it++,i++)
				{
					zTCPTask *task = (*it);

					bool UseIocp = task->UseIocp();

					if( UseIocp )
					{ 
						int retcode = task->WaitRecv( false );
						if ( retcode == -1 )
						{
							//套接口出现错误
							Zebra::logger->debug("zOkayThread::run: 套接口异常错误");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if( retcode > 0 )
						{
							//套接口准备好了读取操作
							if (!task->ListeningRecv(true))
							{
								Zebra::logger->debug("zOkayThread::run: 套接口读操作错误");
								task->Terminate(zTCPTask::terminate_active);
							}
						}
						retcode = task->WaitSend( false );
						if( retcode == -1 )
						{
							//套接口出现错误
							Zebra::logger->debug("zOkayThread::run: 套接口异常错误");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if( retcode ==  1 )
						{
							//套接口准备好了写入操作
							if (!task->ListeningSend())
							{
								Zebra::logger->debug("zOkayThread::run: 套接口写操作错误 port = %u",task->getPort());

								task->Terminate(zTCPTask::terminate_active);
							}
						}
					}						
				}
			}
			m_Lock.unlock();
			time = pool->usleep_time;
		}
		check=true;
	}

	//把所有任务队列中的连接加入到回收队列中,回收这些连接

	fprintf(stderr,"zOkayThread::final\n");


	if(tasks.size() == 0)
		return;

	for(i = 0,it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		it = tasks.erase(it);
		//state_sync -> state_okay
		/*
		* whj
		* 先设置状态再添加容器,
		* 否则会导致一个task同时在两个线程中的危险情况
		*/
		task->getNextState();
		pool->addRecycle(task);
	}
}

/**
* \brief 连接回收线程,回收所有无用的TCP连接,释放相应的资源
*
*/
DWORD dwStep[100];
class zRecycleThread : public zThread,public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< 任务列表 */

	zMutex m_Lock;

	void _add(zTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	zRecycleThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zRecycleThread"))
		: zThread(name),pool(pool)
	{}

	/**
	* \brief 析构函数
	*
	*/
	~zRecycleThread() {};

	void run();

};

/**
* \brief 连接回收处理线程,在删除内存空间之前需要保证recycleConn返回1
*
*/
//std::map<zTCPTask*,int> g_RecycleLog;
void zRecycleThread::run()
{
	Zebra::logger->debug("zRecycleThread::run");
	zTCPTask_IT it;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{		
		//fprintf(stderr,"zVerifyThread::run\n");

		//if( dwBeginTime != 0 )
		//{
		//	Zebra::logger->debug("zRecycleThread循环时间：%d ms", GetTickCount() - dwBeginTime);
		//}

		//dwBeginTime = GetTickCount();
		//fprintf(stderr,"zRecycleThread::run\n");
		check_queue();

		DWORD dwLog = 0;
		int i;
		m_Lock.lock();
		if (!tasks.empty())
		{
			for(i = 0,it = tasks.begin(); it != tasks.end();i++)
			{
				zTCPTask *task = *it;
				switch(task->recycleConn())
				{
				case 1:
					//回收处理完成可以释放相应的资源
					it = tasks.erase(it);
					if (task->isUnique())
						//如果已经通过了唯一性验证,从全局唯一容器中删除
						task->uniqueRemove();
					task->getNextState();
					//				if( !task->UseIocp() ) // [ranqd] 使用Iocp的连接不在这里回收
//					g_RecycleLog[task] = 0;
					SAFE_DELETE(task);
					break;
				default:
					//回收超时,下次再处理
					it++;
					break;
				}
			}
		}
		m_Lock.unlock();

		zThread::msleep(200);
	}

	//回收所有的连接

	fprintf(stderr,"zRecycleThread::final\n");
	for(it = tasks.begin(); it != tasks.end();)
	{
		//回收处理完成可以释放相应的资源
		zTCPTask *task = *it;
		it = tasks.erase(it);
		if (task->isUnique())
			//如果已经通过了唯一性验证,从全局唯一容器中删除
			task->uniqueRemove();
		task->getNextState();
		SAFE_DELETE(task);
	}
}


/**
* \brief 返回连接池中子连接个数
*
*/
const int zTCPTaskPool::getSize()
{
	Zebra::logger->debug("zTCPTaskPool::getSize");
	struct MyCallback : zThreadGroup::Callback
	{
		int size;
		MyCallback() : size(0) {}
		void exec(zThread *e)
		{
			zOkayThread *pOkayThread = (zOkayThread *)e;
			size += pOkayThread->size();
		}
	};
	MyCallback mcb;
	okayThreads.execAll(mcb);
	return mcb.size;
}

/**
* \brief 把一个TCP连接添加到验证队列中,因为存在多个验证队列,需要按照一定的算法添加到不同的验证处理队列中
*
* \param task 一个连接任务
*/
bool zTCPTaskPool::addVerify(zTCPTask *task)
{

	Zebra::logger->debug("zTCPTaskPool::addVerify");
	//因为存在多个验证队列,需要按照一定的算法添加到不同的验证处理队列中
	static DWORD hashcode = 0;
	zVerifyThread *pVerifyThread = (zVerifyThread *)verifyThreads.getByIndex(hashcode++ % maxVerifyThreads);
	if (pVerifyThread)
	{
		// state_sync -> state_okay
		/*
		* whj
		* 先设置状态再添加容器,
		* 否则会导致一个task同时在两个线程中的危险情况
		*/
		task->getNextState();
		pVerifyThread->add(task);
	}
	return true;
}

/**
* \brief 把一个通过验证的TCP连接添加到等待同步验证队列中
*
* \param task 一个连接任务
*/
void zTCPTaskPool::addSync(zTCPTask *task)
{
	Zebra::logger->debug("zTCPTaskPool::addSync");
	// state_sync -> state_okay
	/*
	* whj
	* 先设置状态再添加容器,
	* 否则会导致一个task同时在两个线程中的危险情况
	*/
	task->getNextState();
	syncThread->add(task);
}

/**
* \brief 把一个通过验证的TCP处理队列中
*
* \param task 一个连接任务
* \return 添加是否成功
*/
bool zTCPTaskPool::addOkay(zTCPTask *task)
{
	Zebra::logger->debug("zTCPTaskPool::addOkay");
	//首先遍历所有的线程,找出运行的并且连接数最少的线程,再找出没有启动的线程
	zOkayThread *pmin = NULL,*nostart = NULL;
	for(int i = 0; i < maxThreadCount; i++)
	{
		zOkayThread *pOkayThread = (zOkayThread *)okayThreads.getByIndex(i);
		if (pOkayThread)
		{
			if (pOkayThread->isAlive())
			{
				if (NULL == pmin || pmin->size() > pOkayThread->size())
					pmin = pOkayThread;
			}
			else
			{
				nostart = pOkayThread;
				break;
			}
		}
	}
	if (pmin && pmin->size() < zOkayThread::connPerThread)
	{
		// state_sync -> state_okay
		/*
		* whj
		* 先设置状态再添加容器,
		* 否则会导致一个task同时在两个线程中的危险情况
		*/
		task->getNextState();
		//这个线程同时处理的连接数还没有到达上限
		pmin->add(task);
		return true;
	}
	if (nostart)
	{
		//线程还没有运行,需要创建线程,再把添加到这个线程的处理队列中
		if (nostart->start())
		{
			Zebra::logger->debug("zTCPTaskPool创建工作线程");
			// state_sync -> state_okay
			/*
			* whj
			* 先设置状态再添加容器,
			* 否则会导致一个task同时在两个线程中的危险情况
			*/
			task->getNextState();
			//这个线程同时处理的连接数还没有到达上限
			nostart->add(task);
			return true;
		}
		else
			Zebra::logger->fatal("zTCPTaskPool不能创建工作线程");
	}

	Zebra::logger->fatal("zTCPTaskPool没有找到合适的线程来处理连接");
	//没有找到线程来处理这个连接,需要回收关闭连接
	return false;
}

/**
* \brief 把一个TCP连接添加到回收处理队列中
*
* \param task 一个连接任务
*/

void zTCPTaskPool::addRecycle(zTCPTask *task)
{
	Zebra::logger->debug("zTCPTaskPool::addRecycle");
	//if( g_RecycleLog[task] == 0 )
	//{
	//	g_RecycleLog[task] = 1;
	//}
	//else
	//{
	//	int iii = 0;
	//}
	recycleThread->add(task);
}


/**
* \brief 初始化线程池,预先创建各种线程
*
* \return 初始化是否成功
*/
bool zTCPTaskPool::init()
{
	Zebra::logger->debug("zTCPTaskPool::init");
	//创建初始化验证线程
	for(int i = 0; i < maxVerifyThreads; i++)
	{
		std::ostringstream name;
		name << "zVerifyThread[" << i << "]";
		zVerifyThread *pVerifyThread = new zVerifyThread(this,name.str());
		if (NULL == pVerifyThread)
			return false;
		if (!pVerifyThread->start())
			return false;
		verifyThreads.add(pVerifyThread);
	}

	//创建初始化等待同步验证线程
	syncThread = new zSyncThread(this);
	if (syncThread && !syncThread->start())
		return false;

	//创建初始化主运行线程池
	maxThreadCount = (maxConns + zOkayThread::connPerThread - 1) / zOkayThread::connPerThread;
	Zebra::logger->debug("最大TCP连接数%d,每线程TCP连接数%d,线程个数%d",maxConns,zOkayThread::connPerThread,maxThreadCount);
	for(int i = 0; i < maxThreadCount; i++)
	{
		std::ostringstream name;
		name << "zOkayThread[" << i << "]";
		zOkayThread *pOkayThread = new zOkayThread(this,name.str());
		if (NULL == pOkayThread)
			return false;
		if (i < minThreadCount && !pOkayThread->start())
			return false;
		okayThreads.add(pOkayThread);
	}

	//创建初始化回收线程池
	recycleThread = new zRecycleThread(this);
	if (recycleThread && !recycleThread->start())
		return false;

	return true;
}

/**
* \brief 释放线程池,释放各种资源,等待各种线程退出
*
*/
void zTCPTaskPool::final()
{
	verifyThreads.joinAll();
	if (syncThread)
	{
		syncThread->final();
		syncThread->join();
		SAFE_DELETE(syncThread);
	}

	okayThreads.joinAll();
	if (recycleThread)
	{
		recycleThread->final();
		recycleThread->join();
		SAFE_DELETE(recycleThread);
	}
}

