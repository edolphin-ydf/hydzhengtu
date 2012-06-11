/**
 * \brief �����½��������
 *
 */

#include <zebra/SessionServer.h>
#include <zebra/csTurn.h>

/**
 * \brief ����ÿ���û��Ự��ͬһ���ҵĽ�ɫ����������Ϣ
 */
struct EveryUserSessionAction: public execEntry<UserSession>
{
  DWORD country;
  DWORD cmdLen;
  Cmd::stChannelChatUserCmd * revCmd;
  bool init(Cmd::stChannelChatUserCmd * rev,DWORD len)
  {
    UserSession *pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->pstrName);
    revCmd = rev;
    cmdLen = len;
    if (pUser) 
    {
      country = pUser->country;
      return true;
    }
    return false;
  }

  /**
   * \brief ����ÿ���û��Ự��ͬһ���ҵĽ�ɫ����������Ϣ
   * \param su �û��Ự
   * \return true �ɹ� false ʧ��
   */
  bool exec(UserSession *su)
  {
    if (country == su->country)
      su->sendCmdToMe(revCmd,cmdLen);
    return true;
  }
};


/**
 * \brief ����ÿ���û��Ự��ͬһ���ҵĽ�ɫ�����û�����
 */
struct OneCountryUserSessionAction: public execEntry<UserSession>
{
  DWORD country;
  DWORD cmdLen;
  Cmd::stNullUserCmd* sendCmd;

  void init(Cmd::stNullUserCmd * rev,DWORD len,DWORD countryID)
  {
    sendCmd = rev;
    cmdLen = len;

    country = countryID;
  }

  /**
   * \brief ����ÿ���û��Ự��ͬһ���ҵĽ�ɫ����������Ϣ
   * \param su �û��Ự
   * \return true �ɹ� false ʧ��
   */
  bool exec(UserSession *su)
  {
    if (country == su->country)
      su->sendCmdToMe(sendCmd,cmdLen);
    return true;
  }
};

/**
 * \brief ����ÿ���û��Ự����ɫ����������Ϣ
 */
struct broadcastToEveryUser: public execEntry<UserSession>
{
  DWORD cmdLen;
  Cmd::stChannelChatUserCmd * revCmd;
  bool init(Cmd::stChannelChatUserCmd * rev,DWORD len)
  {
    if (0==rev) return false;
    revCmd = rev;
    cmdLen = len;
    return true;
  }

  /**
   * \brief ����
   * \param su �û��Ự
   * \return true �ɹ� false ʧ��
   */
  bool exec(UserSession *su)
  {
    if (su) su->sendCmdToMe(revCmd,cmdLen);
    return true;
  }
};

/**
 * \brief �㲥֪ͨ��ÿ����ɫ
 */
struct broadcastRushToEveryUser: public execEntry<UserSession>
{
  char * pContent;

  bool init(char * content)
  {
    pContent = content;
    return true;
  }
  /**
   * \brief ����������Ϣ
   * \param su �û��Ự
   * \return true �ɹ� false ʧ��
   */
  bool exec(UserSession *su)
  {
    su->sendSysChat(Cmd::INFO_TYPE_GAME,pContent);
    return true;
  }
};

/**
 * \brief ��֤��½�Ự������������ָ��
 *
 * �����֤��ͨ��ֱ�ӶϿ�����
 *
 * \param ptCmd ��½ָ��
 * \return ��֤�Ƿ�ɹ�
 */
bool SessionTask::verifyLogin(const Cmd::Session::t_LoginSession *ptCmd)
{
  if (Cmd::Session::CMD_LOGIN == ptCmd->cmd
      && Cmd::Session::PARA_LOGIN == ptCmd->para
      && (SCENESSERVER == ptCmd->wdServerType || GATEWAYSERVER == ptCmd->wdServerType))
  {
    const Cmd::Super::ServerEntry *entry = SessionService::getInstance().getServerEntryById(ptCmd->wdServerID);
    char strIP[32];
    strncpy(strIP,getIP(),31);

    if (NULL != entry){
      Zebra::logger->debug("SessionTask::verifyLogin %s,%d,%d for %s,%d,%d",entry->pstrIP,entry->wdServerID,entry->wdServerType,strIP,ptCmd->wdServerID,ptCmd->wdServerType);
    }
    else{
      Zebra::logger->error("SessionTask::verifyLogin NULL for %s,%d,%d",strIP,ptCmd->wdServerID,ptCmd->wdServerType);
    }

    if (entry
        && ptCmd->wdServerType == entry->wdServerType
        && 0 == strcmp(strIP,entry->pstrIP))
    {
      wdServerID   = ptCmd->wdServerID;
      wdServerType = ptCmd->wdServerType;
      return true;
    }
  }

    Zebra::logger->error("!SessionTask::verifyLogin cmd=%d,para=%d,wdServerID=%d,wdServerType=%d",ptCmd->cmd,ptCmd->para,ptCmd->wdServerID,ptCmd->wdServerType);
  return false;
}

