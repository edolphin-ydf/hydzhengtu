/**
 * \brief �������ʵ��
 *
 * 
 */
#include <zebra/ScenesServer.h>

//std::map<DWORD,ScenePet::petBonus> ScenePet::bonusTable;
petBonus bonusTable[] = 
{
  {0,100,100,100},
  {2,120,100,90},
  {3,120,120,100},
  {4,125,120,120}
};

/**
 * \brief ����
 */
ScenePet::ScenePet(Scene* scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype,zNpcB *abase)
:SceneNpc(scene,npc,define,type,entrytype,abase),speedUpOffMaster(false)
{
  this->type = Cmd::PET_TYPE_NOTPET;

  masterID = 0;
  masterType = 0;
  maxlevel = 2;

  //full_PetDataStruct(petData);
  petData.id = this->id;
  strncpy(petData.name,name,MAX_NAMESIZE-1);
  petData.lv = npc->level;
  petData.hp = this->getMaxHP();
  petData.maxhp = npc->hp;
  petData.exp = 0;
  petData.maxexp = npc->exp;
  petData.atk = getMinPDamage();
  petData.maxatk = getMaxPDamage();
  petData.matk = getMinMDamage();
  petData.maxmatk = getMaxMDamage();
  petData.def = npc->maxpdefence;
  petData.mdef = npc->maxmdefence;
  petData.str = npc->str;
  petData.intel = npc->inte;
  petData.agi = npc->dex;
  petData.men = npc->men;
  petData.vit = npc->con;
  petData.cri = npc->cri;

  bzero(&petData.skills[0],sizeof(petData.skills));
  std::vector<DWORD> list;
  if (npc->getAllSkills(list,petData.lv))
  {
    for (int i=list.size()-3>0?list.size()-3:0;i<(int)list.size();i++)
      if (i>=0)
        petData.skills[i] = list[i];
  }
  petData.ai = 0x0201; //sky ���������Ϊ����͹������ڹ������˵ĵ���ģʽ

  petData.maxhp_plus = 0;
  petData.atk_plus = 0;
  petData.maxatk_plus = 0;
  petData.pdef_plus = 0;
  petData.mdef_plus = 0;
  petData.type = this->type;
  petData.state = Cmd::PET_STATE_NORMAL;

  masterIsAlive = true;
  //needSave = false;

  delCount = 0;

  if(npc->id >= 46061 && npc->id <= 46070)
		this->catchme = 100;

}

/**
 * \brief �����ж�
 * \param entry �жϵĶ���
 * \param notify �ж�ʧ��ʱ�Ƿ���ʾ�����ܹ���10�����µ���ң�
 * \return 0:�ѷ� 1:���� -1:����
 */
int ScenePet::isEnemy(SceneEntryPk * entry,bool notify,bool good)
{
  int ret=0;
  bool isPlayer = false;
  if (entry)
  {
    if (type==Cmd::PET_TYPE_SEMI)
    {
      ret = SceneNpc::isEnemy(entry,notify,good);
    }
    else
    {
      SceneEntryPk * master = getMaster();
      if (master)
      {
        if (master->scene==scene)
        {
          if (scene->zPosShortRange(getPos(),master->getPos(),SCREEN_WIDTH*2,SCREEN_HEIGHT*2))
          {
            if (this!=entry)
            {
              if (!((entry->frenzy)||(frenzy)))
              {
                if (master->getType()==zSceneEntry::SceneEntry_Player)
                {
                  isPlayer = true;
                  //Zebra::logger->debug("isPetEnemy 119 %s->%s",master->name,entry->name);
                  ret = isUserMasterEnemy(entry);
                }
                else
                {
                  //Zebra::logger->debug("isPetEnemy 124 %s->%s",master->name,entry->name);
                  ret = master->isEnemy(entry,notify);
                }
              }
              else
                ret = 1;
            }
            else
              ret = 0;
          }
          else
            ret = -1;
        }
        else
          ret = -1;
      }
      else
        ret = -1;
    }
  }
  else
    ret = -1;

  if (isPlayer)
  {
    if (ret==1)
      return 1;
    else
      return 0;
  }
  else
  {
    return ret;
  }
}

/**
 * \brief �����������ʱ,�����˵���ݽ��е����ж�
 * \param entry �жϵĶ���
 * \return 0:�ѷ� 1:���� -1:����
 */
