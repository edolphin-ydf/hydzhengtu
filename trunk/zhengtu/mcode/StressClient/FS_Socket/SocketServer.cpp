#include "FS_DLL.h"
#include "SocketServer.h"


#pragma warning(disable:4313)
#pragma warning(disable:4244)

CSocketServer::CSocketServer()
{
	m_nCurClient = 0;
	m_nMaxClient = 0;
	m_aClient = NULL;
	m_threadSend = NULL;
	m_threadRecv = NULL;
	m_threadListen = NULL;
	m_bRunning = false;

	InitializeCriticalSection(&csSend);
	InitializeCriticalSection(&csRecv);
}

CSocketServer::~CSocketServer()
{
	delete m_aClient;

	DeleteCriticalSection(&csRecv);
	DeleteCriticalSection(&csSend);
}

bool CSocketServer::Initialize(int nMaxClient, short nPort)
{
	m_nServerPort = nPort;
	m_nMaxClient = nMaxClient;
	m_aClient = new SLOT[m_nMaxClient];

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

	int optval = 1;
	setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));

	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = ADDR_ANY;
	sockAddr.sin_port = htons( m_nServerPort);
	m_bRunning = true;

	int err = bind(m_hSocket, (sockaddr*)&sockAddr, sizeof(sockAddr));
	if(err != 0)
	{
		printf("bind port failure!\n");
		return false;
	}

	err = listen(m_hSocket, 5);
	if(err != 0)
	{
		printf("listen failure\n");
		return false;
	}

	//将socket设置为非堵塞模式
	unsigned long flag = 1;
	ioctlsocket(m_hSocket, FIONBIO, &flag);

	m_bRunning = true;
	
	m_threadListen = CreateThread(NULL, 0, ListenThreadProc, this, 0, NULL);
	m_threadSend = CreateThread(NULL, 0, SendThreadProc, this, 0, NULL);
	m_threadRecv = CreateThread(NULL, 0, RecvThreadProc, this, 0, NULL);

	return m_bRunning;
}

const FS_PACKET* CSocketServer::GetPacket(int &nClient)
{
	FS_PACKET* pPacket = NULL;
	EnterCriticalSection(&csRecv);
	if(m_listPacketRecv.size() > 0)
	{
		pPacket = m_listPacketRecv.front().first;
		nClient = m_listPacketRecv.front().second;
		m_listPacketRecv.pop_front();
	}
	LeaveCriticalSection(&csRecv);

	return pPacket;
}

// 释放包
void CSocketServer::DeletePacket(const FS_PACKET** pPacket)
{
	delete *pPacket;
	*pPacket = NULL;
}

bool CSocketServer::SendPacket(const FS_PACKET* pPacket, int nClient)
{
	if(m_aClient[nClient].sock == INVALID_SOCKET)
		return false;

	FS_PACKET* ptr = (FS_PACKET*)new char[pPacket->nSize];
	memcpy(ptr, pPacket, pPacket->nSize);
	EnterCriticalSection(&csSend);
	m_listPacketSend.push_back( pair< FS_PACKET*, int >(ptr, nClient));
	LeaveCriticalSection(&csSend);
	return true;
}

bool CSocketServer::BroadcastPacket(const FS_PACKET* pPacket, int nExcludeClient)
{
	for(int i = 0; i < m_nMaxClient; i++)
	{
		if(m_aClient[i].sock != INVALID_SOCKET)
		{
			if (i != nExcludeClient)
			{
				SendPacket(pPacket, i);
			}
		}
	}
	return true;
}

void CSocketServer::DisconnectClient(int nClient)
{
	closesocket(m_aClient[nClient].sock);
	m_aClient[nClient].sock = INVALID_SOCKET;
}

DWORD CSocketServer::ListenThreadProc(LPVOID pParam)
{
	CSocketServer* pSocketServer = (CSocketServer*)pParam;
	timeval tv;
	int ret;
	fd_set set;

	while(pSocketServer->m_bRunning)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 20000;
		FD_ZERO(&set);
		FD_SET(pSocketServer->m_hSocket, &set);
		ret = select(pSocketServer->m_hSocket+1, &set, NULL, NULL, &tv);
		if (ret > 0) 
		{
			if (FD_ISSET(pSocketServer->m_hSocket, &set) )
			{
				pSocketServer->AcceptConnect();
			}
		}
		else if(ret == 0)//timeout
		{
		}
		else//failed
		{
			Sleep(1);
		}
	}

	OutputDebugString("Listen thread exit normally\n");

	return 0;
}


