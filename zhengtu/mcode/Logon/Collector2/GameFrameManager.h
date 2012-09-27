#ifndef _GAME_FRAME_MANAGER_H_INCLUDE_
#define _GAME_FRAME_MANAGER_H_INCLUDE_

#include "defines.h"
#include "network/MolSingleton.h"
#include "network/MolMessageIn.h"
#include <string.h>
#include "network/MolSocket.h"

class GameFrameManager : public Singleton<GameFrameManager>
{
public:
	/// 构造函数
	GameFrameManager();
	/// 析构函数
	~GameFrameManager();

	/// 用于处理接收到的网络消息
	void OnProcessNetMes(NetClient* connId,CMolMessageIn *mes);

	/// 用于处理接收网络连接消息
	void OnProcessConnectedNetMes(NetClient* connId);

	/// 用于处理用于断开网络连接消息
	void OnProcessDisconnectNetMes(NetClient* connId);

private:
	/// 处理用户登录系统消息
	void OnProcessUserLoginMes(NetClient* connId,CMolMessageIn *mes);
	/// 处理游戏服务器注册消息
	void OnProcessGameServerRegisterMes(NetClient* connId,CMolMessageIn *mes);
	/// 处理得到游戏服务器列表消息
	void OnProcessGetGameServersMes(NetClient* connId,CMolMessageIn *mes);
	/// 更新游戏服务器在线人数
	void OnProcessUpdateGameServerPlayerCountMes(NetClient* connId,CMolMessageIn *mes);
};

#define ServerGameFrameManager GameFrameManager::getSingleton()

#endif