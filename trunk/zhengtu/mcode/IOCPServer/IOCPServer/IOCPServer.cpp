#include "IOCPServer.h"
#include <process.h>
#include <stdio.h>
#include <assert.h>

void ErrorExit(LPTSTR lpszFunction) 
{ 
	TCHAR szBuf[128]; 
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	wsprintf(szBuf, 
		"%s failed with error %d: %s", 
		lpszFunction, dw, lpMsgBuf); 

	MessageBox(NULL, szBuf, "Error", MB_OK); 

	LocalFree(lpMsgBuf);
	ExitProcess(dw); 
}




struct SThreadParam 
{
	CIOCPServer* pIOCPServer;   //当前完成端口所属Server
	SOCKET sSocket;             //当前socket
	HANDLE hCompletionPort;     //完成端口句柄
};





SRecvOverlapped RecvOverlapped[MAX_RECV_OVERLAPPED];

SRecvOverlapped* GetEmptyRecvOverlapped(WORD id)
{
	SRecvOverlapped* pRecvOverlapped=NULL;
	if(id<MAX_RECV_OVERLAPPED) pRecvOverlapped=&RecvOverlapped[id];
	return pRecvOverlapped;
}


CIOCPServer::CIOCPServer(void) :
m_hCompletionPort(NULL),
m_nCurClientNum(0)
{
	for (int i=0; i<MAX_USER_NUM; i++)
	{
		m_vecSocket[i] = INVALID_SOCKET;
	}
}

CIOCPServer::~CIOCPServer(void)
{
	WSACleanup();
}

//把SOCKET加入数组
u_short CIOCPServer::InsertSocket(SOCKET sSocket)
{
	u_short nClient = INVALID_CLIENT_ID;
	CAutoLock CAutoLock(m_csSockets);
	if (m_nCurClientNum < m_nMaxClient)
	{
		for (u_short i=0; i<MAX_USER_NUM; i++)
		{
			if (m_vecSocket[i] == INVALID_SOCKET)
			{
				m_vecSocket[i] = sSocket;
				nClient = i;
				break;
			}	
		}
	}

	return nClient;
}

SOCKET CIOCPServer::GetSocket(u_short nClient)
{
	if (nClient >= MAX_USER_NUM)
	{
		return INVALID_SOCKET;
	}

	return m_vecSocket[nClient];
}

BOOL CIOCPServer::RemoveSocket(u_short nClient)
{
	if (nClient >= MAX_USER_NUM)
	{
		return FALSE;
	}

	CAutoLock CAutoLock(m_csSockets);
	m_vecSocket[nClient] = INVALID_SOCKET;

	return TRUE;
}

