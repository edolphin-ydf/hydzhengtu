#include "TCPTask.h"
#include <sstream>

int CTCPTaskPool::usleep_time=50000;                    /**< ѭ���ȴ�ʱ�� */
/*
���ӻ��մ����߳�,��ɾ���ڴ�ռ�֮ǰ��Ҫ��֤recycleConn����1
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
					//���մ�����ɿ����ͷ���Ӧ����Դ
					it = tasks.erase(it);
					if (task->isUnique())
						//����Ѿ�ͨ����Ψһ����֤,��ȫ��Ψһ������ɾ��
						task->uniqueRemove();
					task->getNextState();
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

		CThread::msleep(200);
	}

	//�������е�����
	fprintf(stderr,"zRecycleThread::final\n");
	for(it = tasks.begin(); it != tasks.end();)
	{
		//���մ�����ɿ����ͷ���Ӧ����Դ
		CTCPTask *task = *it;
		it = tasks.erase(it);
		if (task->isUnique())
			//����Ѿ�ͨ����Ψһ����֤,��ȫ��Ψһ������ɾ��
			task->uniqueRemove();
		task->getNextState();
		SAFE_DELETE(task);
	}
}


/*
�ȴ�������ָ֤��,��������֤
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
						CTCPTask *task = *it;
						int ret = task->WaitRecv( false );
						//printf("verify task size:%d,task:%s,ret:%d\n",tasks.size(),task->getIP(),ret);
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
									printf("�ͻ���Ψһ����֤�ɹ�\n");
									task->setUnique();
									pool->addSync(task);
								}
								else
								{
									//Ψһ����֤ʧ��,������������
									printf("�ͻ���Ψһ����֤ʧ��\n");
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

		CThread::msleep(50);
	}

	//�����еȴ���֤�����е����Ӽ��뵽���ն�����,������Щ����
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
�ȴ������߳�ͬ����֤�������
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
		CThread::msleep(200);
	}

	fprintf(stderr,"SyncThread::final\n");
	//�����еȴ�ͬ����֤�����е����Ӽ��뵽���ն�����,������Щ����
	for(it = tasks.begin(); it != tasks.end();)
	{
		CTCPTask *task = *it;
		it = tasks.erase(it);
		task->resetState();
		pool->addRecycle(task);
	}
}

/**
* \brief �������߳�,�ص��������ӵ��������ָ��
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
							//�׽ӿڳ��ִ���
							printf("OkayThread::run: �׽ӿ��쳣����\n");
							task->Terminate(CTCPTask::terminate_active);
						}
						else if( retcode > 0 )
						{
							//�׽ӿ�׼�����˶�ȡ����
							if (!task->ListeningRecv(true))
							{
								printf("OkayThread::run: �׽ӿڶ���������\n");
								task->Terminate(CTCPTask::terminate_active);
							}
						}
						retcode = task->WaitSend( false );
						if( retcode == -1 )
						{
							//�׽ӿڳ��ִ���
							printf("OkayThread::run: �׽ӿ��쳣����\n");
							task->Terminate(CTCPTask::terminate_active);
						}
						else if( retcode ==  1 )
						{
							//�׽ӿ�׼������д�����
							if (!task->ListeningSend())
							{
								printf("OkayThread::run: �׽ӿ�д�������� port = %u\n",task->getPort());

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

	//��������������е����Ӽ��뵽���ն�����,������Щ����
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
		* ������״̬���������,
		* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
		*/
		task->getNextState();
		pool->addRecycle(task);
	}
}



