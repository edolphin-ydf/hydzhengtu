#ifndef SCOKET_CLIENT_H
#define SCOKET_CLIENT_H

#include "FSSocketpacket.h"
#include "FSClient.h"

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <list>
using namespace std;

class FS_API CSocketClient :public FSClient
{
public:
	CSocketClient();
	virtual ~CSocketClient();
public:

	//////////////////////////////////////////////////////////////////////////
	//��Ҫ���ܺ���

	//���ӷ�������֮ǰ�������Initialize
	bool Connect(const char* lpszServerIP, unsigned short nPort);

	//�Ͽ�����
	void Disconnect();

	//����Ϣ������ȡ��һ����Ϣ��ʹ������delete����Ϣ�����������û����Ϣ�򷵻�NULL
	const FS_PACKET* GetPacket();

	//����һ����Ϣ��bToSelf == TRUE ��ͨ�������ֱ�ӷ����Լ���Ĭ����ͨ�����紫��
	bool SendPacket(const FS_PACKET* pPacket);

	//�ж��Ƿ�ͷ�������������״̬
	virtual bool IsConnect();

	// �ͷŰ�
	void DeletePacket(const FS_PACKET** pPacket);

protected:

	char buffIn[MAX_PACKET_SIZE];
	int nBytesRemain;
	void Recv();
	void Send();

	//������Ϣ��ThreadProc
	static DWORD WINAPI SendThreadProc(LPVOID pParam);
	//������Ϣ��ThreadProc
	static DWORD WINAPI RecvThreadProc(LPVOID pParam);

	WSAEVENT m_hEventSocket;//Socket�¼�
	//�������Ʒ��ͺͽ��ջ�������дͬ��
	CRITICAL_SECTION csSend, csRecv;
	list< FS_PACKET* > m_listPacketRecv;
	list< FS_PACKET* > m_listPacketSend;

	HANDLE 	m_threadSend;
	HANDLE	m_threadRecv;

	int   m_bRunning;		//��־�Ƿ�������

	char m_szServerAddr[MAX_PATH];
	unsigned short m_nServerPort;
	SOCKET m_hSocket;

};


#endif