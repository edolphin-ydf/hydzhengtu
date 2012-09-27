#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include <list>
using namespace std;

#include "FSSocketPacket.h"
#include "FSServer.h"

struct FS_API SLOT
{
	SOCKET sock;
	char buffIn[MAX_PACKET_SIZE];
	int nBytesRemain;

	SLOT()
	{
		sock = INVALID_SOCKET;
		nBytesRemain = 0;
	}
};

class FS_API CSocketServer :public FSServer
{
public:
	CSocketServer();
	~CSocketServer();
public:
	//��ʼ��
	bool Initialize(int nMaxClient, short nPort);

	//��ȡһ���Ѿ��յ��ķ��
	const FS_PACKET* GetPacket(int &nClient);

	//����һ�������ָ���Ŀͻ���
	bool SendPacket(const FS_PACKET* pPacket, int nClient);

	//���ͷ�����������ӿͻ�
	bool BroadcastPacket(const FS_PACKET* pPacket, int nExcludeClient = -1);

	//�Ͽ��ͻ���
	void DisconnectClient(int nClient);

	//�ж�ĳ���ͻ����Ƿ�����״̬
	bool IsConnect(int nClient);

	// ���δ�������Ϣ��
	DWORD GetUnProcessPackNum();

	// ��ȡ��ǰ�ͻ������ӵ�����
	DWORD GetCurClientNum();

	// �ͷŰ�
	void DeletePacket(const FS_PACKET** pPacket);

protected:
	//���ͺͽ��յ��̺߳���
	static DWORD WINAPI SendThreadProc(LPVOID pParam);
	static DWORD WINAPI RecvThreadProc(LPVOID pParam);
	static DWORD WINAPI ListenThreadProc(LPVOID pParam);
	void Send();
	void Recv();
	void AcceptConnect();

	CRITICAL_SECTION csSend, csRecv;
	list< pair< FS_PACKET*, int > > m_listPacketSend;
	list< pair< FS_PACKET*, int > > m_listPacketRecv;
	SOCKET m_hSocket;
	short m_nServerPort;
	SLOT *m_aClient;
	int m_nMaxClient;
	int m_nCurClient;

private:

	bool m_bRunning;
	HANDLE m_threadSend;
	HANDLE m_threadRecv;
	HANDLE m_threadListen;
};
#endif//__SOCKET_SERVER_H__
