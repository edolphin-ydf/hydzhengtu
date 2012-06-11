/**
 * \brief ͬ�˹�������ʵ��
 *
 * 
 */
#include <zebra/SessionServer.h>

CAlly::CAlly()
{
  this->dwCountryID = 0;
  this->dwAllyCountryID = 0;
  this->dwFriendDegree = 0;
  this->dwCreateTime = 0;
  this->dwLastUpTime = 0;
  this->byStatus = 2;
}

CAlly::~CAlly()
{
}

void CAlly::init(DBRecord* rec)
{
  if (rec)
  {
    this->dwCountryID  = rec->get("countryid");
    this->dwAllyCountryID = rec->get("allycountryid");
    this->dwFriendDegree  = rec->get("frienddegree");
    this->dwCreateTime = rec->get("createtime");
    this->dwLastUpTime = rec->get("lastuptime");
  }
}

void CAlly::writeDatabase()
{
  DBRecord rec,where;
  std::ostringstream oss;

  oss << "countryid='" << this->dwCountryID << "'";
  where.put("countryid",oss.str());
  oss.str("");

  oss << "allycountryid='" << this->dwAllyCountryID << "'";
  where.put("allycountryid",oss.str());

  rec.put("frienddegree",this->dwFriendDegree);
  rec.put("lastuptime",this->dwLastUpTime);
  DBFieldSet* ally = SessionService::metaData->getFields("ALLY");

  if (ally)
  {       
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return;
    }

    if ((connHandleID)-1 != handle)
    {
      SessionService::dbConnPool->exeUpdate(handle,ally,&rec,&where);
    }

    SessionService::dbConnPool->putHandle(handle);
  }
  else
  {
    Zebra::logger->error("�����������ݱ���ʧ�ܣ�ALLY������");
    return;
  }
}

bool CAlly::insertDatabase()
{
  DBRecord rec;
  rec.put("countryid",this->dwCountryID);
  rec.put("allycountryid",this->dwAllyCountryID);
  rec.put("createtime",this->dwCreateTime);
  rec.put("frienddegree",this->dwFriendDegree);

  DBFieldSet* ally = SessionService::metaData->getFields("ALLY");

  if (ally)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }
    else
    {
      SessionService::dbConnPool->exeInsert(handle,ally,&rec);
      Zebra::logger->info("[��������]: %s,%s �������˹�ϵ",
                CCountryM::getMe().find(this->dwCountryID)->name,
         CCountryM::getMe().find(this->dwAllyCountryID)->name);

    }

    SessionService::dbConnPool->putHandle(handle);
  }
  else
  {
    Zebra::logger->error("�������������½�ʧ�ܣ�ALLY������");
    return false;
  }

  return true;
}

bool CAlly::deleteMeFromDB()
{
  DBRecord where;
  std::ostringstream oss;

  oss << "countryid='" << this->dwCountryID << "'";
  where.put("countryid",oss.str());
  oss.str("");

  oss << "allycountryid='" << this->dwAllyCountryID << "'";
  where.put("allycountryid",oss.str());

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DBFieldSet* ally = SessionService::metaData->getFields("ALLY");

  if (ally)
  {
    SessionService::dbConnPool->exeDelete(handle,ally,&where);
    Zebra::logger->info("[��������]: %s,%s ������˹�ϵ",
                CCountryM::getMe().find(this->dwCountryID)->name,
         CCountryM::getMe().find(this->dwAllyCountryID)->name);

    this->refreshAlly(true);
    this->refreshAllyToAllUser(true);
  }
  else
  {
    Zebra::logger->error("������������ɾ��ʧ�ܣ�ALLY������");
    SessionService::dbConnPool->putHandle(handle);
    return false;
  }

  SessionService::dbConnPool->putHandle(handle);
  return true;
}