int ScenePet::isUserMasterEnemy(SceneEntryPk * entry)
{
	SceneEntryPk * temp = getTopMaster();
	SceneUser * tm = 0;//top master
	if (temp && temp->getType()==zSceneEntry::SceneEntry_Player)
		tm = (SceneUser*)temp;
	else return -1;

	// TODO �жϴ����ɫ�������Ƿ�Ϊ���ѹ�ϵ
	if (tm==entry) return 0;

	SceneEntryPk * entryMaster = entry->getTopMaster();

	using namespace Cmd;
	//if (PKMODE_ENTIRE==pkMode) return 1;

	switch (entryMaster->getType())
	{
	case zSceneEntry::SceneEntry_Player:
		{
			SceneUser *pUser = (SceneUser *)entryMaster;
			if (pUser == tm) return 0;

			bool def_gem = false;
			bool my_gem = false;

			if (tm->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER)
				|| tm->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON))
			{
				my_gem = true;
			}

			if (pUser->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER)
				|| pUser->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON))
			{
				def_gem = true;
			}

			if ((pUser->charbase.level<20)  && (!pUser->isWarRecord(Cmd::COUNTRY_FORMAL_DARE,tm->charbase.country)) && (!def_gem))
				return 0;
			if ((tm->charbase.level<20)  && (!tm->isWarRecord(Cmd::COUNTRY_FORMAL_DARE,pUser->charbase.country)) && (!my_gem))
				return 0;

			if (tm->scene == pUser->scene && !(my_gem || def_gem))
			{
				//if (tm->charbase.country == pUser->charbase.country)
				//{
				if (tm->charbase.level <= tm->scene->getPkLevel() && pUser->charbase.level >tm->scene->getPkLevel() 
					&& (!tm->isWarRecord(Cmd::SEPT_NPC_DARE,pUser->charbase.septid)))
				{
					return 0;
				}

				if (tm->charbase.level > tm->scene->getPkLevel() && pUser->charbase.level <= tm->scene->getPkLevel() 
					&& (!tm->isWarRecord(Cmd::SEPT_NPC_DARE,pUser->charbase.septid)))
				{
					return 0;
				}
				//}
				//else
				//{
				//  if (tm->charbase.level <= tm->scene->getPkLevel()-10 && pUser->charbase.level >tm->scene->getPkLevel()-10 
				//    && (!tm->isWarRecord(Cmd::SEPT_NPC_DARE,pUser->charbase.septid)))
				//  {
				//    return 0;
				//  }

				//  if (tm->charbase.level > tm->scene->getPkLevel()-10 && pUser->charbase.level <= tm->scene->getPkLevel()-10 
				//    && (!tm->isWarRecord(Cmd::SEPT_NPC_DARE,pUser->charbase.septid)))
				//  {
				//    return 0;
				//  }
				//}
			}

			if (tm->isDiplomatState() ==0 || pUser->isDiplomatState() == 0)
			{
				return 0;
			}

			switch (tm->pkMode)
			{
			case PKMODE_NORMAL:
				{
					return 0;
				}
				break;
			case PKMODE_TEAM:
				{
					//��ͬһ��ӻ�����������ħ��
					if ((tm->TeamThisID != 0) && (pUser->TeamThisID == tm->TeamThisID))
						return 0;
					else
						return 1;
				}
				break;
			case PKMODE_TONG:
				{
					if (tm->charbase.unionid != 0 && tm->charbase.unionid == pUser->charbase.unionid)
						return 0;
					else
						return 1;
				}
				break;
			case PKMODE_SEPT:
				{
					if (tm->charbase.septid != 0 && tm->charbase.septid == pUser->charbase.septid)
						return 0;
					else
						return 1;
				}
				break;
			case PKMODE_COUNTRY:
				{
					if (tm->charbase.country != 0 
						&& ((tm->charbase.country == pUser->charbase.country)
						|| (CountryAllyM::getMe().getFriendLevel(tm->charbase.country,
						pUser->charbase.country)>0
						&& (pUser->isSpecWar(Cmd::COUNTRY_FORMAL_DARE) 
						|| tm->isSpecWar(Cmd::COUNTRY_FORMAL_DARE)))))
						return 0;
					else
						return 1;
				}
				break;
			case PKMODE_GOODNESS:
				{
					if (pUser->isRedNamed(false)||pUser->charbase.country!=tm->charbase.country)
						return 1;
					else
						return 0;
				}
				break;
			case PKMODE_ALLY:
				{
					if ((CountryAllyM::getMe().getFriendLevel(pUser->charbase.country,tm->charbase.country)>0)||
						(pUser->charbase.country==tm->charbase.country))
						return 0;
					else
						return 1;
				}
				break;
			case PKMODE_ENTIRE:
				{
					if (pUser->isPkZone(tm)&&tm->isPkZone(pUser))
						return 1;
					else
						return 0;
				}
			default:
				break;
			}
			return 0;
		}
		break;
	case zSceneEntry::SceneEntry_NPC:
		{
			SceneNpc * n = (SceneNpc *)entry;

			if (n->id==COUNTRY_MAIN_FLAG  //�⼸�����������ж�
				|| n->id==COUNTRY_SEC_FLAG
				|| n->isMainGeneral()
				|| n->id==COUNTRY_KING_MAIN_FLAG
				|| n->id==COUNTRY_KING_SEC_FLAG
				|| n->id==COUNTRY_SEC_GEN
				|| n->id==COUNTRY_EMPEROR_MAIN_GEN
				|| n->id==COUNTRY_EMPEROR_SEC_GEN)
				return 1;

			//����npc
			if (!n->isBugbear())
			{
				if (n->npc->flags==1 && tm->charbase.country!=n->scene->getCountryID())
					return 1;
				else
					return -1;
			}

			SceneEntryPk * m = n->getMaster();
			if (m)
			{
				//Zebra::logger->debug("isPetEnemy 183 %s->%s",tm->name,n->getMaster()->name);
				if (!scene->zPosShortRange(n->getPos(),m->getPos(),20) && n->getPetType()==Cmd::PET_TYPE_GUARDNPC)
					return 1;
				else
					return tm->isEnemy(m);
			}

			if ((n->aif&AIF_ATK_REDNAME)||(n->npc->kind==NPC_TYPE_GUARD))
			{
				if (tm->isRedNamed()) return 1;
				if (tm->charbase.country!=tm->scene->getCountryID())
					return 1;
				if (tm->charbase.goodness&Cmd::GOODNESS_ATT)
					return 1;
			}
			switch (n->npc->kind)
			{
			case NPC_TYPE_HUMAN:                    ///����
			case NPC_TYPE_NORMAL:                   /// ��ͨ����
			case NPC_TYPE_BBOSS:                    /// ��Boss����
			case NPC_TYPE_LBOSS:                    /// СBoss����
			case NPC_TYPE_PBOSS:                    /// ��Boss����
			case NPC_TYPE_BACKBONE:                 /// ��Ӣ����
			case NPC_TYPE_GOLD:                             /// �ƽ�����
			case NPC_TYPE_SUMMONS:                  /// �ٻ�����
			case NPC_TYPE_AGGRANDIZEMENT:   /// ǿ������
			case NPC_TYPE_ABERRANCE:                /// ��������
			case NPC_TYPE_BACKBONEBUG:              /// ��������
			case NPC_TYPE_PET:      /// ��������
			case NPC_TYPE_TOTEM:                    /// ͼ������
			case NPC_TYPE_GHOST:		///Ԫ����
			case NPC_TYPE_TURRET:			/// ����
			case NPC_TYPE_BARRACKS:
			case NPC_TYPE_CAMP:
			case NPC_TYPE_ANIMON: /// ������
				//case NPC_TYPE_DUCKHIT:    /// ����
				return 1;
			case NPC_TYPE_GUARD:    /// ʿ������
			case NPC_TYPE_SOLDIER:    /// ʿ������
				{
					if (tm->charbase.country!=scene->getCountryID())
						return 1;
					if (tm->pkMode==PKMODE_ENTIRE)
						return 1;
					return 0;
				}
			case NPC_TYPE_UNIONGUARD:
				if (tm->isAtt(Cmd::UNION_CITY_DARE))
					return 1;
				else
					if (tm->scene->getUnionDare() && !tm->isSpecWar(Cmd::UNION_CITY_DARE)
						&& !n->isMainGeneral())//�󽫾����������ܴ�
						return 1;//������
					else                                                    
						return 0;//��ս�ڼ���ս���Ҳ��ǹ���,�����ط�
				break;
			case NPC_TYPE_UNIONATTACKER:
				if (tm->isAtt(Cmd::UNION_CITY_DARE))
					return 0;
				else
					if (tm->scene->getUnionDare() && !tm->isSpecWar(Cmd::UNION_CITY_DARE))
						return 1;//������
					else
						return 1;//��ս�ڼ���ս���Ҳ��ǹ���,�����ط�
				break;
			default:
				return -1;
			}
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}

}

