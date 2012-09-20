
/*
 文件名 : testTask.h
 创建时间 : 2012/9/20
 作者 : hyd
 功能 : 
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
	* \brief 虚析构函数
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
	* \brief 登陆错误,返回错误代码到客户端显示
	*
	* \param retcode 错误代码
	* \param tm 是否断开连接
	*/
	void TestReturn(const BYTE retcode,const bool tm = true)
	{
		using namespace Cmd;
		Test::t_LoginTest tCmd;
		sendCmd(&tCmd,sizeof(tCmd));

		//由于验证错误,需要断开连接
		//可能导致coredown,屏蔽测试
		if (tm) Terminate();
	}

	/**
	* \brief 判断连接是否过长
	* 如果连接太长,服务器应该主动断开连接
	* \param ct 当前时间
	* \return 时间是否过长
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
	* \brief 生命期时间
	*/
	RTime lifeTime;
	/**
	* \brief 临时唯一编号
	*
	*/
	DWORD tempid;
	/**
	* \brief 临时唯一编号分配器
	*
	*/
	static DWORD uniqueID;
	
	bool requestClient(const Cmd::Test::t_LoginTest *ptCmd);
};
#endif