int SessionTask::verifyConn()
{
  int retcode = mSocket->recvToBuf_NoPoll();
  if (retcode > 0)
  {
    BYTE pstrCmd[zSocket::MAX_DATASIZE];
    int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
    if (nCmdLen <= 0)
      //����ֻ�Ǵӻ���ȡ���ݰ������Բ������û������ֱ�ӷ���
      return 0;
    else
    {
      if (verifyLogin((Cmd::Session::t_LoginSession *)pstrCmd))
      {
        Zebra::logger->debug("�ͻ�������ͨ����֤");
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

bool SessionTask::checkRecycle()
{
  if (recycle_state == 0)
  {
    return false;
  }
  if (recycle_state == 1)
  {
    //�����Ѿ�ע����û�
    UserSessionManager::getInstance()->removeAllUserByTask(this);
    //ע���Ѿ�ע��ĵ�ͼ
    SceneSessionManager::getInstance()->removeAllSceneByTask(this);
    recycle_state=2;
    return true;
  }
  return true;
}
int SessionTask::recycleConn()
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
  return 1;
}

void SessionTask::addToContainer()
{
  SessionTaskManager::getInstance().addSessionTask(this);
}

void SessionTask::removeFromContainer()
{
  SessionTaskManager::getInstance().removeSessionTask(this);
}

bool SessionTask::uniqueAdd()
{
  return SessionTaskManager::getInstance().uniqueAdd(this);
}

bool SessionTask::uniqueRemove()
{
  return SessionTaskManager::getInstance().uniqueRemove(this);
}

/**
 * \brief ��������
 *
 * \param dwUserID : �����������û�ID
 *
 */
bool SessionTask::change_country(const Cmd::Session::t_changeCountry_SceneSession* cmd)
{
  CUnionM::getMe().fireUnionMember(cmd->dwUserID,false);
  CSeptM::getMe().fireSeptMember(cmd->dwUserID,false); 

  //DBFieldSet* samplerelation = SessionService::metaData->getFields("SAMPLERELATION");
  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(cmd->dwUserID);
  if (!pUser) return true;
  CSchoolM::getMe().fireSchoolMember(pUser->name,false);
  CCountry* pCountry = CCountryM::getMe().find(pUser->country);

  if (pCountry)
  {
    if (strncmp(pCountry->diplomatName,pUser->name,MAX_NAMESIZE) == 0)
    {
      pCountry->cancelDiplomat();
    }

    if (strncmp(pCountry->catcherName,pUser->name,MAX_NAMESIZE) == 0)
    {
      pCountry->cancelCatcher();
    }
  }
  
  /*DBRecord where;
  std::ostringstream oss;
  oss << "charid='" << cmd->dwUserID << "'";
  where.put("charid",oss.str());

  if (samplerelation)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    DBRecordSet* recordset = NULL;

    if ((connHandleID)-1 != handle)
    {
      recordset = SessionService::dbConnPool->exeSelect(handle,samplerelation,NULL,&where);
    }

    SessionService::dbConnPool->putHandle(handle);

    if (recordset)
    {//����ҵ����й�ϵ
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);
        UserSession *pOtherUser = NULL;

        if (rec)
        {
          pOtherUser = UserSessionManager::getInstance()->getUserSessionByName
            (rec->get("relationname"));
        }

        if (pOtherUser)
        {
          pOtherUser->relationManager.removeRelation(pUser->name);
          if (pUser) pUser->relationManager.removeRelation(pOtherUser->name);
        }
        else
        {
          connHandleID handle = SessionService::dbConnPool->getHandle();

          where.clear();
          oss.str("");
          oss << "relationid='" << cmd->dwUserID << "'";
          where.put("relationid",oss.str());

          if ((connHandleID)-1 != handle)
          {
            SessionService::dbConnPool->exeDelete(handle,samplerelation,&where);
          }

          SessionService::dbConnPool->putHandle(handle);
        }

        where.clear();
        oss.str("");
        oss << "charid='" << cmd->dwUserID << "'";
        where.put("charid",oss.str());
        connHandleID handle = SessionService::dbConnPool->getHandle();

        if ((connHandleID)-1 != handle)
        {
          SessionService::dbConnPool->exeDelete(handle,samplerelation,&where);
        }
        SessionService::dbConnPool->putHandle(handle);
      }
    }

    // ---------------��������йص���������ϵ��¼---------------
    SAFE_DELETE(recordset);

    where.clear();
    oss.str("");
    oss << "relationid='" << cmd->dwUserID << "'";
    where.put("relationid",oss.str());
    handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 != handle)
    {
      recordset = SessionService::dbConnPool->exeSelect(handle,samplerelation,NULL,&where);
    }

    SessionService::dbConnPool->putHandle(handle);

    if (recordset)
    {//����ҵ����й�ϵ
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);
        UserSession *pOtherUser = NULL;
        DWORD charid = 0;

        if (rec)
        {
          charid = rec->get("charid");
#ifdef _DEBUG
          Zebra::logger->debug("charid:%d",charid);      
#endif              
          pOtherUser = UserSessionManager::getInstance()->getUserByID
            (charid);
        }

        if (pOtherUser)
        {
          pOtherUser->relationManager.removeRelation(pUser->name);
        }
        else
        {
          connHandleID handle = SessionService::dbConnPool->getHandle();

          where.clear();
          oss.str("");
          oss << "relationid='" << cmd->dwUserID << "'";
          where.put("relationid",oss.str());

          oss.str("");
          if (charid)
          {
            oss << "charid='" << charid << "'";
            where.put("charid",oss.str());
          }

          if ((connHandleID)-1 != handle)
          {
            SessionService::dbConnPool->exeDelete(handle,samplerelation,&where);
          }

          SessionService::dbConnPool->putHandle(handle);
        }
      }
      SAFE_DELETE(recordset);
    }
    SAFE_DELETE(recordset);
  }
  */
  
  Cmd::stUnmarryCmd unmarry;
  pUser->relationManager.processUserMessage(&unmarry,sizeof(unmarry));

  if (pUser)
  {
    pUser->relationManager.sendRelationList();
    pUser->country = cmd->dwToCountryID; 
  }

  Cmd::Session::t_returnChangeCountry_SceneSession send;
  send.dwUserID = cmd->dwUserID;
  if (pUser && pUser->scene) pUser->scene->sendCmd(&send,sizeof(send));

  return true;
}



/**
 * \brief ɾ����ɫ
 *
 * \param cmd : ɾ����ɫ����
 * \param cmdLen: �����
 *
 *
 */
bool SessionTask::del_role(const Cmd::t_NullCmd* cmd,const DWORD cmdLen)
{
  Cmd::Session::t_DelChar_GateSession *rev=(Cmd::Session::t_DelChar_GateSession *)cmd;
  using namespace std;

  MailService::getMe().delMailByNameAndID(rev->name,rev->id);
  AuctionService::getMe().delAuctionRecordByName(rev->name);
  CartoonPetService::getMe().delPetRecordByID(rev->id);

  int retUnion  = 0;
  int retSept     = 0;
  int retSchool  = 0;

  rev->status = 0;

  retUnion  = CUnionM::getMe().fireUnionMember(rev->id,false);
  retSept   = CSeptM::getMe().fireSeptMember(rev->id,false); 
  retSchool = CSchoolM::getMe().fireSchoolMember(rev->name,false);
  RecommendM::getMe().fireRecommendSub(rev->id);

  //  if ((retUnion > 0 )
  //    && (retSept>0)
  //    && (retSchool>0)
  //    )
  {// û�а�����ʦ���峤����������������˳�����ϵ�����ٽ�����Ӧ�Ĵ���
    // ֻҪ��һ������ϵ�����˳�����ȡ������(����ֱ�ӽ������ϵ�������ж���)
    //int ret = 0;

    /*ret = CUnionM::getMe().fireUnionMember(rev->id,false);

      if (ret<=0)
      {
      rev->status = 4;
      }

      ret = CSeptM::getMe().fireSeptMember(rev->id,false);

      if (ret<=0)
      {
      rev->status = 4;
      }

      ret = CSchoolM::getMe().fireSchoolMember(rev->name,false);

      if (ret<=0)
      {
      rev->status = 4;
      }*/

    // �����ݿ��ж�ȡ�ҵ�����ϵ�����ж϶Է��Ƿ����ߣ���������ߣ���ֱ��ɾ�����ݿ��¼�������û����ߴ�����
    // ��ɾ����ϵ������͸��������ó�����������
    // ������ߣ���ɾ�����Լ������ݿ��¼�������öԷ���relationManager->removeRelation(�ҵĽ�ɫ����)
    DBFieldSet* samplerelation = SessionService::metaData->getFields("SAMPLERELATION");

    DBRecord where;
    std::ostringstream oss;
    oss << "charid='" << rev->id << "'";
    where.put("charid",oss.str());

    if (samplerelation)
    {
      connHandleID handle = SessionService::dbConnPool->getHandle();

      DBRecordSet* recordset = NULL;

      if ((connHandleID)-1 != handle)
      {
        recordset = SessionService::dbConnPool->exeSelect(handle,samplerelation,NULL,&where);
      }

      SessionService::dbConnPool->putHandle(handle);

      if (recordset)
      {//����ҵ����й�ϵ
        for (DWORD i=0; i<recordset->size(); i++)
        {
          DBRecord* rec = recordset->get(i);

          UserSession *pOtherUser = NULL;

          if (rec)
          {
            pOtherUser = UserSessionManager::getInstance()->getUserSessionByName
              (rec->get("relationname"));
          }

          if (pOtherUser)
          {
            pOtherUser->relationManager.removeRelation(rev->name);
          }
          else
          {
            connHandleID handle = SessionService::dbConnPool->getHandle();

            where.clear();
            oss.str("");
            oss << "relationid='" << rev->id << "'";
            where.put("relationid",oss.str());

            if ((connHandleID)-1 != handle)
            {
              SessionService::dbConnPool->exeDelete(handle,samplerelation,&where);
            }

            SessionService::dbConnPool->putHandle(handle);
          }

          where.clear();
          oss.str("");
          oss << "charid='" << rev->id << "'";
          where.put("charid",oss.str());
          connHandleID handle = SessionService::dbConnPool->getHandle();

          if ((connHandleID)-1 != handle)
          {
            SessionService::dbConnPool->exeDelete(handle,samplerelation,&where);
          }
          SessionService::dbConnPool->putHandle(handle);
        }
      }


      // ---------------��������йص���������ϵ��¼---------------
      SAFE_DELETE(recordset);

      where.clear();
      oss.str("");
      oss << "relationid='" << rev->id << "'";
      where.put("relationid",oss.str());
      handle = SessionService::dbConnPool->getHandle();


      if ((connHandleID)-1 != handle)
      {
        recordset = SessionService::dbConnPool->exeSelect(handle,samplerelation,NULL,&where);
      }

      SessionService::dbConnPool->putHandle(handle);

      if (recordset)
      {//����ҵ����й�ϵ
        for (DWORD i=0; i<recordset->size(); i++)
        {
          DBRecord* rec = recordset->get(i);
          UserSession *pOtherUser = NULL;
          DWORD charid = 0;

          if (rec)
          {
            charid = rec->get("charid");
#ifdef _DEBUG
            Zebra::logger->debug("charid:%d",charid);      
#endif              
            pOtherUser = UserSessionManager::getInstance()->getUserByID
              (charid);
          }

          if (pOtherUser)
          {
            pOtherUser->relationManager.removeRelation(rev->name);
          }
          else
          {
            connHandleID handle = SessionService::dbConnPool->getHandle();

            where.clear();
            oss.str("");
            oss << "relationid='" << rev->id << "'";
            where.put("relationid",oss.str());

            oss.str("");
            if (charid)
            {
              oss << "charid='" << charid << "'";
              where.put("charid",oss.str());
            }

            if ((connHandleID)-1 != handle)
            {
              SessionService::dbConnPool->exeDelete(handle,samplerelation,&where);
            }

            SessionService::dbConnPool->putHandle(handle);
          }
        }
        SAFE_DELETE(recordset)
      }
    }
  }

  if (rev->status == 0)
  {
    Zebra::logger->info("�ѽ������ϵ������ɾ����ɫ�Ĵ���");
  }
  else
  {
    Zebra::logger->info("���ڲ��ܽ��������ϵ��ɾ����ɫ������ȡ��");
  }

  this->sendCmd(rev,sizeof(Cmd::Session::t_DelChar_GateSession));

  return true;
}

bool SessionTask::msgParse_Scene(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
  if (CUnionM::getMe().processSceneMessage(cmd,cmdLen)) return true; //���ĳ�����������Ϣ����
  if (CSchoolM::getMe().processSceneMessage(cmd,cmdLen)) return true;
  if (CSeptM::getMe().processSceneMessage(cmd,cmdLen)) return true;
  if (CQuizM::getMe().processSceneMessage(cmd,cmdLen)) return true;
  if (CNpcDareM::getMe().processSceneMessage(cmd,cmdLen)) return true;
  if (MailService::getMe().doMailCmd(cmd,cmdLen)) return true;

  switch(cmd->para)
  {
    case Cmd::Session::PARA_DEBUG_COUNTRYPOWER:
      {
        time_t timValue = time(NULL);
        struct tm tmValue;
        zRTime::getLocalTime(tmValue,timValue);
        SessionService::getInstance().checkCountry(tmValue,true);
        return true;
      }
      break;
    case Cmd::Session::PARA_CLOSE_NPC:
      {
        SessionTaskManager::getInstance().broadcastScene(cmd,cmdLen);
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_SEND_CMD:
      {
        Cmd::Session::t_sendCmd_SceneSession *rev = (Cmd::Session::t_sendCmd_SceneSession *)cmd;
        SceneSession * s = SceneSessionManager::getInstance()->getSceneByID(rev->mapID);
        if (!s) return true;

        return s->sendCmd(rev,sizeof(Cmd::Session::t_sendCmd_SceneSession)+rev->len);
      }
      break;
    case Cmd::Session::PARA_SET_SERVICE:
      {
        Cmd::Session::t_SetService_SceneSession *rev = (Cmd::Session::t_SetService_SceneSession *)cmd;

        char buf[32];
        bzero(buf,sizeof(buf));
        _snprintf(buf,32,"%u",rev->flag);
        Zebra::global["service_flag"] = buf;

        SessionTaskManager::getInstance().broadcastScene(cmd,cmdLen);
        return true;
      }
      break;
    case Cmd::Session::PARA_ADD_RELATION_ENEMY:
      {
        Cmd::Session::t_addRelationEnemy *rev = (Cmd::Session::t_addRelationEnemy *)cmd;
        UserSession* pUser = UserSessionManager::getInstance()->getUserByID(rev->dwUserID);
        if (!pUser) return false;
        pUser->relationManager.addEnemyRelation(rev->name);
        return true;
      }
      break;
    case Cmd::Session::PARA_SPEND_GOLD:
      {
        Cmd::Session::t_SpendGold_SceneSession *rev = (Cmd::Session::t_SpendGold_SceneSession *)cmd;
        UserSession* pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
        if (!pUser) return false;

        CSept * sept = CSeptM::getMe().getSeptByID(pUser->septid);
        if (sept)
        {
          DWORD num = ((DWORD)(rev->gold/100))*2;
          sept->sendGoldToMember(rev->userID,num);
        }
        /*
        CSept * sept = CSeptM::getMe().getSeptByID(pUser->septid);
        if (sept)
        {
          DWORD m = sept->dwSpendGold/10;
          sept->dwSpendGold += rev->gold;
          DWORD n = sept->dwSpendGold/10;
          if (n-m)
          {
            sept->dwRepute += n-m;
            sept->sendSeptNotify("�����Ա %s ���ѽ��,������������� %u ��",pUser->name,n-m);
          }
        }
        */
        return true;
      }
      break;
    case Cmd::Session::OVERMAN_TICKET_ADD:
      {
        Cmd::Session::t_OvermanTicketAdd *command = (Cmd::Session::t_OvermanTicketAdd*)cmd;
        UserSession* pUser = UserSessionManager::getInstance()->getUserByID(command->id);
        if (pUser)
        {
          Cmd::Session::t_OvermanTicketAdd add;
          add.id=command->id;
          add.ticket=command->ticket;
          strncpy(add.name,command->name,MAX_NAMESIZE);
          pUser->scene->sendCmd(&add,sizeof(Cmd::Session::t_OvermanTicketAdd));
        }
        return true;
      }
      break;

    case Cmd::Session::QUEST_BULLETIN_USERCMD_PARA:
      {
        Cmd::Session::t_QuestBulletinUserCmd* command = (Cmd::Session::t_QuestBulletinUserCmd*)cmd;
        if (command->kind == 1) {
          CUnionM::getMe().sendUnionNotify(command->id,command->content);
          return true;
        }

        if (command->kind == 2) {
          CSeptM::getMe().sendSeptNotify(command->id,command->content);
          return true;
        }
      }
      break;

    case Cmd::Session::QUEST_CHANGE_AP:
      {
        Cmd::Session::t_ChangeAP* command = (Cmd::Session::t_ChangeAP*) cmd;
        CUnion* u = CUnionM::getMe().getUnionByID(command->id);
        if (u) {
          u->changeActionPoint(command->point);
        }
      }
      break;      

    case Cmd::Session::PARA_CHANGE_USER_DATA:
      {
        Cmd::Session::t_changeUserData_SceneSession* rev = 
          (Cmd::Session::t_changeUserData_SceneSession*)cmd;

        UserSession* pUser = UserSessionManager::getInstance()->getUserByID(rev->dwUserID);
        if (pUser)
        {
          pUser->level = rev->wdLevel;
          pUser->dwExploit = rev->dwExploit;
        }

        return true;
      }
      break;

	case Cmd::Session::GLOBAL_MESSAGE_PARA:
	{
		Cmd::Session::t_GlobalMessage_SceneSession *rev = (Cmd::Session::t_GlobalMessage_SceneSession*)cmd;
		UserSession* pUser = UserSessionManager::getInstance()->getUserByID(rev->dwUserID);
		if(pUser)
		{
			std::string msg(pUser->name);
			msg = msg + ":" + rev->msg;
		    //char *msg = new char[strlen("")]
			SessionChannel::sendAllInfo(Cmd::INFO_TYPE_SCROLL,msg.c_str());
		}
		return true;
	}
	break;

    case Cmd::Session::PARA_AUCTION_CMD:
      {
        Cmd::Session::t_AuctionCmd * rev = (Cmd::Session::t_AuctionCmd *)cmd;
        return AuctionService::getMe().doAuctionCmd(rev,cmdLen);
      }
      break;
    case Cmd::Session::PARA_CARTOON_CMD:
      {
        Cmd::Session::t_CartoonCmd * rev = (Cmd::Session::t_CartoonCmd *)cmd;
        return CartoonPetService::getMe().doCartoonCmd(rev,cmdLen);
      }
      break;
    case Cmd::Session::PARA_SERVER_NOTIFY:
      {
        Cmd::Session::t_serverNotify_SceneSession* rev = 
          (Cmd::Session::t_serverNotify_SceneSession*)cmd;

        SessionChannel::sendAllInfo(rev->infoType,rev->info);
        return true;
      }
      break;
    case Cmd::Session::PARA_COUNTRY_NOTIFY:
      {
        Cmd::Session::t_countryNotify_SceneSession* rev = 
          (Cmd::Session::t_countryNotify_SceneSession*)cmd;

        SessionChannel::sendCountryInfo(rev->infoType,
            rev->dwCountryID,"%s",rev->info);
        return true;
      }
      break;
    case Cmd::Session::PARA_CHANGE_COUNTRY:
      {
        Cmd::Session::t_changeCountry_SceneSession* rev = 
          (Cmd::Session::t_changeCountry_SceneSession*)cmd;
        this->change_country(rev);

        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_FORWARD_USER:
      {
        Cmd::Session::t_forwardUser_SceneSession * rev = (Cmd::Session::t_forwardUser_SceneSession *)cmd;

        UserSession* pUser = 0;
        if (rev->id)
          pUser = UserSessionManager::getInstance()->getUserByID(rev->id);
        if (!pUser && rev->tempid)
          pUser = UserSessionManager::getInstance()->getUserByTempID(rev->id);
        if (!pUser && !strncmp("",rev->name,MAX_NAMESIZE))
          pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->name);

        if (pUser)
          pUser->sendCmdToMe(rev->cmd,rev->cmd_len);

        return true;
      }
      break;
    case Cmd::Session::PARA_RETURN_OBJECT:
      {
        Cmd::Session::t_returnObject_SceneSession* rev = (Cmd::Session::t_returnObject_SceneSession*)cmd;
        UserSession* pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->to_name);
        Cmd::stReturnQuestionObject send;

        if (pUser)
        {
          strncpy(send.name,rev->from_name,MAX_NAMESIZE);
          bcopy(&send.object,&rev->object,sizeof(t_Object),sizeof(rev->object));
          pUser->sendCmdToMe(&send,sizeof(Cmd::stReturnQuestionObject));
        }

        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_REGSCENE:
      {
		  Cmd::Session::t_regScene_SceneSession *reg=(Cmd::Session::t_regScene_SceneSession *)cmd;
		  SceneSession *scene=new SceneSession(this);
		  Cmd::Session::t_regScene_ret_SceneSession ret;
		  ret.dwTempID=reg->dwTempID;
		  if(SceneSessionManager::getInstance()->getSceneByTempID(reg->dwTempID))
		  {
			  printf("�����ظ�ע����Ϣ,session��������(%s-%d-%d)\n", reg->byName, reg->dwID, reg->dwTempID);
			  return true;
		  }

		  if (scene->reg(reg) && SceneSessionManager::getInstance()->addScene(scene))
		  {
			  ret.byValue=Cmd::Session::REGSCENE_RET_REGOK;
			  CCountryM::getMe().refreshTax();
			  CCountryM::getMe().refreshTech(this,reg->dwCountryID);
			  if (KING_CITY_ID==(reg->dwID&0x0000ffff))
				  CCountryM::getMe().refreshGeneral(reg->dwCountryID);
			  CAllyM::getMe().refreshAlly(this);

			  Zebra::logger->info("ע���ͼ%u(%s %s) �ɹ�",reg->dwID,reg->byName,reg->fileName);
			  CCityM::getMe().refreshUnion(reg->dwCountryID,reg->dwID & 0x0FFF);

			  SceneSession * sc = SceneSessionManager::getInstance()->getSceneByID(reg->dwID);
			  if(!sc)
			  {
				  fprintf(stderr,"bad\n");
			  }

			  return true;
		  }
		  else
		  {
			  ret.byValue=Cmd::Session::REGSCENE_RET_REGERR;
			  Zebra::logger->error("ע���ͼ%u(%s %s)ʧ��",reg->dwID,reg->byName,reg->fileName);
		  }
		  sendCmd(&ret,sizeof(ret));
		  return true;
	  }
	  break;
    case Cmd::Session::PARA_SCENE_UNREGUSER:
      {
        Cmd::Session::t_unregUser_SceneSession *unreg=(Cmd::Session::t_unregUser_SceneSession *)cmd;
        UserSession *pUser=UserSessionManager::getInstance()->getUserByID(unreg->dwUserID);
        if (pUser)
        {
          CSortM::getMe().offlineCount(pUser);
          CUnionM::getMe().userOffline(pUser); // ���ڴ������Ա����
          CSchoolM::getMe().userOffline(pUser);
          CSeptM::getMe().userOffline(pUser);
          CQuizM::getMe().userOffline(pUser);
          CGemM::getMe().userOffline(pUser);

          UserSessionManager::getInstance()->removeUser(pUser);
          if (unreg->retcode==Cmd::Session::UNREGUSER_RET_ERROR)
            Zebra::logger->info("ScenesServer����ע��%s(%ld)",pUser->name,pUser->id);
          else
            Zebra::logger->info("���������û�%s(%ld)ע��",pUser->name,pUser->id);
          SAFE_DELETE(pUser);
        }
        return true;
      }
    case Cmd::Session::PARA_SCENE_REGUSERSUCCESS:
      {
        Cmd::Session::t_regUserSuccess_SceneSession *regsuccess=(Cmd::Session::t_regUserSuccess_SceneSession *)cmd;
        UserSession *pUser=UserSessionManager::getInstance()->getUserByID(regsuccess->dwUserID);
        if (pUser)
        {
          pUser->dwExploit = regsuccess->dwExploit;
          pUser->dwUseJob = regsuccess->dwUseJob;
          pUser->qwExp = regsuccess->qwExp;
          pUser->setRelationData(regsuccess);
          pUser->relationManager.init();    // ��ʼ������û��ĺ����б�

          pUser->updateConsort();      // ������ż״̬������

          if (pUser->level >= 50 && SessionService::getInstance().uncheckCountryProcess)
          {
            typedef std::map<DWORD,BYTE>::value_type ValueType;
            std::pair<std::map<DWORD,BYTE>::iterator,bool> retval;
            retval = SessionService::userMap.insert(ValueType(pUser->id,pUser->level));
            if (retval.second) SessionService::getInstance().countryLevel[pUser->country]+=pUser->level;
          }
          CNpcDareM::getMe().sendUserData(pUser);
          CSortM::getMe().onlineCount(pUser);
 //         CUnionM::getMe().userOnline(pUser); // ���û�RecordServer��������Ժ���֪ͨ����
 //         CSchoolM::getMe().userOnline(pUser);
          CSeptM::getMe().userOnline(pUser);
 //         CQuizM::getMe().userOnline(pUser);
          CCountryM::getMe().userOnline(pUser);
          CArmyM::getMe().userOnline(pUser);
//        CGemM::getMe().userOnline(pUser);
//        CAllyM::getMe().userOnline(pUser);
//        CDareM::getMe().userOnline(pUser);
		  

		  //sky ��ѯ��ʱ�б����Ƿ��и��û�
		  g_MoveSceneMemberMapLock.lock();
		  std::map<DWORD,DWORD>::iterator iter;
		  iter = MoveSceneMemberMap.find(regsuccess->dwUserID);

		  if(iter != MoveSceneMemberMap.end())
		  {
			  DWORD TeamID = iter->second;
			  Team * pteam = GlobalTeamIndex::getInstance()->GetpTeam(TeamID);
			  if(pteam)
			  {
				  Cmd::Session::t_Team_AddMe rev;
				  rev.LeaberID = pteam->GetLeader();
				  rev.TeamThisID = TeamID;
				  rev.MeID = regsuccess->dwUserID;

				  //sky �����û���Ӷ����Ա
				  pUser->scene->sendCmd( &rev, sizeof(Cmd::Session::t_Team_AddMe) );
			  }
			  //sky �������ʱ�б���ĸ��û���Ϣ
			  MoveSceneMemberMap.erase(iter);
		  }
		  g_MoveSceneMemberMapLock.unlock();

          WORD degree = CSortM::getMe().getLevelDegree(pUser);
          Cmd::stLevelDegreeDataUserCmd send;
          send.degree = degree;
          pUser->sendCmdToMe(&send,sizeof(send));

          COfflineMessage::getOfflineMessage(pUser);
        }
        return true;
      }
      break;
    case Cmd::Session::PARA_TAXADD_COUNTRY:
      {
        Cmd::Session::t_taxAddCountry_SceneSession *rev=(Cmd::Session::t_taxAddCountry_SceneSession *)cmd;
        CCountry *pCountry = CCountryM::getMe().find(rev->dwCountryID);
        if (pCountry)
        {
          pCountry->addTaxMoney(rev->qwTaxMoney);
        }
        return true;
      }
      break;
    case Cmd::Session::PARA_FRIENDDEGREE_REQUEST:
      {
        Cmd::Session::t_RequestFriendDegree_SceneSession *rev=(Cmd::Session::t_RequestFriendDegree_SceneSession *)cmd;

        for (int i=0; i< rev->size; i++)
        {
          UserSession *pUser=UserSessionManager::getInstance()->getUserSessionByName(rev->namelist[i].name);
          if (pUser)
          {
            pUser->sendFriendDegree(rev);
          }
        }
        return true;
      }
      break;		
    case Cmd::Session::PARA_FRIENDDEGREE_COUNT:
      {
        Cmd::Session::t_CountFriendDegree_SceneSession *rev=(Cmd::Session::t_CountFriendDegree_SceneSession *)cmd;
        UserSession *pMainUser=UserSessionManager::getInstance()->getUserSessionByName(rev->name);
        if (pMainUser)
        {
          pMainUser->setFriendDegree(rev);
        }
        CSchoolMember *member = CSchoolM::getMe().getMember(rev->name);
        if (member) member->setFriendDegree(rev);
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_LEVELUPNOTIFY:
      {
        Cmd::Session::t_levelupNotify_SceneSession *rev=(Cmd::Session::t_levelupNotify_SceneSession *)cmd;
        UserSession *pUser=UserSessionManager::getInstance()->getUserByID(rev->dwUserID);
        if (pUser)
        {
          pUser->level = rev->level;
          pUser->qwExp = rev->qwExp;

          CSortM::getMe().upLevel(pUser); //��ɫ����ϵͳˢ��
          WORD degree = CSortM::getMe().getLevelDegree(pUser);
          Cmd::stLevelDegreeDataUserCmd send;
          send.degree = degree;
          pUser->sendCmdToMe(&send,sizeof(send));

          CSchoolM::getMe().setUserLevel(pUser->name,rev->level);
          CartoonPetService::getMe().userLevelUp(pUser->id,rev->level);
        }
        return true;
      }
      break;
      //������Ϣ
	case Cmd::Session::PARA_USER_TEAM_ADDMEMBER:
		{
			Cmd::Session::t_Team_AddMember *rev = (Cmd::Session::t_Team_AddMember *)cmd;
			UserSession * pUser = (UserSession *)UserSessionManager::getInstance()->getUserByName(rev->AddMember.name);
			if(pUser)
				GlobalTeamIndex::getInstance()->addMember(rev->dwTeam_tempid, rev->dwLeaderID,pUser->id);
			return true;
		}
		break;
	case Cmd::Session::PARA_USER_TEAM_DELMEMBER:
		{
			Cmd::Session::t_Team_DelMember *rev = (Cmd::Session::t_Team_DelMember *)cmd;
			GlobalTeamIndex::getInstance()->delMember(rev->dwTeam_tempid,rev->MemberNeam);
			return true;
		}
		break;
	case Cmd::Session::PARA_USE_TEAM_ADDMOVESCENEMAMBER:	//sky�����ߵĶ����Ա�ŵ���ʱ�б���
		{
			Cmd::Session::t_Team_AddMoveSceneMember * rev = (Cmd::Session::t_Team_AddMoveSceneMember*)cmd;
			g_MoveSceneMemberMapLock.lock();
			MoveSceneMemberMap[rev->MemberID] = rev->TeamThisID;
			g_MoveSceneMemberMapLock.unlock();

			GlobalTeamIndex::getInstance()->UpDataMapID(rev->MemberID, rev->TeamThisID);
		}
		break;
	case Cmd::Session::PARA_USER_TEAM_CHANGE_LEADER:		//sky���������ӳ���Ϣ
		{
			Cmd::Session::t_Team_ChangeLeader *rev = (Cmd::Session::t_Team_ChangeLeader *)cmd;
			if(rev->NewLeaderName[0] == 0)
				GlobalTeamIndex::getInstance()->ChangeLeader(rev->dwTeam_tempid);
			else
				GlobalTeamIndex::getInstance()->ChangeLeader(rev->dwTeam_tempid, rev->NewLeaderName);

			return true;
		}
		break;
	case Cmd::Session::PARA_USE_TEAM_DELTEAM:				//sky ɾ������
		{
			Cmd::Session::t_Team_DelTeam * rev = (Cmd::Session::t_Team_DelTeam *)cmd;
			GlobalTeamIndex::getInstance()->DelTeam(rev->TeamThisID);
		}
		break;
	case Cmd::Session::PARA_USER_TEAM_REQUEST_TEAM:			//sky ���������Ϣ[�糡�������]
		{
			Cmd::Session::t_Team_RequestTeam * rev = (Cmd::Session::t_Team_RequestTeam *)cmd;
			UserSession *pUser=(UserSession*)(UserSessionManager::getInstance()->getUserByName(rev->byAnswerUserName));
			
			//sky �ҵ��ش������ڵĳ���֪ͨ�䴦��������Ϣ
			if(pUser)
				pUser->scene->sendCmd(rev, sizeof(Cmd::Session::t_Team_RequestTeam));
			else
			{
				UserSession *RpUser = (UserSession *)(UserSessionManager::getInstance()->getUserByName(rev->byRequestUserName));

				if(RpUser)
					RpUser->sendSysChat(Cmd::INFO_TYPE_SYS,"�������ʱ�޷��ҵ����:[%s],����ҿ����Ѿ�����!", rev->byAnswerUserName);
			}
		}
		break;
	case Cmd::Session::PARA_USE_TEAM_ANSWER_TEAM:			//sky �ش������Ϣ[�糡�������]
		{
			Cmd::Session::t_Team_AnswerTeam * rev = (Cmd::Session::t_Team_AnswerTeam *)cmd;
			UserSession * pUser = (UserSession*)(UserSessionManager::getInstance()->getUserByName(rev->byRequestUserName));

			//sky �ҵ����������ڵĳ���֪ͨ�䴦��ش���Ϣ
			if(pUser)
				pUser->scene->sendCmd(rev, sizeof(Cmd::Session::t_Team_AnswerTeam));
			else
			{
				UserSession *RpUser = (UserSession*)(UserSessionManager::getInstance()->getUserByName(rev->byRequestUserName));

				if(RpUser)
					RpUser->sendSysChat(Cmd::INFO_TYPE_SYS,"�ش����ʱ���޷��ҵ����:[%s],����ҿ����Ѿ�����!", rev->byAnswerUserName);
			}
		}
		break;
      //�������ʱ����
    case Cmd::Session::PARA_USER_ARCHIVE_REQ:
      {
        //TODO
        Cmd::Session::t_ReqUser_SceneArchive *rev=(Cmd::Session::t_ReqUser_SceneArchive *)cmd;
        SceneSession *scene= SceneSessionManager::getInstance()->getSceneByTempID(rev->dwMapTempID);
        if (!scene)
        {
          return true;
        }
        char buf[sizeof(Cmd::Session::t_ReadUser_SceneArchive) + MAX_TEMPARCHIVE_SIZE];
        if (buf)
        {
          Cmd::Session::t_ReadUser_SceneArchive *ret = (Cmd::Session::t_ReadUser_SceneArchive *)buf;
          constructInPlace(ret);
          ret->id = rev->id;
          ret->dwMapTempID = rev->dwMapTempID;
          ret->dwSize = MAX_TEMPARCHIVE_SIZE;
          if (GlobalTempArchiveIndex::getInstance()->readTempArchive(ret->id,ret->data,ret->dwSize))
          {
            scene->sendCmd(ret,sizeof(Cmd::Session::t_ReadUser_SceneArchive) + ret->dwSize);
            Zebra::logger->info("������ʱ�������ݰ�%u",sizeof(Cmd::Session::t_ReadUser_SceneArchive) + ret->dwSize);
          }
        }
        return true;
      }
      break;
      //����д��ʱ����
    case Cmd::Session::PARA_USER_ARCHIVE_WRITE:
      {
        //TODO
        Cmd::Session::t_WriteUser_SceneArchive *rev=(Cmd::Session::t_WriteUser_SceneArchive *)cmd;
        //TempArchiveMember *temp = (TempArchiveMember *)(rev->data + rev->dwSize);
        //Zebra::logger->debug("temp->dwSize=%u",temp->size);
        //Zebra::logger->debug("rev->dwSize=%u",rev->dwSize);
        //Zebra::logger->debug("�յ���ʱ�浵���ݰ�%u",cmdLen);
        //UserSession *pUser=UserSessionManager::getInstance()->getUserByID(rev->id);
        //if (pUser)
        //{
        GlobalTempArchiveIndex::getInstance()->writeTempArchive(rev->id,rev->data,rev->dwSize);
        //}
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_CHANEG_SCENE:
      {
        Cmd::Session::t_changeScene_SceneSession *rev=(Cmd::Session::t_changeScene_SceneSession *)cmd;
        SceneSession* scene = NULL;
        if ((char)rev->map_file[0]) {
          scene = SceneSessionManager::getInstance()->getSceneByFile((char*) rev->map_file);
        }else if (rev->map_id){
          scene = SceneSessionManager::getInstance()->getSceneByID(rev->map_id);
          //Zebra::logger->debug("��ͼid=%u",rev->map_id);
        }else{
          scene = SceneSessionManager::getInstance()->getSceneByName((char*) rev->map_name);
          //Zebra::logger->debug("��ͼ����%s",rev->map_name);
        }
        if (!scene) return true;

		 UserSession *pUser = UserSessionManager::getInstance()->getUserByID(rev->id);

        if (scene->level>0)
        {
          if (pUser)
          {
            if (pUser->level < scene->level)
            {
              pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"%s���Եȼ�����%d������ҿ��ţ�",scene->name,scene->level);
              Zebra::logger->info("[GOMAP]���%s�ȼ�����������ͼ[%s]ʧ��!",pUser->name,scene->name);
              return true;
            }
          }
          //else return true;
        }


        //Zebra::logger->debug("������Ϣ%s,%u,%u",scene->name,scene->id,scene->tempid);
        //Zebra::logger->debug("������Ϣ%s,%s,%u",rev->map_file,rev->map_name,rev->map_id);
        Cmd::Session::t_changeScene_SceneSession ret;
        ret.id = rev->id;
        ret.x = rev->x;
        ret.y = rev->y;
        ret.map_id = rev->map_id;

        if (scene) {
          ret.temp_id = scene->tempid;
          strncpy((char *)ret.map_file,scene->file.c_str(),MAX_NAMESIZE);
          strncpy((char *)ret.map_name,scene->name,MAX_NAMESIZE);
        }
		else 
		{
          ret.temp_id = (DWORD)-1;
        }

        sendCmd(&ret,sizeof(ret));
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_GM_COMMAND:
      {
        Cmd::Session::t_gmCommand_SceneSession * rev = (Cmd::Session::t_gmCommand_SceneSession *)cmd;

        switch (rev->gm_cmd)
        {
            case Cmd::Session::GM_COMMAND_LOAD_GIFT:
                return Gift::getMe().init();
                break;
            case Cmd::Session::GM_COMMAND_LOAD_PROCESS:
                return SessionTaskManager::getInstance().broadcastScene(cmd,cmdLen);
                break;
            case Cmd::Session::GM_COMMAND_NEWZONE:
                return SessionTaskManager::getInstance().broadcastScene(cmd,cmdLen);
                break;
            case Cmd::Session::GM_COMMAND_REFRESH_GENERAL:
                CCountryM::getMe().refreshGeneral(0);
                return true;
            default:
                break;
        }

        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName((char *)rev->dst_name);
        if (0==pUser)
        {
          Cmd::Session::t_gmCommand_SceneSession ret;
          ret.gm_cmd = rev->gm_cmd;
          strncpy((char *)ret.dst_name,(char *)rev->src_name,MAX_NAMESIZE);
          strncpy((char *)ret.src_name,(char *)rev->dst_name,MAX_NAMESIZE);
          ret.err_code = Cmd::Session::GM_COMMAND_ERR_NOUSER;

          ret.cmd_state = Cmd::Session::GM_COMMAND_STATE_RET;
          sendCmd(&ret,sizeof(ret));
          return true;
        }

        pUser->scene->sendCmd(rev,sizeof(Cmd::Session::t_gmCommand_SceneSession));
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_CREATE_SCHOOL:
      {
        Cmd::Session::t_createSchool_SceneSession * rev = (Cmd::Session::t_createSchool_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName((char *)rev->masterName);
        if (!pUser) return false;
        if (CSchoolM::getMe().createNewSchool(pUser->name,(char *)rev->schoolName))
        {
          Cmd::Session::t_SchoolCreateSuccess_SceneSession send;
          send.dwID = pUser->id;
          send.dwSchoolID = pUser->schoolid;
          strncpy(send.schoolName,rev->schoolName,MAX_NAMESIZE);
          pUser->scene->sendCmd(&send,sizeof(send));
        }
        else
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�ź���֪ͨ�����������ѱ�ռ�ã��뻻������������!");
        }
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_PRIVATE_CHAT:
      {
        Cmd::Session::t_privateChat_SceneSession * rev = (Cmd::Session::t_privateChat_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName((char *)rev->dst_name);
        if (0==pUser)
        {
          if (strstr((char *)rev->dst_name,"GM")
              || strstr((char *)rev->dst_name,"gm"))  return true;//����GM��˽�Ĳ�����

          Cmd::Session::t_privateChat_SceneSession ret;
          ret.err_code = Cmd::Session::PRIVATE_CHAT_ERR_NOUSER;
          strncpy((char *)ret.src_name,(char *)rev->dst_name,MAX_NAMESIZE);
          strncpy((char *)ret.dst_name,(char *)rev->src_name,MAX_NAMESIZE);
          //bcopy(rev->chat_cmd,ret.chat_cmd,rev->cmd_size);
          //sendCmd(&ret,sizeof(Cmd::Session::t_privateChat_SceneSession)+rev->cmd_size);
          sendCmd(&ret,sizeof(ret));
          return true;
        }

        pUser->scene->sendCmd(rev,cmdLen);
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_SYS_SETTING:
      {
        Cmd::Session::t_sysSetting_SceneSession * rev = (Cmd::Session::t_sysSetting_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName((char *)rev->name);
        if (pUser)
        {
          bcopy(rev->sysSetting,pUser->sysSetting,sizeof(pUser->sysSetting),sizeof(pUser->sysSetting));
         // pUser->face = rev->face;
          return true;
        }
      }
      break;
    case Cmd::Session::PARA_SCENE_CITY_RUSH:
      {
        //Zebra::logger->debug("�յ�rush����");
        Cmd::Session::t_cityRush_SceneSession * rev = (Cmd::Session::t_cityRush_SceneSession *)cmd;
        char content[MAX_CHATINFO];
        sprintf(content,"BOSS %s ��ɱ����,�������õ���ħ��ľ��꣬���� %d ���Ӻ�� %s ���� %s",rev->bossName,rev->delay/60,rev->mapName,rev->rushName);
        SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,rev->countryID,content);
        /*
           broadcastRushToEveryUser b;
           if (b.init(content))
           UserSessionManager::getInstance()->execEveryUser(b);
         */
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_CITY_RUSH_CUST:
      {
        Cmd::Session::t_cityRushCust_SceneSession * rev = (Cmd::Session::t_cityRushCust_SceneSession *)cmd;
    if (strncmp("  ",rev->text,128))
        SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,rev->countryID,rev->text);
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_REMOVE_SCENE:
      {
        Cmd::Session::t_removeScene_SceneSession *rev = (Cmd::Session::t_removeScene_SceneSession*)cmd;
        SceneSession *scene= SceneSessionManager::getInstance()->getSceneByID(rev->map_id);
        if (scene)
        {
          SceneSessionManager::getInstance()->removeScene(scene);
          Zebra::logger->info("ж�ص�ͼ%u(%s) �ɹ�",scene->id,scene->name);
          SAFE_DELETE(scene);
        }
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_REQ_ADD_SCENE:
      {
        Cmd::Session::t_reqAddScene_SceneSession *rev = (Cmd::Session::t_reqAddScene_SceneSession*)cmd;
        Zebra::logger->info("ת�����ص�ͼ��Ϣ(%u,%u,%u)",rev->dwServerID,rev->dwCountryID,rev->dwMapID);
        SessionTaskManager::getInstance().broadcastByID(rev->dwServerID,
            rev,sizeof(Cmd::Session::t_reqAddScene_SceneSession));
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_UNLOAD_SCENE:
      {
        Cmd::Session::t_unloadScene_SceneSession *rev = (Cmd::Session::t_unloadScene_SceneSession*)cmd;
        SceneSession *scene= SceneSessionManager::getInstance()->getSceneByID(rev->map_id);
        if (scene)
        {
          //TODO
          //���ò���ע���־
          scene->setRunningState(SCENE_RUNNINGSTATE_UNLOAD);
          scene->sendCmd(rev,sizeof(Cmd::Session::t_unloadScene_SceneSession));
          Zebra::logger->info("��ͼ%s��ж�ص�ͼֹͣע��",scene->name);
          /*
             SceneSessionManager::getInstance()->removeScene(scene);
             struct UnloadSceneSessionExec :public execEntry<UserSession>
             {
             SceneSession *scene;
             std::vector<DWORD> del_vec;
             UnloadSceneSessionExec(SceneSession *s):scene(s)
             {
             }
             bool exec(UserSession *u)
             {
             if (u->scene->id == scene->id)
             {
             del_vec.push_back(u->id);
             }
             return true;
             }
             };
             UnloadSceneSessionExec exec(scene);
             UserSessionManager::getInstance()->execEveryUser(exec);
             for(std::vector<DWORD>::iterator iter = exec.del_vec.begin() ; iter != exec.del_vec.end() ; iter ++)
             {
             UserSession *pUser=UserSessionManager::getInstance()->getUserByID(*iter);
             if (pUser)
             {
             Zebra::logger->info("�û�%s(%ld)��ж�ص�ͼע��",pUser->name,pUser->id);
             UserSessionManager::getInstance() ->removeUser(pUser);
             SAFE_DELETE(pUser);
             }
             }
             Zebra::logger->debug("ж�ص�ͼ%u(%s) �ɹ�",scene->id,scene->name);
             SAFE_DELETE(scene);
          // */
        }
        return true;
      }
      break;
    case Cmd::Session::PARA_SCENE_GUARD_FAIL:
      {
        Cmd::Session::t_guardFail_SceneSession * rev = (Cmd::Session::t_guardFail_SceneSession *)cmd;
        UserSession *pUser=UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (pUser)
          pUser->scene->sendCmd(rev,cmdLen);
        return true;
      }
      break;
      /*
    case Cmd::Session::PARA_SCENE_LOAD_PROCESS:
      {
        SessionTaskManager::getInstance().broadcastScene(cmd,cmdLen);
        return true;
      }
      break;
      */
    default:
      break;
  }

  Zebra::logger->error("SessionTask::msgParse_Scene(%u,%u,%u)",cmd->cmd,cmd->para,cmdLen);
  return false;
}

bool SessionTask::msgParse_Gate(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
  switch(cmd->para)
  {
    case Cmd::Session::PARA_GATE_REGUSER:
      {
        Cmd::Session::t_regUser_GateSession *reg=(Cmd::Session::t_regUser_GateSession *)cmd;
        SceneSession *scene= SceneSessionManager::getInstance()->getSceneByName((char *)reg->byMapName);
		BYTE regtype = 0;
		if (!scene||(reg->wdLevel<scene->level))
		{   
			//sky �ȿ�����û���ϵ�ͼ��λ��
			char map[MAX_NAMESIZE+1];
			DWORD map_x;
			DWORD map_y;
			bzero(map,sizeof(map));
			sscanf(reg->OldMap, "%[^-]%d-%d", map, &map_x, &map_y);

			scene = SceneSessionManager::getInstance()->getSceneByName(map);
			
			if(scene)
				regtype	= 1;
			else
			{
				//sky ����2�ַ�ʽ��û�ҵ���ͼ�Ļ��ʹ��͵�Ĭ�ϵ�ͼ
				bzero(map,sizeof(map));
				bcopy(reg->byMapName,map,6,sizeof(map));
				strncpy(map, "�ι�����Դ��", MAX_NAMESIZE);
				scene=SceneSessionManager::getInstance()->getSceneByName(map);
			}
		}

        if (scene)
        {
          if (scene->getRunningState() == SCENE_RUNNINGSTATE_NORMAL)
          {
            UserSession *pUser=UserSessionManager::getInstance()->getUserByID(reg->dwID);
            if (!pUser)
            {
              pUser=new UserSession(this);
              if (pUser && pUser->reg(reg))
              {
                //Zebra::logger->debug("�����û�Session�ɹ�");
                pUser->scene=scene;

                if (UserSessionManager::getInstance()->addUser(pUser))
                {
                  //����������
                  Cmd::Session::t_regUser_SceneSession reginscene;
                  reginscene.accid = reg->accid;
                  reginscene.dwID = reg->dwID;
                  reginscene.dwTempID = reg->dwTempID;
                  reginscene.dwMapID = scene->id;
                  bcopy(reg->byName,reginscene.byName,MAX_NAMESIZE+1,sizeof(reginscene.byName));
                  bcopy(scene->name,reginscene.byMapName,MAX_NAMESIZE+1,sizeof(reginscene.byMapName));
                  reginscene.dwGatewayServerID = pUser->getTask()->getID();
				  reginscene.RegMapType = regtype;
                  scene->sendCmd(&reginscene,sizeof(reginscene));

                  Zebra::logger->info("�û�%s(%ld)ע��ɹ�",pUser->name,pUser->id);

                  CartoonPetService::getMe().userOnline(pUser);
                  return true;
                }
                else
                {
                  UserSession *pUser=UserSessionManager::getInstance()->getUserByID(reg->dwID);
                  if (pUser)
                  {
                    Zebra::logger->debug("��ɫid�ظ�(id=%u,name=%s,tempid=%u" 
                        ,pUser->id,pUser->name,pUser->tempid);
                  }
                  pUser=UserSessionManager::getInstance()->getUserByTempID(reg->dwTempID);
                  if (pUser)
                  {
                    Zebra::logger->debug("��ɫtempid�ظ�(id=%u,name=%s,tempid=%u" 
                        ,pUser->id,pUser->name,pUser->tempid);
                  }
                  Zebra::logger->error("��ӽ�ɫʧ�ܣ������ǽ�ɫ�ظ���½");
                }
                SAFE_DELETE(pUser);
              }
              else
                Zebra::logger->fatal("ע���û�ʱ������ڴ�ʧ��(%u,%u,%x)",reg->accid,reg->dwID,reg->byName);
            }
            else
            {
              CSortM::getMe().offlineCount(pUser);
              CUnionM::getMe().userOffline(pUser); // ���ڴ������Ա����
              CSchoolM::getMe().userOffline(pUser);
              CSeptM::getMe().userOffline(pUser);
              CQuizM::getMe().userOffline(pUser);
              CGemM::getMe().userOffline(pUser);
              UserSessionManager::getInstance()->removeUser(pUser);

              //֪ͨ���ش���ע��
              Cmd::Session::t_unregUser_GateSession ret;
              ret.dwUserID=reg->dwID;
              ret.dwSceneTempID=scene->tempid;
              ret.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
              sendCmd(&ret,sizeof(ret));

              //֪ͨ��������ע��
              Cmd::Session::t_unregUser_SceneSession send;
              send.dwUserID=reg->dwID;
              send.dwSceneTempID=scene->tempid;
              send.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
              scene->sendCmd(&send,sizeof(send));
              Zebra::logger->info("�����ظ��û�ע��%s(%ld)ע��",pUser->name,pUser->id);
              SAFE_DELETE(pUser);
              return true;
            }
          }
          else
          {
            Zebra::logger->info("����%s���ڲ�����ע���û�",(char *)reg->byMapName);
          }
        }
        else
          Zebra::logger->error("δ�ҵ���ɫ���ڵ�ͼ %s",(char *)reg->byMapName);
        //֪ͨ����ע��ʧ��
        Zebra::logger->error("�û�(%lu,%lu,%s,%lu)ע��ʧ��",reg->accid,reg->dwID,reg->byName,reg->dwTempID);
        Cmd::Session::t_unregUser_GateSession ret;
        ret.dwUserID=reg->dwID;
        if (scene)
          ret.dwSceneTempID=scene->tempid;
        else
          ret.dwSceneTempID=0;
        ret.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
        sendCmd(&ret,sizeof(ret));
        return true;
      }
      break;
      //���������������
    case Cmd::Session::REQUEST_GATE_COUNTRY_ORDER:
      {
        char Buf[200];
        bzero(Buf,sizeof(Buf));
        Cmd::Session::t_order_Country_GateSession *ret_gate = 
          (Cmd::Session::t_order_Country_GateSession*)Buf;
        constructInPlace(ret_gate);
        ret_gate->order.size = UserSession::country_map.size();
        for(std::map<DWORD,DWORD>::iterator iter = 
            UserSession::country_map.begin() ; iter != UserSession::country_map.end() ;iter ++)
        {
          DWORD temp = iter->second;
          DWORD cn = iter->first;
          for(int i=ret_gate->order.size -1 ; i>=0; i--)
          {
            if (ret_gate->order.order[i].count <= temp)
            {
              DWORD temp_1 = ret_gate->order.order[i].count;
              DWORD cn_1 = ret_gate->order.order[i].country;
              ret_gate->order.order[i].count = temp;
              ret_gate->order.order[i].country = cn; 
              temp = temp_1;
              cn = cn_1;
            }
          }
        }
        for(int i = 0 ; i < (int)ret_gate->order.size ; i ++)
        {
          Zebra::logger->debug("����:%d,��������:%d",ret_gate->order.order[i].country,ret_gate->order.order[i].count);
        }
        sendCmd(ret_gate,sizeof(Cmd::Session::t_order_Country_GateSession) 
            + sizeof(ret_gate->order.order[0]) * ret_gate->order.size); 
        return true;
      }
      break;
    case Cmd::Session::PARA_GATE_CHANGE_SCENE_USER:
      {
        Cmd::Session::t_changeUser_GateSession *reg=(Cmd::Session::t_changeUser_GateSession *)cmd;
        SceneSession *scene= SceneSessionManager::getInstance()->getSceneByFile((char *)reg->byMapFileName);

        if (scene)  {
          UserSession *pUser=UserSessionManager::getInstance()->getUserByID(reg->dwID);
          if (pUser)   {
            //Zebra::logger->debug("�����û�Session�ɹ�");
            pUser->scene=scene;

            //����������
            Cmd::Session::t_regUser_SceneSession reginscene;
            reginscene.accid=reg->accid;
            reginscene.dwID=reg->dwID;
            reginscene.dwTempID=reg->dwTempID;
            reginscene.dwMapID=scene->id;
            bcopy(reg->byName,reginscene.byName,MAX_NAMESIZE+1,sizeof(reginscene.byName));
            bcopy(scene->name,reginscene.byMapName,MAX_NAMESIZE+1,sizeof(reginscene.byMapName));
            reginscene.dwGatewayServerID=pUser->getTask()->getID();
            scene->sendCmd(&reginscene,sizeof(reginscene));

            Zebra::logger->info("�û�%s(%ld)�л������ɹ�",pUser->name,pUser->id);

			//sky ���������ж����,�ȰѶ����MAPID����������
			if(pUser && pUser->teamid != 0)
			{
				GlobalTeamIndex::getInstance()->MemberMoveScen(pUser->teamid, scene);
				//sky �����ŵ���ʱ�б���,�Ա����糡�����ߵ�ʱ����
				g_MoveSceneMemberMapLock.lock();
				MoveSceneMemberMap[pUser->id] = pUser->teamid;
				g_MoveSceneMemberMapLock.unlock();
			}

            return true;
          }else {
            Zebra::logger->error("δ�ҵ��û�%s(%ld)",(char*) reg->byName,reg->dwID);
          }
        }
        else
          Zebra::logger->error("δ�ҵ���ɫ���ڵ�ͼ %s",(char *)reg->byMapFileName);
        //֪ͨ����ע��ʧ��
        Zebra::logger->error("�û�(%lu,%lu,%s,%lu)ע��ʧ��",reg->accid,reg->dwID,reg->byName,reg->dwTempID);
        Cmd::Session::t_unregUser_GateSession ret;
        ret.dwUserID=reg->dwID;
        if (scene)
          ret.dwSceneTempID=scene->tempid;
        else
          ret.dwSceneTempID=0;
        ret.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
        sendCmd(&ret,sizeof(ret));
        return true;
      }
      break;

    case Cmd::Session::PARA_GATE_UNREGUSER:
      {
        Cmd::Session::t_unregUser_GateSession *reg=(Cmd::Session::t_unregUser_GateSession *)cmd;
        UserSession *pUser=UserSessionManager::getInstance()->getUserByID(reg->dwUserID);
        SceneSession *scene=SceneSessionManager::getInstance()->getSceneByTempID(reg->dwSceneTempID);

        if (pUser)
        {
          CSortM::getMe().offlineCount(pUser);
          CUnionM::getMe().userOffline(pUser); // ���ڴ������Ա����
          CSchoolM::getMe().userOffline(pUser);
          CSeptM::getMe().userOffline(pUser);
          CQuizM::getMe().userOffline(pUser);
          CGemM::getMe().userOffline(pUser);
          UserSessionManager::getInstance()->removeUser(pUser);

          if (reg->retcode==Cmd::Session::UNREGUSER_RET_ERROR)
          {
            Zebra::logger->error("�û�%s(%ld)�����ش���ע��",pUser->name,pUser->id);
            SAFE_DELETE(pUser);
            return true;
          }
          else if (reg->retcode==Cmd::Session::UNREGUSER_RET_LOGOUT)
          {
            if (scene)
            {
              Cmd::Session::t_unregUser_SceneSession send;
              send.dwUserID=reg->dwUserID;
              send.dwSceneTempID=reg->dwSceneTempID;
              send.retcode=Cmd::Session::UNREGUSER_RET_LOGOUT;
              scene->sendCmd(&send,sizeof(send));
              Zebra::logger->info("���������û�%s(%ld)ע��",pUser->name,pUser->id);
              SAFE_DELETE(pUser);
              return true;
            }
            else
            {
              Cmd::Session::t_unregUser_SceneSession send;
              send.dwUserID=reg->dwUserID;
              send.dwSceneTempID=reg->dwSceneTempID;
              send.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
              SessionTaskManager::getInstance().broadcastScene(&send,sizeof(send));
              Zebra::logger->error("�û�%sע��ʱ��������,���͹㲥��Ϣע�������û�",pUser->name);
            }
          }
          SAFE_DELETE(pUser);
        }
        else
          Zebra::logger->error("ע��ʱδ�ҵ��û�%ld",reg->dwUserID);

        // �������̺�ע��ʧ������֪ͨ����
        /*
           if (reg->retcode==Cmd::Session::UNREGUSER_RET_LOGOUT)
           {
           Cmd::Session::t_unregUser_GateSession ret;
           ret.dwUserID=reg->dwUserID;
           ret.dwSceneTempID=reg->dwSceneTempID;
           ret.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
           sendCmd(&ret,sizeof(ret));
           }
        // */
        return true;
      }
      break;
    case Cmd::Session::PARA_UNION_DISBAND:
      {
        CUnionM::getMe().processGateMessage(cmd,cmdLen);
        return true;
      }
      break;
    case Cmd::Session::PARA_SEPT_DISBAND:
      {
        CSeptM::getMe().processGateMessage(cmd,cmdLen);
        return true;
      }
      break;
    case Cmd::Session::PARA_GATE_DELCHAR:
      {
        this->del_role(cmd,cmdLen);
        return true;

      }
      break;
    default:
      break;
  }

  Zebra::logger->error("SessionTask::msgParse_Gate(%u,%u,%u)",cmd->cmd,cmd->para,cmdLen);
  return false;
}

bool SessionTask::msgParse_Forward(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
  if (pNullCmd->cmd==Cmd::Session::CMD_FORWARD && pNullCmd->para==Cmd::Session::PARA_FORWARD_USER)
  {
    Zebra::logger->error("msgParse_Forward");
    Cmd::Session::t_Session_ForwardUser *rev=(Cmd::Session::t_Session_ForwardUser *)pNullCmd;
    UserSession *pUser=UserSessionManager::getInstance()->getUserByID(rev->dwID);
    Cmd::stNullUserCmd * cmd = (Cmd::stNullUserCmd *)rev->data;
    if (pUser)
    {
      switch(cmd->byCmd)
      {
        case Cmd::GIFT_USERCMD:
          {
            return Gift::getMe().doGiftCmd(pUser,(Cmd::stNullUserCmd*)rev->data,rev->size);
          }
          break;
        case Cmd::NPCDARE_USERCMD:
          {
            return CNpcDareM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,
                rev->size);
          }
          break;
        case Cmd::QUIZ_USERCMD:
          {
            return CQuizM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,
                rev->size);
          }
          break;
        case Cmd::DARE_USERCMD:
          {
            return CDareM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd *)rev->data,rev->size);
          }
          break;
        case Cmd::SCHOOL_USERCMD:
          {
            return CSchoolM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd *)rev->data,rev->size);
          }
          break;
        case Cmd::UNION_USERCMD:
          {
            return CUnionM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd *)rev->data,rev->size);
          }
          break;
        case Cmd::RELATION_USERCMD:
          {
            return pUser->relationManager.processUserMessage((Cmd::stNullUserCmd *)rev->data,rev->size);
          }
          break;
        case Cmd::SEPT_USERCMD:
          {
            return CSeptM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd *)rev->data,rev->size);
          }
          break;
        case Cmd::COUNTRY_USERCMD:
          {
            return CCountryM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,rev->size);
          }
          break;
        case Cmd::ARMY_USERCMD:
          {
            return CArmyM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,rev->size);
          }
          break;
        case Cmd::RECOMMEND_USERCMD:
          {
            return RecommendM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,rev->size);
          }
        case Cmd::ALLY_USERCMD:
          {
            return CAllyM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,rev->size);
          }
          break;
        case Cmd::GEM_USERCMD:
          {
            return CGemM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,rev->size);
          }
          break;
        case Cmd::VOTE_USERCMD:
          {
            return CVoteM::getMe().processUserMessage(pUser,(Cmd::stNullUserCmd*)rev->data,rev->size);
          }
          break;
        case Cmd::DATA_USERCMD:
          {
            WORD degree = CSortM::getMe().getLevelDegree(pUser);
            Cmd::stLevelDegreeDataUserCmd send;
            send.degree = degree;
            pUser->sendCmdToMe(&send,sizeof(send));
            return true;
          }
          break;
		
        case Cmd::CHAT_USERCMD:
          {
            switch (cmd->byParam)
            {
              case REQUEST_COUNTRY_HELP_USERCMD_PARA:
              case KILL_FOREIGNER_USERCMD_PARA:
              case REFRESH_BOSS_USERCMD_PARA:
              case KILL_BOSS_USERCMD_PARA:
                {
                  SessionTaskManager::getInstance().sendCmdToCountry(pUser->country,cmd,cmdLen);
                }
                break;
              case QUESTION_OBJECT_USERCMD_PARA:
                {
                  Cmd::stQuestionObject* questionCmd = (Cmd::stQuestionObject*)cmd;
#ifdef _DEBUG
                  Zebra::logger->debug("�յ���Ʒ��ѯ����");
#endif              
                  if (questionCmd)
                  {
                    UserSession* pFromUser = UserSessionManager::getInstance()->
                      getUserSessionByName(questionCmd->name);
                    Cmd::Session::t_questionObject_SceneSession send;


                    if (pFromUser && pFromUser->scene && pUser)
                    {
                      strncpy(send.from_name,questionCmd->name,MAX_NAMESIZE);
                      strncpy(send.to_name,pUser->name,MAX_NAMESIZE);

                      send.dwObjectTempID = questionCmd->dwObjectTempID;

                      pFromUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_questionObject_SceneSession));
                    }
                    else
                    {
                      if (pUser)
                      {
                        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
                            "�Է��Ѳ�����");
                      }
                    }
                  }

                  return true;
                }
                break;
              case CREATE_CHANNEL_USERCMD_PARAMETER:
                {
                  Cmd::stCreateChannelUserCmd *create=(Cmd::stCreateChannelUserCmd *)cmd;
                  SessionChannel * sc = new SessionChannel(pUser);
                  if (!sc) return false;
                  if (!SessionChannelManager::getMe().add(sc))
                  {
                    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ֻ�ܴ���һ��Ƶ��");
                    SAFE_DELETE(sc);
                    return true;
                  }
                  Cmd::stCreateChannelUserCmd ret;
                  ret.dwChannelID=sc->tempid;
                  ret.dwClientID=create->dwClientID;
                  strncpy(ret.name,create->name,MAX_NAMESIZE);
                  pUser->sendCmdToMe(&ret,sizeof(ret));
                  sc->add(pUser);

                  UserSession * us1 = UserSessionManager::getInstance()->getUserSessionByName(create->name);
                  sc->add(us1);
                  UserSession * us = UserSessionManager::getInstance()->getUserSessionByName(create->name2);
                  if (us)
                  {
                    Cmd::stInvite_ChannelUserCmd inv;
                    inv.dwChannelID=sc->tempid;
                    inv.dwCharType = pUser->face;
                    strncpy(inv.name,pUser->name,MAX_NAMESIZE);
                    us->sendCmdToMe(&inv,sizeof(inv));
                  }
                  return true;
                }
                break;
              case INVITE_CHANNEL_USERCMD_PARAMETER:
                {
                  Cmd::stInvite_ChannelUserCmd *invite=(Cmd::stInvite_ChannelUserCmd *)cmd;
                  //SceneUser *pUser=SceneUserManager::getMe().getUserByName(invite->name);
                  UserSession * us = UserSessionManager::getInstance()->getUserSessionByName(invite->name);
                  if (us)
                  {
                    SessionChannel *cl=SessionChannelManager::getMe().get(invite->dwChannelID);
                    if (cl)
                    {
                      if (strncmp(pUser->name,cl->name,MAX_NAMESIZE)!=0)
                      {
                        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻�������û�");
                        return true;
                      }
                      if (cl->has(us->tempid))
                      {
                        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է��Ѿ���Ƶ������");
                        return true;
                      }
                      if (cl->count()>=20)
                      {
                        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"Ƶ����������");
                        return true;
                      }
                    }
                    else
                    {
                      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����Ƶ��������");
                      return true;
                    }

                    Cmd::stInvite_ChannelUserCmd inv;
                    inv.dwChannelID=invite->dwChannelID;
                    inv.dwCharType = pUser->face;
                    strncpy(inv.name,pUser->name,MAX_NAMESIZE);
                    us->sendCmdToMe(&inv,sizeof(inv));
                  }
                  else
                    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��� %s ������",invite->name);
                  return true;
                }
                break;
              case JOIN_CHANNEL_USERCMD_PARAMETER:
                {
                  Cmd::stJoin_ChannelUserCmd *join=(Cmd::stJoin_ChannelUserCmd *)cmd;
                  //UserSession *pHost=UserSessionManager::getMe().getUserSessionByName(join->host_name);
                  //if (pHost)
                  {
                    SessionChannel *cl = SessionChannelManager::getMe().get(join->dwChannelID);
                    if (cl)
                    {       
                      cl->add(pUser);
                    }
                    else    
                      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ڴ�����Ƶ��");
                  }
                }
                break;
              case LEAVE_CHANNEL_USERCMD_PARAMETER:
                {
                  Cmd::stLeave_ChannelUserCmd *leave=(Cmd::stLeave_ChannelUserCmd *)cmd;
                  //SceneUser *pHost=SceneUserManager::getMe().getUserByName(leave->host_name);
                  //if (pHost)
                  {       
                    SessionChannel *cl=SessionChannelManager::getMe().get(leave->dwChannelID);
                    if (cl)
                    {       
                      if (!cl->remove(pUser->tempid))
                      {       
                        SessionChannelManager::getMe().remove(cl->tempid);
                      }
                    }
                    else    
                      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ڴ�����Ƶ��");
                  }
                }
                break;
              default:
                break;
            }

            Cmd::stChannelChatUserCmd * chatCmd = (Cmd::stChannelChatUserCmd *)cmd;
            switch(chatCmd->dwType)
            {
              case Cmd::CHAT_TYPE_FRIEND_AFFICHE:
              case Cmd::CHAT_TYPE_FRIEND:        /// ����Ƶ��
                {
                  pUser->relationManager.sendChatToMyFriend(chatCmd,rev->size);
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_FRIEND_PRIVATE:      /// ����˽��
                {
                  pUser->relationManager.sendPrivateChatToFriend(chatCmd,rev->size);
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_UNION_AFFICHE:    /// ��ṫ��
              case Cmd::CHAT_TYPE_UNION:        /// ���Ƶ��
                {
                  CUnionM::getMe().sendUnionChatMessages(pUser,chatCmd,rev->size);
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_UNION_PRIVATE:      /// ���˽��
                {
                  CUnionM::getMe().sendUnionPrivateChatMessages(pUser,chatCmd,rev->size);
                  return true;
                }
                break;
        case Cmd::CHAT_TYPE_COUNTRY_MARRY:
              case Cmd::CHAT_TYPE_COUNTRY_PK:
              case Cmd::CHAT_TYPE_COUNTRY:      /// ����Ƶ��
                {
                  UserSession *pUser = UserSessionManager::getInstance()->getUserSessionByName(chatCmd->pstrName);

          if (pUser && pUser->unionid>0 && chatCmd->dwType != Cmd::CHAT_TYPE_COUNTRY_PK && chatCmd->dwType != Cmd::CHAT_TYPE_COUNTRY_MARRY)
                  {
                    if (chatCmd->dwSysInfoType == 
                      Cmd::INFO_TYPE_KING 
                || chatCmd->dwSysInfoType == Cmd::INFO_TYPE_CASTELLAN)
                    {
                      chatCmd->dwSysInfoType = 0;
                    }
                      
                    CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);

                    if (pUnion && pUnion->master && pUnion->master->id == pUser->id)
                    {//�ǰ���
                      if (CCityM::getMe().findByUnionID(pUser->unionid) != NULL)
                      {//����
                        CCity* pCity = CCityM::getMe().findByUnionID(pUser->unionid);
                        SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByID((pCity->dwCountry<<16)+pCity->dwCityID);
                        if (pScene){
                      //    sprintf(chatCmd->pstrName,"%s ����",pScene->name);
                          chatCmd->dwSysInfoType = 
                            Cmd::INFO_TYPE_CASTELLAN;

                        }
                      }

                      if (CCityM::getMe().
                          find(pUser->country,KING_CITY_ID,pUser->unionid) !=NULL)
                      {//�ǹ���
                      //  strncpy(chatCmd->pstrName,"����",MAX_NAMESIZE);
                        chatCmd->dwSysInfoType = Cmd::INFO_TYPE_KING;
                        CCountry* pEmperor = CCountryM::getMe().find(NEUTRAL_COUNTRY_ID);
                        if (pEmperor && pEmperor->dwKingUnionID == pUser->unionid)
                        {
                          chatCmd->dwSysInfoType = Cmd::INFO_TYPE_EMPEROR;
                        }

                      }
                    }
                  }

                  if (pUser)
                  {
                    if (chatCmd->dwSysInfoType == Cmd::INFO_TYPE_EXP &&
                    chatCmd->dwType == Cmd::CHAT_TYPE_COUNTRY)
                    {
                      Zebra::logger->error("�������%sʹ����������Զ�����Ϣˢ��",pUser->name);
                    }
                    SessionChannel::sendCountry(pUser->country,chatCmd,rev->size);
                    BYTE buf[zSocket::MAX_DATASIZE];
                    Cmd::GmTool::t_Chat_GmTool *cmd=(Cmd::GmTool::t_Chat_GmTool *)buf;
                    bzero(buf,sizeof(buf));
                    constructInPlace(cmd);

                    strncpy(cmd->userName,pUser->name,MAX_NAMESIZE);
                    cmd->countryID = pUser->country;
                    cmd->sceneID = pUser->scene->id;
                    cmd->dwType = chatCmd->dwType;
                    strncpy(cmd->content,chatCmd->pstrChat,255);
                    cmd->size = chatCmd->size;
                    if (cmd->size)
                      bcopy(chatCmd->tobject_array,cmd->tobject_array,cmd->size*sizeof(Cmd::stTradeObject),sizeof(buf) - sizeof(Cmd::GmTool::t_Chat_GmTool));
                    SessionService::getInstance().sendCmdToSuperServer(cmd,sizeof(Cmd::GmTool::t_Chat_GmTool)+cmd->size*sizeof(Cmd::stTradeObject));
                  }
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_OVERMAN_AFFICHE:  /// ʦ�Ź���
              case Cmd::CHAT_TYPE_OVERMAN:      /// ʦ��Ƶ��
                {
                  CSchoolM::getMe().sendSchoolChatMessages(pUser,chatCmd,rev->size);
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_OVERMAN_PRIVATE:      /// ʦ��˽��
                {
                  CSchoolM::getMe().sendSchoolPrivateChatMessages(pUser,chatCmd,rev->size);
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_FAMILY_AFFICHE:    /// ���幫��
              case Cmd::CHAT_TYPE_FAMILY:        /// ����Ƶ��
                {
                  CSeptM::getMe().sendSeptChatMessages(pUser,chatCmd,rev->size);
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_FAMILY_PRIVATE:      /// ����˽��
                {
                  CSeptM::getMe().sendSeptPrivateChatMessages(pUser,chatCmd,rev->size);
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_GM:
                chatCmd->dwType = Cmd::CHAT_TYPE_SYSTEM;
                chatCmd->dwSysInfoType = Cmd::INFO_TYPE_SCROLL;
              case Cmd::CHAT_TYPE_WORLD:
                {
                  //broadcastToEveryUser bte;
                  //if (bte.init(chatCmd,rev->size))
                  //{
                  CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);

                  if (pUnion && pUnion->master && pUnion->master->id == pUser->id)
                  {//�ǰ���

                    if (CCityM::getMe().
                        find(pUser->country,KING_CITY_ID,pUser->unionid) !=NULL)
                    {//�ǹ���
                      //  strncpy(chatCmd->pstrName,"����",MAX_NAMESIZE);
                      CCountry* pEmperor = CCountryM::getMe().find(NEUTRAL_COUNTRY_ID);
                      if (pEmperor && pEmperor->dwKingUnionID == pUser->unionid)
                      {
                        chatCmd->dwSysInfoType = Cmd::INFO_TYPE_EMPEROR;
                      }
                    }
                  }
                  SessionTaskManager::getInstance().sendCmdToWorld(chatCmd,rev->size);
                  //UserSessionManager::getInstance()->execEveryUser(bte);
                  BYTE buf[zSocket::MAX_DATASIZE];
                  Cmd::GmTool::t_Chat_GmTool *cmd=(Cmd::GmTool::t_Chat_GmTool *)buf;
                  bzero(buf,sizeof(buf));
                  constructInPlace(cmd);

                  strncpy(cmd->userName,pUser->name,MAX_NAMESIZE);
                  cmd->countryID = pUser->country;
                  cmd->sceneID = pUser->scene->id;
                  cmd->dwType = chatCmd->dwType;
                  strncpy(cmd->content,chatCmd->pstrChat,255);


                  cmd->size = chatCmd->size;
                  if (cmd->size)
                    bcopy(chatCmd->tobject_array,cmd->tobject_array,cmd->size*sizeof(Cmd::stTradeObject),sizeof(buf) - sizeof(Cmd::GmTool::t_Chat_GmTool));
                  SessionService::getInstance().sendCmdToSuperServer(cmd,sizeof(Cmd::GmTool::t_Chat_GmTool)+cmd->size*sizeof(Cmd::stTradeObject));
                  //}
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_PERSON:
                {
                  SessionChannel *cl=SessionChannelManager::getMe().get(chatCmd->dwChannelID);
                  if (cl)                                  
                  {
                    strncpy(chatCmd->pstrName,pUser->name,MAX_NAMESIZE);
                    cl->sendToOthers(pUser,chatCmd,rev->size);
                  }               
                  else                    
                    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ڴ�����Ƶ��");
                  return true;
                }
                break;
              case Cmd::CHAT_TYPE_BLESS_MSG:
                {
                  UserSession *otherUser = UserSessionManager::getInstance()->getUserSessionByName(chatCmd->pstrName);
                  if (otherUser)
                  {
                    if (strncmp(pUser->name,chatCmd->pstrName,MAX_NAMESIZE)!=0)
                    {
                      if (SessionTimeTick::currentTime<pUser->nextBlessTime)
                      {
                        pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"�Բ�������������ֻ�ܷ���һ��ף�����ھ����´οɷ���ʱ�仹��%u��",(pUser->nextBlessTime.msecs()-SessionTimeTick::currentTime.msecs())/1000);
                        return true;
                      }
                      else
                      {
                        pUser->nextBlessTime.now();
                        pUser->nextBlessTime.addDelay(120000);
                        strncpy(chatCmd->pstrName,pUser->name,MAX_NAMESIZE);
                        otherUser->sendCmdToMe(chatCmd,cmdLen);
                        pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"ף�����ͳ�");
                        return true;
                      }
                    }
                    else
                    {
                      pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"�����������");
                    }
                  }
                  else
                  {
                    pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"��������Ʋ���ȷ��ָ������Ҳ�����");
                  }
                  return true;
                }
                break;
            }
            return true;
          }
          break;
      }
    }
    else
    {
      switch(cmd->byCmd)
      {
        case Cmd::CHAT_USERCMD:
          {
            switch (cmd->byParam)
            {
              case REFRESH_BOSS_USERCMD_PARA:
                {
                  Cmd::stRefreshBossUserCmd * msg = (Cmd::stRefreshBossUserCmd *)cmd;
                  SessionTaskManager::getInstance().sendCmdToCountry(msg->country,cmd,cmdLen);
                }
                break;
            }
          }
          break;
        default:
          Zebra::logger->error("�����û�ָ(%d,%d)��ʱ,δ�ҵ���ɫΪ%ld���û�",pNullCmd->cmd,pNullCmd->para,rev->dwID);
          break;
      }
      return true;
    }
  }
  Zebra::logger->error("SessionTask::msgParse_Forward(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,cmdLen);
  return false;
}

