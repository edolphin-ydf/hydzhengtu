/**
* \brief 实现类zTCPClient,TCP连接客户端。
*
*/
#include <zebra/srvEngine.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

CmdAnalysis zTCPClient::analysis("Client指令发送统计",600);
/**
* \brief 向套接口发送指令
*
*
* \param pstrCmd 待发送的指令
* \param nCmdLen 待发送指令的大小
* \return 发送是否成功
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
* \brief 重载zThread中的纯虚函数,是线程的主回调函数,用于处理接收到的指令
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
			Zebra::logger->error("error::remoteport= %u localport = %u",pSocket->getPort(),pSocket->getLocalPort());

			Zebra::logger->error("接收指令失败1,关闭 %s",getThreadName().c_str());
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
		//这里只是从缓冲取数据包,所以不会出错,没有数据直接返回
		if (nCmdLen <= 0) break;
		else
		{
			Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
			if (Cmd::CMD_NULL == pNullCmd->cmd
				&& Cmd::PARA_NULL == pNullCmd->para)
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
		//接收指令到缓冲区,当套接口数据没有准备好的时候,不等待
		nCmdLen = pSocket->recvToCmd(pstrCmd,zSocket::MAX_DATASIZE,false);
		if (nCmdLen > 0)//接收成功
		{
			Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;//转换成空操作指令，测试信号和对时间指令
			if (Cmd::CMD_NULL == pNullCmd->cmd
				&& Cmd::PARA_NULL == pNullCmd->para)//判断是否为测试信号
			{
				Zebra::logger->debug("客户端%s收到测试信号",getThreadName().c_str());
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
			Zebra::logger->error("接收指令失败2,关闭 %s",getThreadName().c_str());
			break;
		}
		else if (0 == nCmdLen)
		{
			//接收指令超时,退出循环,结束线程
			Zebra::logger->error("接收指令失败2,关闭 %s",getThreadName().c_str());
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
					//套接口出现错误
					Zebra::logger->fatal("zTCPBufferClient::run: 套接口错误");
					break;
				}
				else
				{
					if ( retcode > 0 )
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							Zebra::logger->debug("zTCPBufferClient::run: 套接口读操作错误");
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
					Zebra::logger->fatal("zTCPBufferClient::run: 套接口错误");
					break;
				}
				else
				{
					if (pfds_r.revents & POLLIN)
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							Zebra::logger->debug("zTCPBufferClient::run: 套接口读操作错误");
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
					Zebra::logger->fatal("zTCPBufferClient::run: 套接口错误");
					break;
				}
				else
				{
					if( retcode > 0 )
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							Zebra::logger->debug("zTCPBufferClient::run: 套接口读操作错误");
							break;
						}
					}
				}
				retcode = pSocket->WaitSend( false );
				if( retcode == -1 )
				{
					//套接口出现错误
					Zebra::logger->fatal("zTCPBufferClient::run: 套接口错误");
					break;
				}
				else if( retcode == 1 )
				{
					//套接口准备好了写入操作
					if (!ListeningSend())
					{
						Zebra::logger->debug("zTCPBufferClient::run: 套接口写操作错误");
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
						Zebra::logger->fatal("zTCPBufferClient::run: 套接口错误");
						break;
					}
					else
					{
						if (pfds.revents & POLLIN)
						{
							//套接口准备好了读取操作
							if (!ListeningRecv())
							{
								Zebra::logger->debug("zTCPBufferClient::run: 套接口读操作错误");
								break;
							}
						}
						if (pfds.revents & POLLOUT)
						{
							//套接口准备好了写入操作
							if (!ListeningSend())
							{
								Zebra::logger->debug("zTCPBufferClient::run: 套接口写操作错误");
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


/**
* \brief 建立一个到服务器的TCP连接
*
*
* \return 连接是否成功
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
		Zebra::logger->error("创建套接口失败: %s-%ld",strerror(errno),WSAGetLastError());
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

	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	retcode = ::connect(nSocket,(struct sockaddr *) &addr,sizeof(addr));
	if (0 != retcode)
	{
		Zebra::logger->error("创建到服务器(%s:%u) 的连接失败",ip.c_str(),port);
		::closesocket(nSocket);
		return false;
	}
	pSocket = new zSocket(nSocket,&addr,compress);
	if (NULL == pSocket)
	{
		Zebra::logger->fatal("没有足够的内存,不能创建zSocket实例");
		::closesocket(nSocket);
		return false;
	}

	Zebra::logger->info("创建到服务器(%s:%u)的连接成功",ip.c_str(),port);

	return true;
}