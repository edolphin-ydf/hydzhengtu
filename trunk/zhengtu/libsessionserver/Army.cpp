/**
 * \brief ���Ҿ���
 *
 * 
 */
#include <zebra/SessionServer.h>
#include <memory>

CArmyM::CArmyM()
{
}

CArmyM::~CArmyM()
{
}

bool CArmyM::init()
{
  return this->load();
}

void CArmyM::destroyMe()
{
  for (DWORD i=0; i<CArmyM::getMe().armys.size(); i++)
  {       
    CArmyM::getMe().armys[i]->writeDatabase();
  }

  delMe();
}

bool CArmyM::load()
{
  DBFieldSet* army = SessionService::metaData->getFields("ARMY");
  
  if (army)
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
      recordset = SessionService::dbConnPool->exeSelect(handle,army,NULL,NULL);
    }

    SessionService::dbConnPool->putHandle(handle);

    if (recordset)
    {
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);
        CArmy* pArmy = new CArmy();

        if (pArmy)
        {
          pArmy->init(rec);
          pArmy->loadCaptainFromDB();
        }
          
        armys.push_back(pArmy);
      }

      SAFE_DELETE(recordset)
    }
  }
  else
  {
    Zebra::logger->error("�������ݼ���ʧ��,ARMY������");
    return false;
  }

  return true;
}

void CArmyM::userOnline(UserSession* pUser)
{
  capIter sIterator;
  CCaptain *pCaptain = NULL; 

  rwlock.rdlock();
  sIterator = captainIndex.find(pUser->id);
  if (sIterator != captainIndex.end()) pCaptain = (*sIterator).second;
  rwlock.unlock();

  if (pCaptain!=NULL) 
  {
    pCaptain->update_scene();
  }
}

bool CArmyM::processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
  switch (pNullCmd->para)        
  {                               
    case Cmd::Session::REQ_ARMY_LIST_SCENE_PARA:
      {              
        Cmd::Session::t_ReqArmyList_SceneSession* cmd = 
          (Cmd::Session::t_ReqArmyList_SceneSession*)pNullCmd;

        this->processReqArmyList(cmd);
        return true;
      }
    default:
      break;
  } 

  return false;
}

