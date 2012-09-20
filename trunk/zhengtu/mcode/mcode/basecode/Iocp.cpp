#include "Iocp.h"

// [ranqd] �����շ���¼
// [ranqd] �����շ���¼
long g_RecvSize = 0;
long g_SendSize = 0;
long g_WantSendSize = 0;

// [ranqd] ��ȡϵͳ��CPU����
DWORD GetCPUCount()
{
	SYSTEM_INFO  sysinfo;
	GetSystemInfo( &sysinfo );
	return sysinfo.dwNumberOfProcessors;
}


// Iocp���ݽ����߳�
void CIocpRecvThread::run()
{
	BOOL    bResult;
	DWORD   dwNumRead;
	LPOverlappedData lpOverlapped = NULL;

	CSocket* pSock = NULL;
	printf("IOCP�߳̽�����\n");
	while( !isFinal() )
	{
		bResult = GetQueuedCompletionStatus( CIocp::getInstance().m_ComPort, &dwNumRead, (LPDWORD)&pSock, (LPOVERLAPPED*)&lpOverlapped, INFINITE );
		if( bResult == FALSE )
		{
			if( lpOverlapped == NULL )
			{
				printf("Iocp���������أ�\n");
			}
			continue;
		}
		else if( pSock == NULL )
		{
			printf("Iocp���ؿ�ָ�룡\n");
			continue;
		}
		else if( dwNumRead == 0 || dwNumRead == 0xFFFFFFFF )
		{
			continue;
		}
		else if( dwNumRead == 0xFFFFFFFF - 1 )// Ͷ�ݵ�һ��������
		{
			pSock->ReadByte( sizeof(PACK_HEAD) );
		}
		else if( lpOverlapped->OperationType == IO_Read )
		{
			{
				pSock->RecvData( dwNumRead );
			}
		}
		else if( lpOverlapped->OperationType == IO_Write )
		{
			{
				pSock->SendData( dwNumRead );
			}
		}
	}
}

CIocp* CIocp::instance = NULL;


CIocp::CIocp()
{
	m_ComPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}
void CIocp::start()
{
	// [ranqd] ��ȡϵͳCPU����������CPU * 2 + 2�����ݽ����߳�
	m_dwThreadCount = GetCPUCount() * 2 + 2;
	for(DWORD i = 0; i < m_dwThreadCount; i++)
	{
		CIocpRecvThread* pThread = new CIocpRecvThread();
		pThread->start();
		m_RecvThreadList.push_back(pThread);
	}
}

CIocp::~CIocp()
{
	for( std::vector<CIocpRecvThread*>::iterator it = m_RecvThreadList.begin(); 
		it != m_RecvThreadList.end(); it++)
	{
		(*it)->final();
		delete (*it);
	}
	m_RecvThreadList.clear();
}

void CIocp::BindIocpPort(HANDLE hIo, CSocket* key)
{
	::CreateIoCompletionPort( (HANDLE)hIo, m_ComPort, (ULONG_PTR)key, m_dwThreadCount );
	// Ͷ��һ������������
	::PostQueuedCompletionStatus( m_ComPort, 0xFFFFFFFF - 1, (ULONG_PTR)key, NULL );
	printf("IOCP�˿ڰ󶨣�\n");
}

void CIocp::PostStatus(CSocket* key, LPOverlappedData lpOverLapped)
{
	::PostQueuedCompletionStatus( m_ComPort, 0xFFFFFFFF, (ULONG_PTR)key, (LPOVERLAPPED)lpOverLapped );
}

void CIocp::UpdateNetLog()
{
	// [ranqd] ÿ�����һ��������������
	char OutStr[MAX_PATH * 10];
	char RecStr[MAX_PATH];
	char SendStr[MAX_PATH];
	char WantSendStr[MAX_PATH];
	char szName[MAX_PATH];
	char szCode[MAX_PATH];

	if( g_WantSendSize < 1024 )
	{
		sprintf( WantSendStr, "%dB", g_WantSendSize );
	}
	else if( g_WantSendSize < 1024 * 1024 )
	{
		sprintf( WantSendStr, "%dKB", g_WantSendSize / 1024 );
	}
	else if( g_WantSendSize < 1024 * 1024 * 1024 )
	{
		sprintf( WantSendStr, "%dMB", g_WantSendSize / 1024 / 1024 );
	}
	else
	{
		sprintf( WantSendStr, "%dGB", g_WantSendSize / 1024 / 1024 / 1024 );
	}

	if( g_RecvSize < 1024 )
	{
		sprintf( RecStr, "%dB", g_RecvSize );
	}
	else if( g_RecvSize < 1024 * 1024 )
	{
		sprintf( RecStr, "%dKB", g_RecvSize / 1024 );
	}
	else if( g_RecvSize < 1024 * 1024 * 1024 )
	{
		sprintf( RecStr, "%dMB", g_RecvSize / 1024 / 1024 );
	}
	else
	{
		sprintf( RecStr, "%dGB", g_RecvSize / 1024 / 1024 / 1024 );
	}
	if( g_SendSize < 1024 )
	{
		sprintf( SendStr, "%dB", g_SendSize );
	}
	else if( g_SendSize < 1024 * 1024 )
	{
		sprintf( SendStr, "%dKB", g_SendSize / 1024 );
	}
	else if( g_SendSize < 1024 * 1024 * 1024 )
	{
		sprintf( SendStr, "%dMB", g_SendSize / 1024 / 1024 );
	}
	else
	{
		sprintf( SendStr, "%dGB", g_SendSize / 1024 / 1024 / 1024 );
	}
	sprintf( OutStr, "�գ�%s/s ����%s/s %s/s\n", RecStr, SendStr,WantSendStr);
	g_RecvSize = 0;
	g_SendSize = 0;
	g_WantSendSize = 0;
	GetModuleFileName(NULL,szName,sizeof(szName));
	_snprintf(szCode,sizeof(szCode),"HydTest - %s - %s",PathFindFileName(szName),OutStr);
	SetConsoleTitle(szCode);
}