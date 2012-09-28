#include "FS_DLL.h"
#include "SocketClient.h"

#include <time.h>

CSocketClient::CSocketClient(int len)
	: FSClient(len)
{
	m_bRunning = 0;

	m_threadSend = NULL;
	m_threadRecv = NULL;
	m_hSocket = INVALID_SOCKET;
	nBytesRemain = 0;

	char szName[MAX_PATH];
	sprintf(szName, "Socket%d%d", this, (int)time(NULL));
	m_hEventSocket = CreateEvent(NULL, FALSE, FALSE, szName);

	InitializeCriticalSection(&csRecv);
	InitializeCriticalSection(&csSend);
}

CSocketClient::~CSocketClient()
{
	Disconnect();
	CloseHandle(m_hEventSocket);
	DeleteCriticalSection(&csSend);
	DeleteCriticalSection(&csRecv);
}

bool CSocketClient::Connect(LPCSTR lpszServerIP, unsigned short nPort)
{
	if(lpszServerIP)
		strcpy(m_szServerAddr, lpszServerIP);
	else 
		m_szServerAddr[0] = 0;
	m_nServerPort = nPort;

	WSADATA WSAData;
	if ( WSAStartup(WINSOCK_VERSION,&WSAData) ) 
	{
		return false;
	}

	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if (m_hSocket == INVALID_SOCKET) 
	{
		OutputDebugString("socket failure!\n");
		return false;
	}


	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(m_szServerAddr);
	sockAddr.sin_port = htons( m_nServerPort);
	m_bRunning = true;

	//设置事件选择模式，该模式下套接字自动为非阻塞
	int nErrorCode = WSAEventSelect(m_hSocket, m_hEventSocket, FD_CONNECT);
	if (nErrorCode == SOCKET_ERROR) 
	{
		OutputDebugString((" WSAEventSelect failure!\n"));
		return false;
	}

	int nConnect = connect(m_hSocket, (LPSOCKADDR)&sockAddr, sizeof(sockAddr));
	if (nConnect == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) 
	{
		OutputDebugString(( " Socket Connect Error!\n"));
		return false;
	}
	else
	{
		//等待连接返回，timeout时间
		#define CONNECT_TIMEOUT		2000

		DWORD dwWaitRes = WaitForSingleObject(m_hEventSocket, CONNECT_TIMEOUT);
		if( dwWaitRes != WAIT_OBJECT_0 )
		{
			OutputDebugString((" Connect failure!\n"));
			Disconnect();
			return false;
		}

		WSANETWORKEVENTS event;
		int nRes = WSAEnumNetworkEvents(m_hSocket, m_hEventSocket, &event);
		if ( nRes == 0) 
		{
			if (event.lNetworkEvents & FD_CONNECT) 
			{
				if( 0 != event.iErrorCode[FD_CONNECT_BIT])
				{
					OutputDebugString((" Connect failure\n"));
					Disconnect();
					return false;
				}
				else
				{
					OutputDebugString((" Connect success\n"));
				}
			}
		}

		//设置事件选择模式，该模式下套接字自动为非阻塞
		int nErrorCode = WSAEventSelect(m_hSocket, m_hEventSocket, FD_READ|FD_CLOSE);
		if (nErrorCode == SOCKET_ERROR) 
		{
			OutputDebugString((" WSAEventSelect failure!\n"));
			return false;
		}

		m_threadSend = CreateThread(NULL, 0, SendThreadProc, this, 0, NULL);
		if( m_threadSend == NULL)
		{
			OutputDebugString((" CreateSendThread failure!\n"));
			Disconnect();
			return false;
		}

		m_threadRecv = CreateThread(NULL, 0, RecvThreadProc, this, 0, NULL);
		if(m_threadRecv == NULL)
		{
			OutputDebugString(("  CreateRecvThread failure!\n"));
			Disconnect();
			return false;
		}
	}

	return true;
}

void CSocketClient::Disconnect()
{
	m_bRunning = false;

	WaitForSingleObject( m_threadSend, 1000 );

	WaitForSingleObject( m_threadRecv, 1000 );

	closesocket(m_hSocket);
}

