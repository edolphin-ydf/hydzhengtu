/*
 �ļ��� : testServer.h
 ����ʱ�� : 2012/9/20
 ���� : hyd
 ���� : 
*/
#ifndef __testServer_H__
#define __testServer_H__

#include "Server.h"
#include "TCPTask.h"
class testServer : public Server_MNet
{
public:
	/**
	* \brief ���캯��
	*
	*/
	testServer() : Server_MNet("���Է�����")
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
	* \brief ��ȡ���ӳ��е�������
	* \return ������
	*/
	const int getPoolSize() const
	{
		return testTaskPool->getSize();
	}
	/**
	* \brief ��ȡ����������
	* \return ����������
	*/
	const WORD getType() const
	{
		return TESTSERVER;
	}

	void reloadConfig();
private:
	static testServer *instance;

	bool init(); //����Server����Main�е��ô˳�ʼ������
	void newTCPTask(const SOCKET sock,const WORD srcPort);
	void final();

	WORD test_port;
	CTCPTaskPool *testTaskPool;
};
#endif
