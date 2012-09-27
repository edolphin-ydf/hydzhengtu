#include   <winsock2.h>   
#include   <stdio.h>   

#define   PORT   10000   
#define   DATA_BUFSIZE   8192   

typedef   struct   
{   
	OVERLAPPED   Overlapped;   
	WSABUF   DataBuf;   
	CHAR   Buffer[DATA_BUFSIZE];   
	DWORD   BytesSEND;   
	DWORD   BytesRECV;   
}   PER_IO_OPERATION_DATA,   *   LPPER_IO_OPERATION_DATA;   


typedef   struct     
{   
	SOCKET   Socket;   
}   PER_HANDLE_DATA,   *   LPPER_HANDLE_DATA;   


DWORD   WINAPI   ServerWorkerThread(LPVOID   CompletionPortID);   


void TestFunc(void)   
{   
	SOCKADDR_IN   InternetAddr;   
	SOCKET   Listen;   
	SOCKET   Accept;   
	HANDLE   CompletionPort;   
	SYSTEM_INFO   SystemInfo;   
	LPPER_HANDLE_DATA   PerHandleData;   
	LPPER_IO_OPERATION_DATA   PerIoData;   
	int   i;   
	DWORD   RecvBytes;   
	DWORD   Flags;   
	DWORD   ThreadID;   
	WSADATA   wsaData;   
	DWORD   Ret;   

	if   ((Ret   =   WSAStartup(0x0202,   &wsaData))   !=   0)   
	{   
		printf("WSAStartup失败了，错误信息如下:   %d\n",   Ret);   
		return;   
	}   

	// 设置一个I/O完成端口.   
	if   ((CompletionPort   =   CreateIoCompletionPort(INVALID_HANDLE_VALUE,   NULL,   0,   0))   ==   NULL)   
	{   
		printf(   "CreateIoCompletionPort 失败了，错误信息如下:   %d\n",   GetLastError());   
		return;   
	}   

	// 测试系统中有多少cpu处理器 
	GetSystemInfo(&SystemInfo);   

	//   基于系统可用的处理器创建工作线程，为每个处理器创建两个线程   
	for(i   =   0;   i   <   SystemInfo.dwNumberOfProcessors   *   2;   i++)   
	{   
		HANDLE   ThreadHandle;   

		// 创建一个服务端线程并且传递一个完成端口给这个线程.   
		if   ((ThreadHandle   =   CreateThread(NULL,   0,   ServerWorkerThread,   CompletionPort,   
			0,   &ThreadID))   ==   NULL)   
		{   
			printf("CreateThread()发生了如下错误： %d\n",   GetLastError());   
			return;   
		}   
		else 
		{
			printf("创建了一个完成端口.\n");
		}
		//   关闭 thread句柄 
		CloseHandle(ThreadHandle);   
	}   

	//   创建一个监听套接字 
	if   ((Listen   =WSASocket(AF_INET,   SOCK_STREAM,   0,   NULL,0,WSA_FLAG_OVERLAPPED))   ==   INVALID_SOCKET)   
	{   
		printf("WSASocket() 发生了如下错误： %d\n",   WSAGetLastError());   
		return;   
	}
	else     
	{
		printf("创建监听套接字成功\n");
	}
	InternetAddr.sin_family   =   AF_INET;   
	InternetAddr.sin_addr.s_addr   =   htonl(INADDR_ANY);   
	InternetAddr.sin_port   =   htons(PORT);   

	if   (bind(Listen,   (PSOCKADDR)   &InternetAddr,   sizeof(InternetAddr))   ==   SOCKET_ERROR)   
	{   
		printf("bind()端口或IP时发生了如下错误： %d\n",   WSAGetLastError());   
		return;   
	}   
	else
	{
		printf("绑定端口%d成功\n",PORT);
	} 

	// 准备socket 用来监听   
	if   (listen(Listen,   5)   ==   SOCKET_ERROR)   
	{   
		printf("listen() 发生了如下错误   %d\n",   WSAGetLastError());   
		return;   
	}   
	else
	{
		printf("预处理成功，开始在端口 %d 处监听...\n",PORT);
	} 

	//接受连接并且交给完成端口处理 
	while(TRUE)   
	{   
		if   ((Accept   =   WSAAccept(Listen,   NULL,   NULL,   NULL,   0))   ==   SOCKET_ERROR)   
		{   
			printf("WSAAccept()   发生了如下错误：   %d\n",   WSAGetLastError());   
			return;   
		}   

		// 创建一个套接字信息结构体去联系起来socket   
		if   ((PerHandleData   =   (LPPER_HANDLE_DATA)   GlobalAlloc(GPTR,     
			sizeof(PER_HANDLE_DATA)))   ==   NULL)   
		{   
			printf("GlobalAlloc()   发生了如下错误：   %d\n",   GetLastError());   
			return;   
		}   

		// 将接受到的套接字与原始的完成端口联系起来.   

		printf("号码为   %d   的socket连接上了\n",   Accept);   
		PerHandleData->Socket   =   Accept;   

		if   (CreateIoCompletionPort((HANDLE)   Accept,   CompletionPort,   (DWORD)   PerHandleData,   
			0)   ==   NULL)   
		{   
			printf("CreateIoCompletionPort   发生了如下错误：   %d\n",   GetLastError());   
			return;   
		}   

		//   创建每一个I/O 套接字信息结构体去和下面被调用的 to   associate   with   the     
		//   WSARecv 连接.   
		if   ((PerIoData   =   (LPPER_IO_OPERATION_DATA)   GlobalAlloc(GPTR,                     sizeof(PER_IO_OPERATION_DATA)))   ==   NULL)   
		{   
			printf("GlobalAlloc() 发生了如下错误： %d\n",   GetLastError());   
			return;   
		}   
		else
		{
			printf("接收了一个连接\n");
		} 
		ZeroMemory(&(PerIoData->Overlapped),   sizeof(OVERLAPPED));   
		PerIoData->BytesSEND   =   0;   
		PerIoData->BytesRECV   =   0;   
		PerIoData->DataBuf.len   =   DATA_BUFSIZE;   
		PerIoData->DataBuf.buf   =   PerIoData->Buffer;   

		Flags   =   0;   
		if   (WSARecv(Accept,   &(PerIoData->DataBuf),   1,   &RecvBytes,   &Flags,   
			&(PerIoData->Overlapped),   NULL)   ==   SOCKET_ERROR)   
		{   
			if   (WSAGetLastError()   !=   ERROR_IO_PENDING)   
			{   
				printf("WSARecv() 发生了如下错误： %d\n",   WSAGetLastError());   
				return;   
			}   
		}   
	}   
}   


