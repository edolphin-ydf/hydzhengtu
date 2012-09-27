#include "stdafx.h"
#include "MolNet.h"

#include "MolSocket.h"
#include "MolSocketMgrWin32.h"
#include "MolListenSocketWin32.h"

#include <stdio.h>

ListenSocket<NetClient> *m_ServerSocket = NULL;

/** 
* 初始网络，设置相应的参数
*
* @param MaxClients 服务器最大支持的客户端数量，如果为0的话表示没有限制
* @param TimeOut 服务器设置的超时，初始为60
* @param bufferSize 服务器设置的内部缓冲区大小，如果为0的话表示使用默认大小
*
*/
void InitMolNet(uint32 MaxClients,uint32 TimeOut,uint32 bufferSize)
{
	new SocketMgr;
	new SocketGarbageCollector;

	ThreadPool.Startup();

	sSocketMgr.SpawnWorkerThreads();
	sSocketMgr.SetMaxSockets(MaxClients);

	m_ServerSocket = new ListenSocket<NetClient>();
	if(m_ServerSocket)
	{
		m_ServerSocket->SetTimeOut(TimeOut);
		m_ServerSocket->SetBufferSize(bufferSize);
	}

	m_NetworkUpdate = new MolNetworkUpdate();
}

/**
 * 停止网络服务
 */
//void StopMolNet(void)
//{
//	if(m_ServerSocket)
//		m_ServerSocket->Close();
//
//	sSocketMgr.CloseAll();
//	sSocketMgr.ClearMesList();
//	//sSocketGarbageCollector.DeleteSocket();
//}

/** 
* 卸载网络
*/
void CleanMolNet(void)
{
	if(m_ServerSocket == NULL) return;

	if(m_ServerSocket)
		m_ServerSocket->Close();

	sSocketMgr.ShutdownThreads();
	ThreadPool.Shutdown();

	sSocketMgr.CloseAll();
	sSocketMgr.ClearMesList();
	sSocketGarbageCollector.DeleteSocket();

	delete SocketMgr::getSingletonPtr();
	delete SocketGarbageCollector::getSingletonPtr();

	if(m_ServerSocket)
		delete m_ServerSocket;
	m_ServerSocket = NULL;

	if(m_NetworkUpdate)
		delete m_NetworkUpdate;
	m_NetworkUpdate = NULL;

	WSACleanup();
}

/** 
* 开始网络服务
*
* @param ListenAddress 侦听的网络地址
* @param Port 侦听的服务器端口
*
* @return 如果网络服务启动成功返回真,否则返回假
*/
bool StartMolNet(const char * ListenAddress, uint32 Port)
{
	if(!m_ServerSocket) return false;

	if(!m_ServerSocket->Start(ListenAddress,Port))
		return false;

	if(m_ServerSocket->IsOpen())
	{
		ThreadPool.ExecuteTask(m_ServerSocket);
		ThreadPool.ExecuteTask(m_NetworkUpdate);

		return true;
	}

	return false;
}

/** 
* 服务器是否还处于连接状态
*
* @return 如果服务器处于连接状态返回真，否则返回假
*/
bool IsConnected(void)
{
	if(m_ServerSocket == NULL) return false;

	return m_ServerSocket->IsOpen();
}

