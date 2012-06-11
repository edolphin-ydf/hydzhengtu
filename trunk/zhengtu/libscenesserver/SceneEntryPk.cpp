/**
* \brief PK��ر������鷽��
*/

#include <zebra/ScenesServer.h>

/**
* \brief ��ɫPK������,�д�����
*/
bool SceneEntryPk::AttackMe(SceneEntryPk *pAtt,const Cmd::stAttackMagicUserCmd *rev,bool physics,SWORD rangDamageBonus)
{
	//sky ���״̬�����Ա�����
	if(Soulflag)
		return false;

	//�����´��Զ��ָ�hp������Ҫ��ʱ��
	pAtt->lastPkTime = 10;

	//if (rev->wdMagicType == SKILLNORMAL) this->skillValue.brappenddam = 0; //�ǹ�����������ӡ�����ṩ�Ķ����˺���0

	if (pAtt->getType() == zSceneEntry::SceneEntry_Player &&
		this->getType() == zSceneEntry::SceneEntry_Player)
	{
		pAtt->processMaskOnAttack(this);
		this->processMaskOnDefence();
	}


	if (angelMode)  return true;

	if (pAtt->afreetMode)
	{
		toDie(pAtt->tempid);
		return true;
	}

	
	//��ȫ��ס����
	if (this->ignoreDam)
	{
		return true;
	}

	//���㼼���˺�
	//sky �����ħ���Ƿ��ܱ�����
	this->reflectSkill(pAtt,rev);

	//sky �������������������״̬
	if (this->ignorePhysicDam)
	{
		//sky ��⹥���ļ�������ͨ����
		if(	rev->wdMagicType == SERVER_SKILL_ATTACK_NORMAL ||
			rev->wdMagicType == SERVER_SKILL_DAGGER_ATTACK_NORMAL ||
			rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL ||
			rev->wdMagicType == SERVER_SKILL_HANDS_ATTACK_NORMAL)
			return true; //sky ֱ�����ߵ�
		else
		{
			//sky �����ȡ���ܶ������Ƿ�������ϵ����
			zSkill * skill = NULL;
			skill = zSkill::createTempSkill(this,rev->wdMagicType,1);
			if(skill && skill->IsPhysicsSkill())
				return true; //sky �ǵĻ�Ҳֱ�����ߵ�
		}
	}
	else if (this->ignoreMagicDam) //sky ���ϲ������ڼ��ħ�����ߵ����
	{
		if(	!(rev->wdMagicType == SERVER_SKILL_ATTACK_NORMAL) &&
			!(rev->wdMagicType == SERVER_SKILL_DAGGER_ATTACK_NORMAL) &&
			!(rev->wdMagicType == SERVER_SKILL_DART_ATTACK_NORMAL) &&
			!(rev->wdMagicType == SERVER_SKILL_HANDS_ATTACK_NORMAL) )
		{
			//sky �����ȡ���ܶ������Ƿ���ħ��ϵ����
			zSkill * skill = NULL;
			skill = zSkill::createTempSkill(this,rev->wdMagicType,1);
			if(skill && skill->IsMagicSkill())
				return true; //sky �ǵĻ�Ҳֱ�����ߵ�
		}
		return true;
	}

	int dwDam;

	if (physics)
	{
		//sky ��ȡ���������ʵ���˺�
		dwDam = (int)pAtt->pkValue.pdamage  - (this->pdeftozero?0:this->pkValue.pdefence);
		WORD EquipPdamage = 0;

		if( pAtt->getType() == SceneEntry_Player )
			EquipPdamage = ((SceneUser*)pAtt)->packs.equip.getEquips().get_maxpdamage();

		//sky ������������������ļӳ�
		if (pAtt->Daggerflag) // ���ذ���˺�
		{
			if(pAtt->skillValue.DaggerAddDam > 0)
				//sky ��������ֵ�����������˺��İٷֱ���
				dwDam += (int)(EquipPdamage*(pAtt->skillValue.DaggerAddDam/100.0f));

			if(pAtt->skillValue.DaggerAddDamA > 0)
				//sky �����̶�ֵ
				dwDam += pAtt->skillValue.DaggerAddDamA;
		}
		else if (pAtt->Throflag) // ���Ͷ�������˺�
		{
			if(pAtt->skillValue.ThroAddDam > 0)
				//sky ��������ֵ�����������˺��İٷֱ���
				dwDam += (int)(EquipPdamage*(pAtt->skillValue.ThroAddDam/100.0f));

			if(pAtt->skillValue.ThroAddDamA > 0)
				//sky �����̶�ֵ
				dwDam += pAtt->skillValue.ThroAddDamA;
		}
		else if (pAtt->Handsflag) // ���˫�������˺�
		{
			if(pAtt->skillValue.HandsAddDam > 0)
				//sky ��������ֵ�����������˺��İٷֱ���
				dwDam += (int)(EquipPdamage*(pAtt->skillValue.HandsAddDam/100.0f));

			if(pAtt->skillValue.HandsAddDamA > 0)
				//sky �����̶�ֵ
				dwDam += pAtt->skillValue.HandsAddDamA;
		}
		else if (pAtt->Handflag) // ��ߵ��������˺�
		{
			if(pAtt->skillValue.HandAddDam > 0)
				//sky ��������ֵ�����������˺��İٷֱ���
				dwDam += (int)(EquipPdamage*(pAtt->skillValue.HandAddDam/100.0f));

			if(pAtt->skillValue.HandAddDamA > 0)
				//sky �����̶�ֵ
				dwDam += pAtt->skillValue.HandAddDamA;
		}
#ifdef _DEBUG
		Zebra::logger->debug("������[%s]---������[%s]----------------------------------------------------------",pAtt->name,this->name);
		Zebra::logger->debug("��������ֵ  =%d",(int)pAtt->pkValue.pdamage);
		Zebra::logger->debug("���������ֵ  =%d",(this->pdeftozero?0:this->pkValue.pdefence));
		Zebra::logger->debug("�����˺�ֵΪdwDam=%d",dwDam);
#endif
	}
	else
	{
		dwDam = (int)pAtt->pkValue.mdamage  - (this->mdeftozero?0:this->pkValue.mdefence);
#ifdef _DEBUG
		Zebra::logger->debug("������[%s]---������[%s]----------------------------------------------------------",pAtt->name,this->name);
		Zebra::logger->debug("����������ֵ  =%d",(int)pAtt->pkValue.mdamage);
		Zebra::logger->debug("����������ֵ  =%d",(this->mdeftozero?0:this->pkValue.mdefence));
		Zebra::logger->debug("�����˺�ֵΪ%d",dwDam);
#endif
	}
	// dwDamSelf ��ʾ�������˺�  dwDamDef ��ʾ
	int dwDamSelf = 0,dwDamDef = 0,dwReduce = dwDam>=0?0:dwDam;
	if (dwDam<0) dwDam = 0;

	// ���������˺�ֵ
	// �Է����ߵ��˺� = �˺�ֵ
	dwDamDef += this->skillValue.dvalue;
#ifdef _DEBUG
	Zebra::logger->debug("���ݼ��������˺�ֵ��������Ľ���ۼ�ֵdwDamDef:%ld",dwDamDef);
#endif
	dwDamDef += (int)(dwDam * ((this->skillValue.dvaluep>100?this->skillValue.dvaluep-100:0)/100.0f));
#ifdef _DEBUG
	Zebra::logger->debug("���ݼ��������˺��ʼ�������Ľ���ۼ�ֵdwDamDef:%ld",dwDamDef);
#endif

	pAtt->processAddDam(dwDam,dwDamDef,physics);
	this->reduceDam(dwDam,dwDamDef,physics);

	dwDamDef +=dwDam+dwReduce; // 
#ifdef _DEBUG
	Zebra::logger->debug("�м�ֵdwDamDef:%d",dwDamDef);
#endif

	if (dwDamDef <0)
	{
		dwDamDef = 0;
	}

	//������ܵ�ֱ���˺�
	WORD temp = pAtt->getDamageBonus();
	if (temp>0)
	{
		dwDamDef = (int)(dwDamDef*((100-temp)/100.0f));
	}

	if (pAtt->pkValue.damagebonus>0)
	{
		dwDamDef = (int)(dwDamDef*((100-pAtt->pkValue.damagebonus)/100.0f));
	}

	if (rangDamageBonus>0)
	{
		dwDamDef = (int)(dwDamDef*((100-rangDamageBonus)/100.0f));
	}

	//���Ϲ����ߵı����˺�
	dwDamDef +=pAtt->skillValue.passdam;

	if (dwDamDef >0)
	{
		//�����˺����ת��----------------------------
		if (skillValue.tsfdamp != 0)
		{
			DamagCallback callback(this);
			scene->execAllOfScreen(posi,callback);
			SWORD temp=0;
			if ((temp=callback.action((SDWORD)(this->skillValue.tsfdam + dwDamDef*(this->skillValue.tsfdamp/100.0f))))!=0)
			{
				dwDamDef -= temp;
			}
#ifdef _DEBUG
			Zebra::logger->debug("�������ת���˺��ʿ۳��������ֵdwDamDef:%d",dwDamDef);
#endif
		}
		//--------------------------------------------
		this->hp2mp(dwDamDef);
		if (this->dmpbyhp >0)
		{
			if (this->getType()==zSceneEntry::SceneEntry_Player)
			{
				SDWORD dmp= (SDWORD)(dwDamDef*(this->dmpbyhp/100.0f));
				this->changeMP(0-dmp);
				pAtt->changeMP(dmp);
			}
		}
		if (!physics&&(this->mdmtomp >0)&&(selectByPercent(this->mdmtompper)))
		{
			if (this->getType()==zSceneEntry::SceneEntry_Player)
			{
				SDWORD dmp= (SDWORD)(dwDamDef*(this->mdmtomp/100.0f));
				this->changeMP(dmp);
			}
		}
	}

	if (zSceneEntry::SceneEntry_NPC == this->getTopMaster()->getType())
	{
		if (zSceneEntry::SceneEntry_Player == pAtt->getTopMaster()->getType())
		{
			int attLevel = (int)pAtt->getTopMaster()->getLevel();
			int defLevel = (int)this->getLevel();
			int num = defLevel - attLevel -15;
			if (num >5) num=5;
			if (num >0)
			{
				dwDamDef = (DWORD)(dwDamDef*(1-num/5.0f));
			}
		}
	}

	if (zSceneEntry::SceneEntry_NPC == this->getType())
	{
		dwDamDef -=((SceneNpc *)this)->dwReduceDam;
		if (dwDamDef<0) dwDamDef=0;
	}

	if (this->skillValue.teamappend)
	{
		if (pAtt->getType() == zSceneEntry::SceneEntry_Player)
		{
			SceneUser *pUser = (SceneUser *)pAtt;
			TeamManager * team = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);

			if (team->getLeader() == (DWORD)this->skillValue.teamappend)
			{
				dwDamDef+=30;
			}
		}
	}

	if (dwDamDef > 0)
	{
		pAtt->leech(dwDamDef);
	}
