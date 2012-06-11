/**
* \brief 定义服务器信息收集的客户端连接
*/

#include "FLServer.h"

using namespace Cmd;
/**
* \brief 构造函数
* \param ip 服务器地址
* \param port 服务器端口
*/
InfoClient::InfoClient(
					   const std::string &ip,
					   const WORD port) : zTCPClientTask(ip,port)
{
	Zebra::logger->debug("InfoClient::InfoClient");
}

/**
* \brief 析构函数
*
*/
InfoClient::~InfoClient()
{
	Zebra::logger->debug("InfoClient::~InfoClient");
}

int InfoClient::checkRebound()
{
	Zebra::logger->debug("InfoClient::checkRebound");
	int retcode = pSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = pSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
		if (nCmdLen <= 0)
			//这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
			return 0;
		else
		{
			using namespace Cmd::Info;

			t_LoginCmd *pCmd = (t_LoginCmd *)pstrCmd;
			if (CMD_LOGIN == pCmd->cmd
				&& PARA_LOGIN == pCmd->para)
			{
				Zebra::logger->debug("登陆服务器成功");
				return 1;
			}
			else
			{
				Zebra::logger->error("登陆服务器失败");
				return -1;
			}
		}
	}
	else
		return retcode;
}

void InfoClient::addToContainer()
{
	Zebra::logger->debug("InfoClient::addToContainer");
	InfoClientManager::getInstance().add(this);
}

void InfoClient::removeFromContainer()
{
	Zebra::logger->debug("InfoClient::removeFromContainer");
	InfoClientManager::getInstance().remove(this);
}

bool InfoClient::connect()
{
	Zebra::logger->debug("InfoClient::connect");
	if (!zTCPClientTask::connect())
		return false;

	using namespace Cmd::Info;
	t_LoginCmd cmd;
	return sendCmd(&cmd,sizeof(cmd));
}

bool InfoClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
	Zebra::logger->error("?? InfoClient::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif

	using namespace Cmd::Info;

	switch(pNullCmd->cmd)
	{
	case CMD_INFO:
		{
			switch(pNullCmd->para)
			{/*
			 case PARA_REQUEST_SERVERINFO:
			 {
			 t_Request_ServerInfo *pCmd = (t_Request_ServerInfo *)pNullCmd;
			 BYTE buff[zSocket::MAX_DATASIZE];
			 t_ServerInfo *cmd = (t_ServerInfo *)buff;
			 constructInPlace(cmd);
			 cmd->rTimestamp = pCmd->rTimestamp;
			 cmd->ConnNum = FLService::getInstance().getPoolSize();
			 cmd->ServerID = 0;
			 cmd->ServerType = FLService::getInstance().getType();
			 cmd->GameZone.id = 0;
			 bzero(cmd->ZoneName,MAX_NAMESIZE);

			 std::string xmlStr;
			 ServerInfoSingleton::instance().getServerInfo(xmlStr);
			 if (xmlStr.length() < 4096)
			 strncpy(cmd->xml,xmlStr.c_str(),4095);
			 else
			 Zebra::logger->warn("生成服务器信息xml过长");

			 return sendCmd(cmd,sizeof(t_ServerInfo) + strlen(cmd->xml));
			 }
			 break;
			 */
			case PARA_REQUEST_ONLINENUM:
				{
					t_Request_OnlineNum *pCmd = (t_Request_OnlineNum *)pNullCmd;

					t_OnlineNum cmd;

					cmd.rTimestamp  = pCmd->rTimestamp;
					cmd.OnlineNum   = GYListManager::getInstance().getOnline();
					cmd.ServerID    = 0;
					cmd.ServerType  = FLService::getInstance().getType();
					cmd.GameZone.id = 0;
					bzero(cmd.ZoneName,MAX_NAMESIZE);
					return sendCmd(&cmd,sizeof(cmd));
				}
				break;
			}
		}
		break;
	}

	Zebra::logger->error("InfoClient::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}