bool CArmyM::processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  switch (pNullCmd->byParam) 
  {
    case FIRE_ARMY_CAPTAIN_PARA:
      {
        Cmd::stFireArmyCaptainUserCmd* rev = (Cmd::stFireArmyCaptainUserCmd*)pNullCmd;
        this->processFireCaptain(pUser,rev);
        return true;
      }
      break;
    case ADD_ARMY_CAPTAIN_PARA:
      {
        Cmd::stAddArmyCaptainUserCmd* rev = (Cmd::stAddArmyCaptainUserCmd*)pNullCmd;
        this->processAddCaptain(pUser,rev);
        return true;
      }
      break;
    case REMOVE_ARMY_PARA:
      {
        Cmd::stRemoveArmyUserCmd* rev = (Cmd::stRemoveArmyUserCmd*)pNullCmd;
        this->processRemoveArmy(pUser,rev);
        return true;
      }
      break;
    case EXIT_ARMY_PARA:
      {
        Cmd::stExitArmyUserCmd* rev = (Cmd::stExitArmyUserCmd*)pNullCmd;
        this->processExitArmy(pUser,rev);
        return true;
      }
      break;
    case CHANGE_ARMY_NAME_PARA:
      {
        Cmd::stChangeArmyNameUserCmd* rev = (Cmd::stChangeArmyNameUserCmd*)pNullCmd;
        this->processChangeArmyName(pUser,rev);
        return true;
      }
      break;
    case REQ_ARMY_SPEC_PARA:
      {
        Cmd::stReqArmySpecUserCmd* rev = (Cmd::stReqArmySpecUserCmd*)pNullCmd;
        this->processReqArmySpec(pUser,rev);
        return true;
      }
      break;
    case REQ_ARMY_GEN_PARA:
      {
        Cmd::stReqArmyGenUserCmd* rev = (Cmd::stReqArmyGenUserCmd*)pNullCmd;
        
        if (rev->byStatus == Cmd::YES_ARMY_GEN)
        {
          CArmy* pArmy = this->findByGenID(pUser->id);
          
          if (pArmy)
          {
            pArmy->status(CArmy::FINISH_CREATE);
            pArmy->insertDatabase();
            pArmy->hireCaptain(pUser->id);
            pUser->sendSysChat(Cmd::INFO_TYPE_EXP,"��ϲ����Ϊ %s ���ӽ���",
                pArmy->name);
            Zebra::logger->info("[����]: %s ���ӽ����ɹ�",pArmy->name);
            
            SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_EXP,pArmy->dwCountryID,"��ϲ %s ���ӳ���",
                pArmy->name);

          }
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����������ѹ���");
          }
        }

        return true;
      }
      break;
    case CREATE_ARMY_PARA:
      {
        Cmd::stCreateArmyUserCmd* rev = (Cmd::stCreateArmyUserCmd*)pNullCmd;
        this->processCreateArmy(pUser,rev);
        return true;
      }
      break;
    case REQ_WAIT_GEN_PARA:
      {
        Cmd::stReqWaitGenUserCmd* rev = (Cmd::stReqWaitGenUserCmd*)pNullCmd;
        this->processReqWaitGen(pUser,rev);

        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

void CArmyM::processFireCaptain(UserSession* pUser,Cmd::stFireArmyCaptainUserCmd* rev)
{
  capIter sIterator;
  CCaptain *pCaptain = NULL; 

  sIterator = captainIndex.find(pUser->id);
  if (sIterator != captainIndex.end()) pCaptain = (*sIterator).second;

  if (pCaptain!=NULL) 
  {
    if (pCaptain->myArmy && pCaptain->myArmy->dwGenID == pUser->id && pUser->id != rev->dwUserID)
    {
      if (pCaptain->myArmy->fireCaptain(rev->dwUserID))
      {
        Zebra::logger->info("[����]: %s �뿪 %s ����",pUser->name,pCaptain->myArmy->name);
        pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����ɹ�");
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ܿ����Լ��������ǽ���");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǽ������ܿ����ӳ�");
  }

}

void CArmyM::processAddCaptain(UserSession* pUser,Cmd::stAddArmyCaptainUserCmd* rev)
{
  if (rev->byState == Cmd::QUESTION_CAPTAIN) 
  {
    capIter sIterator;
    CCaptain *pCaptain = NULL; 

    sIterator = captainIndex.find(pUser->id);
    if (sIterator != captainIndex.end()) pCaptain = (*sIterator).second;

    if (pCaptain!=NULL) 
    {
      if (pCaptain->myArmy && pCaptain->myArmy->dwGenID == pUser->id)
      {
        if (pCaptain->myArmy->canAddCaptain())
        {
          UserSession* pAddUser = UserSessionManager::getInstance()->
            getUserSessionByName(rev->capName);
          
          if (pAddUser)
          {
            if (captainIndex.find(pAddUser->id) != captainIndex.end())
            {
              pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
                  "�Է��Ѽ�����������,��������");
              return;
            }

            if (!CCityM::getMe().isCastellan(pAddUser))  
            {
              rev->capID = pAddUser->id;
              strncpy(rev->armyName,pCaptain->myArmy->name,MAX_NAMESIZE);
              rev->armyID = pCaptain->myArmy->dwID;
              pAddUser->sendCmdToMe(rev,sizeof(Cmd::stAddArmyCaptainUserCmd));
            }
          }
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է�������,��������");
          }
        }
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǽ���,�������նӳ�");
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǽ���,�������նӳ�");
    }
  }
  else if (rev->byState == Cmd::ANSWER_CAPTAIN_YES)
  {
    CArmy* pArmy = this->findByID(rev->armyID);

    if (pArmy)
    {
      pArmy->hireCaptain(rev->capID);
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"Ҫ�μӵľ��Ӳ�����");
    }
  }
  else if (rev->byState == Cmd::ANSWER_CAPTAIN_NO)
  {
    CArmy* pArmy = this->findByID(rev->armyID);

    if (pArmy)
    {
      UserSession* pGenUser = UserSessionManager::getInstance()->getUserByID(pArmy->dwGenID);
      if (pGenUser)
      {
        pGenUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"%s �ܾ��ξ�",pUser->name);
      }
    }
  }
}

void CArmyM::processRemoveArmy(UserSession* pUser,Cmd::stRemoveArmyUserCmd* rev)
{
  CArmy* pArmy = this->findByID(rev->dwArmyID);
  
  if (pArmy)
  {
    CCity* pCity = CCityM::getMe().findByUnionID(pUser->unionid);
    CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);

    if ((pCity && pUnion && pArmy->dwCityID == pCity->dwCityID &&  
      (pUnion->tempid == pUser->id)) || (pArmy->dwGenID == pUser->id))
    {//�ж��Ƿ�Ϊ�þ������ڳǵĳ����򽫾�
      this->removeArmyByID(rev->dwArmyID);
    }
    else
    {//�þ��ӵĽ���
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����Ǹóǳ�����þ�����,���ܽ�ɢ�þ���");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�þ��Ӳ�����");
  }
}

