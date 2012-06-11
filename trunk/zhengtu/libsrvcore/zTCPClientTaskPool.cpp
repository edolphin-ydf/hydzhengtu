/**
* \brief ʵ���̳߳���,���ڴ�������ӷ�����
*
* 
*/
#include <zebra/srvEngine.h>

#include <assert.h>
//#include <ext/pool_allocator.h>

#include <iostream>

/**
* \brief ���TCP����״��,���δ����,��������
*
*/
class zCheckconnectThread : public zThread
{
private:
	zTCPClientTaskPool *pool;
public:
	zCheckconnectThread(
		zTCPClientTaskPool *pool,
		const std::string &name = std::string("zCheckconnectThread"))
		: zThread(name),pool(pool)
	{
	}
	virtual void run()
	{
		while(!isFinal())
		{
			zThread::sleep(4);
			zTime ct;
			pool->timeAction(ct);
		}
	}
};

/**
* \brief ������������
*
*/
//typedef std::list<zTCPClientTask *,__gnu_cxx::__pool_alloc<zTCPClientTask *> > zTCPClientTaskContainer;
typedef std::list<zTCPClientTask *> zTCPClientTaskContainer;

/**
* \brief �����������������
*
*/
typedef zTCPClientTaskContainer::iterator zTCPClientTask_IT;

typedef std::vector<struct mypollfd> pollfdContainer;

class zTCPClientTaskQueue
{
public:
	zTCPClientTaskQueue() :_size(0) {}
	virtual ~zTCPClientTaskQueue() {}
	inline void add(zTCPClientTask *task)
	{
		mlock.lock();
		_queue.push(task);
		_size++;
		mlock.unlock();
	}
	inline void check_queue()
	{
		mlock.lock();
		while(!_queue.empty())
		{
			zTCPClientTask *task = _queue.front();
			_queue.pop();
			_add(task);
		}
		_size = 0;
		mlock.unlock();
	}
protected:
	virtual void _add(zTCPClientTask *task) = 0;
	DWORD _size;
private:
	zMutex mlock;
	//std::queue<zTCPClientTask *,std::deque<zTCPClientTask *,__gnu_cxx::__pool_alloc<zTCPClientTask *> > > _queue;
	std::queue<zTCPClientTask *> _queue;
};

/**
* \brief ����TCP���ӵ���֤,�����֤��ͨ��,��Ҫ�����������
*
*/
class zCheckwaitThread : public zThread,public zTCPClientTaskQueue
{

private:

	zTCPClientTaskPool *pool;
	zTCPClientTaskContainer tasks;  /**< �����б� */
	zTCPClientTaskContainer::size_type task_count;          /**< tasks����(��֤�̰߳�ȫ*/
	pollfdContainer pfds;

	/**
	* \brief ���һ����������
	* \param task ��������
	*/
	void _add(zTCPClientTask *task)
	{
		Zebra::logger->debug("zCheckwaitThread::_add");

		struct mypollfd pfd;
		task->fillPollFD(pfd,POLLIN | POLLPRI);
		tasks.push_back(task);
		task_count = tasks.size();
		pfds.push_back(pfd);
	}

	void remove(zTCPClientTask_IT &it,int p)
	{
		Zebra::logger->debug("zCheckwaitThread::remove");
		int i=0;
		pollfdContainer::iterator iter;
		for(iter = pfds.begin(),i = 0; iter != pfds.end(); iter++,i++)
		{
			if (i == p)
			{
				pfds.erase(iter);
				it = tasks.erase(it);
				task_count = tasks.size();
				break;
			}
		}
	}

public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zCheckwaitThread(
		zTCPClientTaskPool *pool,
		const std::string &name = std::string("zCheckwaitThread"))
		: zThread(name),pool(pool)
	{
		task_count = 0;
	}

	/**
	* \brief ��������
	*
	*/
	~zCheckwaitThread()
	{
	}

	virtual void run();

};

