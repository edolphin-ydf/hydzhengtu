/*
 文件名 : MCodeNetSocket.h
 创建时间 : 2012/9/28
 作者 : hyd
 功能 : 
*/
#ifndef __MCodeNetSocket_H__
#define __MCodeNetSocket_H__

#include "Network.h"
#include "../Auth/PacketCrypt.h"

enum MNetCmd{
	CMD_SERVER_PING,
	CMD_SERVER_PONG,
	CMD_SERVER_REPONG,
	CMD_SERVER_EXIT,
	CMD_SERVER_COUNT,
};

class MCodeNetSocket : public Socket
{
public:
	MCodeNetSocket(SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize, bool iCrypt/*是否加密*/, uint8* Key = NULL);
	~MCodeNetSocket();

	bool SendPacket(const uint8* Bytes, uint32 Size);//发送数据包专用接口
	virtual void OnRead();
	virtual void _HandlePacket() = 0;

	void SendPing();         /**< 发送PING心跳  */
	void HandPing();         /**< 接收PING返回  */
	void SendPong();
	void HandPong();
	void HandRePong();
	void SendExit();
	void HandExit();

	uint32 last_ping;        /**< 最后发出ping的时间  */
	uint32 last_pong;        /**< 最后接收pong的时间  */
	uint32 pingtime;         /**< 记录发出ping的时间  */
	uint32 latency;          /**< 最后一次ping通的时间间隔  */
	uint32 _id;              /**< 用于标识该socket的身份  */
private:
	void SendCmd(MNetCmd);
	PacketCrypt *_crypt;
};

typedef void (MCodeNetSocket::*MCodeNetHandler)();
static MCodeNetHandler Handlers[CMD_SERVER_COUNT] =
{
	&MCodeNetSocket::HandPing,              // 0
	&MCodeNetSocket::HandPong,              // 1
	&MCodeNetSocket::HandRePong,			// 2
	&MCodeNetSocket::HandExit,		        // 3
};

#endif