/**
 * \brief ���ó�����Ϊ��ʽ
 * \param mode ģʽ
 */
void ScenePet::setPetAI(Cmd::petAIMode mode)
{
	if (mode&0xff00)//���ù���ģʽ
	{
		petData.ai &= 0x00ff;
		petData.ai |= mode;
		if (petData.ai&(Cmd::PETAI_ATK_NOATK|Cmd::PETAI_ATK_PASSIVE))
		{
			unChaseUser();
			setRecoverTime(SceneTimeTick::currentTime,3000);
		}
	}       
	if (mode&0x00ff)//�����ƶ�ģʽ
	{
		petData.ai &= 0xff00;
		petData.ai |= mode;
	}

	for (std::list<ScenePet *>::iterator it=totems.begin(); it!=totems.end(); it++)
	{
		(*it)->setPetAI((Cmd::petAIMode)petData.ai);
	}
	if (pet) pet->setPetAI((Cmd::petAIMode)petData.ai);
	if (summon) summon->setPetAI((Cmd::petAIMode)petData.ai);
}

/**
 * \brief �õ�������Ϊ��ʽ
 * \return ģʽ
 */
WORD ScenePet::getPetAI()
{
  return petData.ai;
}

/**
 * \brief �������ƶ�
 *
 * \return �Ƿ��ƶ��ɹ�
 */
bool ScenePet::moveToMaster()
{
#ifdef _DEBUG
  //Channel::sendNine(this,"ScenePet::moveToMaster()");
#endif
	if (type==Cmd::PET_TYPE_SEMI || type==Cmd::PET_TYPE_TURRET ) return false; //sky ��̨Ҳ�ǲ��ƶ���
  SceneEntryPk * master = getMaster();
  if (!master) return false;

  if ( petData.ai&Cmd::PETAI_MOVE_FOLLOW )
  {

	  AIC->setActRegion(master->getPos(),2,2);

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
			  if (0==masterID) return true;//���ڳɹ�ɾ��������

			  //����˲�Ʒ�Χ
			  if (!(scene->zPosShortRange(getPos(),master->getPos(),npc_pet_warp_region)))
				  return warp(master->getPos());

			  //�뿪��Զ����
			  int region = isFighting()?npc_pet_run_region+4:npc_pet_run_region;
			  if (!(scene->zPosShortRange(getPos(),master->getPos(),region)))
			  {
				  if (!speedUpOffMaster)
				  {
					  setSpeedRate(getSpeedRate()*2.0);
					  speedUpOffMaster = true;
				  }
			  }
			  else
			  {
				  //���������
				  region = isFighting()?npc_pet_chase_region+5:npc_pet_chase_region;
				  if ((scene->zPosShortRange(getPos(),master->getPos(),region)))
				  {
					  if (speedUpOffMaster)
					  {
						  resetSpeedRate();
						  speedUpOffMaster = false;
					  }
					  //check();
					  return false;
				  }
			  }

			  unChaseUser();
			  if (aif&AIF_WARP_MOVE)
				  return warp(master->getPos());
			  else
			  {
				  if (!gotoFindPath(getPos(),master->getPos()))
					  return goTo(master->getPos());
			  }
		  }
		  else
		  {
			  if (master->scene)
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
	  }
  }
  return false;
}

/**
 * \brief �õ����˵�ָ��
 *
 * \return ���˵�ָ��
 * 
 */
SceneEntryPk * ScenePet::getMaster()
{
  if (needClear()) return 0;

  SceneEntryPk * m = 0;
  switch (masterType)
  {
    case zSceneEntry::SceneEntry_Player:
      m = SceneUserManager::getMe().getUserByID(masterID);
      break;
    case zSceneEntry::SceneEntry_NPC:
      m = SceneNpcManager::getMe().getNpcByTempID(masterID);
      break;
    default:
      break;
  }
  if (!m && Cmd::PET_TYPE_GUARDNPC!=getPetType())
  {
#ifdef _DEBUG
    //Zebra::logger->info("%s �Ҳ������� masterTpye=%u masterID=%u needClear=%u",name,masterType,masterID,needClear());
#endif

    delCount++;
    if (delCount==100)
      Zebra::logger->info("%s �Ҳ������� masterTpye=%u masterID=%u needClear=%u %s(%u,%u)",name,masterType,masterID,needClear(),scene->name,pos.x,pos.y);

    //if (delCount>=10 && !needClear())
    //  delMyself();
  }
  return m;
}

