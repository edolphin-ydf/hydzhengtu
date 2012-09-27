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
	CIOCPServer* pIOCPServer;   //��ǰ��ɶ˿�����Server
	SOCKET sSocket;             //��ǰsocket
	HANDLE hCompletionPort;     //��ɶ˿ھ��
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

//��SOCKET��������
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

// ��ʼ��
bool CIOCPServer::Initialize(int nMaxClient, u_short nPort)
{
	nMaxClient = nMaxClient > MAX_USER_NUM ? MAX_USER_NUM : nMaxClient;
	m_nMaxClient = nMaxClient;

	// ����WS2_32.DLL
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
		printf("Socket�汾��һ��\n");
		return false;
	}


	// ����IO��ɶ˿�
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_hCompletionPort == NULL)
	{
		ErrorExit("CreateIoCompletionPort");
		return false; 
	}


	// ���ݵ�ǰϵͳ�Ĵ���������������2���Ĺ����߳�
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


	// ����һ��Socket 
	// �ڸ�����IPPROTO_TCP
	SOCKET ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ListenSocket == INVALID_SOCKET)
	{
		ErrorExit("WSASocket");
		return false;   
	}


	// ��ñ���ϵͳ����������IP��ַ
	hostent* localHost;
	char* localIP;
	localHost = gethostbyname("");
	localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);

	// ��IP�Ͷ˿�
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	//SockAddr.sin_addr.s_addr = inet_addr(localIP);	// ֻ����ָ��IP�Ŀͻ�������
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	SockAddr.sin_port = htons(nPort);
	if ( bind(ListenSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR )
	{   
		ErrorExit("bind");
		return false;   
	}


	// ׼������Socket����
	//if ( listen(ListenSocket, 5) == SOCKET_ERROR )
	if ( listen(ListenSocket, 1024) == SOCKET_ERROR )
	{   
		ErrorExit("listen");
		return false;   
	}


	// ����Accept�̺߳���
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


//����һ�������ָ���Ŀͻ���
bool CIOCPServer::SendData(u_short nClient, char* pBuffer, u_long len)
{
	// ȡ��SOCKET
	SOCKET sSocket = GetSocket(nClient);
	if (sSocket == INVALID_SOCKET)
	{
		return false;
	}

	// ����һ�����ڷ��͵�Overlapped
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
			printf("WSASend() ���������´���   %d\n",   iError);
			// �쳣�����¼�
			OnError(nClient, iError);
			// �ر�SOCKET
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
	// ȡ��SOCKET
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
			printf("WSASend() ���������´���   %d\n",   iError);
			// �쳣�����¼�
			this->OnError(nClient, iError);
			// �ر�SOCKET
			this->CloseSocket(nClient);
			return FALSE;
		}   
	}   
	return TRUE;
}


//���ͷ�����������ӿͻ�
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

//���ͷ�����������ӿͻ�
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

//�Ͽ��ͻ���
void CIOCPServer::DisconnectClient(u_short nClient)
{
	CloseSocket(nClient);
}

//�ж�ĳ���ͻ����Ƿ�����״̬
bool CIOCPServer::IsConnect(u_short nClient)
{
	return GetSocket(nClient) != INVALID_SOCKET;
}

// ���δ�������Ϣ��
DWORD CIOCPServer::GetUnProcessPackNum()
{

	return 0;
}


// ��ȡ��ǰ�ͻ������ӵ�����
DWORD CIOCPServer::GetCurClientNum()
{
	return m_nCurClientNum;
}


// �ر�SOCKET
BOOL CIOCPServer::CloseSocket(u_short nClient)
{
	SOCKET Socket = GetSocket(nClient);
	if (Socket == INVALID_SOCKET)
	{
		return FALSE;
	}

	// �ر�Socket
	if (closesocket(Socket) == SOCKET_ERROR)   
	{   
		ErrorExit("closesocket");
		return FALSE;
	}

	// �ر�SOCKET�¼�
	OnClose(nClient);

	// ��ǰ������-1
	InterlockedDecrement(&m_nCurClientNum);

	RemoveSocket(nClient);

	return TRUE;
}


