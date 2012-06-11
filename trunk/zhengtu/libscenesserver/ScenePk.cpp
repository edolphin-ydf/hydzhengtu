/**
  * \brief PK�ļ�,����PK��ص�ʵ�ַ�������
  * 
  */
#include <zebra/ScenesServer.h>

#define NOTE

/**
 * \brief ���캯��
 */
SkillState::SkillState()
{
  bzero(swdValue,sizeof(swdValue));
}

/**
 * \brief ��ʼ������״̬
 */
void SkillState::init()
{
  // �����⼸��ֵ�ǳ���������ǿ���״̬,������ǿ����֮һ������ħ��ֵ����ǿ�Ժ����������ֵ��������ʱ���
  SWORD bk_uppetdamage = uppetdamage; //�����ٻ��޵Ĺ����� 
  SWORD bk_uppetdefence= uppetdefence; //�����ٻ��޵ķ�����
  SDWORD bk_maxhp     = maxhp;       //����ֵ���ֵ���

  WORD bk_introject_maxmdam = introject_maxmdam;
  WORD bk_introject_maxpdam  = introject_maxpdam;
  WORD bk_introject_mdam  = introject_mdam;
  WORD bk_introject_pdam  = introject_pdam;
  WORD bk_introject_mdef  = introject_mdef;
  WORD bk_introject_pdef  = introject_pdef;
  WORD bk_introject_maxhp = introject_maxhp;

  bzero(swdValue,sizeof(swdValue));

  uppetdamage = bk_uppetdamage; //�����ٻ��޵Ĺ�����
  uppetdefence= bk_uppetdefence; //�����ٻ��޵ķ�����
  maxhp    = bk_maxhp;       //����ֵ���ֵ���
  introject_maxmdam  = bk_introject_maxmdam;
  introject_maxpdam  = bk_introject_maxpdam;
  introject_mdam    = bk_introject_mdam;
  introject_pdam    = bk_introject_pdam;
  introject_mdef    = bk_introject_mdef;
  introject_pdef    = bk_introject_pdef;
  introject_maxhp    = bk_introject_maxhp;
}

/**
 * \brief ������յ���PK��Ϣ
 * 
 * \rev ���յ����ݵĵ�ַ
 *
 * \return �����ɹ�����TRUE,���򷵻�FALSE
 */
// [ranqd] �������
bool SceneUser::attackMagic(const Cmd::stAttackMagicUserCmd *rev,const DWORD cmdLen)
{
  attackTarget = NULL; // ��װ�����Եĸ���״̬Ͷ��Ŀ�����óɿ�

  //sky ���ӳɱ�־��ʼ��
  Daggerflag = false;
  Throflag = false;
  Handsflag = false;
  Handflag = false;
  Flameflag = false;
  Iceflag = false;
  Sacredflag = false;
  Darkflag = false;	
  
  if (this->isQuiz)
  {
    ScenePk::attackFailToMe(rev,this);
    return true;
  }

  //sky ���״̬�²����Խ��й���
  if(Soulflag)
	  return true;
    
  if (this->isSitdown())
  {
    ScenePk::attackFailToMe(rev,this);
    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�㴦�ڴ���״̬��");
    return true;
  }

  //sky �������������˳������ڹ���
  if (hideme)
  {
	 skillStatusM.clearRecoveryElement(241);
  }

  // ���ɷ񹥻���־
  if (!attackAction)
  {
    ScenePk::attackFailToMe(rev,this);
    return true;
  }

  SceneEntryPk * def = 0;

  if (Cmd::ATTACKTYPE_U2U==rev->byAttackType)
  {
	  def = scene->getUserByTempID(rev->dwDefenceTempID);
	  if(!def)
	  {
		  Zebra::logger->error("%s �û�������һ����ЧĿ��!", this->name);
		  return false;
	  }
  }
  else if(Cmd::ATTACKTYPE_U2N==rev->byAttackType)
  {
	  def = SceneNpcManager::getMe().getNpcByTempID(rev->dwDefenceTempID);
	  if(!def)
	  {
		  Zebra::logger->error("%s �û�������һ����ЧĿ��!", this->name);
		  return false;
	  }
  }

  //TODO ����ܷ񹥻������м�������Լ��޷�����
  //����������
  //��ʱ�������ܹ���
  if (horse.mount()&& !horse.canFight())
  {
    ScenePk::attackFailToMe(rev,this);
    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�Բ���,��������ս��");
    return true;
  }

  if (this->assault)
  {
    this->skillStatusM.clearRecoveryElement(121);
    this->reSendMyMapData();
  }
  else if (!ScenePk::checkAttackSpeed(this,rev)) 
  {
    ScenePk::attackFailToMe(rev,this,true,true);
    return true;
  }


  //������װ���;ö�����
  this->packs.equip.costAttackDur(this);

  this->skillValue.init();
  this->skillStatusM.processPassiveness();// �����ҵı���״̬Ӱ��
  this->pkValue.damagebonus=0;

  if (Cmd::ATTACKTYPE_U2U==rev->byAttackType)
    def = scene->getUserByTempID(rev->dwDefenceTempID);
  else if (Cmd::ATTACKTYPE_U2N==rev->byAttackType)
    def = SceneNpcManager::getMe().getNpcByTempID(rev->dwDefenceTempID);
  if (def)
    setPetsChaseTarget(def);


  /*SceneEntryPk * def = 0;
  if (Cmd::ATTACKTYPE_U2U==rev->byAttackType)
    def = scene->getUserByTempID(rev->dwDefenceTempID);
  else if (Cmd::ATTACKTYPE_U2N==rev->byAttackType)
    def = SceneNpcManager::getMe().getNpcByTempID(rev->dwDefenceTempID);
  else if (Cmd::ATTACKTYPE_U2N==rev->byAttackType)
  {
    def = SceneNpcManager::getMe().getNpcByTempID(rev->dwDefenceTempID);
    if ((def)&&((((SceneNpc*)def)->aif&AIF_ATK_REDNAME)||(((SceneNpc*)def)->npc->kind==NPC_TYPE_GUARD)))
      if (pkState.addProtect(this))
      {
        Cmd::stAddUserMapScreenUserCmd send;
        full_t_MapUserData(send.data);
        scene->sendCmdToNine(getPosI(),&send,sizeof(send),false);
        Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�㹥���� %s,��������������ҿ��Զ�����������",def->name);
      }
  }
  if (def)
  {
    setCurTarget(def);
    if (!def->isFighting())
      def->setCurTarget(this);
  }
  */

  switch (rev->wdMagicType)
  {
  case SERVER_SKILL_ATTACK_NORMAL:		//����������ͨ����
  case SERVER_SKILL_DAGGER_ATTACK_NORMAL: //˫������(ذ��)��ͨ����
  case SERVER_SKILL_DART_ATTACK_NORMAL:	//����������ͨ����
  case SERVER_SKILL_HANDS_ATTACK_NORMAL:	//˫��������ͨ����
	  {
		  switch (rev->byAttackType)
		  {
		  case Cmd::ATTACKTYPE_U2U:
			  {
				  //��ֹ����
				  if (this->tempid == rev->dwDefenceTempID)
				  {
					  ScenePk::attackUserCmdToNine(rev,this);
					  return true;
				  }

				  SceneUser *pDef = this->scene->getUserByTempID(rev->dwDefenceTempID);

				  if (pDef && pDef->isPkZone(this)&&this->isPkZone(pDef))// �¼�&&this->isPkZone(pDef)
				  {
					  pDef->isPhysics = true;
					  if (rev->wdMagicType==SERVER_SKILL_DART_ATTACK_NORMAL)
					  {
						  //sky �����Ͷ����������ͨ����
						  Throflag = true;

						  if (this->skillValue.introject_mdam==0) //����ǷǺ���״̬
						  {
							  if (!this->reduce(BOW_ARROW_ITEM_TYPE,1))
							  {
								  ScenePk::attackFailToMe(rev,this);
								  return true;
							  }
						  }
					  }

					  bool bATT = false;
					  switch(rev->wdMagicType)
					  {
					  case SERVER_SKILL_ATTACK_NORMAL:
						  Handflag = true;
						  bATT = true;
						  break;
					  case SERVER_SKILL_DAGGER_ATTACK_NORMAL:
						  Daggerflag = true;
						  bATT = true;
						  break;
					  case SERVER_SKILL_HANDS_ATTACK_NORMAL:
						  Handsflag = true;
						  bATT = true;
						  break;
					  }

					  if (this->isEnemy(pDef,true)&& // �жϵ����Ѻͼ��PKģʽ��һ������,����ֻҪ�ж�һ�ξ�OK��
						  this->checkMagicFlyRoute(pDef,bATT == true?AttackNear:AttackFly))
					  {
						  if (!ScenePk::attackUserCmdToNine(rev,this))
						  {
							  return true;
						  }

						  if (!pDef->preAttackMe(this,rev,true))
						  {
							  //      ScenePk::attackFailToMe(rev,this,false);
							  return true;
						  }

						  bool physicsatt = true;

						  if (!pDef->AttackMe(this,rev,physicsatt))
						  {
							  ScenePk::attackFailToMe(rev,this);
							  return true;
						  }
						  appendAttack(rev);
						  return true;
					  }
					  else
					  {
						  ScenePk::attackFailToMe(rev,this);
						  return true;
					  }
				  }
				  else
				  {
					  //ΪԪ���¼Ӵ���
					  SceneNpc *NpcDef = SceneNpcManager::getMe().getNpcByTempID(rev->dwDefenceTempID);
					  if( NpcDef )
					  {
						  if( NpcDef->npc->kind == NPC_TYPE_GHOST )
						  {
							  NpcDef->isPhysics = true;
							  if ((NpcDef->getPetType()!=Cmd::PET_TYPE_NOTPET)
								  && NpcDef->getMaster())
							  {
								  if (NpcDef->scene!=NpcDef->getMaster()->scene
									  ||!scene->zPosShortRange(NpcDef->getPos(),NpcDef->getMaster()->getPos(),20))
								  {
								  }
								  /*else if (!(NpcDef->isPkZone(this) && this->isPkZone(pDef)))
								  {
									  ScenePk::attackUserCmdToNine(rev,this);
									  ScenePk::attackFailToMe(rev,this);
									  return true;
								  }*/
							  }

							  if (!isEnemy(NpcDef))
							  {
								  ScenePk::attackFailToMe(rev,this);
								  return true;
							  }

							  bool bATT = false;
							  switch(rev->wdMagicType)
							  {
							  case SERVER_SKILL_ATTACK_NORMAL:
								  Handflag = true;
								  bATT = true;
								  break;
							  case SERVER_SKILL_DAGGER_ATTACK_NORMAL:
								  Daggerflag = true;
								  bATT = true;
								  break;
							  case SERVER_SKILL_HANDS_ATTACK_NORMAL:
								  Handsflag = true;
								  bATT = true;
								  break;
							  case SERVER_SKILL_DART_ATTACK_NORMAL:
								  Throflag = true;
								  bATT =true;
								  break;
							  }

							  if (this->checkMagicFlyRoute(NpcDef,bATT == true?AttackNear:AttackFly))
							  {
								  if (!NpcDef->preAttackMe(this,rev,true))
								  {
									  return true;
								  }

								  if (!ScenePk::attackUserCmdToNine(rev,this))
								  {
									  return true;
								  }

								  bool physicsatt = true;

								  if (!NpcDef->AttackMe(this,rev,physicsatt))
								  {
									  ScenePk::attackFailToMe(rev,this);
									  return true;
								  }
								  appendAttack(rev);
								  return true;
							  }
							  else
							  {
								  ScenePk::attackUserCmdToNine(rev,this);
								  ScenePk::attackFailToMe(rev,this);
								  return true;
							  }
						  }
						  else
						  {
							  ScenePk::attackFailToMe(rev,this);
						  }
						  break;
					  }
					  else
					  {
						  ScenePk::attackUserCmdToNine(rev,this);
						  ScenePk::attackFailToMe(rev,this);
						  //Zebra::logger->debug("�����dwDefenceTempID.(%ld,%ld)",rev->dwUserTempID,rev->dwDefenceTempID);
					  }
				  }
		  }  
		  break;

		  case Cmd::ATTACKTYPE_U2N:
			  {
				  SceneNpc *pDef = SceneNpcManager::getMe().getNpcByTempID(rev->dwDefenceTempID);
				  if (pDef)
				  {
					  pDef->isPhysics = true;
					  if ((pDef->getPetType()!=Cmd::PET_TYPE_NOTPET)
						  && pDef->getMaster())
					  {
						  if (pDef->scene!=pDef->getMaster()->scene
							  ||!scene->zPosShortRange(pDef->getPos(),pDef->getMaster()->getPos(),20))
						  {
						  }
						  else if (!(pDef->isPkZone(this) && this->isPkZone(pDef)))
						  {
							  ScenePk::attackUserCmdToNine(rev,this);
							  ScenePk::attackFailToMe(rev,this);
							  return true;
						  }
					  }

					  if (!isEnemy(pDef))
					  {
						  ScenePk::attackFailToMe(rev,this);
						  return true;
					  }

					  if (rev->wdMagicType==SERVER_SKILL_DART_ATTACK_NORMAL)
					  {
						  //sky �����Ͷ����������ͨ����
						  Throflag = true;

						  if (this->skillValue.introject_mdam==0) //����ǷǺ���״̬
						  {
							  if (!this->reduce(BOW_ARROW_ITEM_TYPE,1))
							  {
								  ScenePk::attackFailToMe(rev,this);
								  return true;
							  }
						  }
					  }

					  bool bATT = false;
					  switch(rev->wdMagicType)
					  {
					  case SERVER_SKILL_ATTACK_NORMAL:
						  Handflag = true;
						  bATT = true;
						  break;
					  case SERVER_SKILL_DAGGER_ATTACK_NORMAL:
						  Daggerflag = true;
						  bATT = true;
						  break;
					  case SERVER_SKILL_HANDS_ATTACK_NORMAL:
						  Handsflag = true;
						  bATT = true;
						  break;
					  }

					  if (this->checkMagicFlyRoute(pDef,bATT == true?AttackNear:AttackFly))
					  {
						  if (!pDef->preAttackMe(this,rev,true))
						  {
							  //  ScenePk::attackFailToMe(rev,this,false);
							  return true;
						  }

						  if (!ScenePk::attackUserCmdToNine(rev,this))
						  {
							  return true;
						  }

						  bool physicsatt = true;

						  if (!pDef->AttackMe(this,rev,physicsatt))
						  {
							  ScenePk::attackFailToMe(rev,this);
							  return true;
						  }
						  appendAttack(rev);

						  //sky �����Լ�ΪPVE״̬
						  if(IsPveOrPvp() != USE_FIGHT_PVP && IsPveOrPvp() != USE_FIGHT_PVE)	//sky �ȼ���Լ����Ǵ���pvp״̬
						  {
							  SetPveOrPvp(USE_FIGHT_PVE);		//sky �����Լ���ս��״̬Ϊpveģʽ
							  Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"���ѽ���pveģʽ");
						  }
						  //sky ͬʱˢ��ս��ʱ��
						  SetPkTime();

						  return true;
					  }
					  else
					  {
						  ScenePk::attackUserCmdToNine(rev,this);
						  ScenePk::attackFailToMe(rev,this);
						  return true;
					  }
				  }
				  else
				  {
					  ScenePk::attackFailToMe(rev,this);
					  //Zebra::logger->debug("�����dwDefenceTempID.(AID=%ld,DID=%ld)",rev->dwUserTempID,rev->dwDefenceTempID);
				  }
			  }
			  break;

		  case Cmd::ATTACKTYPE_U2B:
			  {
				  if (!ScenePk::attackUserCmdToNine(rev,this))
				  {
					  return true;
				  }
				  //return true;
			  }
			  break;

		  case Cmd::ATTACKTYPE_U2P:
			  {
				  ScenePk::attackUserCmdToNine(rev,this);
			  }
			  break;

		  default:
			  //return true;
			  break;
		}
	  }
	  break;
	default:
		{
			zSkill *s = this->usm.findSkill(rev->wdMagicType);
			//--[�µļ��ܲ�������]---------------------------
			if (s)
			{
				if (skillAction)
				{
					//sky ħ��ϵħ���ӳɼ��
					if(s->base->preskill3 & SPEC_MAGIC)
					{
						if ((3 == s->base->kind) && (4 == s->base->subkind))
							Flameflag = true;

						if ((3 == s->base->kind) && (5 == s->base->subkind))
							Iceflag = true;

						if ((4 == s->base->kind) && (6 == s->base->subkind))
							Sacredflag = true;

						if ((4 == s->base->kind) && (7 == s->base->subkind))
							Darkflag = true;

					}

					//sky ����ϵ�мӳɼ��
					if(s->base->preskill3 & SPEC_PHYSICS)
					{
						switch(this->getWeaponType())
						{
						case ItemType_Sword:		// sky 105�������ֽ�
						case ItemType_Blade:		// sky 104�����ֵ�
							Handflag = true;
							break;
						case ItemType_Crossbow:		// sky 109������������
						case ItemType_Axe:			// sky 106����˫��������
							Handsflag = true;
							break;
						case ItemType_Hammer:		// sky 107����ذ��������
							Daggerflag = true;
							break;
						case ItemType_Staff:
							Throflag = true;
							break;		
						}
					}
		
					if (!s->action(rev,cmdLen))
					{
						ScenePk::attackFailToMe(rev,this);
						return true;
					}

					//pkState.speedOutM(0);
					appendAttack(rev);
					return true;
				}
				else
				{
					ScenePk::attackFailToMe(rev,this);
					return true;
				}
			}
			else
			{
				ScenePk::attackFailToMe(rev,this);
				Zebra::logger->debug("(%s,%ld)ʹ���Լ�û�����ļ���(%ld)",this->name,this->id,rev->wdMagicType);
				return true;
			}
		}
  }
  return false;
}


