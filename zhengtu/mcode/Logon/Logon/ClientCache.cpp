//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      hyd
///  Create:      2012/10/22
///
/// @file         ClientCache.cpp
/// @brief        用于内网连接缓存服务器
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

#include "LogonStdAfx.h"
#include "ClientCache.h"


ClientCache::ClientCache( SOCKET fd ) : MCodeNetSocket(fd, 32768, 4096,false)
{
	
}

ClientCache::~ClientCache()
{

}

void ClientCache::_HandlePacket()
{
	uint32 cmd;
	readBuffer.Read(&cmd,sizeof(uint32));
	printf("%d",cmd);

	if (cmd == CMD_SERVER_PING)
	{
		HandPong();
		return;
	}
	else if (cmd == CMD_SERVER_PONG)
	{
		
	}
}



