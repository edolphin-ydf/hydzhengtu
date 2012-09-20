#include "testClient.h"

bool testClient::connectTotestServer()
{
	if (!connect())
	{
		printf("���Ӳ��Է�����ʧ��");
		return false;
	}

	using namespace Cmd::Test;
	t_LoginTest tCmd;
	tCmd.wdServerID = 1;  //��ǰ������ID
	tCmd.wdServerType = 1;//��ǰ����������

	return sendCmd(&tCmd,sizeof(tCmd));
}

void testClient::run()
{
	CTCPClient::run();

	//������������ӶϿ����رշ�����
	printf("������������ӶϿ�,����ر�");
	
}

bool testClient::msgParse( const Cmd::Cmd_NULL *pNullCmd,const DWORD nCmdLen )
{
	return MessageQueue::msgParse(pNullCmd,nCmdLen);
}

bool testClient::cmdMsgParse( const Cmd::Cmd_NULL *pNullCmd,const DWORD nCmdLen)
{
	using namespace Cmd::Test;

	if (pNullCmd->cmd==CMD_TestServer)
	{
		switch (pNullCmd->para)
		{
		case PARA_CHK:
			{
				printf("recv check cmd;");
				return true;
			}
			break;
		default:
			break;
		}
	}

	printf("TestClient::cmdMsgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}
