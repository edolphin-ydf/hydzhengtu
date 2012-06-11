/**
 * \brief ���й�����
 *
 * 
 */
#include <zebra/SessionServer.h>

/**
 * \brief ���й�������ʼ��
 * \return true ��ʼ���ɹ� false��ʼ��ʧ��
 */
bool CCityM::init()
{
  return this->load();
}

/**
 * \brief ����������
 */
void CCityM::destroyMe()
{
  for (DWORD i=0; i<CCityM::getMe().citys.size(); i++)
  {
    CCityM::getMe().citys[i]->writeDatabase();
  }
}

CCityM::CCityM()
{
  this->isBeging = false;  
}

/**
 * \brief �����ݿ��м�������Ŀ���¼
 * \return true ���سɹ�
 */
bool CCityM::load()
{
  DBFieldSet* city = SessionService::metaData->getFields("CITY");

  if (city)
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
      recordset = SessionService::dbConnPool->exeSelect(handle,city,NULL,NULL);
    }

    SessionService::dbConnPool->putHandle(handle);

    if (recordset)
    {
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);


        CCity* pCity = new CCity();

        if (pCity)
        {
          pCity->init(rec);

          if (CUnionM::getMe().getUnionByID(pCity->dwUnionID) == NULL)
          {//���ӵ����
            pCity->dwUnionID = 0;
            pCity->isAward = 0;
            pCity->dwGold = 50000; 
            Zebra::logger->info("����Ѳ�����,���й������");
          }
        }


        citys.push_back(pCity);
      }

      SAFE_DELETE(recordset)
    }
  }
  else
  {
    Zebra::logger->error("CCityM::load �������ݼ���ʧ��,CITY������");
    return false;
  }

  return true;
}

bool CCityM::addNewCity(Cmd::Session::t_UnionCity_Dare_SceneSession* pCmd)
{
  bool ret = false;

  CCity* pCity = new CCity();

  if (pCity)
  {
    pCity->dwCountry = pCmd->dwToCountryID;
    if (pCmd->dwCityID > 1000)
    {
      pCity->dwCityID = pCmd->dwCityID/10;
    }
    else
    {
      pCity->dwCityID = pCmd->dwCityID;
    }
    
    if (pCity->dwCountry == PUBLIC_COUNTRY || pCity->dwCityID == KING_CITY_ID) 
    {
      pCity->dwGold = 50000; // ����5�� ��������5��
    }
    else
    {
      pCity->dwGold = 20000; // 2��
    }

    pCity->dwUnionID = pCmd->dwFromUnionID;

    if (pCity->insertDatabase())
    {
      rwlock.wrlock();
      citys.push_back(pCity);
      rwlock.unlock();
      this->refreshUnion(pCity->dwCountry,pCity->dwCityID);
      ret = true;
    }
    else
    {
      SAFE_DELETE(pCity);
    }
  }


  return ret;  
}

CCity* CCityM::find(DWORD country,DWORD cityid,DWORD unionid)
{
  CCity* pCity = NULL;

  if (!country || !cityid || !unionid)
  {
    return NULL;
  }

  rwlock.rdlock();
  for (DWORD i=0; i<citys.size(); i++)
  {
    if (citys[i]->dwCountry==country && citys[i]->dwCityID==cityid && citys[i]->dwUnionID==unionid)
    {
      pCity = citys[i];
      break;
    }
  }
  rwlock.unlock();

  return pCity;
}

CCity* CCityM::find(DWORD country,DWORD cityid)
{
  CCity* pCity = NULL;

  if (!country || !cityid)
    return NULL;

  rwlock.rdlock();
  for (DWORD i=0; i<citys.size(); i++)
  {
    if (citys[i]->dwCountry==country && citys[i]->dwCityID==cityid)
    {
      pCity = citys[i];
      break;
    }
  }
  rwlock.unlock();

  return pCity;
}


CCity* CCityM::findByUnionID(DWORD unionid)
{
  CCity* pCity = NULL;

  if (!unionid)
    return NULL;

  rwlock.rdlock();
  for (DWORD i=0; i<citys.size(); i++)
  {
    if (citys[i]->dwUnionID==unionid)
    {
      pCity = citys[i];
      break;
    }
  }
  rwlock.unlock();

  return pCity;

}