#ifdef _DEBUG
	Zebra::logger->debug("����ֵdwDamDef:%ld",dwDamDef);
#endif  

	if (pAtt->getType() == zSceneEntry::SceneEntry_Player && this->getType() == zSceneEntry::SceneEntry_Player)
	{
		dwDamDef += randBetween(0,pAtt->getLevel()/2);
	}

	if (pAtt->Flameflag) // ��߻���ϵ�����˺�
	{
		if(pAtt->skillValue.FlameAddDam > 0)
			dwDamDef = (int)(dwDamDef*(1+pAtt->skillValue.FlameAddDam/100.0f));

		if(pAtt->skillValue.FlameAddDamA > 0)
			dwDamDef += pAtt->skillValue.FlameAddDamA;
	}
	else if (pAtt->Iceflag ) // ��߱�˪ϵ�����˺�
	{
		if(pAtt->skillValue.IceAddDam > 0)
			dwDamDef = (int)(dwDamDef*(1+pAtt->skillValue.IceAddDam/100.0f));

		if(pAtt->skillValue.IceAddDamA > 0)
			dwDamDef += pAtt->skillValue.IceAddDamA;
	}
	else if (pAtt->Sacredflag) // �����ʥϵ�����˺�
	{
		if(pAtt->skillValue.SacredAddDam > 0)
			dwDamDef = (int)(dwDamDef*(1+pAtt->skillValue.SacredAddDam/100.0f));

		if(pAtt->skillValue.SacredAddDamA > 0)
			dwDamDef += pAtt->skillValue.SacredAddDamA;
	}
	else if (pAtt->Darkflag) // ��߰�Ӱϵ�����˺�
	{
		if(pAtt->skillValue.DarkAddDam > 0)
			dwDamDef = (int)(dwDamDef*(1+pAtt->skillValue.DarkAddDam/100.0f));

		if(pAtt->skillValue.DarkAddDamA > 0)
			dwDamDef += pAtt->skillValue.DarkAddDamA;
	}

	if (this->mhpd)
	{
		if (physics)
		{
			dwDamDef = dwDamDef*2;
		}
		else
		{
			dwDamDef = dwDamDef/2;
		}
	}
	if (dwDamDef ==0)
	{
		dwDamDef = randBetween(0,5); 
	}

	if (stone&&(dwDamDef>1)) dwDamDef=1;

	//sky ������˺�����ٷֱȵ�״̬
	if(this->skillValue.AttAddDam > 0)
		dwDamDef = dwDamDef*(1+this->skillValue.AttAddDam/100.0f);

	dwDamDef += this->skillValue.appenddam + this->skillValue.brappenddam;

	this->reflectDam(dwDamDef,dwDamSelf,rev->wdMagicType);


