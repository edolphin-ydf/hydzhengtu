
#include "testServer.h"
#include "testTask.h"
testServer *testServer::instance = NULL;

bool testServer::init()
{
	printf("testServer::init\n");
	//初始化数据库连接，跳过
	
	//初始化网络框架
	if (!Server_MNet::init())
	{
		return false;
	}

	//初始化连接线程池
	int state = state_none;
	if (0)//此处读全局变量，如果线程池在维护状态，而进入条件
	{
		state = state_maintain;
	}
	testTaskPool = new CTCPTaskPool(1000,state);
	if (NULL == testTaskPool || !testTaskPool->init())
	{
		return false;
	}
	test_port = 5555;
	bool rec = Server_MNet::bind("测试端口",test_port);

	/*if(!rec);
	{
	return false;
	}*/

	return true;
}

/**
* \brief 新建立一个连接任务
* 实现纯虚函数<code>Server_MNet::newTCPTask</code>
* \param sock TCP/IP连接
* \param srcPort 连接来源端口
* \return 新的连接任务
*/
void testServer::newTCPTask( const SOCKET sock,const WORD srcPort )
{
	printf("testServer::newTCPTask\n");

	if (srcPort == test_port)
	{
		//一有新的客户端就要进行连接验证
		CTCPTask *tcpTask = new testTask(testTaskPool,sock);
		if (NULL == tcpTask)
		{
			::closesocket(sock);
		}
		else if (!testTaskPool->addVerify(tcpTask))//加入验证连接的队列
		{
			SAFE_DELETE(tcpTask);
		}
	}
}

void testServer::final()
{
	Server_MNet::final();
}

void testServer::reloadConfig()
{

}
