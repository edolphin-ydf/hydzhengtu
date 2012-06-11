/**
 * \brief    ����
 * 
 */
#include <zebra/ScenesServer.h>

/**
 * \brief ���캯��
 *
 * ��ʼ����ر���
 * \param scene npc���ڵĳ���
 * \param npc ��������
 * \param define ��������
 * \param type ����
 * \param entrytype �������� 
 * \param abase ��ǿnpc�ֵ�����
 */
GuardNpc::GuardNpc(Scene *scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype,zNpcB *abase) :
  ScenePet(scene,npc,define,type,entrytype,abase),_exp(0),_gold(0),_status(0),_time(SceneTimeTick::currentTime)
{
  masterIsAlive = true;
  isSeptGuard = false;
}

/**
 * \brief ��������
 *
 * ������ر���
 *
 */
GuardNpc::~GuardNpc()
{
  reset();  
}

/**     
 * \brief  �����ڳ�������
 *
 * \param user: ������
 * \return ��
 */   
void GuardNpc::owner(SceneUser* user)
{
  //_owner = user;  
  setMaster(user);
  _name = user->name;
  user->guard = this;
  /*
  Zebra::logger->debug("[�ڳ�] %s,%u,%s(%u,%u),%s(%u,%u) ���� _gold=%u",
          _name.c_str(),
          tempid,name,getPos().x,getPos().y,
          user ? user->name : "NULL",user ? user->getPos().x : 0,user ? user->getPos().y : 0,
          _gold);
          */
}  

/**     
 * \brief  ȡ���ڳ�������
 *
 * \return  ������
 */   
SceneUser* GuardNpc::owner()
{
  SceneUser * _owner = (SceneUser *)getMaster();
  return _owner;
}

/**     
 * \brief  �����ڳ����
 *
 * \param money: ���
 * \return ��
 */     
void GuardNpc::gold(int money)
{
  _gold = money;
}

/**     
 * \brief  ȡ���ڳ����
 *
 * \return  ���
 */
int GuardNpc::gold() const
{
  return _gold;
}

/**     
 * \brief  �����ڳ�����
 *
 * \param experience: ����
 * \return ��
 */   
void GuardNpc::exp(int experience)
{
  _exp = experience;
}

/**     
 * \brief  ȡ���ڳ�����
 *
 * \return ����
 */
int GuardNpc::exp() const
{
  return _exp;
}

/**     
 * \brief ����ʧ��
 *
 * ���øô�����ʧ��
 *
 * \return ��
 */
void GuardNpc::reset()
{
  SceneUser * _owner = (SceneUser *)getMaster();
  if (_owner)
  {
    Cmd::stDelPetPetCmd del;
    del.id= tempid;
    del.type = Cmd::PET_TYPE_GUARDNPC;
    _owner->sendCmdToMe(&del,sizeof(del));

    _owner->guard = NULL;
  }
  _owner = NULL;
  clearMaster();
  
//  _gold = 0;
  
  moveAction = false;
  _status = 2;
  _time = SceneTimeTick::currentTime;
  _time.addDelay(3*60*1000); //three mins 
#ifdef _DEBUG
  Zebra::logger->debug("[�ڳ�]%s �ڳ� reset time:%d",_name.c_str(),_time.sec()-SceneTimeTick::currentTime.sec());
#endif
}

/**     
 * \brief ״̬���
 *
 * �ص�����,���ڼ������״̬,������Ӧ����
 *
 * \return ��
 */