/**
* \brief �������ӳ��������Ӹ���
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
* \brief ��һ��TCP������ӵ���֤������,��Ϊ���ڶ����֤����,��Ҫ����һ�����㷨��ӵ���ͬ����֤���������
*
* \param task һ����������
*/
bool CTCPTaskPool::addVerify(CTCPTask *task)
{
	printf("CTCPTaskPool::addVerify\n");
	//��Ϊ���ڶ����֤����,��Ҫ����һ�����㷨��ӵ���ͬ����֤���������
	static DWORD hashcode = 0;
	VerifyThread *pVerifyThread = (VerifyThread *)verifyThreads.getByIndex(hashcode++ % maxVerifyThreads);
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
void CTCPTaskPool::addSync(CTCPTask *task)
{
	printf("CTCPTaskPool::addSync\n");
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
bool CTCPTaskPool::addOkay(CTCPTask *task)
{
	printf("CTCPTaskPool::addOkay\n");
	//���ȱ������е��߳�,�ҳ����еĲ������������ٵ��߳�,���ҳ�û���������߳�
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
			printf("CTCPTaskPool���������߳�\n");
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
			printf("CTCPTaskPool���ܴ��������߳�\n");
	}

	printf("CTCPTaskPoolû���ҵ����ʵ��߳�����������\n");
	//û���ҵ��߳��������������,��Ҫ���չر�����
	return false;
}

/**
* \brief ��һ��TCP������ӵ����մ��������
*
* \param task һ����������
*/

void CTCPTaskPool::addRecycle(CTCPTask *task)
{
	printf("CTCPTaskPool::addRecycle\n");
	
	recycleThread->add(task);
}


/**
* \brief ��ʼ���̳߳�,Ԥ�ȴ��������߳�
*
* \return ��ʼ���Ƿ�ɹ�
*/
bool CTCPTaskPool::init()
{
	printf("CTCPTaskPool::init\n");
	//������ʼ����֤�߳�
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

	//������ʼ���ȴ�ͬ����֤�߳�
	syncThread = new SyncThread(this);
	if (syncThread && !syncThread->start())
		return false;

	//������ʼ���������̳߳�
	maxThreadCount = (maxConns + COkayThread::connPerThread - 1) / COkayThread::connPerThread;
	printf("���TCP������%d,ÿ�߳�TCP������%d,�̸߳���%d\n",maxConns,COkayThread::connPerThread,maxThreadCount);
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

	//������ʼ�������̳߳�
	recycleThread = new RecycleThread(this);
	if (recycleThread && !recycleThread->start())
		return false;

	return true;
}

/**
* \brief �ͷ��̳߳�,�ͷŸ�����Դ,�ȴ������߳��˳�
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
* \brief ���׽ӿڷ���ָ��,��������־����,������ֱ�ӿ�����������������,ʵ�ʵķ��Ͷ���������һ���߳���
*
*
* \param pstrCmd �����͵�ָ��
* \param nCmdLen ������ָ��Ĵ�С
* \return �����Ƿ�ɹ�
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
* \brief ���׽ӿ��н�������,���Ҳ�����д���,�ڵ����������֮ǰ��֤�Ѿ����׽ӿڽ�������ѯ
*
* \param needRecv �Ƿ���Ҫ�������׽ӿڽ�������,false����Ҫ����,ֻ�Ǵ�������ʣ���ָ��,true��Ҫʵ�ʽ�������,Ȼ��Ŵ���
* \return �����Ƿ�ɹ�,true��ʾ���ճɹ�,false��ʾ����ʧ��,������Ҫ�Ͽ����� 
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
				//����ֻ�Ǵӻ���ȡ���ݰ�,���Բ������,û������ֱ�ӷ���
				break;
			else
			{
				Cmd::Cmd_NULL *pNullCmd = (Cmd::Cmd_NULL *)pstrCmd;
				if (Cmd::NULL_USERCMD == pNullCmd->cmd
					&& Cmd::NULL_PARA == pNullCmd->para)
				{
					//���صĲ���ָ��,��Ҫ�ݼ�����
					printf("������յ����ز����ź�\n");
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
* \brief ���ͻ����е����ݵ��׽ӿ�,�ٵ������֮ǰ��֤�Ѿ����׽ӿڽ�������ѯ
*
* \return �����Ƿ�ɹ�,true��ʾ���ͳɹ�,false��ʾ����ʧ��,������Ҫ�Ͽ�����
*/
bool CTCPTask::ListeningSend()
{
	printf("CTCPTask::ListeningSend\n");
	return mSocket->sync();
}

/**
* \brief ��TCP�������񽻸���һ���������,�л�״̬
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
* \brief ��ֵ��������״̬,��������
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
		* ���sync�������ӵ�okay������ʧ�ܻ����okay״̬resetState�Ŀ�����
		*/
		//case okay:
	case recycle:
		//�����ܵ�
		printf("CTCPTask::resetState:������ recycle -> recycle\n");
		break;
	case verify:
	case sync:
	case okay:
		//TODO ��ͬ�Ĵ���ʽ
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
			//�����ź���ָ��ʱ�䷶Χ��û�з���
			printf("�׽ӿڼ������ź�ʧ��\n");
			Terminate(CTCPTask::terminate_active);
		}
		else
		{
			//���Ͳ����ź�
			Cmd::Cmd_NULL tNullCmd;
			printf("����˷��Ͳ����ź�\n");
			if (sendCmd(&tNullCmd,sizeof(tNullCmd)))
				setTick();
		}
	}
}