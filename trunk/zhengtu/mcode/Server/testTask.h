
/*
 �ļ��� : testTask.h
 ����ʱ�� : 2012/9/20
 ���� : hyd
 ���� : 
*/
#ifndef __testTask_H__
#define __testTask_H__
#include "TCPTask.h"

class testTask : public CTCPTask
{

public:
	DWORD old;
	testTask( CTCPTaskPool *pool,const SOCKET sock);
	/**
	* \brief ����������
	*
	*/
	~testTask() {
	};

	int verifyConn();
	int recycleConn();
	void addToContainer();
	bool uniqueAdd();
	bool uniqueRemove();
	bool msgParse(const Cmd::Cmd_NULL *,const DWORD);

	void genTempID()
	{
		tempid = ++uniqueID;
	}

	const DWORD getTempID() const
	{
		return tempid;
	}

	/**
	* \brief ��½����,���ش�����뵽�ͻ�����ʾ
	*
	* \param retcode �������
	* \param tm �Ƿ�Ͽ�����
	*/
	void TestReturn(const BYTE retcode,const bool tm = true)
	{
		using namespace Cmd;
		Test::t_LoginTest tCmd;
		sendCmd(&tCmd,sizeof(tCmd));

		//������֤����,��Ҫ�Ͽ�����
		//���ܵ���coredown,���β���
		if (tm) Terminate();
	}

	/**
	* \brief �ж������Ƿ����
	* �������̫��,������Ӧ�������Ͽ�����
	* \param ct ��ǰʱ��
	* \return ʱ���Ƿ����
	*/
	bool timeout(const RTime &ct)
	{
		if (lifeTime.elapse(ct) >= 600)
			return true;
		else
			return false;
	}

private:
	/**
	* \brief ������ʱ��
	*/
	RTime lifeTime;
	/**
	* \brief ��ʱΨһ���
	*
	*/
	DWORD tempid;
	/**
	* \brief ��ʱΨһ��ŷ�����
	*
	*/
	static DWORD uniqueID;
	
	bool requestClient(const Cmd::Test::t_LoginTest *ptCmd);
};
#endif