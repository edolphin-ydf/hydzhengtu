#include "stdafx.h"
#include "GameFrameManager.h"
#include "defines.h"
#include "network/MolNet.h"
#include "DBOperator.h"
#include "GameServerManager.h"

#include <map>

initialiseSingleton(GameFrameManager);

/**
 * 构造函数
 */
GameFrameManager::GameFrameManager()
{
}

/**
 * 析构函数
 */
GameFrameManager::~GameFrameManager()
{
}

/**
 * 用于处理接收到的网络消息
 *
 * @param connId 要处理的客户端的网络ID
 * @param mes 接收到的客户端的消息
 */
void GameFrameManager::OnProcessNetMes(NetClient* connId,CMolMessageIn *mes)
{
	if(mes == NULL) return;

	switch(mes->getId())
	{
	case IDD_MESSAGE_CENTER_LOGIN:                     // 用户登录
		{
			OnProcessUserLoginMes(connId,mes);
		}
		break;
	case IDD_MESSAGE_REGISTER_GAME:                    // 服务器注册
		{
			OnProcessGameServerRegisterMes(connId,mes);
		}
		break;
	case IDD_MESSAGE_GET_GAMESERVER:                 // 得到游戏服务器列表
		{
			OnProcessGetGameServersMes(connId,mes);
		}
		break;
	case IDD_MESSAGE_UPDATE_GAME_SERVER:             // 更新游戏服务器信息
		{
			OnProcessUpdateGameServerPlayerCountMes(connId,mes);
		}
		break;
	default:
		break;
	}
}

/**
 * 用于处理接收网络连接消息
 *
 * @param connId 要处理的客户端的网络ID
 */
void GameFrameManager::OnProcessConnectedNetMes(NetClient* connId)
{
	// 检测是否有指定连接ID的游戏服务器
	GameServerInfo *pGameServerInfo = ServerGameServerManager.GetGameServerByConnId((int)connId->GetFd());
	if(pGameServerInfo)
	{
		CMolMessageOut out(IDD_MESSAGE_CONNECT);
		out.writeShort(IDD_MESSAGE_CONNECT_EXIST);

		connId->Send(out);
	}
	else
	{
		CMolMessageOut out(IDD_MESSAGE_CONNECT);
		out.writeShort(IDD_MESSAGE_CONNECT_SUCESS);

		connId->Send(out);
	}
}

/**
 * 用于处理用于断开网络连接消息
 *
 * @param connId 要处理的客户端的网络ID
 */
void GameFrameManager::OnProcessDisconnectNetMes(NetClient* connId)
{
	ServerGameServerManager.DeleteGameServerByConnId((int)connId->GetFd());
}

/**
 * 处理用户登录系统消息
 *
 * @param connId 要处理的客户端
 * @param mes 要处理的客户端的消息
 */
void GameFrameManager::OnProcessUserLoginMes(NetClient* connId,CMolMessageIn *mes)
{
	if(mes == NULL) return;

	CMolString pUserName = mes->readString();
	CMolString pUserPW = mes->readString();

	unsigned int pUserId = ServerDBOperator.IsExistUser(pUserName.c_str(),pUserPW.c_str());

	if(pUserId <= 0) 
	{
		CMolMessageOut out(IDD_MESSAGE_CENTER_LOGIN);
		out.writeShort(IDD_MESSAGE_CENTER_LOGIN_FAIL);	
		connId->Send(out);		
		return;
	}

	// 得到用户数据
	UserDataStru pUserData;
	if(!ServerDBOperator.GetUserData(pUserId,pUserData)) 
	{
		CMolMessageOut out(IDD_MESSAGE_CENTER_LOGIN);
		out.writeShort(IDD_MESSAGE_CENTER_LOGIN_FAIL);	
		connId->Send(out);
		return;
	}

	// 向玩家发送成功登录服务器消息
	CMolMessageOut out(IDD_MESSAGE_CENTER_LOGIN);
	out.writeShort(IDD_MESSAGE_CENTER_LOGIN_SUCESS);	
	out.writeLong(pUserId);
	out.writeString(pUserName.c_str());
	out.writeString(pUserData.UserAvatar);
	out.writeShort(pUserData.Level);
	out.writeDouble((double)pUserData.Money);
	out.writeDouble((double)pUserData.BankMoney);
	out.writeLong(pUserData.Experience);
	out.writeShort(pUserData.TotalBureau);
	out.writeShort(pUserData.SBureau);
	out.writeShort(pUserData.FailBureau);
	out.writeFloat(pUserData.SuccessRate);
	out.writeFloat(pUserData.RunawayRate);			
	
	connId->Send(out);
}

