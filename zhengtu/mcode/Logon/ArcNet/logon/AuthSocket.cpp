#include "LogonStdAfx.h"
#include <openssl/md5.h>

enum _errors
{
    CE_SUCCESS = 0x00,
    CE_IPBAN = 0x01,									 //2bd -- unable to connect (some internal problem)
    CE_ACCOUNT_CLOSED = 0x03,							 // "This account has been closed and is no longer in service -- Please check the registered email address of this account for further information.";
    CE_NO_ACCOUNT = 0x04,								 //(5)The information you have entered is not valid.  Please check the spelling of the account name and password.  If you need help in retrieving a lost or stolen password and account
    CE_ACCOUNT_IN_USE = 0x06,							 //This account is already logged in.  Please check the spelling and try again.
    CE_PREORDER_TIME_LIMIT = 0x07,
    CE_SERVER_FULL = 0x08,								//Could not log in at this time.  Please try again later.
    CE_WRONG_BUILD_NUMBER = 0x09,						 //Unable to validate game version.  This may be caused by file corruption or the interference of another program.
    CE_UPDATE_CLIENT = 0x0a,
    CE_ACCOUNT_FREEZED = 0x0c
} ;

#define AUTH_PASS 0
#define REALM_LIST 1
#define CANCEL_TRANSFER 2		// 0x34
#define MAX_AUTH_CMD 4

typedef void (AuthSocket::*AuthHandler)(FS_PACKET*);
static AuthHandler Handlers[MAX_AUTH_CMD] =
{
	&AuthSocket::HandlePress,               // 0
	&AuthSocket::HandlePass,                // 1
	&AuthSocket::HandleRealmlist,			// 2
	&AuthSocket::HandleTransferCancel,		// 3
};


AuthSocket::AuthSocket(SOCKET fd) : MNetSocket(fd, 32768, 4096,false)
{
	m_authenticated = false;
	m_account = NULL;
	last_recv = time(NULL);
	removedFromSet = false;
	_authSocketLock.Acquire();
	_authSockets.insert(this);
	_authSocketLock.Release();
	m_recvNum = 0;
}

AuthSocket::~AuthSocket()
{

}

void AuthSocket::OnDisconnect()
{
	if(!removedFromSet)
	{
		_authSocketLock.Acquire();
		_authSockets.erase(this);
		_authSocketLock.Release();
	}
}

void AuthSocket::OnRead()
{
	MNetSocket::OnRead();
	/*if(readBuffer.GetContiguiousBytes() < 1)
	return;

	uint8 Command = *(uint8*)readBuffer.GetBufferStart();
	last_recv = UNIXTIME;
	if(Command < MAX_AUTH_CMD && Handlers[Command] != NULL)
	(this->*Handlers[Command])();
	else
	LOG_DEBUG("AuthSocket", "Unknown cmd %u", Command);*/
}

void AuthSocket::_HandlePacket()
{
	FS_PACKET* packet = new FS_PACKET;
	int32 len = sizeof(FS_PACKET);
	readBuffer.Read(packet,len);
	uint32 aid = packet->nID;
	last_recv = UNIXTIME;
	if (aid < MAX_AUTH_CMD && Handlers[aid] != NULL)
	{
		(this->*Handlers[aid])(packet);
	}
	else
		printf("AuthSocket Unknown cmd %d",aid);
	delete packet;
	printf("m_recvNum:%d\n",++m_recvNum);
}

void AuthSocket::HandlePass(FS_PACKET* pac)
{
	/*int len = readBuffer.GetContiguiousBytes();
	char *recv = new char[len];
	bool rec = readBuffer.Read(recv,len);
	recv++;*/
	printf("HandlePass");
	FS_PACKET* packet = new FS_PACKET;
	packet->nID = 1;
	SendPacket((const uint8 *)packet,sizeof(FS_PACKET));
	delete packet;

	bool rec = Connect("127.0.0.1",8093);

}

void AuthSocket::HandleRealmlist(FS_PACKET* pac)
{
	//sInfoCore.SendRealms(this);
	FS_PACKET* packet = new FS_PACKET;
	packet->nID = 2;
	SendPacket((const uint8 *)packet,sizeof(FS_PACKET));
	printf("HandleRealmlist");
	delete packet;
}

void AuthSocket::HandleTransferCancel(FS_PACKET* pac)
{
	//RemoveReadBufferBytes(1,false);
	//readBuffer.Remove(1);
	FS_PACKET* packet = new FS_PACKET;
	packet->nID = 3;
	SendPacket((const uint8 *)packet,sizeof(FS_PACKET));
	printf("HandleTransferCancel");
	//Disconnect();
	delete packet;
}

void AuthSocket::HandlePress(FS_PACKET* pac)
{
	FS_PACKET* packet = new FS_PACKET;
	packet->dwClientSendTime = pac->dwClientSendTime;
	packet->nID = 0;
	SendPacket((const uint8 *)packet,sizeof(FS_PACKET));
	int atime = time(NULL);
	printf("HandlePress==%d==\n",atime);
	delete packet;
}