bool CAlly::changeFriendDegree(int degree)
{
  int old_degree = this->dwFriendDegree;

  if (degree<0)
  {
    if (this->dwFriendDegree>::abs(degree))
    {
      this->dwFriendDegree = this->dwFriendDegree - ::abs(degree);
      /*if (old_degree>=10000 && this->dwFriendDegree<10000)
      {
        SessionChannel::sendAllInfo(Cmd::INFO_TYPE_GAME,
        " %s �� %s �Ĺ��ҹ�ϵ����Ϊ ���ƹ�ϵ",
        CCountryM::getMe().find(this->dwCountryID)->name,
        CCountryM::getMe().find(this->dwAllyCountryID)->name);
      }
      // */
    }
    else
    {
      this->dwFriendDegree = 0;
    }
  }
  else
  {
    this->dwFriendDegree += degree;
    /* if (old_degree<10000 && this->dwFriendDegree>=10000)
    {
      SessionChannel::sendAllInfo(Cmd::INFO_TYPE_GAME,
          " %s �� %s �Ĺ��ҹ�ϵ����Ϊ Э����ϵ",
          CCountryM::getMe().find(this->dwCountryID)->name,
          CCountryM::getMe().find(this->dwAllyCountryID)->name);
    }
    // */
  }

  this->writeDatabase();

  if (degree>0 && ((1000 - (old_degree%1000))<=degree))
  {
#ifdef _DEBUG
    Zebra::logger->debug("[��������]: �����ڶ�");
#endif    
    SessionChannel::sendAllInfo(Cmd::INFO_TYPE_EXP,
        " %s ���⽻�����Ѵ����ǳ���,%s ���⽻�����Ѵ����ǳ���,�����Ͻ�ǰ���߾�",
        CCountryM::getMe().find(this->dwCountryID)->name,
        CCountryM::getMe().find(this->dwAllyCountryID)->name);

    Cmd::Session::t_summonAllyNpc_SceneSession send;
    send.dwCountryID = this->dwCountryID;
    SessionTaskManager::getInstance().broadcastScene(&send,sizeof(send));
    
    send.dwCountryID = this->dwAllyCountryID;
    SessionTaskManager::getInstance().broadcastScene(&send,sizeof(send));
  }

  this->refreshAlly();

  return true;
}

void CAlly::refreshAlly(bool isFire)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::Session::_allyItem*  tempItem = NULL;

  Cmd::Session::t_updateAlly_SceneSession* send = (Cmd::Session::t_updateAlly_SceneSession*)buf;
  constructInPlace(send);
  tempItem = (Cmd::Session::_allyItem*)send->data;
  send->dwSize = 0;

  if (isFire)  
  {
    tempItem->dwCountryID = this->dwCountryID;
    tempItem->dwAllyCountryID = this->dwAllyCountryID;
    tempItem->dwFriendDegree =  0;
  }
  else
  {
    tempItem->dwCountryID = this->dwCountryID;
    tempItem->dwAllyCountryID = this->dwAllyCountryID;
    tempItem->dwFriendDegree =  this->friendDegree();
  }

  send->dwSize++;

  SessionTaskManager::getInstance().broadcastScene(send,send->dwSize*sizeof(Cmd::Session::_allyItem) + 
      sizeof(Cmd::Session::t_updateAlly_SceneSession));
}

void CAlly::refreshAllyToAllUser(bool isFire)
{
  Cmd::stUpdateCountryAlly send;
  
  if (isFire)
  {
    send.dwAllyCountryID = 0;
    SessionTaskManager::getInstance().sendCmdToCountry(this->dwCountryID,&send,sizeof(send));

    send.dwAllyCountryID = 0;
    SessionTaskManager::getInstance().sendCmdToCountry(this->dwAllyCountryID,&send,sizeof(send));
  }
  else
  {
    send.dwAllyCountryID = this->dwAllyCountryID;
    SessionTaskManager::getInstance().sendCmdToCountry(this->dwCountryID,&send,sizeof(send));

    send.dwAllyCountryID = this->dwCountryID;
    SessionTaskManager::getInstance().sendCmdToCountry(this->dwAllyCountryID,&send,sizeof(send));
  }
}

//------------------CAllyM----------
bool CAllyM::init()
{
  return this->loadAllyFromDB();
}