void SceneUser::appendAttack(const Cmd::stAttackMagicUserCmd *rev)
{
  Cmd::stAttackMagicUserCmd cmd;

  if (attackTarget)
  {
    memcpy(&cmd,rev,sizeof(cmd),sizeof(cmd));
    
    switch (attackTarget->getType())
    {
      case zSceneEntry::SceneEntry_Player:
        {
          cmd.byAttackType = Cmd::ATTACKTYPE_U2U;
        }
        break;
      case zSceneEntry::SceneEntry_NPC:
        {
          cmd.byAttackType = Cmd::ATTACKTYPE_U2N;
        }
        break;
      default:
        {
          cmd.byAttackType = Cmd::ATTACKTYPE_U2U;
        }
        break;
    }
    

    zSkill *s = NULL;

    //s = zSkill::createTempSkill(this,381,1);
       if (selectByPercent(this->getPoison()))
     {
      s = zSkill::createTempSkill(this,388,1);
      cmd.wdMagicType = 388;
     }
       else if (selectByPercent(this->getLull()))
     {
       s = zSkill::createTempSkill(this,382,1);
       cmd.wdMagicType = 382;
     }
       else if (selectByPercent(this->getReel()))
     {
       s = zSkill::createTempSkill(this,386,1);
       cmd.wdMagicType = 386;
     }
       else if (selectByPercent(this->getChaos()))
     {
       s = zSkill::createTempSkill(this,390,1);
       cmd.wdMagicType = 390;
     }
       else if (selectByPercent(this->getCold()))
     {
       s = zSkill::createTempSkill(this,384,1);
       cmd.wdMagicType = 384;
     }
       else if (selectByPercent(this->getPetrify()))
     {
       s = zSkill::createTempSkill(this,389,1);
       cmd.wdMagicType = 389;
     }
       else if (selectByPercent(this->getBlind()))
     {
       s = zSkill::createTempSkill(this,391,1);
       cmd.wdMagicType = 391;
     }
       else if (selectByPercent(this->getStable()))
     {
       s = zSkill::createTempSkill(this,392,1);
       cmd.wdMagicType = 392;
     }
       else if (selectByPercent(this->getSlow()))
     {
       s = zSkill::createTempSkill(this,387,1);
       cmd.wdMagicType = 387;
     }

    if (s)
    {
      s->action(&cmd,sizeof(cmd));
      SAFE_DELETE(s);
    }
    if (this->skillValue.magicattack >0)
    {
      s = zSkill::createTempSkill(this,this->skillValue.magicattack,1);
      if (s)
      {
        s->action(&cmd,sizeof(cmd));
        SAFE_DELETE(s);
      }
    }
    if (this->skillValue.unitarybattle >0)
    {
      s = zSkill::createTempSkill(this,this->skillValue.unitarybattle,1);
      if (s)
      {
        s->action(&cmd,sizeof(cmd));
        SAFE_DELETE(s);
      }
    }
  }
}


void SceneUser::sendSkill(WORD wdSkillID,BYTE level,DWORD target,BYTE attackType,BYTE action)
{
  Cmd::stAttackMagicUserCmd cmd;

  if (0 == target)
  {
    cmd.dwDefenceTempID = this->tempid;
    cmd.byAttackType = Cmd::ATTACKTYPE_U2U;
    cmd.byAction = Cmd::Ani_Null;
  }
  else
  {
    cmd.dwDefenceTempID = target;
    cmd.byAttackType = attackType;
    cmd.byAction = action;
  }

  cmd.dwUserTempID = this->tempid;
  cmd.wdMagicType = wdSkillID;
  cmd.byDirect = this->getDir();
    
  zSkill *s = NULL;

  s = zSkill::createTempSkill(this,wdSkillID,level);
  if (s)
  {
    s->action(&cmd,sizeof(cmd));
    SAFE_DELETE(s);
  }
}

/**
  * \brief ��������״̬
  *
  *  �Ѹ����״̬��ΪUSTATE_DEATH,
  *  ���Ѹ������������Ϣ,֪ͨ���������
  *  ������,������������,������Զ�����
  *
  * \return �޷���ֵ
  */
void SceneUser::setDeathState()
{  
  if (angelMode) return;

  if (this->bombskillId>0)
    deathWaitTime = 2; //���ȴ���������2���ر���ôд��
  else
    deathWaitTime = 300; //�����ȴ�5����,�Զ�����

  //ȡ������״̬
  if (tradeorder.hasBegin()) {
    tradeorder.cancel();
  }

  //ȡ����̯
  privatestore.step(PrivateStore::NONE,this);
  
  //������ʧ��
  //if (guard) guard->reset();

//  //����������
//  if (horse.mount())
//  {
//    //Cmd::stRideMapScreenUserCmd *rev = 0;//�����Ϣ��ride��������û���õ�
//    //ride(rev);
//    horse.mount(false,false);
//  }

  // ������г���
  this->killAllPets();
  if (guard)//�������ڳ����ƶ�
    guard->masterIsAlive = false;

  //����赲
  this->scene->clearBlock(this->getPos());

  OnDie event(1);
  EventTable::instance().execute(*this,event);
  execute_script_event(this,"die");

  SceneUser *pDef = this;
  //������������˵������б�
  pDef->pkState.clearProtect();
  pDef->charbase.goodness &= (~0x00FF0000);  

  Cmd::stMainUserDeathReliveUserCmd death;
  death.dwUserTempID = pDef->tempid;
  death.deathType = pDef->lastKiller?1:0;
#ifdef _DEBUG
  Zebra::logger->debug("%s ������PKɱ��",name);
#endif

  pDef->scene->sendCmdToNine(pDef->getPosI(),&death,sizeof(death),pDef->dupIndex);
  /*
  // \brief
  // ��ʱ��Ѫ��
  pDef->charbase.hp = pDef->charstate.maxhp;
  pDef->charbase.mp = pDef->charstate.maxmp;
  pDef->charbase.sp = pDef->charstate.maxsp;
  // */
  pDef->charbase.hp=0;
  pDef->setState(SceneUser::SceneEntry_Death);
  pDef->showCurrentEffect(Cmd::USTATE_DEATH,true);
  
  this->skillStatusM.processDeath();
  
  if (npcdareflag)
  {
    Zebra::logger->info("[��������NPC]����[%u]�еĽ�ɫ%sս��",this->charbase.septid,this->name);
  }
  if (this->skillValue.nowrelive>0)
  {
    this->relive(Cmd::ReliveSkill,0,this->skillValue.nowrelive);
    this->skillValue.nowrelive=0;
  }

  if (this->skillValue.relive>0 && selectByPercent((int)(this->skillValue.relive)))
  {
    this->relive(Cmd::ReliveSkill,0,90);
    this->skillValue.relive=0;
  }
#ifdef _TEST_DATA_LOG
  writeCharTest(Cmd::Record::DEATH_WRITEBACK);
#endif // _TEST_DATA_LOG��������
}

/**
  * \brief 
  *
  *
  */
/*void SceneUser::Death()
{
  //����赲״̬
  this->scene->clearBlock(this->getPos());
}*/

/**
 * \brief Pkģʽ�л�
 *
 * \rev �յ��ͻ��˷������л�ָ��
 *
 * \return �л��Ƿ�ɹ�
 */
bool SceneUser::switchPKMode(const Cmd::stPKModeUserCmd *rev)
{
  if (!rev)
  {
    return false;
  }
  using namespace Cmd;
  if (rev->byPKMode == 255)
  {
    this->pkMode = (this->pkMode + 1) % PKMODE_MAX;
  }
  else
  {
    this->pkMode = rev->byPKMode%PKMODE_MAX;
  }
  switch(this->pkMode)
  {
    case PKMODE_NORMAL:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:��ƽģʽ.");
      }
      break;
    case PKMODE_ENTIRE:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:ȫ��ģʽ.");
      }
      break;
    case PKMODE_TEAM:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:���ģʽ.");
      }
      break;
    case PKMODE_TONG:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:���ģʽ.");
      }
      break;
    case PKMODE_SEPT:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:����ģʽ.");
      }
      break;
    /*case PKMODE_SCHOOL:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:ʦ��ģʽ.");
      }
      break;*/
    case PKMODE_COUNTRY:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:����ģʽ.");
      }
      break;
    case PKMODE_GOODNESS:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:�ƶ�ģʽ.");
      }
      break;
    case PKMODE_ALLY:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:�˹�ģʽ.");
      }
      break;
    /*case PKMODE_CHALLENGE:
      {
        Channel::sendSys(this,Cmd::INFO_TYPE_STATE,"PK�л�ģʽΪ:��սģʽ.");
      }
      break;*/
    default:
      break;
  }

  {
    stPKModeUserCmd ret;
    ret.byPKMode = this->pkMode;
    //Zebra::logger->debug("switchPkMode=%u",this->pkMode);
    this->sendCmdToMe(&ret,sizeof(stPKModeUserCmd));

    return true;
  }
  return false;
}

/**
 * \brief ���¼����ƶ��
 *
 * \return ʼ�շ���TRUE
 */
bool SceneUser::checkGoodness()
{

  zRTime ctv;
  if ((charbase.goodness & 0x0000FFFF) != Cmd::GOODNESS_2_1)
  {
    if (ctv > pkState.tGood)
    {
      if ((charbase.goodness & 0x0000FFFF) >= (Cmd::GOODNESS_0 & 0x0000FFFF))
      {
        if ((charbase.goodness & 0x0000FFFF ) == 0x0000FFFF)
        {
          charbase.goodness &= 0xFFFF0000;
        }
        else
        {
          charbase.goodness ++;
        }
      }
      else
      {
        int num = 1;

        for (int i=0; i<num; i++)
        {
          charbase.goodness --;
          switch(charbase.goodness)
          {
            case Cmd::GOODNESS_2_1:
            case Cmd::GOODNESS_2_2:
            case Cmd::GOODNESS_3:
            case Cmd::GOODNESS_4:
            case Cmd::GOODNESS_5:
            case Cmd::GOODNESS_6:
              {
                //֪ͨ�ͻ������Ա仯
                // mark
                //Cmd::stAddUserMapScreenUserCmd send;
                //this->full_t_MapUserData(send.data);
                //this->scene->sendCmdToNine(getPosI(),&send,sizeof(send),false);
                //Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",this->name,this->id,temp,this->charbase.goodness);
                //this->reSendMyMapData();
                this->sendGoodnessToNine();
              }
              break;
            default:
              break;
          }
          if (0==(charbase.goodness & 0x0000FFFF))
          {
            //clearUState(Cmd::USTATE_XIXINGEMIA);
            break;
          }
        }
      }

      //ScenePk::sendChangedUserData(this);
      //Zebra::logger->debug("��ǰ�ƶ��(%s(%ld),%X)",this->name,this->id,charbase.goodness);
      pkState.tGood = ctv;
      pkState.tGood.addDelay(ScenePkState::goodnessPeriod);
    }
  }
  if (charbase.goodness & 0x00FF0000)
  {
    if (ctv > pkState.tProtect)
    {
      if (pkState.cancelProtect(this))
      {
        //֪ͨ�ͻ������Ա仯
        // mark
        //Cmd::stAddUserMapScreenUserCmd send;
        //this->full_t_MapUserData(send.data);
        //this->scene->sendCmdToNine(getPosI(),&send,sizeof(send),false);
        //this->reSendMyMapData();
        this->sendGoodnessToNine();
        
        //Zebra::logger->debug("ȡ������״̬(%s(%ld),%X)",this->name,this->id,this->charbase.goodness);
      }
      pkState.tProtect = ctv;
      pkState.tProtect.addDelay(pkState.protectPeriod);
    }
  }
  return true;
}

/**
 * \brief ����Լ��ǲ����ڶԶԷ���������״̬
 *
 * \param pThis: �Է����
 * \param defid: �Է����ID
 *
 */
bool ScenePkState::deathUserProtect(SceneUser *pThis,DWORD defid)
{
  protect_time=0;
  pThis->charbase.goodness &= (~Cmd::GOODNESS_DEF);  
  pThis->charbase.goodness &= (~Cmd::GOODNESS_ATT);  
  return true;
}

/**
 * \brief ���¼�������ʱ��,���Ϊ��,��ȡ������
 *
 * \param pThis:�Է����
 *
 * \return �������ʱ�䲢δ����,����FALSE,���򷵻�TRUE
 */
