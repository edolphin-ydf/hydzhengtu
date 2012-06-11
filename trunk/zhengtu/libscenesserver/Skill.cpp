#include <zebra/ScenesServer.h>

class ItemObjectCompare:public UserObjectCompare 
{
  public:
    DWORD  dwObjectID;

    bool isIt(zObject *object)
    {
      if (object->data.dwObjectID == dwObjectID) return true;
      return false;
    }
};

///����
extern int getCharType(DWORD type);

/// ��������ΨһID,����ȡֵ��Χ
///DWORD zSkill::uniqueID(1);

/**
 * \brief ��鼼���Ƿ����
 * \return true ��ʾ���� false ��ʾ���ܲ�����
 */
bool zSkill::canUse()
{
  zRTime ctv;
  if (actionbase->dtime > ctv.msecs() - lastUseTime)
  {
    if ((float)(ctv.msecs()-lastUseTime)/(float)actionbase->dtime < 0.9f)
      return false;
  }
  lastUseTime = ctv.msecs();
  return true;
}

/**
 * \brief ������ȴʱ��
 */
void zSkill::resetUseTime()
{
  zRTime ctv;
  lastUseTime = ctv.msecs();
}

/**
 * \brief �����ȴʱ��
 */
void zSkill::clearUseTime()
{
  zRTime ctv;
  lastUseTime = 0;
}

/**
 * \brief ���캯��,��ʼ����������
 */
zSkill::zSkill() : zEntry()
{
  zRTime ctv;
  bzero(&data,sizeof(data));
  base = NULL;
  actionbase = NULL;
  inserted = false;
  needSave = false;
  ctv.setmsecs(0);
  lastUseTime = ctv.msecs();
  _entry = NULL;
  dirty = false;
}

/**
 * \brief ����Ƿ������
 * \param pEntry
 * \return true ��ʾ���������ü��� false��ʾ��������������
 */
bool zSkill::canUpgrade(SceneEntryPk *pEntry)
{
  if (pEntry->getLevel() < MAX_SKILLLEVEL)
  {
    return false;
  }

  /*
  if (pUser->charbase.fivevalue[data.five] < data.nextfive)
  {
    return false;
  }
  // */
  return true;
}

/**
 * \brief �����Ӧ�ļ������Ƿ����,���������ɾ��
 * \param nextbase �Ƿ�ȡ��һ����base
 * \return true ����ѧϰ�ü��� false ����ѧϰ,û����
 */
bool zSkill::checkSkillBook(bool nextbase)
{
  const zSkillB *checkbase=NULL;
  if (nextbase)
  {
    checkbase = skillbm.get(skill_hash(data.skillid,data.level + 1));
  }
  else
  {
    checkbase = base;
  }
  if (NULL == checkbase) return false;

  if ((_entry->getType() == zSceneEntry::SceneEntry_Player)&&( checkbase->useBook>0 ))
  {
      SceneUser *pUser = ((SceneUser *)_entry);
      if (pUser->reduceObjectNum(checkbase->useBook,1)==-1) return false;
    /*
    ItemObjectCompare found;
    found.dwObjectID = checkbase->useBook;
    SceneUser *pUser = ((SceneUser *)_entry);
    zObject *itemobj = pUser->packs.uom.getObject(found);// ���ҵ���
    if (itemobj)
    {
      Cmd::stRemoveObjectPropertyUserCmd send;
      send.qwThisID=itemobj->data.qwThisID;
      pUser->sendCmdToMe(&send,sizeof(send));
      pUser->packs.rmObject(itemobj);
      SAFE_DELETE(itemobj);
    }
    else
    {
      return false;
    }*/
  }

  return true;
}

/**
 * \brief ��鱾���ܵ�ѧϰ����
 * \return false ������ѧϰ true ����ѧϰ
 */
bool zSkill::checkSkillStudy(bool nextbase)
{
  const zSkillB *checkbase=NULL;
  if (nextbase)
  {
    checkbase = skillbm.get(skill_hash(data.skillid,data.level + 1));
  }
  else
  {
    checkbase = base;
  }
  if (NULL == checkbase) return false;

  zSkill *s =NULL;
  if (checkbase->preskill1 >0)
  {
    s = _entry->usm.findSkill(checkbase->preskill1);
    if (s)
    {
      if (s->base->level < checkbase->preskilllevel1)
      {
        return false;
      }
    }
    else
    {
      return false;
    }
    s = NULL;
  }

  if (checkbase->preskill2 >0)
  {
    s = _entry->usm.findSkill(checkbase->preskill2);
    if (s)
    {
      if (s->base->level < checkbase->preskilllevel2)
      {
        return false;
      }
    }
    else
    {
      return false;
    }
    s = NULL;
  }

  if (checkbase->preskilllevel3 >0)
  {
    if (_entry->getLevel() < checkbase->preskilllevel3)
    {
      return false;
    }

  }
/*
  if (checkbase->preskill3 >0)
  {
    s = _entry->usm.findSkill(checkbase->preskill3);
    if (s)
    {
      if (s->base->level < checkbase->preskilllevel3)
      {
        return false;
      }
    }
    else
    {
      return false;
    }
    s = NULL;
  }
*/

  return true;
}

