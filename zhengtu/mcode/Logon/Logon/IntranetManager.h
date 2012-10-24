//////////////////////////////////////////////////
/// @file : IntranetManager.h
/// @brief : 内部网络管理器
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
/// @brief  服务器的信息
////////////////////////////////////////////////////////////////
struct ServerInfo
{
	uint32 ID;        /**< 自身的ID  */
	EServerType Type;
	string Name;
	string Address;
	uint32 Port;
	uint32 ServerID;  /**< 连接的服务器的ID  */
	uint32 RetryTime; 
	bool   Registered;/**< 是否已经成功注册，否则继续重连  */ 
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
/// @brief 管理此服务在局域网作为客户端的套接字，如以客户端连接缓存服务器
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

	/** @brief 启动 */
	void Startup();
	/** @brief 添加连接对象 */
	void AddConnector(ServerInfo* server);
	/** @brief 关闭某个连接 */
	void ConnectionDropped(uint32 ID);
	/** @brief 更新套接字 */
	void UpdateSockets();
	void Connect(ServerInfo* server);
	void ConnectAll();

	/** @brief 把当前服务器的消息发送给连接对象进行注册 */
	void SendRegisterInfo(ServerInfo *server,MCodeNetSocket *conn);
	/** @brief 返回注册信息 */
	void RegisterRes(uint32 ID, uint32 ServID);
	/** @brief 连接到缓存服务器 */
	ClientCache* ConnectToCache(string Address, uint32 Port);

	ServerInfo* GetSelfInfo(){ return &selfInfo;}
};

#define sIntranetMgr IntranetManager::getSingleton()

////////////////////////////////////////////////////////////////
/// @class IntranetWatcherThread
/// @brief 内部网络管理器工作线程
///
/// @note  用于对管理器的定时更新维护
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