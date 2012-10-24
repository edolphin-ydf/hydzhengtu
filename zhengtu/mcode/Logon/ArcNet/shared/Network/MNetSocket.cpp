
#include "MNetSocket.h"

MCodeNetSocket::MCodeNetSocket( SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize, bool iCrypt , uint8* Key)
	: Socket(fd,sendbuffersize,recvbuffersize)
{
	if(iCrypt)
		_crypt = new PacketCrypt(Key);
	else
		_crypt = NULL;
	last_ping = last_pong = (uint32)UNIXTIME;
	pingtime = 0;
	latency = 0;
	_id = 0;
}


MCodeNetSocket::~MCodeNetSocket()
{
	if (_crypt != NULL)
	{
		delete _crypt;
		_crypt = NULL;
	}
}

bool MCodeNetSocket::SendPacket( const uint8* Bytes, uint32 Size )
{
	writeBuffer.Write(&Size,sizeof(uint32));
	if (_crypt != NULL)
	{
		_crypt->EncryptSend((uint8*)&Bytes, Size);
	}

	return Send(Bytes,Size);
}

void MCodeNetSocket::OnRead()
{
	for(;;)
	{
		if (readBuffer.GetSize() < 4)
		{
			return;
		}

		uint32 len = 0;
		readBuffer.Read(&len,sizeof(int));

		if(_crypt != NULL)
			_crypt->DecryptRecv((uint8 *)readBuffer.GetBufferStart(),len);

		_HandlePacket();
	}
}

void MCodeNetSocket::SendPing()
{
	pingtime = getMSTime();
	last_ping = (uint32)UNIXTIME;
	SendCmd(CMD_SERVER_PING);
}

void MCodeNetSocket::HandPing()
{
	latency = getMSTime() - pingtime;
	last_pong = (uint32)UNIXTIME;
	SendCmd(CMD_SERVER_PONG);
}

void MCodeNetSocket::SendPong()
{
	pingtime = getMSTime();
	last_ping = (uint32)UNIXTIME;
	SendCmd(CMD_SERVER_PONG);
}

void MCodeNetSocket::HandPong()
{
	SendCmd(CMD_SERVER_REPONG);
}

void MCodeNetSocket::HandRePong()
{
	latency = getMSTime() - pingtime;
	last_pong = (uint32)UNIXTIME;
}

void MCodeNetSocket::SendExit()
{
	SendCmd(CMD_SERVER_EXIT);
}

void MCodeNetSocket::HandExit()
{
	Disconnect();
}

void MCodeNetSocket::SendCmd( MNetCmd cmd)
{
	uint32 acmd = cmd;
	SendPacket((const uint8 *)&acmd, sizeof(uint32));
}
