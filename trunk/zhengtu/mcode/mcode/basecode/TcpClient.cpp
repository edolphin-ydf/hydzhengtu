#include "TcpClient.h"

CmdAnalysis CTCPClient::analysis("Clientָ���ͳ��\n",600);

/**
* \brief ���׽ӿڷ���ָ��
*
*
* \param pstrCmd �����͵�ָ��
* \param nCmdLen ������ָ��Ĵ�С
* \return �����Ƿ�ɹ�
*/
bool CTCPClient::sendCmd(const void *pstrCmd,const int nCmdLen)
{
#ifdef _DEBUG
	printf("zTCPClient::sendCmd\n");
#endif //_DEBUG
	if (NULL == pSocket) 
		return false;
	else
	{
		return pSocket->sendCmd(pstrCmd,nCmdLen);
	}
}

/**
* \brief ����һ������������TCP����
*
*
* \return �����Ƿ�ɹ�
*/

bool CTCPClient::connect()
{
#ifdef _DEBUG
	printf("=%s=TCPClient::connect",threadName.c_str());
#endif //_DEBUG
	int retcode;
	int nSocket;
	struct sockaddr_in addr;

	nSocket = ::socket(PF_INET,SOCK_STREAM,0);
	if (INVALID_SOCKET == nSocket)
	{
		printf("�����׽ӿ�ʧ��: %s-%ld\n",strerror(errno),WSAGetLastError());
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

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	retcode = ::connect(nSocket,(struct sockaddr *) &addr,sizeof(addr));
	if (0 != retcode)
	{
		printf("������������(%s:%u) ������ʧ��\n",ip.c_str(),port);
		::closesocket(nSocket);
		return false;
	}
	pSocket = new CSocket(nSocket,&addr,compress,false);
	if (NULL == pSocket)
	{
		printf("û���㹻���ڴ�,���ܴ���zSocketʵ��\n");
		::closesocket(nSocket);
		return false;
	}

	printf("������������(%s:%u)�����ӳɹ�\n",ip.c_str(),port);

	return true;
}

/**
* \brief ����zThread�еĴ��麯��,���̵߳����ص�����,���ڴ�����յ���ָ��
*
*/
void CTCPClient::run()
{
	if (pSocket == NULL)
	{
		return;
	}
#ifdef _DEBUG
	printf("TCPClient::remoteport= %u localport = %u\n",pSocket->getPort(),pSocket->getLocalPort());

#endif //_DEBUG
	while(!isFinal())
	{
		BYTE pstrCmd[CSocket::MAX_DATASIZE];
		int nCmdLen;

		nCmdLen = pSocket->recvToCmd(pstrCmd,CSocket::MAX_DATASIZE,false);
		if (nCmdLen > 0) 
		{
			Cmd::Cmd_NULL *pNullCmd = (Cmd::Cmd_NULL *)pstrCmd;
			if (Cmd::NULL_USERCMD == pNullCmd->cmd
				&& Cmd::NULL_PARA == pNullCmd->para)
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
			printf("error::remoteport= %u localport = %u\n",pSocket->getPort(),pSocket->getLocalPort());

			printf("����ָ��ʧ��1,�ر� %s\n",getThreadName().c_str());
			break;
		}
	}
}


bool CTCPBufferClient::sendCmd(const void *pstrCmd,const int nCmdLen)
{
#ifdef _DEBUG
	printf("CTCPBufferClient::sendCmd\n");
#endif //_DEBUG
	if (pSocket)
		return pSocket->sendCmd(pstrCmd,nCmdLen,_buffered);
	else
		return false;
}

bool CTCPBufferClient::ListeningRecv()
{
#ifdef _DEBUG
	printf("CTCPBufferClient::ListeningRecv\n");
#endif //_DEBUG
	int retcode = pSocket->recvToBuf_NoPoll();
	if (-1 == retcode) return false;
	for(;;)
	{
		BYTE pstrCmd[CSocket::MAX_DATASIZE];
		int nCmdLen = pSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
		//����ֻ�Ǵӻ���ȡ���ݰ�,���Բ������,û������ֱ�ӷ���
		if (nCmdLen <= 0) break;
		else
		{
			Cmd::Cmd_NULL *pNullCmd = (Cmd::Cmd_NULL *)pstrCmd;
			if (Cmd::NULL_USERCMD == pNullCmd->cmd
				&& Cmd::NULL_PARA == pNullCmd->para)
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

bool CTCPBufferClient::ListeningSend()
{  
#ifdef _DEBUG
	printf("CTCPBufferClient::ListeningSend\n");
#endif //_DEBUG
	if (pSocket)
		return pSocket->sync();
	else
		return false;
}

void CTCPBufferClient::sync()
{
#ifdef _DEBUG
	printf("CTCPBufferClient::sync\n");
#endif //_DEBUG
	if (pSocket)
		pSocket->force_sync();
}

void CTCPBufferClient::run()
{  
	while(!isFinal())
	{
		BYTE pstrCmd[CSocket::MAX_DATASIZE];
		int nCmdLen;
		//����ָ�������,���׽ӿ�����û��׼���õ�ʱ��,���ȴ�
		nCmdLen = pSocket->recvToCmd(pstrCmd,CSocket::MAX_DATASIZE,false);
		if (nCmdLen > 0)//���ճɹ�
		{
			Cmd::Cmd_NULL *pNullCmd = (Cmd::Cmd_NULL *)pstrCmd;//ת���ɿղ���ָ������źźͶ�ʱ��ָ��
			if (Cmd::NULL_USERCMD == pNullCmd->cmd
				&& Cmd::NULL_PARA == pNullCmd->para)//�ж��Ƿ�Ϊ�����ź�
			{
				printf("�ͻ���%s�յ������ź�\n",getThreadName().c_str());
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
			printf("����ָ��ʧ��2,�ر� %s\n",getThreadName().c_str());
			break;
		}
		else if (0 == nCmdLen)
		{
			//����ָ�ʱ,�˳�ѭ��,�����߳�
			printf("����ָ��ʧ��2,�ر� %s\n",getThreadName().c_str());
			break;
		}
	}


#ifdef _DEBUG
	printf("CTCPBufferClient::run(POLL)");
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
					printf("CTCPBufferClient::run: �׽ӿڴ���\n");
					break;
				}
				else
				{
					if ( retcode > 0 )
					{
						//�׽ӿ�׼�����˶�ȡ����
						if (!ListeningRecv())
						{
							printf("CTCPBufferClient::run: �׽ӿڶ���������\n");
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
					printf("CTCPBufferClient::run: �׽ӿڴ���\n");
					break;
				}
				else
				{
					if (pfds_r.revents & POLLIN)
					{
						//�׽ӿ�׼�����˶�ȡ����
						if (!ListeningRecv())
						{
							printf("CTCPBufferClient::run: �׽ӿڶ���������\n");
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
					printf("CTCPBufferClient::run: �׽ӿڴ���\n");
					break;
				}
				else
				{
					if( retcode > 0 )
					{
						//�׽ӿ�׼�����˶�ȡ����
						if (!ListeningRecv())
						{
							printf("CTCPBufferClient::run: �׽ӿڶ���������\n");
							break;
						}
					}
				}
				retcode = pSocket->WaitSend( false );
				if( retcode == -1 )
				{
					//�׽ӿڳ��ִ���
					printf("CTCPBufferClient::run: �׽ӿڴ���\n");
					break;
				}
				else if( retcode == 1 )
				{
					//�׽ӿ�׼������д�����
					if (!ListeningSend())
					{
						printf("CTCPBufferClient::run: �׽ӿ�д��������\n");
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
						printf("CTCPBufferClient::run: �׽ӿڴ���\n");
						break;
					}
					else
					{
						if (pfds.revents & POLLIN)
						{
							//�׽ӿ�׼�����˶�ȡ����
							if (!ListeningRecv())
							{
								printf("CTCPBufferClient::run: �׽ӿڶ���������\n");
								break;
							}
						}
						if (pfds.revents & POLLOUT)
						{
							//�׽ӿ�׼������д�����
							if (!ListeningSend())
							{
								printf("CTCPBufferClient::run: �׽ӿ�д��������\n");
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