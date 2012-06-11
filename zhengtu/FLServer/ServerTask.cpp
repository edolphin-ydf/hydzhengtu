/**
* \brief 实现服务器连接类
*
* 
*/

#include "FLServer.h"

/**
* \brief 等待接受验证指令并进行验证
*
* 实现虚函数<code>zTCPTask::verifyConn</code>
*
* \return 验证是否成功，或者超时
*/
int ServerTask::verifyConn()
{
	Zebra::logger->debug("ServerTask::verifyConn");
	int retcode = mSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
		if (nCmdLen <= 0)
			//这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
			return 0;
		else
		{
			using namespace Cmd::FL;

			t_LoginFL *ptCmd = (t_LoginFL *)pstrCmd;
			if (CMD_LOGIN == ptCmd->cmd
				&& PARA_LOGIN == ptCmd->para
				)
			{
				bool mcheck = ServerACLSingleton::instance().check(getIP(),ptCmd->port,gameZone,name);
				if (mcheck)
				{
					Zebra::logger->debug("客户端连接通过验证(%s:%d)",getIP(),ptCmd->port);
					return 1;
				}else{
					Zebra::logger->error("客户端连接指令验证失败(%s:%d)ChcekFailer",getIP(),ptCmd->port);
					return -1;
				}
			}
			else
			{
				Zebra::logger->error("客户端连接指令验证失败(%s:%d)",getIP(),ptCmd->port);
				return -1;
			}
		}
	}
	else
		return retcode;
}

int ServerTask::waitSync()
{
	Zebra::logger->debug("ServerTask::waitSync");
	using namespace Cmd::FL;
	t_LoginFL_OK cmd;
	cmd.gameZone = gameZone;
	bzero(cmd.name,sizeof(cmd.name));
	strncpy(cmd.name,name.c_str(),sizeof(cmd.name) - 1);
	t_RQGYList_FL tRQ;
	if (sendCmd(&cmd,sizeof(cmd))
		&& sendCmd(&tRQ,sizeof(tRQ)))
		return 1;
	else
		return -1;
}

/**
* \brief 添加到唯一性验证容器中
* 实现了虚函数<code>zTCPTask::uniqueAdd</code>
*/
bool ServerTask::uniqueAdd()
{
	Zebra::logger->debug("ServerTask::uniqueAdd");
	return ServerManager::getInstance().uniqueAdd(this);
}

/**
* \brief 从唯一性验证容器中删除
* 实现了虚函数<code>zTCPTask::uniqueRemove</code>
*/
bool ServerTask::uniqueRemove()
{
	Zebra::logger->debug("ServerTask::uniqueRemove");
	GYListManager::getInstance().disableAll(gameZone);
	return ServerManager::getInstance().uniqueRemove(this);
}

/**
* \brief 解析来自各个服务器连接的指令
* \param pNullCmd 待处理的指令
* \param nCmdLen 指令长度
* \return 处理是否成功
*/
bool ServerTask::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
	Zebra::logger->error("?? ServerTask::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif
	Zebra::logger->error("a serverTask Cmd");
	using namespace Cmd::FL;

	switch(pNullCmd->cmd)
	{
	case CMD_GYLIST:
		if (msgParse_gyList(pNullCmd,nCmdLen)) return true;
		break;
	case CMD_SESSION:
		if (msgParse_session(pNullCmd,nCmdLen)) return true;
		break;
	}
	Zebra::logger->error("ServerTask::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}

bool ServerTask::msgParse_gyList(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
	using namespace Cmd::FL;

	switch(pNullCmd->para)
	{
	case PARA_FL_GYLIST:
		{
			t_GYList_FL *ptCmd = (t_GYList_FL *)pNullCmd;
			GYList gy;

			Zebra::logger->info("PARA_FL_GYLIST:zoneid=%d(gameid=%d,zone=%d),%d,%s:%d,onlines=%d,state=%d,version=%d",gameZone.id,gameZone.game,gameZone.zone,ptCmd->wdServerID,ptCmd->pstrIP,ptCmd->wdPort,ptCmd->wdNumOnline,ptCmd->state,ptCmd->zoneGameVersion);
			gy.wdServerID = ptCmd->wdServerID;
			bcopy(ptCmd->pstrIP,gy.pstrIP,sizeof(gy.pstrIP),sizeof(gy.pstrIP));
			gy.wdPort = ptCmd->wdPort;
			gy.wdNumOnline = ptCmd->wdNumOnline;
			gy.state = ptCmd->state;
			gy.zoneGameVersion = ptCmd->zoneGameVersion;

			return GYListManager::getInstance().put(gameZone,gy);
		}
		break;
	}

	Zebra::logger->error("ServerTask::msgParse_gyList(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}

bool ServerTask::msgParse_session(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
	using namespace Cmd::FL;

	switch(pNullCmd->para)
	{
	case PARA_SESSION_NEWSESSION:
		{
			t_NewSession_Session *ptCmd = (t_NewSession_Session *)pNullCmd;
			using namespace Cmd;
			stServerReturnLoginSuccessCmd tCmd;

			Zebra::logger->info("PARA_SESSION_NEWSESSION:%d,%d,%s,%d",ptCmd->session.accid,ptCmd->session.loginTempID,ptCmd->session.pstrIP,ptCmd->session.wdPort);
			tCmd.dwUserID = ptCmd->session.accid;
			tCmd.loginTempID = ptCmd->session.loginTempID;
			bcopy(ptCmd->session.pstrIP,tCmd.pstrIP,sizeof(tCmd.pstrIP),sizeof(tCmd.pstrIP));
			tCmd.wdPort = ptCmd->session.wdPort;
			bzero(tCmd.key,sizeof(tCmd.key));
			//密钥隐藏在一段数据中
			for (int i=0; i<256; i++)
				tCmd.key[i] = randBetween(0,255);

			do
			tCmd.key[58] = randBetween(0,248);
			while((tCmd.key[58]>49)&&((tCmd.key[58]<59)));

			bcopy(ptCmd->session.des_key,&tCmd.key[tCmd.key[58]],sizeof(ZES_cblock),sizeof(tCmd.key) - tCmd.key[58]);

			printf("客户端登陆成功\n");
			return LoginManager::getInstance().broadcast(ptCmd->session.loginTempID,&tCmd,sizeof(tCmd));
		}
		break;
	case PARA_SESSION_IDINUSE:
		{
			t_idinuse_Session *ptCmd = (t_idinuse_Session *)pNullCmd;
			using namespace Cmd;

			Zebra::logger->warn("PARA_SESSION_IDINUSE accid = %d",ptCmd->accid);
			LoginManager::getInstance().loginReturn(ptCmd->loginTempID,LOGIN_RETURN_IDINUSE);
			return true;
		}
		break;
	}
	Zebra::logger->error("ServerTask::msgParse_session(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}
