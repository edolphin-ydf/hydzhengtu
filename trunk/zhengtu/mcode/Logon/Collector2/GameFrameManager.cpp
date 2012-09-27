#include "stdafx.h"
#include "GameFrameManager.h"
#include "defines.h"
#include "network/MolNet.h"
#include "DBOperator.h"
#include "GameServerManager.h"

#include <map>

initialiseSingleton(GameFrameManager);

/**
 * ���캯��
 */
GameFrameManager::GameFrameManager()
{
}

/**
 * ��������
 */
GameFrameManager::~GameFrameManager()
{
}

/**
 * ���ڴ�����յ���������Ϣ
 *
 * @param connId Ҫ����Ŀͻ��˵�����ID
 * @param mes ���յ��Ŀͻ��˵���Ϣ
 */
void GameFrameManager::OnProcessNetMes(NetClient* connId,CMolMessageIn *mes)
{
	if(mes == NULL) return;

	switch(mes->getId())
	{
	case IDD_MESSAGE_CENTER_LOGIN:                     // �û���¼
		{
			OnProcessUserLoginMes(connId,mes);
		}
		break;
	case IDD_MESSAGE_REGISTER_GAME:                    // ������ע��
		{
			OnProcessGameServerRegisterMes(connId,mes);
		}
		break;
	case IDD_MESSAGE_GET_GAMESERVER:                 // �õ���Ϸ�������б�
		{
			OnProcessGetGameServersMes(connId,mes);
		}
		break;
	case IDD_MESSAGE_UPDATE_GAME_SERVER:             // ������Ϸ��������Ϣ
		{
			OnProcessUpdateGameServerPlayerCountMes(connId,mes);
		}
		break;
	default:
		break;
	}
}

/**
 * ���ڴ����������������Ϣ
 *
 * @param connId Ҫ����Ŀͻ��˵�����ID
 */
void GameFrameManager::OnProcessConnectedNetMes(NetClient* connId)
{
	// ����Ƿ���ָ������ID����Ϸ������
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
 * ���ڴ������ڶϿ�����������Ϣ
 *
 * @param connId Ҫ����Ŀͻ��˵�����ID
 */
void GameFrameManager::OnProcessDisconnectNetMes(NetClient* connId)
{
	ServerGameServerManager.DeleteGameServerByConnId((int)connId->GetFd());
}

/**
 * �����û���¼ϵͳ��Ϣ
 *
 * @param connId Ҫ����Ŀͻ���
 * @param mes Ҫ����Ŀͻ��˵���Ϣ
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

	// �õ��û�����
	UserDataStru pUserData;
	if(!ServerDBOperator.GetUserData(pUserId,pUserData)) 
	{
		CMolMessageOut out(IDD_MESSAGE_CENTER_LOGIN);
		out.writeShort(IDD_MESSAGE_CENTER_LOGIN_FAIL);	
		connId->Send(out);
		return;
	}

	// ����ҷ��ͳɹ���¼��������Ϣ
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
 * ������Ϸ������ע����Ϣ
 *
 * @param connId ��Ϸ������������ID
 * @param mes ���յ��ķ�������Ϣ
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

	//// ������ݿ����Ƿ���������䣬�������ݿ���Ҫ����ͬ���Ƶ���Ϸ�������ע��ɹ�
	//if(!ServerDBOperator.IsExistGameServer(GameId,ServerName.c_str()))
	//	state = 2;

	CMolMessageOut out(IDD_MESSAGE_REGISTER_GAME);

	if(state == 1)               // �ɹ�
		out.writeShort(IDD_MESSAGE_REGISTER_SUCCESS);
	//else if(state == 2)          // ʧ��
	//	out.writeShort(IDD_MESSAGE_REGISTER_FAIL);
	else if(state == 3)          // �ظ�ע��
		out.writeShort(IDD_MESSAGE_RE_REGISTER);

	connId->Send(out);

	//if(state == 1)
	//{
	//	CString tempStr;
	//	tempStr.Format("��ϵͳ�� ��Ϸ��������%s��IP��%s �˿ڣ�%d ע��ɹ�!",ServerName.c_str(),ServerIp.c_str(),ServerPort);
	//	PrintLog(tempStr);
	//}
}

/**
 * ����õ���Ϸ�������б���Ϣ
 *
 * @param connId Ҫ����Ŀͻ���
 * @param mes Ҫ����Ŀͻ��˵���Ϣ
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
 * ������Ϸ��������������
 *
 * @param connId ��Ϸ������������ID
 * @param mes ���յ��ķ�������Ϣ
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