bool CAllyM::loadAllyFromDB()
{
  DBFieldSet* ally = SessionService::metaData->getFields("ALLY");

  if (ally)
  {       
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {       
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }               

    DBRecordSet* recordset = NULL;

    if ((connHandleID)-1 != handle)
    {
      recordset = SessionService::dbConnPool->exeSelect(handle,ally,NULL,NULL);
    }

    SessionService::dbConnPool->putHandle(handle);
    if (recordset)
    {
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);
        CAlly* pAlly = new CAlly();

        if (pAlly)
        {
          pAlly->init(rec);

          if (this->findAlly(pAlly->dwCountryID,pAlly->dwAllyCountryID) != NULL)
          {
            Zebra::logger->error("[��������]: %d �� %d ���ظ�����������.",
                pAlly->dwCountryID,pAlly->dwAllyCountryID);
            SAFE_DELETE(pAlly);
          }
          else
          {
            rwlock.wrlock();
            allies.push_back(pAlly);
            rwlock.unlock();
          }
        }
      }

      SAFE_DELETE(recordset)
    }
  }
  else
  {
    Zebra::logger->error("�����������ݼ���ʧ�ܣ�ALLY������");
    return false;
  }

  return true;

}

bool CAllyM::processUserMessage(UserSession* pUser,const Cmd::stNullUserCmd* pNullCmd,const DWORD cmdLen)
{
  switch (pNullCmd->byParam)
  {
    case REQ_COUNTRY_ALLY_INFO_PARA:
      {
        Cmd::stReqCountryAllyInfo* rev = (Cmd::stReqCountryAllyInfo*)pNullCmd;
        processReqCountryAllyInfo(pUser,rev);
        return true;
      }
      break;
    case CANCEL_COUNTRY_ALLY_PARA:
      {
        Cmd::stCancelCountryAlly* rev = (Cmd::stCancelCountryAlly*)pNullCmd;
        processCancelCountryAlly(pUser,rev);
        return true;
      }
      break;
    case REQ_COUNTRY_ALLY_PARA:
      {
        Cmd::stReqCountryAlly* rev = (Cmd::stReqCountryAlly*)pNullCmd;
        processReqCountryAlly(pUser,rev);
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

bool CAllyM::processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
  return false;
}

void CAllyM::processReqCountryAllyInfo(UserSession* pUser,Cmd::stReqCountryAllyInfo* rev)
{
  CAlly* pAlly = this->findAlly(pUser->country);
  if (pAlly)
  {
    Cmd::stRtnCountryAllyInfo send;
    if (pAlly->dwCountryID == pUser->country)
    {
      send.dwAllyCountryID = pAlly->dwAllyCountryID;
    }
    else
    {
      send.dwAllyCountryID = pAlly->dwCountryID;
    }
    
    send.dwFriendDegree = pAlly->friendDegree();
    pUser->sendCmdToMe(&send,sizeof(send));
  }
}

void CAllyM::processReqCountryAlly(UserSession* pUser,Cmd::stReqCountryAlly* rev)
{
  switch (rev->byStatus)
  {
    case Cmd::QUESTION_COUNTRY_ALLY:
      {
        if (!CCountryM::getMe().isKing(pUser))
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǹ���,����ʹ�øù���");
          return;
        }

        CCountry* pCountry = CCountryM::getMe().find(pUser->country);
        if (!pCountry) 
        {
          Zebra::logger->error("[��������]:�������ݲ�����,����ʹ���������");
          return;
        }

        if (rev->dwAllyCountryID == pUser->country)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "���ܺ��Լ�����");
        }

        if ((int)pCountry->getMaterial(COUNTRY_MONEY)<CREATE_ALLY_NEED_MONEY)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "���ҽ����������,���ܽ���");
          return;
        }

        CCountry* pAllyCountry = CCountryM::getMe().find(rev->dwAllyCountryID);
        if (!pAllyCountry) 
        { 
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "������Ĺ��Ҳ�����");
          return;
        }

        rev->dwCountryID = pUser->country;
        CUnion* pUnion = CUnionM::getMe().getUnionByID(pAllyCountry->dwKingUnionID);

        if (!pUnion)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "�Է�����������");
          return;
        }

        UserSession* pAllyKing = UserSessionManager::getInstance()->getUserByID(pUnion->tempid);
        if (!pAllyKing)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "�Է�����������");
          return;
        }

        if (this->findAlly(pUser->country) != NULL)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "�������˹�,����������");
          return;
        }

        if (this->findAlly(rev->dwAllyCountryID) != NULL)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "�Է������˹�,����������");
          return;
        }


        pAllyKing->sendCmdToMe(rev,sizeof(Cmd::stReqCountryAlly));
        this->addReadyAlly(rev->dwCountryID,rev->dwAllyCountryID);
      }
      break;
    case Cmd::YES_COUNTRY_ALLY:
      {
        if (!CCountryM::getMe().isKing(pUser))
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǹ���,����ʹ�øù���");
          return;
        }

        CCountry* pAllyCountry = CCountryM::getMe().find(pUser->country);
        if (!pAllyCountry) 
        {
          Zebra::logger->error("[��������]:�������ݲ�����,����ʹ���������");
          return;
        }

        if ((int)pAllyCountry->getMaterial(COUNTRY_MONEY)<CREATE_ALLY_NEED_MONEY)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
              "���ҽ����������,���ܽ���");
          return;
        }

        if (NULL == this->findAlly(rev->dwCountryID,rev->dwAllyCountryID))
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���������Ѿ�ȡ��.");
          return;
        }

        CCountry* pCountry = CCountryM::getMe().find(rev->dwCountryID);
        if (pCountry)
        {
          pAllyCountry->changeMaterial(COUNTRY_MONEY,-CREATE_ALLY_NEED_MONEY);
          pCountry->changeMaterial(COUNTRY_MONEY,-CREATE_ALLY_NEED_MONEY);
          this->addNewAlly(rev->dwCountryID,rev->dwAllyCountryID);
        }
      }
      break;
    case Cmd::NO_COUNTRY_ALLY:
      {
        CCountry* pCountry = CCountryM::getMe().find(rev->dwCountryID);
        CUnion* pUnion = NULL;

        if (pCountry)
        {
          pUnion = CUnionM::getMe().getUnionByID(pCountry->dwKingUnionID);
        }

        if (pUnion)
        {
          UserSession* pKing = UserSessionManager::getInstance()->getUserByID(pUnion->tempid);
          if (pKing)
          {
            pKing->sendSysChat(Cmd::INFO_TYPE_FAIL,
                "�Է��ܾ�����");
            return;
          }
        }
      }
      break;
    default:
      break;

  }
}