/**
 * \brief �õ����ϲ����˵�ָ��
 * û�����˷����Լ�
 * \return ���˵�ָ��
 * 
 */
SceneEntryPk * ScenePet::getTopMaster()
{
  if (type==Cmd::PET_TYPE_SEMI) return this;
  SceneEntryPk * m = this;
  while ((m->getType()!=zSceneEntry::SceneEntry_Player) && (m->getMaster()))
    m = m->getMaster();
  if (!m) return this;
  return m;
}

/**
 * \brief �����Ƿ����
 *
 * \return �Ƿ����
 */
bool ScenePet::isRedNamed(bool allRedMode)
{
  if (type==Cmd::PET_TYPE_SEMI) return false;
  SceneEntryPk * master = getMaster();
  if (master)
    return master->isRedNamed(allRedMode);
  //Zebra::logger->error("ScenePet::isRedNamed(): %s û������",name);
  return false;
}

/**
 * \brief �ص����Χ
 *
 */
void ScenePet::returnToRegion()
{
  if (type==Cmd::PET_TYPE_SEMI) return;
  SceneEntryPk * master = getMaster();
  if (master)
  {
    if (!scene->zPosShortRange(master->getPos(),getPos(),SceneNpc::npc_pet_warp_region))
    {
      warp(master->getPos());
      return;
    }
    setSpeedRate(getSpeedRate()*2.0);
    return;
  }
}

/**
 * \brief ��������Ϣ�ṹ
 *
 *
 * \param data �ṹ��ַ
 */
void ScenePet::full_PetDataStruct(Cmd::t_PetData & data)
{
	 if ( npc->kind == NPC_TYPE_GHOST )  //��Ӱ�������Ǽ̳����˵����Բ���Ҫ����ֵ
	 {
		 petData.maxhp_plus = 0;
		 petData.atk_plus = 0;
		 petData.maxatk_plus = 0;
		 petData.matk_plus = 0;
		 petData.maxmatk_plus = 0;
		 petData.pdef_plus = 0;
		 petData.mdef_plus = 0;

		 if (hp>petData.maxhp )
			 hp = petData.maxhp;
		 petData.hp = hp;
	 }
	 else
	 {
		 petData.maxhp_plus = getMaxHP() - petData.maxhp;
		 petData.atk_plus = getMinPDamage() - petData.atk;
		 petData.maxatk_plus = getMaxPDamage() - petData.maxatk;
		 petData.matk_plus = getMinMDamage() - petData.matk;
		 petData.maxmatk_plus = getMaxMDamage() - petData.maxmatk;
		 petData.pdef_plus = getMaxPDefence() - petData.def;
		 petData.mdef_plus = getMaxMDefence() - petData.mdef;

		 if (hp>getMaxHP())
			 hp = getMaxHP();
		 petData.hp = hp;
	 }

#ifdef _DEBUG
  Zebra::logger->debug("���ͳ���hp plus=%u getMaxHP=%u - petData.maxhp=%u",petData.maxhp_plus,getMaxHP(),petData.maxhp);
#endif
  bcopy(&petData,&data,sizeof(petData),sizeof(data));
}

/**
 * \brief �Ƿ���������
 *
 * \return �Ƿ���������
 */
bool ScenePet::isActive()
{
  //if (aif&AIF_ACTIVE_MODE) return true;
  if (petData.ai&Cmd::PETAI_ATK_ACTIVE) return true;
  return false;
  //return ((aif&AIF_ACTIVE_MODE)||(petData.ai&Cmd::PETAI_ATK_ACTIVE));
}

/**
 * \brief �Ƿ����ս��
 *
 * \return �Ƿ����ս��
 */
bool ScenePet::canFight()
{
  if (aif&AIF_NO_BATTLE) return false;
  if (petData.ai&Cmd::PETAI_ATK_NOATK) return false;
  return true;
  //return ((!(aif&AIF_NO_BATTLE))||(petData.ai&Cmd::PETAI_ATK_PASSIVE));
}

/**
 * \brief �Ƿ�����ƶ�
 *
 * \return �Ƿ�����ƶ�
 */
bool ScenePet::canMove()
{
	if( type == Cmd::PET_TYPE_TURRET ) return false;  //sky ���ź�����̨�Ҳ��������߶�
	if (aif&AIF_NO_MOVE) return false;
	if (petData.ai&Cmd::PETAI_MOVE_STAND) return false;
	return moveAction;
  //return ((!(aif&AIF_NO_MOVE))||(petData.ai&Cmd::PETAI_MOVE_STAND));
}

/**
 * \brief ��������
 *
 *
 * \param m ����ָ��
 */
void ScenePet::setMaster(SceneEntryPk * m)
{
  if (!m) return;

  masterType = m->getType();;
  if (masterType==zSceneEntry::SceneEntry_Player)
    masterID = m->id;
  else
    masterID = m->tempid;

  if (this->type == Cmd::PET_TYPE_SUMMON )
    m->getSummonAppendDamage(this->appendMinDamage,this->appendMaxDamage);
}

/**
 * \brief ��������
 *
 *
 * \param master ����ָ��
 */
void ScenePet::setMaster(DWORD id,DWORD type)
{
  if (type!=zSceneEntry::SceneEntry_Player && type!=zSceneEntry::SceneEntry_NPC)
  {
    Zebra::logger->info("ScenePet::setMaster(): ��ͼ�� %s ���÷Ƿ����������� type=%u",name,type);
    return;
  }

  masterID = id;
  masterType = type;

  if (this->type==Cmd::PET_TYPE_SUMMON && getMaster())
    getMaster()->getSummonAppendDamage(appendMinDamage,appendMaxDamage);
}