#ifdef _DEBUG
	Zebra::logger->debug("���ܼӶ����˺�appenddam:%d ����ӡ�Ƕ����˺�%d",this->skillValue.appenddam,this->skillValue.brappenddam);
#endif

	SWORD wdHP=0;
	SceneUser *pUser= NULL;
	if (this->swapdamcharid>0 || this->swapdamcharid2>0)
	{
		pUser = SceneUserManager::getMe().getUserByTempID(this->swapdamcharid);
		if (pUser&&(pUser->scene==this->scene)&&(pUser->getState()!=zSceneEntry::SceneEntry_Death))
		{
			wdHP = pUser->directDamage(pAtt,(int)(dwDamDef*1.2f));
			ScenePk::attackRTCmdToNine(rev,pAtt,pUser,wdHP,0);
			pUser->attackRTHpAndMp();
		}
		else
		{
			if (this->swapdamcharid>0) this->skillStatusM.clearRecoveryElement(222);
			pUser = SceneUserManager::getMe().getUserByTempID(this->swapdamcharid2);
			if (pUser&&(pUser->scene==this->scene)&&(pUser->getState()!=zSceneEntry::SceneEntry_Death))
			{
				wdHP = pUser->directDamage(pAtt,(int)(dwDamDef));
				ScenePk::attackRTCmdToNine(rev,pAtt,pUser,wdHP,0);
				pUser->attackRTHpAndMp();
			}
			else
			{
				if (this->swapdamcharid2>0) this->skillStatusM.clearRecoveryElement(270);
			}
		}
	}

	if (!pUser)
	{
		if (this->liquidState)
		{
			this->skillStatusM.clearRecoveryElement(248);  ///���Һ̬״̬��
		}
		else if (this->resist==0 || !selectByPercent(this->resist))
		{
			wdHP = this->directDamage(pAtt,dwDamDef);
			ScenePk::attackRTCmdToNine(rev,pAtt,this,wdHP,0);
			this->attackRTHpAndMp();
		}
	}

	if (dwDamSelf > 0)
	{
		SceneEntryPk * aMaster = pAtt->getTopMaster();

		if (aMaster->getType()==zSceneEntry::SceneEntry_Player)
		{
			wdHP = pAtt->directDamage(pAtt,dwDamSelf);
			ScenePk::attackRTCmdToNine(rev,this,pAtt,wdHP,0);

			pAtt->attackRTHpAndMp();
			pAtt->processDeath(pAtt);
		}
		else
		{
			wdHP = pAtt->directDamage(this,dwDamSelf);
			ScenePk::attackRTCmdToNine(rev,this,pAtt,wdHP,0);
			pAtt->attackRTHpAndMp();
			pAtt->processDeath(this);
		}
	}

	SceneEntryPk * aMaster = pAtt->getTopMaster();
	SceneEntryPk * pMaster = getTopMaster();
	SDWORD pkadditionbackup = 0;
	//sky ֻ����Ҳ���ɱ��NPC�𣡣���������
	/*if (aMaster->getType()==zSceneEntry::SceneEntry_Player)
	{*/
	if (aMaster->getType()==zSceneEntry::SceneEntry_Player && 
		pMaster->getType()==zSceneEntry::SceneEntry_Player)
	{
		SceneUser * a = (SceneUser *)aMaster;
		SceneUser * p = (SceneUser *)pMaster;
		pkadditionbackup = a->charbase.goodness&0X0000FFFF;
		ScenePk::checkProtect(a,p);
	}

	//sky ���˺������Ƴ�ʧ��Ч��
	if(this->blind)
	{
		this->skillStatusM.clearSkill(SKILLID_LIME);
	}

	//sky ���˺������Ƴ����Ч��
	if(this->repentange)
	{
		this->skillStatusM.clearSkill(SKILLID_REPENTANCE);
	}

	//sky ���˺�50%�Ƴ��־弼��������Ч��
	if(this->dread)
	{
		if(selectByPercent(50))
			this->skillStatusM.clearSkill(SKILLID_FEAR);
	}

	//sky ���˺�25%�Ƴ���������������Ч��
	if(this->frozen)
	{
		if(selectByPercent(25))
			this->skillStatusM.clearSkill(SKILLID_FROZEN);
	}

	if (this->hideme)
	{
		this->skillStatusM.clearRecoveryElement(241);  ///�������״̬��
	}
	if (this->icebox)
	{
		this->skillStatusM.clearRecoveryElement(245);  ///�����״̬��
	}

	if ((pAtt->getState() != zSceneEntry::SceneEntry_Death)&&(pAtt->dietodam >0))
	{
		wdHP = this->directDamage(pAtt,pAtt->dietodam);
		ScenePk::attackRTCmdToNine(rev,pAtt,this,wdHP,0);
		this->attackRTHpAndMp();
		pAtt->toDie(0);
	}

	if ((this->getState() != zSceneEntry::SceneEntry_Death)&&(pAtt->killnow >0))
	{
		if (selectByPercent(pAtt->killnow))
		{
			this->toDie(pAtt->tempid);
			pAtt->skillStatusM.clearSkill(234);
		}
	}

	if (pAtt->damtohpper >0)
	{
		if (selectByPercent(pAtt->damtohpper))
		{
			DWORD value = (DWORD)(dwDamDef *(pAtt->damtohpvalper/100.0f));
			pAtt->changeHP(value);
		}
	}

	SceneEntryPk *myMaster =getMaster();

	if (!this->processDeath(pAtt))
	{
		/*    pAtt->attackTarget = this;
		///������,���ö���
		*
		if (0==curTargetID)
		{
		//Zebra::logger->debug("%s �� %s ����,����ս��",name,pAtt->name);
		curTargetID = pAtt->tempid;
		curTargetType = pAtt->getType();
		}
		*
		pAtt->setCurTarget(this);
		SceneEntryPk * dt = getDefTarget();
		if (!dt || dt->getState()!=zSceneEntry::SceneEntry_Normal
		|| scene->getDistance(getPos(),pAtt->getPos())<scene->getDistance(getPos(),dt->getPos()))
		{
		if (!(pAtt->getType()==zSceneEntry::SceneEntry_NPC &&(((SceneNpc *)pAtt)->npc->kind == NPC_TYPE_SURFACE)))
		{
		setDefTarget(pAtt);
		if (getType()==zSceneEntry::SceneEntry_NPC)
		setCurTarget(pAtt);
		}
		}
		setEndBattleTime(SceneTimeTick::currentTime,10*1000);
		*/
	}
	else
	{
		SceneEntryPk *attMaster = pAtt->getMaster();
		if (attMaster&&myMaster&&(attMaster->getType()==zSceneEntry::SceneEntry_Player)&&(myMaster->getType()==zSceneEntry::SceneEntry_Player))
		{
			SceneUser *pAttUser = (SceneUser *)attMaster;
			SceneUser *pDefUser = (SceneUser *)myMaster;

			if ((pkadditionbackup < (SDWORD)(pAttUser->charbase.goodness&0X0000FFFF)) ||
				((pAttUser->charbase.country != pDefUser->charbase.country)&&(!pAttUser->isWar(pDefUser))))
			{
				Cmd::Session::t_addRelationEnemy send;
				send.dwUserID = pDefUser->id;
				strncpy(send.name,pAttUser->name,MAX_NAMESIZE);
				sessionClient->sendCmd(&send,sizeof(send));
			}

			DWORD sceneCountry = attMaster->scene->getCountryID();
			if (pAttUser->charbase.country != pDefUser->charbase.country)
			{
				//char buf[MAX_CHATINFO];

				if (pAttUser->charbase.country == sceneCountry)
				{
					if (!pAttUser->isWarRecord(Cmd::COUNTRY_FORMAL_DARE,pDefUser->charbase.country) && 
						!pAttUser->isSpecWar(Cmd::COUNTRY_FORMAL_DARE))
					{
						if (this->getType() == zSceneEntry::SceneEntry_Player)
						{
							Cmd::stKillForeignerUserCmd send;
							strncpy(send.heroName,pAttUser->name,MAX_NAMESIZE);
							strncpy(send.mapName,pAttUser->scene->getName(),MAX_NAMESIZE);
							pAttUser->forwardSession(&send,sizeof(send));
						}
					}
				}
				if (pDefUser->charbase.country == sceneCountry)
				{
					if (!pAttUser->isWarRecord(Cmd::COUNTRY_FORMAL_DARE,pDefUser->charbase.country) &&
						!pDefUser->isSpecWar(Cmd::COUNTRY_FORMAL_DARE))
					{
						if (this->getType() == zSceneEntry::SceneEntry_Player)
						{
							Cmd::stRequestCountryHelpUserCmd send;
							strncpy(send.mapName,pAttUser->scene->getName(),sizeof(send.mapName));
							send.x = pAttUser->getPos().x;
							send.y = pAttUser->getPos().y;
							pDefUser->forwardSession(&send,sizeof(send));
						}
					}
				}
			}
		}
		else if (attMaster&&(attMaster->getType()==zSceneEntry::SceneEntry_Player)&&(this->getType()==zSceneEntry::SceneEntry_NPC))
		{
			SceneNpc *pNpc = (SceneNpc *)this;
			if (pNpc->npc->kind == NPC_TYPE_BBOSS)
			{
				SceneUser *pAttUser = (SceneUser *)attMaster;
				if (pNpc->dwNpcLockedUser == pAttUser->id)
				{
					Cmd::stKillBossUserCmd send;
					strncpy(send.mapName,this->scene->getName(),sizeof(send.mapName));
					send.x = this->getPos().x;
					send.y = this->getPos().y;
					send.npcid = pNpc->npc->id;
					strncpy(send.name,pAttUser->name,sizeof(send.name));
					pAttUser->forwardSession(&send,sizeof(send));
				}
			}
		}

		if (this->getType() == zSceneEntry::SceneEntry_NPC)
		{
			SceneNpc *pNpc = (SceneNpc *)this;
			if (pNpc->getPetType() == Cmd::PET_TYPE_SUMMON)
			{
				if (myMaster&&selectByPercent(this->skillValue.summonrelive)) myMaster->summonPet(this->id,Cmd::PET_TYPE_SUMMON);
			}
		}
	}
	/*}*/
	return true;
}

