/**
 * \brief ������ѹ�ϵ������
 *
 */

#include <zebra/SessionServer.h>

/**
* \brief ��ϵ��Ա���캯��
*/
CRelation::CRelation()
{
  online = false;
}

/**
* \brief ����֪ͨ������,֪ͨ������ص�����ϵ�ֶ��Լ��ڳ�����д��Ӧ�Ĵ���
*/
void CRelation::sendNotifyToScene()
{
  //rwlock.rdlock();
  UserSession*    pUser = NULL;
        pUser = UserSessionManager::getInstance()->getUserByID(this->charid);
  
  if (pUser)
  {
    if (pUser->scene)
    {
      Cmd::Session::t_sendUserRelationID send;
      send.dwUserID = pUser->id;
      send.dwID = 0;
      send.type = Cmd::Session::RELATION_TYPE_NOTIFY;
      if (pUser->scene) pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
    }
  }

  //rwlock.unlock();
}

/**
* \brief ��Ա�Ƿ�����
* \return true ����  false ������
*/
bool CRelation::isOnline()
{
  UserSession*    pUser = NULL;
        pUser = UserSessionManager::getInstance()->getUserByID(this->id);
  return pUser==NULL?false:true;
}

/**
* \brief ��������
*/
CRelationManager::~CRelationManager()
{
  std::list<CRelation *> _deleteList;
  std::list<CRelation *>::iterator vIterator;

  CRelation *temp= NULL;
  //rwlock.wrlock();
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    temp = (CRelation *)it->second;
    if (temp)
    {
      //updateDBRecord(temp); //��������
      UserSession*  pUser = NULL;
      pUser = UserSessionManager::getInstance()->getUserByID(temp->id);

      if (temp->type!=3 && pUser && pUser->id)
      {
        if (temp->type!=3 && user && pUser)
          pUser->relationManager.offline(user->id,user->name);
      }

      _deleteList.push_back(temp);
    }
  }
  clear();

  for(vIterator = _deleteList.begin(); vIterator != _deleteList.end(); vIterator++)
  {
    SAFE_DELETE(*vIterator);
  }
  //rwlock.unlock();
}

/**
* \brief ���ù������û�,�򵥹�ϵ��������ӵ����
* \param pUser ������
*/
void CRelationManager::setUser(UserSession *pUser)
{
  //rwlock.wrlock();
  user = pUser;
  //rwlock.unlock();
}

/**
* \brief ��������ʼ��,�������������Ƿ����ڽ�ɫ���ߵ�ʱ������1�������ݿ�,2���͹�ϵ�б���ͻ���,3���ͺ�����������
*/
void CRelationManager::init()
{
  if (this->size() == 0)
  {
    loadFromDatabase();
    sendRelationList();
    sendAllBlackListToGateway();
  }
}

/**
* \brief ���ͼ�����ϵ�б�
*/
void CRelationManager::sendRelationList()
{
  BYTE buf[zSocket::MAX_DATASIZE];
  DWORD count;
  Cmd::stRelation *tempPoint;

  //rwlock.rdlock();
  Cmd::stSendRelationListCmd *retCmd=(Cmd::stSendRelationListCmd *)buf;
  constructInPlace(retCmd);
  tempPoint = (Cmd::stRelation *)retCmd->member;
  count = 0;
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CRelation *temp=(CRelation *)it->second;
    tempPoint->level = temp->level;

    if (temp->online)
    {
      UserSession *pUser = UserSessionManager::getInstance()->getUserByID(temp->id);
      if (pUser) 
      {
        tempPoint->user_level = pUser->level;
        tempPoint->exploit = pUser->dwExploit;
        tempPoint->country = pUser->country;
        CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);

        if (pUnion)
        {
          strncpy(tempPoint->unionName,pUnion->name,MAX_NAMESIZE);
        }
        else
        {
          bzero(tempPoint->unionName,MAX_NAMESIZE);
        }
      }
    }
    else
    {
      bzero(tempPoint->unionName,MAX_NAMESIZE);
      tempPoint->user_level = 0;
      tempPoint->exploit = 0;
      tempPoint->country = 0;
    }

    tempPoint->online = temp->isOnline();
    tempPoint->type = temp->type;
    tempPoint->occupation = temp->occupation;
    strncpy(tempPoint->name,temp->name,MAX_NAMESIZE);

    tempPoint++;
    count++;
    if (400==count)
    {
      goto breakRation; // ����¼����100��ʱ��ᳬ������͵��������
    }
  }
breakRation:
  //rwlock.unlock();
  retCmd->size = count;
  user->sendCmdToMe(retCmd,(count*sizeof(Cmd::stRelation)+sizeof(Cmd::stSendRelationListCmd)));
}

/**
* \brief ���ߴ���
* \param dwID ���߽�ɫ�� id
*/
void CRelationManager::online(const DWORD dwID)
{
  CRelation *relation;
  //rwlock.rdlock();
  relation = (CRelation *)getEntryByID(dwID);
  if (relation)
  {
    relation->online = true;
    UserSession *pUser = UserSessionManager::getInstance()->getUserByID(dwID);    

    if (pUser && relation->type!=Cmd::RELATION_TYPE_BAD)
    {
      if (relation->occupation != pUser->occupation)
      {
        relation->occupation = pUser->occupation;
        this->updateDBRecord(relation);
      }
        
      sendStatusChange(relation,Cmd::RELATION_ONLINE);

      if (this->user)
      {
        switch(relation->type)
        {
          case Cmd::RELATION_TYPE_LOVE:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"��ż��%s��������",pUser->name);
            }
            break;
          case Cmd::RELATION_TYPE_BAD:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"������%s��������",pUser->name);
            }
            break;
          case Cmd::RELATION_TYPE_FRIEND:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"���ѡ�%s��������",pUser->name);
            }
            break;
          case Cmd::RELATION_TYPE_ENEMY:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"���ˡ�%s��������",pUser->name);
            }
            break;
          default:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"%s��������",pUser->name);
            }
            break;
        }
      }
    }

  }
  //rwlock.unlock();
}