/**
 * \brief �������ָ��
 *
 */
void ScenePet::clearMaster()
{
  masterID = 0;
  masterType = 0;
}

/**
 * \brief ���ó�������
 *
 *
 * \param petType ��������
 */
void ScenePet::setPetType(Cmd::petType petType)
{
  type = petType;
  petData.type = petType;
}

/**
 * \brief ��ȡ��������
 *
 *
 * \return ��������
 */
Cmd::petType ScenePet::getPetType()
{
  return type;
}

/**
 * \brief �������˵ĵ���
 *
 *
 * \param ret �ҵ���Ŀ��
 * \return �Ƿ��ҵ�
 */
bool ScenePet::checkMasterTarget(SceneEntryPk *&ret)
{
  if ((Cmd::PET_TYPE_RIDE==getPetType())
      || (Cmd::PET_TYPE_GUARDNPC==getPetType())
      || (Cmd::PET_TYPE_SEMI==getPetType()))
  {
    ret = 0;
    return false;
  }

  SceneEntryPk * master = getMaster();
  if (master)                     
  {                                       
    int r = npc_search_region;
    if (aif&AIF_DOUBLE_REGION)
      r *= 2;

    SceneEntryPk * tmp = 0;
    if (petData.ai&Cmd::PETAI_ATK_ACTIVE)//���˹����Ķ���
      tmp = master->getCurTarget();
    else if (petData.ai&Cmd::PETAI_ATK_PASSIVE)//�������˵Ķ���
      tmp = master->getDefTarget();

    if ((tmp)&&(tmp!=this))
      if (tmp->getState() == zSceneEntry::SceneEntry_Normal
          && isEnemy(tmp)
          && canReach(tmp)
          && canFight())
      {
        ret = tmp;
        return true;
      }
    return false;
  } 
  //Zebra::logger->error("ScenePet::checkMasterTarget(): %s û������",name);
  ret = 0;
  return false;
}

/**
 * \brief �����������
 *
 */
void ScenePet::petDeath()
{
  SceneEntryPk * master = getMaster();
  if (master)
  {
    /* //��������
       if (Cmd::PET_TYPE_RIDE==type&&zSceneEntry::SceneEntry_Player==master->getType())
       {
       SceneUser * m = (SceneUser *)master;
       Cmd::stDelPetPetCmd del;
       del.id= tempid;
       del.type = Cmd::PET_TYPE_RIDE;
       m->sendCmdToMe(&del,sizeof(del));

       m->horse.horse(0);
       Zebra::logger->info("%s �� %s ����",m->name,name);
       }
       */
    petData.state = Cmd::PET_STATE_DEAD;
    master->killOnePet(this);
  }
  setMoveTime(SceneTimeTick::currentTime,define->interval*1000);
}

/**
 * \brief ��ͻ��˷����Լ�������
 *
 */
void ScenePet::sendData()
{
  SceneEntryPk * master = getMaster();
  if (!master) return;
 
  if (Cmd::PET_TYPE_RIDE==type)
  {
    ((SceneUser *)master)->horse.sendData();
  }
  else
  {
    if (zSceneEntry::SceneEntry_Player==master->getType())
    {
      Cmd::stRefreshDataPetCmd ref;
      ref.type = type;
      ref.id = tempid;
      full_PetDataStruct(ref.data);
#ifdef _DEBUG
      Zebra::logger->debug("���ͳ�����Ϣ name=%s ai=%x hp=%u maxhp=%u",name,petData.ai,petData.hp,petData.maxhp);
#endif
      ((SceneUser *)master)->sendCmdToMe(&ref,sizeof(ref));
    }
  }
}

/**
 * \brief ��ͻ��˷����Լ���Ѫ�;���
 *
 */
void ScenePet::sendHpExp()
{
  SceneEntryPk * master = getMaster();
  if (!master) return;

  if (zSceneEntry::SceneEntry_Player==master->getType())
  {
    Cmd::stHpExpPetCmd ref;
    ref.type = type;
    ref.id = tempid;

	if( npc->kind == NPC_TYPE_GHOST )  //sky Ԫ�����⴦����
	{
		if(hp>petData.maxhp)
			hp = petData.maxhp;
	}
	else
	{
		if (hp>getMaxHP())
			hp = getMaxHP();
	}
    ref.hp = petData.hp = hp;
    ref.exp = petData.exp;
    ((SceneUser *)master)->sendCmdToMe(&ref,sizeof(ref));
  }
}

/**
 * \brief �õ�����ĵȼ�
 *
 * \return �ȼ�
 */
DWORD ScenePet::getLevel() const
{
  return petData.lv;
}

/**
 * \brief �������
 *
 */
bool ScenePet::runOffEnemy(SceneEntryPk_vec& enemies)
{
  return false;
}

/**
 * \brief ��������Χ2��������ƶ�
 * ������ֻ��1��ʱ�����,����1���Ҫ�����һ���Ƿ��ڻ��Χ��
 *
 * \return �Ƿ��ƶ��ɹ�
 */
bool ScenePet::randomMove()
{
	if(type == Cmd::PET_TYPE_TURRET) return false; //sky ��̨Ҳ�ǲ��ƶ���
	if (type == Cmd::PET_TYPE_SEMI) return SceneNpc::randomMove();
	if (!canMove()) return false;
	SceneEntryPk * master = getMaster();
	if (!master) return false;

	if (checkMoveTime(SceneTimeTick::currentTime) && canMove())
		if (selectByPercent(2)
			|| ((getPetType()==Cmd::PET_TYPE_CARTOON || getPetType()==Cmd::PET_TYPE_RIDE) 
			&& scene->checkBlock(getPos())
			&& scene->zPosShortRange(getPos(),master->getPos(),1)))
		{
			int dir = randBetween(0,7);
			zPos newPos;
			scene->getNextPos(pos,dir,newPos);
			zPosI newPosI = 0;
			scene->zPos2zPosI(newPos,newPosI);
			if (getPosI()==newPosI//������
				&& scene->zPosShortRange(master->getPos(),newPos,SceneNpc::npc_pet_chase_region))//����2����
				return shiftMove(dir);//����ƶ�
		}
		return false;
}