int    CArmyM::countByCity(DWORD dwCountryID,DWORD dwCityID)
{
  int ret = 0;
  rwlock.rdlock();
  
  for (std::vector<CArmy*>::iterator vIter=armys.begin(); vIter!=armys.end(); vIter++)
  {
    CArmy* temp = *vIter;
    if (temp->dwCountryID == dwCountryID && temp->dwCityID == dwCityID)
    {
      ret++;
    }
  }
  rwlock.unlock();
  return ret;

}

void CArmyM::processExitArmy(UserSession* pUser,Cmd::stExitArmyUserCmd* rev)
{
  capIter sIterator;
  CCaptain *pCaptain = NULL; 

  sIterator = captainIndex.find(pUser->id);
  if (sIterator != captainIndex.end()) pCaptain = (*sIterator).second;

  if (pCaptain!=NULL)
  {
    if (pCaptain->myArmy && pCaptain->myArmy->dwGenID == pCaptain->dwCharID)
    {
      this->removeArmyByGenID(pCaptain->dwCharID);
    }
    else
    {
      if (pCaptain->myArmy) 
      {
        CArmy* pArmy = pCaptain->myArmy;
        if (pArmy->fireCaptain(pUser->id))
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_EXP,"�����뿪����");
        }
      }
    }  
  }

}

void CArmyM::processChangeArmyName(UserSession* pUser,Cmd::stChangeArmyNameUserCmd* rev)
{
  CArmy* pArmy = this->findByID(rev->dwArmyID);

  if (pArmy)
  {
    CCity* pCity = CCityM::getMe().findByUnionID(pUser->unionid);
    CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);
    
    if (pCity && pUnion && pCity->dwCityID==pArmy->dwCityID && pUnion->tempid == pUser->id)
    {
      if (this->findByName(rev->newArmyName)==NULL)
      {
        pArmy->changeName(rev->newArmyName);
        pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"���Ӹ����ɹ�");
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�µľ��������ظ�,����������");
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����Ǹóǳ���,���ܸ��ĸþ�������");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"û�иþ��ӵ���Ϣ");
  }
}

void CArmyM::processReqArmySpec(UserSession* pUser,Cmd::stReqArmySpecUserCmd* rev)
{
  CArmy* pArmy = this->findByID(rev->dwArmyID);
  if (pArmy)
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stCaptainBase* tempPoint;
    Cmd::stRtnArmySpecUserCmd* retCmd = (Cmd::stRtnArmySpecUserCmd*)buf;
    constructInPlace(retCmd);
    tempPoint = (Cmd::stCaptainBase*)retCmd->data;
    
    strncpy(retCmd->name,pArmy->name,MAX_NAMESIZE); // ������
    strncpy(retCmd->genname,pArmy->genName,MAX_NAMESIZE); // ������
    SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByID((pArmy->dwCountryID<<16) 
        + pArmy->dwCityID);

    if (pScene)
    {
      strncpy(retCmd->cityname,pScene->name,MAX_NAMESIZE); // ��������
    }

    pArmy->rwlock.rdlock();
    for (std::vector<CCaptain*>::iterator vIter=pArmy->captains.begin(); vIter!=pArmy->captains.end(); ++vIter)
    {
      CCaptain* pCaptain = *vIter;
      strncpy(tempPoint->name,pCaptain->szCapName,MAX_NAMESIZE);
      tempPoint->dwCharID = pCaptain->dwCharID;
        
      tempPoint++;
      retCmd->dwSize++;
    }

    pArmy->rwlock.unlock();

    if (retCmd->dwSize>0)                   
    {                                       
      pUser->sendCmdToMe(retCmd,(retCmd->dwSize*sizeof(Cmd::stCaptainBase) + 
            sizeof(Cmd::stRtnArmySpecUserCmd)));
    }     
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"û�иþ��ӵ���Ϣ");
  }
}

