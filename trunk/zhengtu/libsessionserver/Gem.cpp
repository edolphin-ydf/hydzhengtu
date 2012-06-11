/**
 * \brief ʵ�ֻ�������
 *
 */

#include <zebra/SessionServer.h>

const DWORD GEM_ACTIVE_TIME = 4*60*60; // �����������ʱ��

/// ״̬����
char str_gem_state[][20]={"GEM_READY","GEM_ACTIVE","GEM_READY_OVER","GEM_OVER"};

bool CArhat::refreshNPC()
{
  Cmd::Session::t_SummonGemNPC_SceneSession send;
  send.dwMapID = (this->dwCountryID<<16) + this->dwMapRealID;
  send.x = this->x;
  send.y = this->y;
  send.dwBossID = this->dwID;
  SceneSession *pScene = SceneSessionManager::getInstance()->getSceneByID(send.dwMapID);
  
  if (pScene)
  {
    pScene->sendCmd(&send,sizeof(send));
    this->dwHoldUserID = 0;
    this->byState = 0;
      
    if (this->dwID==1000)
    {
      SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
        this->dwCountryID,"�����޺������� %s ��ͼ %d,%d �ص�",
        pScene->name,
        this->x,this->y);
#ifdef _DEBUG
  Zebra::logger->debug("[��������]: �����޺�ˢ�µص�: country:%d mapid:%d x:%d y:%d",
      this->dwCountryID,this->dwMapRealID,this->x,this->y);
#endif  

    }
    else
    {
      SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
        this->dwCountryID,"�����޺������� %s ��ͼ %d,%d �ص�",
        pScene->name,
        this->x,this->y);
#ifdef _DEBUG
  Zebra::logger->debug("[��������]: �����޺�ˢ�µص�: country:%d mapid:%d x:%d y:%d",
      this->dwCountryID,this->dwMapRealID,this->x,this->y);
#endif  
    }

    return true;
  }
  else
  {
    Zebra::logger->error("[����]��δ�ҵ��޺�������ͼ %d,%d",this->dwCountryID,this->dwMapRealID);
  }

  return false;
}

bool CArhat::clearNPC()
{
  Cmd::Session::t_ClearGemNPC_SceneSession send;
  send.dwMapID = (this->dwCountryID<<16) + this->dwMapRealID;
  send.dwBossID = this->dwID;
  SceneSession *pScene = SceneSessionManager::getInstance()->getSceneByID(send.dwMapID);
  
  if (pScene)
  {
    pScene->sendCmd(&send,sizeof(send));
    return true;
  }

  return false;
}

CGem::CGem()
{
  tmStart = time(NULL);
}

CGem::~CGem()
{
}

void CGem::timer()
{
}

void CGem::printState()
{
  Zebra::logger->info("[��������]:%d(%s)",this->dwCountryID,str_gem_state[this->state]);
}

void CGem::setReadyState()
{//׼���ڣ�ѡȡ�������������ڵ�ͼ������,
  rwlock.wrlock();
  this->state = CGem::GEM_READY;
  rwlock.unlock();
  this->printState();

  // ѡȡ�����޺����ֵص�
  int point_index = randBetween(0,mappoint_num-1);
  this->dragon.dwMapRealID = CGemM::getMe().wait_point[point_index].dwMapID; 
  this->dragon.x = CGemM::getMe().wait_point[point_index].x;
  this->dragon.y = CGemM::getMe().wait_point[point_index].y;
  this->dragon.dwID = 1000;
  this->dragon.dwCountryID = this->dwCountryID;


  point_index = randBetween(0,mappoint_num-1);

  while (CGemM::getMe().wait_point[point_index].dwMapID == this->dragon.dwMapRealID)
  {
    point_index = randBetween(0,mappoint_num-1);
  }

  this->tiger.dwMapRealID = CGemM::getMe().wait_point[point_index].dwMapID; 
  this->tiger.x = CGemM::getMe().wait_point[point_index].x;
  this->tiger.y = CGemM::getMe().wait_point[point_index].y;
  this->tiger.dwID = 1001;
  this->tiger.dwCountryID = this->dwCountryID;
  

  this->setActiveState();
}

void CGem::setActiveState()
{
  rwlock.wrlock();
  this->state = CGem::GEM_ACTIVE;
  rwlock.unlock();
  
  this->dragon.refreshNPC();
  this->tiger.refreshNPC();
    
  this->printState();
}

