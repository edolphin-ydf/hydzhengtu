#include "TcpClient.h"

CmdAnalysis CTCPClient::analysis("Client指令发送统计\n",600);

/**
* \brief 向套接口发送指令
*
*
* \param pstrCmd 待发送的指令
* \param nCmdLen 待发送指令的大小
* \return 发送是否成功
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
* \brief 建立一个到服务器的TCP连接
*
*
* \return 连接是否成功
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
		printf("创建套接口失败: %s-%ld\n",strerror(errno),WSAGetLastError());
		return false;
	}

	//设置套接口发送接收缓冲,并且客户端的必须在connect之前设置
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
		printf("创建到服务器(%s:%u) 的连接失败\n",ip.c_str(),port);
		::closesocket(nSocket);
		return false;
	}
	pSocket = new CSocket(nSocket,&addr,compress,false);
	if (NULL == pSocket)
	{
		printf("没有足够的内存,不能创建zSocket实例\n");
		::closesocket(nSocket);
		return false;
	}

	printf("创建到服务器(%s:%u)的连接成功\n",ip.c_str(),port);

	return true;
}

/**
* \brief 重载zThread中的纯虚函数,是线程的主回调函数,用于处理接收到的指令
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
				//Zebra::logger->debug("客户端收到测试信号");
				if (!sendCmd(pstrCmd,nCmdLen))
				{
					//发送指令失败,退出循环,结束线程
					break;
				}
			}
			else
				msgParse(pNullCmd,nCmdLen);
		}
		else if (-1 == nCmdLen)
		{
			//接收指令失败,退出循环,结束线程
			printf("error::remoteport= %u localport = %u\n",pSocket->getPort(),pSocket->getLocalPort());

			printf("接收指令失败1,关闭 %s\n",getThreadName().c_str());
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
		//这里只是从缓冲取数据包,所以不会出错,没有数据直接返回
		if (nCmdLen <= 0) break;
		else
		{
			Cmd::Cmd_NULL *pNullCmd = (Cmd::Cmd_NULL *)pstrCmd;
			if (Cmd::NULL_USERCMD == pNullCmd->cmd
				&& Cmd::NULL_PARA == pNullCmd->para)
			{
				//Zebra::logger->debug("客户端收到测试信号");
				//发送指令失败,退出循环,结束线程
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
		//接收指令到缓冲区,当套接口数据没有准备好的时候,不等待
		nCmdLen = pSocket->recvToCmd(pstrCmd,CSocket::MAX_DATASIZE,false);
		if (nCmdLen > 0)//接收成功
		{
			Cmd::Cmd_NULL *pNullCmd = (Cmd::Cmd_NULL *)pstrCmd;//转换成空操作指令，测试信号和对时间指令
			if (Cmd::NULL_USERCMD == pNullCmd->cmd
				&& Cmd::NULL_PARA == pNullCmd->para)//判断是否为测试信号
			{
				printf("客户端%s收到测试信号\n",getThreadName().c_str());
				if (!sendCmd(pstrCmd,nCmdLen))//把测试信号发回去
				{
					//发送指令失败,退出循环,结束线程
					break;
				}
			}
			else
				msgParse(pNullCmd,nCmdLen);//不是测试信号，则进行解析
		}
		else if (-1 == nCmdLen)
		{
			//接收指令失败,退出循环,结束线程
			printf("接收指令失败2,关闭 %s\n",getThreadName().c_str());
			break;
		}
		else if (0 == nCmdLen)
		{
			//接收指令超时,退出循环,结束线程
			printf("接收指令失败2,关闭 %s\n",getThreadName().c_str());
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
					//套接口出现错误
					printf("CTCPBufferClient::run: 套接口错误\n");
					break;
				}
				else
				{
					if ( retcode > 0 )
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							printf("CTCPBufferClient::run: 套接口读操作错误\n");
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
					//套接口出现错误
					printf("CTCPBufferClient::run: 套接口错误\n");
					break;
				}
				else
				{
					if (pfds_r.revents & POLLIN)
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							printf("CTCPBufferClient::run: 套接口读操作错误\n");
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
					//套接口出现错误
					printf("CTCPBufferClient::run: 套接口错误\n");
					break;
				}
				else
				{
					if( retcode > 0 )
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							printf("CTCPBufferClient::run: 套接口读操作错误\n");
							break;
						}
					}
				}
				retcode = pSocket->WaitSend( false );
				if( retcode == -1 )
				{
					//套接口出现错误
					printf("CTCPBufferClient::run: 套接口错误\n");
					break;
				}
				else if( retcode == 1 )
				{
					//套接口准备好了写入操作
					if (!ListeningSend())
					{
						printf("CTCPBufferClient::run: 套接口写操作错误\n");
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
						//套接口出现错误
						printf("CTCPBufferClient::run: 套接口错误\n");
						break;
					}
					else
					{
						if (pfds.revents & POLLIN)
						{
							//套接口准备好了读取操作
							if (!ListeningRecv())
							{
								printf("CTCPBufferClient::run: 套接口读操作错误\n");
								break;
							}
						}
						if (pfds.revents & POLLOUT)
						{
							//套接口准备好了写入操作
							if (!ListeningSend())
							{
								printf("CTCPBufferClient::run: 套接口写操作错误\n");
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

	//保证缓冲的数据发送完成
	sync();
	_buffered = false;
}