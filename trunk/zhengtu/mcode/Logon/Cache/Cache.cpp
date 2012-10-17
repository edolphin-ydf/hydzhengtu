// Cache.cpp : 定义控制台应用程序的入口点。
//

#include "r_common.h"

int w32initWinSock(void) {

	WSADATA t_wsa;
	WORD wVers;
	int iError;

	wVers = MAKEWORD(2, 2);
	iError = WSAStartup(wVers, &t_wsa);

	if(iError != NO_ERROR || LOBYTE(t_wsa.wVersion) != 2 || HIBYTE(t_wsa.wVersion) != 2 ) {
		return 0; /* not done; check WSAGetLastError() for error number */
	};

	return 1;
}


int main()
{
	if (!w32initWinSock()) {
		printf("Winsock init error %d", WSAGetLastError());
		exit(1);
	};

	redisContext *c = redisConnect("192.168.1.116", 6379);//6001
	if (c->err)
	{
		printf("Error: %s\n",c->errstr);
	}
	else
	{
		redisReply*	reply = (redisReply *)redisCommand(c,"GET foo");
		if (reply->len>0)
		{
			printf(reply->str);
		}
		
		redisCommand(c, "SET foo %s", "foo test1");
		reply = (redisReply *)redisCommand(c,"GET foo");
		printf(reply->str);
	}

	atexit((void(*)(void)) WSACleanup);
	return 0;
}