/**
* \brief ���ߴ���
* \param dwID ���߽�ɫ�� id
*/
void CRelationManager::offline(const DWORD dwID,const char* name)
{
  CRelation *relation = NULL;
  //rwlock.rdlock();

  relation = (CRelation *)getEntryByID(dwID);
  
  if (relation)
  {
    if (relation->type!=Cmd::RELATION_TYPE_BAD)
    {
      sendStatusChange(relation,Cmd::RELATION_OFFLINE);

      if (relation->online && this->user)
      {
        switch(relation->type)
        {
          case Cmd::RELATION_TYPE_LOVE:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"��ż��%s��������",name);
            }
            break;
          case Cmd::RELATION_TYPE_BAD:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"������%s��������",name);
            }
            break;
          case Cmd::RELATION_TYPE_FRIEND:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"���ѡ�%s��������",name);
            }
            break;
          case Cmd::RELATION_TYPE_ENEMY:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"���ˡ�%s��������",name);
            }
            break;
          default:
            {
              this->user->sendSysChat(Cmd::INFO_TYPE_SYS,"%s��������",name);
            }
            break;
        }
      }
    }

    relation->online = false;
  }
  
  
  //rwlock.unlock();
}

/**
* \brief �����ݿ���ر���ɫ���е�����ϵID
*/
void CRelationManager::loadFromDatabase()
{
  static const dbCol relation_define[] = {
    { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`RELATIONID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`RELATIONNAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { "`TYPE`",zDBConnPool::DB_BYTE,sizeof(BYTE) },
    { "`DEGREE`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "`LASTTIME`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`OCCUPATION`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { NULL,0,0}
  };
  struct {
    DWORD  dwCharID;            // ��Ա���
    DWORD  dwRelationID;          // ��ϵID
    char  relationName[MAX_NAMESIZE+1];  // ��ϵ����
    BYTE  byType;              // ��ϵ����
    WORD  wdDegree;                    // �Ѻö�
    DWORD  dwLastTime;            // ������ʱ��
    WORD  wdOccupation;          // ְҵ
  }
  * relationList,*tempPoint;
  
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  bzero(where,sizeof(where));
  
  _snprintf(where,sizeof(where) - 1,"CHARID = %u",user->id);
  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`SAMPLERELATION`",relation_define,where,NULL,(BYTE **)&relationList);
  SessionService::dbConnPool->putHandle(handle);
  
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("��ѯ����:%d,���ؼ�����ϵʧ�ܡ�",retcode);
    return;
  }

  if (relationList)
  {
    tempPoint = &relationList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      switch(tempPoint->byType)
      {
        case Cmd::RELATION_TYPE_LOVE:
        case Cmd::RELATION_TYPE_BAD:
        case Cmd::RELATION_TYPE_FRIEND:
        case Cmd::RELATION_TYPE_ENEMY:
          {
            UserSession *pUser = NULL;
            CRelation *relation = NULL;
            relation = new CRelation();
            if (relation)
            {
              pUser = UserSessionManager::getInstance()->getUserByID(tempPoint->dwRelationID);
              if (pUser)
              {
                //if (user && pUser->country != user->country)
                //{
                //  deleteDBRecord(tempPoint->dwRelationID);
                //}
                //else
                //{
                  relation->occupation = pUser->occupation;
                  relation->online = true;
                  pUser->relationManager.online(tempPoint->dwCharID);
                //}
              }
              else
              {
                relation->occupation = tempPoint->wdOccupation;
              }

              relation->charid = tempPoint->dwCharID;
              relation->id = tempPoint->dwRelationID;
              relation->type = tempPoint->byType;
              relation->level = tempPoint->wdDegree;
              relation->lasttime = tempPoint->dwLastTime;
              strncpy(relation->name,tempPoint->relationName,MAX_NAMESIZE);

              //rwlock.wrlock();
              if (!addEntry(relation))
              {
                Zebra::logger->error("���[%u:%s]����ϵ[%u:%s]���������ʧ��!",user->id,user->name,relation->id,relation->name);
              }
                

              // ��������ʱ�䳬��������Ĺ�ϵ�����Ѻöȿۼ�
              zRTime ctv;
              DWORD curTime = ctv.sec();

              if (curTime - relation->lasttime >= MAX_GROUP_TIME_GAP)
              {
                if (relation->level - DEDUCT_POINT >=0)
                  relation->level-=DEDUCT_POINT;
                else
                  relation->level = 0;

                relation->lasttime = curTime;

                UserSession *otherUser = NULL;
                otherUser = UserSessionManager::getInstance()->getUserByID(relation->id);
                if (otherUser)
                {
                  otherUser->relationManager.setFriendDegreeByOtherUser(user->id,relation->level,curTime);
                }
                else
                {
                  updateOtherOfflineUserDBRecord(relation);
                  this->updateDBRecord(relation);
                }
              }
              //rwlock.unlock();
              /////////////////////////////////////////////////
            }
            else
            {
              Zebra::logger->error("���ش�����װ�غ����б��ʱ���޷�������ڴ�");
            }
          }
          break;
        case Cmd::RELATION_TYPE_OVER:
          {
            deleteDBRecord(tempPoint->dwRelationID);
            user->sendSysChat(Cmd::INFO_TYPE_BREAKFRIEND,"%s�Ѿ�����������",tempPoint->relationName);
          }
          break;
        default:
          Zebra::logger->error("%s�ļ�����ϵ%s���Ͳ���ȷtype=%u",user->name,tempPoint->relationName,tempPoint->byType);
          break;

      }
      tempPoint++;
    }
    SAFE_DELETE_VEC(relationList);
  }
  return;
}