/**
* \brief ���ħ������·���Ƿ����赲
*
*
* \param pTarget ����Ŀ��
* \param ��������
* \return �Ƿ��˳������
*/
bool SceneEntryPk::checkMagicFlyRoute(zSceneEntry *pTarget,BYTE aType)
{
	switch(aType)
	{
	case 0: // ����
		{
			if (abs((long)(this->pos.x - pTarget->getPos().x)) > 1 || abs((long)(this->pos.y - pTarget->getPos().y)) > 1)
			{
#ifdef  _DEBUG 
				Zebra::logger->debug("����������Χ(%s(%ld) x���=%u y���=%u)",this->name,this->id,abs((long)(this->pos.x - pTarget->getPos().x)),abs((long)(this->pos.y - pTarget->getPos().y)));
				Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��Խ��ʽ������");
#endif
				if (abs((long)(this->pos.x - pTarget->getOldPos1().x)) > 1 || abs((long)(this->pos.y - pTarget->getOldPos1().y)) > 1)
				{
#ifdef  _DEBUG 
					Zebra::logger->debug("����������Χ(%s(%ld) oldx���=%u oldy���=%u)",this->name,this->id,abs((long)(this->pos.x - pTarget->getOldPos1().x)),abs((long)(this->pos.y - pTarget->getOldPos1().y)));
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��Խ��ʽ������");
#endif
					if (abs((long)(this->pos.x - pTarget->getOldPos2().x)) > 1 || abs((long)(this->pos.y - pTarget->getOldPos2().y)) > 1)
					{
#ifdef  _DEBUG 
						Zebra::logger->debug("����������Χ(%s(%ld) oldx���=%u oldy���=%u)",this->name,this->id,abs((long)(this->pos.x - pTarget->getOldPos2().x)),abs((long)(this->pos.y - pTarget->getOldPos2().y)));
						Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��Խ��ʽ������");
#endif
						return false;
					}
				}
			}
		}
		break;
	case 1: // ����
		{
#ifdef  _DEBUG 
			Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��Խ��ʽ������");
#endif

			zPos pos1 = this->pos;
			zPos pos2 = pTarget->getPos();

			if (pos1 == pos2) return true;

			float ftA=0.0f;
			float ftB=0.0f;
			if (pos1.x == pos2.x)
			{
				// TILE_MAGIC_BLOCK �赲������
				// 
				if (pos1.y> pos2.y)
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ1����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif
					zPos pos;
					pos.x = pos1.x;
					for(DWORD i = pos2.y+1; i<pos1.y;i++)
					{
						pos.y = i;
#ifdef _DEBUG
						Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��������㣨x=%u,y=%u) %s",pos.x,pos.y,this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)?"���赲":"���赲");
#endif
						if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}
				else
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ2����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif
					zPos pos;
					pos.x = pos1.x;
					for(DWORD i = pos1.y+1; i<pos2.y;i++)
					{
						pos.y = i;
#ifdef _DEBUG
						Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��������㣨x=%u,y=%u) %s",pos.x,pos.y,this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)?"���赲":"���赲");
#endif
						if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}

			}
			else if (pos1.y == pos2.y)
			{
				if (pos1.x> pos2.x)
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ3����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif
					zPos pos;
					pos.y = pos1.y;
					for(DWORD i = pos2.x+1; i<pos1.x;i++)
					{
						pos.x = i;
#ifdef _DEBUG
						Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��������㣨x=%u,y=%u) %s",pos.x,pos.y,this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)?"���赲":"���赲");
#endif
						if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}
				else
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ4����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif
					zPos pos;
					pos.y = pos1.y;
					for(DWORD i = pos1.x+1; i<pos2.x;i++)
					{
						pos.x = i;
#ifdef _DEBUG
						Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��������㣨x=%u,y=%u) %s",pos.x,pos.y,this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)?"���赲":"���赲");
#endif
						if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}
			}
			else
			{
				std::set<zPos> posSet;
				typedef std::set<zPos>::value_type ValueType;
				std::set<zPos>::iterator vIterator;
				if (pos1.x> pos2.x)
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ5����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif 
					ftA =(float)((float)pos1.y-(float)pos2.y)/(float)((float)pos1.x-(float)pos2.x);
					ftB = (float)pos1.y - (float)ftA*(float)pos1.x;

					zPos pos;
					for(DWORD i = pos2.x+1; i<pos1.x;i++)
					{
						pos.x = i;
						pos.y = (DWORD)(ftA*pos.x +ftB);
						posSet.insert(ValueType(pos));
						//if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}
				else
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ6����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif 
					ftA =(float)((float)pos1.y-(float)pos2.y)/(float)((float)pos1.x-(float)pos2.x);
					ftB = (float)pos1.y - (float)(ftA*(float)pos1.x);

					zPos pos;
					for(DWORD i = pos1.x+1; i<pos2.x;i++)
					{
						pos.x = i;
						pos.y = (DWORD)(ftA*pos.x +ftB);
						posSet.insert(ValueType(pos));
						//if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}
				if (pos1.y> pos2.y)
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ7����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif 
					ftA =(float)((float)pos1.y-(float)pos2.y)/(float)((float)pos1.x-(float)pos2.x);
					ftB = (float)pos1.y - (float)ftA*(float)pos1.x;

					zPos pos;
					for(DWORD i = pos2.y+1; i<pos1.y;i++)
					{
						pos.y = i;
						pos.x = (DWORD)(((float)pos.y - (float)ftB)/(float)ftA);
						posSet.insert(ValueType(pos));
						//if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}
				else
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��ʽ8����x=%u,y=%u)��x=%u,y=%u)",pos1.x,pos1.y,pos2.x,pos2.y);
#endif 
					ftA =(float)((float)pos1.y-(float)pos2.y)/(float)((float)pos1.x-(float)pos2.x);
					ftB = (float)pos1.y - (float)(ftA*(float)pos1.x);

					zPos pos;
					for(DWORD i = pos1.y+1; i<pos2.y;i++)
					{
						pos.y = i;
						pos.x = (DWORD)(((float)pos.y - (float)ftB)/(float)ftA);
						posSet.insert(ValueType(pos));
						//if (this->scene->checkBlock(pos,TILE_MAGIC_BLOCK)) return false;
					}
				}

				for(vIterator = posSet.begin(); vIterator != posSet.end(); vIterator++)
				{
#ifdef _DEBUG
					Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��������㣨x=%u,y=%u) %s",vIterator->x,vIterator->y,this->scene->checkBlock(*vIterator,TILE_MAGIC_BLOCK)?"���赲":"���赲");
#endif
					if (this->scene->checkBlock(*vIterator,TILE_MAGIC_BLOCK)) return false;
				}
			}
		}
		break;
	case 2: // ֱ��
		{
#ifdef  _DEBUG 
			Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��Խ��ʽ��ֱ��");
#endif
		}
		break;
	case 3: //������ֱ��
		{
			return true;
		}
	default:
		{
			Zebra::logger->debug("(%s,%ld)ȱ�ٹ�������",this->name,this->tempid);
#ifdef  _DEBUG 
			Channel::sendSys(tempid,Cmd::INFO_TYPE_GAME,"��Խ��ʽ����Ч����");
#endif
			return false;
		}
		break;
	}
	//����һ��
	if (!this->scene->zPosShortRange(this->getPos(),pTarget->getPos(),11,11))//SCREEN_WIDTH,SCREEN_HEIGHT))
	{
		return false;
	}
	return true;
}