///** 
//* 停止指定的客户端
//*
//* @param index 要停止的客户端的索引
//*/
//bool Disconnect(uint32 index)
//{
//	bool isOk = false;
//
//	Socket *pSocket = NULL;
//	//sSocketGarbageCollector.Lock();
//	pSocket = sSocketMgr.FindSocket(index);
//	if(pSocket/* && 
//		pSocket->IsConnected()*/) 
//	{
//		pSocket->Disconnect();
//		isOk = true;
//	}
//	//sSocketGarbageCollector.Unlock();
//
//	return isOk;
//}
//
///**
// * 检测指定客户端是否已经连接
// *
// * @param index 要检测连接的客户端索引
// *
// * @return 如果这个要检测的客户端已经连接返回真，否则返回假
// */
//bool IsConnected(uint32 index)
//{
//	bool isOk = false;
//
//	Socket *pSocket = NULL;
//	//sSocketGarbageCollector.Lock();
//	pSocket = sSocketMgr.FindSocket(index);
//	if(pSocket)
//		isOk = pSocket->IsConnected();
//	//sSocketGarbageCollector.Unlock();
//	
//	return isOk;
//}
//
///** 
//* 发送指定的数据到指定的客户端
//*
//* @param index 要接收数据的客户端索引
//* @param out 要发送的数据
//*
//* @return 如果数据发送成功返回真,否则返回假
//*/
//bool Send(uint32 index,CMolMessageOut &out)
//{
//	bool isOk = false;
//
//	Socket *pSocket = NULL;
//	//sSocketGarbageCollector.Lock();
//	pSocket = sSocketMgr.FindSocket(index);
//	if(pSocket) 
//		isOk = pSocket->Send(out);
//	//sSocketGarbageCollector.Unlock();
//
//	return isOk;
//}
//
///**  
//* 得到指定客户端的IP地址
//*
//* @param index 要得到IP地址的客户端的索引
//*
//* @return 如果这个客户端有IP地址返回IP地址,否则返回NULL
//*/
//std::string GetIpAddress(uint32 index)
//{
//	std::string remoteIP;
//
//	Socket *pSocket = NULL;
//	//sSocketGarbageCollector.Lock();
//	pSocket = sSocketMgr.FindSocket(index);
//	if(pSocket)
//		remoteIP = pSocket->GetRemoteIP();
//	//sSocketGarbageCollector.Unlock();
//
//	return remoteIP;
//}

/** 
* 得到消息列表
*
* @param mes 用于存储得到的消息
*
* @return 返回得到的消息的个数
*/
int GetNetMessage(NetMessage & mes)
{
	//mes.Clear();

	//if(sSocketMgr.GetMesCount() <= 0 ||
	//	mes.GetMaxCount() <= 0)
	//	return 0;

	//int count = 0;

	//// 如果当前系统中的消息个数小于我们要读取的个数时，读取全部的消息；
	//// 否则读取我们设置的消息个数的消息
	//if(sSocketMgr.GetMesCount() < mes.GetMaxCount())
	//{
	//	count = sSocketMgr.GetMesCount();

	//	sSocketMgr.LockMesList();
	//	std::list<MessageStru> *meslist = sSocketMgr.GetMesList();
	//	if(meslist == NULL || meslist->empty()) 
	//	{
	//		sSocketMgr.UnlockMesList();
	//		return 0;
	//	}

	//	std::list<MessageStru>::iterator iter = meslist->begin();
	//	for(;iter != meslist->end();)
	//	{
	//		mes.AddMessage((*iter));
	//		iter = meslist->erase(iter);
	//	}
	//	sSocketMgr.UnlockMesList();
	//}
	//else
	//{
	//	sSocketMgr.LockMesList();
	//	std::list<MessageStru> *meslist = sSocketMgr.GetMesList();
	//	if(meslist == NULL || meslist->empty()) 
	//	{
	//		sSocketMgr.UnlockMesList();
	//		return 0;
	//	}

	//	std::list<MessageStru>::iterator iter = meslist->begin();
	//	for(int i=0;iter != meslist->end(),i<mes.GetMaxCount();i++)
	//	{
	//		if(iter == meslist->end()) break;

	//		mes.AddMessage((*iter));
	//		iter = meslist->erase(iter);
	//	}
	//	sSocketMgr.UnlockMesList();

	//	count = mes.GetMaxCount();
	//}

	return 0;
}

/** 
* 执行一个指定的任务
*
* @param task 我们要执行的任务
*/
void ExecuteTask(ThreadBase * ExecutionTarget)
{
	sSocketMgr.AddTask(ExecutionTarget);

	ThreadPool.ExecuteTask(ExecutionTarget);
}

int GetMsgListCount()
{
	return sSocketMgr.GetMesCount();
}
int GetSocketListCount()
{
	return sSocketMgr.GetClientCount();
}

