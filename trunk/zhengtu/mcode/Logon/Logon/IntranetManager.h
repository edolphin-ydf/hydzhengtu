//////////////////////////////////////////////////
/// @file : IntranetManager.h
/// @brief : �ڲ����������
/// @date:  2012/10/22
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __IntranetManager_H__
#define __IntranetManager_H__

#include "Common.h"
#include "global.h"
#include "shared/Threading/ThreadStarter.h"
class ClientCache;

////////////////////////////////////////////////////////////////
/// @struct ServerInfo
/// @brief  ����������Ϣ
////////////////////////////////////////////////////////////////
struct ServerInfo
{
	uint32 ID;        /**< �����ID  */
	EServerType Type;
	string Name;
	string Address;
	uint32 Port;
	uint32 ServerID;  /**< ���ӵķ�������ID  */
	uint32 RetryTime; 
	bool   Registered;/**< �Ƿ��Ѿ��ɹ�ע�ᣬ�����������  */ 
	ServerInfo(){
		ID = 0;
		Type = EServerType_None;
		Name = "";
		Address = "";
		Port = 0;
		ServerID = 0;
		RetryTime = 0;
		Registered = false;
	}
};
class MCodeNetSocket;
////////////////////////////////////////////////////////////////
/// @class IntranetManager
/// @brief ����˷����ھ�������Ϊ�ͻ��˵��׽��֣����Կͻ������ӻ��������
///
/// @note
class IntranetManager : public Singleton<IntranetManager>
{
	typedef map<ServerInfo*,MCodeNetSocket*> SERVERS;
	SERVERS servers;
	ServerInfo selfInfo;
	Mutex mapLock;
public:
	IntranetManager();
	~IntranetManager();

	/** @brief ���� */
	void Startup();
	/** @brief ������Ӷ��� */
	void AddConnector(ServerInfo* server);
	/** @brief �ر�ĳ������ */
	void ConnectionDropped(uint32 ID);
	/** @brief �����׽��� */
	void UpdateSockets();
	void Connect(ServerInfo* server);
	void ConnectAll();

	/** @brief �ѵ�ǰ����������Ϣ���͸����Ӷ������ע�� */
	void SendRegisterInfo(ServerInfo *server,MCodeNetSocket *conn);
	/** @brief ����ע����Ϣ */
	void RegisterRes(uint32 ID, uint32 ServID);
	/** @brief ���ӵ���������� */
	ClientCache* ConnectToCache(string Address, uint32 Port);

	ServerInfo* GetSelfInfo(){ return &selfInfo;}
};

#define sIntranetMgr IntranetManager::getSingleton()

////////////////////////////////////////////////////////////////
/// @class IntranetWatcherThread
/// @brief �ڲ���������������߳�
///
/// @note  ���ڶԹ������Ķ�ʱ����ά��
class IntranetWatcherThread : public ThreadBase
{
	bool running;
	MCodeNet::Threading::ConditionVariable cond;

public:
	IntranetWatcherThread() : ThreadBase("IntranetWatcherThread")
	{
		running = true;
	}

	~IntranetWatcherThread()
	{

	}

	void OnShutdown()
	{
		running = false;

		cond.Signal();
	}

	bool run()
	{
		sIntranetMgr.ConnectAll();
		while(running)
		{
			sIntranetMgr.UpdateSockets();

			cond.Wait(5000);
		}

		return true;
	}
};


#endif