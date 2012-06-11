/**
 * \brief 实现网关服务器到档案服务器的连接客户端
 */

#include "GatewayServer.h"

RecordClient *recordClient = NULL;

/**
 * \brief 创建到档案服务器的连接
 *
 * \return 连接是否成功
 */
bool RecordClient::connectToRecordServer()
{
  if (!connect())
  {
    Zebra::logger->error("连接档案服务器失败");
    return false;
  }

  using namespace Cmd::Record;
  t_LoginRecord tCmd;
  tCmd.wdServerID = GatewayService::getInstance().getServerID();
  tCmd.wdServerType = GatewayService::getInstance().getServerType();

  return sendCmd(&tCmd,sizeof(tCmd));
}

/**
 * \brief 线程回调函数
 *
 */
void RecordClient::run()
{
  zTCPBufferClient::run();

  //与档案服务器的连接断开，关闭服务器
  GatewayService::getInstance().Terminate();
}

/**
 * \brief 解析来自档案服务器的指令
 *
 * \param pNullCmd 待解析的指令
 * \param nCmdLen 待解析的指令长度
 * \return 解析指令是否成功
 */
bool RecordClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
  Zebra::logger->error("?? RecordClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif

  using namespace Cmd::Record;

  if (pNullCmd->cmd==Cmd::Record::CMD_GATE)
  {
    switch(pNullCmd->para)
    {
      case PARA_GATE_DELCHAR_RETURN:
        {
          //删除角色成功
          t_DelChar_Return_GateRecord *rev = (t_DelChar_Return_GateRecord *)pNullCmd;
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(rev->accid);
          if (pUser && 1 == rev->retcode)
          {
            /*
            if (pUser->delSelectUserInfo(rev->id))
            {
              pUser->noCharInfo();
            }
            else
            {
              pUser->refreshCharInfo();
            }
            // */
            if (GatewayService::getInstance().rolereg_verify)
            {
              //需要通知注册服务器清除角色名称
              using namespace Cmd::Super;
              t_Charname_Gateway cmd;
              cmd.wdServerID = GatewayService::getInstance().getServerID();
              cmd.accid = pUser->accid;
              strncpy(cmd.name,rev->name,MAX_NAMESIZE);
              cmd.state = ROLEREG_STATE_CLEAN;
              GatewayService::getInstance().sendCmdToSuperServer(&cmd,sizeof(cmd));
            }
            Zebra::logger->info("删除角色成功：%u,%u",rev->accid,rev->id);
          }
          else
            Zebra::logger->warn("删除角色失败：%u,%u",rev->accid,rev->id);
          return true;
        }
        break;
      case PARA_GATE_CHECKNAME_RETURN:
        {
          t_CheckName_Return_GateRecord * rev=(t_CheckName_Return_GateRecord *)pNullCmd;
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(rev->accid);
          
          if (pUser)
          {
            Cmd::stCheckNameSelectUserCmd ret;
            ret.err_code = rev->err_code;
            strncpy(ret.name,rev->name,MAX_NAMESIZE-1);

            Zebra::logger->debug("检查角色名:%d",ret.err_code);
            return pUser->sendCmd(&ret,sizeof(ret));
          }
        }
        break;
      case PARA_GATE_CREATECHAR_RETURN:
        {
          t_CreateChar_Return_GateRecord * rev=(t_CreateChar_Return_GateRecord *)pNullCmd;
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(rev->accid);
          if (pUser)
          {
            if (rev->retcode == 0)
            {
              //创建角色失败，角色名称重复
              if (GatewayService::getInstance().rolereg_verify)
              {
                //需要通知注册服务器清除角色名称
                using namespace Cmd::Super;
                t_Charname_Gateway cmd;
                cmd.wdServerID = GatewayService::getInstance().getServerID();
                cmd.accid = pUser->accid;
                strncpy(cmd.name,pUser->createCharCmd.name,MAX_NAMESIZE);
                cmd.state = ROLEREG_STATE_CLEAN;
                GatewayService::getInstance().sendCmdToSuperServer(&cmd,sizeof(cmd));
              }

              pUser->nameRepeat();
            }
            else if (rev->retcode == 1)
            {
              //创建角色成功
              if (GatewayService::getInstance().rolereg_verify)
              {
                //需要通知注册服务器回写角色名称到数据库
                using namespace Cmd::Super;
                t_Charname_Gateway cmd;
                cmd.wdServerID = GatewayService::getInstance().getServerID();
                cmd.accid = pUser->accid;
                strncpy(cmd.name,pUser->createCharCmd.name,MAX_NAMESIZE);
                cmd.state = ROLEREG_STATE_WRITE;
                GatewayService::getInstance().sendCmdToSuperServer(&cmd,sizeof(cmd));
              }

              pUser->putSelectUserInfo(rev->charinfo);
              pUser->refreshCharInfo();
            }
          }
          return true;
        }
        break;
      case PARA_GATE_RET_SELECTINFO:
        {
          t_Ret_SelectInfo_GateRecord * rev=(t_Ret_SelectInfo_GateRecord *)pNullCmd;
          GateUser *pUser=GateUserManager::getInstance()->getUserByAccID(rev->accid);

          if (pUser && pUser->tempid == 0)
          {
            bool empty = true;
            pUser->setSelectUserInfo(rev);
            //Zebra::logger->debug("收到%u所有角色信息",rev->accid);
            for(int i = 0; i < Cmd::MAX_CHARINFO; i++)
            {
              if (rev->info[i].id!=0 && rev->info[i].id!=(DWORD)-1)
              {
                empty = false;
                Zebra::logger->info("角色信息:%ld,%d,%s,%d,%s,%d",
                    rev->accid,rev->info[i].id,rev->info[i].name,
                    rev->info[i].level,rev->info[i].mapName,rev->info[i].type);
              }
            }

            if (empty)
            {
              //Zebra::logger->debug("帐号没有角色信息 %u",rev->accid);
              pUser->noCharInfo();
            }
            else
            {
              pUser->refreshCharInfo();
            }
          }

          return true;
        }
        break;
      case PARA_GATE_COUNTRY_ORDER:
        {
          t_order_Country_GateRecord *cmd = (t_order_Country_GateRecord *)pNullCmd; 
          GatewayService::getInstance().country_info.setCountryOrder((Cmd::Session::CountrOrder *)&cmd->order);
          return true;
        }
        break;
    }
  }

  Zebra::logger->error("RecordClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}
