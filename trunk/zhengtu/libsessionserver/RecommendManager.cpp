/**
 * \brief �Ƽ��˹�����ʵ��
 *
 * 
 */

#include <zebra/SessionServer.h>
#include <memory>

RecommendM::RecommendM()
{
}

RecommendM::~RecommendM()
{
}

bool RecommendM::init()
{
  return this->load();
}

void RecommendM::destroyMe()
{
  for (DWORD i=0; i<RecommendM::getMe().recommends.size(); i++)
  {       
    RecommendM::getMe().recommends[i]->writeDatabase();
  }

  delMe();
}

bool RecommendM::load()
{
  DBFieldSet* recommend = SessionService::metaData->getFields("RECOMMEND");
  
  if (recommend)
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
      recordset = SessionService::dbConnPool->exeSelect(handle,recommend,NULL,NULL);
    }

    SessionService::dbConnPool->putHandle(handle);

    if (recordset)
    {
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);
        Recommend* pRecommend = new Recommend();

        if (pRecommend)
        {
          pRecommend->init(rec);
          pRecommend->loadRecommendSubFromDB();
          this->addRecommend(pRecommend->id,pRecommend);
        }
      }

      SAFE_DELETE(recordset)
    }
  }
  else
  {
    Zebra::logger->error("�Ƽ������ݼ���ʧ�ܣ�RECOMMEND������");
    return false;
  }

  return true;
}

bool RecommendM::processUserMessage(UserSession *pUser,
    const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  switch (pNullCmd->byParam) 
  {
    case REQ_RECOMMENDED_BOUNTY_PARA:
      {
        Cmd::stReqRecommendedBountyUserCmd* rev = (Cmd::stReqRecommendedBountyUserCmd*)pNullCmd;
        
        if (rev->byState == Cmd::QUERY_BOUNTY)
        {
          RecommendSub* rs = this->findSubByID(pUser->id);
          if (rs && rs->myRecommend)
          {
            Cmd::stRtnRecommendedBountyUserCmd send;
            send.dwBalance = rs->queryBounty();
            send.dwTotal = rs->queryTotal();
            send.dwLastLevel = rs->lastLevel;
            strncpy(send.name,rs->myRecommend->name,MAX_NAMESIZE);
            pUser->sendCmdToMe(&send,sizeof(send));
          }
        }
        else if (rev->byState == Cmd::GET_BOUNTY)
        {
          RecommendSub* rs = this->findSubByID(pUser->id);
          if (rs)
          {
            rs->pickupBounty(pUser);
          }
        }
          
        return true;
      }
      break;
    case REQ_RECOMMEND_BOUNTY_PARA:
      {
        Cmd::stReqRecommendBountyUserCmd* rev = (Cmd::stReqRecommendBountyUserCmd*)pNullCmd;

        if (rev->byState == Cmd::QUERY_BOUNTY)
        {
          Recommend* r = this->findByID(pUser->id);
          if (r)
          {
            r->processQuery(pUser);
          }
        }
        else if (rev->byState == Cmd::GET_BOUNTY)
        {
          Recommend* r = this->findByID(pUser->id);
          if (r)
          {
            r->pickupBounty(pUser);
          }
        }

        return true;
      }
      break;
    case SET_RECOMMEND_PARA:
      {
        Cmd::Record::t_chkUserExist_SessionRecord send;
        Cmd::stSetRecommendUserCmd* rev = (Cmd::stSetRecommendUserCmd*)pNullCmd;
        
        if (strchr(rev->name,'\'')
            || strchr(rev->name,';')
            || strchr(rev->name,'\"'))
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��������ȷ�Ľ�ɫ����");
          return true;
        }

        if (strncmp(pUser->name,rev->name,MAX_NAMESIZE) == 0)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���������Լ�Ϊ�Ƽ���");
          return true;
        }
        
        if (/*pUser->level>=10 &&*/ pUser->level<=15)
        {
          if (this->findSubByID(pUser->id) == NULL)
          {
            strncpy(send.name,rev->name,MAX_NAMESIZE);
            send.from_id = pUser->id;
            recordClient->sendCmd(&send,sizeof(send));
          }
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Ѿ������Ƽ���.");
          }
        }
        else
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"ֻ����15������ʱ,�����趨�Ƽ���");
        }
        
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

