/**
 * \brief ʵ�����ط����������������������ӿͻ���
 */

#include "GatewayServer.h"

RecordClient *recordClient = NULL;

/**
 * \brief ����������������������
 *
 * \return �����Ƿ�ɹ�
 */
bool RecordClient::connectToRecordServer()
{
  if (!connect())
  {
    Zebra::logger->error("���ӵ���������ʧ��");
    return false;
  }

  using namespace Cmd::Record;
  t_LoginRecord tCmd;
  tCmd.wdServerID = GatewayService::getInstance().getServerID();
  tCmd.wdServerType = GatewayService::getInstance().getServerType();

  return sendCmd(&tCmd,sizeof(tCmd));
}

/**
 * \brief �̻߳ص�����
 *
 */
void RecordClient::run()
{
  zTCPBufferClient::run();

  //�뵵�������������ӶϿ����رշ�����
  GatewayService::getInstance().Terminate();
}

/**
 * \brief �������Ե�����������ָ��
 *
 * \param pNullCmd ��������ָ��
 * \param nCmdLen ��������ָ���
 * \return ����ָ���Ƿ�ɹ�
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
          //ɾ����ɫ�ɹ�
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
              //��Ҫ֪ͨע������������ɫ����
              using namespace Cmd::Super;
              t_Charname_Gateway cmd;
              cmd.wdServerID = GatewayService::getInstance().getServerID();
              cmd.accid = pUser->accid;
              strncpy(cmd.name,rev->name,MAX_NAMESIZE);
              cmd.state = ROLEREG_STATE_CLEAN;
              GatewayService::getInstance().sendCmdToSuperServer(&cmd,sizeof(cmd));
            }
            Zebra::logger->info("ɾ����ɫ�ɹ���%u,%u",rev->accid,rev->id);
          }
          else
            Zebra::logger->warn("ɾ����ɫʧ�ܣ�%u,%u",rev->accid,rev->id);
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

            Zebra::logger->debug("����ɫ��:%d",ret.err_code);
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
              //������ɫʧ�ܣ���ɫ�����ظ�
              if (GatewayService::getInstance().rolereg_verify)
              {
                //��Ҫ֪ͨע������������ɫ����
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
              //������ɫ�ɹ�
              if (GatewayService::getInstance().rolereg_verify)
              {
                //��Ҫ֪ͨע���������д��ɫ���Ƶ����ݿ�
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
            //Zebra::logger->debug("�յ�%u���н�ɫ��Ϣ",rev->accid);
            for(int i = 0; i < Cmd::MAX_CHARINFO; i++)
            {
              if (rev->info[i].id!=0 && rev->info[i].id!=(DWORD)-1)
              {
                empty = false;
                Zebra::logger->info("��ɫ��Ϣ:%ld,%d,%s,%d,%s,%d",
                    rev->accid,rev->info[i].id,rev->info[i].name,
                    rev->info[i].level,rev->info[i].mapName,rev->info[i].type);
              }
            }

            if (empty)
            {
              //Zebra::logger->debug("�ʺ�û�н�ɫ��Ϣ %u",rev->accid);
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
