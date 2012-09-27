#pragma once

#include "FSSocketPacket.h"

class FSServer
{
public:
	virtual ~FSServer(void){}

	//初始化
	virtual bool Initialize(int nMaxClient, short nPort) = 0;

	//获取一个已经收到的封包
	virtual const FS_PACKET* GetPacket(int &nClient) = 0;

	//发送一个封包给指定的客户端
	virtual bool SendPacket(const FS_PACKET* pPacket, int nClient) = 0;

	//发送封包给所有连接客户
	virtual bool BroadcastPacket(const FS_PACKET* pPacket, int nExcludeClient = -1) = 0;

	//断开客户端
	virtual void DisconnectClient(int nClient) = 0;

	//判断某个客户端是否连接状态
	virtual bool IsConnect(int nClient) = 0;

	// 获得未处理的消息数
	virtual DWORD GetUnProcessPackNum() = 0;

	// 获取当前客户端连接的数量
	virtual DWORD GetCurClientNum() = 0;

	// 释放包
	virtual void DeletePacket(const FS_PACKET** pPacket) = 0;
};