/**
 * \brief �������˹��฽�ӹ�����
 * \param mindamage ��С������
 * \param maxdamage ��󹥻���
 */
void ScenePet::setAppendDamage(WORD mindamage,WORD maxdamage)
{
  appendMinDamage = mindamage;
  appendMaxDamage = maxdamage;
  this->sendData();
}

/**
 * \brief ���ӳ��ﾭ��
 *
 *
 * \param num ����
 * \return �Ƿ��������
 */
bool ScenePet::addExp(DWORD num)
{
	if( type == Cmd::PET_TYPE_TURRET )  //sky ��̨Ҳ���Ի�ȡ�����𣿲�����
		return false;

	if (0==num) return false;
	if (getPetType()!=Cmd::PET_TYPE_PET
		&& getPetType()!= Cmd::PET_TYPE_SUMMON
		&& getPetType()!= Cmd::PET_TYPE_CARTOON) 
		return false;

	petData.exp += num;
	bool ret = false;
	if (petData.exp>=petData.maxexp)
	{
		levelUp();
		sendData();
	}
	else
		sendHpExp();

	addPetExp(num);

	return ret;
}

/**
 * \brief ���ӳ���ľ���
 *
 *
 * \param num ����
 */
void ScenePet::addPetExp(DWORD num)
{
  /*
     return;//����
     if (summon)
     if (summon->addExp(num))
     petLevelUp(summon);
     */
}

/**
 * \brief ��Ѫ
 *
 * \return �Ƿ�ɹ�
 */
bool ScenePet::recover()
{
  SceneNpc::recover();
  if ((petData.ai&Cmd::PETAI_ATK_NOATK)&&(checkRecoverTime(SceneTimeTick::currentTime)))
  {
    int hpr = this->getMaxHP()*2/100;
    if (hp+hpr>=this->getMaxHP())//npc->hp)
      hp = this->getMaxHP();
    else
      hp += hpr;
    setRecoverTime(SceneTimeTick::currentTime,3000);
    return true;
  }
  return false;
}

/**
 * \brief ��ȡ���˵ķ���ֵ
 * \return ���˷���ֵ
 */
DWORD ScenePet::getMasterMana()
{
  DWORD mana = 0;
  SceneEntryPk * master = getMaster();
  if (master)
    mana= master->getMP();
  return mana;
}

/**
 * \brief �ı䲢ˢ�½�ɫ����
 */
void  ScenePet::changeAndRefreshHMS(bool lock,bool sendData)
{
  this->resetSpeedRate();
  this->sendData();
  this->reSendData = false;
}

/**
 * \brief ����Ƿ���PK����
 * \param other PK�����
 * \return �Ƿ���PK����
 */
bool ScenePet::isPkZone(SceneEntryPk *other)
{
  SceneEntryPk * master = getMaster();
  if (!master) return false;

  return master->isPkZone(other);
}

/**
 * \brief ѡ�����
 *
 * \return ���˵�ָ��
 */
SceneEntryPk * ScenePet::chooseEnemy(SceneEntryPk_vec &enemies)
{
	if (type==Cmd::PET_TYPE_SEMI || type==Cmd::PET_TYPE_TURRET) return SceneNpc::chooseEnemy(enemies);
	//��ս��npc                     
	if (!canFight()) return false;          

	SceneEntryPk * ret = 0;                 

	SceneEntryPk * ct = 0;
	checkChaseAttackTarget(ct);
	if (ct && lockTarget)
	{
		return ct;
	}

	checkMasterTarget(ret);
	if (ret)
	{
		chaseSceneEntry(ret->getType(),ret->tempid);
		return ret;
	}
	else
		return SceneNpc::chooseEnemy(enemies);
}

/*
 * \brief �õ���С��������
 *
 *
 * \return ��С��������
 */
DWORD ScenePet::getMinPDamage()
{
  SDWORD value = 0;
  switch (type)
  {
    
    case Cmd::PET_TYPE_PET:
      value =  (SDWORD)((petData.atk+appendMinDamage+skillValue.uppetdamage-skillValue.dpdam-skillValue.theurgy_dpdam)*(1.0f+((float)boostupPet+(float)skillValue.protectUpAtt)/100.0f));
      if (value <0) value = 0;
      return value;
      break;
    case Cmd::PET_TYPE_SUMMON:
      return (DWORD)(SceneNpc::getMinPDamage()+boostupSummon);
      break;
    default:
      return SceneNpc::getMinPDamage();
  }
}

/*
 * \brief �õ������������
 *
 *
 * \return �����������
 */
DWORD ScenePet::getMaxPDamage()
{
  SDWORD value = 0;
  switch (type)
  {
    case Cmd::PET_TYPE_PET:
      value = (SDWORD)((petData.maxatk+appendMaxDamage+skillValue.uppetdamage-skillValue.dpdam-skillValue.theurgy_dpdam)*(1.0f+((float)boostupPet+(float)skillValue.protectUpAtt)/100.0f));
      if (value <0) value =0;
      return value;
      break;
    case Cmd::PET_TYPE_SUMMON:
      return (DWORD)( SceneNpc::getMaxPDamage()+boostupSummon);
      break;
    default:
      return SceneNpc::getMaxPDamage();
  }
}

/*
 * \brief �õ���Сħ��������
 *
 *
 * \return ��Сħ��������
 */