void CAllyM::processCancelCountryAlly(UserSession* pUser,Cmd::stCancelCountryAlly* rev)
{
  if (!CCountryM::getMe().isKing(pUser))
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǹ���,����ʹ�øù���");
    return;
  }

  if (NULL == this->findAlly(pUser->country))
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"û���˹�");
    return;
  }


  this->fireAlly(pUser->country,0);
}

void CAllyM::refreshAlly(SessionTask* scene)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::Session::_allyItem*  tempItem= NULL;

  Cmd::Session::t_updateAlly_SceneSession* send = (Cmd::Session::t_updateAlly_SceneSession*)buf;
  constructInPlace(send);
  tempItem = (Cmd::Session::_allyItem*)send->data;
  send->dwSize = 0;

  std::vector<CAlly*>::iterator pos;
  for (pos=allies.begin(); pos!=allies.end(); pos++)  
  {
    CAlly* pAlly = *pos;
    if (pAlly)
    {
      tempItem->dwCountryID = pAlly->dwCountryID;
      tempItem->dwAllyCountryID = pAlly->dwAllyCountryID;
      tempItem->dwFriendDegree =  pAlly->friendDegree();
      tempItem++;
      send->dwSize++;
    }
  }

  if (scene) scene->sendCmd(send,send->dwSize*sizeof(Cmd::Session::_allyItem) + 
      sizeof(Cmd::Session::t_updateAlly_SceneSession));
}