/**
 * \brief ���ݼ����ֵ��ʼ�������ܶ���
 * \param pEntry �ȴ����ؼ��ܵĽ�ɫ
 * \return false ��ʼ��ʧ�� true ��ʼ���ɹ�
 */
bool zSkill::setupSkillBase(SceneEntryPk *pEntry)
{
  zSkillB *basebm = skillbm.get(skill_hash(data.skillid,data.level));
  if (basebm == NULL) 
  {
    return false;
  }
  zSkillB *basebmnext = NULL;
  if (data.level < MAX_SKILLLEVEL)
  {
    basebmnext = skillbm.get(skill_hash(data.skillid,data.level + 1));
  }
  base = basebm;
  actionbase = this->getNewBase();
  data.skillid = basebm->skillid;
  strncpy(name,basebm->name,MAX_NAMESIZE);
  data.level = basebm->level;
  if (base->usetype == SKILL_TYPE_PASSIVENESS) doPassivenessSkill();
  return true;
}

/**
 * \brief ��һ��ѧϰһ�����ܵ�ʱ��ʹ�ô˷�������һ�����ܶ���Ͷ�͵���ɫ����
 * \param pEntry �ȴ����ؼ��ܵĽ�ɫ
 * \param id ����id
 * \param level ���ܵļ���
 * \return ����ɹ������򷵻�������ܶ���,���򷵻�NULL
 */
zSkill *zSkill::create(SceneEntryPk *pEntry,DWORD id,DWORD level)
{
	zSkillB *basebm = skillbm.get(skill_hash(id,level));
	if (basebm == NULL) 
	{
		return NULL;
	}

	zSkill *ret = new zSkill();
	if (ret == NULL)
	{
		return NULL;
	}
	ret->_entry = pEntry;
	ret->base = basebm;
	ret->id = basebm->id;
	ret->tempid = basebm->skillid;

	//TODO �������
	ret->data.skillid = basebm->skillid;
	strncpy(ret->name,basebm->name,MAX_NAMESIZE);
	ret->data.level = basebm->level;

	if (pEntry->getType() == zSceneEntry::SceneEntry_Player && pEntry->getPriv()<=Gm::normal_mode)
	{
		if (!ret->checkSkillStudy())
		{
			SAFE_DELETE(ret);
			return NULL;
		}

		if (!ret->checkSkillBook())
		{
			SAFE_DELETE(ret);
			return NULL;
		}

		if (pEntry->usm.getPointInTree(ret->base->subkind) < ret->base->needpoint)
		{
			SAFE_DELETE(ret);
			return NULL;
		}
	}

	ret->setupSkillBase(pEntry);

	if (!pEntry->addSkillToMe(ret))
	{
		Zebra::logger->debug("zSkill::create():�޷����¼��ܼӵ���ɫ����");
		SAFE_DELETE(ret);
		return NULL;
	}

	zRTime ctv;
	ret->lastUseTime = ctv.msecs() - ret->base->dtime;
	ret->inserted = true;
	ret->istemp = false;
	pEntry->skillStatusM.processPassiveness();
	return ret;
}

/**
 * \brief ������ʱ���ܶ��󲢷���
 * \param pEntry �ȴ���ȡ���ܵĽ�ɫ
 * \param id ����id
 * \param level ���ܵļ���
 * \return ����ɹ������򷵻�������ܶ���,���򷵻�NULL
 */
zSkill *zSkill::createTempSkill(SceneEntryPk *pEntry,DWORD id,DWORD level)
{
  zSkillB *basebm = skillbm.get(skill_hash(id,level));
  if (basebm == NULL) 
  {
    return NULL;
  }

  zSkill *ret = new zSkill();
  if (ret == NULL)
  {
    return NULL;
  }
  ret->_entry = pEntry;
  ret->base = basebm;
  ret->id = basebm->id;
  ret->tempid = basebm->skillid;///++uniqueID;
  //if (!ret->tempid)
  //{
  //  ret->tempid = ++uniqueID;
  //}

  //TODO �������
  ret->data.skillid = basebm->skillid;
  strncpy(ret->name,basebm->name,MAX_NAMESIZE);
  ret->data.level = basebm->level;

  ret->inserted = true;
  ret->istemp = true;
  return ret;
}

/**
 * \brief ����һ�����ܵ���ɫ����
 * \param pEntry �ȴ����ؼ��ܵĽ�ɫ
 * \param s ��Ҫ���صĴ浵
 * \return ����ɹ������򷵻�������ܶ���,���򷵻�NULL
 */
zSkill *zSkill::load(SceneEntryPk *pEntry,const SaveSkill *s)
{
  DWORD mylevel = 0;
  DWORD colddown = 0;

  if (s == NULL)
  {
    return NULL;
  }
  
  mylevel = s->level%100;
  colddown = (s->level - mylevel)/100;
  zSkillB *basebm = skillbm.get(skill_hash(s->type,mylevel));
  

  if (basebm == NULL)
  {
    return NULL;
  }

  zSkill *ret = new zSkill();
  if (ret == NULL)
  {
    return NULL;
  }
  ret->_entry = pEntry;
  ret->id = basebm->id;
  ret->tempid = basebm->skillid;///++uniqueID;
  //if (!ret->tempid)
  //{
  //  ret->tempid = ++uniqueID;
  //}
  /*
  if (ret->tempid == uniqueID.invalid())
  {
    SAFE_DELETE(ret);
    return NULL;
  }
  // */
  ret->data.skillid = basebm->skillid;
  strncpy(ret->name,basebm->name,MAX_NAMESIZE);
  ret->data.level = basebm->level;
  ret->setupSkillBase(pEntry);
  ret->_entry=pEntry;

  zRTime ctv;
  if (colddown>ret->base->dtime) colddown = ret->base->dtime;
  ret->lastUseTime = ctv.msecs() - (ret->base->dtime - colddown);
  
  if (!pEntry->addSkillToMe(ret))
  {
    SAFE_DELETE(ret);
    return NULL;
  }
  ret->inserted = true;
  ret->istemp = false;
  //if (ret->base->usetype == SKILL_TYPE_PASSIVENESS) ret->doPassivenessSkill();
  return ret;
}