bool ScenePkState::cancelProtect(SceneUser *pThis,DWORD time)
{
  if (time)
  {
    protect_time=0;
  }
  else if (protect_time)
  {
    protect_time --;
  }
  //Zebra::logger->debug("%s(%d)����ʱ��:%d",pThis->name,pThis->id,protect_time);
  if (!protect_time)
  {
    pThis->charbase.goodness &= ~0x00020000;
    pThis->charbase.goodness &= ~0x00010000;
    return true;
  }
  return false;
}

/**
 * \brief �û���������
 *
 * \pDef ��Ҫ�����������û�
 *
 * \return �����Ƿ�ɹ�
 */
bool ScenePk::attackDeathUser(SceneUser *pAtt,SceneUser *pDef)
{
  //����״̬������pkֵ
  bool guard = pDef->issetUState(Cmd::USTATE_GUARD);
  //Ӧ�ü���,��Ҫ��pDef�����ݽ����޸�
  // ����ϵ��ս״̬�������ƶ�ȣ���ͽ,��ħ�ȣ�
  bool dare = pAtt->isWar(pDef);
  // ����״̬������pkֵ
  bool gem = false;
  // ��ͷ״̬������pkֵ
  bool catcher = false;

  if ((pDef->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER)
        || pDef->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON))
      && pAtt->charbase.country == pDef->charbase.country)
  {
    gem = true;
  }

  if (pAtt->isCatcherState() && pAtt->charbase.country == pDef->charbase.country)
  {
    catcher = true;
  }

  if (pDef->getState() == SceneUser::SceneEntry_Death)
  {
    return true;
  }
#define attGood ((SDWORD)(pAtt->charbase.goodness & 0x0000FFFF))
#define defGood ((SDWORD)(pDef->charbase.goodness & 0x0000FFFF))
#define attSub  ((SDWORD)(pAtt->charbase.goodness & 0xFFFF0000))
#define defSub  ((SDWORD)(pDef->charbase.goodness & 0xFFFF0000))
#define bitMask  0x00FF0000

  //����Ƿ�������������
  /*
     bool protection = false;
     if (attSub & Cmd::GOODNESS_DEF)
     {
     protection = pAtt->pkState.deathUserProtect(pAtt,pDef->id);
     }

  //�����������
  if (!protection)
  // */

  if (!pAtt->scene->isNoRedScene())
  {
    DWORD protect_state=pAtt->charbase.goodness & 0x00FF0000;
    if (!(defSub & Cmd::GOODNESS_ATT) && !guard  &&  !dare && !gem && !catcher
        && (pAtt->charbase.country == pDef->charbase.country || pAtt->charbase.country == PUBLIC_COUNTRY)
        && (!pDef->scene->checkZoneType(pDef->getPos(),ZoneTypeDef::ZONE_SPORTS) 
          &&!pAtt->scene->checkZoneType(pAtt->getPos(),ZoneTypeDef::ZONE_SPORTS))
        && (pAtt != pDef))
    {
      //pAtt->pkState.cancelProtect(pAtt,0);

      switch(pAtt->getGoodnessState())
      {
        case Cmd::GOODNESS_2_1: // ��ͨ
          {
            if (pDef->getGoodnessState() <= 60)
            {
              DWORD temp=pAtt->charbase.goodness;
              pAtt->charbase.goodness = (DWORD)Cmd::GOODNESS_3 + pDef->getPkAddition();
              Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,temp,pAtt->charbase.goodness);
            }
            else//ɱ����
            {
              pDef->lastKiller = 0;

              if ((attSub & Cmd::GOODNESS_2_2) == 0)
              {
                pAtt->charbase.goodness = (DWORD)Cmd::GOODNESS_2_2;
                zRTime ctv;
                pAtt->pkState.tGoodNormal = ctv;
              }
              else
              {
                zRTime ctv;
                ctv.addDelay(-3600 * 1000);
                if (ctv <= pAtt->pkState.tGoodNormal)
                {
                  DWORD temp=pAtt->charbase.goodness;
                  pAtt->charbase.goodness = (pAtt->charbase.goodness & 0x00FF0000) | (DWORD)Cmd::GOODNESS_1;
                  Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,temp,pAtt->charbase.goodness);
                  //pAtt->charstate.lucky ++;
                }
                else
                {
                  pAtt->pkState.tGoodNormal = ctv;
                }
              }
            }
          }
          break;

        case Cmd::GOODNESS_3://��ͽ
          {
            DWORD temp=pAtt->charbase.goodness;
            pAtt->charbase.goodness = (DWORD)Cmd::GOODNESS_4 + pDef->getPkAddition();
            Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,temp,pAtt->charbase.goodness);
          }
          break;

        case Cmd::GOODNESS_4://��ͽ
          {
            DWORD temp=pAtt->charbase.goodness;

            //����ɱ����PKֵ����300
            if ((short)(pDef->charbase.goodness & 0x0000FFFF) < 0)
            {
              pAtt->charbase.goodness += 300;
            }else{
              pAtt->charbase.goodness = (DWORD)Cmd::GOODNESS_5 + pDef->getPkAddition();
            }
            Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,temp,pAtt->charbase.goodness);
            //pAtt->charstate.lucky --;
          }
          break;

        case Cmd::GOODNESS_5://��ħ
          {
            DWORD temp=pAtt->charbase.goodness;

            if ((short)(pDef->charbase.goodness & 0x0000FFFF) < 0)
            {
              pAtt->charbase.goodness += 300;
            }else{
              pAtt->charbase.goodness = (DWORD)Cmd::GOODNESS_6 + pDef->getPkAddition();
            }
            Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,temp,pAtt->charbase.goodness);
            //pAtt->charstate.lucky --;
          }
          break;
        case Cmd::GOODNESS_6://��ħ
          {
            DWORD temp=pAtt->charbase.goodness;

            if ((short)(pDef->charbase.goodness & 0x0000FFFF) < 0)
            {
              pAtt->charbase.goodness += 300;
            }else{
              pAtt->charbase.goodness += 30;
            }

            if (pAtt->charbase.goodness > MAX_GOODNESS)
            {
              pAtt->charbase.goodness = MAX_GOODNESS;
            }
            Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,temp,pAtt->charbase.goodness);
            //pAtt->charstate.lucky --;
          }
          break;

        case Cmd::GOODNESS_1://��ʿ
          {
            if (pDef->getGoodnessState() > (short)Cmd::GOODNESS_3 && pDef->getGoodnessState() < (short)MAX_GOODNESS)
            {
              pAtt->charbase.goodness = (pAtt->charbase.goodness & 0x00FF0000) | (DWORD)Cmd::GOODNESS_0;
              //pAtt->charstate.lucky ++;
            }
            else
            {
              DWORD temp=pAtt->charbase.goodness;
              pAtt->charbase.goodness = (DWORD)Cmd::GOODNESS_3 + pDef->getPkAddition();
              Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,temp,pAtt->charbase.goodness);
            }
          }
          break;

        case Cmd::GOODNESS_0://Ӣ��
          {
            if (pDef->getGoodnessState() <= (short)Cmd::GOODNESS_3)
            {
              Zebra::logger->debug("%s(%d)pk״̬�ı�(%d-->%d)",pAtt->name,pAtt->id,pAtt->charbase.goodness,(DWORD)Cmd::GOODNESS_3 + pDef->getPkAddition());
              pAtt->charbase.goodness = (DWORD)Cmd::GOODNESS_3 + pDef->getPkAddition();
            }
          }
          break;
        default:
          break;
      }
      if (!pDef->isRedNamed(false) && !pAtt->mask.is_masking())
          pDef->lastKiller = pAtt->tempid;
    }

    if (pAtt->mask.is_masking() ) {
      Channel::sendSys(pDef,Cmd::INFO_TYPE_FAIL,"�㱻������ɱ����." );  
    }else {
      Channel::sendSys(pDef,Cmd::INFO_TYPE_FAIL,"�㱻%sɱ����.",pAtt->name);

      if (!guard && !dare && !gem && !catcher &&
          !pDef->scene->checkZoneType(pDef->getPos(),ZoneTypeDef::ZONE_SPORTS)&&
          !pAtt->scene->checkZoneType(pAtt->getPos(),ZoneTypeDef::ZONE_SPORTS))
      {
        if (pDef->mask.is_masking())
          Channel::sendSys(pAtt,Cmd::INFO_TYPE_STATE,"��ɱ���� ������,���Ϊ%s",pAtt->getGoodnessName());
        else if (pAtt->charbase.country == pDef->charbase.country )
          Channel::sendSys(pAtt,Cmd::INFO_TYPE_STATE,"��ɱ����%s,���Ϊ%s",pDef->name,pAtt->getGoodnessName());
        else
          Channel::sendSys(pAtt,Cmd::INFO_TYPE_STATE,"��ɱ����%s",pDef->name);
        //Channel::sendNine(pAtt,"%sɱ����%s,��Ϊ%s",pAtt->name,pDef->name,pAtt->getGoodnessName());
      }
      else if (dare)
      {
        if (pDef->mask.is_masking())
          Channel::sendSys(pAtt,Cmd::INFO_TYPE_STATE,"���ڶ�ս��ɱ���� ������");
        else
          Channel::sendSys(pAtt,Cmd::INFO_TYPE_STATE,"���ڶ�ս��ɱ����%s",pDef->name);
        //Channel::sendNine(pAtt,"%s�ڶ�ս��ɱ����%s",pAtt->name,pDef->name);

      }
    }

    // mark
    //Cmd::stAddUserMapScreenUserCmd send;
    //pAtt->full_t_MapUserData(send.data);
    //pAtt->scene->sendCmdToNine(pAtt->getPosI(),&send,sizeof(send),false);
    //���й��ҹ�ѫֵ�ļ���
    if (pAtt->charbase.country!=pDef->charbase.country && pAtt->charbase.country!=PUBLIC_COUNTRY)
    {  
      int level_diff = (int)pAtt->charbase.level - (int)pDef->charbase.level;
      if (level_diff <= 0)
      {
        pAtt->charbase.exploit+=(2*exploit_arg);
        Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"������� %d �㹦ѫֵ",2);

      }
      else if (level_diff<20)
      {
        pAtt->charbase.exploit+=(1*exploit_arg);
        Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"������� %d �㹦ѫֵ",1);
      }
      //�ڹ�ս�򱾹�ɱ�����,��ʹ��ɱ���˱��Լ���20�����Ϲ�ѫҲֵ������
      if (pAtt->scene->getCountryID() == pAtt->charbase.country ||
          pAtt->isWarRecord(Cmd::COUNTRY_FORMAL_DARE,pDef->charbase.country))
      {
      }else{
        //��ɱ�ߵȼ�<�Լ��ȼ�-60 ��۳�5�㹦ѫֵ
        if (level_diff>60)
        {
          if (pAtt->charbase.exploit > (5*exploit_arg) )
          {
            pAtt->charbase.exploit -= (5*exploit_arg);
          }
          else
          {
            pAtt->charbase.exploit = 0;
          }

          Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"��ʧȥ�� %d �㹦ѫֵ",5);
        }

        if (level_diff <= 60 && level_diff>40)
        {
          if (pAtt->charbase.exploit > (2*exploit_arg) )
          {
            pAtt->charbase.exploit -= (2*exploit_arg);
          }
          else
          {
            pAtt->charbase.exploit = 0;
          }
  
          Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"��ʧȥ�� %d �㹦ѫֵ",2);
        }

        if (level_diff <= 40 && level_diff>=20)
        {
          if (pAtt->charbase.exploit > (1*exploit_arg) )
          {
            pAtt->charbase.exploit -= (1*exploit_arg);
          }
          else
          {
            pAtt->charbase.exploit = 0;
          }
  
          Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"��ʧȥ�� %d �㹦ѫֵ",1);
        }  
      }

      if (level_diff < 0 && level_diff >= -40)
      {
        if (pDef->charbase.exploit > (1*exploit_arg) )
        {
          pDef->charbase.exploit = pDef->charbase.exploit - (1*exploit_arg);
        }
        else
        {
          pDef->charbase.exploit = 0;
        }
        Channel::sendSys(pDef,Cmd::INFO_TYPE_GAME,"��ʧȥ�� %d �㹦ѫֵ",1);
      }
      if (level_diff < -40)
      {
        if (pDef->charbase.exploit > (2*exploit_arg) )
        {
          pDef->charbase.exploit = pDef->charbase.exploit - (2*exploit_arg);
        }
        else
        {
          pDef->charbase.exploit = 0;
        }
        Channel::sendSys(pDef,Cmd::INFO_TYPE_GAME,"��ʧȥ�� %d �㹦ѫֵ",2);
      } 
      
      // ��д����
      //pAtt->save(Cmd::Record::OPERATION_WRITEBACK);
      //pDef->save(Cmd::Record::OPERATION_WRITEBACK);
    }

    //��֤����״̬����
    if (pAtt->charbase.goodness <= Cmd::GOODNESS_3)
    {
      pAtt->charbase.goodness |= protect_state;
    }
    else
    {
      pAtt->pkState.clearProtect();
    }
    if (!pDef->scene->checkZoneType(pDef->getPos(),ZoneTypeDef::ZONE_SPORTS) 
        && !pAtt->scene->checkZoneType(pAtt->getPos(),ZoneTypeDef::ZONE_SPORTS) 
        && !((pDef->getGoodnessState() == Cmd::GOODNESS_2_1 || pDef->isPkAddition()) && dare) )
    {
      pDef->lostObject(pAtt);
    }          
  }

  if (pAtt->getGoodnessState()>(short)Cmd::GOODNESS_3)
  {
    pAtt->charbase.pkaddition=0;
    if (pAtt->issetUState(Cmd::USTATE_PK))
    {
      pAtt->clearUState(Cmd::USTATE_PK);
      pAtt->sendtoSelectedPkAdditionState();
    }
  }
  pAtt->reSendMyMapData();

  if (dare)
  {// ��Ự���������Ͷ�ս�Ʒ�����
    Cmd::Session::t_darePk_SceneSession send;
    send.attID = pAtt->id;
    send.defID = pDef->id;
    sessionClient->sendCmd(&send,sizeof(send));
  }

  if (gem)
  {// ��Ự���������ͻ�����������
    bool dragon = pDef->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON);
    bool tiger = pDef->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER);
    Cmd::Session::t_ChangeGemState_SceneSession change_send;

    change_send.fromUserID = pDef->id;
    change_send.toUserID = pAtt->id;

    if (tiger && dragon)
    {
      int choice_state = randBetween(1,2);
      change_send.dwState = choice_state;
    }
    else if (dragon)
    {
      change_send.dwState = 1;
    }
    else if (tiger)
    {
      change_send.dwState = 2;
    }

    sessionClient->sendCmd(&change_send,sizeof(change_send));
  }

  if (pDef->miniGame)
  {
    Dice * temp = pDef->miniGame;
    pDef->miniGame->endGame();

    delete temp;
    pDef->miniGame = 0;
  }

  pAtt->leaveBattle();
  pDef->leaveBattle();
  pDef->clearDefTarget();
  pDef->setDeathState();
  /// ����ʱ���ҩƷ����
  pDef->leechdom.clear();

  if (attGood>=1500 && attGood<=(SDWORD)MAX_GOODNESS && pAtt->scene->getRealMapID()!=189)
  {
    Scene * s=SceneManager::getInstance().getSceneByName("������������");
    if (s)
    {
      bool suc = pAtt->changeMap(s,zPos(80,70));
      if (!suc)
        Zebra::logger->error("%s PKֵ %u,��������ʧ��,Ŀ�� %s (%d,%d)",pAtt->name,pAtt->charbase.goodness,s->name,100,100);
      else
        Zebra::logger->error("%s PKֵ %u,��������",pAtt->name,pAtt->charbase.goodness);
    }
    else
    {
      //if (pAtt->guard && pAtt->guard->canMove()) pAtt->saveGuard = true;//ʹ�ڳ�����ָ��ʹ����
      //if (pAtt->adoptList.size()) pAtt->saveAdopt = true;
      Cmd::Session::t_changeScene_SceneSession cmd;
      cmd.id = pAtt->id;
      cmd.temp_id = pAtt->tempid;
      cmd.x = 80;
      cmd.y = 70;
      cmd.map_id = 0;
      cmd.map_file[0] = '\0';
      strncpy((char *)cmd.map_name,"������������",MAX_NAMESIZE);
      sessionClient->sendCmd(&cmd,sizeof(cmd));

    }

    Channel::sendCountryInfo(pAtt->charbase.country,Cmd::INFO_TYPE_EXP,"%s ��������,����׽�ù鰸��",pAtt->name);
  }