/**
* \brief  ɾ��ָ����ɫ�����ݿ��¼
* \param dwID ��ɾ���Ľ�ɫ��ID
*/
void CRelationManager::deleteDBRecord(const DWORD dwID)
{
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"CHARID = %u AND RELATIONID = %u ",user->id,dwID);
  DWORD retcode = SessionService::dbConnPool->exeDelete(handle,"`SAMPLERELATION`",where);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->debug("%sɾ����ϵ��¼ʧ�� %u",user->name,dwID);
    return;
  }
  return;
}

/**
* \brief ����һ���µļ�����ϵ��¼
* \param relation ������ϵ����
*/
bool CRelationManager::insertDBRecord(const CRelation *relation)
{
  static const dbCol relation_define[] = {
    { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`RELATIONID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`RELATIONNAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { "`TYPE`",zDBConnPool::DB_BYTE,sizeof(BYTE) },
    { "`DEGREE`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "`LASTTIME`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`OCCUPATION`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { NULL,0,0}
  };
  struct {
    DWORD  dwCharID;            // ��Ա���
    DWORD  dwRelationID;          // ��ϵID
    char  relationName[MAX_NAMESIZE+1];  // ��ϵ����
    BYTE  byType;              // ��ϵ����
    WORD  wdDegree;                    // �Ѻö�
    DWORD  dwLastTime;            // ������ʱ��
    WORD  wdOccupation;          // ְҵ
  }
  createrelation_data;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  //�������ݿ��ɫ��Ϣ
  bzero(&createrelation_data,sizeof(createrelation_data));

  //rwlock.rdlock();

  createrelation_data.dwCharID    = user->id;
  createrelation_data.dwRelationID  = relation->id;
  strncpy(createrelation_data.relationName,relation->name,MAX_NAMESIZE);
  createrelation_data.byType      = relation->type;
  createrelation_data.wdDegree    = relation->level;
  createrelation_data.dwLastTime    = relation->lasttime;
  createrelation_data.wdOccupation  = relation->occupation;

  //rwlock.unlock();

  DWORD retcode = SessionService::dbConnPool->exeInsert(handle,"`SAMPLERELATION`",relation_define,(const BYTE *)(&createrelation_data));
  SessionService::dbConnPool->putHandle(handle);

  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("���Ӻ��Ѽ�¼���ݿ����");
    return false;
  }
  
  return true;
}

/**
* \brief ���¼�����ϵ��¼
* \param relation ������ϵ����
*/
void CRelationManager::updateDBRecord(const CRelation *relation)
{
  static const dbCol relation_define[] = {
    { "`TYPE`",zDBConnPool::DB_BYTE,sizeof(BYTE) },
    { "`DEGREE`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "`LASTTIME`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`OCCUPATION`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { NULL,0,0}
  };
  struct {
    BYTE  byType;              // ��ϵ����
    WORD  wdDegree;                    // �Ѻö�
    DWORD  dwLastTime;            // ������ʱ��
    WORD  wdOccupation;          // ְҵ
  }
  update_data;
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  bzero(&update_data,sizeof(update_data));
  update_data.byType = relation->type;
  update_data.wdDegree  = relation->level;
  update_data.dwLastTime = relation->lasttime;
  update_data.wdOccupation = relation->occupation;
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"CHARID = %u AND RELATIONID = %u",user->id,relation->id);
  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`SAMPLERELATION`",relation_define,(BYTE*)(&update_data),where);
  SessionService::dbConnPool->putHandle(handle);

  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("CRelationManager::updateDBRecord() �޸ĺ��ѵ���ʧ��CHARID = %u RELATIONID = %u retcode =%u",user->id,relation->id,retcode);
  }
}

/**
* \brief ����ָ������ϵ��ɫ�ĶԶ˽�ɫ�����ݿ��¼
* \param relation ����ϵ����
*/
void CRelationManager::updateOtherOfflineUserDBRecord(const CRelation *relation)
{
  static const dbCol relation_define[] = {
    { "`TYPE`",zDBConnPool::DB_BYTE,sizeof(BYTE) },
    { "`DEGREE`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "`LASTTIME`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`OCCUPATION`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { NULL,0,0}
  };
  struct {
    BYTE  byType;              // ��ϵ����
    WORD  wdDegree;                    // �Ѻö�
    DWORD  dwLastTime;            // ������ʱ��
    WORD  wdOccupation;          // ְҵ
  }
  update_data;
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  bzero(&update_data,sizeof(update_data));
  update_data.byType = relation->type;
  update_data.wdDegree  = relation->level;
  update_data.dwLastTime = relation->lasttime;
  update_data.wdOccupation = user->occupation;
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"CHARID = %u AND RELATIONID = %u",relation->id,user->id);
  SessionService::dbConnPool->exeUpdate(handle,"`SAMPLERELATION`",relation_define,(BYTE*)(&update_data),where);
  SessionService::dbConnPool->putHandle(handle);
}

