
#ifndef AUTHSOCKET_H
#define AUTHSOCKET_H

#include "InfoCore.h"
#include "AuthStructs.h"

class LogonCommServerSocket;

class AuthSocket : public Socket
{
		friend class LogonCommServerSocket;
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

		void HandleChallenge();         //处理挑战
		void HandleProof();             //处理证据
		void HandleRealmlist();         //处理Realmlist
		void HandleReconnectChallenge();//处理连接挑战
		void HandleReconnectProof();    //处理重新证明
		void HandleTransferAccept();    //
		void HandleTransferResume();    //
		void HandleTransferCancel();    //

		///////////////////////////////////////////////////
		// Server Packet Builders
		//////////////////////////

		void SendChallengeError(uint8 Error);
		void SendProofError(uint8 Error, uint8* M2);
		ARCEMU_INLINE sAuthLogonChallenge_C* GetChallenge() { return &m_challenge; }
		ARCEMU_INLINE void SendPacket(const uint8* data, const uint16 len) { Send(data, len); }
		void OnDisconnect();
		ARCEMU_INLINE time_t GetLastRecv() { return last_recv; }
		bool removedFromSet;
		ARCEMU_INLINE uint32 GetAccountID() { return m_account ? m_account->AccountId : 0; }

	protected:

		sAuthLogonChallenge_C m_challenge;
		Account* m_account;
		bool m_authenticated;

		// BigNumbers for the SRP6 implementation
		BigNumber N; // Safe prime
		BigNumber g; // Generator
		BigNumber s; // Salt
		BigNumber v; // Verifier
		BigNumber b; // server private value
		BigNumber B; // server public value
		BigNumber rs;

		//////////////////////////////////////////////////
		// Session Key
		/////////////////////////

		BigNumber m_sessionkey;
		time_t last_recv;
};

#endif