/**
 * \brief ˢ�¼���,��Ҫ�������������Լ��ܵǻ���ɵ�Ӱ��
 */
void zSkill::refresh(bool ignoredirty)
{
  actionbase = this->getNewBase();
  if (dirty||ignoredirty)
  {
    dirty = false;
    zRTime ctv;
    DWORD colddown = ctv.msecs() - this->lastUseTime;
    if (colddown >= this->base->dtime)
      colddown = 0;
    else
      colddown = this->base->dtime - colddown;

    Cmd::stAddUserSkillPropertyUserCmd ret;
    ret.dwSkillID = this->base->skillid;
    ret.wdLevel = this->base->level;
    ret.wdUpNum = _entry->skillUpLevel(base->skillid,base->kind);
    if (ret.wdUpNum+ret.wdLevel>10) ret.wdUpNum = 10 - ret.wdLevel;
    ret.dwExperience = colddown;
    if (this->base->usetype == SKILL_TYPE_PASSIVENESS) this->doPassivenessSkill();
    if (_entry->getType()==zSceneEntry::SceneEntry_Player) ((SceneUser *)_entry)->sendCmdToMe(&ret,sizeof(ret));
  }
}


/**
 * \brief ��ȡ���ܴ浵����
  * \return ture ������Ҫ�������ֵ��ʲô��
 */
bool zSkill::getSaveData(SaveSkill *save)
{
  zRTime ctv;
  DWORD colddown = ctv.msecs() - this->lastUseTime;
  if (colddown >= this->base->dtime)
    colddown = 0;
  else
    colddown = this->base->dtime - colddown;
  save->type = data.skillid;
  save->level = (colddown) *100 + data.level;
  return true;
}


/**
 * \brief ��������
  */

zSkill::~zSkill()
{
  if (inserted)
  {
    inserted = false;
    bzero(&data,sizeof(data));
  }
}

const zSkillB *zSkill::getNewBase()
{
  const zSkillB *temp = NULL;
  DWORD curLevel = base->level + _entry->skillUpLevel(base->skillid,base->kind);
  if (curLevel >10) curLevel = 10;

  temp = skillbm.get(skill_hash(base->skillid,curLevel));

  if (!temp)
    temp = base;
  if (!dirty &&(temp != actionbase)) dirty = true;
  return temp;
}

/**
 * \brief ִ��һ�����ñ�������
 */
void zSkill::doPassivenessSkill()
{
  std::vector<SkillStatus>::const_iterator iter;

  actionbase = getNewBase();

  for(iter  = actionbase->skillStatus.begin(); iter != actionbase->skillStatus.end(); iter ++)
  {
    SkillStatus *pSkillStatus = (SkillStatus *)&*iter;
    DWORD skillid = actionbase->skillid;
    _entry->skillStatusM.putPassivenessOperationToMe(skillid,pSkillStatus);
  }
}

/**
 * \brief  ����һ������[SKY���ܴ���]
 * \param  rev �����յ��Ŀͻ�����Ϣ
 * \param cmdLen ��Ϣ����
 * \return true ����ʹ�óɹ� false ����ʹ��ʧ��
 */
bool zSkill::action(const Cmd::stAttackMagicUserCmd *rev,const DWORD cmdLen)
{
  if (_entry)
  {
    actionbase = getNewBase();
    curRevCmd = rev;
    curRevCmdLen = cmdLen;

	//sky SKILLID_IMMOLATE �׼����ܵ�ID
	if(rev->wdMagicType == SKILLID_IMMOLATE && _entry->Immolate)
	{
		_entry->skillStatusM.clearSkill(rev->wdMagicType);
		return true;
	}

    // ����
    if (this->istemp||_entry->needWeapon(curRevCmd->wdMagicType)) // ���ʩ�ż����Ƿ���Ҫ����
    {
      if (actionbase->ride==0) 
      {
        if (_entry->checkMountHorse()) return false;
      }
      if (_entry->checkSkillCost(actionbase))  // ���������������
      {
        if (_entry->checkPercent())  // �жϼ���ʩ�ųɹ��ļ���
        {
          std::vector<SkillStatus>::const_iterator iter;
          if (_entry->checkReduce(actionbase->objcost,actionbase->objnum))
          {
            showMagicToAll();
            switch(rev->wdMagicType)
            {
              case 226: // ʳʬ��
              case 319: // ʬ����
                {
                  SceneNpc *pNpc = SceneNpcManager::getMe().getNpcByTempID(rev->dwDefenceTempID);
                  if (pNpc)
                  {
                    if (pNpc->getState() == zSceneEntry::SceneEntry_Death && !pNpc->isUse) 
                    {
                      pNpc->isUse = true;
                      Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
                      removeNpc.dwMapNpcDataPosition = rev->dwDefenceTempID;
                      pNpc->scene->sendCmdToNine(pNpc->getPosI(),&removeNpc,sizeof(removeNpc),pNpc->dupIndex);
                    }
                    else
                    {
                      return false;
                    }
                  }
                  else
                  {
                    return false;
                  }
                }
                break;
              default:
                break;
            }
            _entry->doSkillCost(actionbase);
            for(iter  = actionbase->skillStatus.begin(); iter != actionbase->skillStatus.end(); iter ++)
            {
              // ��������������Ʒ�ͼ��ܶ���Ʒ������,Ŀǰ�˽ӿ�δ����,���������ֱ�����ƷID����������
              {
                SkillStatus *pSkillStatus = (SkillStatus *)&*iter;
                doOperation(pSkillStatus);
              }
            }
            _entry->reduce(actionbase->objcost,actionbase->objnum);
            return true;
          }
        }
      }
    }
  }
  else
  {
    Zebra::logger->error("ѧϰ�ļ�����_entryָ��Ϊ��");
  }
  return false;
}