DWORD   WINAPI   ServerWorkerThread(LPVOID   CompletionPortID)   
{   
	HANDLE   CompletionPort   =   (HANDLE)   CompletionPortID;   
	DWORD   BytesTransferred;   
	LPOVERLAPPED   Overlapped;   
	LPPER_HANDLE_DATA   PerHandleData;   
	LPPER_IO_OPERATION_DATA   PerIoData;   
	DWORD   SendBytes,   RecvBytes;   
	DWORD   Flags;   

	while(TRUE)   
	{   

		if   (GetQueuedCompletionStatus(CompletionPort,   &BytesTransferred,   
			(LPDWORD)&PerHandleData,   (LPOVERLAPPED   *)   &PerIoData,   INFINITE)   ==   0)   
		{   
			printf("GetQueuedCompletionStatus   发生了如下错误： %d\n",   GetLastError());   
			continue;
		}   

		//首先检查一下去套接字看是否在上发生了错误并且如果发生了错误就关闭套接
		//字并且清除与套接字连接的 SOCKET_INFORMATION结构信息体 
		if   (BytesTransferred   ==   0)   
		{   
			printf("正在关闭socket   %d\n",   PerHandleData->Socket);   

			if   (closesocket(PerHandleData->Socket)   ==   SOCKET_ERROR)   
			{   
				printf("closesocket()   发生了如下错误： %d\n",   WSAGetLastError());   
				return   0;   
			}   

			GlobalFree(PerHandleData);   
			GlobalFree(PerIoData);   
			continue;   
		}   
		//检查如果 BytesRECV字段等于0，这就意味着一个 WSARecv调用刚刚完成了所以从完成的WSARecv()调用中
		//用BytesTransferred值更新 BytesRECV字段 
		if   (PerIoData->BytesRECV   ==   0)   
		{   
			PerIoData->BytesRECV   =   BytesTransferred;   
			PerIoData->BytesSEND   =   0;   
		}   
		else   
		{   
			PerIoData->BytesSEND   +=   BytesTransferred;   
		}   

		if   (PerIoData->BytesRECV   >   PerIoData->BytesSEND)   
		{   
			//发布另外一个 WSASend()请求
			//既然WSASend()不是 gauranteed去发送所有字节的请求
			//继续调用 WSASend()发送直到所有收到的字节被发送 
			ZeroMemory(&(PerIoData->Overlapped),   sizeof(OVERLAPPED));   

			PerIoData->DataBuf.buf   =   PerIoData->Buffer   +   PerIoData->BytesSEND;   
			PerIoData->DataBuf.len   =   PerIoData->BytesRECV   -   PerIoData->BytesSEND;   

			if   (WSASend(PerHandleData->Socket,   &(PerIoData->DataBuf),   1,   &SendBytes,   0,   
				&(PerIoData->Overlapped),   NULL)   ==   SOCKET_ERROR)   
			{   
				if   (WSAGetLastError()   !=   ERROR_IO_PENDING)   
				{   
					printf("WSASend() 发生了如下错误：   %d\n",   WSAGetLastError());   
					return   0;   
				}   
			}   
		}   
		else   
		{   
			PerIoData->BytesRECV   =   0;   
			//现在没有更多的字节发送过去用来post另外一个WSARecv()请求 
			Flags   =   0;   
			ZeroMemory(&(PerIoData->Overlapped),   sizeof(OVERLAPPED));   

			PerIoData->DataBuf.len   =   DATA_BUFSIZE;   
			PerIoData->DataBuf.buf   =   PerIoData->Buffer;   

			if   (WSARecv(PerHandleData->Socket,   &(PerIoData->DataBuf),   1,   &RecvBytes,   &Flags,   
				&(PerIoData->Overlapped),   NULL)   ==   SOCKET_ERROR)   
			{   
				if   (WSAGetLastError()   !=   ERROR_IO_PENDING)   
				{   
					printf("WSARecv() 发生了如下错误：   %d\n",   WSAGetLastError());   
					return   0;   
				}   
			}   
		}   
	}   
}  



