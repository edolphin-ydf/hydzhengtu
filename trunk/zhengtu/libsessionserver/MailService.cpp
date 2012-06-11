#include <zebra/SessionServer.h>

const dbCol mail_head_define[] = {
  { "ID",                zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "STATE",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "FROMNAME",          zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "DELTIME",           zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "ACCESSORY",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "ITEMGOT",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "TYPE",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { NULL,0,0}           
};

const dbCol mail_content_define[] = {
  { "ID",                zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "STATE",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "TONAME",            zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TITLE",             zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "ACCESSORY",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "ITEMGOT",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "TEXT",              zDBConnPool::DB_STR,   sizeof(char[255+1]) },
  { "SENDMONEY",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "RECVMONEY",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "SENDGOLD",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "RECVGOLD",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "TOID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "BIN",               zDBConnPool::DB_BIN,  sizeof(t_Object)},
  { NULL,0,0}           
};

const dbCol mail_state_define[] = {
  { "STATE",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { NULL,0,0}           
};

const dbCol mail_item_define[] = {
  { "ITEMGOT",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { NULL,0,0}           
};

const dbCol mail_turnback_define[] = {
  //{ "ID",                zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "STATE",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "FROMNAME",          zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TONAME",            zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TITLE",             zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TYPE",              zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "CREATETIME",        zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "DELTIME",           zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "ACCESSORY",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "ITEMGOT",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "TEXT",              zDBConnPool::DB_STR,   sizeof(char[255+1]) },
  { "RECVMONEY",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "RECVGOLD",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "FROMID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "TOID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { NULL,0,0}           
};

const dbCol mail_define[] = {
  //{ "ID",                zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "STATE",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "FROMNAME",          zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TONAME",            zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TITLE",             zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TYPE",              zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "CREATETIME",        zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "DELTIME",           zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "ACCESSORY",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "ITEMGOT",    zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "TEXT",              zDBConnPool::DB_STR,   sizeof(char[255+1]) },
  { "SENDMONEY",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "RECVMONEY",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "SENDGOLD",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "RECVGOLD",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "FROMID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "TOID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "ITEMID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "BIN",    zDBConnPool::DB_BIN,  sizeof(Cmd::Session::SessionObject)},
  { NULL,0,0}           
};

const dbCol mail_forward_define[] = {
  { "STATE",             zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "FROMNAME",          zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TONAME",            zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TYPE",              zDBConnPool::DB_BYTE,  sizeof(BYTE) },
  { "DELTIME",           zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "TEXT",              zDBConnPool::DB_STR,   sizeof(char[255+1]) },
  { "RECVMONEY",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "RECVGOLD",         zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "TOID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { NULL,0,0}           
};

const dbCol mail_check_define[] = {
  { "ID",                zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "TONAME",            zDBConnPool::DB_STR,   sizeof(char[MAX_NAMESIZE+1]) },
  { "TOID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { NULL,0,0}           
};

const dbCol mail_new_define[] = {
  { "ID",                zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { "TOID",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
  { NULL,0,0}           
};

MailService::MailService(){}
MailService::~MailService(){}

bool MailService::doMailCmd(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
  using namespace Cmd;
  using namespace Cmd::Session;
  switch (cmd->para)
  {
    case PARA_SCENE_SENDMAIL:
      {
        t_sendMail_SceneSession * rev = (t_sendMail_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->mail.fromName);
        if (!pUser)
          pUser = UserSessionManager::getInstance()->getUserByID(rev->mail.fromID);

        /*//ȡ��������ȷ��,����һЩ��Ʒ��ʧ
        if (!pUser && rev->mail.type!=Cmd::Session::MAIL_TYPE_ACTIVITY)
        {
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_SENDMAIL): �����ʼ�ʱδ�ҵ������� %s",rev->mail.fromName);
          return true;
        }
        */

        if (sendMail(* rev))
        {
          if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"���ͳɹ�");
          Zebra::logger->info("[�ʼ�]�ʼ����ͳɹ� %s->%s(%u)",rev->mail.fromName,rev->mail.toName,rev->mail.toID);

          UserSession * toUser = UserSessionManager::getInstance()->getUserSessionByName(rev->mail.toName);
          if (!toUser) toUser = UserSessionManager::getInstance()->getUserByID(rev->mail.toID);
          if (toUser)
          {
            Cmd::stNotifyNewMail n;
            toUser->sendCmdToMe(&n,sizeof(n));
          }
        }
        return true;
      }
      break;
    case PARA_SCENE_GET_MAIL_LIST:
      {
        t_getMailList_SceneSession * rev = (t_getMailList_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->tempID);
        if (!pUser)
        {
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_GET_MAIL_LIST): ȡ���ʼ��б�ʱδ�ҵ����");
          return true;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {               
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"ȡ���ʼ�ʧ��");
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_GET_MAIL_LIST): �õ����ݿ���ʧ��");
          return true;
        }

        char where[128];

        //�õ��ʼ��б�
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"TOID=%u",pUser->id);

        mailHeadInfo *mailList;
        DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`MAIL`",mail_head_define,where,"DELTIME DESC",(BYTE **)&mailList);
        SessionService::dbConnPool->putHandle(handle);

        if (mailList)
        {
          for (DWORD i=0; i< retcode; i++)
          {
            if (mailList[i].state==MAIL_STATE_DEL)
              continue;

            stAddListMail al;
            al.id = mailList[i].id;
            al.state = mailList[i].state;
            if (MAIL_TYPE_AUCTION==mailList[i].type || MAIL_TYPE_SYS==mailList[i].type)
              al.type=1;//ϵͳ�ʼ�
            strncpy(al.fromName,mailList[i].fromName,MAX_NAMESIZE);
            if (mailList[i].accessory && !mailList[i].itemGot)
              al.accessory = true;
            else
              al.accessory = false;
            zRTime ct;
            al.endTime = mailList[i].delTime>ct.sec()?mailList[i].delTime-ct.sec():0;

            pUser->sendCmdToMe(&al,sizeof(al));
          }
        }
        SAFE_DELETE_VEC(mailList);

        /*
        //�õ��ʼ��б�
        std::string escapeName;
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"TONAME='%s'",pUser->name);

        retcode = SessionService::dbConnPool->exeSelect(handle,"`MAIL`",mail_head_define,where,"DELTIME DESC",(BYTE **)&mailList);
        SessionService::dbConnPool->putHandle(handle);

        if (mailList)
        {
          for (DWORD i=0; i< retcode; i++)
          {
            if (mailList[i].state==MAIL_STATE_DEL
                || strncmp(mailList[i].toName,pUser->name,MAX_NAMESIZE))
              continue;

            stAddListMail al;
            al.id = mailList[i].id;
            al.state = mailList[i].state;
            strncpy(al.fromName,mailList[i].fromName,MAX_NAMESIZE);
            if (mailList[i].accessory && !mailList[i].itemGot)
              al.accessory = true;
            else
              al.accessory = false;
            zRTime ct;
            al.endTime = mailList[i].delTime>ct.sec()?mailList[i].delTime-ct.sec():0;

            pUser->sendCmdToMe(&al,sizeof(al));
          }
        }
        SAFE_DELETE_VEC(mailList);
        */
        return true;
      }
      break;
    case PARA_SCENE_OPEN_MAIL:
      {
        t_openMail_SceneSession * rev = (t_openMail_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->tempID);
        if (!pUser)
        {
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_OPEN_MAIL): ���ʼ�ʱδ�ҵ����");
          return true;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {               
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ʼ�ʧ��");
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_OPEN_MAIL): �õ����ݿ���ʧ��");
          return true;
        }

        std::string escapeName;
        char where[128];
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"ID=%u",rev->mailID);
        mailContentInfo content;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`MAIL`",mail_content_define,where,NULL,1,(BYTE*)&content);
        if (1 != retcode
            ||content.state==MAIL_STATE_DEL
            ||(strncmp(content.toName,pUser->name,MAX_NAMESIZE) && content.toID!=pUser->id))
        {
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }

        if (content.state==MAIL_STATE_NEW)
        {
          mailStateInfo st;
          st.state = MAIL_STATE_OPENED;
          retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_state_define,(BYTE*)&st,where);

          newMailMap[content.toID].erase(rev->mailID);//�����ʼ��б���ɾ��
        }
        SessionService::dbConnPool->putHandle(handle);

        stContentMail cm;
        cm.mailID = content.id;
        strncpy(cm.title,content.title,MAX_NAMESIZE);
        strncpy(cm.text,content.text,256);
        if (content.accessory>0 && content.itemGot==0)
        {
          cm.accessory = true;
          cm.sendMoney = content.sendMoney;
          cm.recvMoney = content.recvMoney;
          cm.sendGold = content.sendGold;
          cm.recvGold = content.recvGold;
          bcopy(&content.item.object,&cm.item,sizeof(t_Object),sizeof(cm.item));
        }
        else
        {
          cm.accessory = false;
        }
        pUser->sendCmdToMe(&cm,sizeof(cm));
#ifdef _DEBUG
        //Zebra::logger->debug("[�ʼ�]%s ���ʼ� id=%u ��Ʒ:thisID=%u ObjectID=%u",pUser->name,cm.mailID,cm.item.qwThisID,cm.item.dwObjectID);
#endif

        return true;
      }
      break;
    case PARA_SCENE_GET_MAIL_ITEM:
      {
        t_getMailItem_SceneSession * rev = (t_getMailItem_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->tempID);
        if (!pUser)
        {
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_GET_MAIL_ITEM): ��ȡ����ʱδ�ҵ����");
          return true;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {               
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ȡ����ʧ��");
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_GET_MAIL_ITEM): �õ����ݿ���ʧ��");
          return true;
        }

        std::string escapeName;
        char where[128];
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"ID=%u",rev->mailID);
        mailContentInfo content;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`MAIL`",mail_content_define,where,NULL,1,(BYTE*)&content);
        if (1 != retcode
            ||content.state==MAIL_STATE_DEL
            ||content.accessory!=1
            ||content.itemGot==1
            ||(strncmp(content.toName,pUser->name,MAX_NAMESIZE) && content.toID!=pUser->id))
        {
          //pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ܻ�ȡ����");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }
        if (content.item.object.qwThisID!=0 && rev->space==0)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��İ����ռ䲻��");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }
        if (content.sendMoney && rev->money+content.sendMoney>10000000)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"������ҳ���������,������ȡ����");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }
        if (content.sendGold && rev->gold+content.sendGold>10000000)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��Ľ�ҳ���������,������ȡ����");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }
        if (content.recvMoney>rev->money)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"������Ҳ���,������ȡ����");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }
        if (content.recvGold>rev->gold)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��Ľ�Ҳ���,������ȡ����");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }

        content.itemGot = 1;
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"ID=%u",rev->mailID);
        retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_content_define,(BYTE*)&content,where);
        SessionService::dbConnPool->putHandle(handle);

        t_getMailItemReturn_SceneSession gmir;
        gmir.userID = pUser->tempid;
        gmir.mailID = rev->mailID;
        gmir.sendMoney = content.sendMoney;
        gmir.recvMoney = content.recvMoney;
        gmir.sendGold = content.sendGold;
        gmir.recvGold = content.recvGold;
        bcopy(&content.item,&gmir.item,sizeof(Cmd::Session::SessionObject),sizeof(gmir.item));
        pUser->scene->sendCmd(&gmir,sizeof(gmir));

        return true;
      }
      break;
    case PARA_SCENE_GET_MAIL_ITEM_CONFIRM:
      {
        t_getMailItemConfirm_SceneSession * rev = (t_getMailItemConfirm_SceneSession *)cmd;
        /*
        //�Ѿ���ȡ�����ɹ�
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)
        {
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_GET_MAIL_ITEM_CONFIRM): ȷ�ϻ�ȡ����ʱδ�ҵ����");
          return true;
        }
        */

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {               
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_GET_MAIL_ITEM_CONFIRM): �õ����ݿ���ʧ��");
          return true;
        }

        char where[128];
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"ID=%u",rev->mailID);

        mailInfo mail;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`MAIL`",mail_define,where,NULL,1,(BYTE*)&mail);
        if (1 != retcode)
        {
          if (retcode!=0)
        Zebra::logger->error("[�ʼ�]%s ȷ�ϻ�ȡ�������� mailID=%u retCode=%d",mail.toName,rev->mailID,retcode);
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }

        if (mail.recvMoney)
          if (!sendMoneyMail(mail.toName,mail.toID,mail.fromName,mail.fromID,mail.recvMoney,"֧�����������"))
          {
            Zebra::logger->error("[�ʼ�]%s ֧������ʧ�� mailID=%u",mail.toName,rev->mailID);
          }
        //BYTE i = 1;
        //retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_item_define,&i,where);
        SessionService::dbConnPool->putHandle(handle);

        //if ((DWORD)-1 == retcode || 0 == retcode)
        //  Zebra::logger->error("[�ʼ�]%s ȷ�ϻ�ȡ�������� mailID=%u retCode=%d",mail.toName,rev->mailID,retcode);

        return true;
      }
      break;
    case PARA_SCENE_DEL_MAIL:
      {
        t_delMail_SceneSession * rev = (t_delMail_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->tempID);
        if (!pUser)
        {
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_DEL_MAIL): ɾ���ʼ�ʱδ�ҵ����");
          return true;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {               
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_DEL_MAIL): �õ����ݿ���ʧ��");
          return true;
        }

        char where[128];
        mailContentInfo st;
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"ID=%u",rev->mailID);
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`MAIL`",mail_content_define,where,NULL,1,(BYTE*)&st);
        if (1!=retcode
            ||st.state==MAIL_STATE_DEL
            ||(st.accessory==1 && st.itemGot==0)
            ||(strncmp(st.toName,pUser->name,MAX_NAMESIZE) && st.toID!=pUser->id))
        {
          if (st.accessory==1 && st.itemGot==0)
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻��ɾ�����������ʼ�");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }

        st.state = MAIL_STATE_DEL;
        retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_content_define,(BYTE*)&st,where);
        if (1 != retcode)
          Zebra::logger->error("[�ʼ�]ɾ���ʼ�ʧ�ܣ�mailID=%u,retcode=%u",rev->mailID,retcode);
        stDelMail dm;
        dm.mailID = rev->mailID;
        pUser->sendCmdToMe(&dm,sizeof(dm));
        Zebra::logger->info("[�ʼ�]%s ɾ���ʼ� mailID=%u",pUser->name,rev->mailID);
        SessionService::dbConnPool->putHandle(handle);

        return true;
      }
      break;
    case PARA_SCENE_CHECK_NEW_MAIL:
      {
        t_checkNewMail_SceneSession * rev = (t_checkNewMail_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)  return true;
        if (0==newMailMap[pUser->id].size()) return true;

        Cmd::stNotifyNewMail n;
        pUser->sendCmdToMe(&n,sizeof(n));

        return true;
      }
      break;
    case PARA_SCENE_TURN_BACK_MAIL:
      {
        t_turnBackMail_SceneSession * rev = (t_turnBackMail_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)
        {
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_TURN_BACK_MAIL): �˻��ʼ�ʱδ�ҵ����");
          return true;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�˻��ʼ�ʧ��");
          Zebra::logger->error("[�ʼ�]doMailCmd(PARA_SCENE_TURN_BACK_MAIL): �õ����ݿ���ʧ��");
          return true;
        }

        char where[128];
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"ID=%u",rev->mailID);
        mailTurnBackInfo info;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`MAIL`",mail_turnback_define,where,NULL,1,(BYTE*)&info);
        SessionService::dbConnPool->putHandle(handle);

        if (1!=retcode
            ||info.state==MAIL_STATE_DEL
            ||info.type==MAIL_TYPE_RETURN
            ||info.accessory!=1
            ||info.itemGot!=0
            ||(strncmp(info.toName,pUser->name,MAX_NAMESIZE) && info.toID!=pUser->id))
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����˻����ʼ�");
          return true;
        }
        if (turnBackMail(rev->mailID))
        {
          stDelMail dm;
          dm.mailID = rev->mailID;
          pUser->sendCmdToMe(&dm,sizeof(dm));
        Zebra::logger->info("[�ʼ�]%s �˻� %s ���ʼ� mailID=%u",pUser->name,info.fromName,rev->mailID);
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

bool MailService::sendTextMail(char * fromName,DWORD fromID,char * toName,DWORD toID,char * text,DWORD h,BYTE type)
{
  using namespace Cmd::Session;

  t_sendMail_SceneSession sm;
  sm.mail.state = MAIL_STATE_NEW;
  strncpy(sm.mail.fromName,fromName,MAX_NAMESIZE);
  strncpy(sm.mail.toName,toName,MAX_NAMESIZE);
  sm.mail.fromID = fromID;
  sm.mail.toID = toID;
  sm.mail.type = type;
  zRTime ct;
  sm.mail.createTime = ct.sec();
  sm.mail.delTime = sm.mail.createTime + 60*60*24*7;
  sm.mail.accessory = 0;
  sm.mail.itemGot = 0;
  strncpy(sm.mail.text,text,255-1);
  sm.mail.sendMoney = 0;
  sm.mail.recvMoney = 0;

  if ((DWORD)-1==h)
    return sendMail(sm);
  else
    return sendMail(h,sm);
}

bool MailService::sendMoneyMail(char * fromName,DWORD fromID,char * toName,DWORD toID,DWORD money,char * text,DWORD h,BYTE type,DWORD itemID)
{
  using namespace Cmd::Session;
  t_sendMail_SceneSession sm;

  sm.mail.state = MAIL_STATE_NEW;
  strncpy(sm.mail.fromName,fromName,MAX_NAMESIZE);
  strncpy(sm.mail.toName,toName,MAX_NAMESIZE);
  sm.mail.fromID = fromID;
  sm.mail.toID = toID;
  if (strstr(text,"��ѹ��"))
    strncpy(sm.mail.title,"��ѹ��",MAX_NAMESIZE);
  else if (strstr(text,"�ɹ��۳�"))
    strncpy(sm.mail.title,"�����ɹ�",MAX_NAMESIZE);
  else
    strncpy(sm.mail.title,"֧������",MAX_NAMESIZE);
  sm.mail.type = type;
  zRTime ct;
  sm.mail.createTime = ct.sec();
  sm.mail.delTime = sm.mail.createTime + 60*60*24*7;
  sm.mail.accessory = 1;
  sm.mail.itemGot = 0;
  _snprintf(sm.mail.text,255-1,"%s",text);
  sm.mail.sendMoney = money;
  sm.mail.recvMoney = 0;
  sm.mail.itemID = itemID;

  if ((DWORD)-1==h)
    return sendMail(sm);
  else
    return sendMail(h,sm);
}

bool MailService::sendMail(DWORD h,Cmd::Session::t_sendMail_SceneSession & sm)
{
  if (0==sm.mail.toID && 0==strncmp("",sm.mail.toName,MAX_NAMESIZE))
  {
    Zebra::logger->error("[�ʼ�]sendMoneyMail �ռ���Ϊ�� fromName=%s money=%u text=%s type=%u",sm.mail.fromName,sm.mail.sendMoney,sm.mail.text,sm.mail.type);
    return false;
  }

  connHandleID handle = (connHandleID)h;
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[�ʼ�]sendMail: ��Ч�����ݿ���");
    Zebra::logger->error("[�ʼ�]%s->%s ��ʧ money=%u item=%s",sm.mail.fromName,sm.mail.toName,sm.mail.sendMoney,sm.item.object.strName);
    return false;
  }
  DWORD retcode = SessionService::dbConnPool->exeInsert(handle,"`MAIL`",mail_define,(const BYTE *)&sm.mail);

  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("[�ʼ�]sendMail: �������ʼ����ݿ���� retcode=%d",retcode);
    Zebra::logger->error("[�ʼ�]%s->%s ��ʧ money=%u item=%s",sm.mail.fromName,sm.mail.toName,sm.mail.sendMoney,sm.item.object.strName);
    return false;
  }

  newMailMap[sm.mail.toID].insert(retcode);

  UserSession * toUser = UserSessionManager::getInstance()->getUserSessionByName(sm.mail.toName);
  if (toUser)
  {
    Cmd::stNotifyNewMail n;
    toUser->sendCmdToMe(&n,sizeof(n));
  }

  return true;
}

