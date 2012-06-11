/**
 * \brief 定义服务器信息收集的客户端连接
 */

#include "SuperServer.h"

DWORD InfoClient::tempidAllocator = 0;

/**
 * \brief 构造函数
 * \param ip 服务器地址
 * \param port 服务器端口
 */
InfoClient::InfoClient(
    const std::string &ip,
    const WORD port) : zTCPClientTask(ip,port,true),tempid(++tempidAllocator),netType(NetType_near)
{
}

/**
 * \brief 析构函数
 *
 */
InfoClient::~InfoClient()
{
}

int InfoClient::checkRebound()
{
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

      t_LoginCmd_OK *ptCmd = (t_LoginCmd_OK *)pstrCmd;
      if (CMD_LOGIN == ptCmd->cmd
          && PARA_LOGIN_OK == ptCmd->para)
      {
        Zebra::logger->debug("登陆InfoServer成功，收到区的编号：%u(%u,%u),%s,%u",
            ptCmd->gameZone.id,
            ptCmd->gameZone.game,
            ptCmd->gameZone.zone,
            ptCmd->name,
            ptCmd->netType);
        netType = (ptCmd->netType == 0 ? NetType_near : NetType_far);
        SuperService::getInstance().setZoneID(ptCmd->gameZone);
        SuperService::getInstance().setZoneName(ptCmd->name);
        return 1;
      }
      else
      {
        Zebra::logger->error("登陆InfoServer失败");
        return -1;
      }
    }
  }
  else
    return retcode;
}

void InfoClient::addToContainer()
{
  InfoClientManager::getInstance().add(this);
}

void InfoClient::removeFromContainer()
{
  InfoClientManager::getInstance().remove(this);
}

bool InfoClient::connect()
{
  if (!zTCPClientTask::connect())
    return false;

  using namespace Cmd::Info;
  t_LoginCmd cmd;
  strncpy(cmd.strIP,SuperService::getInstance().getIP(),sizeof(cmd.strIP));
  cmd.port = SuperService::getInstance().getPort();
  return sendCmd(&cmd,sizeof(cmd));
}

bool InfoClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
  Zebra::logger->error("?? InfoClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif 

  switch(pNullCmd->cmd)
  {
    case Cmd::GmTool::CMD_GMTOOL:
      {
        using namespace Cmd::GmTool;

        switch(pNullCmd->para)
        {
          case PARA_PUNISH_GMTOOL:
            {
              ServerManager::getInstance().broadcastByType(SCENESSERVER,pNullCmd,nCmdLen);
              return true;
            }
            break;
          case PARA_MSG_REPLY_GMTOOL:
          case PARA_BROADCAST_GMTOOL:
          case PARA_CHAT_GMTOOL:
            {
#ifdef _DEBUG
              if (PARA_BROADCAST_GMTOOL==pNullCmd->para)
              {
                t_Broadcast_GmTool * rev = (t_Broadcast_GmTool *)pNullCmd;
                Zebra::logger->debug("[GM工具]收到GM公告消息 %s:%s",rev->GM,rev->content);
              }
#endif
              ServerManager::getInstance().broadcastByType(SESSIONSERVER,pNullCmd,nCmdLen);
              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
    case Cmd::Info::CMD_INFO:
      {
        using namespace Cmd::Info;

        switch(pNullCmd->para)
        {
          case PARA_REQUEST_ONLINENUM:
            {
              t_Request_OnlineNum *ptCmd = (t_Request_OnlineNum *)pNullCmd;

              {
                //向会话服务器请求每个国家的信息
                Cmd::Super::t_Request_CountryOnline req;
                req.rTimestamp = ptCmd->rTimestamp;
                req.infoTempID = tempid;
                ServerManager::getInstance().broadcastByType(SESSIONSERVER,&req,sizeof(req));
              }

              t_OnlineNum cmd;
              cmd.rTimestamp = ptCmd->rTimestamp;
              cmd.OnlineNum = ServerManager::getInstance().caculateOnlineNum();
              cmd.ServerID = SuperService::getInstance().getID();
              cmd.ServerType = SuperService::getInstance().getType();
              cmd.GameZone = SuperService::getInstance().getZoneID();
              bzero(cmd.ZoneName,MAX_NAMESIZE);
              strncpy(cmd.ZoneName,SuperService::getInstance().getZoneName().c_str(),MAX_NAMESIZE);
              return sendCmd(&cmd,sizeof(cmd));
            }
            break;
        }
      }
      break;
  }

  Zebra::logger->error("InfoClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