void CSocketServer::AcceptConnect()
{
	int i;
	for(i = 0; i < m_nMaxClient; i++)
	{
		if (m_aClient[i].sock == INVALID_SOCKET) 
		{
			break;
		}
	}

	if (i >= m_nMaxClient) 
	{
		OutputDebugString("Server full!\n");
		return;
	}

	sockaddr_in		saClient; // Socker Address of client
	int nLen = sizeof( sockaddr );

	SOCKET sock = accept( m_hSocket, (sockaddr*)&saClient, &nLen );

	if ( sock < 0)
	{
		OutputDebugString("accept failure\n");
	}
	else
	{
		OutputDebugString("connection accepted!\n");
		m_aClient[i].sock = sock;
		m_nCurClient++;
	}
}

DWORD CSocketServer::SendThreadProc(LPVOID pParam)
{
	CSocketServer* pSocketServer = (CSocketServer*)pParam;
	while (pSocketServer->m_bRunning) 
	{
		pSocketServer->Send();
		Sleep(1);
	}

	return 0;
}

void CSocketServer::Send()
{
	FS_PACKET* pPacket = NULL;
	int nClient = 0;
	EnterCriticalSection(&csSend);
	if( m_listPacketSend.size() > 0)
	{
		pPacket = m_listPacketSend.front().first;
		nClient = m_listPacketSend.front().second;
		m_listPacketSend.pop_front();
	}
	LeaveCriticalSection(&csSend);

	if(pPacket != NULL)
	{
		if( m_aClient[nClient].sock != INVALID_SOCKET)
			send(m_aClient[nClient].sock, (const char*)pPacket, pPacket->nSize, 0);
		delete pPacket;
	}
}

DWORD CSocketServer::RecvThreadProc(LPVOID pParam)
{
	CSocketServer* pSocketServer = (CSocketServer*)pParam;
	while (pSocketServer->m_bRunning) 
	{
		pSocketServer->Recv();
		Sleep(1);
	}

	return 0;
}

void CSocketServer::Recv()
{
	for(int i = 0; i < m_nMaxClient; i++)
	{
		if(m_aClient[i].sock != INVALID_SOCKET)
		{
			int nLen = recv(m_aClient[i].sock, m_aClient[i].buffIn + m_aClient[i].nBytesRemain, MAX_PACKET_SIZE - m_aClient[i].nBytesRemain, 0 );
			if( nLen > 0 )
			{
				m_aClient[i].nBytesRemain += nLen;

				while( m_aClient[i].nBytesRemain >= sizeof(FS_PACKET)
					&& m_aClient[i].nBytesRemain >=  ((FS_PACKET*)m_aClient[i].buffIn)->nSize
					)
				{
					FS_PACKET* pPacket = (FS_PACKET*)m_aClient[i].buffIn;
					// if (pPacket->wHeader != 0xFFFF) break;
					FS_PACKET* ptr = (FS_PACKET*) new char[pPacket->nSize];
					memcpy(ptr, pPacket, pPacket->nSize);
					EnterCriticalSection(&csRecv);
					m_listPacketRecv.push_back( pair< FS_PACKET*, int >( ptr, i));
					LeaveCriticalSection(&csRecv);

					m_aClient[i].nBytesRemain -= pPacket->nSize;
					if(m_aClient[i].nBytesRemain)
						memmove(m_aClient[i].buffIn, m_aClient[i].buffIn + pPacket->nSize, m_aClient[i].nBytesRemain);
				}
			}
			else
			{
				int nErr = WSAGetLastError();
				if(nErr != WSAEWOULDBLOCK)
				{
					FS_PACKET pack;
					pack.nSize = sizeof(FS_PACKET);
					pack.nID = PID_SOCKET_DISCONNECT;
					FS_PACKET* pPacket = &pack;

					FS_PACKET* ptr = (FS_PACKET*) new char[pPacket->nSize];
					memcpy(ptr, pPacket, pPacket->nSize);
					EnterCriticalSection(&csRecv);
					m_listPacketRecv.push_back( pair< FS_PACKET*, int >( ptr, i));
					LeaveCriticalSection(&csRecv);

					closesocket(m_aClient[i].sock);
					m_aClient[i].sock = INVALID_SOCKET;
					m_nCurClient--;
				}
			}

		}
	}
}

//判断某个客户端是否连接状态
bool CSocketServer::IsConnect(int nClient)
{
	if (nClient < 0 || nClient >= m_nMaxClient)
		return false;

	return (m_aClient[nClient].sock != INVALID_SOCKET);
}


// 获得未处理的消息数
DWORD CSocketServer::GetUnProcessPackNum()
{
	DWORD dwNum;
	EnterCriticalSection(&csRecv);
	dwNum = m_listPacketRecv.size();
	LeaveCriticalSection(&csRecv);
	return dwNum;
}

// 获取当前客户端连接的数量
DWORD CSocketServer::GetCurClientNum()
{
	return m_nCurClient;
}