bool MailService::sendMail(Cmd::Session::t_sendMail_SceneSession & sm)
{
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[�ʼ�]sendMail: �õ����ݿ���ʧ��");
    Zebra::logger->error("[�ʼ�]%s->%s ��ʧ money=%u item=%s",sm.mail.fromName,sm.mail.toName,sm.mail.sendMoney,sm.item.object.strName);
    return false;
  }
  DWORD retcode = SessionService::dbConnPool->exeInsert(handle,"`MAIL`",mail_define,(const BYTE *)&sm.mail);
  SessionService::dbConnPool->putHandle(handle);

  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("[�ʼ�]sendMail: �������ʼ����ݿ���� retcode=%d",retcode);
    Zebra::logger->error("[�ʼ�]%s->%s ��ʧ money=%u item=%s",sm.mail.fromName,sm.mail.toName,sm.mail.sendMoney,sm.item.object.strName);
    return false;
  }

  newMailMap[sm.mail.toID].insert(retcode);

  UserSession * toUser = UserSessionManager::getInstance()->getUserSessionByName(sm.mail.toName);
  if (toUser)
  {
    Cmd::stNotifyNewMail n;
    toUser->sendCmdToMe(&n,sizeof(n));
  }

  return true;
}

/* \brief ��Ҫѭ��ִ�е��ʼ�����
 * 
 * ��������ʼ��Ƿ�ʱ��Ҫ�˻ء�ɾ��
 *
 */
