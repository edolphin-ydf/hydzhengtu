// Client.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "testClient.h"


int main()
{
	sockStartup();

	testClient tClient("testClient1","127.0.0.1",5555);
	bool rec = tClient.connectTotestServer();
	if (rec)//���ӳɹ�
	{
	}
	sockCleanup();
	getchar();
	return 0;
}

