#include "stdafx.h"
#include "GameServerManager.h"

initialiseSingleton(GameServerManager);

/**  
 * ���캯��
 */
GameServerManager::GameServerManager()
{

}

/** 
 * ��������
 */
GameServerManager::~GameServerManager()
{

}

/** 
 * ���һ����Ϸ�������������б���
 *
 * @param pGameServerInfo Ҫ��ӵ���Ϸ������
 *
 * @return �������1��ʾע��ɹ���2��ʾע��ʧ�ܣ�3��ʾ�ظ�ע��
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
 * ɾ��ָ������ID�ķ������ӹ����б���
 *
 * @param connId Ҫɾ������Ϸ������������ID
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
 * ����ָ������ID�ķ���������
 *
 * @param connId Ҫ���µķ�����������ID
 * @param playerCount Ҫ���µķ���������������
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
 * �õ�ָ������ID�ķ�������Ϣ
 *
 * @param connId Ҫȡ�÷�������Ϣ�ķ���������ID
 *
 * @return ������������������������������������򷵻�NULL
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