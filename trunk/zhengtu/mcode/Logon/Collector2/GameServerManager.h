#ifndef _GAME_SERVER_MANAGER_H_INCLUDE_
#define _GAME_SERVER_MANAGER_H_INCLUDE_

#include <string>
#include <map>

#include "network/MolSingleton.h"
#include "network/MolMutex.h"

/** 
 * ���ڴ洢���е���Ϸ��������Ϣ
 */
struct GameServerInfo
{
	GameServerInfo()
		: ConnId(0),ServerPort(0),OnlinePlayerCount(0),lastMoney(0)
	{
	}
	GameServerInfo(uint32 ci,std::string sn,std::string si,int sp,int opc,__int64 money)
		: ConnId(ci),ServerName(sn),ServerIp(si),ServerPort(sp),OnlinePlayerCount(opc),lastMoney(money)
	{
	}

	uint32 ConnId;                 // ����ID
	std::string ServerName;     // ����������
	std::string ServerIp;       // ������IP
	int ServerPort;             // �������˿�
	int OnlinePlayerCount;      // ��������
	__int64 lastMoney;                // ��Ϸ�������ٽ��ֵ
};

class GameServerManager : public Singleton<GameServerManager>
{
public:
	/// ���캯��
	GameServerManager();
	/// ��������
	~GameServerManager();

	/// ���һ����Ϸ�������������б���
	int AddGameServer(GameServerInfo pGameServerInfo);
	/// ɾ��ָ������ID�ķ������ӹ����б���
	void DeleteGameServerByConnId(int connId);
	/// ����ָ������ID�ķ���������
	void UpdateGameServerByConnId(int connId,int playerCount);
	/// �õ�ָ������ID�ķ�������Ϣ
	GameServerInfo* GetGameServerByConnId(int connId);

	/// �õ���Ϸ�������б���Ϣ
	inline std::map<uint32,GameServerInfo>& GetGameServerInfo(void) { return m_GameServerInfoList; }
	/// ��ס��Ϸ�������б�
	inline void LockGameServerList(void) { m_GameServerLock.Acquire(); }
	/// ������Ϸ�������б�
	inline void UnlockGameServerList(void) { m_GameServerLock.Release(); }

private:
	std::map<uint32,GameServerInfo> m_GameServerInfoList;      /**< ���ڱ������е���Ϸ��������Ϣ */
	Mutex m_GameServerLock;                                 /**< ���ڱ������е���Ϸ������ */
};

#define ServerGameServerManager GameServerManager::getSingleton()

#endif