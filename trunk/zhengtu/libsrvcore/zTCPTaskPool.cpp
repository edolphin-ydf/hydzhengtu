/**
* \brief ʵ���̳߳���,���ڴ�������ӷ�����
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

int zTCPTaskPool::usleep_time=50000;                    /**< ѭ���ȴ�ʱ�� */
/**
* \brief ������������
*
*/
//typedef std::list<zTCPTask *,__gnu_cxx::__pool_alloc<zTCPTask *> > zTCPTaskContainer;
typedef std::vector<zTCPTask *> zTCPTaskContainer;

/**
* \brief �����������������
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
* \brief ����TCP���ӵ���֤,�����֤��ͨ��,��Ҫ�����������
*
*/
class zVerifyThread : public zThread,public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< �����б� */
	zTCPTaskContainer::size_type task_count;      /**< tasks����(��֤�̰߳�ȫ*/
//	pollfdContainer pfds;

	zMutex m_Lock;
	/**
	* \brief ���һ����������
	* \param task ��������
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
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zVerifyThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zVerifyThread"))
		: zThread(name),pool(pool)
	{
		task_count = 0;
	}

	/**
	* \brief ��������
	*
	*/
	~zVerifyThread()
	{
	}

	void run();

};

/**
* \brief �ȴ�������ָ֤��,��������֤
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
		//	Zebra::logger->debug("zVerifyThreadѭ��ʱ�䣺%d ms", GetTickCount() - dwBeginTime);
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
						//����ָ��ʱ����֤��û��ͨ��,��Ҫ��������
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
							//�׽ӿڳ��ִ���
							it = tasks.erase(it);
							task_count = tasks.size();
							task->resetState();
							printf("�׽ӿڴ������\n");
							pool->addRecycle(task);
						}
						else if( ret > 0 )
						{
							switch(task->verifyConn())
							{
							case 1:
								//��֤�ɹ�
								it = tasks.erase(it);
								task_count = tasks.size();
								//����Ψһ����֤
								if (task->uniqueAdd())
								{
									//Ψһ����֤�ɹ�,��ȡ��һ��״̬
									Zebra::logger->debug("�ͻ���Ψһ����֤�ɹ�");
									task->setUnique();
									pool->addSync(task);
								}
								else
								{
									//Ψһ����֤ʧ��,������������
									Zebra::logger->debug("�ͻ���Ψһ����֤ʧ��");
									task->resetState();
									printf("Ψһ����֤ʧ�ܻ���\n");
									pool->addRecycle(task);
								}
								break;

							case -1:
								//��֤ʧ��,��������
								it = tasks.erase(it);
								task_count = tasks.size();
								task->resetState();
								printf("��֤ʧ�ܻ���\n");
								pool->addRecycle(task);
								break;	
							default:
								//��ʱ,����ᴦ��
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

	//�����еȴ���֤�����е����Ӽ��뵽���ն�����,������Щ����

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
* \brief �ȴ������߳�ͬ����֤�������,���ʧ�ܻ��߳�ʱ,����Ҫ��������
*
*/
class zSyncThread : public zThread,public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< �����б� */

	zMutex m_Lock;
	void _add(zTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zSyncThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zSyncThread"))
		: zThread(name),pool(pool)
	{}

	/**
	* \brief ��������
	*
	*/
	~zSyncThread() {};

	void run();

};

/**
* \brief �ȴ������߳�ͬ����֤�������
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
		//	Zebra::logger->debug("zSyncThreadѭ��ʱ�䣺%d ms", GetTickCount() - dwBeginTime);
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
					//�ȴ������߳�ͬ����֤�ɹ�
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
					//�ȴ������߳�ͬ����֤ʧ��,��Ҫ��������
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
	//�����еȴ�ͬ����֤�����е����Ӽ��뵽���ն�����,������Щ����
	for(it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		it = tasks.erase(it);
		task->resetState();
		pool->addRecycle(task);
	}
}

/**
* \brief TCP���ӵ��������߳�,һ��һ���̴߳�����TCP����,���������������Ч��
*
*/
class zOkayThread : public zThread,public zTCPTaskQueue
{

private:

