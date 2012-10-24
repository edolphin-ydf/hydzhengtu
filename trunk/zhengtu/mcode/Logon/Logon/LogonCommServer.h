
#ifndef __LOGON_COMM_SERVER_H
#define __LOGON_COMM_SERVER_H

#include <RC4Engine.h>

class LogonCommServerSocket : public MCodeNetSocket
{
		uint32 remaining;
		uint16 opcode;
		uint32 seed;
		RC4Engine sendCrypto;
		RC4Engine recvCrypto;
	public:
		uint32 authenticated;
		bool use_crypto;

		LogonCommServerSocket(SOCKET fd);
		~LogonCommServerSocket();

		void _HandlePacket();

		void OnRead();
		void OnDisconnect();
		void OnConnect();
		void SendPacket(WorldPacket* data);
		void HandlePacket(WorldPacket & recvData);

		void HandleRegister(WorldPacket & recvData);
		void HandlePing(WorldPacket & recvData);
		void HandleSessionRequest(WorldPacket & recvData);
		void HandleSQLExecute(WorldPacket & recvData);
		void HandleReloadAccounts(WorldPacket & recvData);
		void HandleAuthChallenge(WorldPacket & recvData);
		void HandleMappingReply(WorldPacket & recvData);
		void HandleUpdateMapping(WorldPacket & recvData);
		void HandleTestConsoleLogin(WorldPacket & recvData);
		void HandleDatabaseModify(WorldPacket & recvData);
		void HandlePopulationRespond(WorldPacket & recvData);

		void RefreshRealmsPop();

		MCodeNet::Threading::AtomicCounter last_ping;
		bool removed;
		set<uint32> server_ids;
};

typedef void (LogonCommServerSocket::*logonpacket_handler)(WorldPacket &);

#endif