void GuardNpc::check()
{
  SceneUser * _owner = (SceneUser *)getMaster();

  if (_owner && 0==_owner->guard) _owner->guard = this;

  if (_status == 0)
  {
    if (!_owner || abs((long)(pos.x-_owner->getPos().x))>SCREEN_WIDTH || abs((long)(pos.y-_owner->getPos().y))>SCREEN_HEIGHT)
    {
      if (_owner)
      {
        Channel::sendSys(_owner,Cmd::INFO_TYPE_GAME,"���뻤��Ŀ��̫Զ��,����Σ�յ�!");
        Channel::sendSys(_owner,Cmd::INFO_TYPE_EXP,"���뻤��Ŀ��̫Զ��");
        if (_owner->getState() == zSceneEntry::SceneEntry_Normal)
          masterIsAlive = true;
      }

      moveAction = false;
      /*
      Zebra::logger->debug("[�ڳ�] %s,%u,%s(%u,%u),%s(%u,%u) _gold=%u,0 -> 1",
              _name.c_str(),
          tempid,name,getPos().x,getPos().y,
          _owner ? _owner->name : "NULL",_owner ? _owner->getPos().x : 0,_owner ? _owner->getPos().y : 0,
          _gold);
          */
      _status = 1;
      _time = SceneTimeTick::currentTime;
      _time.addDelay(3*60*1000); //three mins 
    }
  }
  
  if (_status == 1)
  {
    if (_owner && _owner->scene==scene && (abs((long)(pos.x-_owner->getPos().x)) < SCREEN_WIDTH && abs((long)(pos.y-_owner->getPos().y)) < SCREEN_HEIGHT))
    {
      if (_owner->getState() == zSceneEntry::SceneEntry_Normal)
        masterIsAlive = true;
      moveAction = true;
      _status = 0;

      /*
      Zebra::logger->debug("[�ڳ�] %s,%u,%s(%u,%u),%s(%u,%u) _gold=%u,1 -> 0",
              _name.c_str(),
          tempid,name,getPos().x,getPos().y,
          _owner ? _owner->name : "NULL",_owner ? _owner->getPos().x : 0,_owner ? _owner->getPos().y : 0,
          _gold);
          */
    }
    else if (!_owner && (SceneTimeTick::currentTime>_time))
    {
      /*
      Zebra::logger->debug("[�ڳ�] %s,%u,%s(%u,%u),%s(%u,%u) _gold=%u,1 -> 2,3����û�ҵ�����,����ʧ��",
              _name.c_str(),
          tempid,name,getPos().x,getPos().y,
          _owner ? _owner->name : "NULL",_owner ? _owner->getPos().x : 0,_owner ? _owner->getPos().y : 0,
          _gold);
          */

      reset();
      setClearState();
    }
  }
  /*
  if (_status == 1 && _owner && _owner->scene==scene && (abs(pos.x-_owner->getPos().x) < SCREEN_WIDTH && abs(pos.y-_owner->getPos().y) < SCREEN_HEIGHT)) {
    moveAction = true;
    _status = 0;
  }


  if (_status != 1  && _owner && (abs(pos.x-_owner->getPos().x) > SCREEN_WIDTH || abs(pos.y-_owner->getPos().y) > SCREEN_HEIGHT)) {
    Channel::sendSys(_owner,Cmd::INFO_TYPE_GAME,"���뻤��Ŀ��̫Զ��,����Σ�յ�!");
    Channel::sendSys(_owner,Cmd::INFO_TYPE_EXP,"���뻤��Ŀ��̫Զ��");
    moveAction = false;
    _status = 1;
  }
  */

  /*
  if (_status !=2 && _owner && (abs(pos.x-_owner->getPos().x) > 2*SCREEN_WIDTH || abs(pos.y-_owner->getPos().y) > 2*SCREEN_HEIGHT)) {
    Channel::sendSys(_owner,Cmd::INFO_TYPE_GAME,"���������ڳ�̫Զ��,��λ���ʧ����");
    OnOther event(2);
    EventTable::instance().execute(*_owner,event);
    reset();
  }
  */
  
  //Zebra::logger->debug("check: current time:%d,check time:%d",SceneTimeTick::currentTime.msecs(),_time.msecs());
  if (_status == 2  && SceneTimeTick::currentTime > _time) {
    //Zebra::logger->debug("clear: current time:%d,check time:%d",SceneTimeTick::currentTime.msecs(),_time.msecs());
    setClearState();
    return;
  }

  bool ok = false;
  if (scene) {
    std::string n = scene->name;
    if (n.find(_map) != std::string::npos) {
      ok = true;
    }
  }
  if (_status == 0 && ok && abs((long)(pos.x-_dest.x)) < (SCREEN_WIDTH>>1) && abs((long)(pos.y-_dest.y)) < (SCREEN_HEIGHT >> 1) ) {
    on_reached();
  }
  //_time = SceneTimeTick::currentTime;
}

/**     
 * \brief �ڳ�����
 *
 * �ص�����,���ڳ�������ʱ����,����������
 * ]param attacker ����
 * \return ��
 */
void GuardNpc::on_death(SceneEntryPk* attacker)
{
  if (0==attacker) return;
  /*
     if (0 == strcmp(_name.c_str().c_str(),attacker->name) ) {
     return;
     }
     */

  SceneEntryPk *m = attacker->getTopMaster();
  if (!m) return;

  if (m->getType()==zSceneEntry::SceneEntry_Player)
  {
/*
    Cmd::stChannelChatUserCmd send;
    zRTime ctv;
    send.dwType = Cmd::CHAT_TYPE_NINE;
    send.dwChatTime = ctv.sec();
    if (((SceneUser *)m)->mask.is_masking())
      _snprintf((char *)send.pstrChat,MAX_CHATINFO,"������ ���ڳɹ�,�������%d",_gold);  
    else
      _snprintf((char *)send.pstrChat,MAX_CHATINFO,"%s���ڳɹ�,�������%d",m->name,_gold);  
    strncpy((char *)send.pstrName,"����",MAX_NAMESIZE);
    scene->sendCmdToNine(getPosI(),&send,sizeof(send));  
    Zebra::logger->debug("%s���ڳɹ�,�������%d",m->name,_gold);
*/
    char info[MAX_CHATINFO];
    memset(info,0,MAX_CHATINFO);
    _snprintf(info,MAX_CHATINFO,"�����ٳɹ�,�������"); 

    if (((SceneUser*)m)->charbase.country!=scene->getCountryID())//��������ڲ���Ǯ
      ((SceneUser*)m)->packs.addMoney(getRobGold(),"���ٻ��",info);
  }

  SceneUser * _owner = (SceneUser *)getMaster();
  /*
  Zebra::logger->debug("[�ڳ�] %s,%u,%s(%u,%u),%s(%u,%u),����,ɱ�� %s _gold=%u",
              _name.c_str(),
      tempid,name,getPos().x,getPos().y,
      _owner ? _owner->name : "NULL",_owner ? _owner->getPos().x : 0,_owner ? _owner->getPos().y : 0,
      m->name,_gold);
      */

  if (_owner) {
    OnOther event(2);
    EventTable::instance().execute(*_owner,event);
    Channel::sendSys(_owner,Cmd::INFO_TYPE_EXP,"��Ļ���Ŀ������");
  }
  else
  {
    if (getMasterID())
    {
      Cmd::Session::t_guardFail_SceneSession send;
      send.userID = getMasterID();
      send.type = 0;
      sessionClient->sendCmd(&send,sizeof(send));
    }
  }
}

