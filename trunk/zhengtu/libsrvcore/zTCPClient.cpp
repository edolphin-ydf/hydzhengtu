/**
* \brief ʵ����zTCPClient,TCP���ӿͻ��ˡ�
*
*/
#include <zebra/srvEngine.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

CmdAnalysis zTCPClient::analysis("Clientָ���ͳ��",600);
/**
* \brief ���׽ӿڷ���ָ��
*
*
* \param pstrCmd �����͵�ָ��
* \param nCmdLen ������ָ��Ĵ�С
* \return �����Ƿ�ɹ�
*/
bool zTCPClient::sendCmd(const void *pstrCmd,const int nCmdLen)
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPClient::sendCmd");
#endif //_DEBUG
	if (NULL == pSocket) 
		return false;
	else
	{
		/*
		Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
		analysis.add(pNullCmd->cmd,pNullCmd->para,nCmdLen);
		// */
		return pSocket->sendCmd(pstrCmd,nCmdLen);
	}
}
/**
* \brief ����zThread�еĴ��麯��,���̵߳����ص�����,���ڴ�����յ���ָ��
*
*/
void zTCPClient::run()
{
#ifdef _DEBUG
	Zebra::logger->error("zTCPClient::remoteport= %u localport = %u",pSocket->getPort(),pSocket->getLocalPort());

#endif //_DEBUG
	while(!isFinal())
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen;

		nCmdLen = pSocket->recvToCmd(pstrCmd,zSocket::MAX_DATASIZE,false);
		if (nCmdLen > 0) 
		{
			Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
			if (Cmd::CMD_NULL == pNullCmd->cmd
				&& Cmd::PARA_NULL == pNullCmd->para)
			{
				//Zebra::logger->debug("�ͻ����յ������ź�");
				if (!sendCmd(pstrCmd,nCmdLen))
				{
					//����ָ��ʧ��,�˳�ѭ��,�����߳�
					break;
				}
			}
			else
				msgParse(pNullCmd,nCmdLen);
		}
		else if (-1 == nCmdLen)
		{
			//����ָ��ʧ��,�˳�ѭ��,�����߳�
			Zebra::logger->error("error::remoteport= %u localport = %u",pSocket->getPort(),pSocket->getLocalPort());

			Zebra::logger->error("����ָ��ʧ��1,�ر� %s",getThreadName().c_str());
			break;
		}
	}
}

bool zTCPBufferClient::sendCmd(const void *pstrCmd,const int nCmdLen)
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::sendCmd");
#endif //_DEBUG
	if (pSocket)
		return pSocket->sendCmd(pstrCmd,nCmdLen,_buffered);
	else
		return false;
}

bool zTCPBufferClient::ListeningRecv()
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::ListeningRecv");
#endif //_DEBUG
	int retcode = pSocket->recvToBuf_NoPoll();
	if (-1 == retcode) return false;
	for(;;)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = pSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
		//����ֻ�Ǵӻ���ȡ���ݰ�,���Բ������,û������ֱ�ӷ���
		if (nCmdLen <= 0) break;
		else
		{
			Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
			if (Cmd::CMD_NULL == pNullCmd->cmd
				&& Cmd::PARA_NULL == pNullCmd->para)
			{
				//Zebra::logger->debug("�ͻ����յ������ź�");
				//����ָ��ʧ��,�˳�ѭ��,�����߳�
				if (!sendCmd(pstrCmd,nCmdLen)) return false;
			}
			else msgParse(pNullCmd,nCmdLen);
		}
	}
	return true;
}

bool zTCPBufferClient::ListeningSend()
{  
#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::ListeningSend");
#endif //_DEBUG
	if (pSocket)
		return pSocket->sync();
	else
		return false;
}

void zTCPBufferClient::sync()
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::sync");
#endif //_DEBUG
	if (pSocket)
		pSocket->force_sync();
}

void zTCPBufferClient::run()
{  
	while(!isFinal())
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen;
		//����ָ�������,���׽ӿ�����û��׼���õ�ʱ��,���ȴ�
		nCmdLen = pSocket->recvToCmd(pstrCmd,zSocket::MAX_DATASIZE,false);
		if (nCmdLen > 0)//���ճɹ�
		{
			Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;//ת���ɿղ���ָ������źźͶ�ʱ��ָ��
			if (Cmd::CMD_NULL == pNullCmd->cmd
				&& Cmd::PARA_NULL == pNullCmd->para)//�ж��Ƿ�Ϊ�����ź�
			{
				Zebra::logger->debug("�ͻ���%s�յ������ź�",getThreadName().c_str());
				if (!sendCmd(pstrCmd,nCmdLen))//�Ѳ����źŷ���ȥ
				{
					//����ָ��ʧ��,�˳�ѭ��,�����߳�
					break;
				}
			}
			else
				msgParse(pNullCmd,nCmdLen);//���ǲ����źţ�����н���
		}
		else if (-1 == nCmdLen)
		{
			//����ָ��ʧ��,�˳�ѭ��,�����߳�
			Zebra::logger->error("����ָ��ʧ��2,�ر� %s",getThreadName().c_str());
			break;
		}
		else if (0 == nCmdLen)
		{
			//����ָ�ʱ,�˳�ѭ��,�����߳�
			Zebra::logger->error("����ָ��ʧ��2,�ر� %s",getThreadName().c_str());
			break;
		}
	}