void CArmyM::processReqArmyList(Cmd::Session::t_ReqArmyList_SceneSession* rev)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::stArmyBaseInfo* tempPoint;
  Cmd::stRtnArmyListUserCmd* retCmd = (Cmd::stRtnArmyListUserCmd*)buf;
  constructInPlace(retCmd);
  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(rev->dwUserID);
  if (pUser == NULL)
  {
    return;
  }

  tempPoint = (Cmd::stArmyBaseInfo*)retCmd->data;
  retCmd->byType = rev->byType;

  rwlock.rdlock();
  for (std::vector<CArmy*>::iterator vIter=armys.begin(); vIter!=armys.end(); vIter++)
  {
    CArmy* temp = *vIter;
    if (temp->byStatus != CArmy::WAIT_CREATE && temp->dwCountryID == pUser->country)
    {
      if ((rev->byType == Cmd::CITY_ARMY_LIST && temp->dwCityID == rev->dwCityID)
       || (rev->byType != Cmd::CITY_ARMY_LIST))
      {
        tempPoint->dwArmyID = temp->dwID;
        strncpy(tempPoint->name,temp->name,MAX_NAMESIZE); // ��������
        SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByID((temp->dwCountryID<<16) + temp->dwCityID);

        if (pScene)
        {
          strncpy(tempPoint->cityname,pScene->name,MAX_NAMESIZE); // ��������
        }

        strncpy(tempPoint->genname,temp->genName,MAX_NAMESIZE); // ��������

        tempPoint->dwCapNum = temp->captains.size(); // �ӳ�����
        tempPoint++;
        retCmd->dwSize++;
      }
    }
  }

  rwlock.unlock();

  if (retCmd->dwSize>0)                   
  {                                       

    if (pUser)
    pUser->sendCmdToMe(retCmd,(retCmd->dwSize*sizeof(Cmd::stArmyBaseInfo) + sizeof(Cmd::stRtnArmyListUserCmd)));
  }    
}

void CArmyM::processCreateArmy(UserSession* pUser,Cmd::stCreateArmyUserCmd* rev)
{
  CCity* pCity = CCityM::getMe().findByUnionID(pUser->unionid);
  CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);
  UserSession* pGenUser = UserSessionManager::getInstance()->getUserSessionByName(rev->genName);

  if (strlen(rev->armyName) <=0)
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������������");
    return;
  }

  if (pGenUser == NULL)
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ѡ��Ľ������ڲ�����,���Ժ�����");
    return;
  }

  if (pGenUser->id == pUser->id)
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"������ѡ���Լ���Ϊ����!");
    return;
  }
  
  if (this->findByGenID(pGenUser->id) != NULL)
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"%s �Ѿ��ǽ���,�벻Ҫ�ظ�����",rev->genName);
    return;
  }

  if (this->findByName(rev->armyName) != NULL)
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���������ظ�,���ܽ�������");
    return;
  }
  

  if (pCity!= NULL && pUnion!=NULL)
  {
    if (this->countByCity(pCity->dwCountry,pCity->dwCityID)>1)
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������Ѵ����ޡ������ٴ�������");
      return;
    }
      
    if (pUnion->master && pUnion->master->id == pUser->id)
    {
      if (pGenUser)
      {
        CArmy* pArmy = new CArmy();
        if (pArmy) {
        
        pArmy->dwCountryID = pUser->country;
        pArmy->dwCityID = pCity->dwCityID;

        strncpy(pArmy->name,rev->armyName,MAX_NAMESIZE); // ��������
        pArmy->dwGenID = pGenUser->id; // ���ӽ�����ɫID
        strncpy(pArmy->genName,pGenUser->name,MAX_NAMESIZE); // ���ӽ�������
        pArmy->byStatus = CArmy::WAIT_CREATE;
        pArmy->dwCreateTime = time(NULL);
        pArmy->dwID = 0;

        rwlock.wrlock();
        armys.push_back(pArmy);
        rwlock.unlock();

        Cmd::stReqArmyGenUserCmd send;
        pGenUser->sendCmdToMe(&send,sizeof(send));
        }
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"ѡ��Ľ���������,��������ʧ��");
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǳ���,����ʹ�ø����");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǳ���,����ʹ�ø����");
  }
}

