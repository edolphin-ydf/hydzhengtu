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
#define MAX_AUTH_CMD 3

typedef void (AuthSocket::*AuthHandler)();
static AuthHandler Handlers[MAX_AUTH_CMD] =
{
	&AuthSocket::HandlePass,                // 0
	&AuthSocket::HandleRealmlist,			// 1
	&AuthSocket::HandleTransferCancel,		// 2
};
struct FS_PACKET
{
	FS_PACKET() : wHeader(0xFFFF), nSize(0), nID(0xFFFFFFFF)
	{

	}

	WORD wHeader;
	DWORD nSize;
	DWORD nID;
};

AuthSocket::AuthSocket(SOCKET fd) : Socket(fd, 32768, 4096)
{
	m_authenticated = false;
	m_account = NULL;
	last_recv = time(NULL);
	removedFromSet = false;
	_authSocketLock.Acquire();
	_authSockets.insert(this);
	_authSocketLock.Release();
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
	if(readBuffer.GetContiguiousBytes() < 1)
		return;

	uint8 Command = *(uint8*)readBuffer.GetBufferStart();
	last_recv = UNIXTIME;
	if(Command < MAX_AUTH_CMD && Handlers[Command] != NULL)
		(this->*Handlers[Command])();
	else
		LOG_DEBUG("AuthSocket", "Unknown cmd %u", Command);
}


void AuthSocket::HandlePass()
{
	int len = readBuffer.GetContiguiousBytes();
	char *recv = new char[len];
	bool rec = readBuffer.Read(recv,len);
	recv++;
	printf(recv);
}


void AuthSocket::HandleRealmlist()
{
	sInfoCore.SendRealms(this);
}

void AuthSocket::HandleTransferCancel()
{
	//RemoveReadBufferBytes(1,false);
	readBuffer.Remove(1);
	Disconnect();
}
