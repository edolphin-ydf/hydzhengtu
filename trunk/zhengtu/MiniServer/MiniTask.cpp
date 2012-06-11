/**
 * \brief ʵ�ֶ���������
 *
 * 
 */

#include "MiniServer.h"

/**
 * \brief ��֤��½����������������ָ��
 *
 * �����֤��ͨ��ֱ�ӶϿ�����
 *
 * \param ptCmd ��½ָ��
 * \return ��֤�Ƿ�ɹ�
 */
bool MiniTask::verifyLogin(const Cmd::Mini::t_LoginMini *ptCmd)
{
  using namespace Cmd::Mini;

  if (CMD_LOGIN == ptCmd->cmd
      && PARA_LOGIN == ptCmd->para)
  {
    const Cmd::Super::ServerEntry *entry = MiniService::getInstance().getServerEntryById(ptCmd->wdServerID);
    char strIP[32];
    strncpy(strIP,getIP(),31);
    if (entry
        && ptCmd->wdServerType == entry->wdServerType
        && 0 == strcmp(strIP,entry->pstrIP))
    {
      wdServerID = ptCmd->wdServerID;
      wdServerType = ptCmd->wdServerType;
      return true;
    }
  }

  return false;
}

/**
 * \brief �ȴ�������ָ֤�������֤
 *
 * ʵ���麯��<code>zTCPTask::verifyConn</code>
 *
 * \return ��֤�Ƿ�ɹ�,���߳�ʱ
 */
int MiniTask::verifyConn()
{
  int retcode = mSocket->recvToBuf_NoPoll();
  if (retcode > 0)
  {
    BYTE pstrCmd[zSocket::MAX_DATASIZE];
    int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
    if (nCmdLen <= 0)
      //����ֻ�Ǵӻ���ȡ���ݰ�,���Բ������,û������ֱ�ӷ���
      return 0;
    else
    {
      using namespace Cmd::Mini;
      if (verifyLogin((t_LoginMini *)pstrCmd))
      {
        Zebra::logger->debug("�ͻ�������ͨ����֤");
        veriry_ok=true;
        return 1;
      }
      else
      {
        Zebra::logger->error("�ͻ���������֤ʧ��");
        return -1;
      }
    }
  }
  else
    return retcode;
}

bool MiniTask::checkRecycle()
{
  if (recycle_state == 0)
  {
    return false;
  }
  if (recycle_state == 1)
  {
    MiniUserManager::getInstance()->removeUserByGatewayID(this);
    recycle_state=2;
    return true;
  }
  return true;
}
/**
 * \brief ȷ��һ�����������ӵ�״̬�ǿ��Ի��յ�
 *
 * ��һ������״̬�ǿ��Ի��յ�״̬,��ô��ζ��������ӵ������������ڽ���,���Դ��ڴ��а�ȫ��ɾ���ˣ���<br>
 * ʵ�����麯��<code>zTCPTask::recycleConn</code>
 *
 * \return �Ƿ���Ի���
 */
int MiniTask::recycleConn()
{
  if (veriry_ok)
  {
    switch(recycle_state)
    {
      case 0:
        {
          recycle_state=1;
          return 0;
        }
        break;
      case 1:
        {
          return 0;
        }
        break;
      case 2:
        {
          return 1;
        }
        break;
    }
  }
  return 1;
}
/*
{

    MiniUserManager::getInstance()->removeUserByGatewayID(wdServerID);
  return 1;
}
// */

bool MiniTask::uniqueAdd()
{
  return MiniTaskManager::getInstance().uniqueAdd(this);
}

bool MiniTask::uniqueRemove()
{
  return MiniTaskManager::getInstance().uniqueRemove(this);
}

/**
 * \brief �������Ը������������ӵ�ָ��
 *
 * \param pNullCmd �������ָ��
 * \param nCmdLen ָ���
 * \return �����Ƿ�ɹ�
 */
bool MiniTask::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  return MessageQueue::msgParse(pNullCmd,nCmdLen);
}