void CArmyM::processReqWaitGen(UserSession* pUser,Cmd::stReqWaitGenUserCmd* rev)
{
  CCity* pCity = CCityM::getMe().findByUnionID(pUser->unionid);
  CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);

  if (pCity!= NULL && pUnion!=NULL)
  {
    if (pUnion->master && pUnion->master->id == pUser->id)
    {
      UserSessionManager::getInstance()->sendExploitSort(pUser);
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǳ���,����ʹ�ø����");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ǳ���,����ʹ�ø����");
  }

}
CArmy* CArmyM::findByID(DWORD dwArmyID)
{
  CArmy* pArmy = NULL;
  rwlock.rdlock();
  
  for (std::vector<CArmy*>::iterator vIter=armys.begin(); vIter!=armys.end(); vIter++)
  {
    CArmy* temp = *vIter;
    if (temp->dwID == dwArmyID)
    {
      pArmy = temp;
      break;
    }
  }
    
  rwlock.unlock();
  return pArmy;
}

CArmy* CArmyM::findByName(const char* value)
{
  CArmy* pArmy = NULL;
  rwlock.rdlock();
  
  for (std::vector<CArmy*>::iterator vIter=armys.begin(); vIter!=armys.end(); vIter++)
  {
    CArmy* temp = *vIter;
    if (strncmp(temp->name,value,MAX_NAMESIZE) == 0)
    {
      pArmy = temp;
      break;
    }
  }
    
  rwlock.unlock();
  return pArmy;
}

CArmy* CArmyM::findByGenID(DWORD dwGenID)
{
  CArmy* pArmy = NULL;
  rwlock.rdlock();
  
  for (std::vector<CArmy*>::iterator vIter=armys.begin(); vIter!=armys.end(); vIter++)
  {
    CArmy* temp = *vIter;
    if (temp->dwGenID == dwGenID)
    {
      pArmy = temp;
      break;
    }
  }
    
  rwlock.unlock();
  return pArmy;
}

void CArmyM::removeArmyByID(DWORD dwArmyID)
{
  std::vector<CArmy*>::iterator pos;
  
  for (pos = armys.begin(); pos!=armys.end(); pos++)
  {
    CArmy* pArmy = *pos;
    if (pArmy->dwID == dwArmyID)
    {
      pArmy->deleteMeFromDB();

      rwlock.wrlock();
      armys.erase(pos);
      rwlock.unlock();

      SAFE_DELETE(pArmy);
      break;
    }
  }

}

void CArmyM::removeArmyByGenID(DWORD dwGenID)
{
  std::vector<CArmy*>::iterator pos;
  
  for (pos = armys.begin(); pos!=armys.end(); pos++)
  {
    CArmy* pArmy = *pos;
    if (pArmy->dwGenID == dwGenID)
    {
      if (pArmy->dwID > 0) {
        pArmy->deleteMeFromDB();
      }

      rwlock.wrlock();
      armys.erase(pos);
      rwlock.unlock();

      SAFE_DELETE(pArmy);
      break;
    }
  }

}
void CArmyM::timer()
{
  time_t cur_time = time(NULL);

  std::vector<CArmy*> vRemoveList;
  
  rwlock.rdlock();
  for (std::vector<CArmy*>::iterator vIter=armys.begin(); vIter!=armys.end(); vIter++)
  {
    CArmy* temp = *vIter;
    if (temp->byStatus == CArmy::WAIT_CREATE && ::abs(cur_time-temp->dwCreateTime) > 60)
    {
      vRemoveList.push_back(temp);
    }
  }
  rwlock.unlock();
  
  for (std::vector<CArmy*>::iterator vIter=vRemoveList.begin(); vIter!=vRemoveList.end(); vIter++)
  {
    CArmy* temp = *vIter;
    Zebra::logger->info("[����]: %s �����򽫾��ܾ�����,��������",temp->name);
    this->removeArmyByGenID(temp->dwGenID);
  }
}

bool CArmyM::addCaptain(DWORD dwUserID,CCaptain* pCaptain)
{
  std::pair<std::map<DWORD,CCaptain*>::iterator,bool> retval;
  rwlock.wrlock();
  retval = captainIndex.insert(captainIndexValueType(dwUserID,pCaptain));
  rwlock.unlock();
  return retval.second;
}

bool CArmyM::removeCaptain(DWORD dwUserID)
{
  bool ret;
  std::map <DWORD,CCaptain*>::iterator sIterator;
  rwlock.wrlock();
  ret = true;
  sIterator = captainIndex.find(dwUserID);
  if (sIterator != captainIndex.end()) captainIndex.erase(sIterator);
  else ret = false;
  rwlock.unlock();
  return ret;
}