CCity* CCityM::findDareUnionByID(DWORD unionid)
{
  CCity* pCity = NULL;

  if (!unionid)
    return NULL;

  rwlock.rdlock();
  for (DWORD i=0; i<citys.size(); i++)
  {
    if (citys[i]->isDare(unionid))
    {
      pCity = citys[i];
      break;
    }
  }
  rwlock.unlock();

  return pCity;
}

void CCityM::timer()
{
  struct tm tv1;
  time_t timValue = time(NULL);
  zRTime::getLocalTime(tv1,timValue);
  static int last_fulltime = 0;
  std::vector<CCity*>::iterator vIterator;
  for (DWORD i=0; i< citys.size(); i++)
  {
    //if (tv1.tm_hour >21 && tv1.tm_min>30)
    if ((timValue-citys[i]->isAward) > 24*60*60)
    {//�����Ǯ��ˢ�º�,��Ϊtrue
      if (citys[i]->dwCountry == PUBLIC_COUNTRY || citys[i]->dwCityID == KING_CITY_ID) 
      {
        citys[i]->dwGold = 50000; // ����5�� ��������5��
      }
      else
      {
        citys[i]->dwGold = 20000; // 2��
      }
      
      citys[i]->isAward = timValue;
      citys[i]->writeDatabase();
    }
  }

  if (tv1.tm_hour>last_fulltime && tv1.tm_min == 0)
  {// ���㷢��֪ͨ
    rwlock.rdlock();
    for(vIterator = citys.begin(); vIterator!=citys.end(); vIterator++)
    {
      if ((*vIterator)->dareSize()>0)
      {
        CUnionM::getMe().sendUnionNotify((*vIterator)->dwUnionID,
            "��ע��:19:30��,��ʼ�����ս");

        for (DareSet::iterator pos = (*vIterator)->vDareList.begin(); 
            pos!=(*vIterator)->vDareList.end(); ++pos)  
        {    
          CUnionM::getMe().sendUnionNotify(*pos,
              "��ע��:19:30��,��ʼ�����ս");
        }
      }
    }
    rwlock.unlock();

    last_fulltime = tv1.tm_hour;
  }

  if (tv1.tm_hour==19)
  {       
    if (!isBeging && tv1.tm_min>=30 && tv1.tm_min<33)
    {       
      this->beginDare();
      isBeging = true;
    }

    if (tv1.tm_min==20)
    {      
      rwlock.rdlock();  
      for(vIterator = citys.begin(); vIterator!=citys.end(); vIterator++)
      {
        if ((*vIterator)->dareSize()>0)
        {
          CUnionM::getMe().sendUnionNotify((*vIterator)->dwUnionID,
              "��ע��:10�ֺ�,��ʼ�����ս");
          
          for (DareSet::iterator pos = (*vIterator)->vDareList.begin(); 
              pos!=(*vIterator)->vDareList.end(); ++pos)  
          {    
            CUnionM::getMe().sendUnionNotify(*pos,
                "��ע��:10�ֺ�,��ʼ�����ս");
          }
        }
      }
      rwlock.unlock();
    }
  }

  if (tv1.tm_hour==20 && tv1.tm_min>=30 && tv1.tm_min<33)
  {       
    //��ս����
    //this->endDare();
    isBeging = false;
  }  
}

void   CCityM::awardTaxGold(UserSession *pUser)
{
  CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);

  if (pUnion && pUnion->master)
  {
    if (pUnion->master->byStatus)
    {
      if (pUnion->master->id != pUser->id)
      {
        if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"ֻ�а���������ȡ˰��");
        return;
      }
    }
    else
    {
      if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"ֻ�а���������ȡ˰��");
      return;
    }
  }
  else
  {
    if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"��᲻����");
    return;
  }
  
  CCity* pCity = NULL;
  pCity = this->findByUnionID(pUser->unionid);
  rwlock.wrlock();

  if (pCity)
  {
    if (pCity->dwGold>0)
    {
      Cmd::Session::t_dareGold_SceneSession send;
      send.dwUserID = pUser->id;
      send.dwNum = pCity->dwGold;
      send.dwWarID = 0;

      if (pUser->scene) 
      {
        pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_dareGold_SceneSession));
        Zebra::logger->info("��ɫ %s ��ȡ�˳���˰��%u��",pUser->name,pCity->dwGold);
      }

      if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"��ȡ˰�� %d ��",pCity->dwGold);
      pCity->dwGold = 0;
      pCity->writeDatabase();
    }
    else
    {
      if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����˰��������ȡ");
    }
  }
  else
  {
    if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"��ûӵ�г���,������ȡ˰��");
  }

  rwlock.unlock();
}