/**
* \brief �ȴ�������ָ֤��,��������֤
*
*/
void zCheckwaitThread::run()
{
	Zebra::logger->debug("zCheckwaitThread::run");

	zTCPClientTask_IT it,next;
	pollfdContainer::size_type i;

	while(!isFinal())
	{
		check_queue();

		if (tasks.size() > 0)
		{
			if( WaitRecvAll( &pfds[0],pfds.size(), 0 ) <= 0 ) continue;

			for(i = 0,it = tasks.begin(); it != tasks.end();)
			{
				zTCPClientTask *task = *it;

				if ( pfds[i].revents & POLLPRI )
				{
					//�׽ӿڳ��ִ���
					printf("�׽ӿڳ��ִ���remove\n");
					remove(it,i--);
					task->resetState();
				}
				else if( pfds[i].revents & POLLIN )
				{
					switch(task->checkRebound())
					{
					case 1:
						//��֤�ɹ�,��ȡ��һ��״̬
						remove(it,i);
						if (!pool->addMain(task))
							task->resetState();
						break;
					case -1:
						//��֤ʧ��,��������
						printf("��֤ʧ��remove\n");
						remove(it,i);
						task->resetState();
						break;
					default:
						it ++;
						i  ++;
						//��ʱ,����ᴦ��
						break;
					}
				}
				else
				{
					i ++;
					it ++;
				}
			}
		}

		zThread::msleep(50);
	}

	if(tasks.size() == 0)
		return;
	//�����еȴ���֤�����е����Ӽ��뵽���ն�����,������Щ����
	for(i = 0,it = tasks.begin(); it != tasks.end();)
	{
		zTCPClientTask *task = *it;
		remove(it,i);
		task->resetState();
	}
}

/**
* \brief TCP���ӵ��������߳�,һ��һ���̴߳�����TCP����,���������������Ч��
*
*/
class zTCPClientTaskThread : public zThread,public zTCPClientTaskQueue
{

private:

	zTCPClientTaskPool *pool;
	zTCPClientTaskContainer tasks;  /**< �����б� */
	zTCPClientTaskContainer::size_type task_count;          /**< tasks����(��֤�̰߳�ȫ*/

	pollfdContainer pfds;

	zMutex m_Lock;
	/**
	* \brief ���һ����������
	* \param task ��������
	*/
	void _add(zTCPClientTask *task)
	{

		struct mypollfd pfd;
		m_Lock.lock();
		task->fillPollFD(pfd,POLLIN | POLLOUT | POLLPRI);
		tasks.push_back(task);
		task_count = tasks.size();
		pfds.push_back(pfd);
		m_Lock.unlock();
	}


	void remove(zTCPClientTask_IT &it,int p)
	{
		int i;
		pollfdContainer::iterator iter;
		m_Lock.lock();
		for(iter = pfds.begin(),i = 0; iter != pfds.end(); iter++,i++)
		{
			if (i == p)
			{
				pfds.erase(iter);
				it = tasks.erase(it);
				task_count = tasks.size();
				break;
			}
		}
		m_Lock.unlock();
	}

public:

	static const zTCPClientTaskContainer::size_type connPerThread = 256;  /**< ÿ���̴߳����������� */

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zTCPClientTaskThread(
		zTCPClientTaskPool *pool,
		const std::string &name = std::string("zTCPClientTaskThread"))
		: zThread(name),pool(pool)
	{
		task_count = 0;

	}

	/**
	* \brief ��������
	*
	*/
	~zTCPClientTaskThread()
	{
	}

	virtual void run();

	/**
	* \brief ������������ĸ���
	* \return ����̴߳��������������
	*/
	const zTCPClientTaskContainer::size_type size() const
	{
		return task_count + _size;
	}

};