#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::run(POLL)");
#endif //_DEBUG

	_buffered = true;
	struct mypollfd pfds;
	struct mypollfd pfds_r;
	pSocket->fillPollFD(pfds,POLLIN | POLLOUT | POLLPRI);
	pSocket->fillPollFD(pfds_r,POLLIN | POLLPRI);
	int time=usleep_time;
	while(!isFinal())
	{
		struct timeval _tv_1;
		struct timeval _tv_2;
		gettimeofday(&_tv_1,NULL);

		int retcode;

		if( pSocket->m_bUseIocp )
		{
			retcode = pSocket->WaitRecv( true, time / 1000 );
			if ( retcode != 0)
			{
				if ( retcode == -1 )
				{
					//�׽ӿڳ��ִ���
					Zebra::logger->fatal("zTCPBufferClient::run: �׽ӿڴ���");
					break;
				}
				else
				{
					if ( retcode > 0 )
					{
						//�׽ӿ�׼�����˶�ȡ����
						if (!ListeningRecv())
						{
							Zebra::logger->debug("zTCPBufferClient::run: �׽ӿڶ���������");
							break;
						}
					}
				}
			}
		}
		else
		{
			retcode = ::poll(&pfds_r,1,time/1000);
			if ( retcode > 0)
			{
				if (pfds_r.revents & POLLPRI)
				{
					//�׽ӿڳ��ִ���
					Zebra::logger->fatal("zTCPBufferClient::run: �׽ӿڴ���");
					break;
				}
				else
				{
					if (pfds_r.revents & POLLIN)
					{
						//�׽ӿ�׼�����˶�ȡ����
						if (!ListeningRecv())
						{
							Zebra::logger->debug("zTCPBufferClient::run: �׽ӿڶ���������");
							break;
						}
					}
				}
			}
		}
		gettimeofday(&_tv_2,NULL);
		int end=_tv_2.tv_sec*1000000 + _tv_2.tv_usec;
		int begin= _tv_1.tv_sec*1000000 + _tv_1.tv_usec;
		time = time - (end - begin);
		if (time <= 0)
		{
			int retcode;
			if( pSocket->m_bUseIocp )
			{
				retcode = pSocket->WaitRecv( false );
				if( retcode == -1 )
				{
					//�׽ӿڳ��ִ���
					Zebra::logger->fatal("zTCPBufferClient::run: �׽ӿڴ���");
					break;
				}
				else
				{
					if( retcode > 0 )
					{
						//�׽ӿ�׼�����˶�ȡ����
						if (!ListeningRecv())
						{
							Zebra::logger->debug("zTCPBufferClient::run: �׽ӿڶ���������");
							break;
						}
					}
				}
				retcode = pSocket->WaitSend( false );
				if( retcode == -1 )
				{
					//�׽ӿڳ��ִ���
					Zebra::logger->fatal("zTCPBufferClient::run: �׽ӿڴ���");
					break;
				}
				else if( retcode == 1 )
				{
					//�׽ӿ�׼������д�����
					if (!ListeningSend())
					{
						Zebra::logger->debug("zTCPBufferClient::run: �׽ӿ�д��������");
						break;
					}
				}
			}
			else
			{
				if (::poll(&pfds,1,0) > 0)
				{
					if (pfds.revents & POLLPRI)
					{
						//�׽ӿڳ��ִ���
						Zebra::logger->fatal("zTCPBufferClient::run: �׽ӿڴ���");
						break;
					}
					else
					{
						if (pfds.revents & POLLIN)
						{
							//�׽ӿ�׼�����˶�ȡ����
							if (!ListeningRecv())
							{
								Zebra::logger->debug("zTCPBufferClient::run: �׽ӿڶ���������");
								break;
							}
						}
						if (pfds.revents & POLLOUT)
						{
							//�׽ӿ�׼������д�����
							if (!ListeningSend())
							{
								Zebra::logger->debug("zTCPBufferClient::run: �׽ӿ�д��������");
								break;
							}
						}
					}
				}
			}
			time = usleep_time;
		}
		//zThread::usleep(usleep_time);
	}

	//��֤��������ݷ������
	sync();
	_buffered = false;
}


/**
* \brief ����һ������������TCP����
*
*
* \return �����Ƿ�ɹ�
*/

bool zTCPClient::connect()
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPClient::connect");
#endif //_DEBUG
	int retcode;
	int nSocket;
	struct sockaddr_in addr;

	nSocket = ::socket(PF_INET,SOCK_STREAM,0);
	if (INVALID_SOCKET == nSocket)
	{
		Zebra::logger->error("�����׽ӿ�ʧ��: %s-%ld",strerror(errno),WSAGetLastError());
		return false;
	}

	//�����׽ӿڷ��ͽ��ջ���,���ҿͻ��˵ı�����connect֮ǰ����
	int window_size = 128 * 1024;
	retcode = ::setsockopt(nSocket,SOL_SOCKET,SO_RCVBUF,(char*)&window_size,sizeof(window_size));
	if (0 != retcode)
	{
		::closesocket(nSocket);
		return false;
	}
	retcode = ::setsockopt(nSocket,SOL_SOCKET,SO_SNDBUF,(char*)&window_size,sizeof(window_size));
	if (0 != retcode)
	{
		::closesocket(nSocket);
		return false;
	}

	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	retcode = ::connect(nSocket,(struct sockaddr *) &addr,sizeof(addr));
	if (0 != retcode)
	{
		Zebra::logger->error("������������(%s:%u) ������ʧ��",ip.c_str(),port);
		::closesocket(nSocket);
		return false;
	}
	pSocket = new zSocket(nSocket,&addr,compress);
	if (NULL == pSocket)
	{
		Zebra::logger->fatal("û���㹻���ڴ�,���ܴ���zSocketʵ��");
		::closesocket(nSocket);
		return false;
	}

	Zebra::logger->info("������������(%s:%u)�����ӳɹ�",ip.c_str(),port);

	return true;
}