	Timer  _one_sec_; // �붨ʱ��
	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< �����б� */
	zTCPTaskContainer::size_type task_count;      /**< tasks����(��֤�̰߳�ȫ*/

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

	static const zTCPTaskContainer::size_type connPerThread = 512;  /**< ÿ���̴߳����������� */

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zOkayThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zOkayThread"))
		: zThread(name),pool(pool),_one_sec_(1)
	{
		task_count = 0;
	}

	/**
	* \brief ��������
	*
	*/
	~zOkayThread()
	{
	}

	void run();

	/**
	* \brief ������������ĸ���
	* \return ����̴߳��������������
	*/
	const zTCPTaskContainer::size_type size() const
	{
		return task_count + _size;
	}

};

/**
* \brief �������߳�,�ص��������ӵ��������ָ��
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
		//	Zebra::logger->debug("zOkayThreadѭ��ʱ�䣺%d ms", GetTickCount() - dwBeginTime);
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
					//�������ź�ָ��
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
						* ������״̬���������,
						* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
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
							//�׽ӿڳ��ִ���
							Zebra::logger->debug("zOkayThread::run: �׽ӿ��쳣����");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if( retcode > 0 )
						{
							//�׽ӿ�׼�����˶�ȡ����
							if (!task->ListeningRecv(true))
							{
								Zebra::logger->debug("zOkayThread::run: �׽ӿڶ���������");
								task->Terminate(zTCPTask::terminate_active);
							}
						}
						retcode = task->WaitSend( false );
						if( retcode == -1 )
						{
							//�׽ӿڳ��ִ���
							Zebra::logger->debug("zOkayThread::run: �׽ӿ��쳣����");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if( retcode ==  1 )
						{
							//�׽ӿ�׼������д�����
							if (!task->ListeningSend())
							{
								Zebra::logger->debug("zOkayThread::run: �׽ӿ�д�������� port = %u",task->getPort());

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

	//��������������е����Ӽ��뵽���ն�����,������Щ����

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
		* ������״̬���������,
		* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
		*/
		task->getNextState();
		pool->addRecycle(task);
	}
}

/**
* \brief ���ӻ����߳�,�����������õ�TCP����,�ͷ���Ӧ����Դ
*
*/
DWORD dwStep[100];
class zRecycleThread : public zThread,public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< �����б� */

	zMutex m_Lock;

	void _add(zTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zRecycleThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zRecycleThread"))
		: zThread(name),pool(pool)
	{}

	/**
	* \brief ��������
	*
	*/
	~zRecycleThread() {};

	void run();

};

/**
* \brief ���ӻ��մ����߳�,��ɾ���ڴ�ռ�֮ǰ��Ҫ��֤recycleConn����1
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
		//	Zebra::logger->debug("zRecycleThreadѭ��ʱ�䣺%d ms", GetTickCount() - dwBeginTime);
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
					//���մ�����ɿ����ͷ���Ӧ����Դ
					it = tasks.erase(it);
					if (task->isUnique())
						//����Ѿ�ͨ����Ψһ����֤,��ȫ��Ψһ������ɾ��
						task->uniqueRemove();
					task->getNextState();
					//				if( !task->UseIocp() ) // [ranqd] ʹ��Iocp�����Ӳ����������
//					g_RecycleLog[task] = 0;
					SAFE_DELETE(task);
					break;
				default:
					//���ճ�ʱ,�´��ٴ���
					it++;
					break;
				}
			}
		}
		m_Lock.unlock();

		zThread::msleep(200);
	}

	//�������е�����

	fprintf(stderr,"zRecycleThread::final\n");
	for(it = tasks.begin(); it != tasks.end();)
	{
		//���մ�����ɿ����ͷ���Ӧ����Դ
		zTCPTask *task = *it;
		it = tasks.erase(it);
		if (task->isUnique())
			//����Ѿ�ͨ����Ψһ����֤,��ȫ��Ψһ������ɾ��
			task->uniqueRemove();
		task->getNextState();
		SAFE_DELETE(task);
	}
}


/**
* \brief �������ӳ��������Ӹ���
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
* \brief ��һ��TCP������ӵ���֤������,��Ϊ���ڶ����֤����,��Ҫ����һ�����㷨��ӵ���ͬ����֤���������
*
* \param task һ����������
*/
bool zTCPTaskPool::addVerify(zTCPTask *task)
{

	Zebra::logger->debug("zTCPTaskPool::addVerify");
	//��Ϊ���ڶ����֤����,��Ҫ����һ�����㷨��ӵ���ͬ����֤���������
	static DWORD hashcode = 0;
	zVerifyThread *pVerifyThread = (zVerifyThread *)verifyThreads.getByIndex(hashcode++ % maxVerifyThreads);
	if (pVerifyThread)
	{
		// state_sync -> state_okay
		/*
		* whj
		* ������״̬���������,
		* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
		*/
		task->getNextState();
		pVerifyThread->add(task);
	}
	return true;
}