bool CArmyM::isCaptain(DWORD dwUserID)
{
  bool ret = false;
  std::map <DWORD,CCaptain*>::iterator sIterator;
  rwlock.rdlock();
  sIterator = captainIndex.find(dwUserID);
  rwlock.unlock();

  if (sIterator != captainIndex.end())
  {
    ret = true;
  }

  return ret;
}

//------------------------CArmy--------------------------------------------
CArmy::CArmy()
{
  this->dwID = 0; // ����ID
  this->dwCountryID = 0; // ��������ID
  this->dwCityID = 0; // ��������ID
  this->dwGenID = 0; // ���ӽ�����ɫID
  this->byStatus = 0;
  this->dwCreateTime = 0;

  bzero(name,MAX_NAMESIZE); // ��������
  bzero(genName,MAX_NAMESIZE); // ���ӽ�������
}

CArmy::~CArmy()
{
  for (std::vector<CCaptain*>::iterator pos = captains.begin(); pos!=captains.end(); pos++)
  {
    SAFE_DELETE(*pos);
  }
}

void CArmy::init(DBRecord* rec)
{
  rwlock.wrlock();
  this->dwID = rec->get("id");
  this->dwCountryID = rec->get("countryid");
  this->dwCityID = rec->get("cityid");
  strncpy(this->name,rec->get("name"),sizeof(this->name));
  strncpy(this->genName,rec->get("genname"),sizeof(this->genName));
  this->dwGenID = rec->get("genid");

  rwlock.unlock();
  //this->writeDatabase();
}

void CArmy::writeDatabase()
{
  DBRecord rec,where;
  std::ostringstream oss;

  if (this->byStatus == CArmy::WAIT_CREATE)
  {
    return;
  }
    
  oss << "id='" << this->dwID << "'";
  where.put("id",oss.str());
  
  rec.put("genname",this->genName);
  rec.put("genid",this->dwGenID);
  rec.put("name",this->name);

  DBFieldSet* army = SessionService::metaData->getFields("ARMY");

  if (army)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();
    
    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return;
    }

    if ((connHandleID)-1 != handle)
    {
      SessionService::dbConnPool->exeUpdate(handle,army,&rec,&where);
    }

    SessionService::dbConnPool->putHandle(handle);

    /*for (std::vector<CVoteItem*>::iterator vIter=items.begin(); vIter!=items.end(); vIter++)
    {
      CVoteItem* pVoteItem = *vIter;
      if (pVoteItem) {
        pVoteItem->writeDatabase();
      }
    }
    // */
  }
  else
  {
    Zebra::logger->error("�������ݱ���ʧ��,ARMY������");
    return;
  }  
}

bool CArmy::insertDatabase()
{
  DBRecord rec;
  rec.put("countryid",this->dwCountryID);
  rec.put("cityid",this->dwCityID);
  rec.put("name",this->name);
  rec.put("genname",this->genName);
  rec.put("genid",this->dwGenID);

  DBFieldSet* army = SessionService::metaData->getFields("ARMY");

  if (army)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {       
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }
    else
    {       
      DWORD retcode = SessionService::dbConnPool->exeInsert(handle,army,&rec);
      if ((DWORD)-1 == retcode)
      {
        SessionService::dbConnPool->putHandle(handle);
        return false;
      }  
      SessionService::dbConnPool->putHandle(handle);
      this->dwID = retcode;
    }
  }
  else
  {
    Zebra::logger->error("��Ӿ��Ӽ�¼ʧ��,ARMY������");
    return false;
  }

  return true;
}

bool CArmy::deleteMeFromDB()
{
  DBRecord where;
  std::ostringstream oss;
  oss << "id='" << this->dwID << "'";
  where.put("id",oss.str());
  
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DBFieldSet* army = SessionService::metaData->getFields("ARMY");
  
  if (army)
  {
    SessionService::dbConnPool->exeDelete(handle,army,&where);
  }

  SessionService::dbConnPool->putHandle(handle);

  rwlock.wrlock();  
  for (std::vector<CCaptain*>::iterator vIter=captains.begin(); vIter!=captains.end(); vIter++)
  {
    CCaptain* temp = *vIter;
    temp->fireMe();
    SAFE_DELETE(temp);
  }
  captains.clear();
  rwlock.unlock();
  
  return true;
}