/**
* \brief �������߳�,�ص��������ӵ��������ָ��
*
*/
void zTCPClientTaskThread::run()
{
	Zebra::logger->debug("zTCPClientTaskThread::run");

	zTCPClientTask_IT it,next;
	pollfdContainer::size_type i;

	while(!isFinal())
	{
		check_queue();
		m_Lock.lock();
		if (!tasks.empty())
		{
			for(i = 0,it = tasks.begin(); it != tasks.end();)
			{
				zTCPClientTask *task = *it;

				if (task->isTerminate())
				{
					m_Lock.unlock();
					remove(it,i);
					m_Lock.lock();
					// state_okay -> state_recycle
					task->getNextState();
				}
				else
				{
					if (task->checkFirstMainLoop())
					{
						//����ǵ�һ�μ��봦��,��ҪԤ�ȴ������е�����
						task->ListeningRecv(false);
					}
					i++;
					it++;
				}
			}

			if (!tasks.empty())
			{
				for(i = 0,it = tasks.begin(); it != tasks.end(); it++,i++)
				{
					zTCPClientTask *task = *it;

					bool UseIocp = task->UseIocp();
					if( UseIocp )
					{
						int retcode = task->WaitRecv( false );
						if ( retcode == -1 )
						{
							//�׽ӿڳ��ִ���
							Zebra::logger->debug("%zTCPClientTaskThread::run: �׽ӿ��쳣����");
							task->Terminate(zTCPClientTask::TM_sock_error);
						}
						else if( retcode > 0 )
						{
							//�׽ӿ�׼�����˶�ȡ����
							if (!task->ListeningRecv(true))
							{
								Zebra::logger->debug("zTCPClientTaskThread::run: �׽ӿڶ���������");
								task->Terminate(zTCPClientTask::TM_sock_error);
							}
						}
						retcode = task->WaitSend( false );
						if( retcode == - 1 )
						{
							//�׽ӿڳ��ִ���
							Zebra::logger->debug("%zTCPClientTaskThread::run: �׽ӿ��쳣����");
							task->Terminate(zTCPClientTask::TM_sock_error);
						}
						else if( retcode == 1 )
						{
							//�׽ӿ�׼������д�����
							if (!task->ListeningSend())
							{
								Zebra::logger->debug("zTCPClientTaskThread::run: �׽ӿ�д��������");
								task->Terminate(zTCPClientTask::TM_sock_error);
							}
						}
					}
					else
					{
						if( ::poll(&pfds[i],1,0) <= 0 ) continue;
						if ( pfds[i].revents & POLLPRI )
						{
							//�׽ӿڳ��ִ���
							Zebra::logger->debug("%zTCPClientTaskThread::run: �׽ӿ��쳣����");
							task->Terminate(zTCPClientTask::TM_sock_error);
						}
						else
						{
							if( pfds[i].revents & POLLIN)
							{
								//�׽ӿ�׼�����˶�ȡ����
								if (!task->ListeningRecv(true))
								{
									Zebra::logger->debug("zTCPClientTaskThread::run: �׽ӿڶ���������");
									task->Terminate(zTCPClientTask::TM_sock_error);
								}
							}
							if ( pfds[i].revents & POLLOUT)
							{
								//�׽ӿ�׼������д�����
								if (!task->ListeningSend())
								{
									Zebra::logger->debug("zTCPClientTaskThread::run: �׽ӿ�д��������");
									task->Terminate(zTCPClientTask::TM_sock_error);
								}
							}
						}
					}
				}
			}			
		}
		else
		{
			int iii = 0;
		}
		m_Lock.unlock();
		zThread::usleep(pool->usleep_time);
	}

	//��������������е����Ӽ��뵽���ն�����,������Щ����


	if(tasks.size() == 0)
		return ;

	for(i = 0,it = tasks.begin(); it != tasks.end();)
	{
		zTCPClientTask *task = *it;
		remove(it,i);
		// state_okay -> state_recycle
		task->getNextState();
	}
}