void CGem::setReadyOverState()
{//�����������״̬������װ��
  rwlock.wrlock();
  this->state = CGem::GEM_READY_OVER;
  rwlock.unlock();

  this->printState();
  
  Cmd::Session::t_BlastGemNPC_SceneSession send;

  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(this->dragon.dwHoldUserID);
  if (pUser)
  {
    send.dwUserID = pUser->id;
    send.dwBossID = 1002; // ����NPC
    if (pUser->scene) pUser->scene->sendCmd(&send,sizeof(send));
    pUser = NULL;
  }
  
  pUser = UserSessionManager::getInstance()->getUserByID(this->tiger.dwHoldUserID);
  if (pUser)
  {
    send.dwUserID = pUser->id;
    send.dwBossID = 1003; // ����NPC
    if (pUser->scene) pUser->scene->sendCmd(&send,sizeof(send));
    pUser = NULL;
  }

  this->setOverState();
}

void CGem::setOverState()
{
  rwlock.wrlock();
  this->state = CGem::GEM_OVER;
  rwlock.unlock();
  this->printState();
}

void CGem::sendActiveStateToScene(UserSession* pUser)
{
}

bool CGem::isActivePeriod()
{
  time_t cur_time = time(NULL);
  
  if (::abs(cur_time - tmStart) > 4*60*60)
  {
    return false;
  }

  return true;
}

void    CGem::holdDragon(UserSession* pUser)
{
  if (pUser->scene)       
  {
    Cmd::Session::t_SetGemState_SceneSession send;
    send.dwUserID = pUser->id;
    send.dwState = 1;

    pUser->scene->sendCmd(&send,sizeof(send));
    rwlock.wrlock();
    this->dragon.byState = 1;
    this->dragon.dwHoldUserID = pUser->id;
    rwlock.unlock();
    
    this->dragon.clearNPC();

    SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
      this->dwCountryID,"���� �Ѹ����� %s ����",pUser->name);
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ȡ����ʧ�ܡ�");
  }
}

void    CGem::holdTiger(UserSession* pUser)
{
  if (pUser->scene)
  {
    Cmd::Session::t_SetGemState_SceneSession send;
    send.dwUserID = pUser->id;
    send.dwState = 2;

    pUser->scene->sendCmd(&send,sizeof(send));
    
    rwlock.wrlock();
    this->tiger.byState = 1;
    this->tiger.dwHoldUserID = pUser->id;
    rwlock.unlock();
    this->tiger.clearNPC();

    SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
      this->dwCountryID,"���� �Ѹ����� %s ����",pUser->name);
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ȡ����ʧ�ܡ�");
  }
}

void    CGem::resetDragon()
{
  rwlock.wrlock();
  this->dragon.byState = 0;
  this->dragon.dwHoldUserID = 0;
  rwlock.unlock();
    
  SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
      this->dwCountryID,"���������ٶȻص��޺�����");

    
  this->dragon.refreshNPC();
}

void    CGem::resetTiger()
{
  rwlock.wrlock();
  this->tiger.byState = 0;
  this->tiger.dwHoldUserID = 0;
  rwlock.unlock();
  
  SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_GAME,
      this->dwCountryID,"���Ǹ����ٶȻص��޺�����");

  this->tiger.refreshNPC();
}

//-------------------------------------------------------------------------------------
CGemM::CGemM()
{
  channelUniqeID = new zUniqueDWORDID(1000);
}

CGemM::~CGemM()
{
  rwlock.wrlock();
  for(zEntryTempID::hashmap::iterator it=zEntryTempID::ets.begin();it!=zEntryTempID::ets.end();it++)
  {
    CGem *temp =(CGem *)it->second;
    SAFE_DELETE(temp);
  }
  clear();
  rwlock.unlock();
  SAFE_DELETE(channelUniqeID);
}

void CGemM::destroyMe()
{
  delMe();
}

bool CGemM::getUniqeID(DWORD &tempid)
{
  tempid=channelUniqeID->get();
  return (tempid!=channelUniqeID->invalid());
}

void CGemM::putUniqeID(const DWORD &tempid)
{
  channelUniqeID->put(tempid);
}