// Ͷ��һ��������Ϣ
BOOL CIOCPServer::PostRecv(u_short nClient, SOCKET sSocket)
{
	// ���ͽ���
	DWORD RecvBytes = 0;
	DWORD Flags = 0; 
	SRecvOverlapped* pRecvOverlapped = GetEmptyRecvOverlapped(nClient);
	pRecvOverlapped->id = nClient;	// ��¼�ͻ���ID
	if ( WSARecv(sSocket, &pRecvOverlapped->WsaBuf, 1, &RecvBytes, &Flags,
		(LPOVERLAPPED)pRecvOverlapped, NULL)   ==   SOCKET_ERROR )   
	{   
		int iError = WSAGetLastError();
		if ( iError != ERROR_IO_PENDING )
		{   
			// �쳣����
			OnError(nClient, iError);
			CloseSocket(nClient);
			return FALSE;
		}   
	}  

	return TRUE;
}


// ��������������̺߳���
UINT WINAPI CIOCPServer::ThreadFuncForAccept(void* pParam)
{
	SThreadParam* pAcceptParam = (SThreadParam*)pParam;
	CIOCPServer*pIOCPServer = pAcceptParam->pIOCPServer;
	SOCKET ListenSocket = pAcceptParam->sSocket;
	HANDLE hCompletionPort = pAcceptParam->hCompletionPort;
	BOOL bExit = FALSE;
	while (!bExit)
	{
		// ������������
		sockaddr saClient;
		int iClientSize = sizeof(saClient);
		SOCKET AcceptSocket = WSAAccept(ListenSocket, &saClient, &iClientSize, NULL, NULL);
		if ( AcceptSocket == SOCKET_ERROR )
		{
			// �쳣�����¼�
			pIOCPServer->OnError(INVALID_CLIENT_ID, WSAGetLastError());
			continue;
		}

		// ��SOCKET��������
		u_short nClientId = pIOCPServer->InsertSocket(AcceptSocket);
		if (nClientId == INVALID_CLIENT_ID)
		{
			printf("�ͻ������ӹ���\n");
			pIOCPServer->CloseSocket(nClientId);
			continue;
		}

		// �����¼�
		pIOCPServer->OnAccept(nClientId);

		// ��ǰ��������ס+1
		InterlockedIncrement(&pIOCPServer->m_nCurClientNum);

		// ����IO��ɶ˿�
		if ( CreateIoCompletionPort((HANDLE)AcceptSocket, hCompletionPort, AcceptSocket, 0) == NULL )
		{
			ErrorExit("CreateIoCompletionPort");
			return 0;
		}

		// ����һ����������
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
				printf("GetQueuedCompletionStatus   ���������´��� %d\n",   GetLastError());
			}
		}   

		// ��Ч�ľ��
		nClientId = pSOverlapped->id;

		if (BytesTransferred == 0)
		{
			// �ر�SOCKET
			pIOCPServer->CloseSocket(nClientId);
			continue;
		}

		if ( pSOverlapped->IoMode == IoRecv )
		{
			// ���ݽ����¼�
			pIOCPServer->OnReceive(nClientId, pSOverlapped->WsaBuf.buf, BytesTransferred);

			// ����һ����������
			pIOCPServer->PostRecv(nClientId, Socket);
		}
		else if ( pSOverlapped->IoMode == IoSend )
		{
			SSendOverlapped* pSendOverlapped = (SSendOverlapped*)pSOverlapped;

			pSendOverlapped->SentBytes += BytesTransferred;
			if (pSendOverlapped->SentBytes < pSendOverlapped->TotalBytes)
			{
				// ����ʣ���û�з����ֽ���, Ȼ���������
				assert(pSendOverlapped->SentBytes <= pSendOverlapped->TotalBytes);
				DWORD RemainBytes = pSendOverlapped->TotalBytes - pSendOverlapped->SentBytes;
				pSendOverlapped->WsaBuf.buf = pSendOverlapped->pBuffer + pSendOverlapped->SentBytes;// ���÷��ͻ���
				pSendOverlapped->WsaBuf.len = RemainBytes;						// ���÷��ͻ����С

				pIOCPServer->SendByte(nClientId,pSendOverlapped);
			}
			else if (pSendOverlapped->SentBytes == pSendOverlapped->TotalBytes)
			{
				// ���ݷ����¼�
				pIOCPServer->OnSend(nClientId, pSOverlapped->WsaBuf.buf , BytesTransferred);

				// �������,����ڴ�
				delete[] pSendOverlapped->pBuffer;
				delete pSendOverlapped;
			}
			else if (pSendOverlapped->SentBytes > pSendOverlapped->TotalBytes)
			{
				printf("�����쳣\n");
			}
		}
	}

	delete pWorkerParam;
	return 0;
}