bool CArmy::loadCaptainFromDB()
{
  DBFieldSet* captain = SessionService::metaData->getFields("CAPTAIN");
  
  if (captain)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }

    DBRecordSet* recordset = NULL;
    DBRecord where;
    std::ostringstream oss;

    oss << "armyid='" << this->dwID << "'";
    where.put("armyid",oss.str());

    if ((connHandleID)-1 != handle)
    {
      recordset = SessionService::dbConnPool->exeSelect(handle,captain,NULL,&where);
    }

    SessionService::dbConnPool->putHandle(handle);

    if (recordset)
    {
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);
        CCaptain* pCaptain = new CCaptain();

        if (pCaptain)
        {
          pCaptain->init(rec);
          pCaptain->myArmy = this;
          CArmyM::getMe().addCaptain(pCaptain->dwCharID,pCaptain);      
          captains.push_back(pCaptain);

        }
      }

      SAFE_DELETE(recordset)
    }
  }
  else
  {
    Zebra::logger->error("������ϸ���ݼ���ʧ��,CAPTAIN������");
    return false;
  }

  return true;
}

bool CArmy::hireCaptain(DWORD dwUserID)
{
  bool ret = false;

  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(dwUserID);
  if (pUser)
  {
    CCaptain* pCaptain = new CCaptain();

    if (pCaptain)
    {
      pCaptain->dwArmyID = this->dwID;
      pCaptain->dwCharID = pUser->id;
      strncpy(pCaptain->szCapName,pUser->name,MAX_NAMESIZE);
      pCaptain->dwNpcNum = 0;

      pCaptain->myArmy = this;
      if (pCaptain->insertDatabase())
      {
        CArmyM::getMe().addCaptain(pCaptain->dwCharID,pCaptain);      

        rwlock.wrlock();
        captains.push_back(pCaptain);
        rwlock.unlock();
        Zebra::logger->info("[����]: %s �μ� %s ����",pUser->name,this->name);

        pCaptain->update_scene();
        ret=true;
      }
      else
      {
        SAFE_DELETE(pCaptain);
      }
    }
  }

  return ret;
}

bool CArmy::fireCaptain(DWORD dwUserID)
{
  bool ret = false;
  rwlock.wrlock();

  for (std::vector<CCaptain*>::iterator vIter=this->captains.begin(); vIter!=this->captains.end(); ++vIter)
  {
    CCaptain* pCaptain = *vIter;
    if (pCaptain->dwCharID == dwUserID)
    {
      pCaptain->fireMe();
      captains.erase(vIter);
      SAFE_DELETE(pCaptain);
      ret = true;
      break;
    }
  }

  rwlock.unlock();
  return ret;
}

void CArmy::changeName(const char* newname)
{
  rwlock.wrlock();
  strncpy(name,newname,MAX_NAMESIZE);
  rwlock.unlock();
  
  this->writeDatabase();
  this->update_all_captain();
}

void CArmy::update_all_captain()
{
  for (std::vector<CCaptain*>::iterator vIter=this->captains.begin(); vIter!=this->captains.end(); ++vIter)
  {
    CCaptain* pCaptain = *vIter;
    pCaptain->update_scene();
  }
  
  Cmd::Session::t_sendUserArmyInfo_SceneSession send;
  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(this->dwGenID);
  if (pUser && pUser->scene)
  {
    strncpy(send.title,this->name,MAX_NAMESIZE);
    send.byType = 2;
    send.dwUserID = this->dwGenID;

    pUser->scene->sendCmd(&send,sizeof(send));
  }

}

bool CArmy::canAddCaptain()
{
  int cur_cap =  captains.size();
  int max_cap = 0;
  int vexploit[] = {0,100,150,200,250,300,350,400,450,500,550,660,770,880,990,1100,1210,1320,1430,
      1540,
      1700,1860,2020,2180,2340,2500,2660,2820,2980,3100,3220,3340,3460,3580,3700,3820,
      3940,4060,4180,4300,4420,4600,4780,4960,5140,5320,5500,5680,5860,7000,8140,9280,
      10420,11560,12700,13840,15000,16160,17320,18480,19640,25000,30360,35720,41080,46440,
      51800,57160,62520,67800};

  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(this->dwGenID);

  if (pUser)
  {
    int cur_exploit = pUser->dwExploit/100;

    if (cur_exploit <100) {
      max_cap = 10;
    }
    else
    {
      for (int i=0; i<69; i++)
      {
        if (cur_exploit>=vexploit[i] && cur_exploit<vexploit[i+1])
        {
          max_cap = i*10 + 10;
          break;
        }
      }
    }

    if (cur_cap<max_cap)
      return true;
  }

  return false;
}

