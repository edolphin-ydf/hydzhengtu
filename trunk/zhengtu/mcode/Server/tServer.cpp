// Server.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "testServer.h"

int main()
{
	sockStartup();

	testServer::getInstance().main();

	sockCleanup();

	getchar();
	return 0;
}