void MailService::checkDB()
{
  using namespace Cmd::Session;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[�ʼ�]MailService::checkDB: �õ����ݿ���ʧ��");
    return;
  }

  zRTime ct;
  char where[128];

  /*
  //ת��û��Ҫ���ʼ�
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where),"STATE!=%u AND DELTIME<%lu AND ACCESSORY=1 AND ITEMGOT=0 AND TYPE=%u",MAIL_STATE_DEL,ct.sec(),MAIL_TYPE_RETURN);
  mailForwardInfo fi;
  fi.state = MAIL_STATE_NEW;
  strncpy(fi.fromName,"����������",MAX_NAMESIZE);
  strncpy(fi.toName,"�ʼ������ռ���",MAX_NAMESIZE);
  strncpy(fi.text,"�ռ����������ʼ�",MAX_NAMESIZE);
  fi.delTime = ct.sec()+86400*7;
  fi.recvMoney = 0;
  fi.recvGold = 0;
  fi.toID = 0;
  fi.type = MAIL_TYPE_MAIL;
  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_forward_define,(BYTE*)&fi,where);
  if ((DWORD)-1 == retcode)
  {
    //Zebra::logger->error("[�ʼ�]MailService::checkDB: ת�������ʼ�ʧ�� retCode=%d",retcode);
    SessionService::dbConnPool->putHandle(handle);
    return;
  }
  else
    //Zebra::logger->error("[�ʼ�]MailService::checkDB: ת�� %d ������ʼ�",retcode);
  */

  //ɾ�����ڵ��ʼ�
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where),"STATE!=%u AND DELTIME<%lu AND (ACCESSORY=0 OR ITEMGOT=1 OR TYPE=%u)",MAIL_STATE_DEL,ct.sec(),MAIL_TYPE_RETURN);
  mailStateInfo st;
  st.state = MAIL_STATE_DEL;
  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_state_define,(BYTE*)&st,where);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("[�ʼ�]MailService::checkDB: ɾ�������ʼ�ʧ�� retCode=%d",retcode);
    SessionService::dbConnPool->putHandle(handle);
    return;
  }
  else
    if (retcode) Zebra::logger->info("[�ʼ�]MailService::checkDB: ɾ�� %d ������ʼ�",retcode);

  //�˻ع��ڵ��ʼ�
  mailCheckInfo * checkList,* tempPoint;
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"STATE!=%u AND ACCESSORY=1 AND ITEMGOT=0 AND TYPE=%u AND DELTIME<%lu",MAIL_STATE_DEL,MAIL_TYPE_MAIL,ct.sec());
  retcode = SessionService::dbConnPool->exeSelect(handle,"`MAIL`",mail_check_define,where,NULL,(BYTE **)&checkList);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("[�ʼ�]MailService::checkDB: ������ʧ�� retCode=%d",retcode);
    SessionService::dbConnPool->putHandle(handle);
    return;
  }

  if (checkList)
  {
    tempPoint = &checkList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      turnBackMail(tempPoint->id);
      tempPoint++;
    }
    SAFE_DELETE_VEC(checkList);
  }

  //ɾ��10��֮ǰ��
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where),"DELTIME<%lu",ct.sec()-864000);
  retcode = SessionService::dbConnPool->exeDelete(handle,"`MAIL`",where);
  if (retcode) Zebra::logger->debug("[�ʼ�]ɾ�� %u ��10��֮ǰ���ʼ���¼",retcode);

  SessionService::dbConnPool->putHandle(handle);

  return;
}