/**
* \brief ���npc������Ƿ����
*
* \return �Ƿ����
bool SceneEntryPk::isRedNamed()
{
switch (getType())
{
case zSceneEntry::SceneEntry_Player:
{
return ((SceneUser *)this)->isRedNamed();
}
case zSceneEntry::SceneEntry_NPC:
{
SceneEntryPk * m = ((SceneNpc *)this)->getMaster();
if (m)
return m->isRedNamed();
else
return false;
}
default:
return false;
}
return false;
}
*/

/**
* \brief �õ���ǰ��hp
*
* \return ��ǰ��hp
*/
DWORD SceneEntryPk::getHp()
{
	switch (getType())
	{
	case zSceneEntry::SceneEntry_Player:
		return ((SceneUser *)this)->charbase.hp;
		break;          
	case zSceneEntry::SceneEntry_NPC:
		return ((SceneNpc *)this)->hp;
		break;
	default:
		return 0;
	}               
}

/**
* \brief �õ����hp
*
* \return ����hp
*/
DWORD SceneEntryPk::getMaxHp()
{
	switch (getType())
	{
	case zSceneEntry::SceneEntry_Player:
		return ((SceneUser *)this)->charstate.maxhp;
		break;          
	case zSceneEntry::SceneEntry_NPC:
		return ((SceneNpc *)this)->getMaxHP();
		break;
	default:
		return 0;
	}               
}