/** 
 * 处理游戏服务器注册消息
 *
 * @param connId 游戏服务器的连接ID
 * @param mes 接收到的服务器消息
 */
void GameFrameManager::OnProcessGameServerRegisterMes(NetClient* connId,CMolMessageIn *mes)
{
	if(mes == NULL) return;

	CMolString ServerName = mes->readString();
	CMolString ServerIp = mes->readString();
	int ServerPort = mes->readShort();
	int OnlinePlayerCount = mes->readLong();
	__int64 lastmoney = (__int64)mes->readDouble();

	int state = ServerGameServerManager.AddGameServer(GameServerInfo((uint32)connId->GetFd(),ServerName.c_str(),ServerIp.c_str(),ServerPort,OnlinePlayerCount,lastmoney));

	//// 检测数据库中是否有这个房间，必须数据库中要有相同名称的游戏房间才能注册成功
	//if(!ServerDBOperator.IsExistGameServer(GameId,ServerName.c_str()))
	//	state = 2;

	CMolMessageOut out(IDD_MESSAGE_REGISTER_GAME);

	if(state == 1)               // 成功
		out.writeShort(IDD_MESSAGE_REGISTER_SUCCESS);
	//else if(state == 2)          // 失败
	//	out.writeShort(IDD_MESSAGE_REGISTER_FAIL);
	else if(state == 3)          // 重复注册
		out.writeShort(IDD_MESSAGE_RE_REGISTER);

	connId->Send(out);

	//if(state == 1)
	//{
	//	CString tempStr;
	//	tempStr.Format("【系统】 游戏服务器“%s”IP：%s 端口：%d 注册成功!",ServerName.c_str(),ServerIp.c_str(),ServerPort);
	//	PrintLog(tempStr);
	//}
}

/**
 * 处理得到游戏服务器列表消息
 *
 * @param connId 要处理的客户端
 * @param mes 要处理的客户端的消息
 */
void GameFrameManager::OnProcessGetGameServersMes(NetClient* connId,CMolMessageIn *mes)
{
	if(mes == NULL) return;

	ServerGameServerManager.LockGameServerList();

	if(ServerGameServerManager.GetGameServerInfo().empty())
	{
		ServerGameServerManager.UnlockGameServerList();

		CMolMessageOut out(IDD_MESSAGE_CENTER_LOGIN);
		out.writeShort(IDD_MESSAGE_GET_GAMESERVER);
		out.writeShort(0);
		connId->Send(out);

		return;
	}

	CMolMessageOut out(IDD_MESSAGE_CENTER_LOGIN);
	out.writeShort(IDD_MESSAGE_GET_GAMESERVER);
	out.writeShort((int)ServerGameServerManager.GetGameServerInfo().size());

	std::map<uint32,GameServerInfo>::iterator iter = ServerGameServerManager.GetGameServerInfo().begin();
	for(;iter != ServerGameServerManager.GetGameServerInfo().end();iter++)
	{
		out.writeString((*iter).second.ServerName);
		out.writeString((*iter).second.ServerIp);
		out.writeShort((*iter).second.ServerPort);
		out.writeShort((*iter).second.OnlinePlayerCount);
		out.writeDouble((double)(*iter).second.lastMoney);
	}

	connId->Send(out);

	ServerGameServerManager.UnlockGameServerList();
}

/** 
 * 更新游戏服务器在线人数
 *
 * @param connId 游戏服务器的连接ID
 * @param mes 接收到的服务器消息
 */
void GameFrameManager::OnProcessUpdateGameServerPlayerCountMes(NetClient* connId,CMolMessageIn *mes)
{
	if(mes == NULL) return;

	int OnlinePlayerCount = mes->readLong();

	GameServerInfo *pGameServerInfo = ServerGameServerManager.GetGameServerByConnId(connId->GetFd());
	if(pGameServerInfo && pGameServerInfo->ConnId == connId->GetFd())
	{
		pGameServerInfo->OnlinePlayerCount = OnlinePlayerCount;
	}
}