//------------------------CCaptain--------------------------------------------
CCaptain::CCaptain()
{
  this->dwArmyID = 0; // ����ID
  this->dwCharID = 0; // �ӳ�ID
  bzero(szCapName,MAX_NAMESIZE); // �ӳ�����
  this->dwNpcNum = 0; // �ӳ������NPC��,��ʱδ��
  this->myArmy = NULL;
}

CCaptain::~CCaptain()
{
}

void CCaptain::init(DBRecord* rec)
{
  this->dwArmyID = rec->get("armyid");
  this->dwCharID = rec->get("charid");
  this->dwNpcNum = rec->get("npcnum");
  strncpy(this->szCapName,rec->get("charname"),MAX_NAMESIZE);
}

void CCaptain::writeDatabase()
{
  DBRecord rec,where;
  std::ostringstream oss;

  oss << "armyid='" << this->dwArmyID << "'";
  where.put("armyid",oss.str());
  
  oss.str("");
  oss << "`charid`='" << this->dwCharID << "'";
  where.put("charid",oss.str());

  rec.put("npcnum",this->dwNpcNum);
  

  DBFieldSet* captain = SessionService::metaData->getFields("CAPTAIN");

  if (captain)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();
    
    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return;
    }
    else
    {
      SessionService::dbConnPool->exeUpdate(handle,captain,&rec,&where);
      SessionService::dbConnPool->putHandle(handle);
    }
  }
  else
  {
    Zebra::logger->error("������ϸ���ݱ���ʧ��,CAPTAIN������");
    return;
  }  
}

bool CCaptain::insertDatabase()
{
  DBRecord rec;
  rec.put("armyid",this->dwArmyID);
  rec.put("charid",this->dwCharID);
  rec.put("charname",this->szCapName);
  rec.put("npcnum",this->dwNpcNum);

  DBFieldSet* captain = SessionService::metaData->getFields("CAPTAIN");

  if (captain)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {       
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }
    else
    {       
      DWORD retcode = SessionService::dbConnPool->exeInsert(handle,captain,&rec);
      if ((DWORD)-1 == retcode)
      {
        SessionService::dbConnPool->putHandle(handle);
        return false;
      }  
      SessionService::dbConnPool->putHandle(handle);
    }
  }
  else
  {
    Zebra::logger->error("��Ӿ��Ӽ�¼ʧ��,CAPTAIN������");
    return false;
  }

  return true;
}

void CCaptain::fireMe()
{
  if (this->deleteMeFromDB())
  {
    CArmyM::getMe().removeCaptain(this->dwCharID);

    UserSession* pUser = UserSessionManager::getInstance()->getUserByID(this->dwCharID);
    if (pUser && pUser->scene)
    {
      Cmd::Session::t_sendUserArmyInfo_SceneSession send;
      send.dwUserID = this->dwCharID;
      bzero(send.title,MAX_NAMESIZE);
      send.byType = 0;
      pUser->scene->sendCmd(&send,sizeof(send));
      pUser->sendSysChat(Cmd::INFO_TYPE_EXP,"�����뿪����");
    }
  }
}

bool CCaptain::deleteMeFromDB()
{
  DBRecord where;
  std::ostringstream oss;

  oss << "armyid='" << this->dwArmyID << "'";
  where.put("armyid",oss.str());
  
  oss.str("");
  oss << "`charid`='" << this->dwCharID << "'";
  where.put("charid",oss.str());
      
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DBFieldSet* captain = SessionService::metaData->getFields("CAPTAIN");
    
  if (captain)
  {
    SessionService::dbConnPool->exeDelete(handle,captain,&where);
  }

  SessionService::dbConnPool->putHandle(handle);
    
  return true;
}

void CCaptain::update_scene()
{
  Cmd::Session::t_sendUserArmyInfo_SceneSession send;
  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(this->dwCharID);
  if (this->myArmy && pUser && pUser->scene)
  {
    /*SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByID((this->myArmy->dwCountryID<<16) 
        + this->myArmy->dwCityID);

    if (pScene)
    {
      strncpy(send.title,pScene->name,MAX_NAMESIZE); // ��������
    }*/

    strncpy(send.title,this->myArmy->name,MAX_NAMESIZE);

    if (this->dwCharID == this->myArmy->dwGenID)
    {
      send.byType = 2;
    }
    else
    {
      send.byType = 1;
    }
    send.dwUserID = this->dwCharID;

    pUser->scene->sendCmd(&send,sizeof(send));
  }
}

/*
 * $log$
 */