/* \brief �˻�һ���ʼ�
 *  \param mailID �ʼ�ID
 *
 */
bool MailService::turnBackMail(DWORD mailID)
{
  using namespace Cmd::Session;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[�ʼ�]turnBackMail: �õ����ݿ���ʧ��");
    return false;
  }

  char where[128];
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"ID=%u",mailID);

  mailTurnBackInfo mail;
  DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`MAIL`",mail_turnback_define,where,NULL,1,(BYTE*)&mail);
  if (1 != retcode
      || mail.state==MAIL_STATE_DEL
      || mail.type!=MAIL_TYPE_MAIL
      || mail.accessory!=1
      || mail.itemGot!=0
      || (0==mail.fromID && 0==strncmp("",mail.fromName,MAX_NAMESIZE)))
      //|| 0==strncmp("",mail.fromName,MAX_NAMESIZE)
      //|| 0==mail.fromID)
  {
    Zebra::logger->error("[�ʼ�]�����˻��ʼ� mailID=%u retCode=%d",mailID,retcode);
    SessionService::dbConnPool->putHandle(handle);
    return false;
  }

  char temp[MAX_NAMESIZE];
  mail.state = MAIL_STATE_NEW;
  strncpy(temp,mail.fromName,MAX_NAMESIZE);
  strncpy(mail.fromName,mail.toName,MAX_NAMESIZE);
  strncpy(mail.toName,temp,MAX_NAMESIZE);
  strncpy(mail.title,"�˻ص���Ʒ",MAX_NAMESIZE);
  mail.type = MAIL_TYPE_RETURN;
  zRTime ct;
  mail.createTime = ct.sec();
  mail.delTime = mail.createTime + 60*60*24*7;
  _snprintf(mail.text,255-1,"%s л�����㷢�͵���Ʒ",mail.fromName);
  mail.recvMoney = 0;
  mail.recvGold = 0;
  DWORD t = mail.fromID;
  mail.fromID = mail.toID;
  mail.toID = t;

  retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_turnback_define,(BYTE*)&mail,where);
  SessionService::dbConnPool->putHandle(handle);
  if (1 != retcode)
  {
    Zebra::logger->error("[�ʼ�]�����ʼ�Updateʧ�ܣ�mailID=%u,retcode=%d",mailID,retcode);
    return false;
  }

  UserSession * toUser = UserSessionManager::getInstance()->getUserSessionByName(mail.toName);
  if (toUser)
  {
    Cmd::stNotifyNewMail n;
    toUser->sendCmdToMe(&n,sizeof(n));
  }
  return true;
}

