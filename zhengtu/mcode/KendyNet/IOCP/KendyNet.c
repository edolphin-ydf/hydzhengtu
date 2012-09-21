#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include "KendyNet.h"
#include "stdio.h"
enum
{
	IO_RECVREQUEST = 1<<1,   //应用层接收请求
	IO_SENDREQUEST = 1<<3,   //应用层发送请求
	IO_RECVFINISH =  1<<2,//接收完成
	IO_SENDFINISH =  1<<4,   //发送完成
};


enum
{
	IO_RECV = (1<<1) + (1<<2),
	IO_SEND = (1<<3) + (1<<4),
	IO_REQUEST = (1<<1) + (1<<3),
};

int32_t    InitNetSystem()
{
	int32_t nResult;
	WSADATA wsaData;
	nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (NO_ERROR != nResult)
	{
		printf("\nError occurred while executing WSAStartup().");
		return -1; //error
	}
	return 0;
}
void   CleanNetSystem()
{
	 WSACleanup();
}


HANDLE CreateNetEngine(DWORD NumberOfConcurrentThreads)
{
	HANDLE CompletePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, NumberOfConcurrentThreads);
	if(NULL == CompletePort)
	{
		printf("\nError occurred while creating IOCP: %d.", WSAGetLastError());
	}
	return CompletePort;
}

void   CloseNetEngine(HANDLE CompletePort)
{
	CloseHandle(CompletePort);
}

/* return:
*  >  0 :bytestransfer
*  == 0 :WSA_IO_PENDING
*  <  0 :error or socket close
*/
static int32_t raw_Send(Socket_t s,struct OverLapContext *overLapped,uint32_t *err_code)
{
	uint32_t dwFlags = 0;
	uint32_t dwBytes = 0;
	if(SOCKET_ERROR == WSASend(s->sock, overLapped->wbuf, overLapped->buf_count, 
		&dwBytes, dwFlags, (OVERLAPPED*)overLapped, NULL))
	{
		if(err_code)
			*err_code = WSAGetLastError();
		return IO_ERROR;
	}	
	else 
		return dwBytes;
}

static int32_t raw_Recv(Socket_t s,struct OverLapContext *overLapped,uint32_t *err_code)
{
	uint32_t dwFlags = 0;
	uint32_t dwBytes = 0;
	int32_t ret = 0;
	if(SOCKET_ERROR == WSARecv(s->sock, overLapped->wbuf, overLapped->buf_count, 
		&dwBytes, &dwFlags, (OVERLAPPED*)overLapped, NULL))
	{
		if(err_code)
			*err_code = WSAGetLastError();
		return IO_ERROR;
	}	
	else 
		return dwBytes;
}

extern uint32_t iocp_count;
typedef void (*CallBack)(struct Socket*,struct OverLapContext*,int32_t,uint32_t);
int    RunEngine(HANDLE CompletePort,DWORD timeout)
{

	int32_t bytesTransfer;
	Socket_t       socket;
	struct OverLapContext *overLapped = 0;
	uint32_t lastErrno = 0;
	BOOL bReturn;
	CallBack call_back;
	uint32_t ms;
	uint32_t tick = GetTickCount();
	uint32_t _timeout = tick + timeout;
	do
	{
		ms = _timeout - tick;
		call_back = 0;
		lastErrno = 0;
		bReturn = GetQueuedCompletionStatus(
			CompletePort,&bytesTransfer,
			(LPDWORD)&socket,
			(OVERLAPPED**)&overLapped,ms);
		
		if(FALSE == bReturn && !overLapped)// || socket == NULL || overLapped == NULL)
		{
			++iocp_count;
			break;
		}
		if(0 == bytesTransfer)
		{
			//连接中断或错误
			lastErrno = WSAGetLastError();			
			if(bReturn == TRUE && lastErrno == 0)
			{
			}
			else
			{
				if(overLapped->m_Type & IO_RECV)
					call_back = socket->RecvFinish;	
				else
					call_back = socket->SendFinish;
				if(FALSE == bReturn)
					bytesTransfer = -1;
			}

		}
		else
		{
			//++iocp_count;
#ifdef _WIN7
			if(overLapped->m_Type & IO_REQUEST)
			{
				overLapped->m_Type = overLapped->m_Type << 1;
				if(overLapped->m_Type  & IO_RECVFINISH)
					bytesTransfer = raw_Recv(socket,overLapped,&lastErrno);
				else if(overLapped->m_Type  & IO_SENDFINISH)
					bytesTransfer = raw_Send(socket,overLapped,&lastErrno);
				else
				{
					//出错
					continue;
				}

				//if(bytesTransfer == 0)
				//	continue;//WSA_IO_PENDING
			}
#endif		
			if(overLapped->m_Type & IO_RECVFINISH)
				call_back = socket->RecvFinish;
			else if(overLapped->m_Type & IO_SENDFINISH)
				call_back = socket->SendFinish;
			else
			{
				//出错
				continue;
			}
		}

		if(call_back)
		{
			call_back(socket,overLapped,bytesTransfer,lastErrno);
		}
		tick = GetTickCount();
	}while(tick < _timeout);
	return 0;
}

