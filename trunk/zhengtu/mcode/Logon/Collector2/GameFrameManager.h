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
	/// ���캯��
	GameFrameManager();
	/// ��������
	~GameFrameManager();

	/// ���ڴ�����յ���������Ϣ
	void OnProcessNetMes(NetClient* connId,CMolMessageIn *mes);

	/// ���ڴ����������������Ϣ
	void OnProcessConnectedNetMes(NetClient* connId);

	/// ���ڴ������ڶϿ�����������Ϣ
	void OnProcessDisconnectNetMes(NetClient* connId);

private:
	/// �����û���¼ϵͳ��Ϣ
	void OnProcessUserLoginMes(NetClient* connId,CMolMessageIn *mes);
	/// ������Ϸ������ע����Ϣ
	void OnProcessGameServerRegisterMes(NetClient* connId,CMolMessageIn *mes);
	/// ����õ���Ϸ�������б���Ϣ
	void OnProcessGetGameServersMes(NetClient* connId,CMolMessageIn *mes);
	/// ������Ϸ��������������
	void OnProcessUpdateGameServerPlayerCountMes(NetClient* connId,CMolMessageIn *mes);
};

#define ServerGameFrameManager GameFrameManager::getSingleton()

#endif