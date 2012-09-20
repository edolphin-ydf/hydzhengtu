/*
 文件名 : testClient.h
 创建时间 : 2012/9/19
 作者 : hyd
 功能 : 
*/
#ifndef __testClient_H__
#define __testClient_H__

#include "TcpClient.h"
#include "MessageQueue.h"

class testClient : public CTCPBufferClient,public MessageQueue
{
public:
	testClient(const std::string &name,const std::string &ip,const WORD port)
		: CTCPBufferClient(name,ip,port)
	{
	}

	bool connectTotestServer();

	void run();
	bool msgParse(const Cmd::Cmd_NULL *pNullCmd,const DWORD nCmdLen);
	bool cmdMsgParse(const Cmd::Cmd_NULL *,const DWORD);
	
};
#endif