void RecommendM::processAddRecommended(const Cmd::Record::t_chkUserExist_SessionRecord* cmd)
{
  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(cmd->from_id);
  bool ret = false;
  
  if (!pUser)
    return;
  
  if (pUser && cmd->user_id && cmd->user_level>=30)
  {// ���Ƽ�������,�Ƽ�����Ч,���Ҵ���30��
    Recommend* pRecommend = findByID(cmd->user_id);

    if (pRecommend)
    {// ��������Ƽ��˼�¼
      ret = pRecommend->addRecommended(cmd->from_id);
    }
    else
    {// ����Ƽ��˼�¼Ϊ��
      pRecommend = new Recommend();
      pRecommend->id = cmd->user_id;
      strncpy(pRecommend->name,cmd->name,MAX_NAMESIZE);
      pRecommend->dwBalance = 0;
      if (pRecommend->insertDatabase())
      {
        this->addRecommend(pRecommend->id,pRecommend);    
        ret = pRecommend->addRecommended(cmd->from_id);
      }
      else
        SAFE_DELETE(pRecommend);
    }
  }

  if (ret)
  {
    UserSession* pRecommendUser = UserSessionManager::getInstance()->getUserByID(cmd->user_id);
    if (pRecommendUser)
      pRecommendUser->sendSysChat(Cmd::INFO_TYPE_GAME,"%s �Ѱ�����Ϊ�Ƽ���",pUser->name);

    pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����Ƽ��˳ɹ�");
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����Ƽ���ʧ��,�����Ƽ���������д�Ƿ���ȷ");
  }
}

bool RecommendM::processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
  switch (pNullCmd->para)        
  {                               
    default:
      break;
  }

  return false;
}

Recommend* RecommendM::findByID(DWORD dwUserID)
{
  Recommend *pRecommend = NULL; 
  recommendIter iter;

  iter = recommends.find(dwUserID);
  if (iter != recommends.end()) pRecommend = (*iter).second;

  return pRecommend;
}

RecommendSub* RecommendM::findSubByID(DWORD dwUserID)
{
  RecommendSub *pRecommendSub = NULL; 
  resubIter iter;

  iter = recommendsubIndex.find(dwUserID);
  if (iter != recommendsubIndex.end()) pRecommendSub = (*iter).second;

  return pRecommendSub;
}

bool RecommendM::addRecommend(DWORD dwUserID,Recommend* r)
{
  std::pair<std::map<DWORD,Recommend*>::iterator,bool> retval;
  retval = recommends.insert(recommendValueType(dwUserID,r));
  return retval.second;
}

bool RecommendM::addRecommendSub(DWORD dwUserID,RecommendSub* rs)
{
  std::pair<std::map<DWORD,RecommendSub*>::iterator,bool> retval;
  retval = recommendsubIndex.insert(recommendsubIndexValueType(dwUserID,rs));
  return retval.second;
}

bool RecommendM::removeRecommendSub(DWORD dwUserID)
{
  bool ret;
  resubIter  iter;
  ret = true;
  iter = recommendsubIndex.find(dwUserID);
  if (iter != recommendsubIndex.end()) recommendsubIndex.erase(iter);
  else ret = false;

  return ret;
}

void RecommendM::fireRecommendSub(DWORD dwUserID)
{
  if (this->findByID(dwUserID) == NULL && this->findSubByID(dwUserID) != NULL)
  {   
    RecommendSub* rs = findSubByID(dwUserID);
    if (rs && rs->myRecommend)
    {
      rs->myRecommend->rmRecommended(dwUserID); 
    }
  }
}