#ifdef _DEBUG
  Zebra::logger->debug("%s �� %s ɱ��,����tempid",pDef->name,pAtt->name,pDef->lastKiller);
#endif
  return true;
}

/**
 * \brief NPC��������
 * \pAtt ������
 * \pDef ��Ҫ����������NPC
 *
 * \return �����Ƿ�ɹ�
 */
bool ScenePk::attackDeathNpc(SceneEntryPk *pAtt,SceneNpc *pDef)
{
  if (pAtt)
  {
    if (zSceneEntry::SceneEntry_NPC==pAtt->getType())
    {
      if (!((SceneNpc *)pAtt)->chaseSecondTarget() && !((SceneNpc *)pAtt)->chaseItsMaster())
        pAtt->leaveBattle();
    }
  }

  pDef->on_death(pAtt);
  pDef->death(SceneTimeTick::currentTime);

  //�����ǡ��߾����������Ͻ�֪ͨ
  if (pDef->id==COUNTRY_SEC_GEN)
  {
    SceneEntryPk * am = pAtt->getTopMaster();

    if (am->getType()==zSceneEntry::SceneEntry_Player
        && pDef->scene->getCountryID()!=((SceneUser *)am)->charbase.country)
    {
      char buf[MAX_CHATINFO];
      bzero(buf,sizeof(buf));
      _snprintf(buf,MAX_CHATINFO-1,"%s(%s) �� %s(%u,%u) ɱ�� %s"
          ,am->name,SceneManager::getInstance().getCountryNameByCountryID(((SceneUser *)am)->charbase.country)
          ,pDef->scene->name,pDef->getPos().x,pDef->getPos().y,pDef->name);

      DWORD high = (pDef->scene->getCountryID()<<16);
      Channel::sendMapInfo(high+WANGCHENG_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
      Channel::sendMapInfo(high+DONGJIAO_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
      Channel::sendMapInfo(high+NANJIAO_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
      Channel::sendMapInfo(high+BIANJING_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
    }
  }

  Cmd::stNpcDeathUserCmd death;
  death.dwNpcTempID = pDef->tempid;
  pDef->scene->sendCmdToNine(pDef->getPosI(),&death,sizeof(death),pDef->dupIndex);

  pDef->setUState(Cmd::USTATE_DEATH);
  Cmd::stRTMagicPosUserCmd ret;
  pDef->full_stRTMagicPosUserCmd(ret);
  pDef->scene->sendCmdToNine(pDef->getPosI(),&ret,sizeof(ret),pDef->dupIndex);

  SceneEntryPk * m = pDef->getTopMaster();
  if (m && zSceneEntry::SceneEntry_NPC==m->getType())
  {
    if (pAtt) pAtt->reduceGoodness(pDef);
    if (Cmd::PET_TYPE_NOTPET==pDef->getPetType()) pDef->distributeExp();
  }

  pDef->leaveBattle(true);
  pDef->sendMeToNine();
  //pDef->setStateToNine(Cmd::USTATE_DEATH);
  return true;
}

/* 
 * �����������鿨�Ļƽ�װ��
 *
 */
void SceneUser::sendGiftEquip(WORD level)
{
}

/*
 * �������������ʱ��
 * petPoint����Ϊ��λ��¼
 */
void SceneUser::givePetPoint()
{
  if (Zebra::global["pet_point"]!="true") return;
  DWORD hour = 0;
  switch (charbase.level)
  {
    case 30:
    case 40:
    case 50:
    case 60:
    case 70:
    case 80:
    case 90:
    case 100:
    case 110:
    case 120:
      hour = 10+(charbase.level/10-3)*5;
      //���ֿ��û�30������10Сʱ
      if ((charbase.accPriv&ACCPRIV_NEWBIE_EQUIP_AT_5_15) && charbase.level==30)
        hour += 10;
      //80������ÿ������5Сʱ
      if (charbase.level>=80)
          hour += ((charbase.level/10-7)*5);
      charbase.petPoint += hour*3600;

      char text[MAX_CHATINFO];
      bzero(text,sizeof(text));
      _snprintf(text,sizeof(text),"�װ������:\n\t��ϲ���Ѿ��ﵽ%u��,ϵͳ��Ϊ����ǰ����������ֵ%uСʱ����ʱ���Լ�������������ǿ�ȡ����������û�����������ڹ�����һʱ�����\n\t\t\t\t\t\tӢ����˫��Ӫ�Ŷ�",charbase.level,hour);
      sendMail("Ӣ����˫��Ӫ",0,name,id,Cmd::Session::MAIL_TYPE_SYS,0,0,text);

      
	  if(cartoonList.empty())
		  return;


      if (cartoonList.begin()->second.state==Cmd::CARTOON_STATE_PUTAWAY && !cartoon)
      {
        //���������ĳ�������ʱ��,һ���Լӵ���������
        cartoonList.begin()->second.time += charbase.petPoint;

        Cmd::Session::t_chargeCartoon_SceneSession send;
        send.masterID = id;
        send.cartoonID = cartoonList.begin()->first;
        send.time = charbase.petPoint;
        sessionClient->sendCmd(&send,sizeof(send));

        Cmd::stAddCartoonCmd ac;
        ac.isMine = true;
        ac.cartoonID = cartoonList.begin()->first;
        ac.data = cartoonList.begin()->second;
        sendCmdToMe(&ac,sizeof(ac));

        charbase.petPoint = 0;

        Zebra::logger->info("[����]%s %u��,�����%s(%u)��� %u ������ʱ��",name,charbase.level,cartoonList.begin()->second.name,cartoonList.begin()->first,hour*3600);
      }
      break;
    default:
      return;
  }
}

/**
 * \brief ��ҵȼ�,��������
 *
 * ��������ɹ���֪ͨ�ͻ���
 * 
 * \param num: ������ʣ�ྭ��ֵ
 *
 * \return �����Ƿ�ɹ�
 */
bool SceneUser::upgrade(DWORD num)
{
  int old_level = charbase.level;
  if (num > 0)
  {
    while(num--)
    {
      if (charbase.level < max_level)
      {
        charbase.level ++;
        OnOther event(3);
        EventTable::instance().execute(*this,event);
        charbase.skillpoint ++;
        charbase.points += (charbase.level-1)/20 +3;
      }
      else
      {
        break;
      }
      setupCharBase();

      sendGiftEquip(charbase.level);//�͸��ֵ��߿��͵�װ��
      givePetPoint();//�ͳ�������ʱ��
      if (30==charbase.level && (atoi(Zebra::global["service_flag"].c_str())&Cmd::Session::SERVICE_HORSE))//30���������
      {
          horse.horse(3000);
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"������ϵͳ���͵�һƥ�����");
      }
    }
  }
  else
  {
    while(charbase.exp >= charstate.nextexp)
    {
      if (charbase.level < max_level)
      {
        charbase.level ++;
        OnOther event(3);
        EventTable::instance().execute(*this,event);
        charbase.skillpoint ++;
        charbase.points += (charbase.level-1)/20 +3;
      }
      else
      {
        break;
      }
      charbase.exp -= charstate.nextexp;
      if (charbase.level == max_level) charbase.exp = 0;
      setupCharBase();

      sendGiftEquip(charbase.level);//�͸��ֵ��߿��͵�װ��
      givePetPoint();//�ͳ�������ʱ��
      if (30==charbase.level && (atoi(Zebra::global["service_flag"].c_str())&Cmd::Session::SERVICE_HORSE))//30���������
      {
          horse.horse(3000);
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"������ϵͳ���͵�һƥ�����");
      }
    }
  }

  if (charbase.level == max_level) Channel::sendSys(this,Cmd::INFO_TYPE_EXP,"��ϲ���Ѿ��ﵽ����%u��",max_level);

  if (old_level < 20 && this->charbase.level >= 20)
  {
    Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�����ڿ��Թ���һ���µĲֿ���");    
  }
  if (old_level < 40  && this->charbase.level >= 40)
  {
    Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�����ڿ��Թ���һ���µĲֿ���");    
  }
  if (charbase.level>20 && this->charbase.septid)
  {
    Cmd::Session::t_OpRepute_SceneSession send;
    send.dwSeptID = this->charbase.septid;
    send.dwRepute = 1;
    sessionClient->sendCmd(&send,sizeof(send));
  }
  
    
  charbase.hp = charstate.maxhp;
  charbase.sp = charstate.maxsp;
  charbase.mp = charstate.maxmp;
  ScenePk::sendChangedUserData(this);
  Cmd::stLevelUpUserCmd stl;
  stl.dwUserTempID = this->tempid;
  this->scene->sendCmdToNine(this->getPosI(),&stl,sizeof(stl),this->dupIndex);
  Zebra::logger->info("�û�(%s(%ld))����%ld��",this->name,this->id,charbase.level);
  Cmd::Session::t_levelupNotify_SceneSession send;
  send.dwUserID = charbase.id;
  send.level = charbase.level;
  send.qwExp = charbase.exp;
  sessionClient->sendCmd(&send,sizeof(send));

  for (cartoon_it it=cartoonList.begin(); it!=cartoonList.end(); it++)
    it->second.masterLevel = charbase.level;
  if (cartoon)
    cartoon->setCartoonData(cartoonList[cartoon->getCartoonID()]);

#ifdef _TEST_DATA_LOG
  writeCharTest(Cmd::Record::LEVELUP_WRITEBACK);
#endif // _TEST_DATA_LOG��������
  //ScenePk::attackRTExp(this,charbase.exp);
  return true;
}

/**
 * \brief ֪ͨ�û����Է����仯
 *
 * \pUser ���Է����仯���û�
 * \return �����޸��Ƿ�ɹ� 
 */
bool  ScenePk::sendChangedUserData(SceneUser *pUser)
{
  Cmd::stMainUserDataUserCmd toDef;
  pUser->full_t_MainUserData(toDef.data);
  pUser->sendCmdToMe(&toDef,sizeof(toDef));

  pUser->sendMeToNine();

  pUser->sendInitHPAndMp();

  //֪ͨ�������Ա仯
  TeamManager * team = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);

  if(team)
	  team->sendtoTeamCharData(pUser);

  return true;
}

/**
 * \brief �û�����ָ��ʧ��ʱ����
 * 
 *
 * \rev ����ʧ�ܵ�ָ��
 * \pAtt ����������
 * \isTargetU Ŀ�깥�����û�����npc
 * \return ʼ�շ���true
 */
bool ScenePk::attackFailToMe(const Cmd::stAttackMagicUserCmd *rev,SceneEntryPk * pAtt,bool failed,bool me)
{
#ifdef _DEBUG
  //Zebra::logger->debug("%s ����ʧ��",pAtt->name);
#endif

  if (rev &&(rev->wdMagicType == 351 || rev->wdMagicType == 352)) //���ﷴ������״̬������ʧ��
  {
    return true;
  }

  Cmd::stRTMagicUserCmd ret;
  ret.dwUserTempID = rev->dwDefenceTempID;
  ret.dwSrcTempID = pAtt->tempid;

  if (rev->byAttackType == Cmd::ATTACKTYPE_U2U || 
    rev->byAttackType == Cmd::ATTACKTYPE_N2U)
  {
    ret.byTarget = Cmd::MAPDATATYPE_USER;
  }
  else
  {
	  ret.byTarget = Cmd::MAPDATATYPE_NPC;
  }

  if (pAtt->getType() == zSceneEntry::SceneEntry_Player )
	  ret.bySrc = Cmd::MAPDATATYPE_USER;
  else
  {
	  if( ((SceneNpc *)pAtt)->npc->kind == NPC_TYPE_GHOST )
		  ret.bySrc = Cmd::MAPDATATYPE_USER;
	  else
		  ret.bySrc = Cmd::MAPDATATYPE_NPC;
  }
  ret.byLuck = 0;
  ret.byDirect = pAtt->getDir();
  ret.dwHP = 0;
  ret.sdwHP = 0;

  if (failed)
  {
    ret.byRetcode = Cmd::RTMAGIC_FAILURE;
  }
  else
  {
    ret.byRetcode = Cmd::RTMAGIC_DUCK;
  }

  Cmd::stAttackMagicUserCmd attCmd;
  bcopy(rev,&attCmd,sizeof(attCmd),sizeof(attCmd));
  attCmd.dwUserTempID = pAtt->tempid;

  SceneEntryPk *pEntry = pAtt->getTopMaster();
  if (pEntry&&pEntry->getType() == zSceneEntry::SceneEntry_Player)
  {
    SceneUser *pUser = (SceneUser *)pEntry;
    if (pUser)
    {
      pUser->sendCmdToMe(&ret,sizeof(ret));
      //if (me)
      //  pUser->sendCmdToMe(&attCmd,sizeof(attCmd));
      //else
      //  pAtt->scene->sendCmdToNine(pAtt->getPosI(),&attCmd,sizeof(attCmd),false);

    }
  }
  else
  {
    //��ʱ�޸ĵȴ��߻�����
    pAtt->scene->sendCmdToNine(pAtt->getPosI(),&attCmd,sizeof(attCmd),pAtt->dupIndex);

  }

  return true;
}

/**
 * \brief ֪ͨ����ֵ�����ı�
 *
 * \pUser �û�
 * \return ��
 */
/*
void ScenePk::attackRTExp(SceneUser *pUser,DWORD exp,DWORD dwTempID,BYTE byType)
{
  Cmd::stObtainExpUserCmd ret;
  ret.dwTempID = dwTempID;        ** ����ֵ��Դ��ʱ��� *
  ret.byType = byType;        ** ����ֵ��Դ enumMapDataType *
  //  ret.dwUserTempID = pUser->tempid;
  ret.dwExp = exp;
  if (pUser->charbase.exp >= pUser->charstate.nextexp)
  {
    ret.dwUserExp = pUser->charbase.exp-pUser->charstate.nextexp;
  }
  else
  {
    ret.dwUserExp = pUser->charbase.exp;
  }
#ifdef _DEBUG
  Zebra::logger->info("[���;�������֪ͨ]��þ��飺%u �û���ǰ���飺%u",ret.dwExp,ret.dwUserExp);
#endif
  pUser->sendCmdToMe(&ret,sizeof(ret));
}
*/
/**
 * \brief �����û�HP��MP
 *
 * ֪ͨ�û���team,HP��MP�ĸı�
 *
 * \pUser �û�
 * \wdHP �û���ǰHP
 * wdMP �û���ǰMP
 *
 * \return �Ƿ�ɹ�
 */
void ScenePk::attackRTHpAndMp(SceneUser *pUser)
{
  Cmd::stSetHPAndMPDataUserCmd ret;
  ret.dwHP = pUser->charbase.hp;
  ret.dwMP = pUser->charbase.mp;
  //ret.dwSP = pUser->charbase.sp;
  pUser->sendCmdToMe(&ret,sizeof(ret));

  TeamManager * team = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);
  if(team)
	  team->sendtoTeamCharData(pUser);

  pUser->sendtoSelectedHpAndMp();
}