/**
* \brief д���߿���֪ͨ,�����ݿ��еĶԷ���¼д��RELATION_TYPE_OVER�´��û����ߵ�ʱ�򽫻����֪ͨ
* \param relation �����������ϵ����
*/
void CRelationManager::writeOfflineNotify(const CRelation *relation)
{
  static const dbCol relation_define[] = {
    { "`TYPE`",zDBConnPool::DB_BYTE,sizeof(BYTE) },
    { NULL,0,0}
  };
  BYTE  byType;
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  byType = Cmd::RELATION_TYPE_OVER;
  bzero(where,sizeof(where));
  //rwlock.rdlock();
  _snprintf(where,sizeof(where) - 1,"CHARID = %u AND RELATIONID = %u",relation->id,user->id);
  //rwlock.unlock();
  SessionService::dbConnPool->exeUpdate(handle,"`SAMPLERELATION`",relation_define,(BYTE*)(&byType),where);
  SessionService::dbConnPool->putHandle(handle);
}

/**
* \brief ����״̬�ı���Ϣ���ͻ���
* \param relation ����ϵ����
* \param byState ��ǰ״̬
*/
void CRelationManager::sendStatusChange(const CRelation *relation,const BYTE byState)
{
  Cmd::stRelationStatusCmd ret;

  ret.type = relation->type;
  strncpy(ret.name,relation->name,MAX_NAMESIZE);
  ret.level = relation->level;
  ret.byState = byState;
  ret.occupation = relation->occupation;
  ret.user_level = 0;
  bzero(ret.unionName,MAX_NAMESIZE);
  ret.exploit = 0;
  ret.country = 0;
  ret.user_level = 0;
  

  if (relation)  
  {
    UserSession *pUser = UserSessionManager::getInstance()->getUserByID(relation->id);
    if (pUser) 
    {
      ret.country = pUser->country;
      if (relation->online)
      {
        CUnion* pUnion = CUnionM::getMe().getUnionByID(pUser->unionid);

        if (pUnion)
        {
          strncpy(ret.unionName,pUnion->name,MAX_NAMESIZE);
        }

        ret.exploit = pUser->dwExploit;
        ret.user_level = pUser->level;
      }
    }
  }
    
  if (user)
  {
    user->sendCmdToMe(&ret,sizeof(Cmd::stRelationStatusCmd));
  }
}

/**
* \brief ɾ��������ϵ
* \param name ��ɾ���ߵ�����
*/
void CRelationManager::removeRelation(const char * name)
{
  CRelation *pRelation = NULL;

  //rwlock.rdlock();
  pRelation = (CRelation *)getEntryByName(name);
  //rwlock.unlock();
  if (pRelation)
  {
    //rwlock.wrlock();
    deleteDBRecord(pRelation->id);
    removeEntry(pRelation);
    //rwlock.unlock();
    sendStatusChange(pRelation,Cmd::RELATION_REMOVE);
    if (Cmd::RELATION_TYPE_BAD == pRelation->type) sendBlackListToGateway(name,Cmd::Session::BLACK_LIST_REMOVE);
    if (Cmd::RELATION_TYPE_BAD != pRelation->type) pRelation->sendNotifyToScene();
    SAFE_DELETE(pRelation);
  }
}


CRelation*  CRelationManager::getRelationByType(int relationType)
{
struct findall : public execEntry<CRelation>
  {
    CRelation* _pRelation;

	int _relationType;

	findall(int relationType):_relationType(relationType)
    {
      _pRelation = NULL;
    }

    ~findall(){}

    bool exec(CRelation* pRelation)
    {
      if (pRelation && pRelation->type == _relationType)
      {
        _pRelation = pRelation;
        return false;
      }

      return true;
    }
  };

  findall find_marry(relationType);
  execEveryOne(find_marry);

  return find_marry._pRelation;
}

CRelation* CRelationManager::getMarryRelation()
{
  struct findall : public execEntry<CRelation>
  {
    CRelation* _pRelation;

    findall()
    {
      _pRelation = NULL;
    }

    ~findall(){}

    bool exec(CRelation* pRelation)
    {
      if (pRelation && pRelation->type == Cmd::RELATION_TYPE_LOVE)
      {
        _pRelation = pRelation;
        return false;
      }

      return true;
    }
  };

  findall find_marry;
  execEveryOne(find_marry);

  return find_marry._pRelation;

}