bool CGemM::init()
{
  wait_point[0].dwMapID = 102; // ��˳�
  wait_point[0].x = 196;
  wait_point[0].y = 166;

  wait_point[1].dwMapID = 102; // ��˳�
  wait_point[1].x = 106;
  wait_point[1].y = 628;
  
  wait_point[2].dwMapID = 128; // ���޹�
  wait_point[2].x = 105;
  wait_point[2].y = 261;
  
  wait_point[3].dwMapID = 129; // ������
  wait_point[3].x = 247;
  wait_point[3].y = 211;

  wait_point[4].dwMapID = 104; // ��β��
  wait_point[4].x = 81;
  wait_point[4].y = 59;
  
  wait_point[5].dwMapID = 105; // ɽկǰ��
  wait_point[5].x = 115;
  wait_point[5].y = 199;
  
  wait_point[6].dwMapID = 136; // ����
  wait_point[6].x = 125;
  wait_point[6].y = 119;
  
  wait_point[7].dwMapID = 136; // ����
  wait_point[7].x = 236;
  wait_point[7].y = 260;
    
  wait_point[8].dwMapID = 137; // �߾�
  wait_point[8].x = 218;
  wait_point[8].y = 79;
  
  wait_point[9].dwMapID = 137; // �߾�
  wait_point[9].x = 360;
  wait_point[9].y = 193;

  return true;
}

bool CGemM::processSceneMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
  switch (cmd->para)
  {
    case Cmd::Session::OP_GEMSTATE_SCENE_PARA:
      {
        Cmd::Session::t_OpGemState_SceneSession* rev = 
          (Cmd::Session::t_OpGemState_SceneSession*)cmd;

        if (rev->dwState == 1)
        {
          CCountryM::getMe().beginGem();
        }
        else
        {
          this->forceEnd();
        }

        return true;
      }
      break;
    case Cmd::Session::CHANGE_GEMSTATE_SCENE_PARA:
      {
        Cmd::Session::t_ChangeGemState_SceneSession* rev = 
          (Cmd::Session::t_ChangeGemState_SceneSession*)cmd;

        UserSession *pFromUser = UserSessionManager::getInstance()->getUserByID(rev->fromUserID);
        UserSession *pToUser = UserSessionManager::getInstance()->getUserByID(rev->toUserID);
        
        if (pFromUser && pToUser && (pFromUser->country == pToUser->country))
        {
          CGem *pGem = this->findGem(pFromUser->country);
          if (pGem) 
          {
            if (rev->dwState==1)
            {
              if (pGem->dragon.dwHoldUserID == pFromUser->id) {
                pGem->holdDragon(pToUser);
              }

              if (pGem->tiger.dwHoldUserID == pFromUser->id) {
                pGem->resetTiger();
              }
            }
            else if (rev->dwState==2)
            {
              if (pGem->tiger.dwHoldUserID == pFromUser->id) {
                pGem->holdTiger(pToUser);
              }

              if (pGem->dragon.dwHoldUserID == pFromUser->id) {
                pGem->resetDragon();
              }
            }


            // �������ӵ������ǰ��״̬
            Cmd::Session::t_SetGemState_SceneSession set_send;
            set_send.dwUserID = pFromUser->id;
            set_send.dwState = 0;

            if (pFromUser->scene)
            {
              pFromUser->scene->sendCmd(&set_send,sizeof(set_send));
            }
          }
        }
        
        return true;
      }
      break;
    default:
      break;
  }


  return false;
}


void CGemM::userOnline(UserSession* pUser)
{  
  CGem* pGem = NULL;
  rwlock.rdlock();
  pGem = this->findGem(pUser->country);
  rwlock.unlock();

  if (pGem)
  {
    if (pGem->dragon.dwHoldUserID == pUser->id)
    {
      Cmd::Session::t_SetGemState_SceneSession send;
      send.dwUserID = pUser->id;
      send.dwState = 1;
      if (pUser->scene) pUser->scene->sendCmd(&send,sizeof(send));
    }
  }
}

void CGemM::userOffline(UserSession* pUser)
{
  CGem* pGem = this->findGem(pUser->country);
  if (pGem)
  {
    if (pGem->dragon.dwHoldUserID == pUser->id)
    {
      pGem->resetDragon();
    }
    
    if (pGem->tiger.dwHoldUserID == pUser->id)
    {
      pGem->resetTiger();
    }
  }
}