// 初始化
bool CIOCPServer::Initialize(int nMaxClient, u_short nPort)
{
	nMaxClient = nMaxClient > MAX_USER_NUM ? MAX_USER_NUM : nMaxClient;
	m_nMaxClient = nMaxClient;

	// 加载WS2_32.DLL
	WSADATA wsaData;
	ZeroMemory(&wsaData, sizeof(WSADATA));
	int Ret = 0;
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	if ( (Ret=WSAStartup(wVersionRequested, &wsaData)) != 0 )
	{
		ErrorExit("WSAStartup");
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Socket版本不一致\n");
		return false;
	}


	// 创建IO完成端口
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_hCompletionPort == NULL)
	{
		ErrorExit("CreateIoCompletionPort");
		return false; 
	}


	// 根据当前系统的处理器数量，创建2倍的工作线程
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	for (UINT i=0; i<SystemInfo.dwNumberOfProcessors*2; i++)
	{
		SThreadParam* pParam = new SThreadParam;
		pParam->sSocket = INVALID_SOCKET;
		pParam->hCompletionPort = m_hCompletionPort;
		pParam->pIOCPServer = this;

		UINT nThreadId = 0;
		HANDLE hWorkerTread = NULL;
		hWorkerTread = (HANDLE)_beginthreadex(NULL, 0, ServerWorkerThread, pParam, 0, &nThreadId);
		if (hWorkerTread == NULL)
		{
			ErrorExit("Create Worker Thread");
			return false; 
		}
	}


	// 创建一个Socket 
	// 第个参数IPPROTO_TCP
	SOCKET ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ListenSocket == INVALID_SOCKET)
	{
		ErrorExit("WSASocket");
		return false;   
	}


	// 获得本地系统的主机名和IP地址
	hostent* localHost;
	char* localIP;
	localHost = gethostbyname("");
	localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);

	// 绑定IP和端口
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	//SockAddr.sin_addr.s_addr = inet_addr(localIP);	// 只允许指定IP的客户端连接
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	SockAddr.sin_port = htons(nPort);
	if ( bind(ListenSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR )
	{   
		ErrorExit("bind");
		return false;   
	}


	// 准备监听Socket连接
	//if ( listen(ListenSocket, 5) == SOCKET_ERROR )
	if ( listen(ListenSocket, 1024) == SOCKET_ERROR )
	{   
		ErrorExit("listen");
		return false;   
	}


	// 创建Accept线程函数
	SThreadParam* pParam = new SThreadParam;
	pParam->sSocket = ListenSocket;
	pParam->hCompletionPort = m_hCompletionPort;
	pParam->pIOCPServer = this;
	if ( _beginthreadex(NULL, 0, ThreadFuncForAccept, pParam, 0, NULL) == NULL )
	{
		ErrorExit("Create Accept Thread");
		return false; 
	}


	return true;
}


//发送一个封包给指定的客户端
bool CIOCPServer::SendData(u_short nClient, char* pBuffer, u_long len)
{
	// 取出SOCKET
	SOCKET sSocket = GetSocket(nClient);
	if (sSocket == INVALID_SOCKET)
	{
		return false;
	}

	// 创建一个用于发送的Overlapped
	SSendOverlapped* pSendOverlapped = new SSendOverlapped;
	pSendOverlapped->id = nClient;
	pSendOverlapped->pBuffer = new char[len];
	memcpy(pSendOverlapped->pBuffer, pBuffer, len);
	pSendOverlapped->WsaBuf.buf = pSendOverlapped->pBuffer;
	pSendOverlapped->WsaBuf.len = len;
	pSendOverlapped->SentBytes = 0;
	pSendOverlapped->TotalBytes = len;
	DWORD SendBytes;
	if ( WSASend(sSocket, &pSendOverlapped->WsaBuf, 1, &SendBytes, 0,
		(LPOVERLAPPED)pSendOverlapped, NULL)   ==   SOCKET_ERROR )
	{   
		int iError = WSAGetLastError();
		if ( iError != ERROR_IO_PENDING )
		{   
			printf("WSASend() 发生了如下错误：   %d\n",   iError);
			// 异常错误事件
			OnError(nClient, iError);
			// 关闭SOCKET
			CloseSocket(nClient);
			delete[] pSendOverlapped->pBuffer;
			delete pSendOverlapped;
			return FALSE;
		}   
	}   

	return TRUE;
}


bool CIOCPServer::SendByte( u_short nClient, SSendOverlapped* pSendOverlapped )
{
	// 取出SOCKET
	SOCKET sSocket = GetSocket(nClient);
	if (sSocket == INVALID_SOCKET)
	{
		return false;
	}
	DWORD SendBytes;
	if ( WSASend(sSocket, &pSendOverlapped->WsaBuf, 1, &SendBytes, 0,
		(LPOVERLAPPED)pSendOverlapped, NULL)   ==   SOCKET_ERROR )
	{   
		int iError = WSAGetLastError();
		if ( iError != ERROR_IO_PENDING )
		{   
			printf("WSASend() 发生了如下错误：   %d\n",   iError);
			// 异常错误事件
			this->OnError(nClient, iError);
			// 关闭SOCKET
			this->CloseSocket(nClient);
			return FALSE;
		}   
	}   
	return TRUE;
}