/* \brief ����������ֻ�IDɾ�������ʼ�
 *  \param userName ����
 *  \param id ��ɫID
 *
 */
void MailService::delMailByNameAndID(char * userName,DWORD id)
{
  using namespace Cmd::Session;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("[�ʼ�]delMailbyName: �õ����ݿ���ʧ�� name=%s",userName);
    return;
  }

  char where[128];

  //�˻ر��˼ĵĴ���Ʒ���ʼ�
  mailCheckInfo * checkList,* tempPoint;
  bzero(where,sizeof(where));
  std::string escapeName;
  SessionService::dbConnPool->escapeString(handle,userName,escapeName);
  _snprintf(where,sizeof(where) - 1,"TOID=%u AND ACCESSORY=1 AND ITEMGOT=0 AND TYPE=%u AND STATE!=%u",id,MAIL_TYPE_MAIL,MAIL_STATE_DEL);
  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`MAIL`",mail_check_define,where,NULL,(BYTE **)&checkList);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("[�ʼ�]ɾ����������ʼ�ʧ�� retCode=%d",retcode);
    SessionService::dbConnPool->putHandle(handle);
    return;
  }

  if (checkList)
  {
    tempPoint = &checkList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      if (strncmp(tempPoint->toName,escapeName.c_str(),MAX_NAMESIZE)
          && tempPoint->toID!=id)
        continue;

      turnBackMail(tempPoint->id);
      tempPoint++;
    }
    SAFE_DELETE_VEC(checkList);
  }

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"TOID=%u AND STATE != %u",id,MAIL_STATE_DEL);

  mailStateInfo st;
  st.state = MAIL_STATE_DEL;

  retcode = SessionService::dbConnPool->exeUpdate(handle,"`MAIL`",mail_state_define,(BYTE*)&st,where);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
    Zebra::logger->error("[�ʼ�]delMailbyName:ɾ����ɫ�����ʼ�Updateʧ�ܣ�userName=%s,retcode=%d",userName,retcode);
  else
    Zebra::logger->info("[�ʼ�]delMailbyName:ɾ����ɫ�����ʼ���userName=%s,retcode=%d",userName,retcode);
}

void MailService::loadNewMail()
{
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("[�ʼ�]loadNewMail: �õ����ݿ���ʧ��");
    return;
  }

  char where[128];
  bzero(where,sizeof(where));

  mailNewInfo * newList,* tempPoint;
  _snprintf(where,sizeof(where) - 1,"STATE=1");
  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`MAIL`",mail_new_define,where,NULL,(BYTE **)&newList);
  SessionService::dbConnPool->putHandle(handle);

  if (newList)
  {
    for (DWORD i=0; i< retcode; i++)
    {
      tempPoint = &newList[i];
      newMailMap[tempPoint->toID].insert(tempPoint->id);
    }
    SAFE_DELETE_VEC(newList);
  }

  Zebra::logger->debug("[�ʼ�]loadNewMail %u�� %u��",newMailMap.size(),retcode);
}