/**
* \brief ����Ƿ���ս��״̬
*
* \return �Ƿ���ս��״̬
*/
bool SceneEntryPk::isFighting()
{
	return curTargetID!=0;
	return !checkEndBattleTime(SceneTimeTick::currentTime);
	switch (getType())
	{
	case zSceneEntry::SceneEntry_Player:
		return !((SceneUser *)this)->checkEndBattleTime(SceneTimeTick::currentTime);
		break;          
	case zSceneEntry::SceneEntry_NPC:
		return getCurTarget()!=0;
		break;
	default:
		return 0;
	}      
}

/**
* \brief ��������ս����ʱ��
*
* \param ct ��ʼ��ʱ��ʱ��
* \param delay ������ʱ���ӳ�
* \return 
*/
void SceneEntryPk::setEndBattleTime(const zRTime &ct,int delay)
{
	endBattleTime = ct;
	endBattleTime.addDelay(delay);
}

/**
* \brief ����Ƿ��ѵ�����ս����ʱ��
*
*
* \param ct ���ڱȽϵ�ʱ��
* \return �Ƿ�ʱ��
*/
bool SceneEntryPk::checkEndBattleTime(const zRTime &ct)
{
	return ct>=endBattleTime;
}

/**
* \brief ����ս��
*
*/
void SceneEntryPk::leaveBattle()
{
	for (std::list<ScenePet*>::iterator it=totems.begin(); it!=totems.end(); it++)
	{
		if (getCurTarget()==(*it)->getCurTarget())
			(*it)->leaveBattle();
	}

	if (pet && pet->getCurTarget()==getCurTarget())
		pet->leaveBattle();

	if (summon && summon->getCurTarget()==getCurTarget())
		summon->leaveBattle();

	curTargetID = 0;
	curTargetType = 0;
	clearDefTarget();
	setEndBattleTime(SceneTimeTick::currentTime,0);
#ifdef _DEBUG
	//Zebra::logger->debug("%s ����ս��",name);
#endif
}