void CAllyM::userOnline(UserSession* pUser)
{
  Cmd::stUpdateCountryAlly send;
  
  rwlock.rdlock();
  
  for (DWORD i=0; i<allies.size(); i++)
  {
    if (allies[i]->dwCountryID == pUser->country || allies[i]->dwAllyCountryID == pUser->country)
    {
      if (allies[i]->dwCountryID == pUser->country)
      {
        send.dwAllyCountryID = allies[i]->dwAllyCountryID;
      }
      else
      {
        send.dwAllyCountryID = allies[i]->dwCountryID;
      }
      break;
    }
  }
  
  rwlock.unlock();
  pUser->sendCmdToMe(&send,sizeof(send));
}


CAlly*  CAllyM::findAlly(DWORD dwCountryID1,DWORD dwCountryID2)
{
  CAlly* pAlly = NULL;

  rwlock.rdlock();
  for (DWORD i=0; i<allies.size(); i++)
  {
    CAlly* pTemp = allies[i];
    if (pTemp && ((pTemp->dwCountryID == dwCountryID1 && pTemp->dwAllyCountryID == dwCountryID2)
          || (pTemp->dwCountryID == dwCountryID2 && pTemp->dwAllyCountryID == dwCountryID1))
       )
    {
      pAlly = pTemp;
      break;
    }
  }
  rwlock.unlock();

  return pAlly;
}

CAlly*  CAllyM::findAlly(DWORD dwCountryID1)
{
  CAlly* pAlly = NULL;

  rwlock.rdlock();
  for (DWORD i=0; i<allies.size(); i++)
  {
    CAlly* pTemp = allies[i];
    if (pTemp && (pTemp->dwCountryID == dwCountryID1 || pTemp->dwAllyCountryID == dwCountryID1)
      )
    {
      pAlly = pTemp;
      break;
    }
  }
  rwlock.unlock();

  return pAlly;
}

void   CAllyM::addReadyAlly(DWORD dwCountryID1,DWORD dwCountryID2)
{
  CAlly* pAlly = new CAlly();

  if (pAlly)
  {
    pAlly->dwCountryID = dwCountryID1;
    pAlly->dwAllyCountryID = dwCountryID2;
    pAlly->dwFriendDegree = 1100;
    pAlly->byStatus = 1;
    pAlly->dwCreateTime = time(NULL);
    rwlock.wrlock();
    allies.push_back(pAlly);
    rwlock.unlock();
  }
}

bool   CAllyM::addNewAlly(DWORD dwCountryID1,DWORD dwCountryID2)
{
  CAlly* pAlly = this->findAlly(dwCountryID1,dwCountryID2);
  bool ret = false;

  rwlock.wrlock();

  if (pAlly)
  {
    if (pAlly->insertDatabase())
    {
      pAlly->byStatus = 2;
      ret = true;
      pAlly->refreshAlly();
      pAlly->refreshAllyToAllUser();
    }
  }

  rwlock.unlock();

  //TODO ֪ͨȫ�����˽���
  if (ret && pAlly)
  {
    SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_EXP,
        pAlly->dwCountryID,"�ҹ��� %s ��ʽ�޽���Լ��",
        CCountryM::getMe().find(pAlly->dwAllyCountryID)->name);

    SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
        pAlly->dwCountryID,"����ʹ�� 20�� ������ %s �����ˡ�",
        CCountryM::getMe().find(pAlly->dwAllyCountryID)->name);

    SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_EXP,
        pAlly->dwAllyCountryID,"�ҹ��� %s ����ʽ�޽���Լ��",
        CCountryM::getMe().find(pAlly->dwCountryID)->name);

    SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
        pAlly->dwAllyCountryID,"����ʹ�� 20�� ������ %s �����ˡ�",
        CCountryM::getMe().find(pAlly->dwCountryID)->name);

  }

  return true;
}