/**
 * \brief ������������
 *
 * \rev �յ��Ĺ�������
 * \pAtt ������
 * \pDef ��������
 * \wdHP ���ι�����Ѫֵ
 * \byLuck ���ι���������ֵ
 *
 * \return �㲥�Ƿ�ɹ�
 */
bool ScenePk::attackRTCmdToNine(const Cmd::stAttackMagicUserCmd *rev,SceneEntryPk *pAtt,SceneEntryPk *pDef,const SDWORD sdwHP,BYTE byLuck)
{
  Cmd::stRTMagicUserCmd ret;
  Cmd::stRTOtherMagicUserCmd ret1;
  ret.dwUserTempID = pDef->tempid;
  ret.dwSrcTempID = pAtt->tempid;
  SceneEntryPk *pAttMaster=NULL;
  SceneEntryPk *pDefMaster=NULL;

  switch(pAtt->getType())
  {
  case zSceneEntry::SceneEntry_Player:  /** ��ҽ�ɫ*/
	  ret.bySrc = Cmd::MAPDATATYPE_USER;
	  break;
  case zSceneEntry::SceneEntry_NPC:    /** NPC*/
	  {
		  if( ((SceneNpc *)pAtt)->npc->kind == NPC_TYPE_GHOST )
		  {
			  pAttMaster=pAtt->getTopMaster(); 
			  ret.bySrc = Cmd::MAPDATATYPE_USER;
		  }
		  else
		  {
			  pAttMaster=pAtt->getTopMaster(); 
			  ret.bySrc = Cmd::MAPDATATYPE_NPC;
		  }
	  }
      break;
      //    case zSceneEntry::SceneEntry_Build:    /**< ����*/
      //      break;
      //    case zSceneEntry::SceneEntry_Object:  /**< ������Ʒ*/
      //      break;
    default:
      ret.bySrc = Cmd::MAPDATATYPE_NPC;
      break;
  }

  switch(pDef->getType())
  {
    case zSceneEntry::SceneEntry_Player:  /** ��ҽ�ɫ*/
      ret.byTarget = Cmd::MAPDATATYPE_USER;
      ret.dwHP = ((SceneUser *)pDef)->charbase.hp;
      break;
    case zSceneEntry::SceneEntry_NPC:    /** NPC*/
      {
		  if( ((SceneNpc *)pDef)->npc->kind == NPC_TYPE_GHOST )
		  {
			  pAttMaster=pAtt->getTopMaster();
			  ret.bySrc = Cmd::MAPDATATYPE_USER;
			  ret.dwHP = ((SceneNpc *)pDef)->hp;
		  }
		  else
		  {
			  pAttMaster=pAtt->getTopMaster(); 
			  ret.byTarget = Cmd::MAPDATATYPE_NPC;
			  ret.dwHP = ((SceneNpc *)pDef)->hp;
		  }
      }
      break;
      //    case zSceneEntry::SceneEntry_Build:    /**< ����*/
      //      break;
      //    case zSceneEntry::SceneEntry_Object:  /**< ������Ʒ*/
      //      break;
    default:
      ret.byTarget = Cmd::MAPDATATYPE_NPC;
      break;
  }

  if (pDef->isPhysics)
  {
    if (pAtt->isPhysicBang)
    {
      ret.byLuck = 1;
    }
    else if (pAtt->isHPhysicBang)
    {
      ret.byLuck = 2;
    }
    else
    {
      ret.byLuck = 0;
    }
  }
  else
  {
    if (pAtt->isMagicBang)
    {
      ret.byLuck = 1;
    }
    else if (pAtt->isHMagicBang)
    {
      ret.byLuck = 2;
    }
    else
    {
      ret.byLuck = 0;
    }
  }
#ifdef _DEBUG
  if (pAtt->getType() == zSceneEntry::SceneEntry_Player) Channel::sendSys((SceneUser *)pAtt,Cmd::INFO_TYPE_GAME,"�㲥������%s����%s ������ʽ[%s] ħ������[%s] ������[%s] ret.byLuck[%u]",pAtt->name,pDef->name,pDef->isPhysics?"����":"����",pAtt->isMagicBang?"��":"��",pAtt->isPhysicBang?"��":"��",ret.byLuck);
#endif
  pAtt->isPhysicBang=false;
  pAtt->isMagicBang=false;
  pAtt->isHPhysicBang=false;
  pAtt->isHMagicBang=false;
  ret.byDirect = rev?rev->byDirect:pAtt->getDir();
  ret.sdwHP = sdwHP;
  if (ret.sdwHP >= 0)
  {
    ret.byRetcode = Cmd::RTMAGIC_SUCCESS;
  }
  else
  {
    ret.byRetcode = Cmd::RTMAGIC_FAILURE;
  }


  /*
  //���ٲ������û�
  if (pDef->getType()==zSceneEntry::SceneEntry_NPC)
  {
    ((SceneNpc *)pDef)->forceChaseUser(pAtt);
#ifdef _DEBUG
    if (pAtt->getType() == zSceneEntry::SceneEntry_Player) Channel::sendSys((SceneUser *)pAtt,Cmd::INFO_TYPE_GAME,"�㲥������%s����%s �˺�����%u",pAtt->name,pDef->name,ret.wdHP);
#endif
  }
  */

  if (pAtt->getType() == zSceneEntry::SceneEntry_Player)
  {
    ((SceneUser*)pAtt)->sendCmdToMe(&ret,sizeof(ret));
  }
  else if (pAttMaster && pAttMaster->getType() == zSceneEntry::SceneEntry_Player)
  {
    ((SceneUser*)pAttMaster)->sendCmdToMe(&ret,sizeof(ret));
  }
  if (pDef->getType() == zSceneEntry::SceneEntry_Player)
  {
    ((SceneUser*)pDef)->sendCmdToMe(&ret,sizeof(ret));
  }
  else if (pDefMaster && pDefMaster->getType() == zSceneEntry::SceneEntry_Player)
  {
    ((SceneUser*)pDefMaster)->sendCmdToMe(&ret,sizeof(ret));
  }
  ret1.byTarget=ret.byTarget;        /**< Ŀ�����ͣ�enumMapDataType */
  ret1.dwUserTempID=ret.dwUserTempID;      /**< Ŀ����ʱ��� */
  ret1.bySrc=ret.bySrc;          /**< ���������ͣ�enumMapDataType */
  ret1.dwSrcTempID=ret.dwSrcTempID;      /**< ��������ʱ��� */
  pDef->sendCmdToSelected(&ret1,sizeof(ret1));
  return pAtt->scene->sendCmdToNine(pAtt->getPosI(),&ret1,sizeof(ret1),pAtt->dupIndex);
}

/**
 * \brief ����������״̬
 *
 * \pAtt ������
 *
 * \pDef ��������
 */
bool ScenePkState::hasProtected()
{
  return protect_time?true:false;
}

/**
 * \brief ���һ����ҽ����Լ��������б�
 *
 * \param pThis:�����Լ������
 * \param attid:�����Լ������ID
 */
bool ScenePkState::addProtect(SceneUser * pThis,DWORD time)
{
  bool bret = false;
  if (!(pThis->charbase.goodness & Cmd::GOODNESS_ATT))
  {
    bret=true;
  }
  else if (!protect_time)
  {
    bret=true;
  }
  pThis->charbase.goodness |= Cmd::GOODNESS_ATT;  
  if (time)
  {
    protect_time = time;
  }
  else
  {
    protect_time = 120000/ScenePkState::protectPeriod;
  }
  return bret;
}

/**
 * \brief ��ʼ�������ʱ,����Ƿ��������״̬��
 *
 * ��SceneEntryPk::AttackMe������,�����δ���뵽�����б���,�����
 * 
 * \param  psAtt: ������
 * \param  psDef: ��������
 */
void ScenePk::checkProtect(SceneEntryPk *psAtt,SceneEntryPk *psDef)
{
  if ((!psAtt)||(!psDef)) return;

  if (zSceneEntry::SceneEntry_NPC==psAtt->getType())
  {
    if (psAtt->getTopMaster())
      checkProtect(psAtt->getMaster(),psDef);
    return;
  }
  if (zSceneEntry::SceneEntry_NPC==psDef->getType())
  {
    if (psDef->getTopMaster())
      checkProtect(psAtt,psDef->getMaster());
    return;
  }

  SceneUser *pAtt =(SceneUser *)psAtt;
  SceneUser *pDef =(SceneUser *)psDef;

  if (pAtt->scene && pAtt->scene->isNoRedScene()) return;
  if (pAtt->isWar(pDef)) return;

  if (pAtt->isCatcherState()) return;

  //����Ǻ����˺�����
  SWORD swGoodAtt = (SWORD)(pAtt->charbase.goodness & 0x0000FFFF);
  SWORD swGoodDef = (SWORD)(pDef->charbase.goodness & 0x0000FFFF);
  if (!pAtt->issetUState(Cmd::USTATE_GUARD) && 
      (!psDef->scene->checkZoneType(pDef->getPos(),ZoneTypeDef::ZONE_SPORTS)  
      && !psAtt->scene->checkZoneType(psAtt->getPos(),ZoneTypeDef::ZONE_SPORTS)) 
      && pAtt->charbase.country == pDef->charbase.country)
  {
    if (swGoodAtt <= 60 && swGoodDef <= 60)
    {
      if (!pDef->pkState.hasProtected())
      {
        if (pAtt->pkState.addProtect(pAtt))
        {
          if (pAtt->mask.is_masking())
            Channel::sendSys(pDef,Cmd::INFO_TYPE_GAME,"������ ��������,������������Զ���������������");    
          else
            Channel::sendSys(pDef,Cmd::INFO_TYPE_GAME,"%s��������,������������Զ���������������",pAtt->name);    
          if (pDef->mask.is_masking())
          {
            Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"�㹥���� ������,�������� ������ ���Զ��������������");
            if (pDef->charbase.pkaddition>1800)
            {
              Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"�Է�����PK����״̬��");    
            }
          }
          else
          {
            Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"�㹥���� %s,�������� %s ���Զ��������������",pDef->name,pDef->name);    
            if (pDef->charbase.pkaddition>1800)
            {
              Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"�Է�����PK����״̬��");    
            }
          }
          if (pAtt->charbase.pkaddition>1800)
          {
            Channel::sendSys(pAtt,Cmd::INFO_TYPE_SYS,"���PK����״̬�����");
          }
          pAtt->charbase.pkaddition=0;
          if (pAtt->issetUState(Cmd::USTATE_PK))
          {
            pAtt->clearUState(Cmd::USTATE_PK);
            pAtt->sendtoSelectedPkAdditionState();
            pAtt->clearStateToNine(Cmd::USTATE_PK);
          }
          pAtt->sendGoodnessToNine();
        }
      }
    }
  }
  /*
   * \brief
   * ֪ͨ�����������Ա仯
   */
  //ScenePk::sendChangedUserData(pDef);
}

/**
 * \brief �㲥�յ��Ĺ���ָ��
 *
 * �㲥����ָ��������ڵ����
 *
 * \rev,�յ���ָ��
 * \pAtt ������
 *
 * \return �㲥�Ƿ�ɹ�
 */
bool ScenePk::attackUserCmdToNine(const Cmd::stAttackMagicUserCmd *rev,SceneEntryPk *pAtt)
{
  Cmd::stAttackMagicUserCmd attCmd;
  bcopy(rev,&attCmd,sizeof(attCmd),sizeof(attCmd));
  attCmd.dwUserTempID = pAtt->tempid;


  return pAtt->scene->sendCmdToNine(pAtt->getPosI(),&attCmd,sizeof(attCmd),pAtt->dupIndex);
}

/*
 * \brief ����������ٶ��Ƿ����
 * 
 * \pAtt �û�
 * \speed ��ǰ��ɫ�ٶ�
 * \return �Ƿ����
 */
bool ScenePkState::speedOutP(WORD speed,DWORD dwTime)
{
  if (dwTime - lastPTime < speed)
  {
    if (speed==0) speed=1;
    if ((float)(dwTime-lastPTime)/(float)speed < 0.94f)
      return false;
  }
  lastPTime = dwTime;
  return true;
}

/*
 * \brief ���ħ�������ٶ��Ƿ����
 * \speed ��ǰ��ɫ�ٶ�
 * \return �Ƿ����
 */
bool ScenePkState::speedOutM(WORD speed,DWORD dwTime)
{
  if (dwTime - lastMTime < speed)
  {
    if (speed==0) speed=1;
    if ((float)(dwTime - lastMTime)/(float)speed < 0.94f)
      return false;
  }
  lastMTime = dwTime;
  return true;
}

/*
 * \brief ��⹥���ٶ��Ƿ����
 * 
 * \pAtt �û�
 * \rev ���յ���ָ��
 * \return �Ƿ����
 */
