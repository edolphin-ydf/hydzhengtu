/**
 * \brief 定义档案服务器连接客户端
 *
 * 负责与档案服务器交互，存取档案
 * 
 */

#include <zebra/SessionServer.h>

RecordClient *recordClient = NULL;

bool RecordClient::connectToRecordServer()
{
  if (!connect())
  {
    Zebra::logger->error("连接档案服务器失败");
    return false;
  }

  using namespace Cmd::Record;
  t_LoginRecord tCmd;
  tCmd.wdServerID = SessionService::getInstance().getServerID();
  tCmd.wdServerType = SessionService::getInstance().getServerType();

  return sendCmd(&tCmd,sizeof(tCmd));
}

void RecordClient::run()
{
  zTCPClient::run();

  //与档案服务器的连接断开，关闭服务器
  Zebra::logger->error("session 与record断开,服务关闭");
  SessionService::getInstance().Terminate();
  while(!SessionService::getInstance().isSequeueTerminate())
  {
    zThread::msleep(10);
  }
}

bool RecordClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  return MessageQueue::msgParse(pNullCmd,nCmdLen);
}

bool RecordClient::cmdMsgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  using namespace Cmd::Record;

  if (pNullCmd->cmd==CMD_SESSION)
  {
    switch (pNullCmd->para)
    {
      case PARA_CHK_USER_EXIST:
        {
          Cmd::Record::t_chkUserExist_SessionRecord* rev = 
            (Cmd::Record::t_chkUserExist_SessionRecord*)pNullCmd;
          
          RecommendM::getMe().processAddRecommended(rev);
          return true;
        }
        break;
      default:
        break;
    }
  }

  Zebra::logger->error("RecordClient::cmdMsgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