//------------------------Recommend--------------------------------------------
Recommend::Recommend()
{
  this->id = 0;
  bzero(this->name,sizeof(this->name));
  this->dwBalance = 0;
  this->dwTotal = 0;
}

Recommend::~Recommend()
{
  for (std::vector<RecommendSub*>::iterator pos = subs.begin(); pos!=subs.end(); pos++)
  {
    SAFE_DELETE(*pos);
  }
}

void Recommend::init(DBRecord* rec)
{
  this->id = rec->get("id");
  strncpy(this->name,rec->get("name"),sizeof(this->name));
  this->dwBalance = rec->get("balance");
  this->dwTotal = rec->get("total");
}

void Recommend::writeDatabase()
{
  DBRecord rec,where;
  std::ostringstream oss;
    
  oss << "id='" << this->id << "'";
  where.put("id",oss.str());
  
  rec.put("balance",this->dwBalance);
  rec.put("total",this->dwTotal);

  DBFieldSet* recommend = SessionService::metaData->getFields("RECOMMEND");

  if (recommend)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();
    
    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return;
    }

    if ((connHandleID)-1 != handle)
    {
      SessionService::dbConnPool->exeUpdate(handle,recommend,&rec,&where);
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
    Zebra::logger->error("�Ƽ������ݱ���ʧ�ܣ�RECOMMEND������");
    return;
  }  

}

bool Recommend::insertDatabase()
{
  DBRecord rec;
  rec.put("id",this->id);
  rec.put("name",this->name);
  rec.put("balance",this->dwBalance);
  rec.put("total",this->dwTotal);

  DBFieldSet* recommend = SessionService::metaData->getFields("RECOMMEND");

  if (recommend)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {       
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }
    else
    {       
      DWORD retcode = SessionService::dbConnPool->exeInsert(handle,recommend,&rec);
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
    Zebra::logger->error("����Ƽ��˼�¼ʧ�ܣ�RECOMMEND������");
    return false;
  }

  return true;
}

bool Recommend::deleteMeFromDB()
{
  DBRecord where;
  std::ostringstream oss;
  oss << "id='" << this->id << "'";
  where.put("id",oss.str());
  
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DBFieldSet* recommend = SessionService::metaData->getFields("RECOMMEND");
  
  if (recommend)
  {
    SessionService::dbConnPool->exeDelete(handle,recommend,&where);
  }

  SessionService::dbConnPool->putHandle(handle);

  for (std::vector<RecommendSub*>::iterator vIter=subs.begin(); vIter!=subs.end(); vIter++)
  {
    RecommendSub* temp = *vIter;
    temp->fireMe();
    SAFE_DELETE(temp);
  }
  subs.clear();
  
  return true;
}

bool Recommend::loadRecommendSubFromDB()
{
  DBFieldSet* recommendsub = SessionService::metaData->getFields("RECOMMENDSUB");
  
  if (recommendsub)
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

    oss << "recommendid='" << this->id << "'";
    where.put("recommendid",oss.str());

    if ((connHandleID)-1 != handle)
    {
      recordset = SessionService::dbConnPool->exeSelect(handle,recommendsub,NULL,&where);
    }

    SessionService::dbConnPool->putHandle(handle);

    if (recordset)
    {
      for (DWORD i=0; i<recordset->size(); i++)
      {
        DBRecord* rec = recordset->get(i);
        RecommendSub* pRecommendSub = new RecommendSub();

        if (pRecommendSub)
        {
          pRecommendSub->init(rec);
          pRecommendSub->myRecommend = this;
          RecommendM::getMe().addRecommendSub(pRecommendSub->id,pRecommendSub);      
          subs.push_back(pRecommendSub);
        }
      }

      SAFE_DELETE(recordset)
    }
  }
  else
  {
    Zebra::logger->error("���Ƽ�����ϸ���ݼ���ʧ�ܣ�RECOMMENDSUB������");
    return false;
  }

  return true;
}

