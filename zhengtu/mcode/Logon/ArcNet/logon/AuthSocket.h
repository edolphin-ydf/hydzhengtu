
#ifndef AUTHSOCKET_H
#define AUTHSOCKET_H

#include "InfoCore.h"
#include "AuthStructs.h"


class AuthSocket : public Socket
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

		void HandlePass();              //判断密码是否通过
		void HandleRealmlist();         //处理Realmlist
		void HandleTransferCancel();    //断开连接

		///////////////////////////////////////////////////
		// Server Packet Builders
		//////////////////////////

		inline void SendPacket(const uint8* data, const uint16 len) { Send(data, len); }
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
};

#endif