/**
* \brief ��������
*
*/
bool SceneEntryPk::processDeath(SceneEntryPk *pAtt)
{
	leaveBattle();
	return true;
}

/**
* \brief ��������ս���Ķ���
*
* \return ����ָ��
*/
SceneEntryPk * SceneEntryPk::getCurTarget()
{       
	switch (curTargetType)
	{
	case zSceneEntry::SceneEntry_Player:
		return scene->getUserByTempID(curTargetID);
	case zSceneEntry::SceneEntry_NPC:
		return SceneNpcManager::getMe().getNpcByTempID(curTargetID);
	default:
		return 0;
	}      
}

/**
* \brief �������ڹ����Լ��Ķ���
*
* \return ����ָ��
*/
SceneEntryPk * SceneEntryPk::getDefTarget()
{       
	switch (defTargetType)
	{
	case zSceneEntry::SceneEntry_Player:
		return scene->getUserByTempID(defTargetID);
	case zSceneEntry::SceneEntry_NPC:
		return SceneNpcManager::getMe().getNpcByTempID(defTargetID);
	default:
		return 0;
	}      
}

/**
* \brief ���õ�ǰ��Ŀ��
*
*
* \param tempid ���ֵ�tempid
* \param type ���ֵ�����
* \param force ǿ������Ŀ��
* \return 
*/
bool SceneEntryPk::setCurTarget(DWORD tempid,DWORD type,bool force)
{
	if (tempid==this->tempid) return false;//�������Լ�

	SceneEntryPk * def = NULL;
	if (zSceneEntry::SceneEntry_Player==type)
		def = scene->getUserByTempID(tempid);
	else if (zSceneEntry::SceneEntry_NPC==type)
		def = SceneNpcManager::getMe().getNpcByTempID(tempid);
	if (def)
		if (!isEnemy(def)) return false;

	curTargetID = tempid;
	curTargetType = type;
	setEndBattleTime(SceneTimeTick::currentTime,10*1000);
#ifdef _DEBUG
	//Zebra::logger->debug("%s ���ù������� %u",name,tempid);
#endif
	return true;
}


/**
* \brief ���õ�ǰ��Ŀ��
*
*
* \param target ���ֵ�ָ��
* \param force ǿ������Ŀ��
* \return 
*/
bool SceneEntryPk::setCurTarget(SceneEntryPk * target,bool force)
{
	if (target==this) return false;//�������Լ�
	if (!isEnemy(target)) return false;//����������

	curTargetID = target->tempid;
	curTargetType = target->getType();
	setEndBattleTime(SceneTimeTick::currentTime,10*1000);
	//Zebra::logger->debug("%s ���ù������� %s",name,target->name);
	return true;
}

