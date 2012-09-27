#ifndef _MOL_NET_H_INCLUDE 
#define _MOL_NET_H_INCLUDE 

/** 
* MolNet网络引擎
*
* 描述:
* 作者:akinggw
* 日期:2010.2.11
*/

#include "MolCommon.h"
#include "MolMutex.h"
#include "MolString.h"
#include "MolSingleton.h"
#include "MolThreadPool.h"

#include "MolMessageOut.h"
#include "MolMessageIn.h"

#include "MolNetMessage.h"

/** 
 * 初始网络，设置相应的参数
 *
 * @param MaxClients 服务器最大支持的客户端数量，如果为0的话表示没有限制
 * @param TimeOut 服务器设置的超时，初始为60
 * @param bufferSize 服务器设置的内部缓冲区大小，如果为0的话表示使用默认大小
 *
 */
void InitMolNet(uint32 MaxClients=0,uint32 TimeOut=60,uint32 bufferSize=0);

/** 
 * 卸载网络
 */
void CleanMolNet(void);

/** 
 * 开始网络服务
 *
 * @param ListenAddress 侦听的网络地址
 * @param Port 侦听的服务器端口
 *
 * @return 如果网络服务启动成功返回真,否则返回假
 */
bool StartMolNet(const char * ListenAddress, uint32 Port);

/**
 * 停止网络服务
 */
//void MOLNETEXPORT StopMolNet(void);

/** 
 * 服务器是否还处于连接状态
 *
 * @return 如果服务器处于连接状态返回真，否则返回假
 */
bool IsConnected(void);

/**
 * 检测指定客户端是否已经连接
 *
 * @param index 要检测连接的客户端索引
 *
 * @return 如果这个要检测的客户端已经连接返回真，否则返回假
 */
//bool IsConnected(uint32 index);

/** 
 * 停止指定的客户端
 *
 * @param index 要停止的客户端的索引
 */
//bool Disconnect(uint32 index);

/** 
 * 发送指定的数据到指定的客户端
 *
 * @param index 要接收数据的客户端索引
 * @param out 要发送的数据
 *
 * @return 如果数据发送成功返回真,否则返回假
 */
//bool Send(uint32 index,CMolMessageOut &out);

/**  
 * 得到指定客户端的IP地址
 *
 * @param index 要得到IP地址的客户端的索引
 *
 * @return 如果这个客户端有IP地址返回IP地址,否则返回NULL
 */
//std::string GetIpAddress(uint32 index);

/** 
 * 得到消息列表
 *
 * @param mes 用于存储得到的消息
 *
 * @return 返回得到的消息的个数
 */
int GetNetMessage(NetMessage & mes);

/** 
 * 执行一个指定的任务
 *
 * @param task 我们要执行的任务
 */
void ExecuteTask(ThreadBase * ExecutionTarget);

//for test
int GetMsgListCount();
int GetSocketListCount();

#endif
