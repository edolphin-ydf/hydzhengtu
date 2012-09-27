#pragma once

#include "FSSocketPacket.h"

class FS_API FSClient
{
public:
	FSClient(int len = 0){  m_PacketSize=len; }
	virtual ~FSClient(void);

	// 连接服务器，之前必须调用Initialize
	virtual bool Connect(const char* lpszServerIP, unsigned short nPort) = 0;

	// 断开连接
	virtual void Disconnect() = 0;

	// 从消息缓冲区取出一个消息，使用後需delete该消息，如果缓冲区没有消息则返回NULL
	virtual const char* GetPacket(int &len) = 0;

	// 发送一个消息，bToSelf == TRUE 则不通过网络而直接发给自己，默认是通过网络传送
	virtual bool SendPacket(const char* data,int len) = 0;

	//判断是否和服务器处于连接状态
	virtual bool IsConnect() = 0;

	// 释放包
	virtual void DeletePacket(const char** pPacket) = 0;

protected:
	int m_PacketSize;
};
