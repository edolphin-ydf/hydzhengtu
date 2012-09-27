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
		printf("WSAStartupʧ���ˣ�������Ϣ����:   %d\n",   Ret);   
		return;   
	}   

	// ����һ��I/O��ɶ˿�.   
	if   ((CompletionPort   =   CreateIoCompletionPort(INVALID_HANDLE_VALUE,   NULL,   0,   0))   ==   NULL)   
	{   
		printf(   "CreateIoCompletionPort ʧ���ˣ�������Ϣ����:   %d\n",   GetLastError());   
		return;   
	}   

	// ����ϵͳ���ж���cpu������ 
	GetSystemInfo(&SystemInfo);   

	//   ����ϵͳ���õĴ��������������̣߳�Ϊÿ�����������������߳�   
	for(i   =   0;   i   <   SystemInfo.dwNumberOfProcessors   *   2;   i++)   
	{   
		HANDLE   ThreadHandle;   

		// ����һ��������̲߳��Ҵ���һ����ɶ˿ڸ�����߳�.   
		if   ((ThreadHandle   =   CreateThread(NULL,   0,   ServerWorkerThread,   CompletionPort,   
			0,   &ThreadID))   ==   NULL)   
		{   
			printf("CreateThread()���������´��� %d\n",   GetLastError());   
			return;   
		}   
		else 
		{
			printf("������һ����ɶ˿�.\n");
		}
		//   �ر� thread��� 
		CloseHandle(ThreadHandle);   
	}   

	//   ����һ�������׽��� 
	if   ((Listen   =WSASocket(AF_INET,   SOCK_STREAM,   0,   NULL,0,WSA_FLAG_OVERLAPPED))   ==   INVALID_SOCKET)   
	{   
		printf("WSASocket() ���������´��� %d\n",   WSAGetLastError());   
		return;   
	}
	else     
	{
		printf("���������׽��ֳɹ�\n");
	}
	InternetAddr.sin_family   =   AF_INET;   
	InternetAddr.sin_addr.s_addr   =   htonl(INADDR_ANY);   
	InternetAddr.sin_port   =   htons(PORT);   

	if   (bind(Listen,   (PSOCKADDR)   &InternetAddr,   sizeof(InternetAddr))   ==   SOCKET_ERROR)   
	{   
		printf("bind()�˿ڻ�IPʱ���������´��� %d\n",   WSAGetLastError());   
		return;   
	}   
	else
	{
		printf("�󶨶˿�%d�ɹ�\n",PORT);
	} 

	// ׼��socket ��������   
	if   (listen(Listen,   5)   ==   SOCKET_ERROR)   
	{   
		printf("listen() ���������´���   %d\n",   WSAGetLastError());   
		return;   
	}   
	else
	{
		printf("Ԥ����ɹ�����ʼ�ڶ˿� %d ������...\n",PORT);
	} 

	//�������Ӳ��ҽ�����ɶ˿ڴ��� 
	while(TRUE)   
	{   
		if   ((Accept   =   WSAAccept(Listen,   NULL,   NULL,   NULL,   0))   ==   SOCKET_ERROR)   
		{   
			printf("WSAAccept()   ���������´���   %d\n",   WSAGetLastError());   
			return;   
		}   

		// ����һ���׽�����Ϣ�ṹ��ȥ��ϵ����socket   
		if   ((PerHandleData   =   (LPPER_HANDLE_DATA)   GlobalAlloc(GPTR,     
			sizeof(PER_HANDLE_DATA)))   ==   NULL)   
		{   
			printf("GlobalAlloc()   ���������´���   %d\n",   GetLastError());   
			return;   
		}   

		// �����ܵ����׽�����ԭʼ����ɶ˿���ϵ����.   

		printf("����Ϊ   %d   ��socket��������\n",   Accept);   
		PerHandleData->Socket   =   Accept;   

		if   (CreateIoCompletionPort((HANDLE)   Accept,   CompletionPort,   (DWORD)   PerHandleData,   
			0)   ==   NULL)   
		{   
			printf("CreateIoCompletionPort   ���������´���   %d\n",   GetLastError());   
			return;   
		}   

		//   ����ÿһ��I/O �׽�����Ϣ�ṹ��ȥ�����汻���õ� to   associate   with   the     
		//   WSARecv ����.   
		if   ((PerIoData   =   (LPPER_IO_OPERATION_DATA)   GlobalAlloc(GPTR,                     sizeof(PER_IO_OPERATION_DATA)))   ==   NULL)   
		{   
			printf("GlobalAlloc() ���������´��� %d\n",   GetLastError());   
			return;   
		}   
		else
		{
			printf("������һ������\n");
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
				printf("WSARecv() ���������´��� %d\n",   WSAGetLastError());   
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
			printf("GetQueuedCompletionStatus   ���������´��� %d\n",   GetLastError());   
			continue;
		}   

		//���ȼ��һ��ȥ�׽��ֿ��Ƿ����Ϸ����˴�������������˴���͹ر��׽�
		//�ֲ���������׽������ӵ� SOCKET_INFORMATION�ṹ��Ϣ�� 
		if   (BytesTransferred   ==   0)   
		{   
			printf("���ڹر�socket   %d\n",   PerHandleData->Socket);   

			if   (closesocket(PerHandleData->Socket)   ==   SOCKET_ERROR)   
			{   
				printf("closesocket()   ���������´��� %d\n",   WSAGetLastError());   
				return   0;   
			}   

			GlobalFree(PerHandleData);   
			GlobalFree(PerIoData);   
			continue;   
		}   
		//������ BytesRECV�ֶε���0�������ζ��һ�� WSARecv���øո���������Դ���ɵ�WSARecv()������
		//��BytesTransferredֵ���� BytesRECV�ֶ� 
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
			//��������һ�� WSASend()����
			//��ȻWSASend()���� gauranteedȥ���������ֽڵ�����
			//�������� WSASend()����ֱ�������յ����ֽڱ����� 
			ZeroMemory(&(PerIoData->Overlapped),   sizeof(OVERLAPPED));   

			PerIoData->DataBuf.buf   =   PerIoData->Buffer   +   PerIoData->BytesSEND;   
			PerIoData->DataBuf.len   =   PerIoData->BytesRECV   -   PerIoData->BytesSEND;   

			if   (WSASend(PerHandleData->Socket,   &(PerIoData->DataBuf),   1,   &SendBytes,   0,   
				&(PerIoData->Overlapped),   NULL)   ==   SOCKET_ERROR)   
			{   
				if   (WSAGetLastError()   !=   ERROR_IO_PENDING)   
				{   
					printf("WSASend() ���������´���   %d\n",   WSAGetLastError());   
					return   0;   
				}   
			}   
		}   
		else   
		{   
			PerIoData->BytesRECV   =   0;   
			//����û�и�����ֽڷ��͹�ȥ����post����һ��WSARecv()���� 
			Flags   =   0;   
			ZeroMemory(&(PerIoData->Overlapped),   sizeof(OVERLAPPED));   

			PerIoData->DataBuf.len   =   DATA_BUFSIZE;   
			PerIoData->DataBuf.buf   =   PerIoData->Buffer;   

			if   (WSARecv(PerHandleData->Socket,   &(PerIoData->DataBuf),   1,   &RecvBytes,   &Flags,   
				&(PerIoData->Overlapped),   NULL)   ==   SOCKET_ERROR)   
			{   
				if   (WSAGetLastError()   !=   ERROR_IO_PENDING)   
				{   
					printf("WSARecv() ���������´���   %d\n",   WSAGetLastError());   
					return   0;   
				}   
			}   
		}   
	}   
}  