bool    CAllyM::fireAlly(DWORD dwCountryID1,DWORD dwCountryID2)
{
  CAlly* pAlly = NULL;
  
  if (0 == dwCountryID2) {
    pAlly = this->findAlly(dwCountryID1);
  }
  else{
    pAlly = this->findAlly(dwCountryID1,dwCountryID2);
  }
    
  if (pAlly)
  {
    if (pAlly->deleteMeFromDB())
    {
      SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_EXP,
          pAlly->dwCountryID,"�ҹ��� %s ���������Լ��",
          CCountryM::getMe().find(pAlly->dwAllyCountryID)->name);

      
      SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_EXP,
          pAlly->dwAllyCountryID,"�ҹ��� %s ���������Լ��",
          CCountryM::getMe().find(pAlly->dwCountryID)->name);

    
      this->removeAlly(dwCountryID1,dwCountryID2);
      return true;
    }
  }

  return false;
}

void CAllyM::removeAlly(DWORD dwCountryID1,DWORD dwCountryID2)
{
  std::vector<CAlly*>::iterator pos;
  rwlock.wrlock();
  for (pos = allies.begin(); pos!=allies.end(); pos++)
  {
    CAlly* pTemp = *pos;
    if (pTemp && ((pTemp->dwCountryID == dwCountryID1 && pTemp->dwAllyCountryID == dwCountryID2)
          || (pTemp->dwCountryID == dwCountryID2 && pTemp->dwAllyCountryID == dwCountryID1)
        || ((pTemp->dwCountryID==dwCountryID1 || pTemp->dwAllyCountryID==dwCountryID1)&&dwCountryID2==0) )
       )
    {
      allies.erase(pos);
      SAFE_DELETE(pTemp);
      break;
    }
  }
  rwlock.unlock();
}

void CAllyM::timer()
{
  std::vector<CAlly*>::iterator pos;
  time_t ct = time(NULL);
  struct tm tv1;
  zRTime::getLocalTime(tv1,ct);
  
  for (pos = allies.begin(); pos!=allies.end(); )
  {
    CAlly* pTemp = *pos;
    if (1 == pTemp->byStatus && (ct - pTemp->dwCreateTime) > 17)
    {
#ifdef _DEBUG
      Zebra::logger->debug("[��������]: %d,%d ���볬ʱ,�ر�",pTemp->dwCountryID,pTemp->dwAllyCountryID);
#endif      
      rwlock.wrlock();
      pos = allies.erase(pos);
      rwlock.unlock();
      SAFE_DELETE(pTemp);
    }
    else
    {
      pos++;
    }
  }
  
  if (tv1.tm_hour == 0 && (tv1.tm_min>=0 && tv1.tm_min<=4))
  {
    for (pos = allies.begin(); pos!=allies.end(); pos++)
    {
      CAlly* pTemp = *pos;
#ifdef _DEBUG
      Zebra::logger->debug("[��������]: ÿ�ո��������Ѻö�: (%d,%d) %d,ct:%u lasttime:%u",
          pTemp->dwCountryID,
          pTemp->dwAllyCountryID,pTemp->friendDegree(),
          ct,pTemp->dwLastUpTime);
#endif      
      if (2 == pTemp->byStatus && ((ct - pTemp->dwLastUpTime) > ((24*60*60)-10)))
      {
        pTemp->changeFriendDegree(-500);
        pTemp->dwLastUpTime = ct;
        pTemp->writeDatabase();
      }
    }

    std::vector<CAlly*> removeAlly;

    for (pos = allies.begin(); pos!=allies.end(); pos++)
    {
      CAlly* pTemp = *pos;
      if (pTemp->friendDegree()  == 0)
      {
        removeAlly.push_back(pTemp);
      }
    }

    for (pos = removeAlly.begin(); pos!=removeAlly.end(); pos++)
    {
      CAlly* pTemp = *pos;
      CAllyM::getMe().fireAlly(pTemp->dwCountryID,pTemp->dwAllyCountryID);
    }
  }
}

