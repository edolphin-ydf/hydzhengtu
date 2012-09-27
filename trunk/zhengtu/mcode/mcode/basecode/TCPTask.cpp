#include "TCPTask.h"
#include <sstream>

int CTCPTaskPool::usleep_time=50000;                    /**< 循环等待时间 */
/*
连接回收处理线程,在删除内存空间之前需要保证recycleConn返回1
*/
void RecycleThread::run()
{
	printf("RecycleThread::run\n");
	CTCPTask_IT it;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{		
		check_queue();

		DWORD dwLog = 0;
		int i;
		m_Lock.lock();
		if (!tasks.empty())
		{
			for(i = 0,it = tasks.begin(); it != tasks.end();i++)
			{
				CTCPTask *task = *it;
				switch(task->recycleConn())
				{
				case 1:
					//回收处理完成可以释放相应的资源
					it = tasks.erase(it);
					if (task->isUnique())
						//如果已经通过了唯一性验证,从全局唯一容器中删除
						task->uniqueRemove();
					task->getNextState();
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

		CThread::msleep(200);
	}

	//回收所有的连接
	fprintf(stderr,"zRecycleThread::final\n");
	for(it = tasks.begin(); it != tasks.end();)
	{
		//回收处理完成可以释放相应的资源
		CTCPTask *task = *it;
		it = tasks.erase(it);
		if (task->isUnique())
			//如果已经通过了唯一性验证,从全局唯一容器中删除
			task->uniqueRemove();
		task->getNextState();
		SAFE_DELETE(task);
	}
}


/*
等待接受验证指令,并进行验证
*/
void VerifyThread::run()
{
	printf("VerifyThread::run\n");

	RTime currentTime;
	CTCPTask_IT it,next;
	pollfdContainer::size_type i;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{
		currentTime.now();

		check_queue();
		{
			m_Lock.lock();
			if(!tasks.empty())
			{
				for(i = 0,it = tasks.begin();  it != tasks.end();)
				{
					CTCPTask *task = *it;
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
						CTCPTask *task = *it;
						int ret = task->WaitRecv( false );
						//printf("verify task size:%d,task:%s,ret:%d\n",tasks.size(),task->getIP(),ret);
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
									printf("客户端唯一性验证成功\n");
									task->setUnique();
									pool->addSync(task);
								}
								else
								{
									//唯一性验证失败,回收连接任务
									printf("客户端唯一性验证失败\n");
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

		CThread::msleep(50);
	}

	//把所有等待验证队列中的连接加入到回收队列中,回收这些连接
	fprintf(stderr,"VerifyThread::final\n");

	if(tasks.size() == 0)
		return;
	for(i = 0,it = tasks.begin(); it != tasks.end();)
	{
		CTCPTask *task = *it;
		task->resetState();
		pool->addRecycle(task);
	}
}

/**
等待其它线程同步验证这个连接
*/
void SyncThread::run()
{
	printf("SyncThread::run\n");
	CTCPTask_IT it;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{
		check_queue();

		m_Lock.lock();
		if (!tasks.empty())
		{
			for(it = tasks.begin(); it != tasks.end();)
			{
				CTCPTask *task = (*it);
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
		CThread::msleep(200);
	}

	fprintf(stderr,"SyncThread::final\n");
	//把所有等待同步验证队列中的连接加入到回收队列中,回收这些连接
	for(it = tasks.begin(); it != tasks.end();)
	{
		CTCPTask *task = *it;
		it = tasks.erase(it);
		task->resetState();
		pool->addRecycle(task);
	}
}

/**
* \brief 主处理线程,回调处理连接的输入输出指令
*
*/
void COkayThread::run()
{
	printf("COkayThread::run\n");

	RTime currentTime;
	CTCPTask_IT it,next;
	pollfdContainer::size_type i;

	int time = pool->usleep_time;
	pollfdContainer::iterator iter_r;
	pollfdContainer pfds_r;
	CTCPTaskContainer tasks_r;    
	bool check=false;
	DWORD dwBeginTime = 0;
	while(!isFinal())
	{
		currentTime.now();
		check_queue();
		if (check)
		{
			m_Lock.lock();
			if (!tasks.empty())
			{
				for(i = 0,it = tasks.begin(); it != tasks.end(); )
				{
					CTCPTask *task = *it;
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
				}
			}
			m_Lock.unlock();
			check=false;
		}
		CThread::usleep(time);
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
					CTCPTask *task = (*it);

					bool UseIocp = task->UseIocp();

					if( UseIocp )
					{ 
						int retcode = task->WaitRecv( false );
						if ( retcode == -1 )
						{
							//套接口出现错误
							printf("OkayThread::run: 套接口异常错误\n");
							task->Terminate(CTCPTask::terminate_active);
						}
						else if( retcode > 0 )
						{
							//套接口准备好了读取操作
							if (!task->ListeningRecv(true))
							{
								printf("OkayThread::run: 套接口读操作错误\n");
								task->Terminate(CTCPTask::terminate_active);
							}
						}
						retcode = task->WaitSend( false );
						if( retcode == -1 )
						{
							//套接口出现错误
							printf("OkayThread::run: 套接口异常错误\n");
							task->Terminate(CTCPTask::terminate_active);
						}
						else if( retcode ==  1 )
						{
							//套接口准备好了写入操作
							if (!task->ListeningSend())
							{
								printf("OkayThread::run: 套接口写操作错误 port = %u\n",task->getPort());

								task->Terminate(CTCPTask::terminate_active);
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
	fprintf(stderr,"OkayThread::final\n");

	if(tasks.size() == 0)
		return;

	for(i = 0,it = tasks.begin(); it != tasks.end();)
	{
		CTCPTask *task = *it;
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
* \brief 返回连接池中子连接个数
*
*/
const int CTCPTaskPool::getSize()
{
	printf("CTCPTaskPool::getSize\n");
	struct MyCallback : CThreadGroup::Callback
	{
		int size;
		MyCallback() : size(0) {}
		void exec(CThread *e)
		{
			COkayThread *pOkayThread = (COkayThread *)e;
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
bool CTCPTaskPool::addVerify(CTCPTask *task)
{
	printf("CTCPTaskPool::addVerify\n");
	//因为存在多个验证队列,需要按照一定的算法添加到不同的验证处理队列中
	static DWORD hashcode = 0;
	VerifyThread *pVerifyThread = (VerifyThread *)verifyThreads.getByIndex(hashcode++ % maxVerifyThreads);
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
void CTCPTaskPool::addSync(CTCPTask *task)
{
	printf("CTCPTaskPool::addSync\n");
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
bool CTCPTaskPool::addOkay(CTCPTask *task)
{
	printf("CTCPTaskPool::addOkay\n");
	//首先遍历所有的线程,找出运行的并且连接数最少的线程,再找出没有启动的线程
	COkayThread *pmin = NULL,*nostart = NULL;
	for(int i = 0; i < maxThreadCount; i++)
	{
		COkayThread *pOkayThread = (COkayThread *)okayThreads.getByIndex(i);
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
	if (pmin && pmin->size() < COkayThread::connPerThread)
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
			printf("CTCPTaskPool创建工作线程\n");
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
			printf("CTCPTaskPool不能创建工作线程\n");
	}

	printf("CTCPTaskPool没有找到合适的线程来处理连接\n");
	//没有找到线程来处理这个连接,需要回收关闭连接
	return false;
}

/**
* \brief 把一个TCP连接添加到回收处理队列中
*
* \param task 一个连接任务
*/

void CTCPTaskPool::addRecycle(CTCPTask *task)
{
	printf("CTCPTaskPool::addRecycle\n");
	
	recycleThread->add(task);
}


/**
* \brief 初始化线程池,预先创建各种线程
*
* \return 初始化是否成功
*/
bool CTCPTaskPool::init()
{
	printf("CTCPTaskPool::init\n");
	//创建初始化验证线程
	for(int i = 0; i < maxVerifyThreads; i++)
	{
		std::ostringstream name;
		name << "VerifyThread[" << i << "]";
		VerifyThread *pVerifyThread = new VerifyThread(this,name.str());
		if (NULL == pVerifyThread)
			return false;
		if (!pVerifyThread->start())
			return false;
		verifyThreads.add(pVerifyThread);
	}

	//创建初始化等待同步验证线程
	syncThread = new SyncThread(this);
	if (syncThread && !syncThread->start())
		return false;

	//创建初始化主运行线程池
	maxThreadCount = (maxConns + COkayThread::connPerThread - 1) / COkayThread::connPerThread;
	printf("最大TCP连接数%d,每线程TCP连接数%d,线程个数%d\n",maxConns,COkayThread::connPerThread,maxThreadCount);
	for(int i = 0; i < maxThreadCount; i++)
	{
		std::ostringstream name;
		name << "OkayThread[" << i << "]";
		COkayThread *pOkayThread = new COkayThread(this,name.str());
		if (NULL == pOkayThread)
			return false;
		if (i < minThreadCount && !pOkayThread->start())
			return false;
		okayThreads.add(pOkayThread);
	}

	//创建初始化回收线程池
	recycleThread = new RecycleThread(this);
	if (recycleThread && !recycleThread->start())
		return false;

	return true;
}

/**
* \brief 释放线程池,释放各种资源,等待各种线程退出
*
*/
void CTCPTaskPool::final()
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


/**
* \brief 向套接口发送指令,如果缓冲标志设置,则发送是直接拷贝到缓冲区队列中,实际的发送动作在另外一个线程做
*
*
* \param pstrCmd 待发送的指令
* \param nCmdLen 待发送指令的大小
* \return 发送是否成功
*/
bool CTCPTask::sendCmd(const void *pstrCmd,int nCmdLen)
{
	return mSocket->sendCmd(pstrCmd,nCmdLen,buffered);
}

bool CTCPTask::sendCmdNoPack(const void *pstrCmd,int nCmdLen)
{
	printf("CTCPTask::sendCmdNoPack\n");
	return mSocket->sendCmdNoPack(pstrCmd,nCmdLen,buffered);
}

/**
* \brief 从套接口中接受数据,并且拆包进行处理,在调用这个函数之前保证已经对套接口进行了轮询
*
* \param needRecv 是否需要真正从套接口接受数据,false则不需要接收,只是处理缓冲中剩余的指令,true需要实际接收数据,然后才处理
* \return 接收是否成功,true表示接收成功,false表示接收失败,可能需要断开连接 
*/
bool CTCPTask::ListeningRecv(bool needRecv)
{
	printf("CTCPTask::ListeningRecv\n");

	int retcode = 0;
	if (needRecv) {
		retcode = mSocket->recvToBuf_NoPoll();
	}
	//struct timeval tv_2;
	if (-1 == retcode)
	{
		printf("CTCPTask::ListeningRecv -1\n");  
		printf("CTCPTask::remoteport= %u localport = %u",mSocket->getPort(),mSocket->getLocalPort());
		return false;
	}
	else
	{
		do
		{
			BYTE pstrCmd[CSocket::MAX_DATASIZE];
			int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
			if (nCmdLen <= 0)
				//这里只是从缓冲取数据包,所以不会出错,没有数据直接返回
				break;
			else
			{
				Cmd::Cmd_NULL *pNullCmd = (Cmd::Cmd_NULL *)pstrCmd;
				if (Cmd::NULL_USERCMD == pNullCmd->cmd
					&& Cmd::NULL_PARA == pNullCmd->para)
				{
					//返回的测试指令,需要递减计数
					printf("服务端收到返回测试信号\n");
					clearTick();
				}
				else
				{
					msgParse(pNullCmd,nCmdLen);
				}
			}
		}
		while(true);
	}

	return true;
}

/**
* \brief 发送缓冲中的数据到套接口,再调用这个之前保证已经对套接口进行了轮询
*
* \return 发送是否成功,true表示发送成功,false表示发送失败,可能需要断开连接
*/
bool CTCPTask::ListeningSend()
{
	printf("CTCPTask::ListeningSend\n");
	return mSocket->sync();
}

/**
* \brief 把TCP连接任务交给下一个任务队列,切换状态
*
*/
void CTCPTask::getNextState()
{
	printf("CTCPTask::getNextState()\n");
	CTCPTask_State old_state = getState();

	switch(old_state)
	{
	case notuse:
		setState(verify);
		break;
	case verify:
		setState(sync);
		break;
	case sync:
		buffered = true;
		addToContainer();
		setState(okay);
		break;
	case okay:
		removeFromContainer();
		setState(recycle);
		break;
	case recycle:
		setState(notuse);
		break;
	}

	printf("CTCPTask::getNextState(%s:%u),%s -> %s)\n",getIP(),getPort(),getStateString(old_state),getStateString(getState()));
}

/**
* \brief 重值连接任务状态,回收连接
*
*/
void CTCPTask::resetState()
{
	printf("CTCPTask::resetState\n");
	CTCPTask_State old_state = getState();

	switch(old_state)
	{
	case notuse:
		/*
		* whj 
		* 如果sync情况下添加到okay管理器失败会出现okay状态resetState的可能性
		*/
		//case okay:
	case recycle:
		//不可能的
		printf("CTCPTask::resetState:不可能 recycle -> recycle\n");
		break;
	case verify:
	case sync:
	case okay:
		//TODO 相同的处理方式
		break;
	}

	setState(recycle);
	printf("CTCPTask::resetState(%s:%u),%s -> %s)\n",getIP(),getPort(),getStateString(old_state),getStateString(getState()));
}

void CTCPTask::checkSignal(const RTime &ct)
{
	printf("CTCPTask::checkSignal\n");
	if (ifCheckSignal() && checkInterval(ct))
	{
		if (checkTick())
		{
			//测试信号在指定时间范围内没有返回
			printf("套接口检查测试信号失败\n");
			Terminate(CTCPTask::terminate_active);
		}
		else
		{
			//发送测试信号
			Cmd::Cmd_NULL tNullCmd;
			printf("服务端发送测试信号\n");
			if (sendCmd(&tNullCmd,sizeof(tNullCmd)))
				setTick();
		}
	}
}