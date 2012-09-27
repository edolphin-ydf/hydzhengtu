
#include "testServer.h"
#include "testTask.h"
testServer *testServer::instance = NULL;

bool testServer::init()
{
	printf("testServer::init\n");
	//��ʼ�����ݿ����ӣ�����
	
	//��ʼ��������
	if (!Server_MNet::init())
	{
		return false;
	}

	//��ʼ�������̳߳�
	int state = state_none;
	if (0)//�˴���ȫ�ֱ���������̳߳���ά��״̬������������
	{
		state = state_maintain;
	}
	testTaskPool = new CTCPTaskPool(1000,state);
	if (NULL == testTaskPool || !testTaskPool->init())
	{
		return false;
	}
	test_port = 5555;
	bool rec = Server_MNet::bind("���Զ˿�",test_port);

	/*if(!rec);
	{
	return false;
	}*/

	return true;
}

/**
* \brief �½���һ����������
* ʵ�ִ��麯��<code>Server_MNet::newTCPTask</code>
* \param sock TCP/IP����
* \param srcPort ������Դ�˿�
* \return �µ���������
*/
void testServer::newTCPTask( const SOCKET sock,const WORD srcPort )
{
	printf("testServer::newTCPTask\n");

	if (srcPort == test_port)
	{
		//һ���µĿͻ��˾�Ҫ����������֤
		CTCPTask *tcpTask = new testTask(testTaskPool,sock);
		if (NULL == tcpTask)
		{
			::closesocket(sock);
		}
		else if (!testTaskPool->addVerify(tcpTask))//������֤���ӵĶ���
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
