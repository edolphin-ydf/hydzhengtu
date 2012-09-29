
#ifndef AUTHSOCKET_H
#define AUTHSOCKET_H

#include "InfoCore.h"
#include "../shared/Network/MNetSocket.h"

#pragma pack(1) /**< �趨�ֽڶ���  */ 
////////////////////////////////////////////////////////////////
/// @struct FS_PACKET
/// @brief  ѹ�����԰��ṹ��
////////////////////////////////////////////////////////////////
struct FS_PACKET
{
	FS_PACKET() : wHeader(0xFFFF), nSize(0), nID(0xFFFFFFFF)
	{

	}

	WORD wHeader;//WORD2�ֽ�
	DWORD nSize; //DWORD4�ֽ�
	DWORD nID;
	DWORD dwClientSendTime;		// �ͻ��˷��͵�ʱ��
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
		void HandlePress(FS_PACKET* packet);             //ѹ������
		void HandlePass(FS_PACKET* packet);              //�ж������Ƿ�ͨ��
		void HandleRealmlist(FS_PACKET* packet);         //����Realmlist
		void HandleTransferCancel(FS_PACKET* packet);    //�Ͽ�����

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