void CCityM::beginDare()
{
  std::vector<CCity*>::iterator vIterator;

  for(vIterator = citys.begin(); vIterator!=citys.end(); vIterator++)
  {
    if ((*vIterator)->dareSize() > 0)
    {
      (*vIterator)->beginDare();
    }
    else
    {
      Zebra::logger->info("[��ս]: (UNION_CITY_DARE,%d,%d) �����޶�ս",
          (*vIterator)->dwCountry,(*vIterator)->dwCityID);
    }
    
  }
}

void CCityM::endDare()
{
  std::vector<CCity*>::iterator vIterator;
  rwlock.rdlock();

  for (vIterator = citys.begin(); vIterator!=citys.end(); vIterator++)
  {
    if ((*vIterator)->dareSize() > 0)
    {
      (*vIterator)->endDare();
    }
  }
  rwlock.unlock();
}

void CCityM::execEveryCity(cityCallback& cb)
{
  std::vector<CCity*>::iterator vIterator;
  rwlock.rdlock();

  for (vIterator = citys.begin(); vIterator!=citys.end(); vIterator++)
  {
    cb.exec(*vIterator);
  }
  rwlock.unlock();

}

bool   CCityM::isCastellan(UserSession* pUser)
{
  bool ret = false;
  
  CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);
  if (pUnion)
  {
    // TODO:�ж��Ƿ��ǳ���
    if (pUnion->master && pUnion->master->id == pUser->id)  
    {//�ǰ���
      if (CCityM::getMe().findByUnionID(pUnion->id) !=NULL)
      {//�ǳ���
        ret = true;
      }
    }
  }
  
  return ret;
}

bool   CCityM::isCatcher(UserSession* pUser)
{
  for (DWORD i=0; i<CCityM::getMe().citys.size(); i++)
  {
    if (strncmp(CCityM::getMe().citys[i]->catcherName,pUser->name,MAX_NAMESIZE) == 0)
    {
      return true;
    }
  }

  return false;
}

void CCityM::refreshUnion(DWORD dwCountryID,DWORD dwCityID)
{
  Cmd::Session::t_updateSceneUnion_SceneSession send;
  SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByID((dwCountryID<<16)
      + dwCityID);   
  
  CCity* pCity = this->find(dwCountryID,dwCityID);
  if (pCity && pScene)
  {
    //if (pCity->dwUnionID>0)
    //{
      send.dwSceneID = pScene->id;
      send.dwUnionID = pCity->dwUnionID;
      pScene->sendCmd(&send,sizeof(send));
    //}
  }
}

//------------------------------------------------------------------------------------------------------------


void CCity::init(DBRecord* rec)
{
  char dare_list[256];
  char seps[]   = ",\t\n;";
  char *token;
  bzero(dare_list,sizeof(dare_list));
  
  if (rec)
  {
    this->dwCountry = rec->get("country");
    this->dwCityID  = rec->get("cityid");
    this->dwUnionID = rec->get("unionid");
    this->isAward   = rec->get("isaward");
    this->dwGold  = rec->get("gold");
    strncpy(this->catcherName,rec->get("catchername"),MAX_NAMESIZE);
    strncpy(dare_list,rec->get("dareunionlist"),sizeof(dare_list));
    token = strtok( dare_list,seps );
    while( token != NULL )
    {
      this->addDareList(atoi(token));
      token = strtok( NULL,seps );
    }
  }
}

/** 
 * \brief �������ݿ��¼
 */   
