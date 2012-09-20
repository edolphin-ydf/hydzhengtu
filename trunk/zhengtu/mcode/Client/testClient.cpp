#include "testClient.h"

bool testClient::connectTotestServer()
{
	if (!connect())
	{
		printf("连接测试服务器失败");
		return false;
	}

	using namespace Cmd::Test;
	t_LoginTest tCmd;
	tCmd.wdServerID = 1;  //当前服务器ID
	tCmd.wdServerType = 1;//当前服务器类型

	return sendCmd(&tCmd,sizeof(tCmd));
}

void testClient::run()
{
	CTCPClient::run();

	//与服务器的连接断开，关闭服务器
	printf("与服务器的连接断开,服务关闭");
	
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