/**
* \brief ����һ�����������͵Ĺ�ϵ
* \param name �Ϻ�������
*/
void CRelationManager::addBadRelation(const char *name)
{
  CRelation *relation = NULL;
  relation = (CRelation *)getEntryByName(name);

  if (relation)
  {
    if (Cmd::RELATION_TYPE_BAD == relation->type)
    {
      user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է��Ѿ��ں������б���");
    }
    else
    {
      if (Cmd::RELATION_TYPE_LOVE == relation->type)
      {
        user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����������ܽ�������������");
        return;
      }
      UserSession *otherUser = NULL;
      otherUser = UserSessionManager::getInstance()->getUserSessionByName(name);
      if (otherUser) 
      {
        otherUser->relationManager.removeRelation(user->name);
        otherUser->sendSysChat(Cmd::INFO_TYPE_BREAKFRIEND,"%sѡ�����������",user->name);
      }
      else
      {
        CRelation *relation = NULL;
        relation = (CRelation *)getEntryByName(name);
        if (relation) writeOfflineNotify(relation);
      }
      changeRelationType(name,Cmd::RELATION_TYPE_BAD);
    }
  }
  else
  {
    UserSession *otherUser = NULL;
    otherUser = UserSessionManager::getInstance()->getUserSessionByName(name);
    if (otherUser)
    {
      CRelation *relation = NULL;
      zRTime ctv;
      relation = new CRelation();
      if (relation)
      {
        relation->id = otherUser->id;
        relation->level = 0;
        strncpy(relation->name,otherUser->name,MAX_NAMESIZE);
        relation->type = Cmd::RELATION_TYPE_BAD;
        relation->lasttime = ctv.sec();
        relation->occupation = otherUser->occupation;
        //if (insertDBRecord(relation))
        //{
          //rwlock.wrlock();
          addEntry(relation);
          //rwlock.unlock();
          insertDBRecord(relation);

          sendStatusChange(relation,Cmd::RELATION_ADD);
          sendBlackListToGateway(name,Cmd::Session::BLACK_LIST_ADD);
        //}
        //else
        //{

        //  if (user) user->sendSysChat(Cmd::INFO_TYPE_GAME,"�� %s ���������ʧ��",name);
        //  SAFE_DELETE(relation);
        //}
      }

      if (user) user->sendSysChat(Cmd::INFO_TYPE_GAME,"�� %s �����˺�����",name);
    }
    else
    {
      if (user) user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�˲������޷�ȷ��");
    }
  }
}

/**
* \brief ����һ�����˵�����ϵ��
* \param name �ϳ���������
*/

/**
* \brief ����һ�����������͵Ĺ�ϵ
* \param name �Ϻ�������
*/
void CRelationManager::addEnemyRelation(const char *name)
{
  int iCount=0;
  CRelation *lastPoint = NULL;
  zRTime ctv;
  DWORD dwLastTime = ctv.sec();

  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CRelation *temp = (CRelation *)it->second;

    if (temp && (temp->type == Cmd::RELATION_TYPE_ENEMY))
    {
      iCount++;
      if (dwLastTime >= temp->lasttime)
      {
        dwLastTime = temp->lasttime;
        lastPoint = temp;
      }
    }
  }

  if (iCount>=5 && lastPoint)
  {
    this->deleteDBRecord(lastPoint->id);
    this->removeEntry(lastPoint);
  }

  CRelation *relation = NULL;
  relation = (CRelation *)getEntryByName(name);

  if (relation)
  {
  }
  else
  {
    UserSession *otherUser = NULL;
    otherUser = UserSessionManager::getInstance()->getUserSessionByName(name);
    if (otherUser)
    {
      CRelation *relation = NULL;
      zRTime ctv;
      relation = new CRelation();
      if (relation)
      {
        relation->id = otherUser->id;
        relation->level = 0;
        strncpy(relation->name,otherUser->name,MAX_NAMESIZE);
        relation->type = Cmd::RELATION_TYPE_ENEMY;
        relation->lasttime = ctv.sec();
        relation->occupation = otherUser->occupation;
        //rwlock.wrlock();
        addEntry(relation);
        //rwlock.unlock();
        insertDBRecord(relation);
        sendStatusChange(relation,Cmd::RELATION_ADD);
      }

      if (user) user->sendSysChat(Cmd::INFO_TYPE_GAME,"�� %s �����˳����б�",name);
    }
  }
}

/**
* \brief �ı�����ϵ����,����֪ͨ���������ݿ��¼
* \param name �Զ�����
* \param type ����ϵ����
*/
void CRelationManager::changeRelationType(const char * name,const BYTE type)
{
  CRelation *relation = NULL;
  relation = (CRelation *)getEntryByName(name);
  if (relation)
  {
    zRTime ctv;
    //rwlock.wrlock();
    if (Cmd::RELATION_TYPE_BAD == relation->type) 
    {
      sendBlackListToGateway(name,Cmd::Session::BLACK_LIST_REMOVE);
      relation->level = 0;
    }
    if (Cmd::RELATION_TYPE_BAD == type)
    {
      sendBlackListToGateway(name,Cmd::Session::BLACK_LIST_ADD);
      relation->level = 0;
      relation->lasttime = ctv.sec();
    }
    relation->type = type;
    relation->sendNotifyToScene();
    sendStatusChange(relation,Cmd::RELATION_TYPECHANGE);
    updateDBRecord(relation);
    //rwlock.unlock();
  }
}