bool ScenePk::checkAttackSpeed(SceneUser *pAtt,const Cmd::stAttackMagicUserCmd *rev)
{
#ifdef _DEBUG

#endif

  switch(rev->wdMagicType)
  {
	case SERVER_SKILL_ATTACK_NORMAL:		//����������ͨ����
	case SERVER_SKILL_DAGGER_ATTACK_NORMAL: //˫������(ذ��)��ͨ����
	case SERVER_SKILL_HANDS_ATTACK_NORMAL:	//˫��������ͨ����
    {
      if (pAtt->pkState.speedOutP((WORD)(pAtt->charstate.attackspeed/640.0f*1320),rev->dwTimestamp))
      {
        return true;
      }
      else
      {
        //Zebra::logger->info("�����ٶȹ���(%s(%ld),%ld)",pAtt->name,pAtt->id,rev->byAction);
        return false;
      }
    }
	case SERVER_SKILL_DART_ATTACK_NORMAL:	//����������ͨ����:
    {
      if (pAtt->pkState.speedOutP((WORD)(pAtt->charstate.attackspeed/640.0f*1320),rev->dwTimestamp))
      {
        return true;
      }
      else
      {
        //Zebra::logger->info("�����ٶȹ���(%s(%ld),%ld)",pAtt->name,pAtt->id,rev->byAction);
        return false;
      }
    }
    default:
    {
      WORD value = 1320;
      switch(pAtt->charbase.useJob)
      {
	  case JOB_FIGHTER:		//սʿ
	  case JOB_THIEVES:		//����
		  value = 1320;
		  break;
	  case JOB_MASTER:		//��ʦ
	  case JOB_PASTOR:		//��ʦ
          value = 1560;
          break;
	  default:
		  value = 1880;
		  break;
      }

      if (pAtt->pkState.speedOutM((WORD)(pAtt->charstate.attackspeed/640.0f*value),rev->dwTimestamp))
      {
      //��������֮������й���������������
        zSkill *s = pAtt->usm.findSkill(rev->wdMagicType);
        if (s)
        {
			//sky �жϸü����Ƿ�����ս����ʹ��
			if(s->base->useBook == 1)
			{
				//sky ���ܵĻ��ж����û��Ƿ���ս��״̬
				if(pAtt->IsPveOrPvp() == USE_FIGHT_PVE || pAtt->IsPveOrPvp() == USE_FIGHT_PVP)
				{
					pAtt->sendMessageToMe("�ü��ܲ�������ս����ʹ��!");
					return false;
				}
			}

			if (!s->canUse())
			{
#ifdef _DEBUG
				zRTime ctv;
				Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"���ܹ����ٶȹ��������ȴ�ڼ�",s->actionbase->dtime,ctv.msecs() - s->lastUseTime);
#endif
				//Zebra::logger->info("���ܹ����ٶȹ��������ȴ�ڼ�(%s(%ld),%ld)",pAtt->name,pAtt->id,rev->byAction);
				return false;
			}
			pAtt->lastUseSkill = rev->wdMagicType;
			switch(rev->wdMagicType)
			{
			case 219://��������սʿ
			case 224://�����Ļ�
			case 229://���ɺ�������
			case 235://������������
			case 239://�������
			case 244://�����콫
			case 249://��������
			case 255://������������
			case 259://�ں�ʯ��
			case 264://�ںϷ���
			case 269://�ں�����
			case 275://�ں���ħ����
				{
					zSkill *ss = pAtt->usm.findSkill(219);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(224);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(229);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(235);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(239);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(244);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(249);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(255);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(259);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(264);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(269);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(275);
					if (ss) ss->canUse();
				}
				break;
			case 120://��������   120
			case 128://��������   128
			case 123://��������   123
			case 125://��������   125
			case 130://��������   130
			case 133://��������   133
			case 137://��ħ����   137
			case 138://��������   138
				{
					zSkill *ss = pAtt->usm.findSkill(120);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(128);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(123);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(125);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(130);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(133);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(137);
					if (ss) ss->canUse();
					ss = pAtt->usm.findSkill(138);
					if (ss) ss->canUse();
				}
				break;
			default:
				break;
			}
			return true;
		}
		else
		{
			Zebra::logger->info("���:��ͼʹ��û��ѧϰ�ļ���(%s(%ld),%ld) ����id = %ld",pAtt->name,pAtt->id,rev->byAction,rev->wdMagicType);
			return false;
		}
	  }
	  else
	  {
		  //Zebra::logger->info("�����ٶȹ���(%s(%ld),%ld)",pAtt->name,pAtt->id,rev->byAction);
		  return false;
	  }
	  //      }
	  //      else
	  //      {
	  //        if (pAtt->pkState.speedOutM((WORD)((pAtt->charstate.attackspeed/640.0f*1000)*(pAtt->skillValue.mgspeed==0?1:(pAtt->skillValue.mgspeed/100.0f)))))
	  //        {
	  //          zSkill *s = pAtt->usm.findSkill(rev->wdMagicType);
	  //          if (s)
	  //          {
	  //            if (!s->canUse())
	  //            {
	  //#ifdef _DEBUG
	  //              zRTime ctv;
	  //              Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"���ܹ����ٶȹ��������ȴ�ڼ�",s->actionbase->dtime,ctv.msecs() - s->lastUseTime);
	  //#endif
	  //              Zebra::logger->info("���ܹ����ٶȹ��������ȴ�ڼ�(%s(%ld),%ld)",pAtt->name,pAtt->id,rev->byAction);
	  //              return false;
	  //            }
	  //            pAtt->lastUseSkill = rev->wdMagicType;
	  //            return true;
	  //          }
	  //          else
	  //          {
	  //            Zebra::logger->info("���:��ͼʹ��û��ѧϰ�ļ���(%s(%ld),%ld) ����id = %ld",pAtt->name,pAtt->id,rev->byAction,rev->wdMagicType);
	  //            return false;
	  //          }
	  //        }
	  //        else
	  //        {
	  //#ifdef _DEBUG
	  //            Channel::sendSys(pAtt,Cmd::INFO_TYPE_GAME,"û�й������л����ʱ��,ʩ��ʧ��");
	  //#endif
	  //          ScenePk::attackFailToMe(rev,pAtt);
	  //          Zebra::logger->info("�����ٶȹ���(%s(%ld),%ld)",pAtt->name,pAtt->id,rev->byAction);
	  //          return false;
	  //        }
	  //      }
	}
  }

  return true;
}


#define checkholyp(percent)\
pAtt->isPhysicBang = false;\
pAtt->isHPhysicBang = false;\
if (selectByPercent(pAtt->charstate.bang))\
{\
  pAtt->pkValue.pdamage = (WORD)(pAtt->pkValue.pdamage * 1.5f);\
  pAtt->isPhysicBang = true;\
}\
if (selectByPercent(pAtt->packs.equip.getEquips().get_holy()))\
{\
  pAtt->pkValue.pdamage = (WORD)(pAtt->pkValue.pdamage * 1.5f);\
  pAtt->isHPhysicBang = true;\
}

#define checkholym(percent)\
pAtt->isMagicBang = false;\
pAtt->isHMagicBang = false;\
if (selectByPercent(pAtt->charstate.bang))\
{\
  pAtt->pkValue.mdamage = (WORD)(pAtt->pkValue.mdamage * 1.5f);\
  pAtt->isMagicBang = true;\
}\
if (selectByPercent(pAtt->packs.equip.getEquips().get_holy()))\
{\
  pAtt->pkValue.mdamage = (WORD)(pAtt->pkValue.mdamage * 1.5f);\
  pAtt->isHMagicBang = true;\
}



/**
 * \brief ������Ҷ���ҵ������˺�ֵ 
 *
 * ��SceneUser:preAttackMe�б�����
 *
 * \param rev: ��������
 * \param pAtt: ������
 * \param pDef: ������
 *
 */
void ScenePk::calpdamU2U(const Cmd::stAttackMagicUserCmd *rev,SceneUser *pAtt,SceneUser *pDef)
{
  float percent=0.0f;
  //���д���
  switch(pAtt->IsOppose(pDef->charstate.defencefive))
  {
    //�׿���
    case  1:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ���������
        pAtt->pkValue.pdamage = (DWORD)(pAtt->pkpreValue.fivedam+(pAtt->pkpreValue.fivemaxdam-pAtt->pkpreValue.fivedam)*percent);
        checkholyp(percent);
        //pAtt->pkValue.pdamage= (DWORD)(pAtt->pkValue.pdamage*(1.0f+pAtt->packs.equip.getEquips().getAttFivePoint()/100.0f));

        pDef->pkValue.pdefence = pDef->pkpreValue.nofivedef;
      }
      break;
    case 2:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ���������
        pAtt->pkValue.pdamage = (DWORD)(pAtt->pkpreValue.nofivedam+(pAtt->pkpreValue.nofivemaxdam-pAtt->pkpreValue.nofivedam)*percent);

        checkholyp(percent);

        /// ��������ߵ���������������е�����Ӱ��
        pDef->pkValue.pdefence = pDef->pkpreValue.fivedef;
        //pDef->pkValue.pdefence = (DWORD)(pDef->pkValue.pdefence*(1.0f+pDef->packs.equip.getEquips().getDefFivePoint()/100.0f));
      }
      break;
    case 0:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ���������
        pAtt->pkValue.pdamage = (DWORD)(pAtt->pkpreValue.nofivedam+(pAtt->pkpreValue.nofivemaxdam-pAtt->pkpreValue.nofivedam)*percent);

        checkholyp(percent);

        /// ��������ߵ���������������е�����Ӱ��
        pDef->pkValue.pdefence = pDef->pkpreValue.nofivedef;
      }
      break;
    default:
      break;

  }

  if (selectByPercent(pAtt->packs.equip.getEquips().get_ignoredef())) pDef->pkValue.pdefence = 0;
}

/**
 * \brief ������Ҷ���ҵ�ħ���˺�ֵ
 *
 * ��SceneUser::preAttackMe�б�����
 *
 * \param rev: ��������
 * \param pAtt: ������
 * \param pDef: ������
 *
 */
void ScenePk::calmdamU2U(const Cmd::stAttackMagicUserCmd *rev,SceneUser *pAtt,SceneUser *pDef)
{
  float percent=0.0f;
  //���д���
  switch(pAtt->IsOppose(pDef->charstate.defencefive))
  {
    //�׿���
    case  1: // pAtt�˶Է�
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ�ħ��������
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkpreValue.fivemdam+(pAtt->pkpreValue.fivemaxmdam-pAtt->pkpreValue.fivemdam)*percent);
        checkholym(percent);
        //pAtt->pkValue.mdamage= (DWORD)(pAtt->pkValue.mdamage*(1.0f+pAtt->packs.equip.getEquips().getAttFivePoint()/100.0f));

        /// ��������ߵ�ħ�������������е�����Ӱ��
        pDef->pkValue.mdefence = pDef->pkpreValue.nofivemdef;
      }
      break;
    case 2: // �Է���pAtt
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ�ħ��������
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkpreValue.nofivemdam+(pAtt->pkpreValue.nofivemaxmdam-pAtt->pkpreValue.nofivemdam)*percent);

        checkholym(percent);

        /// ��������ߵ�ħ�������������е�����Ӱ��
        pDef->pkValue.mdefence = pDef->pkpreValue.fivemdef;
        //pDef->pkValue.mdefence = (DWORD)(pDef->pkValue.mdefence*(1.0f+pDef->packs.equip.getEquips().getDefFivePoint()/100.0f));
      }
      break;
    case 0: // ����˹�ϵ
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ�ħ��������
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkpreValue.nofivemdam+(pAtt->pkpreValue.nofivemaxmdam-pAtt->pkpreValue.nofivemdam)*percent);

        checkholym(percent);

        /// ��������ߵ�ħ�������������е�����Ӱ��
        pDef->pkValue.mdefence = pDef->pkpreValue.nofivemdef;
      }
      break;
    default:
      break;

  }
  if (selectByPercent(pAtt->packs.equip.getEquips().get_ignoredef())) pDef->pkValue.mdefence = 0;
}

/**
 * \brief ������Ҷ�NPC�������˺�ֵ 
 *
 * ��SceneUser:preAttackMe�б�����
 *
 * \param rev: ��������
 * \param pAtt: ������
 * \param pDef: ������
 *
 */
void ScenePk::calpdamU2N(const Cmd::stAttackMagicUserCmd *rev,SceneUser *pAtt,SceneNpc *pDef)
{
  float percent=0.0f;
  //���д���
  switch(pAtt->IsOppose(pDef->npc->five))
  {
	  //�׿���
  case  1:
	  {
		  percent=randBetween(0,100)/100.0f;
		  if (pAtt->maxattack) percent = 1.0f;
		  if (pAtt->attacklow) percent = 0.0f;
		  if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
		  /// ���㹥���ߵ���������
		  pAtt->pkValue.pdamage = (DWORD)(pAtt->pkpreValue.fivedam+(pAtt->pkpreValue.fivemaxdam-pAtt->pkpreValue.fivedam)*percent);
#ifdef _DEBUG
		  Zebra::logger->info("1-----------------------------------------------------------");
		  Zebra::logger->info("pkpreValue.fivedam=%u,pkpreValue.fivemaxdam=%u",pAtt->pkpreValue.fivedam,pAtt->pkpreValue.fivemaxdam);
		  Zebra::logger->info("Ԥ����pkValue.pdamage=[%u] percent=[%f]",pAtt->pkValue.pdamage,percent);
#endif
		  checkholyp(percent);
#ifdef _DEBUG
		  Zebra::logger->info("��������Ԥ����pkValue.pdamage=[%u] ����[%s] ħ����[%s]",pAtt->pkValue.pdamage,pAtt->isPhysicBang?"��":"��",pAtt->isMagicBang?"��":"��");
#endif
		  //pAtt->pkValue.pdamage= (DWORD)(pAtt->pkValue.pdamage*(1.0f+pAtt->packs.equip.getEquips().getAttFivePoint()/100.0f));

		  /// ��������ߵ����������
		  if( pDef->npc->kind == NPC_TYPE_GHOST )   //Ԫ�����⴦��ֱ�Ӽ������˵�ֵsky
		  {
			  pDef->pkValue.pdefence = ((ScenePet *)pDef)->petData.def;
		  }
		  else
		  {
			  pDef->pkValue.pdefence= randBetween(pDef->getMinPDefence(),pDef->getMaxPDefence());
		  }
		  if (pDef->pkValue.pdefence > (DWORD)pDef->skillValue.dpdef)
			  pDef->pkValue.pdefence -= (DWORD)pDef->skillValue.dpdef;
		  else
			  pDef->pkValue.pdefence = 0;
	  }
	  break;
    case 2:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ���������
        pAtt->pkValue.pdamage = (DWORD)(pAtt->pkpreValue.nofivedam+(pAtt->pkpreValue.nofivemaxdam-pAtt->pkpreValue.nofivedam)*percent);
#ifdef _DEBUG
              Zebra::logger->info("1-----------------------------------------------------------");
          Zebra::logger->info("pkpreValue.nofivedam=%u,pkpreValue.nofivemaxdam=%u",pAtt->pkpreValue.nofivedam,pAtt->pkpreValue.nofivemaxdam);
              Zebra::logger->info("Ԥ����pkValue.pdamage=[%u] percent=[%f]",pAtt->pkValue.pdamage,percent);
#endif
        checkholyp(percent);
#ifdef _DEBUG
              Zebra::logger->info("��������Ԥ����pkValue.pdamage=[%u] ����[%s] ħ����[%s]",pAtt->pkValue.pdamage,pAtt->isPhysicBang?"��":"��",pAtt->isMagicBang?"��":"��");
#endif


			  /// ��������ߵ����������
			  if( pDef->npc->kind == NPC_TYPE_GHOST )		//Ԫ�����⴦��ֱ�Ӽ������˵�ֵsky
			  {
				  pDef->pkValue.pdefence = ((ScenePet *)pDef)->petData.def;
			  }
			  else
			  {
				  pDef->pkValue.pdefence = randBetween(pDef->getMinPDefence(),pDef->getMaxPDefence());
			  }

			  pDef->pkValue.pdefence = (DWORD)(pDef->pkValue.pdefence*(1.0f +pDef->npc->fivepoint/100.0f));
			  if (pDef->pkValue.pdefence > (DWORD)pDef->skillValue.dpdef)
				  pDef->pkValue.pdefence -=(DWORD) pDef->skillValue.dpdef;
			  else
				  pDef->pkValue.pdefence = 0;
	  }
	  break;
    case 0:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ���������
        pAtt->pkValue.pdamage = (DWORD)((float)pAtt->pkpreValue.nofivedam+((float)(pAtt->pkpreValue.nofivemaxdam-pAtt->pkpreValue.nofivedam))*percent);

#ifdef _DEBUG
              Zebra::logger->info("1-----------------------------------------------------------");
          Zebra::logger->info("pkpreValue.nofivedam=%u,pkpreValue.nofivemaxdam=%u",pAtt->pkpreValue.nofivedam,pAtt->pkpreValue.nofivemaxdam);
              Zebra::logger->info("Ԥ����pkValue.pdamage=[%u] percent=[%f]",pAtt->pkValue.pdamage,percent);
#endif

        checkholyp(percent);
#ifdef _DEBUG
              Zebra::logger->info("��������Ԥ����pkValue.pdamage=[%u] ����[%s] ħ����[%s]",pAtt->pkValue.pdamage,pAtt->isPhysicBang?"��":"��",pAtt->isMagicBang?"��":"��");
#endif

        /// ��������ߵ����������
			  if( pDef->npc->kind == NPC_TYPE_GHOST )   //Ԫ�����⴦��ֱ�Ӽ������˵�ֵsky
			  {
				  pDef->pkValue.pdefence = ((ScenePet *)pDef)->petData.def;
			  }
			  else
			  {
				  pDef->pkValue.pdefence= 
					  randBetween(pDef->getMinPDefence(),pDef->getMaxPDefence());
			  }
			  if (pDef->pkValue.pdefence > (DWORD)pDef->skillValue.dpdef)
				  pDef->pkValue.pdefence -= (DWORD)pDef->skillValue.dpdef;
			  else
				  pDef->pkValue.pdefence = 0;
      }
      break;
    default:
      break;

  }
  if (selectByPercent(pAtt->packs.equip.getEquips().get_ignoredef())) pDef->pkValue.pdefence = 0;
}

