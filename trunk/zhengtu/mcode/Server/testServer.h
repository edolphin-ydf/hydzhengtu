/*
 文件名 : testServer.h
 创建时间 : 2012/9/20
 作者 : hyd
 功能 : 
*/
#ifndef __testServer_H__
#define __testServer_H__

#include "Server.h"
#include "TCPTask.h"
class testServer : public Server_MNet
{
public:
	/**
	* \brief 构造函数
	*
	*/
	testServer() : Server_MNet("测试服务器")
	{
		test_port = 0;
		testTaskPool = NULL;
	}
	~testServer()
	{
		instance = NULL;
		SAFE_DELETE(testTaskPool);
	}

	static testServer &getInstance()
	{
		if (NULL == instance)
			instance = new testServer();

		return *instance;
	}
	static void delInstance()
	{
		SAFE_DELETE(instance);
	}
	/**
	* \brief 获取连接池中的连接数
	* \return 连接数
	*/
	const int getPoolSize() const
	{
		return testTaskPool->getSize();
	}
	/**
	* \brief 获取服务器类型
	* \return 服务器类型
	*/
	const WORD getType() const
	{
		return TESTSERVER;
	}

	void reloadConfig();
private:
	static testServer *instance;

	bool init(); //基类Server会在Main中调用此初始化函数
	void newTCPTask(const SOCKET sock,const WORD srcPort);
	void final();

	WORD test_port;
	CTCPTaskPool *testTaskPool;
};
#endif