DWORD ScenePet::getMinMDamage()
{
  SDWORD value = 0;
  switch (type)
  {
    case Cmd::PET_TYPE_PET:
      value = (SDWORD)((petData.matk+appendMinDamage+skillValue.uppetdamage-skillValue.dmdam-skillValue.theurgy_dmdam)*(1.0f+((float)boostupPet+(float)skillValue.protectUpAtt)/100.0f));
      if (value<0) value =0;
      return value;
      break;
    case Cmd::PET_TYPE_SUMMON:
      return (DWORD)(SceneNpc::getMinMDamage()+boostupSummon);
      break;
    default:
      return SceneNpc::getMinMDamage();
  }
}

/*
 * \brief �õ����ħ��������
 *
 *
 * \return ���ħ��������
 */
DWORD ScenePet::getMaxMDamage()
{
  SDWORD value = 0;
  switch (type)
  {
    case Cmd::PET_TYPE_PET:
      value = (SDWORD)((petData.maxmatk+appendMinDamage+skillValue.uppetdamage-skillValue.dmdam-skillValue.theurgy_dmdam)*(1.0f+((float)boostupPet+(float)skillValue.protectUpAtt)/100.0f));
      if (value <0) value =0;
      return value;
      break;
    case Cmd::PET_TYPE_SUMMON:
      return (DWORD)(SceneNpc::getMaxMDamage()+boostupSummon);
      break;
    default:
      return SceneNpc::getMaxMDamage();
  }
}

/*
 * \brief �õ���С���������
 *
 *
 * \return ��С���������
 */
DWORD ScenePet::getMinPDefence()
{
  if (type==Cmd::PET_TYPE_PET)
  {
    SDWORD value = (SDWORD)((petData.def+skillValue.uppetdefence-skillValue.theurgy_dpdef)*(1.0f+((float)boostupPet)/100.0f));
    if (value <0) value =0;
    return value;
  }
  else
    return SceneNpc::getMinPDefence();
}

/*
 * \brief �õ�������������
 *
 *
 * \return ������������
 */
DWORD ScenePet::getMaxPDefence()
{
  if (type==Cmd::PET_TYPE_PET)
  {
    SDWORD value = (SDWORD)((petData.def+skillValue.uppetdefence-skillValue.theurgy_dpdef)*(1.0f+((float)boostupPet)/100.0f));
    if (value <0) value = 0;
    return value;
  }
  else
    return SceneNpc::getMaxPDefence();
}

/*
 * \brief �õ����ħ��������
 *
 *
 * \return ���ħ��������
 */
DWORD ScenePet::getMinMDefence()
{
  if (type==Cmd::PET_TYPE_PET)
  {
    SDWORD value = (SDWORD)((petData.mdef+skillValue.uppetdefence+this->boostupPetMDef-skillValue.theurgy_dmdef)*(1.0f+((float)boostupPet)/100.0f));
    value = (SDWORD)(value *( 1 - skillValue.dmdefp/100.0f));
    if (value <0) value =0;
    return value;
  }
  else
    return SceneNpc::getMinMDefence();
}

/*
 * \brief �õ����ħ��������
 *
 *
 * \return ���ħ��������
 */
DWORD ScenePet::getMaxMDefence()
{
  if (type==Cmd::PET_TYPE_PET)
  {
    SDWORD value = (SDWORD)((petData.mdef+skillValue.uppetdefence+this->boostupPetMDef-skillValue.theurgy_dmdef)*(1.0f+((float)boostupPet)/100.0f));
    value = (SDWORD)(value *( 1 - skillValue.dmdefp/100.0f));
    if (value <0) value =0;
    return value;
  }
  else
    return SceneNpc::getMaxMDefence();
}

/*
 * \brief �ӱ���ȡ���������
 *
 */
void ScenePet::getAbilityByLevel(DWORD level)
{
  BYTE aType = getAType();

  if (aType>0)
  {
    zPetB * base = petbm.get((aType-1)*200+level);
    if (!base) return;

    petData.hp = base->hp;
    hp = base->hp;
    petData.maxhp = base->hp;
    petData.maxexp = base->exp;
    petData.atk = base->atk;
    petData.maxatk = base->maxatk;
    petData.matk = base->matk;
    petData.maxmatk = base->maxmatk;
    petData.def = base->def;
    petData.mdef = base->mdef;

    petData.str = base->str;
    petData.intel = base->intel;
    petData.agi = base->agi;
    petData.men = base->men;
    petData.vit = base->vit;
    petData.cri = base->cri;

    for (DWORD i=1; i<(sizeof(bonusTable)/sizeof(petBonus)); i++)
    {
      if (npc->bear_type==bonusTable[i].type)
      {
        petData.maxhp = petData.maxhp*bonusTable[i].hpB/100;
        petData.hp = petData.maxhp;
        hp = petData.maxhp;

        petData.atk = petData.atk*bonusTable[i].atkB/100;
        petData.maxatk = petData.maxatk*bonusTable[i].atkB/100;
        petData.matk = petData.matk*bonusTable[i].atkB/100;
        petData.maxmatk = petData.maxmatk*bonusTable[i].atkB/100;

        petData.def = petData.def*bonusTable[i].defB/100;
        petData.mdef = petData.mdef*bonusTable[i].defB/100;
      }
    }

    bzero(&petData.skills[0],sizeof(petData.skills));
    std::vector<DWORD> list;
    if (npc->getAllSkills(list,level))
    {
      for (int i=list.size()-3>0?list.size()-3:0;i<(int)list.size();i++)
        if (i>=0)
          petData.skills[i] = list[i];
    }
  }
  else
    Zebra::logger->debug("npc %s δ֪�Ĺ������� type=%d",name,aType);
  /*
     std::vector<DWORD> list;
     if (base->getAllSkills(list))
     {
     for (DWORD i=0;i<list.size()&&i<4;i++)
     petData.skills[i] = list[i];
     }
     */


  //petData.ai = 0x0201;

  /*
     PetData.maxhp_plus = getMaxHP()-npc->hp;
     PetData.atk_plus = getMinPDamage()-PetData.atk;
     PetData.maxatk_plus = getMaxMDamage()-PetData.maxmatk;
     PetData.def_plus = getMaxPDefence()-npc->maxpdefence;
     */
}

