#ifndef SCOKET_CLIENT_H
#define SCOKET_CLIENT_H

#include "FSSocketpacket.h"
#include "FSClient.h"

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <map>
using namespace std;

class FS_API CSocketClient :public FSClient
{
public:
	CSocketClient(int len = 0);
	virtual ~CSocketClient();
public:

	//////////////////////////////////////////////////////////////////////////
	//主要功能函数

	//连接服务器，之前必须调用Initialize
	bool Connect(const char* lpszServerIP, unsigned short nPort);

	//断开连接
	void Disconnect();

	//从消息缓冲区取出一个消息，使用後需delete该消息，如果缓冲区没有消息则返回NULL
	const char* GetPacket(int &len);

	//发送一个消息，bToSelf == TRUE 则不通过网络而直接发给自己，默认是通过网络传送
	bool SendPacket(const char* pPacket, int len);

	//判断是否和服务器处于连接状态
	virtual bool IsConnect();

	// 释放包
	void DeletePacket(const char** pPacket);

protected:

	char buffIn[MAX_PACKET_SIZE];
	int nBytesRemain;
	void Recv();
	void Send();

	//发送信息的ThreadProc
	static DWORD WINAPI SendThreadProc(LPVOID pParam);
	//接受信息的ThreadProc
	static DWORD WINAPI RecvThreadProc(LPVOID pParam);

	WSAEVENT m_hEventSocket;//Socket事件
	//用来控制发送和接收缓冲区读写同步
	CRITICAL_SECTION csSend, csRecv;
	map< char*, int > m_listPacketRecv;
	map< char*, int > m_listPacketSend;

	HANDLE 	m_threadSend;
	HANDLE	m_threadRecv;

	int   m_bRunning;		//标志是否运行中

	char m_szServerAddr[MAX_PATH];
	unsigned short m_nServerPort;
	SOCKET m_hSocket;

};


#endif