/**
 * \brief չʾħ��Ч��
 * \return ����Ƿ���true��ʾ���Լ����������������̷���false��ʾ����
 */
bool zSkill::showMagicToAll()
{
  if (!ScenePk::attackUserCmdToNine(curRevCmd,_entry))
  {
    return false;
  }
  return true;
}

/**
 * \brief ����,��������Ч��Χ�ڵĵ�
 *
struct GetMagicPosExec : public MagicPosExec
{
  GetMagicPosExec(SceneMagic::zPos_vector &pos_vec,Scene *s) : _pos_vec(pos_vec),scene(s){}
  //~GetMaigcPosExec(){}
  bool exec(zPos &pos)
  {
    //��鷶Χ��Ч
    if (!scene->zPosValidate(pos))
    {
      return false;
    }
    _pos_vec.push_back(pos);
    return true;
  }
  private:
  SceneMagic::zPos_vector &_pos_vec;
  Scene *scene;
};
*/

/**
 * \brief  �����ṹ,��������������ж�Ա�㲥��Ϣ
 */
struct SendStatusToTeamExec : public TeamMemExec
{

  SceneUser * _user;
  const SkillStatus *_skillStatus;
  /**
   * \brief  �����ʼ������
   * \param  data ��Ϣ��
   * \param  dataLen ��Ϣ����
   */
  SendStatusToTeamExec(SceneUser *pUser,const SkillStatus *pSkillStatus)
  {
    _user = pUser;
    _skillStatus = pSkillStatus;
  }

