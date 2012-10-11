/*
 �ļ��� : MNetSocket.h
 ����ʱ�� : 2012/9/28
 ���� : hyd
 ���� : 
*/
#ifndef __MNetSocket_H__
#define __MNetSocket_H__

#include "Network.h"
#include "../Auth/PacketCrypt.h"

class SERVER_DECL MNetSocket : public Socket
{
public:
	MNetSocket(SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize, bool iCrypt/*�Ƿ����*/, uint8* Key = NULL);
	~MNetSocket();

	bool SendPacket(const uint8* Bytes, uint32 Size);//�������ݰ�ר�ýӿ�
	virtual void OnRead();
	virtual void _HandlePacket() = 0;
private:
	PacketCrypt *_crypt;
};
#endif