bool CGemM::processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  switch (pNullCmd->byParam)
  {
    case REQUEST_DRAGON_PARA:
      {
        CGem* pGem = findGem(pUser->country);

        if (pGem)
        {
          if (pGem->dragon.dwHoldUserID==0)  
          {
            pGem->holdDragon(pUser);
          }
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����Ѿ�����ȡ");
          }
        }

        return true;
      }
      break;
    case REQUEST_TIGER_PARA:
      {
        CGem* pGem = findGem(pUser->country);

        if (pGem)
        {
          if (pGem->tiger.dwHoldUserID==0)  
          {
            pGem->holdTiger(pUser);
          }
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����Ѿ�����ȡ");
          }
        }

        return true;
      }
      break;  
    default:
      break;
  }

  return false;
}


void  CGemM::timer()
{
  struct execAll : public execEntry<CGem>
  {
    std::vector<CGem *> _removeList;
    
    execAll(){}

    void removeList()
    {
      std::vector<CGem *>::iterator tIterator;
      for (tIterator = _removeList.begin(); tIterator != _removeList.end(); tIterator++)
      {
        CGem *cd = *tIterator;
        CGemM::getMe().removeEntry(cd);
        SAFE_DELETE(cd);
      }
      _removeList.clear();
    }

    ~execAll(){}

    bool exec(CGem *pGem)
    {
      if (!pGem->isActivePeriod())
      {
        pGem->setReadyOverState();
      }
      
      if (pGem->state == CGem::GEM_OVER)
      {
        // TODO ��������
        _removeList.push_back(pGem);
      }

      return true;
    }
  };

  if (0)
  {//�������񲻿���
    
  execAll myList;
  execEveryOne(myList);
  myList.removeList();


  struct tm tv1;
  time_t timValue = time(NULL);
  zRTime::getLocalTime(tv1,timValue);

  if (tv1.tm_hour==8 && tv1.tm_min>=0 && tv1.tm_min<2)
  {
    CCountryM::getMe().beginGem();  
  }

  if (tv1.tm_hour==14 && tv1.tm_min>=0 && tv1.tm_min<2)
  {
    CCountryM::getMe().beginGem();  
  }
  
  if (tv1.tm_hour==20 && tv1.tm_min>=0 && tv1.tm_min<2)
  {
    CCountryM::getMe().beginGem();  
  }
  }
}

bool CGemM::addNewGem(DWORD dwCountryID)
{
  if (findGem(dwCountryID) != NULL) return false;
  
  CGem* pGem = new CGem();

  rwlock.wrlock();
  addEntry(pGem);
  rwlock.unlock();

  pGem->dwCountryID = dwCountryID;
  pGem->setReadyState();
  
  return true;
}

CGem * CGemM::findGem(DWORD dwCountryID)
{
  struct execAll : public execEntry<CGem>
  {
    DWORD _dwCountryID;
    
    CGem *_gem;
    
    execAll(DWORD dwCountryID)
    {
      _dwCountryID = dwCountryID;
      _gem = NULL;
    }
    ~execAll(){}
    bool exec(CGem *pGem)
    {
      if (pGem)
      {
        if (pGem->dwCountryID == _dwCountryID)
        {
          _gem = pGem;
          return false;
        }
      }

      return true;
    }
  };

  execAll myList(dwCountryID);
  execEveryOne(myList);
  return myList._gem;
}

CGem*   CGemM::findGemByID(DWORD dwID)
{
  return (CGem*)getEntryByTempID(dwID);
}

void CGemM::forceEnd()
{
  struct execAll : public execEntry<CGem>
  {
    std::vector<CGem *> _removeList;
    
    execAll(){}

    void removeList()
    {
      std::vector<CGem *>::iterator tIterator;
      for (tIterator = _removeList.begin(); tIterator != _removeList.end(); tIterator++)
      {
        CGem *cd = *tIterator;
        CGemM::getMe().removeEntry(cd);
        SAFE_DELETE(cd);
      }
      _removeList.clear();
    }

    ~execAll(){}

    bool exec(CGem *pGem)
    {
      pGem->setReadyOverState();
      
      if (pGem->state == CGem::GEM_OVER)
      {
        // TODO ��������
        _removeList.push_back(pGem);
      }

      return true;
    }
  };

  execAll myList;
  execEveryOne(myList);
  myList.removeList();
}