/**
* \brief ���õ�ǰ�ķ���Ŀ��
*
*
* \param tempid ���ֵ�tempid
* \param type ���ֵ�����
* \param force ǿ������Ŀ��
* \return 
*/
bool SceneEntryPk::setDefTarget(DWORD tempid,DWORD type,bool force)
{
	if (tempid==this->tempid) return false;//�������Լ�

	SceneEntryPk * def = NULL;
	if (zSceneEntry::SceneEntry_Player==type)
		def = scene->getUserByTempID(tempid);
	else if (zSceneEntry::SceneEntry_NPC==type)
		def = SceneNpcManager::getMe().getNpcByTempID(tempid);
	if (def)
		if (!isEnemy(def)) return false;

	defTargetID = tempid;
	defTargetType = type;
	setEndBattleTime(SceneTimeTick::currentTime,10*1000);
#ifdef _DEBUG
	//Zebra::logger->debug("%s ���÷������� %u",name,tempid);
#endif
	return true;
}

/**
* \brief ���õ�ǰ�ķ���Ŀ��
*
*
* \param target ���ֵ�ָ��
* \param force ǿ������Ŀ��
* \return 
*/
bool SceneEntryPk::setDefTarget(SceneEntryPk * target,bool force)
{
	if (target==this) return false;//�������Լ�
	if (!isEnemy(target)) return false;//����������

	defTargetID = target->tempid;
	defTargetType = target->getType();
	setEndBattleTime(SceneTimeTick::currentTime,10*1000);
#ifdef _DEBUG
	//Zebra::logger->debug("%s ���÷������� %s",name,target->name);
#endif
	return true;
}

/**
* \brief ȡ����ǰ�ķ���Ŀ��
*/
void SceneEntryPk::clearDefTarget()
{
	defTargetID = 0;
	defTargetType = 0;
#ifdef _DEBUG
	//Zebra::logger->debug("%s �������Ŀ��",name);
#endif
}

/**
* \brief ����������
* ����ԭ���ĳ���
*
* \param up Ҫ�����ĳ���
*/
void SceneEntryPk::petLevelUp(ScenePet * up)
{
	if (Cmd::PET_TYPE_GUARDNPC==up->getPetType()
		|| Cmd::PET_TYPE_RIDE==up->getPetType()) return;

	//������ߵȼ�=���˼��ܵȼ�+2
	while ((up->petData.exp>=up->petData.maxexp)&&(up->petData.lv<up->maxlevel))
	{
		zNpcB *base = npcbm.get(up->id+1);
		if (!base)
		{
			return;
		}

		up->petData.exp -= up->petData.maxexp;
		up->petData.maxexp = base->exp;
		up->petData.lv++;
		up->npc = base;
		up->id++;
		up->petData.id++;
		up->hp = up->npc->hp;
	}

	up->sendMeToNine();
}

/**
* \brief ��9�����ͳ�����Ϣ
*
*/
void SceneEntryPk::sendPetDataToNine()
{
	for (std::list<ScenePet *>::iterator it=totems.begin(); it!=totems.end(); it++)
		(*it)->sendPetDataToNine();
	if (pet) pet->sendPetDataToNine();
	if (summon) summon->sendPetDataToNine();
}

/**
* \brief ���ֱ���˺�
* \param pAtt ������
* \param dam �˺�
* \param notify �Ƿ�֪ͨ�ͻ���
* \return �˺�ֵ
*/
SWORD SceneEntryPk::directDamage(SceneEntryPk *pAtt,const SDWORD &dam,bool notify)
{
	pAtt->attackTarget = this;

	pAtt->setCurTarget(this);
	SceneEntryPk * dt = getDefTarget();
	if (!dt || dt->getState()!=zSceneEntry::SceneEntry_Normal
		|| scene->getDistance(getPos(),pAtt->getPos())<scene->getDistance(getPos(),dt->getPos()))
	{
		if (!(pAtt->getType()==zSceneEntry::SceneEntry_NPC &&(((SceneNpc *)pAtt)->npc->kind == NPC_TYPE_SURFACE)))
		{
			setDefTarget(pAtt);
			if (getType()==zSceneEntry::SceneEntry_NPC)
				setCurTarget(pAtt);
		}
	}
	setEndBattleTime(SceneTimeTick::currentTime,10*1000);

	//sky �˺�����Ҳ�����ϲ������
	SDWORD attHp = dam;
	if (icedam >0)
	{
		//sky �������պ�����˺�
		attHp -= icedam;
		if(attHp < 0)
			attHp = 0;

		icedam -= dam;

		if(icedam < 0)
			icedam = 0; //�ƶ�
	}

	return attHp;
}
void SceneEntryPk::sendCmdToSelected(const void *pstrCmd,const DWORD nCmdLen)
{
	//selected_lock.lock();
	SelectedSet_iterator iter = selected.begin();
	for(; iter != selected.end() ;)
	{
		SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(*iter);
		if (pUser)
		{
			if (this->scene->checkTwoPosIInNine(this->getPosI(),pUser->getPosI()))
			{
				pUser->sendCmdToMe(pstrCmd,nCmdLen);
				iter ++ ;
				continue;
			}
		}
		SelectedSet_iterator iter_del = iter;
		iter_del ++;
		selected.erase(iter);
		iter = iter_del;
	}
	//selected_lock.unlock();
}
