#ifndef __IOCPSERVER_H__
#define __IOCPSERVER_H__

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include "CriticalSection.h"


#define INVALID_CLIENT_ID	(-1)

#define MAX_USER_NUM		3000

#define MAX_RECV_OVERLAPPED MAX_USER_NUM

#define MAX_RECV_BUFFER		4096 // 4K���ջ���

#define IoSend	1
#define IoRecv	2



struct SOverlapped : public OVERLAPPED
{
	UINT IoMode;
	u_short id;
	WSABUF WsaBuf;

	SOverlapped()
	{
		Internal=0;
		InternalHigh=0;
		Offset=0;
		OffsetHigh=0;
		hEvent=0;
		IoMode=0;
		id=(WORD)-1;
		WsaBuf.buf=NULL;
		WsaBuf.len=0;
	}
};


struct SSendOverlapped : public SOverlapped
{
	char *pBuffer;
	DWORD SentBytes;
	DWORD TotalBytes;

	SSendOverlapped()
	{
		Internal=0;
		InternalHigh=0;
		Offset=0;
		OffsetHigh=0;
		hEvent=0;
		IoMode=IoSend;
		pBuffer = NULL;
		WsaBuf.buf = NULL;
		WsaBuf.len=0;
		TotalBytes=0;
		SentBytes=0;
	}
};

struct SRecvOverlapped : public SOverlapped
{
	char buffer[MAX_RECV_BUFFER];
	SRecvOverlapped()
	{
		Internal=0;
		InternalHigh=0;
		Offset=0;
		OffsetHigh=0;
		hEvent=0;
		IoMode=IoRecv;
		WsaBuf.buf=buffer;
		WsaBuf.len=MAX_RECV_BUFFER;
		id=(WORD)-1;
	}
};

class CIOCPServer
{
public:
	CIOCPServer(void);
public:
	virtual ~CIOCPServer(void);

	// ��ʼ��
	bool Initialize(int nMaxClient, u_short nPort);

	//����һ�������ָ���Ŀͻ���
	bool SendData(u_short nClient, char* pBuffer, u_long len);
	bool SendByte(u_short nClient, SSendOverlapped*);

	//���ͷ�����������ӿͻ�
	bool SendDataToAll(char* pBuffer, u_long len);

	//���ͷ���������ͻ���
	bool SendDataToOther(u_short nExcludeClient, char* pBuffer, u_long len);

	//�Ͽ��ͻ���
	void DisconnectClient(u_short nClient);

	//�ж�ĳ���ͻ����Ƿ�����״̬
	bool IsConnect(u_short nClient);

	// ���δ�������Ϣ��
	DWORD GetUnProcessPackNum();

	// ��ȡ��ǰ�ͻ������ӵ�����
	DWORD GetCurClientNum();

protected:
	virtual int OnAccept(WORD nClient) = 0;                                    //һ���ͻ������ӽ����󣬽����ϲ�����ߵ���
	virtual int OnClose(WORD nClient) = 0;                                     //һ���ͻ��˹ر�
	virtual int OnSend(WORD nClient, char* pData, DWORD dwDataLen) = 0;        //һ�����ݷ�������¼�
	virtual int OnReceive(WORD nClient, char* pData, DWORD dwDataLen) = 0;     //���յ�һ������
	virtual int OnError(WORD nClient, int iError) = 0;                         //���ִ���

private:
	u_short InsertSocket(SOCKET sSocket);                                     //��SOCKET��������
	SOCKET GetSocket(u_short nClient);
	BOOL RemoveSocket(u_short nClient);

private:
	BOOL PostRecv(u_short nClient, SOCKET sSocket);
	BOOL CloseSocket(u_short nClient);

private:
	static UINT WINAPI ThreadFuncForAccept(void* pParam);
	static UINT WINAPI ServerWorkerThread(void* pParam);

	friend UINT WINAPI ThreadFuncForAccept(void* pParam);
	friend UINT WINAPI ServerWorkerThread(void* pParam);

private:
	HANDLE m_hCompletionPort;
	long m_nCurClientNum;
	long m_nMaxClient;
	SOCKET m_vecSocket[MAX_USER_NUM];
	CCriticalSection m_csSockets;
};


#endif 