bool MiniTask::cmdMsgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  using namespace Cmd::Mini;

  switch(pNullCmd->cmd)
  {
    case CMD_GATE:
      {
        return parseGateMsg(pNullCmd,nCmdLen);
      }
      break;
    case CMD_FORWARD:
      {
        return parseForwardMsg(pNullCmd,nCmdLen);
      }
      break;
    case CMD_SCENE:
      {
        return parseSceneMsg(pNullCmd,nCmdLen);
      }
      break;
    default:
      break;
  }

  Zebra::logger->error("MiniTask::cmdMsgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

bool MiniTask::parseGateMsg(const Cmd::t_NullCmd * cmd,const DWORD len)
{
  using namespace Cmd::Mini;

  if (!cmd || cmd->cmd!=CMD_GATE) return false;

  switch (cmd->para)
  {
    case PARA_GATE_USER_LOGIN:
      {
        t_UserLogin_Gateway *rev = (t_UserLogin_Gateway *)cmd;
        MiniUser * user = MiniUserManager::getInstance()->newUser(rev);
        if (!user)
        {
          Zebra::logger->error("�û���¼ʧ�ܣ�id=%u name=%s",rev->userID,rev->name);
          return false;
        }
        MiniHall::getMe().userEnter(user);
      }
      break;
    case PARA_GATE_USER_LOGOUT:
      {
        t_UserLogout_Gateway *rev = (t_UserLogout_Gateway *)cmd;
        MiniUser *u = MiniUserManager::getMe().getUserByID(rev->userID);
        if (u)
          MiniHall::getMe().userLeave(u);
        return true;
      }
      break;
    case PARA_GATE_USER_DELETE:
      {
        t_UserDelete_Gateway *rev = (t_UserDelete_Gateway *)cmd;

        DBFieldSet* fs = MiniService::metaData->getFields("MINIGAME");

        if (fs)
        {
          connHandleID handle = MiniService::dbConnPool->getHandle();

          if ((connHandleID)-1 == handle)
          {   
            Zebra::logger->error("newUser()���ܻ�ȡ���ݿ���");
            return false;
          }

          DBRecord where;
          char w[32];
          bzero(w,sizeof(w));
          _snprintf_s(w,sizeof(w)-1,"`CHARID`=%u",rev->userID);
          where.put("charid",w);

          if (MiniService::dbConnPool->exeDelete(handle,fs,&where))
            Zebra::logger->info("���ɾ��,����÷ּ�¼ id=%u",rev->userID);
          MiniService::dbConnPool->putHandle(handle);
        }
      }
      break;
    default:
      break;
  }

  return false;
}

bool MiniTask::parseForwardMsg(const Cmd::t_NullCmd * cmd,const DWORD len)
{
  using namespace Cmd::Mini;

  if (!cmd || cmd->cmd!=CMD_FORWARD) return false;

  switch (cmd->para)
  {
    /*
    case PARA_FORWARD_SCENE_TO_MINI:
      {
        t_Scene_ForwardMini *rev = (t_Scene_ForwardMini *)cmd;
        return true;
      }
      break;
      */
    case PARA_USER_FORWARD_MINI:
      {
        t_Mini_UserForwardMini *rev = (t_Mini_UserForwardMini *)cmd;
        MiniUser *u = MiniUserManager::getMe().getUserByID(rev->id);
        //if (!u) return false;
        Cmd::stMiniGameUserCmd *msg = (Cmd::stMiniGameUserCmd *)rev->data;
        if (msg->byCmd!=Cmd::MINIGAME_USERCMD) break;

        return MiniHall::getMe().parseUserCmd(u,msg,rev->size);
      }
      break;
    default:
      break;
  }

  return false;
}

bool MiniTask::parseSceneMsg(const Cmd::t_NullCmd * cmd,const DWORD len)
{  
  using namespace Cmd::Mini;
  using namespace Cmd;

  if (!cmd || cmd->cmd!=CMD_SCENE) return false;

  switch (cmd->para)
  {
    case PARA_SCENE_SET_SCENE:
      {
        t_Scene_SetScene *rev = (t_Scene_SetScene *)cmd;
        MiniUser *u = MiniUserManager::getMe().getUserByID(rev->userID);
        if (!u) return true;

        Zebra::logger->info("%s(%u) �л����������� %u",u->name,u->id,rev->serverID);
        MiniTask *t = MiniTaskManager::getInstance().getTaskByID(rev->serverID);
        if (!t)
        {
          //MiniHall::getMe().userLeave(u);
          return false;
        }

        u->setScene(t);
        return true;
      }
    case PARA_SCENE_DEPOSIT:
      {
        t_Scene_Deposit *rev = (t_Scene_Deposit *)cmd;
        MiniUser *u = MiniUserManager::getMe().getUserByID(rev->userID);
        if (!u)
        {
          if (addDBMoney(rev->userID,rev->num))
          {
            Zebra::logger->info("��� userID=%u ֱ��д���ݿ��ֵ money=%u �ɹ�",rev->userID,rev->num);
            return true;
          }
          else
            return false;
        }

        u->addMoney(rev->num);
        u->save();
        u->sendMiniInfo(Cmd::MCT_POPUP,"����ɹ���");
        Zebra::logger->info("%s(%u) ��ֵ %u,���� %u",u->name,u->id,rev->num,u->getMoney());
      }
      break;
    case PARA_SCENE_CHECK_DRAW:
      {
        t_Scene_Check_Draw *rev = (t_Scene_Check_Draw *)cmd;
        MiniUser *u = MiniUserManager::getMe().getUserByID(rev->userID);
        if (!u) return false;

        if (!u->checkMoney(rev->num))
        {
          stDrawRetCommonMiniGameCmd send;
          send.ret = 2;//�ɵ�����
          u->sendCmdToMe(&send,sizeof(send));
          return true;
        }

        t_Scene_Draw_Ret send;
        send.userID = u->id;
        send.num = rev->num;
        send.ret = 0;
        if (!u->sendCmdToScene(&send,sizeof(send)))
          return false;

        u->removeMoney(rev->num);

        if (u->save())
          Zebra::logger->info("%s(%u) �һ����� %u,���� %u",u->name,u->id,rev->num,u->getMoney());
      }
      break;
    case PARA_SCENE_DRAW_RET:
      {
        t_Scene_Draw_Ret *rev = (t_Scene_Draw_Ret *)cmd;
        MiniUser *u = MiniUserManager::getMe().getUserByID(rev->userID);
        if (rev->ret==1)
        {
          stDrawRetCommonMiniGameCmd send;
          send.ret = 1;//�ɹ�
          u->sendCmdToMe(&send,sizeof(send));
          Zebra::logger->info("�һ�������� userID=%u rev->num=%u",rev->userID,rev->num);
          return true;
        }
        else
        {
          if (u)
          {
            u->addMoney(rev->num);
            Zebra::logger->info("%s(%u) �һ��ɵ�ʧ��,�����ɹ� num=%u ���� %u",u->name,u->id,rev->num,u->getMoney());
            return true;
          }
          else
          {
            if (addDBMoney(rev->userID,rev->num))
            {
              Zebra::logger->info("�һ��ɵ�ʧ��,�����ɹ� userID=%u money=%u ret=%u",rev->userID,rev->num,rev->ret);
              return true;
            }
            else
            {
              Zebra::logger->error("�һ��ɵ�ʧ��,����Ҳʧ�� userID=%u money=%u ret=%u",rev->userID,rev->num,rev->ret);
              stDrawRetCommonMiniGameCmd send;
              send.ret = 0;//ʧ��
              u->sendCmdToMe(&send,sizeof(send));
              return true;
            }
          }
        }
      }
      break;
    default:
      break;
  }

  return false;
}

/**
 * \brief ��������������û�
 *
 * \param id �û�id
 * \param pstrCmd ����ָ��
 * \param nCmdLen �����
 */
bool MiniTask::sendCmdToScene(DWORD id,const void *pstrCmd,const DWORD nCmdLen)
{
  /*
  using namespace Cmd::Mini;
  using namespace Cmd;
  
  BYTE buf[zSocket::MAX_DATASIZE];
  t_Mini_ForwardMiniToScene *scmd=(t_Mini_ForwardMiniToScene *)buf;
  constructInPlace(scmd);
  
  scmd->id=id;
  scmd->size=nCmdLen;
  bcopy(pstrCmd,scmd->data,nCmdLen);
  return sendCmd(scmd,sizeof(t_Mini_ForwardMiniToScene)+nCmdLen);
  */
  return true;
}

/**
 * \brief ����������û�
 *
 * \param id �û�id
 * \param pstrCmd ����ָ��
 * \param nCmdLen �����
 */
bool MiniTask::sendCmdToUser(DWORD id,const void *pstrCmd,const DWORD nCmdLen)
{
  using namespace Cmd::Mini;
  using namespace Cmd;
  
  BYTE buf[zSocket::MAX_DATASIZE];
  t_Mini_ForwardUser *scmd=(t_Mini_ForwardUser *)buf;
  constructInPlace(scmd);
  
  scmd->id = id;
  scmd->size = nCmdLen;
  bcopy(pstrCmd,scmd->data,nCmdLen,sizeof(buf) - sizeof(t_Mini_ForwardUser));
  return sendCmd(scmd,sizeof(t_Mini_ForwardUser)+nCmdLen);
}

bool MiniTask::addDBMoney(DWORD userID,DWORD num)
{
  DBFieldSet* fs = MiniService::metaData->getFields("MINIGAME");

  if (fs)
  {
    connHandleID handle = MiniService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {   
      Zebra::logger->error("addDBMoney()���ܻ�ȡ���ݿ���");
      return false;
    }   

    DBRecord where;
    char w[32];
    bzero(w,sizeof(w));
    _snprintf_s(w,sizeof(w)-1,"`CHARID`=%u",userID);
    where.put("charid",w);

    DBRecordSet *recordset = MiniService::dbConnPool->exeSelect(handle,fs,NULL,&where);

    if (recordset)
    {
      DBRecord* rec = recordset->get(0);
      DWORD money = rec->get("money");

      if (money+num>MAX_MONEY)
        money = MAX_MONEY;
      else
        money += num;

      DBRecord r;
      r.put("money",money);

      if ((DWORD)-1==MiniService::dbConnPool->exeUpdate(handle,fs,&r,&where))
      {
        Zebra::logger->error("�����ݿ����ӽ�Ǯʱд���ݿ�ʧ��! userID=%u money=%u",userID,num);
        return false;
      }

      SAFE_DELETE(recordset);
      MiniService::dbConnPool->putHandle(handle);
      return true;
    }
    else
      Zebra::logger->error("�����ݿ����ӽ�Ǯʱû�ҵ���¼ userID=%u money=%u",userID,num);

    MiniService::dbConnPool->putHandle(handle);
  }
  return false;
}