/**
 * \brief ������Ҷ�NPC��ħ���˺�ֵ 
 *
 * ��SceneUser:preAttackMe�б�����
 *
 * \param rev: ��������
 * \param pAtt: ������
 * \param pDef: ������
 *
 */
void ScenePk::calmdamU2N(const Cmd::stAttackMagicUserCmd *rev,SceneUser *pAtt,SceneNpc *pDef)
{
  float percent=0.0f;
  //���д���
  switch(pAtt->IsOppose(pDef->npc->five))
  {
    //�׿���
    case  1:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ�ħ��������
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkpreValue.fivemdam+(pAtt->pkpreValue.fivemaxmdam-pAtt->pkpreValue.fivemdam)*percent);
#ifdef _DEBUG
              Zebra::logger->info("1-----------------------------------------------------------");
              Zebra::logger->info("Ԥ����pkValue.mdamage=[%u]",pAtt->pkValue.mdamage);
#endif
        checkholym(percent);
#ifdef _DEBUG
              Zebra::logger->info("��������Ԥ����pkValue.mdamage=[%u] ����[%s] ħ����[%s]",pAtt->pkValue.mdamage,pAtt->isPhysicBang?"��":"��",pAtt->isMagicBang?"��":"��");
#endif
        //pAtt->pkValue.mdamage= (DWORD)(pAtt->pkValue.mdamage*(1+pAtt->packs.equip.getEquips().getAttFivePoint()/100.0f));

        /// ��������ߵ�ħ��������
			  if( pDef->npc->kind == NPC_TYPE_GHOST ) //Ԫ�����⴦��ֱ�Ӽ������˵�ֵsky
			  {
				  pDef->pkValue.mdefence = ((ScenePet *)pDef)->petData.mdef;
			  }
			  else
			  {
				  pDef->pkValue.mdefence= 
					  randBetween(pDef->getMinMDefence(),pDef->getMaxMDefence());
			  }
      }
      break;
    case 2:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ�ħ��������
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkpreValue.nofivemdam+(pAtt->pkpreValue.nofivemaxmdam-pAtt->pkpreValue.nofivemdam)*percent);

#ifdef _DEBUG
              Zebra::logger->info("2------------------------------------------------[%u]-----------",pDef->npc->five);
              Zebra::logger->info("Ԥ����pkValue.mdamage=[%u]",pAtt->pkValue.mdamage);
#endif
        checkholym(percent);
#ifdef _DEBUG
              Zebra::logger->info("��������Ԥ����pkValue.mdamage=[%u] ����[%s] ħ����[%s]",pAtt->pkValue.mdamage,pAtt->isPhysicBang?"��":"��",pAtt->isMagicBang?"��":"��");
#endif

			  /// ��������ߵ�ħ��������
			  if( pDef->npc->kind == NPC_TYPE_GHOST )  //Ԫ�����⴦��ֱ�Ӽ������˵�ֵsky
			  {
				  pDef->pkValue.mdefence = ((ScenePet *)pDef)->petData.mdef;
			  }
			  else
			  {
				  pDef->pkValue.mdefence = randBetween(pDef->getMinMDefence(),pDef->getMaxMDefence());
			  }

			  pDef->pkValue.mdefence = (DWORD)(pDef->pkValue.mdefence*(1.0f +pDef->npc->fivepoint/100.0f));
      }
      break;
    case 0:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;
        /// ���㹥���ߵ�ħ��������
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkpreValue.nofivemdam+(pAtt->pkpreValue.nofivemaxmdam-pAtt->pkpreValue.nofivemdam)*percent);

#ifdef _DEBUG
              Zebra::logger->info("0-----------------------------------------------------------");
              Zebra::logger->info("Ԥ����pkValue.mdamage=[%u]",pAtt->pkValue.mdamage);
#endif
        checkholym(percent);
#ifdef _DEBUG
              Zebra::logger->info("��������Ԥ����pkValue.mdamage=[%u] ����[%s] ħ����[%s]",pAtt->pkValue.mdamage,pAtt->isPhysicBang?"��":"��",pAtt->isMagicBang?"��":"��");
#endif

        /// ��������ߵ�ħ��������
			  if( pDef->npc->kind == NPC_TYPE_GHOST )  //Ԫ�����⴦��ֱ�Ӽ������˵�ֵsky
			  {
				  pDef->pkValue.mdefence = ((ScenePet *)pDef)->petData.mdef;
			  }
			  else
			  {
				  pDef->pkValue.mdefence= 
					  randBetween(pDef->getMinMDefence(),pDef->getMaxMDefence());
			  }
      }
      break;
    default:
      break;

  }
  if (selectByPercent(pAtt->packs.equip.getEquips().get_ignoredef())) pDef->pkValue.mdefence = 0;
}

/**
 * \brief ����NPC����ҵ������˺�ֵ 
 *
 * ��SceneUser:preAttackMe�б�����
 *
 * \param rev: ��������
 * \param pAtt: ������
 * \param pDef: ������
 *
 */
void ScenePk::calpdamN2U(const Cmd::stAttackMagicUserCmd *rev,SceneNpc *pAtt,SceneUser *pDef)
{
  float percent=0.0f;
  //���д���,�������˵Ĵ����෴
  switch(pAtt->IsOppose(pDef->charstate.defencefive))
  {
    //NPC����
    case  2:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.pdamage = 
				randBetween(((ScenePet *)pAtt)->petData.atk, ((ScenePet *)pAtt)->petData.maxatk );
		}
		else
		{
			pAtt->pkValue.pdamage = (DWORD)(pAtt->getMinPDamage()+(pAtt->getMaxPDamage()-pAtt->getMinPDamage())*percent);
		}
		pAtt->pkValue.pdamage = (DWORD)(pAtt->pkValue.pdamage*(1.0f +pAtt->npc->fivepoint/100.0f));

        /// ��������ߵ�����������������е�����Ӱ��
        pDef->pkValue.pdefence = pDef->pkpreValue.nofivedef;
      }
      break;
    case 1:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.pdamage = 
				randBetween(((ScenePet *)pAtt)->petData.atk, ((ScenePet *)pAtt)->petData.maxatk );
		}
		else
		{
			pAtt->pkValue.pdamage = (DWORD)(pAtt->getMinPDamage()+(pAtt->getMaxPDamage()-pAtt->getMinPDamage())*percent);
		}

        /// ��������ߵ���������������е�����Ӱ��
        pDef->pkValue.pdefence = pDef->pkpreValue.fivedef;
        //pDef->pkValue.pdefence = (DWORD)(pDef->pkValue.pdefence*(1+pDef->packs.equip.getEquips().getDefFivePoint()/100.0f));
      }
      break;
    case 0:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.pdamage = 
				randBetween(((ScenePet *)pAtt)->petData.atk, ((ScenePet *)pAtt)->petData.maxatk );
		}
		else
		{
			pAtt->pkValue.pdamage = (DWORD)(pAtt->getMinPDamage()+(pAtt->getMaxPDamage()-pAtt->getMinPDamage())*percent);
		}

        /// ��������ߵ���������������е�����Ӱ��
        pDef->pkValue.pdefence = pDef->pkpreValue.nofivedef;
      }
      break;
    default:
      break;

  }
}


/**
 * \brief ����NPC����ҵ�ħ���˺�ֵ 
 *
 * ��SceneUser:preAttackMe�б�����
 *
 * \param rev ����ָ��
 * \param pAtt ������
 * \param pDef ������
 *
 */
void ScenePk::calmdamN2U(const Cmd::stAttackMagicUserCmd *rev,SceneNpc *pAtt,SceneUser *pDef)
{
  float percent=0.0f;
  //���д���,�������˵Ĵ����෴
  switch(pAtt->IsOppose(pDef->charstate.defencefive))
  {
    //NPC����
    case  2:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵķ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.mdamage = 
				randBetween(((ScenePet *)pAtt)->petData.matk, ((ScenePet *)pAtt)->petData.maxmatk );
		}
		else
		{
			pAtt->pkValue.mdamage = (DWORD)(pAtt->getMinMDamage()+(pAtt->getMaxMDamage()-pAtt->getMinMDamage())*percent);
		}
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkValue.mdamage*(1.0f +pAtt->npc->fivepoint/100.0f));

        /// �������жԷ�����������Ӱ��
        pDef->pkValue.mdefence = pDef->pkpreValue.nofivemdef;
      }
      break;
    case 1:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵķ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.mdamage = 
				randBetween(((ScenePet *)pAtt)->petData.matk, ((ScenePet *)pAtt)->petData.maxmatk );
		}
		else
		{
			pAtt->pkValue.mdamage = (DWORD)(pAtt->getMinMDamage()+(pAtt->getMaxMDamage()-pAtt->getMinMDamage())*percent);
		}

        /// �������жԷ�����������Ӱ��
        pDef->pkValue.mdefence = pDef->pkpreValue.fivemdef;
        //pDef->pkValue.mdefence = (DWORD)(pDef->pkValue.mdefence*(1+pDef->packs.equip.getEquips().getDefFivePoint()/100.0f));
      }
      break;
    case 0:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵķ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.mdamage = 
				randBetween(((ScenePet *)pAtt)->petData.matk, ((ScenePet *)pAtt)->petData.maxmatk );
		}
		else
		{
			pAtt->pkValue.mdamage = (DWORD)(pAtt->getMinMDamage()+(pAtt->getMaxMDamage()-pAtt->getMinMDamage())*percent);
		}

        /// �������жԷ�����������Ӱ��
        pDef->pkValue.mdefence = pDef->pkpreValue.nofivemdef;
      }
      break;
    default:
      break;

  }
}


/**
 * \brief ����NPC��NPC��ħ���˺�ֵ 
 *
 * ��SceneUser:preAttackMe�б�����
 *
 * \param rev ����ָ��
 * \param pAtt ������
 * \param pDef ������
 *
 */
void ScenePk::calmdamN2N(const Cmd::stAttackMagicUserCmd *rev,SceneNpc *pAtt,SceneNpc *pDef)
{
  //TODO N2N��������˼��㣨������
  float percent=0.0f;
  //���д���,�������˵Ĵ����෴
  switch(pAtt->IsOppose(pDef->npc->five))
  {
    //NPC����
    case  2:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵķ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.mdamage = 
				randBetween(((ScenePet *)pAtt)->petData.matk, ((ScenePet *)pAtt)->petData.maxmatk );
		}
		else
		{
			pAtt->pkValue.mdamage = (DWORD)(pAtt->getMinMDamage()+(pAtt->getMaxMDamage()-pAtt->getMinMDamage())*percent);
		}
        pAtt->pkValue.mdamage = (DWORD)(pAtt->pkValue.mdamage*(1.0f +pAtt->npc->fivepoint/100.0f));


        /// ��������ߵ�ħ��������
        pDef->pkValue.mdefence= 
          randBetween(pDef->getMinMDefence(),pDef->getMaxMDefence());
        if (pDef->pkValue.pdefence > (DWORD)pDef->skillValue.dpdef)
          pDef->pkValue.pdefence -= (DWORD)pDef->skillValue.dpdef;
        else
          pDef->pkValue.pdefence = 0;
      }
      break;
    case 1:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵķ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.mdamage = 
				randBetween(((ScenePet *)pAtt)->petData.matk, ((ScenePet *)pAtt)->petData.maxmatk );
		}
		else
		{
			pAtt->pkValue.mdamage = (DWORD)(pAtt->getMinMDamage()+(pAtt->getMaxMDamage()-pAtt->getMinMDamage())*percent);
		}

        /// ��������ߵ�ħ��������
        pDef->pkValue.mdefence= 
          randBetween(pDef->getMinMDefence(),pDef->getMaxMDefence());
        pDef->pkValue.mdefence = (DWORD)(pDef->pkValue.mdefence*(1.0f +pDef->npc->fivepoint/100.0f));
        if (pDef->pkValue.pdefence > (DWORD)pDef->skillValue.dpdef)
          pDef->pkValue.pdefence -= (DWORD)pDef->skillValue.dpdef;
        else
          pDef->pkValue.pdefence = 0;
      }
      break;
    case 0:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵķ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.mdamage = 
				randBetween(((ScenePet *)pAtt)->petData.matk, ((ScenePet *)pAtt)->petData.maxmatk );
		}
		else
		{
			pAtt->pkValue.mdamage = (DWORD)(pAtt->getMinMDamage()+(pAtt->getMaxMDamage()-pAtt->getMinMDamage())*percent);
		}

        /// ��������ߵ�ħ��������
        pDef->pkValue.mdefence= 
          randBetween(pDef->getMinMDefence(),pDef->getMaxMDefence());
        if (pDef->pkValue.pdefence > (DWORD)pDef->skillValue.dpdef)
          pDef->pkValue.pdefence -= (DWORD)pDef->skillValue.dpdef;
        else
          pDef->pkValue.pdefence = 0;
      }
      break;
    default:
      break;

  }
}

/**
 * \brief ����NPC��NPC�������˺�ֵ 
 *
 * ��SceneUser:preAttackMe�б�����
 *
 * \param rev  ��������
 * \param pAtt ������
 * \param pDef ������
 *
 */