const char* CSocketClient::GetPacket(int &len)
{
	char* pPacket = NULL;
	EnterCriticalSection(&csRecv);
	if(m_listPacketRecv.size() > 0 && IsConnect())
	{
		pPacket = m_listPacketRecv.begin()->first;
		len = m_listPacketRecv.begin()->second;
		m_listPacketRecv.erase(m_listPacketRecv.begin());
	}
	LeaveCriticalSection(&csRecv);

	return pPacket;
}


// 释放包
void CSocketClient::DeletePacket(const char** pPacket)
{
	delete *pPacket;
	*pPacket = NULL;
}

bool CSocketClient::SendPacket(const char* pPacket, int len)
{
	char* ptr = new char[len];
	memcpy(ptr, pPacket, len);

	EnterCriticalSection(&csSend);
	m_listPacketSend.insert(map<char*, int>::value_type(ptr,len));
	LeaveCriticalSection(&csSend);

	return true;
}


DWORD CSocketClient::RecvThreadProc(LPVOID pParam)
{
	CSocketClient *pSocketClient = (CSocketClient *)pParam;
	WSANETWORKEVENTS event;
	while(pSocketClient->m_bRunning)
	{
		DWORD dwWaitRes = WaitForSingleObject(pSocketClient->m_hEventSocket, 1);
		switch(dwWaitRes) 
		{
		case WAIT_OBJECT_0:
			{
				int nRes = WSAEnumNetworkEvents(pSocketClient->m_hSocket, pSocketClient->m_hEventSocket, &event);
				if ( nRes == 0) 
				{
					if(event.lNetworkEvents & FD_READ)
					{
						pSocketClient->Recv();
					}
					if(event.lNetworkEvents & FD_CLOSE)
					{
						OutputDebugString(("  Socket closed!\n"));
						pSocketClient->Disconnect();
					}
				}
			}
			break;
		default:
			break;
		}
	}
	OutputDebugString(("  Receive thread exit normally!\n"));

	return 0;
}

DWORD CSocketClient::SendThreadProc(LPVOID pParam)
{
	CSocketClient *pSocketClient = (CSocketClient *)pParam;
	while(pSocketClient->m_bRunning)
	{
		pSocketClient->Send();
		Sleep(1);
	}
	OutputDebugString(("  Send thread exit normally!\n"));
	return 0;

}

void CSocketClient::Send()
{
	char *pPacket = NULL;
	int len = 0;
	EnterCriticalSection(&csSend);
	if(m_listPacketSend.size() > 0)
	{
		pPacket = m_listPacketSend.begin()->first;
		len = m_listPacketSend.begin()->second;
		m_listPacketSend.erase(m_listPacketSend.begin());
	}
	LeaveCriticalSection(&csSend);

	if(pPacket != NULL)
	{
		int nLen = send(m_hSocket, (char*)pPacket, len, 0);
		if(nLen < 0)
		{//发送失败
		}

		delete pPacket;
	}
}

void CSocketClient::Recv()
{
 	int nLen = recv(m_hSocket, buffIn + nBytesRemain, MAX_PACKET_SIZE - nBytesRemain, 0 );
	if( nLen > 0 )
	{
		nBytesRemain += nLen;
		if (m_PacketSize == 0)
		{
			m_PacketSize = *buffIn;
			nBytesRemain -= sizeof(int);
		}
	
		while( nBytesRemain >= m_PacketSize && m_PacketSize>0)
		{
			char* pPacket = (char*)buffIn;
			char* ptr = new char[m_PacketSize];
			memcpy(ptr, pPacket+4, m_PacketSize);
			EnterCriticalSection(&csRecv);
			m_listPacketRecv.insert(map< char*, int >::value_type(ptr,m_PacketSize));
			LeaveCriticalSection(&csRecv);

			nBytesRemain -= m_PacketSize;
			if(nBytesRemain)
				memmove(buffIn, buffIn + m_PacketSize, nBytesRemain);
			m_PacketSize = 0;
		}
	}
	else
	{
		int nErr = WSAGetLastError();
		if(nErr != WSAEWOULDBLOCK)
		{
			Disconnect();
		}
	}
}


//判断某个客户端是否连接状态
bool CSocketClient::IsConnect()
{
	return m_bRunning;
}