/**
* \brief ����һ���µ�����ϵ����
* \param dwID �Զ˽�ɫid
* \param type ����ϵ����
*/
void CRelationManager::addRelation(const DWORD dwID,const BYTE type)
{
  UserSession *otherUser = NULL;

  otherUser = UserSessionManager::getInstance()->getUserByID(dwID);
  if (otherUser)
  {
    CRelation *relation = NULL;
    relation = (CRelation *)getEntryByID(dwID);
    if (relation)
    {
      if (relation->type == type)
      {
        user->sendSysChat(Cmd::INFO_TYPE_FAIL,"����Ѿ����б���");
      }
      else
      {
        changeRelationType(otherUser->name,type);
        relation->online = true;
      }
    }
    else
    {
      zRTime ctv;
      relation = new CRelation();
      if (relation)
      {
        relation->online = true;
        relation->id = otherUser->id;
        relation->level = 0;
        strncpy(relation->name,otherUser->name,MAX_NAMESIZE);
        relation->type = type;
        relation->lasttime = ctv.sec();
        relation->occupation = otherUser->occupation;

        //if (insertDBRecord(relation))
        //{
          //rwlock.wrlock();
          addEntry(relation);
          //rwlock.unlock();
          insertDBRecord(relation);
          sendStatusChange(relation,Cmd::RELATION_ADD);
          relation->sendNotifyToScene();
        //}
        //else
        //{
        //  SAFE_DELETE(relation);
        //}
      }
    }
  }
  else
  {
    if (user) user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�˲������޷�ȷ��");
  }
}