/**     
 * \brief �ڳ�����
 *
 * �ص�����,���ڳ�����Ŀ�ĵ�ʱ������,����������
 *
 * \return ��
 */
void GuardNpc::on_reached()
{
  SceneUser * _owner = (SceneUser *)getMaster();
  if (_owner)
  {
    if (id>54201 && id<54350)
    {
      OnOther event(11);
      EventTable::instance().execute(*_owner,event);
    }
    else
    {
      OnOther event(1);
      EventTable::instance().execute(*_owner,event);
    }
  }
  /*
  Zebra::logger->debug("[�ڳ�] %s,%u,%s(%u,%u),%s(%u,%u),�ɹ��˵� _gold=%u",
              _name.c_str(),
      tempid,name,getPos().x,getPos().y,
      _owner ? _owner->name : "NULL",_owner ? _owner->getPos().x : 0,_owner ? _owner->getPos().y : 0,
      _gold);
      */
/*
  if (_owner) {
    Channel::sendSys(_owner,Cmd::INFO_TYPE_GAME,"���ڳɹ�,��þ���%d",_exp);
    //add exp bonus  
    _owner->charbase.exp += _exp;
    ScenePk::attackRTExp(_owner,_exp);
    if (_owner->charbase.exp >= _owner->charstate.nextexp) { 
      _owner->upgrade();
    }
  }
*/
  reset();
  setClearState();
  return;
}

void GuardNpc::dest(const zPos& pos)
{
  _dest = pos;
}

void GuardNpc::map(const std::string& name)
{
  _map = name;
}

/**
 * \brief ����Ƿ�����ƶ�
 *
 * \return �Ƿ�����ƶ�
 */
bool GuardNpc::canMove()
{
  return (masterIsAlive && ScenePet::canMove()); 
}

/**
 * \brief �������ƶ�,��������
 *
 * \return �Ƿ��ƶ��ɹ�
 */
bool GuardNpc::moveToMaster()
{
#ifdef _DEBUG
  //Channel::sendNine(this,"GuardNpc::moveToMaster()");
#endif
  SceneEntryPk * master = getMaster();
  if (!master)
  {
    check();
    return true;
  }

  //AIC->setActRegion(master->getPos(),2,2);

  if (!canMove())
  {
    check();
    if (getMaster())
      return false;
    else
      return true;
  }

  //check();
  if (checkMoveTime(SceneTimeTick::currentTime))
  {
    if (master->scene == scene)
    {
      check();
      if (!getMaster()) return true;//���ڳɹ�ɾ��������

      if ((scene->zPosShortRange(getPos(),master->getPos(),npc_pet_chase_region)))
        return false;
      else
      {
        if (!gotoFindPath(getPos(),master->getPos()))
          return goTo(master->getPos());
        return true;
      }
    }
    /*
    else
    {
      if (canMove() && master->scene && masterIsAlive)
      {
        if (changeMap(master->scene,master->getPos()))
        {
          check();
          return true;
        }
        else
          return false;
        
      }
    }
    */
  }
  return false;
}

/*
 * \brief ���滤��NPC�����ݵ�ָ��λ�ã����ʱ�ã�
 *
 * \param dest Ŀ�ĵ�ַ
 *
 * \return ���ݴ�С
 */
DWORD GuardNpc::save(BYTE * dest)
{
  bcopy(&petData,dest,sizeof(Cmd::t_PetData),sizeof(Cmd::t_PetData));
  Cmd::t_PetData * t = (Cmd::t_PetData *)dest;
  strncpy(t->name,_map.c_str(),MAX_NAMESIZE);
  t->str = _dest.x;
  t->intel = _dest.y;
  t->agi = _gold;
  t->men = _exp;
  return sizeof(Cmd::t_PetData);
}

DWORD GuardNpc::getRobGold()
{
  return _gold/(isSeptGuard?2:1);
}