/**
* \brief ��һ��ͨ����֤��TCP������ӵ��ȴ�ͬ����֤������
*
* \param task һ����������
*/
void zTCPTaskPool::addSync(zTCPTask *task)
{
	Zebra::logger->debug("zTCPTaskPool::addSync");
	// state_sync -> state_okay
	/*
	* whj
	* ������״̬���������,
	* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
	*/
	task->getNextState();
	syncThread->add(task);
}

/**
* \brief ��һ��ͨ����֤��TCP���������
*
* \param task һ����������
* \return ����Ƿ�ɹ�
*/
bool zTCPTaskPool::addOkay(zTCPTask *task)
{
	Zebra::logger->debug("zTCPTaskPool::addOkay");
	//���ȱ������е��߳�,�ҳ����еĲ������������ٵ��߳�,���ҳ�û���������߳�
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
		* ������״̬���������,
		* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
		*/
		task->getNextState();
		//����߳�ͬʱ�������������û�е�������
		pmin->add(task);
		return true;
	}
	if (nostart)
	{
		//�̻߳�û������,��Ҫ�����߳�,�ٰ���ӵ�����̵߳Ĵ��������
		if (nostart->start())
		{
			Zebra::logger->debug("zTCPTaskPool���������߳�");
			// state_sync -> state_okay
			/*
			* whj
			* ������״̬���������,
			* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
			*/
			task->getNextState();
			//����߳�ͬʱ�������������û�е�������
			nostart->add(task);
			return true;
		}
		else
			Zebra::logger->fatal("zTCPTaskPool���ܴ��������߳�");
	}

	Zebra::logger->fatal("zTCPTaskPoolû���ҵ����ʵ��߳�����������");
	//û���ҵ��߳��������������,��Ҫ���չر�����
	return false;
}

/**
* \brief ��һ��TCP������ӵ����մ��������
*
* \param task һ����������
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
* \brief ��ʼ���̳߳�,Ԥ�ȴ��������߳�
*
* \return ��ʼ���Ƿ�ɹ�
*/
bool zTCPTaskPool::init()
{
	Zebra::logger->debug("zTCPTaskPool::init");
	//������ʼ����֤�߳�
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

	//������ʼ���ȴ�ͬ����֤�߳�
	syncThread = new zSyncThread(this);
	if (syncThread && !syncThread->start())
		return false;

	//������ʼ���������̳߳�
	maxThreadCount = (maxConns + zOkayThread::connPerThread - 1) / zOkayThread::connPerThread;
	Zebra::logger->debug("���TCP������%d,ÿ�߳�TCP������%d,�̸߳���%d",maxConns,zOkayThread::connPerThread,maxThreadCount);
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

	//������ʼ�������̳߳�
	recycleThread = new zRecycleThread(this);
	if (recycleThread && !recycleThread->start())
		return false;

	return true;
}

/**
* \brief �ͷ��̳߳�,�ͷŸ�����Դ,�ȴ������߳��˳�
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