/**
* \brief ����Gatewayת�������Ŀͻ�����Ϣ
* \param pNullCmd ��Ϣ��
* \param cmdLen ��Ϣ����
* \return true �������,false ���ڴ���Χ֮��
*/
bool CRelationManager::processUserMessage(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  switch(pNullCmd->byCmd)
  {
    case Cmd::RELATION_USERCMD:
      {
        switch(pNullCmd->byParam)
        {
          case UNMARRY_PARA:
            {
              CRelation* relation = NULL;
              relation = getMarryRelation();

              if (relation)
              {
                UserSession *otherUser = NULL;
                otherUser = UserSessionManager::getInstance()->getUserSessionByName(relation->name);
                if (otherUser) 
                {
                  removeRelation(relation->name);
                  otherUser->relationManager.removeRelation(user->name);
                  otherUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"%sѡ��������",user->name);
                  otherUser->updateConsort();
                }
                else
                {
                  CRelation *tRelation = NULL;

                  tRelation = (CRelation *)getEntryByName(relation->name);
                  if (tRelation) writeOfflineNotify(tRelation);
                  removeRelation(relation->name);
                }
                user->updateConsort();
              }
            }
            break;
          case Cmd::RELATION_STATUS_PARA:
            {
              Cmd::stRelationStatusCmd *rev = (Cmd::stRelationStatusCmd *)pNullCmd;
              switch(rev->byState)
              {
                case Cmd::RELATION_ADD:
                  {
                    if (!strncmp(rev->name,user->name,MAX_NAMESIZE))
                    {
                      user->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ܰ��Լ�����������?");
                      return true;
                    }

                    if (300>size())
                    {
                      if (rev->type == Cmd::RELATION_TYPE_BAD)
                      {
                        addBadRelation(rev->name);
                      }
                      else
                      {
                        addEnemyRelation(rev->name);
                      }
                    }
                    else
                    {
                      user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����б�������");
                    }
                    return true;
                  }
                  break;
                case Cmd::RELATION_ANSWER_NO:
                  {
                    UserSession *otherUser = NULL;
                    otherUser = UserSessionManager::getInstance()->getUserByID(rev->userid);
                    if (otherUser)
                    {
                      switch(rev->type)
                      {
                        case Cmd::RELATION_TYPE_FRIEND:
                          {
                            otherUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է���ͬ�������Ϊ����");
                            return true;
                          }
                          break;
                        case Cmd::RELATION_TYPE_LOVE:
                          {
                            otherUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է���ͬ�������Ϊ����");
                          }
                          break;
                        default:
                          break;
                      }
                    }
                  }
                  break;
                case Cmd::RELATION_ANSWER_YES:
                  {
                    UserSession *otherUser = NULL;
                    otherUser = UserSessionManager::getInstance()->getUserByID(rev->userid);
                    if (otherUser)
                    {
                      if (300>otherUser->relationManager.size()|| rev->type == Cmd::RELATION_TYPE_LOVE)
                      {
                        if (300>size() || rev->type == Cmd::RELATION_TYPE_LOVE)
                        {
                          addRelation(rev->userid,rev->type);
                          otherUser->relationManager.addRelation(user->id,rev->type);
                          if (rev->type != Cmd::RELATION_TYPE_LOVE)
                          {
                            otherUser->sendSysChat(Cmd::INFO_TYPE_ADDFRIEND,"���� %s ������,��Ϊ����",user->name);
                            user->sendSysChat(Cmd::INFO_TYPE_ADDFRIEND,"���� %s ������,��Ϊ����",otherUser->name);
                          }
                        }
                        else
                        {
                          user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�ҵĺ����б�����");
                          otherUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է������б�����");
                        }
                      }
                      else
                      {
                        user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է������б�����");
                        otherUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ĺ����б�����");
                      }
                    }
                  }
                  break;
                case Cmd::RELATION_QUESTION:
                     //free ��ֹ���ѹ���
//                     user->sendSysChat(Cmd::INFO_TYPE_FAIL, "����ϵͳ���ڿ� ���У�");
//                                     break;
//#if 0                     
					{
                    if (!strncmp(rev->name,user->name,MAX_NAMESIZE))
                    {
                      user->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ڿ���Ц�𣿼��Լ�Ϊ���ѣ�");
                      return true;
                    }
                    CRelation *relation = NULL;
                    relation = (CRelation *)getEntryByName(rev->name);
                    if (relation)
                    {
                      if (!strncmp(rev->name,relation->name,MAX_NAMESIZE) && Cmd::RELATION_TYPE_BAD != relation->type)
                      {
                        user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է��Ѿ�����ĺ����б�����,��������ӣ�");
                        return true;
                      }
                    }

                    UserSession *otherUser = NULL;
                    otherUser = UserSessionManager::getInstance()->getUserSessionByName(rev->name);
                    if (otherUser)
                    {
                      if (isset_state(otherUser->sysSetting,Cmd::USER_SETTING_FRIEND))
                      {

                        user->sendSysChat(Cmd::INFO_TYPE_GAME,"���������ѷ���,�ȴ��Է�Ӧ��!");
                        rev->userid = user->id;
                        strncpy(rev->name,user->name,MAX_NAMESIZE);
                        otherUser->sendCmdToMe(rev,sizeof(Cmd::stRelationStatusCmd));
                      }
                      else
                        user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է���Ӻ���δ����");
                    }
                    else
                    {
                      user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է������߲�����Ӧ�������");
                    }
//#endif 
                  break;
					}
                case Cmd::RELATION_REMOVE:
                  {
                    CRelation *rel = NULL;
                    rel = (CRelation *)getEntryByName(rev->name);
                    if (!rel) return true;
                    int type = rel->type;
                
                    if (Cmd::RELATION_TYPE_BAD == type || Cmd::RELATION_TYPE_ENEMY == type)
                    {
                      removeRelation(rev->name); // ɾ����������Ա
                    }
                    else
                    {
                      if (Cmd::RELATION_TYPE_LOVE == type)
                      {
                        user->sendSysChat(Cmd::INFO_TYPE_FAIL,"����뵽����������ȥ�������������");
                        return true;
                      }
                      UserSession *otherUser = NULL;
                      otherUser = UserSessionManager::getInstance()->getUserSessionByName(rev->name);
                      if (otherUser) 
                      {
                        removeRelation(rev->name);
                        otherUser->relationManager.removeRelation(user->name);
                        otherUser->sendSysChat(Cmd::INFO_TYPE_BREAKFRIEND,"%sѡ�������ϯ�Ͻ�",user->name);
                        user->sendSysChat(Cmd::INFO_TYPE_BREAKFRIEND,"��ѡ���� %s ��ϯ�Ͻ�",otherUser->name);
                        otherUser->updateConsort();
                      }
                      else
                      {
                        user->sendSysChat(Cmd::INFO_TYPE_BREAKFRIEND,"��ѡ���� %s ��ϯ�Ͻ�",rev->name);
                        CRelation *relation = NULL;

                        relation = (CRelation *)getEntryByName(rev->name);
                        if (relation) writeOfflineNotify(relation);
                        removeRelation(rev->name);
                      }
                      user->updateConsort();
                    }
                  }
                  break;
              }
              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
  return false;
}

/**
* \brief ����������Ϣ���Լ������м�����ϵ,������������Ϊ������Ϣ
* \param pCmd ������Ϣ
* \param cmdLen ��Ϣ����
*/
void CRelationManager::sendChatToMyFriend(const Cmd::stChannelChatUserCmd *pCmd,const DWORD cmdLen)
{
  //rwlock.rdlock();
  user->sendCmdToMe(pCmd,cmdLen);  // ת��һ����Ϣ���Լ�,���⿴�����Լ��������¼
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CRelation *temp = (CRelation *)it->second;

    if (temp && (temp->type != Cmd::RELATION_TYPE_BAD)&& (temp->type != Cmd::RELATION_TYPE_ENEMY))
    {
      if (temp->online)
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserByID(temp->id);
        if (pUser) pUser->sendCmdToMe(pCmd,cmdLen);
      }
      else
      {
        if (Cmd::CHAT_TYPE_FRIEND_AFFICHE == pCmd->dwChannelID)
        {
          COfflineMessage::writeOfflineMessage(pCmd->dwType,user->id,pCmd,cmdLen);
        }
      }
    }
  }
  //rwlock.unlock();
}

/**
* \brief ������Ϣ���Լ������м�����ϵ
* \param pCmd ��Ϣ
* \param cmdLen ��Ϣ����
* \param sendMe �Ƿ񷢸��Լ�
*/
void CRelationManager::sendCmdToMyFriend(const void *pCmd,const DWORD cmdLen,bool sendMe)
{
  //rwlock.rdlock();
  if (sendMe)
    user->sendCmdToMe(pCmd,cmdLen);  // ת��һ����Ϣ���Լ�
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CRelation *temp = (CRelation *)it->second;

    if (temp && (temp->type != Cmd::RELATION_TYPE_BAD) && (temp->type != Cmd::RELATION_TYPE_ENEMY))
    {
      if (temp->online)
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserByID(temp->id);
        if (pUser)
        {
          pUser->sendCmdToMe(pCmd,cmdLen);
        }
      }
    }
  }
  //rwlock.unlock();
}

/**
* \brief ������Ϣ���Լ������м�����ϵ
* \param pCmd ��Ϣ
* \param cmdLen ��Ϣ����
* \param sendMe �Ƿ񷢸��Լ�
*/
void CRelationManager::sendCmdToMyFriendExcept(const void *pCmd,const DWORD cmdLen,bool sendMe,const char * except)
{
  //rwlock.rdlock();
  if (sendMe)
    user->sendCmdToMe(pCmd,cmdLen);  // ת��һ����Ϣ���Լ�
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CRelation *temp = (CRelation *)it->second;

    if (temp && (temp->type != Cmd::RELATION_TYPE_BAD) && (temp->type != Cmd::RELATION_TYPE_ENEMY))
    {
      if (temp->online)
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserByID(temp->id);
        if (pUser && strncmp(pUser->name,except,MAX_NAMESIZE))
        {
          pUser->sendCmdToMe(pCmd,cmdLen);
        }
      }
    }
  }
  //rwlock.unlock();
}