  /**
   * \brief  �ص�����
   * \param  member ��Ա
   * \return false ��ֹ���� true ��������
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().
            getUserByTempID(member.tempid);
    if (pUser)
    {
      if (_user->checkMagicFlyRoute(pUser,_skillStatus->mode))
      {
        pUser->skillStatusM.putOperationToMe(_user->carrier,true);
      }
    }
    return true;
  }
};

/**
 * \brief ִ��һ�����ܲ�����һ�������������ɸ�����״̬,�����в�ͬ��Ŀ�����ͷ�Χ�ȣ�[sky]���ܲ���
 */
void zSkill::doOperation(const SkillStatus *pSkillStatus)
{
  _entry->carrier.status  = pSkillStatus;      // һ�����ܲ���
  //_entry->carrier.skilltype = base->usetype;  // ����ʹ������
  //_entry->carrier.skillID  = id;        // ����ID
  _entry->carrier.skillbase = actionbase;        // �����ֵ�
  _entry->carrier.revCmd  = *curRevCmd;      // ���汾���յ��Ĺ�����Ϣ
  _entry->carrier.attacker  = _entry;      // �����ߵ�ָ��


  WORD wdTarget = pSkillStatus->target;
  if (wdTarget& TARGET_SELF)
  {
    if (_entry->skillStatusM.putOperationToMe(_entry->carrier,true))
    {
      //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
    }
    wdTarget&=(0xff & (~(1))); // �����ʾ�Լ���λ,���ֻ���Լ�ʩ����ôwdTarget��Ϊ0
  }

  _entry->pkValue.damagebonus = actionbase->damnum;
  Scene *pScene = NULL;
  pScene = _entry->scene;
  if (wdTarget)
  {
    if (1 == pSkillStatus->range) // 1��˵��ʾ����,������Ϊ�������жϱ�־
    {
      switch(curRevCmd->byAttackType)
      {
        case Cmd::ATTACKTYPE_N2U:  /// Npc�����û�
          {
            //if (!_entry->isPkZone()) return;
            /*if (_entry->tempid == curRevCmd->dwDefenceTempID)
            {
              ScenePk::attackUserCmdToNine(curRevCmd,_entry);
              return;
            }*/
            SceneUser *pDef = pScene->getUserByTempID(curRevCmd->dwDefenceTempID);
            if (pDef)
            {
              if (wdTarget&TARGET_FRIEND)
              {
                if (!_entry->isEnemy(pDef,false,true)&&_entry->checkMagicFlyRoute(pDef,pSkillStatus->mode))
                {
                  if (pDef->skillStatusM.putOperationToMe(_entry->carrier,true))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
              }
              if (wdTarget&TARGET_ENEMY)
              {
                if (pDef->getTopMaster()&&pDef->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player)
                {
                  if (!(pDef->isPkZone(_entry) && _entry->isPkZone(pDef))) // �¼�&&this->isPkZone(pDef)
                  {
                    ScenePk::attackFailToMe(curRevCmd,_entry);
                    return;
                  }
                }

                if (_entry->isEnemy(pDef,_entry->getType() == zSceneEntry::SceneEntry_Player)&& // �жϵ����Ѻͼ��PKģʽ��һ������,����ֻҪ�ж�һ�ξ�OK��
                  _entry->checkMagicFlyRoute(pDef,pSkillStatus->mode))
                {
                  if (pDef->skillStatusM.putOperationToMe(_entry->carrier))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
              }
            }
          }
          break;
        case Cmd::ATTACKTYPE_U2U:  /// �û������û�
          {
            //if (!_entry->isPkZone()) return;
            /*if (_entry->tempid == curRevCmd->dwDefenceTempID)
            {
              ScenePk::attackUserCmdToNine(curRevCmd,_entry);
              return;
            }*/
            SceneUser *pDef = pScene->getUserByTempID(curRevCmd->dwDefenceTempID);
            if (pDef)
            {
              if (wdTarget&TARGET_FRIEND)
              {
                if (!_entry->isEnemy(pDef,false,true)&&_entry->checkMagicFlyRoute(pDef,pSkillStatus->mode))
                {
                  if (pDef->skillStatusM.putOperationToMe(_entry->carrier,true))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
              }
              if (wdTarget&TARGET_ENEMY)
              {
                if (_entry->isEnemy(pDef,_entry->getType() == zSceneEntry::SceneEntry_Player)&& // �жϵ����Ѻͼ��PKģʽ��һ������,����ֻҪ�ж�һ�ξ�OK��
                  pDef->isPkZone(_entry)&&_entry->isPkZone(pDef)&&//�¼� _entry->isPkZone(pDef)
                  _entry->checkMagicFlyRoute(pDef,pSkillStatus->mode))
                {
                  if (pDef->skillStatusM.putOperationToMe(_entry->carrier))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
                else
                {
                  ScenePk::attackFailToMe(curRevCmd,_entry);
                }
              }
            }
          }
          break;
        case Cmd::ATTACKTYPE_U2N:  /// �û�����Npc
          {
            SceneNpc *pNpc = SceneNpcManager::getMe().getNpcByTempID(curRevCmd->dwDefenceTempID);
            if (pNpc)
            {
              if (wdTarget&TARGET_PET)
              {
                if (!_entry->isEnemy(pNpc,false,true)&&_entry->checkMagicFlyRoute(pNpc,pSkillStatus->mode)&&pNpc->getPetType()==Cmd::PET_TYPE_PET)
                {
                  if (pNpc->skillStatusM.putOperationToMe(_entry->carrier,true))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
              }
              else if (wdTarget&TARGET_SUMMON)
              {
                if (!_entry->isEnemy(pNpc,false,true)&&_entry->checkMagicFlyRoute(pNpc,pSkillStatus->mode)&&pNpc->getPetType()==Cmd::PET_TYPE_SUMMON)
                {
                  if (pNpc->skillStatusM.putOperationToMe(_entry->carrier,true))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
              }
              else if (wdTarget&TARGET_NPC)
              {
                if (pNpc->getTopMaster()&&pNpc->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player)
                {
                  if (!(pNpc->isPkZone(_entry) && _entry->isPkZone(pNpc))) // �¼�&&this->isPkZone(pDef)
                  {
                    ScenePk::attackFailToMe(curRevCmd,_entry);
                    return;
                  }
                }

                if (_entry->isEnemy(pNpc)&& // �жϵ����Ѻͼ��PKģʽ��һ������,����ֻҪ�ж�һ�ξ�OK��
                  _entry->checkMagicFlyRoute(pNpc,pSkillStatus->mode))
                {
                  if (pNpc->skillStatusM.putOperationToMe(_entry->carrier))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
                else
                {
                  ScenePk::attackFailToMe(curRevCmd,_entry);
                }
              }
            }
          }
          break;
        case Cmd::ATTACKTYPE_N2N:  /// Npc����Npc
          {
            SceneNpc *pNpc = SceneNpcManager::getMe().getNpcByTempID(curRevCmd->dwDefenceTempID);
            if (pNpc)
            {
              if (wdTarget&TARGET_PET)
              {
                if (!_entry->isEnemy(pNpc,false,true)&&_entry->checkMagicFlyRoute(pNpc,pSkillStatus->mode)&&pNpc->getPetType()==Cmd::PET_TYPE_PET)
                {
                  if (pNpc->skillStatusM.putOperationToMe(_entry->carrier,true))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
              }
              else if (wdTarget&TARGET_SUMMON)
              {
                if (!_entry->isEnemy(pNpc,false,true)&&_entry->checkMagicFlyRoute(pNpc,pSkillStatus->mode)&&pNpc->getPetType()==Cmd::PET_TYPE_SUMMON)
                {
                  if (pNpc->skillStatusM.putOperationToMe(_entry->carrier,true))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                    return;
                  }
                  else
                  {
                    return;
                  }
                }
              }
              else if (wdTarget&TARGET_NPC)
              {
                if (_entry->getTopMaster() &&
                  _entry->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player &&
                  pNpc->getTopMaster() &&
                  pNpc->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player)
                {
                  if (!(pNpc->isPkZone(_entry) && _entry->isPkZone(pNpc))) // �¼�&&this->isPkZone(pDef)
                  {
                    ScenePk::attackFailToMe(curRevCmd,_entry);
                    return;
                  }
                }

                if (_entry->isEnemy(pNpc)&& // �жϵ����Ѻͼ��PKģʽ��һ������,����ֻҪ�ж�һ�ξ�OK��
                  _entry->checkMagicFlyRoute(pNpc,pSkillStatus->mode))
                {
                  if (pNpc->skillStatusM.putOperationToMe(_entry->carrier))
                  {
                    //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
                  }
                  return;
                }
              }
            }
          }
          break;
        case Cmd::ATTACKTYPE_U2P:  /// �û�������
          {
            zPos pd;
            DWORD num =0;
            pd.x = (DWORD)curRevCmd->xDes;
            pd.y = (DWORD)curRevCmd->yDes;
            if (findAttackTarget(pSkillStatus,pd,num))
            {
              //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
            }
            return;
          }
          break;
        case Cmd::ATTACKTYPE_U2B:  /// �û���������
        default:
          {
            //if (!ScenePk::attackUserCmdToNine(curRevCmd,_entry))
            //{
            //  return;
            //}
            return;
          }
          break;
      }
//      ScenePk::attackFailToMe(curRevCmd,_entry);
    }
    else
    {
      zPos center,pd;
      BYTE byDir=0;
      if (curRevCmd->dwDefenceTempID!=0)
      {
        switch(curRevCmd->byAttackType)
        {
          case Cmd::ATTACKTYPE_N2U:  /// Npc�����û�
          case Cmd::ATTACKTYPE_U2U:  /// Npc�����û�
            {
              SceneUser *pDef = pScene->getUserByTempID(curRevCmd->dwDefenceTempID);
              if (pDef)
              {
                center = pDef->getPos();
              }
              else
              {
                center.x = (DWORD)curRevCmd->xDes;
                center.y = (DWORD)curRevCmd->yDes;
              }
            }
            break;
          case Cmd::ATTACKTYPE_N2N:  /// Npc�����û�
          case Cmd::ATTACKTYPE_U2N:  /// Npc�����û�
            {
              SceneNpc *pNpc = SceneNpcManager::getMe().getNpcByTempID(curRevCmd->dwDefenceTempID);
              if (pNpc)
              {
                center = pNpc->getPos();
              }
              else
              {
                center.x = (DWORD)curRevCmd->xDes;
                center.y = (DWORD)curRevCmd->yDes;
              }
            }
            break;
          default:
            {
              center.x = (DWORD)curRevCmd->xDes;
              center.y = (DWORD)curRevCmd->yDes;
            }
            break;
        }
      }
      else
      {
        center.x = (DWORD)curRevCmd->xDes;
        center.y = (DWORD)curRevCmd->yDes;
      }

      switch(pSkillStatus->center)
      {
        case SKILL_CENTER_TYPE_MOUSE:
          {
//            center.x = (DWORD)curRevCmd->xDes;
//            center.y = (DWORD)curRevCmd->yDes;
            byDir = curRevCmd->byDirect;
            _entry->setDir(byDir);
#ifdef _DEBUG 
            Channel::sendSys(_entry->tempid,Cmd::INFO_TYPE_GAME,"���ĵ����ͣ���꣨%u,%u) ����:%u",center.x,center.y,byDir);
#endif
            // debug��ʾ
          }
          break;
        case SKILL_CENTER_TYPE_SELF:
          {
            center.x = _entry->getPos().x;
            center.y = _entry->getPos().y;
            byDir = curRevCmd->byDirect;//_entry->getDir();
            _entry->setDir(byDir);
#ifdef  _DEBUG 
            Channel::sendSys(_entry->tempid,Cmd::INFO_TYPE_GAME,"���ĵ����ͣ�����%u,%u) ����:%u",center.x,center.y,byDir);
#endif
          }
          break;
        default:
#ifdef _DEBUG 
          Channel::sendSys(_entry->tempid,Cmd::INFO_TYPE_GAME,"���ĵ����ͣ���д�������֤");
#endif
          break;
      }

      switch(pSkillStatus->range)
      {
        case 20:
          {
            DWORD tempLen = sizeof(struct Cmd::stAttackMagicUserCmd);
            DWORD itemNum = 0;
#ifdef _DEBUG
            Zebra::logger->error("!!!---�ͻ��˷������Ĺ���ָ��ԭʼ����[%u] ��ǰ����[%u]",tempLen,curRevCmdLen);
#endif
            if (curRevCmdLen>tempLen)
            {
              itemNum = (curRevCmdLen-tempLen)/sizeof(DWORD);
#ifdef _DEBUG
              Zebra::logger->error("!!!---�ͻ��˷������Ĺ����б���Ŀ[%u]",itemNum);
#endif
              if (itemNum >5) itemNum=5;
              for (DWORD i=0; i<itemNum; i++)
              {
                SceneNpc *pNpc=NULL;
                SceneUser *pUser=NULL;
#ifdef _DEBUG
                Zebra::logger->error("!!!---�ͻ��˷������ı�������ʱ����[%u]",curRevCmd->dwTempIDList[i]);
#endif
                if ((pNpc=_entry->scene->getNpcByTempID(curRevCmd->dwTempIDList[i]))!=NULL)
                {
                  pd = pNpc->getPos();
                }
                else if ((pUser=_entry->scene->getUserByTempID(curRevCmd->dwTempIDList[i]))!=NULL)
                {
                  pd = pUser->getPos();
                }
                else continue;
/*
                pd.x= (curRevCmd->dwTempIDList[i]>>16) &0xffff;
                pd.y= (curRevCmd->dwTempIDList[i]&0xff);
*/    
#ifdef _DEBUG
                Zebra::logger->error("!!!---�ͻ��˷������Ĺ�������[%u][x=%u,y=%u]",curRevCmd->dwTempIDList[i],pd.x,pd.y);
#endif

//-���ܷ�Χ����
#ifdef _DEBUG
                zObjectB *base = objectbm.get(501);
                if (base)
                {
                  zObject *o=zObject::create(base,1);
                  _entry->scene->addObject(o,pd);
                }
#endif
                DWORD num=0;
                if (!findAttackTarget(pSkillStatus,pd,num)) break;
              }
            }
          }
          break;
        case 21:
          {
            zPosVector range;
            pScene->findEntryPosInNine(_entry->getPos(),_entry->getPosI(),range);
            for(zPosVector::iterator iter = range.begin(); iter != range.end() ; iter ++)
            {
              pd = *iter;
//-���ܷ�Χ����
#ifdef _DEBUG
              zObjectB *base = objectbm.get(585);
              if (base)
              {
                zObject *o=zObject::create(base,randBetween(1,10));
                _entry->scene->addObject(o,pd);
              }
#endif
              DWORD num=0;
              if (!findAttackTarget(pSkillStatus,pd,num)) break;
            }
          }
          break;
        case 22:
          {
            if (_entry->getType() == zSceneEntry::SceneEntry_Player)
            {
              SceneUser *pUser = (SceneUser *)_entry;

			  TeamManager * team = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);

              if (team)
              {
				  SendStatusToTeamExec exec(pUser,pSkillStatus);
				  team->execEveryOne(exec);
              }
            }
          }
          break;
        case 50:
          {
            zPosVector range;
            int count=0;
            pScene->findEntryPosInOne(_entry->getPos(),_entry->getPosI(),range);
            for(zPosVector::iterator iter = range.begin(); iter != range.end() ; iter ++)
            {
              count++;
              if (count>20) break;
              pd = *iter;
//-���ܷ�Χ����
#ifdef _DEBUG
              zObjectB *base = objectbm.get(585);
              if (base)
              {
                zObject *o=zObject::create(base,randBetween(1,10));
                _entry->scene->addObject(o,pd);
              }
#endif
              DWORD num=0;
              if (!findAttackTarget(pSkillStatus,pd,num)) break;
            }
          }
          break;
        default:
          {
            SMagicRange range;
            DWORD maxCount =0;
            DWORD count=0;
            MagicRangeInit::getInstance().get(pSkillStatus->range,byDir % 2,range);
            maxCount = range.num;
            if (0==maxCount) maxCount = 65535;
            for(std::vector<RelativePos>::iterator iter = range.lib.begin(); iter != range.lib.end() ; iter ++)
            {
              SWORD rangDamageBonus=0;
              pd = iter->getAbsolutePos(center,byDir);
              rangDamageBonus = (*iter).w;
//-���ܷ�Χ����
#ifdef _DEBUG
              zObjectB *base = objectbm.get(585);
              if (base)
              {
                zObject *o=zObject::create(base,randBetween(1,10));
                _entry->scene->addObject(o,pd);
              }
#endif
              DWORD num=0;
              if (!findAttackTarget(pSkillStatus,pd,num,rangDamageBonus)) break;
              if (num>0)
              {
                count++;
                if (count>= maxCount) break;
              }
            }
          }
      }
      return;
    }
  }
}

/**
 * \brief Ͷ�Ͳ�����Ŀ���������
 * \param pSkillStatus ����
 * \param pd �����
 * \return ����Ƿ���true��ʾ���Լ����������������̷���false��ʾ����
 */
bool zSkill::findAttackTarget(const SkillStatus *pSkillStatus,zPos &pd,DWORD &count,SWORD rangDamageBonus)
{
  WORD wdTarget = pSkillStatus->target;
  Scene *pScene = _entry->scene;

  //��鷶Χ��Ч
  if (!pScene->zPosValidate(pd))
  {
    return true;
  }
  SceneUser *pFindUser = NULL;
  SceneNpc  *pFindNpc = NULL;
  if (wdTarget&TARGET_FRIEND)
  {
    pFindUser = pScene->getSceneUserByPos(pd);
    if (pFindUser/* && pFindUser!=_entry*/)
    {
      if (!_entry->isEnemy(pFindUser,false,true)&&_entry->checkMagicFlyRoute(pFindUser,pSkillStatus->mode))
      {
        count=1;
        if (pFindUser->skillStatusM.putOperationToMe(_entry->carrier,true,rangDamageBonus))
        {
          //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
          return true;
        }
        else
        {
          return false;
        }
      }
    }
  }
  if (wdTarget&TARGET_ENEMY)
  {
    if (!pFindUser) pFindUser = pScene->getSceneUserByPos(pd);
    if (pFindUser && pFindUser!=_entry)
    {
      if (_entry->getTopMaster()&&_entry->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player)
      {
        if (!(pFindUser->isPkZone(_entry) && _entry->isPkZone(pFindUser))) // �¼�&&this->isPkZone(pDef)
        {
          return true;
        }
      }

      if (_entry->isEnemy(pFindUser)&&
         _entry->checkMagicFlyRoute(pFindUser,pSkillStatus->mode))
      {
        count=1;
        if (pFindUser->skillStatusM.putOperationToMe(_entry->carrier,false,rangDamageBonus))
        {
          //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
          return true;
        }
        else
        {
          return false;
        }
      }
    }
  }

  if (!pFindUser)
  {
    if (wdTarget&TARGET_PET)
    {
      pFindNpc = pScene->getSceneNpcByPos(pd);
      if (pFindNpc/* && pFindNpc!=_entry*/)
      {
        if (!_entry->isEnemy(pFindNpc,false,true)&&_entry->checkMagicFlyRoute(pFindNpc,pSkillStatus->mode)&&pFindNpc->getPetType()==Cmd::PET_TYPE_PET)
        {
          count=1;
          if (pFindNpc->skillStatusM.putOperationToMe(_entry->carrier,true,rangDamageBonus))
          {
            //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
            return true;
          }
          else
          {
            return false;
          }
        }
      }
    }

    if (wdTarget&TARGET_SUMMON)
    {
      pFindNpc = pScene->getSceneNpcByPos(pd);
      if (pFindNpc/* && pFindNpc!=_entry*/)
      {
        if (!_entry->isEnemy(pFindNpc,false,true)&&_entry->checkMagicFlyRoute(pFindNpc,pSkillStatus->mode)&&pFindNpc->getPetType()==Cmd::PET_TYPE_SUMMON)
        {
          count=1;
          if (pFindNpc->skillStatusM.putOperationToMe(_entry->carrier,true,rangDamageBonus))
          {
            //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
            return true;
          }
          else
          {
            return false;
          }
        }
      }
    }

    if (wdTarget&TARGET_NPC)
    {
      if (!pFindNpc) pFindNpc = pScene->getSceneNpcByPos(pd);
      if (pFindNpc && pFindNpc!=_entry)
      {
        if (_entry->getTopMaster() &&
          _entry->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player &&
          pFindNpc->getTopMaster() &&
          pFindNpc->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player)
        {
          if (!(pFindNpc->isPkZone(_entry) && _entry->isPkZone(pFindNpc))) // �¼�&&this->isPkZone(pDef)
          {
            return true;
          }
        }
        if (_entry->isEnemy(pFindNpc)&&
           _entry->checkMagicFlyRoute(pFindNpc,pSkillStatus->mode))
        {
          count=1;
          if (pFindNpc->skillStatusM.putOperationToMe(_entry->carrier,false,rangDamageBonus))
          {
            //ScenePk::attackUserCmdToNine(curRevCmd,_entry);
            return true;
          }
          else
          {
            return false;
          }
        }
      }
    }
  }
  return true;
}

/*
	sky ���������������Լ�⺯�� begin
*/

bool zSkill::IsMagicSkill()	//sky ��⼼���Ƿ���ħ��ϵ����
{
	if(base->preskill3 & SPEC_MAGIC)
		return true;

	return false;
}

bool zSkill::IsPhysicsSkill()	//sky ��⼼���Ƿ�������ϵ����
{
	if(base->preskill3 & SPEC_PHYSICS)
		return true;

	return false;
}

bool zSkill::IsImmuneSkill()	//sky ��⼼���Ƿ��ܱ�����
{
	if(base->preskill3 & SPEC_IMMUNE)
		return true;

	return false;
}

bool zSkill::IsReboundSkill()	//sky ��⼼���Ƿ��ܱ�����
{
	if(base->preskill3 & SPEC_REBOUND)
		return true;

	return false;
}

bool zSkill::IsTreatmentSkill()	//sky ��⼼���Ƿ�������ϵ
{
	if(base->preskill3 & SPEC_TREATMENT)
		return true;

	return false;
}

bool zSkill::IsBuffSkill()		//sky ��⼼���Ƿ�������BUFFϵ
{
	if(base->preskill3 & SPEC_BUFF)
		return true;

	return false;
}

bool zSkill::IsDeBuffSkill()	//sky ��⼼���Ƿ��Ǽ���BUFFϵ
{
	if(base->preskill3 & SPEC_DEBUFF)
		return true;

	return false;
}

bool zSkill::IsDrugSkill()		//sky ��⼼���Ƿ��Ƕ�ϵ
{
	if(base->preskill3 & SPEC_DRUG)
		return true;

	return false;
}

bool zSkill::IsOtherSkill()		//sky ��⼼���Ƿ�������ϵ
{
	if(base->preskill3 & SPEC_OTHER)
		return true;

	return false;
}

/*
	sky ���������������Լ�⺯�� end
*/
