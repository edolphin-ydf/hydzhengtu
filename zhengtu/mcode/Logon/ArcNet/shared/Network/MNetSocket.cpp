
#include "MNetSocket.h"


MNetSocket::MNetSocket( SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize, bool iCrypt , uint8* Key)
	: Socket(fd,sendbuffersize,recvbuffersize)
{
	if(iCrypt)
		_crypt = new PacketCrypt(Key);
	else
		_crypt = NULL;
}


MNetSocket::~MNetSocket()
{
	if (_crypt != NULL)
	{
		delete _crypt;
		_crypt = NULL;
	}
}

bool MNetSocket::SendPacket( const uint8* Bytes, uint32 Size )
{
	writeBuffer.Write(&Size,sizeof(uint32));
	if (_crypt != NULL)
	{
		_crypt->EncryptSend((uint8*)&Bytes, Size);
	}

	return Send(Bytes,Size);
}

void MNetSocket::OnRead()
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