//发送封包给所有连接客户
bool CIOCPServer::SendDataToAll(char* pBuffer, u_long len)
{
	for (unsigned int i=0; i<MAX_USER_NUM; i++)
	{
		if (m_vecSocket[i] != INVALID_SOCKET)
		{
			SendData(i, pBuffer, len);
		}
	}
	return true;
}

//发送封包给其他连接客户
bool CIOCPServer::SendDataToOther(u_short nExcludeClient, char* pBuffer, u_long len)
{
	for (unsigned int i=0; i<MAX_USER_NUM; i++)
	{
		if (m_vecSocket[i] != INVALID_SOCKET && m_vecSocket[i] != nExcludeClient)
		{
			SendData(i, pBuffer, len);
		}
	}
	return true;
}

//断开客户端
void CIOCPServer::DisconnectClient(u_short nClient)
{
	CloseSocket(nClient);
}

//判断某个客户端是否连接状态
bool CIOCPServer::IsConnect(u_short nClient)
{
	return GetSocket(nClient) != INVALID_SOCKET;
}

// 获得未处理的消息数
DWORD CIOCPServer::GetUnProcessPackNum()
{

	return 0;
}


// 获取当前客户端连接的数量
DWORD CIOCPServer::GetCurClientNum()
{
	return m_nCurClientNum;
}


// 关闭SOCKET
BOOL CIOCPServer::CloseSocket(u_short nClient)
{
	SOCKET Socket = GetSocket(nClient);
	if (Socket == INVALID_SOCKET)
	{
		return FALSE;
	}

	// 关闭Socket
	if (closesocket(Socket) == SOCKET_ERROR)   
	{   
		ErrorExit("closesocket");
		return FALSE;
	}

	// 关闭SOCKET事件
	OnClose(nClient);

	// 当前连接数-1
	InterlockedDecrement(&m_nCurClientNum);

	RemoveSocket(nClient);

	return TRUE;
}


// 投递一个接受消息
BOOL CIOCPServer::PostRecv(u_short nClient, SOCKET sSocket)
{
	// 发送接受
	DWORD RecvBytes = 0;
	DWORD Flags = 0; 
	SRecvOverlapped* pRecvOverlapped = GetEmptyRecvOverlapped(nClient);
	pRecvOverlapped->id = nClient;	// 记录客户端ID
	if ( WSARecv(sSocket, &pRecvOverlapped->WsaBuf, 1, &RecvBytes, &Flags,
		(LPOVERLAPPED)pRecvOverlapped, NULL)   ==   SOCKET_ERROR )   
	{   
		int iError = WSAGetLastError();
		if ( iError != ERROR_IO_PENDING )
		{   
			// 异常错误
			OnError(nClient, iError);
			CloseSocket(nClient);
			return FALSE;
		}   
	}  

	return TRUE;
}


// 处理连接请求的线程函数
UINT WINAPI CIOCPServer::ThreadFuncForAccept(void* pParam)
{
	SThreadParam* pAcceptParam = (SThreadParam*)pParam;
	CIOCPServer*pIOCPServer = pAcceptParam->pIOCPServer;
	SOCKET ListenSocket = pAcceptParam->sSocket;
	HANDLE hCompletionPort = pAcceptParam->hCompletionPort;
	BOOL bExit = FALSE;
	while (!bExit)
	{
		// 处理连接请求
		sockaddr saClient;
		int iClientSize = sizeof(saClient);
		SOCKET AcceptSocket = WSAAccept(ListenSocket, &saClient, &iClientSize, NULL, NULL);
		if ( AcceptSocket == SOCKET_ERROR )
		{
			// 异常错误事件
			pIOCPServer->OnError(INVALID_CLIENT_ID, WSAGetLastError());
			continue;
		}

		// 把SOCKET加入数组
		u_short nClientId = pIOCPServer->InsertSocket(AcceptSocket);
		if (nClientId == INVALID_CLIENT_ID)
		{
			printf("客户端连接过多\n");
			pIOCPServer->CloseSocket(nClientId);
			continue;
		}

		// 连接事件
		pIOCPServer->OnAccept(nClientId);

		// 当前连接数锁住+1
		InterlockedIncrement(&pIOCPServer->m_nCurClientNum);

		// 创建IO完成端口
		if ( CreateIoCompletionPort((HANDLE)AcceptSocket, hCompletionPort, AcceptSocket, 0) == NULL )
		{
			ErrorExit("CreateIoCompletionPort");
			return 0;
		}

		// 发送一个接收请求
		pIOCPServer->PostRecv(nClientId, AcceptSocket);
	}

	delete pAcceptParam;
	return 0;
}


