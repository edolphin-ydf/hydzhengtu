#include "stdafx.h"
#include "GameServerManager.h"

initialiseSingleton(GameServerManager);

/**  
 * 构造函数
 */
GameServerManager::GameServerManager()
{

}

/** 
 * 析构函数
 */
GameServerManager::~GameServerManager()
{

}

/** 
 * 添加一个游戏服务器到管理列表中
 *
 * @param pGameServerInfo 要添加的游戏服务器
 *
 * @return 如果返回1表示注册成功；2表示注册失败；3表示重复注册
 */
int GameServerManager::AddGameServer(GameServerInfo pGameServerInfo)
{
	m_GameServerLock.Acquire();
	std::map<uint32,GameServerInfo>::iterator iter = m_GameServerInfoList.find(pGameServerInfo.ConnId);
	if(iter != m_GameServerInfoList.end())
	{
		m_GameServerLock.Release();
		return 3;
	}
	else
	{
		m_GameServerInfoList.insert(std::pair<int,GameServerInfo>(pGameServerInfo.ConnId,pGameServerInfo));

		m_GameServerLock.Release();
		return 1;
	}
	
	m_GameServerLock.Release();
	return 2;
}

/**  
 * 删除指定连接ID的服务器从管理列表中
 *
 * @param connId 要删除的游戏服务器的连接ID
 */
void GameServerManager::DeleteGameServerByConnId(int connId)
{
	if(m_GameServerInfoList.empty()) return;

	m_GameServerLock.Acquire();
	std::map<uint32,GameServerInfo>::iterator iter = m_GameServerInfoList.find(connId);
	if(iter != m_GameServerInfoList.end())
	{
		m_GameServerInfoList.erase(iter);
	}
	m_GameServerLock.Release();
}

/** 
 * 更新指定连接ID的服务器人数
 *
 * @param connId 要更新的服务器的连接ID
 * @param playerCount 要更新的服务器的在线人数
 */
void GameServerManager::UpdateGameServerByConnId(int connId,int playerCount)
{
	if(m_GameServerInfoList.empty()) return;

	m_GameServerLock.Acquire();
	std::map<uint32,GameServerInfo>::iterator iter = m_GameServerInfoList.find(connId);
	if(iter != m_GameServerInfoList.end())
	{
		(*iter).second.OnlinePlayerCount = playerCount;
	}
	m_GameServerLock.Release();
}

/** 
 * 得到指定连接ID的服务器信息
 *
 * @param connId 要取得服务器信息的服务器连接ID
 *
 * @return 如果存在这个服务器，返回这个服务器，否则返回NULL
 */
GameServerInfo* GameServerManager::GetGameServerByConnId(int connId)
{
	if(m_GameServerInfoList.empty()) return NULL;

	m_GameServerLock.Acquire();
	std::map<uint32,GameServerInfo>::iterator iter = m_GameServerInfoList.find(connId);
	if(iter != m_GameServerInfoList.end())
	{
		m_GameServerLock.Release();
		return &((*iter).second);
	}
	m_GameServerLock.Release();

	return NULL;
}