void CCity::writeDatabase()
{
  DBRecord rec,where;
  char dare_list[256];
  bzero(dare_list,sizeof(dare_list));

  std::ostringstream oss;
  oss << "country='" << this->dwCountry << "'";
  where.put("country",oss.str());

  oss.str("");
  oss << "cityid='" << this->dwCityID << "'";
  where.put("cityid",oss.str());

  rec.put("unionid",this->dwUnionID);
  rec.put("isaward",this->isAward);
  rec.put("gold",this->dwGold);
  rec.put("catchername",this->catcherName);
  oss.str("");
  
  for (DareSet::iterator pos=vDareList.begin();
      pos!=vDareList.end(); ++pos)
  {
    oss << *pos << ";";
  }

  strncpy(dare_list,oss.str().c_str(),sizeof(dare_list));
  rec.put("dareunionlist",dare_list);

  DBFieldSet* city = SessionService::metaData->getFields("CITY");

  if (city)       
  {       
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return;
    }

    if ((connHandleID)-1 != handle)
    {
      SessionService::dbConnPool->exeUpdate(handle,city,&rec,&where);
    }

    SessionService::dbConnPool->putHandle(handle);

  }
  else
  {
    Zebra::logger->error("CCity::writeDatabase �������ݼ���ʧ��,CITY������");
    return;
  }
}


bool CCity::insertDatabase()
{
  DBRecord rec;
  rec.put("country",this->dwCountry);
  rec.put("cityid",this->dwCityID);
  rec.put("unionid",this->dwUnionID);
  rec.put("isaward",0);
  rec.put("gold",50000);
  DBFieldSet* city = SessionService::metaData->getFields("CITY");

  if (city)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }
    else
    {
      SessionService::dbConnPool->exeInsert(handle,city,&rec);
    }

    SessionService::dbConnPool->putHandle(handle);
  }
  else
  {
    Zebra::logger->error("CCity::insertDatabase �������ݼ���ʧ��,CITY������");
    return false;
  }

  return true;
}

bool CCity::changeCatcher(UserSession* pUser)
{
  if (strncmp(pUser->name,this->catcherName,MAX_NAMESIZE) != 0)
  {
    UserSession *u = UserSessionManager::getInstance()->getUserSessionByName(this->catcherName);
    if (u && u->scene)
    {
      Cmd::Session::t_setCatcherState_SceneSession send;
      send.byState = 0;
      send.dwUserID = pUser->id;
      pUser->scene->sendCmd(&send,sizeof(send));
    }
    
    rwlock.wrlock();
    strncpy(this->catcherName,pUser->name,MAX_NAMESIZE);
    rwlock.unlock();

    Cmd::Session::t_setCatcherState_SceneSession send;
    if (pUser->scene) 
    {
      send.byState = 1;
      send.dwUserID = pUser->id;
      pUser->scene->sendCmd(&send,sizeof(send));

      pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"������������Ϊ��ͷ");
    }
  }

  return true;
}

bool CCity::cancelCatcher()
{
  UserSession *u = UserSessionManager::getInstance()->getUserSessionByName(this->catcherName);

  if (u && u->scene)
  {
    Cmd::Session::t_setCatcherState_SceneSession send;
    send.byState = 0;
    send.dwUserID = u->id;
    u->scene->sendCmd(&send,sizeof(send));
  }

  rwlock.wrlock();
  bzero(this->catcherName,MAX_NAMESIZE);
  rwlock.unlock();
  this->writeDatabase();

  return true;
}

bool CCity::changeUnion(DWORD unionid)
{
  CUnion* pSrcUnion = CUnionM::getMe().getUnionByID(this->dwUnionID);
  CUnion* pDestUnion = CUnionM::getMe().getUnionByID(unionid);

  SceneSession* pScene =pScene = SceneSessionManager::getInstance()->
    getSceneByID((this->dwCountry<<16)+this->dwCityID);


  rwlock.wrlock();

  this->dwUnionID = unionid;
  this->isAward = time(NULL);
  this->dwGold = 20000; // 2��
  this->writeDatabase();

  rwlock.unlock();

  if (pSrcUnion)  
  {
    pSrcUnion->update_all_data();
    if (pDestUnion) pSrcUnion->sendUnionNotify("��������Ȩ�ѱ� %s ���",pDestUnion->name);
  }
  
  if (pDestUnion) 
  {
    pDestUnion->update_all_data();
    pDestUnion->sendUnionNotify("����ѻ�ó�������Ȩ");

    if (pScene) SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,pDestUnion->dwCountryID,"%s ����� %s ����Ȩ",pDestUnion->name,pScene->name);
  }

  CCityM::getMe().refreshUnion(this->dwCountry,this->dwCityID);
  
  return true;
}

