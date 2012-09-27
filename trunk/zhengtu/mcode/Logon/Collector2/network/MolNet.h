#ifndef _MOL_NET_H_INCLUDE 
#define _MOL_NET_H_INCLUDE 

/** 
* MolNet��������
*
* ����:
* ����:akinggw
* ����:2010.2.11
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
 * ��ʼ���磬������Ӧ�Ĳ���
 *
 * @param MaxClients ���������֧�ֵĿͻ������������Ϊ0�Ļ���ʾû������
 * @param TimeOut ���������õĳ�ʱ����ʼΪ60
 * @param bufferSize ���������õ��ڲ���������С�����Ϊ0�Ļ���ʾʹ��Ĭ�ϴ�С
 *
 */
void InitMolNet(uint32 MaxClients=0,uint32 TimeOut=60,uint32 bufferSize=0);

/** 
 * ж������
 */
void CleanMolNet(void);

/** 
 * ��ʼ�������
 *
 * @param ListenAddress �����������ַ
 * @param Port �����ķ������˿�
 *
 * @return ���������������ɹ�������,���򷵻ؼ�
 */
bool StartMolNet(const char * ListenAddress, uint32 Port);

/**
 * ֹͣ�������
 */
//void MOLNETEXPORT StopMolNet(void);

/** 
 * �������Ƿ񻹴�������״̬
 *
 * @return �����������������״̬�����棬���򷵻ؼ�
 */
bool IsConnected(void);

/**
 * ���ָ���ͻ����Ƿ��Ѿ�����
 *
 * @param index Ҫ������ӵĿͻ�������
 *
 * @return ������Ҫ���Ŀͻ����Ѿ����ӷ����棬���򷵻ؼ�
 */
//bool IsConnected(uint32 index);

/** 
 * ָֹͣ���Ŀͻ���
 *
 * @param index Ҫֹͣ�Ŀͻ��˵�����
 */
//bool Disconnect(uint32 index);

/** 
 * ����ָ�������ݵ�ָ���Ŀͻ���
 *
 * @param index Ҫ�������ݵĿͻ�������
 * @param out Ҫ���͵�����
 *
 * @return ������ݷ��ͳɹ�������,���򷵻ؼ�
 */
//bool Send(uint32 index,CMolMessageOut &out);

/**  
 * �õ�ָ���ͻ��˵�IP��ַ
 *
 * @param index Ҫ�õ�IP��ַ�Ŀͻ��˵�����
 *
 * @return �������ͻ�����IP��ַ����IP��ַ,���򷵻�NULL
 */
//std::string GetIpAddress(uint32 index);

/** 
 * �õ���Ϣ�б�
 *
 * @param mes ���ڴ洢�õ�����Ϣ
 *
 * @return ���صõ�����Ϣ�ĸ���
 */
int GetNetMessage(NetMessage & mes);

/** 
 * ִ��һ��ָ��������
 *
 * @param task ����Ҫִ�е�����
 */
void ExecuteTask(ThreadBase * ExecutionTarget);

//for test
int GetMsgListCount();
int GetSocketListCount();

#endif
