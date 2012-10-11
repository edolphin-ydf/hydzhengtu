/*
 文件名 : MNetSocket.h
 创建时间 : 2012/9/28
 作者 : hyd
 功能 : 
*/
#ifndef __MNetSocket_H__
#define __MNetSocket_H__

#include "Network.h"
#include "../Auth/PacketCrypt.h"

class SERVER_DECL MNetSocket : public Socket
{
public:
	MNetSocket(SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize, bool iCrypt/*是否加密*/, uint8* Key = NULL);
	~MNetSocket();

	bool SendPacket(const uint8* Bytes, uint32 Size);//发送数据包专用接口
	virtual void OnRead();
	virtual void _HandlePacket() = 0;
private:
	PacketCrypt *_crypt;
};
#endif
