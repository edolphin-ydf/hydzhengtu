// Client.cpp : 定义控制台应用程序的入口点。
//

#include "testClient.h"


int main()
{
	sockStartup();

	testClient tClient("testClient1","127.0.0.1",5555);
	bool rec = tClient.connectTotestServer();
	if (rec)//连接成功
	{
	}
	sockCleanup();
	getchar();
	return 0;
}