int    Bind2Engine(HANDLE CompletePort,Socket_t socket)
{
	HANDLE hTemp;
	if(!socket->RecvFinish || !socket->SendFinish)
		return -1;
	hTemp = CreateIoCompletionPort((HANDLE)socket->sock, CompletePort,(ULONG_PTR)socket, 0);
	if (NULL == hTemp)
		return -1;
	socket->complete_port = CompletePort;
#ifdef _WIN7
	SetFileCompletionNotificationModes((HANDLE)socket->sock,FILE_SKIP_COMPLETION_PORT_ON_SUCCESS);
#endif
	return 0;
}


#ifdef _WIN7
int32_t    WSA_Send(Socket_t socket,struct OverLapContext *OverLap,int32_t now,uint32_t *err_code)
{
	if(!socket->complete_port)
		return UNBIND2ENGINE;
	ZeroMemory(&OverLap->m_overLapped, sizeof(OVERLAPPED));
	if(!now)
	{
		OverLap->m_Type = IO_SENDREQUEST;
		PostQueuedCompletionStatus(socket->complete_port,1,(ULONG_PTR)socket,(OVERLAPPED*)OverLap);
		if(err_code)
			*err_code = WSA_IO_PENDING;
		return IO_ERROR;
	}
	else
	{
		OverLap->m_Type = IO_SENDFINISH;
		return raw_Send(socket,OverLap,err_code);
	}
}

int32_t    WSA_Recv(Socket_t socket,struct OverLapContext *OverLap,int32_t now,uint32_t *err_code)
{
	if(!socket->complete_port)
		return UNBIND2ENGINE;
	ZeroMemory(&OverLap->m_overLapped, sizeof(OVERLAPPED));
	if(!now)
	{
		OverLap->m_Type = IO_RECVREQUEST;
		PostQueuedCompletionStatus(socket->complete_port,1,(ULONG_PTR)socket,(OVERLAPPED*)OverLap);
		if(err_code)
			*err_code = WSA_IO_PENDING;
		return IO_ERROR;
	}
	else
	{	
		OverLap->m_Type = IO_RECVFINISH;
		return raw_Recv(socket,OverLap,err_code);
	}
}
#else
//非VC编译的now参数被忽略
int32_t    WSA_Send(Socket_t socket,struct OverLapContext *OverLap,int32_t now,uint32_t *err_code)
{
	int32_t bytetransfer = 0;
	if(!socket->complete_port)
		return UNBIND2ENGINE;
	ZeroMemory(&OverLap->m_overLapped, sizeof(OVERLAPPED));
	OverLap->m_Type = IO_SENDFINISH;
	bytetransfer =  raw_Send(socket,OverLap,err_code);
	if(bytetransfer > 0)
	{
		bytetransfer = -1;
		*err_code = WSA_IO_PENDING;
	}
	return bytetransfer;
}

int32_t    WSA_Recv(Socket_t socket,struct OverLapContext *OverLap,int32_t now,uint32_t *err_code)
{
	int32_t bytetransfer = 0;
	if(!socket->complete_port)
		return UNBIND2ENGINE;
	ZeroMemory(&OverLap->m_overLapped, sizeof(OVERLAPPED));

	OverLap->m_Type = IO_RECVFINISH;
	bytetransfer = raw_Recv(socket,OverLap,err_code);
	if(bytetransfer > 0)
	{
		bytetransfer = -1;
		*err_code = WSA_IO_PENDING;
	}
	return bytetransfer;
}


#endif