void Recommend::rmRecommended(DWORD dwUserID)
{
  for (std::vector<RecommendSub*>::iterator pos = subs.begin(); pos!=subs.end();)
  {
    RecommendSub* temp = (RecommendSub*)*pos;
    if (temp->id == dwUserID)
    {
      if (temp->fireMe())
      {
        subs.erase(pos);
        SAFE_DELETE(temp);
      }
      break;
    }
    else
      pos++;
  }
}

bool Recommend::addRecommended(DWORD dwUserID)
{
  for (std::vector<RecommendSub*>::iterator pos = subs.begin(); pos!=subs.end(); pos++)
  {
    RecommendSub* temp = (RecommendSub*)*pos;
    if (temp->id == dwUserID)
      return false;
  }

  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(dwUserID);

  if (pUser)
  {
    RecommendSub* pRS = new RecommendSub();
    pRS->id = pUser->id;
    pRS->lastLevel = 0;
    pRS->recommendid = this->id;
    pRS->dwTotal = 0;
    strncpy(pRS->name,pUser->name,MAX_NAMESIZE);

    if (pRS->insertDatabase())
    {
      pRS->myRecommend = this;
      subs.push_back(pRS);
      RecommendM::getMe().addRecommendSub(pRS->id,pRS);
      return true;
    }
  }
  
  return  false;
}

void Recommend::processQuery(UserSession* pUser)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::stRecommendItem *tempPoint;
  Cmd::stRtnRecommendBountyUserCmd *retCmd=(Cmd::stRtnRecommendBountyUserCmd *)buf;
  constructInPlace(retCmd);
  tempPoint = (Cmd::stRecommendItem *)retCmd->data;

  for (DWORD i=0; i<subs.size(); i++)
  {
    RecommendSub* rs = subs[i];
    if (rs)
    {
      strncpy(tempPoint->name,rs->name,MAX_NAMESIZE);
      tempPoint->dwLevel = rs->lastLevel;
      tempPoint->dwTotal = rs->dwTotal * 5;
      tempPoint++;
      retCmd->dwSize++;
    }
  }
      
  retCmd->dwBalance = this->queryBounty();
  retCmd->dwTotal = this->queryTotal();
  pUser->sendCmdToMe(retCmd,sizeof(Cmd::stRtnRecommendBountyUserCmd)+(retCmd->dwSize*sizeof(Cmd::stRecommendItem)));
}

void Recommend::pickupBounty(UserSession* pUser)
{
  if (pUser && pUser->scene && this->dwBalance>0)
  {
    Cmd::Session::t_PickupRecommend_SceneSession send;
    send.dwUserID = pUser->id;
    send.dwMoney = this->dwBalance;
    send.byType = 0;
    pUser->scene->sendCmd(&send,sizeof(send));
    Zebra::logger->info("[�Ƽ���]: %s ��ȡ���Ƽ��˽�����:%u ��",pUser->name,this->dwBalance);
    this->dwBalance = 0;
    this->dwTotal += send.dwMoney;
    this->writeDatabase();
  }
}

//------------------------RecommendSub--------------------------------------------
RecommendSub::RecommendSub()
{
  bzero(this->name,sizeof(this->name));
  this->id =       0;
  this->lastLevel =   0;
  this->recommendid = 0;
  this->myRecommend =  NULL;
  this->dwTotal = 0;
}

RecommendSub::~RecommendSub()
{
}

void RecommendSub::init(DBRecord* rec)
{
  this->id = rec->get("id");
  this->recommendid = rec->get("recommendid");
  this->lastLevel = rec->get("lastlevel");
  strncpy(this->name,rec->get("name"),MAX_NAMESIZE);
  this->dwTotal = rec->get("total");
}

