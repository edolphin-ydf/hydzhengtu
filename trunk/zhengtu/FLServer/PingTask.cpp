/**
* \brief ����PING��������
*
*/

#include "FLServer.h"

int PingTask::verifyConn()
{
	Zebra::logger->debug("PingTask::verifyConn");
	int retcode = mSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
		if (nCmdLen <= 0)
			//����ֻ�Ǵӻ���ȡ���ݰ������Բ������û������ֱ�ӷ���
			return 0;
		else
		{
			using namespace Cmd;

			stLoginPing *ptCmd = (stLoginPing *)pstrCmd;

			Zebra::logger->debug("PingTask::verifyConn(%d,%d)",ptCmd->byCmd,ptCmd->byParam);
			if (PING_USERCMD == ptCmd->byCmd
				&& LOGIN_PING_PARA == ptCmd->byParam)  
			{
				Zebra::logger->debug("�ͻ�������ͨ����֤");
				return 1;
			}
			else
			{
				Zebra::logger->error("�ͻ�������ָ����֤ʧ��");
				return -1;
			}
		}  
	}
	else
		return retcode;
}

int PingTask::recycleConn()
{
	Zebra::logger->debug("PingTask::recycleConn");
	return 1;
	/*
	mutex.lock();
	while(!cmd_queue.empty())
	{
	zSocket::t_BufferCmd *ptCmd = cmd_queue.front();
	int retcode = pSocket->sendRawData_NoPoll(&ptCmd->pstrCmd[ptCmd->offset],ptCmd->nCmdLen - ptCmd->offset);
	if (retcode > 0)
	{
	ptCmd->offset += retcode;
	if (ptCmd->offset < ptCmd->nCmdLen)
	//�������û�з�����ɲ��ܷ�����һ������break
	break;
	else if (ptCmd->offset == ptCmd->nCmdLen)
	{
	//������巢�������continue
	cmd_queue.pop();
	SAFE_DELETE(ptCmd);
	}
	#if 0
	else if (ptCmd->offset > ptCmd->nCmdLen)
	//���ش��󣬲����ܳ����������
	assert(0);
	#endif
	}
	else if (0 == retcode)
	// should retry
	break;
	else if (-1 == retcode)
	{
	Zebra::logger->error("PingTask::recycleConn");
	break;
	}
	}
	mutex.unlock();
	return 1;
	*/
}

bool PingTask::msgParse(const Cmd::t_NullCmd *ptNull,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
	Zebra::logger->error("?? PingTask::msgParse(%d,%d,%d)",ptNull->cmd,ptNull->para,nCmdLen);
#endif

	using namespace Cmd;
	const stNullUserCmd *pNullCmd = (const stNullUserCmd *)ptNull;

	if (PING_USERCMD == pNullCmd->byCmd)
	{
		switch(pNullCmd->byParam)
		{
		case REQUEST_PING_LIST_PARA:
			{

				stRequestPingList* pingCmd = (stRequestPingList*)pNullCmd;
				GameZone_t gameZone;
				gameZone.id = pingCmd->id;
				//BYTE buf[zSocket::MAX_DATASIZE];

				//stPingList *retCmd=(stPingList *)buf;
				//constructInPlace(retCmd);

				stPingList retCmd;

				GYListManager::getInstance().full_ping_list(&retCmd,gameZone);
				//          sendCmd(retCmd,(retCmd->size*sizeof(Cmd::ping_element)+sizeof(Cmd::stRequestPingList)));
				Zebra::logger->info("PING_USERCMD,zone=%d,ip=%s",retCmd.zone_id,retCmd.ping_list.gateway_ip);
				sendCmd(&retCmd,sizeof(retCmd));

				return true;
			}
			break;
		}
	}

	Zebra::logger->error("PingTask::msgParse(%d,%d,%d)",ptNull->cmd,ptNull->para,nCmdLen);
	return false;
}

