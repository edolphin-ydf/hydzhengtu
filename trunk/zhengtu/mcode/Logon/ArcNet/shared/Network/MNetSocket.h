/*
 �ļ��� : MCodeNetSocket.h
 ����ʱ�� : 2012/9/28
 ���� : hyd
 ���� : 
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
	MCodeNetSocket(SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize, bool iCrypt/*�Ƿ����*/, uint8* Key = NULL);
	~MCodeNetSocket();

	bool SendPacket(const uint8* Bytes, uint32 Size);//�������ݰ�ר�ýӿ�
	virtual void OnRead();
	virtual void _HandlePacket() = 0;

	void SendPing();         /**< ����PING����  */
	void HandPing();         /**< ����PING����  */
	void SendPong();
	void HandPong();
	void HandRePong();
	void SendExit();
	void HandExit();

	uint32 last_ping;        /**< ��󷢳�ping��ʱ��  */
	uint32 last_pong;        /**< ������pong��ʱ��  */
	uint32 pingtime;         /**< ��¼����ping��ʱ��  */
	uint32 latency;          /**< ���һ��pingͨ��ʱ����  */
	uint32 _id;              /**< ���ڱ�ʶ��socket�����  */
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