void RecommendSub::writeDatabase()
{
  DBRecord rec,where;
  std::ostringstream oss;

  oss << "id='" << this->id << "'";
  where.put("id",oss.str());
  
  rec.put("lastlevel",this->lastLevel);
  rec.put("total",this->dwTotal);
  
  DBFieldSet* recommendsub = SessionService::metaData->getFields("RECOMMENDSUB");

  if (recommendsub)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();
    
    if ((connHandleID)-1 == handle)
    {
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return;
    }
    else
    {
      SessionService::dbConnPool->exeUpdate(handle,recommendsub,&rec,&where);
      SessionService::dbConnPool->putHandle(handle);
    }
  }
  else
  {
    Zebra::logger->error("���Ƽ�����ϸ���ݱ���ʧ�ܣ�RECOMMENDSUB������");
    return;
  }  

}

bool RecommendSub::insertDatabase()
{
  DBRecord rec;
  rec.put("id",this->id);
  rec.put("name",this->name);
  rec.put("lastLevel",this->lastLevel);
  rec.put("recommendid",this->recommendid);
  rec.put("total",this->dwTotal);

  DBFieldSet* recommendsub = SessionService::metaData->getFields("RECOMMENDSUB");

  if (recommendsub)
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {       
      Zebra::logger->error("���ܻ�ȡ���ݿ���");
      return false;
    }
    else
    {       
      DWORD retcode = SessionService::dbConnPool->exeInsert(handle,recommendsub,&rec);
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
    Zebra::logger->error("��ӱ��Ƽ��˼�¼ʧ�ܣ�RECOMMENDSUB������");
    return false;
  }

  return true;
}

bool RecommendSub::deleteMeFromDB()
{
  DBRecord where;
  std::ostringstream oss;

  oss << "id='" << this->id << "'";
  where.put("id",oss.str());
      
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DBFieldSet* recommendsub = SessionService::metaData->getFields("recommendsub");
    
  if (recommendsub)
  {
    SessionService::dbConnPool->exeDelete(handle,recommendsub,&where);
  }

  SessionService::dbConnPool->putHandle(handle);

  return true;
}

bool RecommendSub::fireMe()
{
  if (this->deleteMeFromDB())
  {
    return RecommendM::getMe().removeRecommendSub(this->id);
  }

  return false;
}

void RecommendSub::pickupBounty(UserSession* pUser)
{
  // ���Ƽ�����ȡһ��,����������Ƽ���,����һ�ʽ���
  if (pUser && pUser->scene && this->lastLevel!=pUser->level)
  {
    Cmd::Session::t_PickupRecommend_SceneSession send;
    send.dwUserID = pUser->id;
    send.dwMoney = this->queryBounty();
    
    if (send.dwMoney)
    {
      send.byType = 1;
      pUser->scene->sendCmd(&send,sizeof(send));
      Zebra::logger->info("[�Ƽ���]: %s(%u,%u) ��ȡ�˱��Ƽ��˽�����:%u ��",
          pUser->name,pUser->level,this->lastLevel,send.dwMoney);

      this->lastLevel = pUser->level;
      this->dwTotal += send.dwMoney;

      if (myRecommend) 
      {
        myRecommend->dwBalance += send.dwMoney*5;
        myRecommend->writeDatabase();
      }
      this->writeDatabase();
    }
  }
}

DWORD RecommendSub::queryBounty()
{
  double money = 0.0;
  float  coef = 0.5;

  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(this->id);
  if (!pUser) return (DWORD)money;
  
  if (pUser->level<10)
    return 0;

  // ����ϵ��
  if (pUser->level>30 && this->lastLevel<=30)
  {
    coef = 0.5;
  }
  else if (pUser->level>30 && this->lastLevel>30)
  {
    coef = 1.0;
  }

  if (this->lastLevel==0)
  {
    money = (0.2 * (pUser->level*pUser->level - 19*pUser->level + 90) * 100) * coef;

    if (money<0)
      money = 0;
  }
  else
  {
    money = (0.2 * (pUser->level*pUser->level - 19*pUser->level - 
          this->lastLevel*this->lastLevel + 19*this->lastLevel) * 100) * coef;

    if (money<0)
      money = 0;
  }

  return (DWORD)money;
}