//sky ����ս��������Session��Ϣ������
bool SessionTask::msgParse_Arena(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
	switch(cmd->para)
	{
	case Cmd::Session::PARA_USE_SPORTS_ADDMETOQUEUING:	//sky ֪ͨsess���Ŷӵ��û��ŵ��������д���
		{
			Cmd::Session::t_Sports_AddMeToQueuing * pCmd = (Cmd::Session::t_Sports_AddMeToQueuing*)cmd;
			CArenaManager::getInstance().Arena_AddUser(pCmd);
		}
		break;
	case Cmd::Session::PARA_SCENE_MEISBATTLEFIELD:
		{
			CArenaManager::getInstance().InsertBattleTask(this);
		}
		break;
	case Cmd::Session::PARA_USE_SPORTS_RETURNMAPID:		//sky �����ҵ����õ�ս����ͼ��֪ͨsession��ͼID
		{
			Cmd::Session::t_Sports_ReturnMapID *reg=(Cmd::Session::t_Sports_ReturnMapID *)cmd;

			if(reg->dwID != 0)		//sky ���ɶ�̬��ͼ�ɹ�
			{
				SceneSession *scene=new SceneSession(this);
				Cmd::Session::t_regScene_ret_SceneSession ret;
				ret.dwTempID=reg->dwTempID;

				//sky ��ע���ͼ
				if (scene->reg((Cmd::Session::t_regScene_SceneSession *)reg) 
					&& SceneSessionManager::getInstance()->addScene(scene))
				{
					ret.byValue=Cmd::Session::REGSCENE_RET_REGOK;
					CCountryM::getMe().refreshTax();
					CCountryM::getMe().refreshTech(this,reg->dwCountryID);
					if (KING_CITY_ID==(reg->dwID&0x0000ffff))
						CCountryM::getMe().refreshGeneral(reg->dwCountryID);
					CAllyM::getMe().refreshAlly(this);

					Zebra::logger->info("ע���ͼ%u(%s %s) �ɹ�",reg->dwID,reg->byName,reg->fileName);
					CCityM::getMe().refreshUnion(reg->dwCountryID,reg->dwID & 0x0FFF);

					SceneSession * sc = SceneSessionManager::getInstance()->getSceneByID(reg->dwID);
					if(!sc)
					{
						fprintf(stderr,"bad\n");
					}

					//sky ��ս����ͼ��ID�ŵ���������Ķ��й�������ȥ
					CArenaManager::getInstance().AddMapToQueuing(reg);
					//sky һ������ɹ�������ָ�Ϊ��
					CArenaManager::getInstance().NewMap_Lock(reg->AddMeType, true);
					return true;
				}
				else
				{
					ret.byValue=Cmd::Session::REGSCENE_RET_REGERR;
					Zebra::logger->error("ע���ͼ%u(%s %s)ʧ��",reg->dwID,reg->byName,reg->fileName);
				}
				sendCmd(&ret,sizeof(ret));
				return true;
			}
			else
			{
				//sky ���ض����й����������볡��ƫ������ƶ�
				CArenaManager::getInstance().MoveSceneM(1);
			}
			
		}
		break;	
	default:
		break;
	}

	return true;
}