void ScenePk::calpdamN2N(const Cmd::stAttackMagicUserCmd *rev,SceneNpc *pAtt,SceneNpc *pDef)
{
  //TODO N2N��������˼��㣨����
  float percent=0.0f;
  //���д���,�������˵Ĵ����෴
  switch(pAtt->IsOppose(pDef->npc->five))
  {
    //NPC����
    case  2:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.pdamage = 
				randBetween(((ScenePet *)pAtt)->petData.atk, ((ScenePet *)pAtt)->petData.maxatk );
		}
		else
		{
			pAtt->pkValue.pdamage = (DWORD)(pAtt->getMinPDamage()+(pAtt->getMaxPDamage()-pAtt->getMinPDamage())*percent);
		}
        pAtt->pkValue.pdamage = (DWORD)(pAtt->pkValue.pdamage*(1.0f +pAtt->npc->fivepoint/100.0f));

        /// ��������ߵ����������
        pDef->pkValue.pdefence= 
          randBetween(pDef->getMinPDefence(),pDef->getMaxPDefence());
      }
      break;
    case 1:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.pdamage = 
				randBetween(((ScenePet *)pAtt)->petData.atk, ((ScenePet *)pAtt)->petData.maxatk );
		}
		else
		{
			pAtt->pkValue.pdamage = (DWORD)(pAtt->getMinPDamage()+(pAtt->getMaxPDamage()-pAtt->getMinPDamage())*percent);
		}

        /// ��������ߵ����������
        pDef->pkValue.pdefence= 
          randBetween(pDef->getMinPDefence(),pDef->getMaxPDefence());
        pDef->pkValue.pdefence = (DWORD)(pDef->pkValue.pdefence*(1.0f +pDef->npc->fivepoint/100.0f));
      }
      break;
    case 0:
      {
        percent=randBetween(0,100)/100.0f;
        if (pAtt->maxattack) percent = 1.0f;
        if (pAtt->attacklow) percent = 0.0f;
        if (rev&&(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL)&&pDef->huntermark) percent=1.0f;

        /// ���㹥���ߵ���������
		if( pAtt->npc->kind == NPC_TYPE_GHOST )
		{
			pAtt->pkValue.pdamage = 
				randBetween(((ScenePet *)pAtt)->petData.atk, ((ScenePet *)pAtt)->petData.maxatk );
		}
		else
		{
			pAtt->pkValue.pdamage = (DWORD)(pAtt->getMinPDamage()+(pAtt->getMaxPDamage()-pAtt->getMinPDamage())*percent);
		}

        /// ��������ߵ����������
        pDef->pkValue.pdefence= 
          randBetween(pDef->getMinPDefence(),pDef->getMaxPDefence());
      }
      break;
    default:
      break;

  }
}

void SceneUser::reduceGoodness(SceneNpc *pNpc)
{
  if (pNpc->npc->level < charbase.level || getGoodnessState() <= 0)
  {
    return;
  }
  if (++killedNpcNum >= 5)
  {
    killedNpcNum = 0;
    charbase.goodness --;
    switch(charbase.goodness)
    {
      case Cmd::GOODNESS_0:
      case Cmd::GOODNESS_1:
      case Cmd::GOODNESS_2_1:
      case Cmd::GOODNESS_2_2:
      case Cmd::GOODNESS_3:
      case Cmd::GOODNESS_4:
      case Cmd::GOODNESS_5:
      case Cmd::GOODNESS_6:
        {
          // ֪ͨ�ͻ������Ա仯
          // mark
          //Cmd::stAddUserMapScreenUserCmd send;
          //this->full_t_MapUserData(send.data);
          //this->scene->sendCmdToNine(getPosI(),&send,sizeof(send),false);
          this->reSendMyMapData();
        }
        break;
      default:
        break;

    }
  }
}

/**
 * \brief ��������������ϴ���
 * 
 * \rev ���յ����ݵĵ�ַ
 * \isOrigin �Ƿ��������ص�����
 *
 * \return �����Ƿ�ɹ�
 */
bool SceneUser::reliveReady(const Cmd::stOKReliveUserCmd *rev,bool isOrigin)
{
  SceneUser *pDef = this;
  bool mapchange = false;
  bool finddare = false;
  bool changeService = false;

  //��֪ͨ�������Լ�ɾ����
  Cmd::stRemoveUserMapScreenUserCmd send;
  send.dwUserTempID = tempid;
  scene->sendCmdToNineExceptMe(this,getPosI(),&send,sizeof(send));

  if (!isOrigin)
  {
    //׼����������
    zPos newPos = this->getPos();
    //�������������������
    if (this->isSpecWar(Cmd::COUNTRY_FORMAL_DARE) 
     && this->scene->getRealMapID() == 139)
    {
      Scene* pToScene = NULL;
      
      if (this->charbase.country == this->scene->getCountryID())
      {// ���ط�
        pToScene = SceneManager::getInstance().getSceneByID(
          SceneManager::getInstance().buildMapID(this->scene->getCountryID(),
            this->scene->getCountryDefBackToMapID()));
      }
      else if (this->scene->getCountryDareBackToMapID())
      {
        pToScene = SceneManager::getInstance().getSceneByID(
          SceneManager::getInstance().buildMapID(this->scene->getCountryID(),
            this->scene->getCountryDareBackToMapID()));
      }

      if (pToScene && pToScene->randzPosByZoneType(ZoneTypeDef::ZONE_PRIVATE_RELIVE,newPos,newPos))
      {
        Zebra::logger->info("����������ɹ���%s,%u,%u",pToScene->name,newPos.x,newPos.y);
        mapchange = false;
      }

      if (!mapchange)
      {
        zPos findedPos;
        pToScene->findPosForUser(newPos,findedPos);
        this->changeMap(pToScene,findedPos);
        //pToScene->refresh(this,founded ? findedPos : newPos);
        //pDef->scene->setBlock(pDef->getPos());
        finddare=true;
      }
    }
    
    if (!finddare)
    {
      if (this->charbase.country == this->scene->getCountryID())
      {
        if (this->scene->randzPosByZoneType(ZoneTypeDef::ZONE_RELIVE,newPos,newPos))
        {
          Zebra::logger->info("����������ɹ���%s,%u,%u",this->scene->name,newPos.x,newPos.y);
          mapchange = false;
        }
        else
        {
          int retcode =this->scene->changeMap(this);
          mapchange = (retcode!=0);
          changeService = (2==retcode);
          Zebra::logger->info("�л���ͼ����%s,%u,%u",this->scene->name,newPos.x,newPos.y);
        }
      }
      else
      {
        int retcode = this->scene->changeMap(this);
        mapchange = (retcode!=0);
        changeService = (2==retcode);
        Zebra::logger->info("�л������ͼ����%s,%u,%u",this->scene->name,newPos.x,newPos.y);
      }
      if (!mapchange)
      {
        zPos findedPos;
        bool founded = this->scene->findPosForUser(newPos,findedPos);
        this->removeNineEntry(getPosI());
        this->scene->refresh(this,founded ? findedPos : newPos);
        pDef->scene->setBlock(pDef->getPos());
      }
    }
  }

  if (changeService) return true;
//  skillStatusM.clearActiveSkillStatus();
  this->setupCharBase();

  Cmd::stMainUserReliveReliveUserCmd ret;
  ret.dwUserTempID = this->tempid;
  ret.x = this->getPos().x;
  ret.y = this->getPos().y;

  this->setState(SceneUser::SceneEntry_Normal);

  this->sendCmdToMe(&ret,sizeof(ret));

  if (!mapchange)
  {
    //�õ��������������Լ�
    sendNineToMe();
    //֪ͨ��������û�
    //ScenePk::sendChangedUserData �ᷢ��sendMeToNine
    //sendMeToNine();
  }
  else
  {
    sendNineToMe();
    //ScenePk::sendChangedUserData �ᷢ��sendMeToNine
    //sendMeToNine();
  }

  /*
   * \brief
   * ֪ͨ������������
   */
  ScenePk::sendChangedUserData(this);

  //Zebra::logger->debug("����״̬(%s(%ld),%X) ",this->name,this->id,this->charbase.goodness);

  return true;
}

/**
  * \brief �����
  *
  * ���ָ�������Ĵ����سǸ���,��Ǯ����,���ܸ���
  *
  * \param relive_type: ��������,��ѡֵΪ:ReliveHome,ReliveMoney,ReliveSkill,
  * \param delaytime: ����������Ϊ���ܸ���ʱ,Ϊ���ܸ��������ĵ�ʱ�䡣
  * \param data_percent: HP,SP,MP��İٷֱ�
  *
  * \return ��������,����TRUE,�򸴷����Ͳ�����ԭ������²��ܽ��д���ʱ,����FALSE
  */
bool SceneUser::relive(const int relive_type,const int delaytime,const int data_percent)
{
	int percent = data_percent;
	//using namespace Cmd;
	bool bret = false;
	bool isOrigin = false;
	DWORD punishTime = 0;

	deathWaitTime = 0;

	//����������
	if (horse.mount())
	{
		horse.mount(false,false);
	}

	if (this->charbase.reliveWeakTime >0)
	{
		this->charbase.reliveWeakTime = 0;
		this->setupCharBase();    

		Cmd::stMainUserDataUserCmd  userinfo;
		full_t_MainUserData(userinfo.data);
		sendCmdToMe(&userinfo,sizeof(userinfo));
	}

	switch(relive_type)
	{
	case Cmd::ReliveHome:
		{
			bret = true;
			isOrigin = false;
			percent = 75; //100
			npcdareflag = false;
		}
		break;
	case Cmd::ReliveMoney:
		{
			npcdareflag = false;
			DWORD cost = (int)((0.5 * charbase.level * charbase.level) + 0.5);

			//��ɫ�ȼ�<=40��ʱ,�۳����=��ʽ������*0.4 ȡ��
			if (charbase.level <= 40)
				cost = (int)(cost * 0.4);

			if (packs.checkMoney(cost) && packs.removeMoney(cost,"����")) {
				bret = true;
				isOrigin = true;
				percent = 100;
				Channel::sendMoney(this,Cmd::INFO_TYPE_GAME,cost,"������ԭ�ظ������");         
			}
			else
			{
				Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��û���㹻�Ľ�Ǯ����.");
				//ûǮ�س�����
				bret = false;
				//isOrigin = false;
				//percent = 75;
			}
		}
		break;
	//sky ս������ ֱ��ԭ�ظ���
	case Cmd::ReliveSkill:
		{
			bret = true;
			isOrigin = true;
			deathWaitTime = delaytime;
		}
		break;
	case Cmd::ReliveBattle:
		{
			bret = true;
			isOrigin = true;
			percent = 100;
		}
		break;
	case Cmd::Relive_1_min:
		{
			percent = 100;
			punishTime = 1;
		}
		break;
	case Cmd::Relive_5_min:
		{
			percent = 100;
			punishTime = 5;
		}
		break;
	case Cmd::Relive_10_min:
		{
			//bret = true;
			percent = 100;
			punishTime = 10;
		}
		break;
	default:
		{
			bret = true;
			isOrigin = false;
			percent = 100;
			npcdareflag = false;
		}
		break;
	}
	if (npcdareflag)
	{
		Zebra::logger->info("[��������NPC]����[%u]�еĽ�ɫ%s�����ܸ������»�ö�ս�ʸ�",this->charbase.septid,this->name);
	}

	if (punishTime && lastKiller)
	{
		SceneUser * pAtt = SceneUserManager::getMe().getUserByTempID(lastKiller);
		if (pAtt && pAtt->scene->getRealMapID()!=189 && pAtt->scene->getRealMapID()!=203)
		{
			DWORD cost = (int)(((0.5 * charbase.level * charbase.level) + 0.5) * punishTime);
			if ((packs.checkMoney(cost) && packs.removeMoney(cost,"����")))
			{
				Channel::sendMoney(this,Cmd::INFO_TYPE_GAME,cost,"�����˱�������");

				pAtt->charbase.punishTime = punishTime;
				Scene * s=SceneManager::getInstance().getSceneByName("������������");
				if (s)
				{
					bool suc = pAtt->changeMap(s,zPos(80,70));
					if (!suc)
						Zebra::logger->error("%s ɱ�� %s,��������ʧ��,Ŀ�� %s (%d,%d) ʱ�� %u ����",pAtt->name,name,s->name,80,70,punishTime);
					else
						Zebra::logger->error("%s ɱ�� %s,��������,ʱ�� %u ����",pAtt->name,name,punishTime);
				}
				else
				{
					//if (pAtt->guard && pAtt->guard->canMove()) pAtt->saveGuard = true;//ʹ�ڳ�����ָ��ʹ����
					//if (pAtt->adoptList.size()) pAtt->saveAdopt = true;
					Cmd::Session::t_changeScene_SceneSession cmd;
					cmd.id = pAtt->id;
					cmd.temp_id = pAtt->tempid;
					cmd.x = 80;
					cmd.y = 70;
					cmd.map_id = 0;
					cmd.map_file[0] = '\0';
					strncpy((char *)cmd.map_name,"������������",MAX_NAMESIZE);
					sessionClient->sendCmd(&cmd,sizeof(cmd));
				}

				Channel::sendCountryInfo(charbase.country,Cmd::INFO_TYPE_EXP,"%s ����ɱ�� %s,����׽�ù鰸,���� %u ����",pAtt->name,name,punishTime);
			}
			else
				Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��û���㹻�Ľ�Ǯ����");
		}
		else
			Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����Ѿ���֪ȥ��...");

		if (lastKiller && !bret)
		{
			lastKiller = 0;

			Cmd::stMainUserDeathReliveUserCmd death;
			death.dwUserTempID = tempid;
			death.deathType = lastKiller?1:0;

			sendCmdToMe(&death,sizeof(death));
		}
	}

	if (bret)
	{
		charbase.hp = (WORD)(charstate.maxhp*percent/100.0f);
		charbase.mp = (WORD)(charstate.maxmp*percent/100.0f);
		charbase.sp = (WORD)(charstate.maxsp*percent/100.0f);
		showCurrentEffect(Cmd::USTATE_DEATH,false);

		if (relive_type == Cmd::ReliveMoney)
		{
			this->charbase.reliveWeakTime = (SceneTimeTick::currentTime.sec() + charbase.level * 3)%10000;
			// ����Ԥ������,��������
			this->setupCharBase();

			Cmd::stMainUserDataUserCmd  userinfo;
			full_t_MainUserData(userinfo.data);
			sendCmdToMe(&userinfo,sizeof(userinfo));

			showCurrentEffect(Cmd::USTATE_RELIVEWEAK,true);
			this->sendtoSelectedReliveWeakState();
			this->sendtoSelectedTrainState();
			//this->save(Cmd::Record::OPERATION_WRITEBACK);
		}

		if (deathWaitTime == 0)
		{
			reliveReady(NULL,isOrigin);
		}
		if (relive_type == Cmd::ReliveMoney)
		{
			if (guard) guard->masterIsAlive = true;
		}
	}

	lastKiller = 0;//�����ɱ��¼
#ifdef _DEBUG
	Zebra::logger->debug("%s �����ɱ��¼",name);
#endif
	return bret;
}
