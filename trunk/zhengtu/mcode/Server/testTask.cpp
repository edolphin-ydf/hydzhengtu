
#include "testTask.h"

DWORD testTask::uniqueID = 0;

/**
* \brief ���캯��
* \param pool ���������ӳ�
* \param sock TCP/IP�׽ӿ�
*/
DWORD g_testTaskDebugValue = 0;
testTask::testTask( CTCPTaskPool *pool,const SOCKET sock) : CTCPTask(pool,sock,NULL,true,false),lifeTime()
{
	printf("testTask::testTask");
}

int testTask::verifyConn()
{
	int retcode = mSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE pstrCmd[CSocket::MAX_DATASIZE];
		int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
		if (nCmdLen <= 0)
			//����ֻ�Ǵӻ���ȡ���ݰ������Բ������û������ֱ�ӷ���
			return 0;
		else
		{
			printf("testTask::verifyConn");

			Cmd::Test::t_LoginTest *testCmd = (Cmd::Test::t_LoginTest *)pstrCmd;
			switch(testCmd->wdServerID)
			{
			case 1:
				printf("�ͻ�������ָ����֤ͨ��(%s:%u)",mSocket->getIP(),mSocket->getPort());
				return 1;
				break;
			case 2:
				printf("�ͻ�������ָ����֤ͨ��(%s:%u)",mSocket->getIP(),mSocket->getPort());
				return -1;
				break;
			}
		}
	}
	else
		return retcode;
}

int testTask::recycleConn()
{
	printf("testTask::recycleConn()");
	return 1;
}
//��Ҫ��ӵ�ȫ��������
void testTask::addToContainer()
{
	printf("testTask::addToContainer()");
	
}

bool testTask::uniqueAdd()
{
	printf("testTask::uniqueAdd()");
	return true;//LoginManager::getInstance().add(this)
}

bool testTask::uniqueRemove()
{
	printf("testTask::uniqueRemove");
	//LoginManager::getInstance().remove(this);
	return true;
}

bool testTask::requestClient(const Cmd::Test::t_LoginTest *ptCmd)
{
	printf("testTask::requestLogin");

	return true;  
}

bool testTask::msgParse(const Cmd::Cmd_NULL *pNullCmd,const DWORD nCmdLen)
{
	printf("testTask::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);

	Cmd::Test::t_LoginTest *testCmd = (Cmd::Test::t_LoginTest *)pNullCmd;
	switch(testCmd->wdServerID)
	{
	case 1:
		printf("testTask::msgParse ok");
		return true;
		break;
	}
	printf("=error=testTask::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}
