
#ifndef AUTHSOCKET_H
#define AUTHSOCKET_H

#include "InfoCore.h"
#include "../shared/Network/MNetSocket.h"

#pragma pack(1) /**< 设定字节对齐  */ 
////////////////////////////////////////////////////////////////
/// @struct FS_PACKET
/// @brief  压力测试包结构体
////////////////////////////////////////////////////////////////
struct FS_PACKET
{
	FS_PACKET() : wHeader(0xFFFF), nSize(0), nID(0xFFFFFFFF)
	{

	}

	WORD wHeader;//WORD2字节
	DWORD nSize; //DWORD4字节
	DWORD nID;
	DWORD dwClientSendTime;		// 客户端发送的时间
};


#pragma pack()

class AuthSocket : public MNetSocket
{
	public:

		///////////////////////////////////////////////////
		// Netcore shit
		//////////////////////////
		AuthSocket(SOCKET fd);
		~AuthSocket();

		void OnRead();

		///////////////////////////////////////////////////
		// Client Packet Handlers
		//////////////////////////
		void _HandlePacket();
		void HandlePress(FS_PACKET* packet);             //压力测试
		void HandlePass(FS_PACKET* packet);              //判断密码是否通过
		void HandleRealmlist(FS_PACKET* packet);         //处理Realmlist
		void HandleTransferCancel(FS_PACKET* packet);    //断开连接

		///////////////////////////////////////////////////
		// Server Packet Builders
		//////////////////////////

		//inline void SendPacket(const uint8* data, const uint16 len) { Send(data, len); }
		void OnDisconnect();
		inline time_t GetLastRecv() { return last_recv; }
		bool removedFromSet;
		inline uint32 GetAccountID() { return m_account ? m_account->AccountId : 0; }

	protected:

		Account* m_account;
		bool m_authenticated;
		//////////////////////////////////////////////////
		// Session Key
		/////////////////////////
		time_t last_recv;
		uint32 m_recvNum;
};

#endif