/**
* \brief ��������
*
*/
zTCPClientTaskPool::~zTCPClientTaskPool()
{
	if (checkconnectThread)
	{
		checkconnectThread->final();
		checkconnectThread->join();
		SAFE_DELETE(checkconnectThread);
	}
	if (checkwaitThread)
	{
		checkwaitThread->final();
		checkwaitThread->join();
		SAFE_DELETE(checkwaitThread);
	}

	taskThreads.joinAll();

	zTCPClientTask_IT it,next;


	if(tasks.size() > 0)
		for(it = tasks.begin(),next = it,next++; it != tasks.end(); it = next,next == tasks.end()? next : next++)
		{
			zTCPClientTask *task = *it;
			tasks.erase(it);
			SAFE_DELETE(task);
		}
}

zTCPClientTaskThread *zTCPClientTaskPool::newThread()
{
	std::ostringstream name;
	name << "zTCPClientTaskThread[" << taskThreads.size() << "]";
	zTCPClientTaskThread *taskThread = new zTCPClientTaskThread(this,name.str());
	if (NULL == taskThread)
		return NULL;
	if (!taskThread->start())
		return NULL;
	taskThreads.add(taskThread);
	return taskThread;
}

/**
* \brief ��ʼ���̳߳�,Ԥ�ȴ��������߳�
*
* \return ��ʼ���Ƿ�ɹ�
*/
bool zTCPClientTaskPool::init()
{
	checkconnectThread = new zCheckconnectThread(this); 
	if (NULL == checkconnectThread)
		return false;
	if (!checkconnectThread->start())
		return false;
	checkwaitThread = new zCheckwaitThread(this);
	if (NULL == checkwaitThread)
		return false;
	if (!checkwaitThread->start())
		return false;

	if (NULL == newThread())
		return false;

	return true;
}

/**
* \brief ��һ��ָ��������ӵ�����
* \param task ����ӵ�����
*/
bool zTCPClientTaskPool::put(zTCPClientTask *task)
{
	if (task)
	{
		mlock.lock();
		tasks.push_front(task);
		mlock.unlock();
		return true;
	}
	else
		return false;
}

/**
* \brief ��ʱִ�е�����
* ��Ҫ������ͻ��˶��߳�������
*/
void zTCPClientTaskPool::timeAction(const zTime &ct)
{
	mlock.lock();
	for(zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
	{
		zTCPClientTask *task = *it;
		switch(task->getState())
		{
		case zTCPClientTask::close:
			if (task->checkStateTimeout(zTCPClientTask::close,ct,4)
				&& task->connect())
			{
				addCheckwait(task);
			}
			break;
		case zTCPClientTask::sync:
			break;
		case zTCPClientTask::okay:
			//�Ѿ�������״̬,������������ź�
			task->checkConn();
			break;
		case zTCPClientTask::recycle:
			if (task->checkStateTimeout(zTCPClientTask::recycle,ct,4))
				task->getNextState();
			break;
		}
	}
	mlock.unlock();
}

/**
* \brief ��������ӵ��ȴ�������֤���صĶ�����
* \param task ����ӵ�����
*/
void zTCPClientTaskPool::addCheckwait(zTCPClientTask *task)
{
	checkwaitThread->add(task);
	task->getNextState();
}

/**
* \brief ��������ӵ�������ѭ����
* \param task ����ӵ�����
* \return ����Ƿ�ɹ�
*/
bool zTCPClientTaskPool::addMain(zTCPClientTask *task)
{
	zTCPClientTaskThread *taskThread = NULL;
	for(DWORD i = 0; i < taskThreads.size(); i++)
	{
		zTCPClientTaskThread *tmp = (zTCPClientTaskThread *)taskThreads.getByIndex(i);
		//Zebra::logger->debug("%u",tmp->size());
		if (tmp && tmp->size() < connPerThread)
		{
			taskThread = tmp;
			break;
		}
	}
	if (NULL == taskThread)
		taskThread = newThread();
	if (taskThread)
	{
		taskThread->add(task);
		task->getNextState();
		return true;
	}
	else
	{
		Zebra::logger->fatal("zTCPClientTaskPool::addMain: ���ܵõ�һ�������߳�");
		return false;
	}
}