/*
 * \brief ��������
 *
 *
 */
void ScenePet::levelUp()
{
	if( type == Cmd::PET_TYPE_TURRET ) return; //sky ���ѣ���̨�ǲ����ȡ����������

	SceneEntryPk * master = getMaster();
	if (!master) return;
	if (master->getType()!=zSceneEntry::SceneEntry_Player) return;

	DWORD maxlv = 1;
	switch (getPetType())
	{
	case Cmd::PET_TYPE_PET:
		{
			maxlv = ((SceneUser *)master)->charbase.level+5;
			while ((petData.exp>=petData.maxexp)&&(petData.lv<maxlv))
			{ 
				petData.exp -= petData.maxexp;
				petData.lv++;
				getAbilityByLevel(petData.lv);
				hp = getMaxHP();
			}
			if (petData.exp>petData.maxexp)
				petData.exp = petData.maxexp;
		}
		break;
	case Cmd::PET_TYPE_SUMMON:
		{
			while ((petData.exp>=petData.maxexp)&&(petData.lv<maxlevel))
			{
				zNpcB *base = npcbm.get(id+1);
				if (!base) break;

				petData.exp -= petData.maxexp;
				petData.maxexp = base->exp;
				petData.lv++;
				npc = base;
				id++;
				petData.id++; 
				hp = npc->hp;
			}
			if (petData.exp>petData.maxexp)
				petData.exp = petData.maxexp;
		}
		break;
	case Cmd::PET_TYPE_CARTOON:
		{
			maxlv = ((SceneUser *)master)->charbase.level;
			while ((petData.exp>=petData.maxexp)&&(petData.lv<maxlv))
			{
				petData.exp -= petData.maxexp;
				petData.lv++;
				/*
				petData.maxexp = base->exp;
				npc = base;
				id++;
				petData.id++; 
				hp = npc->hp;
				*/
			}
			if (petData.exp>petData.maxexp)
				petData.exp = petData.maxexp;
		}
		break;
	case Cmd::PET_TYPE_GUARDNPC:
	case Cmd::PET_TYPE_RIDE:
	default:
		return;
	}
	sendMeToNine();
}

DWORD ScenePet::getBaseMaxHP()
{
  if (type==Cmd::PET_TYPE_PET)
    return (DWORD)(petData.maxhp+(anpc?anpc->hp:0));
  else
    return SceneNpc::getBaseMaxHP();
}

/*
 * \brief �õ�������������ֵ
 *
 * 
 * \return �������ֵ
 */
DWORD ScenePet::getMaxHP()
{
  if (type==Cmd::PET_TYPE_PET)
    return (DWORD)((petData.maxhp+(anpc?anpc->hp:0)+skillValue.maxhp)*(1.0f+boostupPet/100.0f));
  else
    return SceneNpc::getMaxHP();
}

void ScenePet::full_t_MapPetData(Cmd::t_MapPetData &data)
{
  data.tempID = tempid;
  strncpy(data.name,petData.name,MAX_NAMESIZE);

  bzero(data.masterName,sizeof(data.masterName));
  if (getMaster()&&getPetType()!=Cmd::PET_TYPE_SEMI)
  {
    data.masterType = getMaster()->getType();
    data.masterID = getMaster()->tempid;
    if (zSceneEntry::SceneEntry_Player==data.masterType
        &&((SceneUser *)getMaster())->mask.is_masking())
      strncpy(data.masterName,"������",MAX_NAMESIZE-1);
    else
      strncpy(data.masterName,getMaster()->name,MAX_NAMESIZE-1);
  }
  else
    data.masterID = (DWORD)-1;
  data.pet_type = getPetType();
}

void ScenePet::sendMeToNine()
{
  Cmd::stAddMapNpcPetMapScreenUserCmd ret; 
  full_t_MapNpcDataAndPos(ret.data);
  full_t_MapPetData(ret.pet);
  if (npc->kind == NPC_TYPE_TRAP)
  {
    SceneEntryPk * pMaster = getMaster();
    if (pMaster && pMaster->getType()==zSceneEntry::SceneEntry_Player)
    {
      ((SceneUser *)pMaster)->sendCmdToMe(&ret,sizeof(ret));
    }
  }
  else
  {
    scene->sendCmdToNine(getPosI(),&ret,sizeof(ret),this->dupIndex);
  }
}

//��9�����ͳ�������
void ScenePet::sendPetDataToNine()
{
  Cmd::stAddMapPetMapScreenUserCmd ret;
  full_t_MapPetData(ret.pet);
  if (npc->kind == NPC_TYPE_TRAP)
  {
    SceneEntryPk * pMaster = getMaster();
    if (pMaster && pMaster->getType()==zSceneEntry::SceneEntry_Player)
    {
      ((SceneUser *)pMaster)->sendCmdToMe(&ret,sizeof(ret));
    }
  }
  else
  {
    scene->sendCmdToNine(getPosI(),&ret,sizeof(ret),this->dupIndex);
  }
}

void ScenePet::delMyself()
{
  if (masterType==zSceneEntry::SceneEntry_Player)
    Zebra::logger->debug("[����]pet %s ���Ҳ�������(%u)��ɾ�� %s(%u,%u)",name,masterID,scene->name,pos.x,pos.y);
  else
    Zebra::logger->debug("[����]pet %s ���Ҳ������˶�ɾ�� %s(%u,%u)",name,scene->name,pos.x,pos.y);
  setClearState();
}