/**
* \brief ����˽����Ϣ������,����Է��������Ϊ������Ϣ
* \param  pCmd ������Ϣ
* \param cmdLen ��Ϣ����
* \return 
*/
void CRelationManager::sendPrivateChatToFriend(const Cmd::stChannelChatUserCmd *pCmd,const DWORD cmdLen)
{
  //rwlock.rdlock();

  CRelation *rel = (CRelation *)getEntryByName(pCmd->pstrName);
  if (rel)
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stChannelChatUserCmd *chatCmd;

    chatCmd = (Cmd::stChannelChatUserCmd *)buf;
    memccpy(chatCmd,pCmd,cmdLen,sizeof(buf));
    strncpy(chatCmd->pstrName,user->name,MAX_NAMESIZE);

    if (rel->isOnline())
    {
      UserSession *pUser = UserSessionManager::getInstance()->getUserByID(rel->id);
      if (pUser) pUser->sendCmdToMe(chatCmd,cmdLen);
    }
    else
    {
      COfflineMessage::writeOfflineMessage(chatCmd->dwType,rel->id,chatCmd,cmdLen);
    }
  }
  //rwlock.unlock();
}

/**
* \brief  ���ͺ�����������Ϣ��GateWay
* \param name �����Ľ�ɫ����
* \param oper �������� Cmd::Session::BLACK_LIST_ADD,Cmd::Session::BLACK_LIST_REMOVE
*/
void CRelationManager::sendBlackListToGateway(const char *name,const BYTE oper)
{
  if (user) 
  {
    Cmd::Session::t_Session_HandleBlackList send;
    strncpy(send.name,name,MAX_NAMESIZE);
    send.byOper = oper;
    send.dwID = user->id;
    user->sendCmd(&send,sizeof(Cmd::Session::t_Session_HandleBlackList));
  }
}

/**
* \brief  �������еĺ������б�Gateway
*/
void CRelationManager::sendAllBlackListToGateway()
{
  //rwlock.rdlock();
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CRelation *temp = (CRelation *)it->second;
    if (temp)
    {
      if (Cmd::RELATION_TYPE_BAD == temp->type)
      {
        sendBlackListToGateway(temp->name,Cmd::Session::BLACK_LIST_ADD);
      }
    }
  }
  //rwlock.unlock();
}

/**
* \brief ���ݶԶ˹�ϵ����ȡ��ϵ����
* \param name �Զ˹�ϵ��
* \return ������ϵ����
*/
CRelation * CRelationManager::getRelationByName(const char *name)
{
  return (CRelation *)getEntryByName(name);
}

/**
* \brief ���ݶԶ˹�ϵID��ȡ��ϵ����
* \param  dwRelationID �Զ˹�ϵID
* \return ������ϵ����
*/
CRelation* CRelationManager::getRelationByID(DWORD dwRelationID)
{
  return (CRelation*)getEntryByID(dwRelationID);
}

/**
* \brief �����Ѻö�
* \param rev �Ѻö�����������Ϣ
*/
void CRelationManager::setFriendDegree(Cmd::Session::t_CountFriendDegree_SceneSession *rev)
{
  //rwlock.wrlock();
  for (int i=0; i<rev->size; i++)
  {
    CRelation *rel = NULL;
    rel = (CRelation *)getEntryByID(rev->namelist[i].dwUserID);
    if (rel)
    {
#ifdef _DEBUG
      Zebra::logger->info("�����Ѻöȣ�����%s �� %s �� %u �Ѻö�Ϊ %d",this->user->name,rel->name,rel->type,rev->namelist[i].wdDegree);
#endif
      if (rel->type == rev->namelist[i].byType)
      {
        if (rel->level < rev->namelist[i].wdDegree)
        {
#ifdef _DEBUG
        Zebra::logger->info("�����Ѻöȣ��ɹ�����");
#endif
          UserSession *otherUser = NULL;
          zRTime ctv;

          rel->level = rev->namelist[i].wdDegree;
          rel->lasttime = ctv.sec();
          this->updateDBRecord(rel);
          otherUser = UserSessionManager::getInstance()->getUserByID(rel->id);
          if (otherUser) 
          {
            otherUser->relationManager.setFriendDegreeByOtherUser(user->id,rel->level,ctv.sec());
          }
          else
          {
            updateOtherOfflineUserDBRecord(rel);
          }
        }
      }
    }
  }
  //rwlock.unlock();
}

/**
* \brief ���öԶ��Ѻö�,��Ҫ������˫���Ѻö�ͬ��
* \param dwUserID �Զ˽�ɫid
* \param wdDegree �Ѻö�
* \param currTime ��ǰʱ��
*/
void CRelationManager::setFriendDegreeByOtherUser(const DWORD dwUserID,const WORD wdDegree,const DWORD currTime)
{
  CRelation *rel = NULL;

  //rwlock.wrlock();
  rel = (CRelation *)getEntryByID(dwUserID);
  if (rel)
  {
    if (rel->level < wdDegree)
    {
      rel->level = wdDegree;
      rel->lasttime = currTime;
      this->updateDBRecord(rel);
    }
  }
  //rwlock.unlock();
}