bool SessionTask::msgParse(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
  return MessageQueue::msgParse(cmd,cmdLen);
}

bool SessionTask::cmdMsgParse(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
	NFilterModuleArray::const_iterator pIterator;

	//command
	for(pIterator=g_nFMA.begin(); pIterator != g_nFMA.end();pIterator++)
	{
		if (pIterator->filter_command((PBYTE)cmd,cmdLen)) return true;
	}

	if (Cmd::Session::CMD_FORWARD == cmd->cmd && Cmd::Session::PARA_FORWARD_USER == cmd->para)
	{
		Zebra::logger->error("cmdMsgParse");
		Cmd::Session::t_Session_ForwardUser *pSFU=(Cmd::Session::t_Session_ForwardUser *)cmd;
		UserSession *pUser=UserSessionManager::getInstance()->getUserByID(pSFU->dwID);
		Cmd::stNullUserCmd * pNUC = (Cmd::stNullUserCmd *)pSFU->data;
		Cmd::stChannelChatUserCmd *pCCUC;

		Zebra::logger->error("SessionTask::cmdMsgParse(%u,%u,%u)",pNUC->byCmd,pNUC->byParam,cmdLen);

		if (Cmd::CHAT_USERCMD == pNUC->byCmd && ALL_CHAT_USERCMD_PARAMETER == pNUC->byParam)
		{
			pCCUC = (Cmd::stChannelChatUserCmd*)pNUC;
			Zebra::logger->error("SessionTask::cmdMsgParse dwType=%d,dwSysInfoType=%d,dwChannelID=%d,size=%d,pstrChat=%s",pCCUC->dwType,pCCUC->dwSysInfoType,pCCUC->dwChannelID,pCCUC->size,pCCUC->pstrChat);
		}


		using namespace Cmd;

		if(pNUC->byCmd == TURN_USERCMD && pNUC->byParam == PARA_CHECKRELATION_EMPTY)

		{



			Zebra::logger->error("�յ��û���ϵ�����Ϣ");
			bool isEmpty = true;
			for(int i = 0; i < 6; ++i)
			{
				if( i == Cmd::RELATION_TYPE_FRIEND || i == Cmd::RELATION_TYPE_OVER )
					continue;
				CRelation* relation = pUser->relationManager.getRelationByType(i);
				if(relation != NULL)
					isEmpty = false;

			}


			Zebra::logger->error("isEmpty = %d",isEmpty);

			Cmd::t_CheckRelationEmptyResult sendM;
			sendM.isEmpty = isEmpty;
			sendM.dwUserID = pSFU->dwID;

			pUser->scene->sendCmd(&sendM,sizeof(sendM));

			//forwardScene(&sendM,sizeof(sendM));

			return true;
		}


	}

	switch(cmd->cmd)
	{
	case Cmd::CMD_NULL:
		break;
	case Cmd::Session::CMD_SCENE:
		return msgParse_Scene(cmd,cmdLen);
	case Cmd::Session::CMD_GATE:
		return msgParse_Gate(cmd,cmdLen);
	case Cmd::Session::CMD_FORWARD:
		return msgParse_Forward(cmd,cmdLen);
	case Cmd::Session::CMD_SCENE_SHUTDOWN:
		{
			switch(cmd->para)
			{
			case Cmd::Session::PARA_SHUTDOWN:
				{
					Cmd::Session::t_shutdown_SceneSession *sss = (Cmd::Session::t_shutdown_SceneSession*)cmd; 
					struct tm tm_1;
					time_t timval=time(NULL);
					//tm_1=*localtime(&timval);
					zRTime::getLocalTime(tm_1,timval);
					Zebra::logger->info("ϵͳ��ǰʱ��%d��%d��%d��%dʱ%d��%d��",tm_1.tm_year+1900,tm_1.tm_mon+1,tm_1.tm_mday,tm_1.tm_hour,tm_1.tm_min,tm_1.tm_sec);
					if (sss->time)
					{
						SessionChannel::sendAllInfo(Cmd::INFO_TYPE_SCROLL,"ϵͳ��ǰʱ��%d��%d��%d��%dʱ%d��%d��",tm_1.tm_year+1900,tm_1.tm_mon+1,tm_1.tm_mday,tm_1.tm_hour,tm_1.tm_min,tm_1.tm_sec);
						//tm_1=*localtime(&sss->time);
						zRTime::getLocalTime(tm_1,sss->time);
						SessionChannel::sendAllInfo(Cmd::INFO_TYPE_SCROLL,"ϵͳ����%d��%d��%d��%dʱ%d��%d��ͣ��ά��",tm_1.tm_year+1900,tm_1.tm_mon+1,tm_1.tm_mday,tm_1.tm_hour,tm_1.tm_min,tm_1.tm_sec);
						Zebra::logger->debug("ϵͳ����%d��%d��%d��%dʱ%d��%d��ͣ��ά��",tm_1.tm_year+1900,tm_1.tm_mon+1,tm_1.tm_mday,tm_1.tm_hour,tm_1.tm_min,tm_1.tm_sec);
						if (strlen(sss->info)>0)
						{
							SessionChannel::sendAllInfo(Cmd::INFO_TYPE_SCROLL,"%s",sss->info);
						}
						SessionService::getInstance().shutdown_time=*sss;
					}
					else
					{
						Zebra::logger->debug("ȡ��ͣ��ά��");
						SessionChannel::sendAllInfo(Cmd::INFO_TYPE_SCROLL,"ȡ��ͣ��ά��");
						SessionService::getInstance().shutdown_time=*sss;
						Cmd::Session::t_SetService_SceneSession send;
						send.flag |= Cmd::Session::SERVICE_MAIL;
						send.flag |= Cmd::Session::SERVICE_AUCTION;
						SessionTaskManager::getInstance().broadcastScene(&send,sizeof(send));

						SessionChannel::sendAllInfo(Cmd::INFO_TYPE_SYS,"�ʼ�ϵͳ������ϵͳ�Ѿ���������������ʹ����");
						Zebra::logger->info("ȡ��ͣ���������ʼ�����������");
					}
					return true;
				}
				break;
			}
		}
		break;
	case Cmd::Session::CMD_SCENE_SEPT:
		CSeptM::getMe().processSceneSeptMessage(cmd,cmdLen);
		return true;
	case Cmd::Session::CMD_SCENE_UNION:
		CUnionM::getMe().processSceneUnionMessage(cmd,cmdLen);
		return true;
	case Cmd::Session::CMD_SCENE_COUNTRY:
		CCountryM::getMe().processSceneMessage(cmd,cmdLen);
		return true;
	case Cmd::Session::CMD_SCENE_DARE:
		CDareM::getMe().processSceneMessage(cmd,cmdLen);
		return true;
	case Cmd::Session::CMD_SCENE_ARMY:
		CArmyM::getMe().processSceneMessage(cmd,cmdLen);
		return true;
	case Cmd::Session::CMD_SCENE_GEM:
		CGemM::getMe().processSceneMessage(cmd,cmdLen);
		return true;
	case Cmd::Session::CMD_SCENE_TMP:
		{
			switch(cmd->para)
			{
			case Cmd::Session::CLEARRELATION_PARA:
				{
					Cmd::Session::t_ClearRelation_SceneSession* rev=
						(Cmd::Session::t_ClearRelation_SceneSession*)cmd;

					//CSeptM::getMe().delSeptAllMember();
					CUnionM::getMe().delAllUnion(rev->dwUserID);
					return true;
				}
				break;
			case Cmd::Session::RETURN_CREATE_UNION_ITEM_PARA:
				{//��ָ��ID���û���������ʼ�
					Cmd::Session::t_ReturnCreateUnionItem_SceneSession* rev=
						(Cmd::Session::t_ReturnCreateUnionItem_SceneSession*)cmd;

					Cmd::Session::t_sendMail_SceneSession sm;

					sm.mail.state = Cmd::Session::MAIL_STATE_NEW;
					strncpy(sm.mail.fromName,"ϵͳ",MAX_NAMESIZE);
					sm.mail.toID = rev->dwUserID;
					strncpy(sm.mail.title,"������",MAX_NAMESIZE);
					sm.mail.type = Cmd::Session::MAIL_TYPE_MAIL;
					sm.mail.createTime = rev->item.createtime;
					sm.mail.delTime = sm.mail.createTime + 60*60*24*7;
					sm.mail.accessory = 1;
					sm.mail.itemGot = 0;
					_snprintf(sm.mail.text,255-1,"%s","����������õ���");
					sm.mail.sendMoney = 0;
					sm.mail.recvMoney = 0;
					bcopy(&rev->item,&sm.item,sizeof(sm.item),sizeof(sm.item));

					MailService::getMe().sendMail(sm);
					MailService::getMe().sendMoneyMail("ϵͳ",0,"",rev->dwUserID,
						UnionDef::CREATE_UNION_NEED_PRICE_GOLD,
						"���������������",(DWORD)-1,
						Cmd::Session::MAIL_TYPE_MAIL);

					return true;
				}
				break;
			}
		}
		break;
	case Cmd::Session::CMD_SCENE_SPORTS:
		{
			return msgParse_Arena(cmd,cmdLen);
		}
		break;
  }
  return false;
}

SessionTask::~SessionTask()
{
}