bool CCity::abandonCity()
{
  CUnion* pSrcUnion = CUnionM::getMe().getUnionByID(this->dwUnionID);

  rwlock.wrlock();
  this->dwUnionID = 0;
  this->isAward = time(NULL);
  if (this->dwCityID == KING_CITY_ID || this->dwCountry == PUBLIC_COUNTRY)
  {
    this->dwGold = 50000;
  }
  else
  {
    this->dwGold = 20000;
  }
  rwlock.unlock();

  this->writeDatabase();

  if (pSrcUnion)  
  {
    pSrcUnion->update_all_data();
    pSrcUnion->sendUnionNotify("��������Ȩ�ѱ�����");
  }
  
  SceneSession* pScene =pScene = SceneSessionManager::getInstance()->
    getSceneByID((this->dwCountry<<16)+this->dwCityID);

  if (pScene) SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,this->dwCountry,"%s ������ %s ����Ȩ",
      pSrcUnion->name,pScene->name);

  CCityM::getMe().refreshUnion(this->dwCountry,this->dwCityID);
  this->writeDatabase();
  return true;
}


bool CCity::addDareList(DWORD dwUnionID)
{
  bool ret = false;
  rwlock.wrlock();
  
  if (vDareList.find(dwUnionID) == vDareList.end())
        {
    vDareList.insert(dwUnionID);
    ret = true;
        }
  
  rwlock.unlock();
  return ret;
}

bool CCity::isDare(DWORD dwUnionID)
{
  if (vDareList.find(dwUnionID) != vDareList.end())
  {
    return true;
  }
  
  return false;
}

bool CCity::isMe(DWORD country,DWORD cityid,DWORD unionid)
{
  if (this->dwCountry==country && this->dwCityID==cityid && this->dwUnionID==unionid)
    return true;

  return false;
}

void CCity::beginDare()
{
  if (this->dareSize()>0)
  {
    CUnion* pDefUnion = CUnionM::getMe().getUnionByID(this->dwUnionID);
    std::vector<DWORD> dare_list;

    if (pDefUnion == NULL)
    {
      Zebra::logger->info("[��ս]: ����:%d ӵ�а���Ѳ�����,ȡ����ս",this->dwCityID);
      if (this->dareSize()>0)
      {
        DareSet::iterator pos= vDareList.begin();
        this->changeUnion(*pos);
      }
      
      this->endDare();
      return;
    }
    
    for (DareSet::iterator pos = vDareList.begin(); pos!=vDareList.end(); ++pos)
    {
      CUnion* pAttUnion = CUnionM::getMe().getUnionByID(*pos);
      if (pAttUnion)
      {
        dare_list.push_back(*pos);
        Zebra::logger->info("[��ս]: �����ս�� %s �� %s ֮�����",
            pAttUnion->name,pDefUnion->name);
        pAttUnion->sendUnionNotify("�����ս��ʼ,��ͨ��ӡ�ҹ���Ա����ӡ��");
      }
    }
    
    pDefUnion->sendUnionNotify("�����ս��ʼ,��ͨ��ӡ�ҹ���Ա����ӡ��");
    
    Cmd::Session::t_createDare_SceneSession pCmd;

    pCmd.active_time        =       3600;
    pCmd.ready_time =       1;      
    pCmd.relationID2        =       this->dwUnionID;
    pCmd.type               =       Cmd::UNION_CITY_DARE;

    CDareM::getMe().createDare_sceneSession(&pCmd,dare_list);
  }
}

void CCity::endDare()
{       
  vDareList.clear();
}

char* CCity::getName()
{
  DWORD mapid = (this->dwCountry<<16) + this->dwCityID;
  SceneSession *pScene = SceneSessionManager::getInstance()->getSceneByID(mapid);
  if (pScene)
    return pScene->name;
  
  return NULL;
}


