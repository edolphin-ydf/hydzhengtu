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
	//初始化
	bool Initialize(int nMaxClient, short nPort);

	//获取一个已经收到的封包
	const FS_PACKET* GetPacket(int &nClient);

	//发送一个封包给指定的客户端
	bool SendPacket(const FS_PACKET* pPacket, int nClient);

	//发送封包给所有连接客户
	bool BroadcastPacket(const FS_PACKET* pPacket, int nExcludeClient = -1);

	//断开客户端
	void DisconnectClient(int nClient);

	//判断某个客户端是否连接状态
	bool IsConnect(int nClient);

	// 获得未处理的消息数
	DWORD GetUnProcessPackNum();

	// 获取当前客户端连接的数量
	DWORD GetCurClientNum();

	// 释放包
	void DeletePacket(const FS_PACKET** pPacket);

protected:
	//发送和接收的线程函数
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
