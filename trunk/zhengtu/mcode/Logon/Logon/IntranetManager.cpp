//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      hyd
///  Create:      2012/10/22
///
/// @file         IntranetManager.cpp
/// @brief        内部网络管理器
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

#include "IntranetManager.h"
#include "ClientCache.h"

initialiseSingleton(IntranetManager);


IntranetManager::IntranetManager()
{

}

IntranetManager::~IntranetManager()
{
	SERVERS::iterator it = servers.begin();
	while(it!=servers.end())
	{
		ServerInfo* info = it->first;
		Macro_DeleteClass(info,ServerInfo);
		if (it->second)
		{
			it->second->Disconnect();
		}
		
		servers.erase(it++);
	}
}

void IntranetManager::UpdateSockets()
{
	mapLock.Acquire();

	SERVERS::iterator it = servers.begin();
	MCodeNetSocket *cs;
	uint32 t = (uint32)UNIXTIME;
	for(; it != servers.end(); ++it)
	{
		cs = it->second;
		if (cs != NULL)
		{
			if(cs->IsDeleted() || !cs->IsConnected())
			{
				cs->_id = 0;
				it->second = 0;
				continue;
			}

			if(cs->last_pong < t && ((t - cs->last_pong) > 60))
			{
				// 超过60秒ping没有返回，移除该socket
				LOG_DETAIL(" >> realm id %u connection dropped due to pong timeout.", (unsigned int)it->first->ID);
				cs->_id = 0;
				cs->Disconnect();
				it->second = 0;
				continue;
			}

			if((t - cs->last_ping) > 15)
			{
				// 重新发一个心跳包
				cs->SendPing();
			}
		}
		else
		{
			// check retry time
			if(t >= it->first->RetryTime)
			{
				Connect(it->first);
			}
		}
	}
}

ClientCache* IntranetManager::ConnectToCache( string Address, uint32 Port )
{
	ClientCache * conn = ConnectTCPSocket<ClientCache>(Address.c_str(), static_cast<u_short>(Port));
	return conn;
}

void IntranetManager::Connect( ServerInfo* server )
{
	if (server->Registered == true)
		return;
	
	server->RetryTime = (uint32)UNIXTIME + 10;
	server->Registered = false;
	MCodeNetSocket *conn = NULL;
	SERVERS::iterator it = servers.find(server);
	if(it==servers.end())
		return;
	conn = it->second;
	if (conn == NULL)
	{
		switch(server->Type)
		{
		case EServerType_Login:
			break;
		case EServerType_Cache:
			conn = ConnectToCache(server->Address,server->Port);
			break;
		case EServerType_Gate:
			break;
		case EServerType_World:
			break;
		}
		if (conn == 0)
		{
			Log.Error("IntranetManager", "Connection failed. Will try again in 10 seconds.");
			return;
		}
		Log.Success("IntranetManager", "连接成功，开始通讯...");
		servers[server] = conn;
		conn->_id = server->ID;
	}

	/** @brief PING一下 */
	conn->SendPing();
	/** @brief 等待PING成功返回 */
	uint32 tt = 0;
	while(1)
	{
		if (conn->last_pong>conn->last_ping)
		{
			break;
		}
		if (tt>10*1000)
		{
			return;
		}
		tt+=50;
		MCodeNet::Sleep(50);
	}
	
	/** @brief 注册发送本服务器信息 */
	SendRegisterInfo(server,conn);
	uint32 st = (uint32)UNIXTIME + 10;
	/** @brief 等待注册成功 */
	while(server->Registered == false)
	{
		// Don't wait more than.. like 10 seconds for a registration
		if((uint32)UNIXTIME >= st)
		{
			Log.Error("LogonCommClient", "Realm registration timed out.");
			servers[server] = 0;
			conn->Disconnect();
			break;
		}
		MCodeNet::Sleep(50);
	}

	Log.Success("IntranetManager", "注册服务器成功，消耗时间 %ums.", conn->latency);
}

void IntranetManager::Startup()
{
	// 启动管理者的工作线程
	ThreadPool.ExecuteTask(new IntranetWatcherThread());
}

void IntranetManager::ConnectionDropped( uint32 ID )
{
	mapLock.Acquire();
	SERVERS::iterator itr= servers.begin();
	for(; itr != servers.end(); ++itr)
	{
		if(itr->first->ID == ID)
		{
			if (itr->second != 0)
			{
				itr->second->Disconnect();
			}
			
			LOG_ERROR(" >> id %u connection was dropped unexpectedly. reconnecting next loop.", ID);
			servers.erase(itr);
			break;
		}
	}
	mapLock.Release();
}

void IntranetManager::ConnectAll()
{
	Log.Success("IntranetManager", "Attempting to connect to all servers...");
	for(SERVERS::iterator itr = servers.begin(); itr != servers.end(); ++itr)
		Connect(itr->first);
}

void IntranetManager::SendRegisterInfo( ServerInfo *server,MCodeNetSocket *conn )
{

}

void IntranetManager::RegisterRes( uint32 ID, uint32 ServID)
{
	SERVERS::iterator it = servers.begin();
	for(; it != servers.end(); ++it)
	{
		if(it->first->ID == ID)
		{
			it->first->ServerID = ServID;
			it->first->Registered = true;
			return;
		}
	}
}

void IntranetManager::AddConnector( ServerInfo* server )
{
	servers[server] = NULL;
}