UINT WINAPI CIOCPServer::ServerWorkerThread(void* pParam)
{
	SThreadParam* pWorkerParam = (SThreadParam*)pParam;
	CIOCPServer*pIOCPServer = pWorkerParam->pIOCPServer;
	HANDLE hCompletionPort = pWorkerParam->hCompletionPort;
	DWORD   BytesTransferred;   
	SOverlapped* pSOverlapped;
	SOCKET Socket = NULL;
	u_short nClientId = INVALID_CLIENT_ID;
	int iError = 0;

	while(TRUE)   
	{   
		if ( GetQueuedCompletionStatus(hCompletionPort, &BytesTransferred,   
			(PULONG_PTR)&Socket, (LPOVERLAPPED*)&pSOverlapped, INFINITE) == 0 )   
		{   
			DWORD dwLastError = GetLastError();
			if ( dwLastError != 64 )
			{
				printf("GetQueuedCompletionStatus   发生了如下错误： %d\n",   GetLastError());
			}
		}   

		// 无效的句柄
		nClientId = pSOverlapped->id;

		if (BytesTransferred == 0)
		{
			// 关闭SOCKET
			pIOCPServer->CloseSocket(nClientId);
			continue;
		}

		if ( pSOverlapped->IoMode == IoRecv )
		{
			// 数据接收事件
			pIOCPServer->OnReceive(nClientId, pSOverlapped->WsaBuf.buf, BytesTransferred);

			// 发送一个接收请求
			pIOCPServer->PostRecv(nClientId, Socket);
		}
		else if ( pSOverlapped->IoMode == IoSend )
		{
			SSendOverlapped* pSendOverlapped = (SSendOverlapped*)pSOverlapped;

			pSendOverlapped->SentBytes += BytesTransferred;
			if (pSendOverlapped->SentBytes < pSendOverlapped->TotalBytes)
			{
				// 计算剩余的没有发送字节数, 然后继续发送
				assert(pSendOverlapped->SentBytes <= pSendOverlapped->TotalBytes);
				DWORD RemainBytes = pSendOverlapped->TotalBytes - pSendOverlapped->SentBytes;
				pSendOverlapped->WsaBuf.buf = pSendOverlapped->pBuffer + pSendOverlapped->SentBytes;// 设置发送缓冲
				pSendOverlapped->WsaBuf.len = RemainBytes;						// 设置发送缓冲大小

				pIOCPServer->SendByte(nClientId,pSendOverlapped);
			}
			else if (pSendOverlapped->SentBytes == pSendOverlapped->TotalBytes)
			{
				// 数据发送事件
				pIOCPServer->OnSend(nClientId, pSOverlapped->WsaBuf.buf , BytesTransferred);

				// 发送完成,清除内存
				delete[] pSendOverlapped->pBuffer;
				delete pSendOverlapped;
			}
			else if (pSendOverlapped->SentBytes > pSendOverlapped->TotalBytes)
			{
				printf("数据异常\n");
			}
		}
	}

	delete pWorkerParam;
	return 0;
}

