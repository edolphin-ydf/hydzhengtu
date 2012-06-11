﻿/**
 * \brief 定义Npc
 *
 * 
 */
#include <zebra/ScenesServer.h>
#include <algorithm>
#include "duplicateManager.h"

DWORD SceneNpc::serialID = SceneNpc::maxUniqueID;
zUniqueDWORDID SceneNpc::uniqueID(1,SceneNpc::maxUniqueID);
DWORD MapdwNpcNum = 0;

/**
 * \brief 判断改npc是否可以被攻击
 * 返回false则完全不能被攻击
 *
 */
bool SceneNpc::canBeAttack()
{
  if (getPetType()==Cmd::PET_TYPE_RIDE//马匹也不能被攻击
      || getPetType()==Cmd::PET_TYPE_CARTOON)
    return false;

  switch(npc->kind)
  {
    case NPC_TYPE_STORAGE:
    case NPC_TYPE_ROADSIGN:
    case NPC_TYPE_TREASURE:
    case NPC_TYPE_MOBILETRADE:
    case NPC_TYPE_LIVENPC:
    case NPC_TYPE_MAILBOX:
    case NPC_TYPE_AUCTION:
    case NPC_TYPE_SURFACE:
    case NPC_TYPE_CARTOONPET:
      {
        return false;
      }
      break;

    default:
      break;
  }
  return true;
}

/**
 * \brief 判断改npc是否可以被攻击(是否是怪物)
 * 但是其中NPC_TYPE_TRADE NPC_TYPE_TASK 两个类型可以被外国人攻击
 */
bool SceneNpc::isBugbear()
{
  if (getPetType()==Cmd::PET_TYPE_RIDE//马匹也不能被攻击
      || getPetType()==Cmd::PET_TYPE_CARTOON)
    return false;

  switch(npc->kind)
  {
    case NPC_TYPE_TRADE:
    case NPC_TYPE_TASK:
    case NPC_TYPE_STORAGE:
    case NPC_TYPE_ROADSIGN:
    case NPC_TYPE_TREASURE:
    case NPC_TYPE_MOBILETRADE:
    case NPC_TYPE_LIVENPC:
    case NPC_TYPE_MAILBOX:
    case NPC_TYPE_AUCTION:
    case NPC_TYPE_SURFACE:
    case NPC_TYPE_CARTOONPET:
      {
        return false;
      }
      break;

    default:
      break;
  }
  return true;
}
/**
 * \brief Npc死亡
 *
 */
void SceneNpc::death(const zRTime &ct)

}

/**
 * \brief 被击退
 * 不会对场景进行锁操作,保证调用者对场景进行了锁操作
 * \param direct 后退方向
 * \param step 后退步伐
 */
void SceneNpc::backoff(const int direct,const int step)
{
  const int walk_adjust[9][2]= { {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,0} };
  int i = 0;

  zPosI oldPosI = getPosI();
  zPos oldPos = getPos(),newPos = getPos();
  for(i = 1; i <= step; i++)
  {
    newPos.x += walk_adjust[direct][0];
    newPos.y += walk_adjust[direct][1];
    if (scene->checkBlock(newPos))
      break;
  }
  if (i > 1)
  {
    newPos.x -= walk_adjust[direct][0];
    newPos.y -= walk_adjust[direct][1];

    if (scene->refresh(this,newPos))
    {
      scene->setBlock(newPos);
      scene->clearBlock(oldPos);
      setDir(scene->getReverseDirect(direct));

	  if( npc->kind == NPC_TYPE_GHOST )	
	  {
		  Cmd::stBackOffMagicUserCmd cmd;
		  cmd.dwTempID = tempid;
		  cmd.byType = Cmd::MAPDATATYPE_USER;
		  cmd.byDirect = direct;
		  cmd.x = newPos.x;
		  cmd.y = newPos.y;
		  scene->sendCmdToNine(oldPosI,&cmd,sizeof(cmd),this->dupIndex);
	  }
	  else
	  {
		  Cmd::stBackOffMagicUserCmd cmd;
		  cmd.dwTempID = tempid;
		  cmd.byType = Cmd::MAPDATATYPE_NPC;
		  cmd.byDirect = direct;
		  cmd.x = newPos.x;
		  cmd.y = newPos.y;
		  scene->sendCmdToNine(oldPosI,&cmd,sizeof(cmd),this->dupIndex);
	  }

      if (oldPosI != getPosI())
      {
		  if( npc->kind == NPC_TYPE_GHOST )		//sky元神特殊处理
		{
			SceneUser *entry = (SceneUser *)getMaster();
			Cmd::stRemoveUserMapScreenUserCmd remove;
			remove.dwUserTempID = tempid;
			scene->sendCmdToReverseDirect(oldPosI,
				scene->getScreenDirect(oldPosI,getPosI()),&remove,sizeof(remove),this->dupIndex);

			BUFFER_CMD(Cmd::stAddUserAndPosMapScreenStateUserCmd,send,zSocket::MAX_USERDATASIZE);
			((SceneGhost *)this)->full_t_MapUserDataPosState( send->data, entry );
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),send,send->size(),this->dupIndex);
			Cmd::stRTMagicPosUserCmd ret;
			full_stRTMagicPosUserCmd(ret);
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),&ret,sizeof(ret),this->dupIndex);

			attackRTHpAndMp();
		}
		else
		{
			Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
			removeNpc.dwMapNpcDataPosition = tempid;
			scene->sendCmdToReverseDirect(oldPosI,
				scene->getScreenDirect(oldPosI,getPosI()),&removeNpc,sizeof(removeNpc),this->dupIndex);
			Cmd::stAddMapNpcMapScreenUserCmd addNpc;
			full_t_MapNpcData(addNpc.data);
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),
				&addNpc,sizeof(addNpc),this->dupIndex);
			Cmd::stRTMagicPosUserCmd ret;
			full_stRTMagicPosUserCmd(ret);
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),&ret,sizeof(ret),this->dupIndex);
			attackRTHpAndMp();
		  }
      }
    }
  }
  backOffing = 1000;
}

/**
 * \brief 检查经验列表中玩家的攻击是否超时
 * \param pAtt 攻击者
 */
void SceneNpc::refreshExpmapAttackTime(SceneUser* pAtt)
{
}

/**
 * \brief 减少 user的hp
 *
 * \param pAtt 攻击者
 * \param wdHP 减少的hp
 * \return 
 */
void SceneNpc::reduceHP(SceneUser *pAtt,DWORD wdHP)
{
}

/**
 * \brief 队伍分钱的回调
 *
 */
struct MoneyTeamExecExceptMe : public TeamMemExec
{
  ///自己
  SceneUser *me;
  ///钱数
  WORD money;
  ///队伍指针
  TeamManager * team;

  /**
   * \brief 构造函数
   *
   * \param pMe 自己
   * \param u 队长
   * \param mon 钱数
   */
  MoneyTeamExecExceptMe(SceneUser *pMe,TeamManager * temMan,WORD mon)
  {
    me = pMe;
    team = temMan;
    money = mon;
  }
  /**
   * \brief 分钱
   *
   * \param member 队友
   * \return 是否继续回调
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
    if (pUser && pUser != me)
    {
      if (pUser->scene->checkTwoPosIInNine(pUser->getPosI(),me->getPosI()))
      {
        if (pUser->getState() != SceneUser::SceneEntry_Death)
        {
          pUser->packs.addMoney((DWORD)(money),"队伍分钱");
        }
      }
    }
    return true;
  }

};
//银子分配
/**
 * \brief 队伍分钱
 *
 * \param money 总钱数
 */
void SceneNpc::distributeMoney(DWORD money)
{
	SceneUser *pUser = SceneUserManager::getMe().getUserByID(dwNpcLockedUser);
	if (pUser)
	{
		TeamManager * team = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);
		if (team)
		{
			WORD mon = 0;
			WORD myMon = (WORD)(money*0.2f + 0.9f);
			if (team->getExpSize(pUser->getPosI(),this->scene->id))
			{
				mon = (WORD)((money - myMon) / team->getExpSize(pUser->getPosI(),this->scene->id) + 0.9f);
			}
			myMon +=mon;
			pUser->packs.addMoney(myMon,"队伍分钱");

			MoneyTeamExecExceptMe exec(pUser,team,mon);
			team->execEveryOne(exec);
		}
		else
		{
			pUser->packs.addMoney(money,"队伍分钱");
		}
	}
}

struct CountSeptTeamExec : public TeamMemExec
{
  ///家族记数
  int count;
  ///分经验角色的家族ID
  DWORD septid;
  ///杀死的npc
  SceneNpc *npc;

  CountSeptTeamExec(DWORD dwSeptid,SceneNpc *pnpc)
  {
    septid = dwSeptid;
    count=0;
    npc = pnpc;
  }

  /**
   * \brief 同一家族人数统计函数
   *
   *
   * \param member 队友
   * \return 是否继续回调
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
    if (pUser)
    {
      if (pUser->scene->tempid == npc->scene->tempid &&
          pUser->scene->checkTwoPosIInNine(pUser->getPosI(),npc->getPosI()))
      {
        if (pUser->charbase.septid == septid)
        {
          count++;
          if (count >1) return false;
        }
      }
    }
    return true;
  }
};

/**
 * \brief 队伍分经验的回调
 *
 */
struct ExpTeamExec : public TeamMemExec
{
	///sky 队伍指针
	TeamManager * team;
	///杀死的npc
	SceneNpc *npc;
	///经验值
	DWORD exp;
	/// sky 队伍等级差
	DWORD Leveldec;
	/// sky 队伍等级和 
	DWORD LevelSum;

	/**
	* \brief 构造函数
	*
	* \param u 队长
	* \param n 杀死的npc
	* \param wdExp 经验
	*/
	ExpTeamExec(TeamManager * teamMan,SceneNpc *n,DWORD wdExp, DWORD Leveldec, DWORD LevelSum)
	{
		team = teamMan;
		npc = n;
		exp = wdExp;
		Leveldec = Leveldec;
		LevelSum = LevelSum;
	}
	/**
	* \brief 分钱的函数
	*
	*
	* \param member 队友
	* \return 是否继续回调
	*/
	bool exec(TeamMember &member)
	{
		SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
		if (pUser)
		{
			DWORD wdExp = npc->levelExp(exp, pUser->getLevel());

			if(Leveldec <= 20)
				wdExp = (DWORD)(wdExp * ((float)pUser->getLevel() / (float)LevelSum) + (0.1 * pUser->getLevel() * team->getSize()));
			else
				wdExp = (DWORD)(wdExp * ((float)pUser->getLevel() / (float)LevelSum));


			pUser->addExp(wdExp,false,npc->tempid,Cmd::MAPDATATYPE_NPC,true);
			//pUser->sendExpToSept(wdExp);  sky 给家族分经验暂时不需要
		}

		return true;
	}

 };
/**
 * \brief 队伍特殊分经验的回调
 *
 */
struct SpecialExpTeamExec : public TeamMemExec
{
  ///队员
  SceneUser *user;
  ///经验值
  SceneNpc *npc;
  DWORD exp;
  ///家族ID

  /**
   * \brief 构造函数
   *
   * \param u 得到经验的用户
   * \param wdExp 经验
   */
  SpecialExpTeamExec(SceneUser *u,SceneNpc *n,DWORD wdExp)
  {
    user = u;
    npc=n;
    exp = wdExp/10;
    exp = exp?exp:1;
  }
  /**
   * \brief 分钱的函数
   *
   *
   * \param member 队友
   * \return 是否继续回调
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
    if (pUser)
    {
      // 特殊分经验部分不给自己分
      if (pUser == user)
      {
        return true;
      }
      if (pUser->getState() != SceneUser::SceneEntry_Death)
      {
        if (user->scene->id == pUser->scene->id
            && pUser->scene->checkTwoPosIInNine(pUser->getPosI(),user->getPosI()))
        {
          pUser->addExp(exp,false,npc->tempid,Cmd::MAPDATATYPE_NPC);
        }
      }
    }
    return true;
  }

};
/**
 * \brief 队伍分经验
 *
 */
void SceneNpc::distributeExp()
{

	Zebra::logger->error("分经验啦!");
	NpcHpHashmap_iterator iter;
	for(iter = expmap.begin() ; iter != expmap.end() ; iter++)
	{
   		if (abs((long)(iter->second.attack_time.sec() - SceneTimeTick::currentTime.sec())) >=10)
		{
#ifdef _DEBUG
			Zebra::logger->debug("%d 最后一次攻击大于十秒",iter->first);
#endif      
			continue;
		}
		switch(iter->second.IdType)
		{
		case TYPE_USER:
			{
				SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(iter->first);

				if(pUser)
				{
					DWORD wdExp = (DWORD)(npc->exp * ((float)iter->second.wdHP / (this->getMaxHP())) + 0.9f);

					//如果技能增加经验值
					wdExp +=pUser->pkValue.exp;
					//宠物
					if (pUser->pet) pUser->pet->addExp(wdExp);
					//领养
					for (SceneUser::adopt_it it=pUser->adoptList.begin(); it!=pUser->adoptList.end(); it++)
						it->second->releaseExp(wdExp*15/100);
					wdExp = levelExp(wdExp,pUser->charbase.level);
					wdExp = addOtherExp(wdExp,pUser);
					if (wdExp >0)
					{
						//if (pUser->scene &&(pUser->charbase.country == pUser->scene->getCountryID())) wdExp = (DWORD)(wdExp*(pUser->wdTirePer/100.0f));

						if (pUser->isSpecWar(Cmd::COUNTRY_FORMAL_DARE))
						{//国战期,打怪经验减50%
							wdExp = wdExp/2;
						}

						//判断是否是经验倍率状态
						if (pUser->issetUState(Cmd::USTATE_EXP_125))
						{
							Zebra::logger->debug("原经验值为:%u",wdExp);
							wdExp = (DWORD)(wdExp*1.25);
							Zebra::logger->debug("提高1.25倍经验值为:%u",wdExp);
						}

						if (pUser->issetUState(Cmd::USTATE_EXP_150))
						{
							Zebra::logger->debug("原经验值为:%u",wdExp);
							wdExp = (DWORD)(wdExp*1.5);
							Zebra::logger->debug("提高1.50倍经验值为:%u",wdExp);
						}

						if (pUser->issetUState(Cmd::USTATE_EXP_175))
						{
							Zebra::logger->debug("原经验值为:%u",wdExp);
							wdExp = (DWORD)(wdExp*1.75);
							Zebra::logger->debug("提高1.75倍经验值为:%u",wdExp);
						}

						if (wdExp==0) wdExp=1;
						//pUser->sendExpToSept(wdExp);
						pUser->packs.equip.obtain_exp(pUser,wdExp);

						DWORD oldExp = wdExp;
						wdExp += pUser->scene->sceneExp(wdExp);
						if (pUser->charbase.country == pUser->scene->getCountryID())
						{
							wdExp += pUser->scene->winnerExp(oldExp);
						}
						//wdExp = wdExp*(100+5*pUser->adoptList.size())/100;  //收养一个多5%
						pUser->addExp(wdExp,false,this->tempid,Cmd::MAPDATATYPE_NPC,true);
					}
				}
			}
			break;
		case TYPE_TEAM:
			{
				//sky 根据队伍唯一ID找到队长ID
				TeamManager* team = SceneManager::getInstance().GetMapTeam(iter->first);

				if (team)
				{
					if (this->getMaxHP() > 0)
					{
						DWORD exp = (DWORD)(((float)npc->exp * ((float)iter->second.wdHP / (float)(this->getMaxHP()))) + 0.9f);

						//sky 遍历队伍成员求出等级差和等级和
						DWORD levelDer = 0;	//sky 等级差
						DWORD levelSum = 0;	//sky 等级和

						WORD Maxlevel = 0;	//sky 最大等级
						WORD Minlevel = 0xFFFF;	//sky 最小等级

						std::vector<TeamMember>::iterator iter;
						for(iter = team->getTeam().member.begin() ; iter != team->getTeam().member.end() ; iter ++)
						{
							SceneUser *u = SceneUserManager::getMe().getUserByTempID(iter->tempid);

							if(u->getLevel() > Maxlevel)
								Maxlevel = u->getLevel();

							if(u->getLevel() < Minlevel)
								Minlevel = u->getLevel();

							levelSum += u->getLevel();
						}

						levelDer = Maxlevel - Minlevel;

						ExpTeamExec exec(team,this,exp, levelDer, levelSum);
						team->execEveryOne(exec);
					}
				}

			}
			break;
		}
	}
	expmap.clear();
	return;
}

DWORD SceneNpc::addOtherExp(DWORD wdExp,SceneUser *pUser)
{
	if (pUser)
	{
		BYTE per = pUser->packs.equip.getEquips().get_doublexp();
		if ((per>20)&&(per<100)) per=20;
		else if (per>=100) per=100;

		if (selectByPercent(per))
		{
			wdExp <<= 1;
		}
	}
	return wdExp;
}

DWORD SceneNpc::levelExp(DWORD wdExp,DWORD char_level)
{
  int diff = (int)(char_level - this->npc->level);

  if(diff >= 5)
  {
	  wdExp = 0;
  }
  else if(diff >= 4 && diff > 0)
  {
	  wdExp = (DWORD)((float)wdExp*(1-0.05f*diff));
  }
  else if (diff < 0)
  {
	  int i = (-diff)>4?4:(-diff);
	  wdExp = (DWORD)((float)wdExp*(1+0.05f*i));
  }

  return wdExp;
}

void SceneNpc::changeHP(const SDWORD &curHp)
{
  SDWORD changeValue = 0;

  if (((int)this->hp)+(int)curHp>=0)
  {
    changeValue = this->hp;
    this->hp += curHp;
    if (this->hp > this->getMaxHP()) this->hp = this->getMaxHP();
    changeValue = (int)this->hp-changeValue;
  }
  else
  {
    changeValue= this->hp;
    this->hp=0;
  }

  notifyHMS = true;

  if (changeValue !=0)
  {
	  if( npc->kind == NPC_TYPE_GHOST )	
	  {
		  Cmd::stObjectHpMpPopUserCmd ret;
		  ret.dwUserTempID = this->tempid;
		  ret.byTarget = Cmd::MAPDATATYPE_USER;
		  ret.vChange = (int)changeValue;
		  ret.type = Cmd::POP_HP;
		  this->scene->sendCmdToNine(getPosI(),&ret,sizeof(ret),this->dupIndex);
	  }
	  else
	  {
		  Cmd::stObjectHpMpPopUserCmd ret;
		  ret.dwUserTempID = this->tempid;
		  ret.byTarget = Cmd::MAPDATATYPE_NPC;
		  ret.vChange = (int)changeValue;
		  ret.type = Cmd::POP_HP;
		  this->scene->sendCmdToNine(getPosI(),&ret,sizeof(ret),this->dupIndex);
	  }
  }
}


void SceneNpc::changeMP(const SDWORD &mp)
{
    notifyHMS = true;
}

void SceneNpc::changeSP(const SDWORD &sp)
{
    notifyHMS = true;
}


bool SceneNpc::preAttackMe(SceneEntryPk *pEntry,const Cmd::stAttackMagicUserCmd *rev,bool physics,const bool good)
{
#ifdef _DEBUG
  //Zebra::logger->debug("SceneNpc::preAttackMe %s hp=%u",name,hp);
#endif
  if (getState() != zSceneEntry::SceneEntry_Normal
      || !canBeAttack())  
  {
    if (rev)
      ScenePk::attackFailToMe(rev,pEntry,true);

    return false;
  }

  if (this->npc->kind == NPC_TYPE_RESOURCE)
  {
    if (pEntry->getType() != zSceneEntry::SceneEntry_Player)
    {
      if (rev)
        ScenePk::attackFailToMe(rev,pEntry,true);
      return false;
    }
    else
    {
      if (rev)
      {
        if (rev->wdMagicType != SERVER_SKILL_ATTACK_NORMAL &&
			rev->wdMagicType != SERVER_SKILL_DAGGER_ATTACK_NORMAL &&
			rev->wdMagicType != SERVER_SKILL_HANDS_ATTACK_NORMAL)
        {
          return false;
        }
      }
      SceneUser *pUser = (SceneUser *)pEntry;
      zObject *temp=NULL;
      bool ret=false;
      if (pUser->packs.equip.getObjectByZone(&temp,0,Cmd::EQUIPCELLTYPE_HANDR))
      {
        if (temp)
        {
          if (0 != temp->data.dur)
          {
            if (temp->base->id == 876) //采集手套
            {
              ret=true;
            }
          }
        }
      }
      if (!ret)
      {
        if (rev)
          ScenePk::attackFailToMe(rev,pEntry,true);
        return false;
      }
    }
  }

  if (!good)
  {
    DWORD attackRating = 0; // 攻击者
    DWORD attLevel = 0;
    DWORD attCountryID = 0;
    bool  isUnionCityWar = false;
    bool  isCountryFormal = false; // 判断大将军所属国与攻击者是否属于敌对
    bool  isAtt = true; // 默认为夺旗类对战中的攻方
    bool  isAttCountry = true; // 国战中的攻方
    bool  isAntiCountry = false;

    SceneEntryPk * m = ((SceneEntryPk *)pEntry)->getMaster();
    while (m && m->getType()!=zSceneEntry::SceneEntry_Player)
      m = m->getMaster();

    SceneUser * master = NULL;

    if (m)
    {
      master = (SceneUser*)m;
      attackRating = master->charstate.attackrating;
      attLevel = master->charbase.level;
      attCountryID = master->charbase.country;
      isUnionCityWar = master->isSpecWar(Cmd::UNION_CITY_DARE);

      isCountryFormal = master->isWarRecord(Cmd::COUNTRY_FORMAL_DARE,this->scene->getCountryID());
      isAtt  = master->isAtt(Cmd::UNION_CITY_DARE);
      isAttCountry = master->isAtt(Cmd::COUNTRY_FORMAL_DARE);
      isAntiCountry = master->isAntiAtt(Cmd::COUNTRY_FORMAL_DARE,this->scene->getCountryID());

      refreshExpmapAttackTime(master);
    }
    else
    {
      attackRating = ((SceneNpc*)pEntry)->npc->rating;
      attLevel = ((SceneNpc *)pEntry)->getLevel();
      attCountryID = ((SceneNpc *)pEntry)->scene->getCountryID();
    }

    if (this->id == 58101) // 城旗的ID
    {
//      if (!isUnionCityWar ||   (this->scene->getCountryID() != attCountryID && this->scene->getCountryID()!=6) || !isAtt)
      if (!isUnionCityWar || !isAtt)

      {
#ifdef _DEBUG
        Channel::sendNine(this,"不处于夺城战,或不是攻方,不能攻击58101");
#endif      

        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;
      }
    }

    if (this->id == COUNTRY_MAIN_FLAG)
    {
      if (!CountryDareM::getMe().isAttackMainFlag(this->scene,COUNTRY_SEC_FLAG))
      {
        if (master)  
        {
          Channel::sendSys(master,Cmd::INFO_TYPE_FAIL,"还存在副旗,不能攻击主旗"); 
        }
#ifdef _DEBUG
        Channel::sendNine(this,"攻击国旗失败,还存在副旗");
#endif      

        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;
      }
    }

    if (this->id==COUNTRY_MAIN_FLAG || this->id==COUNTRY_SEC_FLAG)
    {

      if (this->scene->getCountryID() ==attCountryID)
      {
#ifdef _DEBUG
        Channel::sendNine(this,"本国人不允许攻击国旗");
#endif      
        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);
        return false;
      }

      if (attLevel>50)
      {
#ifdef _DEBUG
        Channel::sendNine(this,"高于50级不允许攻击凤凰城内的国旗");
#endif      

        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;
      }
    }

    if (this->id==COUNTRY_KING_MAIN_FLAG || this->id==COUNTRY_KING_SEC_FLAG)
    {
      if (this->scene->getCountryID() ==attCountryID)
      {
#ifdef _DEBUG
        Channel::sendNine(this,"本国人不允许攻击国旗");
#endif      
        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;

      }

      if (isCountryFormal)
      {
        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;
      }
    }

    if (this->id == COUNTRY_KING_MAIN_FLAG)
    {
      if (!CountryDareM::getMe().isAttackMainFlag(this->scene,COUNTRY_KING_SEC_FLAG))
      {
        if (master)  
        {
          Channel::sendSys(master,Cmd::INFO_TYPE_FAIL,"还存在副旗,不能攻击主旗"); 
        }
#ifdef _DEBUG
        Channel::sendNine(this,"攻击国旗失败,还存在副旗");
#endif      
        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;
      }
    }

    if (this->isMainGeneral() || this->id==COUNTRY_SEC_GEN)
    {//本国人不允许攻击大将军和灵将
      if (this->scene->getCountryID() ==attCountryID)
      {
        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);
        return false;
      }
    }
    if (this->isMainGeneral())
    {//判断是否允许攻击大将军
      if (!isCountryFormal || !CountryDareM::getMe().isAttackMainGen(this->scene) ||
          (!isAttCountry && !isAntiCountry))
      {
        if (master)
        {
          Channel::sendSys(master,Cmd::INFO_TYPE_FAIL,"不在国战期间或不是进攻方,不能攻击大将军");
        }

        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;
      }
    }

    if (this->id==COUNTRY_EMPEROR_MAIN_GEN || this->id==COUNTRY_EMPEROR_SEC_GEN)
    {//占领国人不允许攻击皇城大将军和皇城灵将
      if (this->scene->getEmperorDare())
      {
        if (this->scene->getEmperorDareDef() == attCountryID)
        {// 自己人不打自己人
          if (rev)
            ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);
          return false;
        }
      }
      else
      {//不在皇城争夺战期间,谁都不能打皇城大将军和灵将
        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);
        return false;
      }
    }

    if (this->id==COUNTRY_EMPEROR_MAIN_GEN)
    {//判断是否允许攻击皇城大将军
      if (!this->scene->getEmperorDare() 
          || !CountryDareM::getMe().isAttackMainFlag(this->scene,COUNTRY_EMPEROR_SEC_GEN))
      {
        if (master) 
        {
          //Channel::sendSys(master,Cmd::INFO_TYPE_FAIL,"还存在禁卫队长或不是参战国,不能攻击大将军"); 
          Channel::sendSys(master,Cmd::INFO_TYPE_FAIL,"不在皇城战期间或还有禁军队长,不能攻击大将军");
        }

        if (rev)
          ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

        return false;
      }
    }

    if ((this->id == COUNTRY_SEC_GEN) && !this->scene->getCountryDare())
    {//判断是否允许攻击灵将
      if (master) 
      {
        Channel::sendSys(master,Cmd::INFO_TYPE_FAIL,"不在国战期间,不能攻击禁卫队长"); 
      }

      if (rev)
        ScenePk::attackFailToMe(rev,(SceneUser*)pEntry,true);

      return false;
    }
  }

  //初始化pk数据
  this->pkValue.init();
  this->skillValue.init();

  this->skillStatusM.processPassiveness();  // 处理我的被动状态影响

  if (pEntry->getType() == SceneEntry_Player)
  {
    SceneUser *pAtt = (SceneUser *)pEntry;
    ScenePk::calpdamU2N(rev,pAtt,this);
  }
  else
  {
    SceneNpc *pAtt = (SceneNpc *)pEntry;
    ScenePk::calpdamN2N(rev,pAtt,this);
  }
  if (pEntry->getType() == SceneEntry_Player)
  {
    SceneUser *pAtt = (SceneUser *)pEntry;
    ScenePk::calmdamU2N(rev,pAtt,this);
  }
  else
  {
    SceneNpc *pAtt = (SceneNpc *)pEntry;
    ScenePk::calmdamN2N(rev,pAtt,this);
  }

  SceneEntryPk *pMaster = this->getTopMaster();
  if (pMaster&&pMaster->getType() == zSceneEntry::SceneEntry_Player)
  {
    SceneUser *pUser = (SceneUser *)pMaster;
    pUser->packs.equip.costDefenceDurByPet(pUser);
  }
  return true;
}

bool SceneNpc::AttackMe(SceneEntryPk *pAtt,const Cmd::stAttackMagicUserCmd *rev,bool physics,SWORD rangDamageBonus)
{
  using namespace Cmd;

  SceneEntryPk * am = pAtt->getTopMaster();
  if (am->getType()==zSceneEntry::SceneEntry_Player)
  {
    SceneUser * a = (SceneUser *)am;
    //被外国人打第一下时给予通知
    if (hp==getMaxHp() && scene->getCountryID()!=a->charbase.country)
    {
      char buf[MAX_CHATINFO];
      bzero(buf,sizeof(buf));
      _snprintf(buf,MAX_CHATINFO-1,"%s(%s) 正在 %s(%u,%u) 攻击 %s,请速支援"
          ,a->name,SceneManager::getInstance().getCountryNameByCountryID(a->charbase.country)
          ,scene->name,getPos().x,getPos().y,name);

      //攻击功能NPC,全国通知
      if (!isBugbear() && canBeAttack())
        Channel::sendCountryInfo(scene->getCountryID(),Cmd::INFO_TYPE_EXP,buf);

      //攻击大将军和近卫队长,向王城、边境、东郊、南郊通知
      if (isMainGeneral() || id==COUNTRY_SEC_GEN)
      {
        DWORD high = (scene->getCountryID()<<16);
        Channel::sendMapInfo(high+WANGCHENG_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
        Channel::sendMapInfo(high+DONGJIAO_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
        Channel::sendMapInfo(high+NANJIAO_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
        Channel::sendMapInfo(high+BIANJING_MAP_ID,Cmd::INFO_TYPE_EXP,buf);
      }
    }
  }

  if (pAtt->getMaster())
  {
    this->setLockUser(pAtt->getMaster()->id);
  }
#ifdef _DEBUG
  //Zebra::logger->debug("SceneNpc::AttackMe %s hp=%u",name,hp);
#endif
  SceneEntryPk::AttackMe(pAtt,rev,physics,rangDamageBonus);

  if (Cmd::PET_TYPE_NOTPET!=getPetType())
  {
    ((ScenePet *)this)->sendHpExp();
  }
  //处理AI
  if (AIC)
    AIC->on_hit(pAtt);
  if (aif&AIF_GIVEUP_6_SEC)
    setEndBattleTime(SceneTimeTick::currentTime,6*1000);
  if (aif&AIF_GIVEUP_3_SEC)
    setEndBattleTime(SceneTimeTick::currentTime,3*1000);
  //setAttackTime(SceneTimeTick::currentTime,1200);
  return true;
}


struct HpTeamExec : public TeamMemExec
{
  const Cmd::stNPCHPMapScreenUserCmd &ret;
  HpTeamExec(const Cmd::stNPCHPMapScreenUserCmd &cmd) : ret(cmd)
  {
  }
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
    if (pUser)
    {
      pUser->sendCmdToMe(&ret,sizeof(ret));
      return true;
    }
    else
    {
      return true;
    }
  }
};

/**
 * \brief 判断一个对象是否在攻击自己
 * 根据经验列表来判断
 * 只适用于判断玩家和宠物
 *
 * \param entry 要判断的对象
 * \return 是否在攻击自己
 */
bool SceneNpc::isAttackMe(SceneEntryPk *entry)
{
	if (!entry) return false;

	if (zSceneEntry::SceneEntry_NPC==entry->getType())
	{
		if (Cmd::PET_TYPE_NOTPET!=((SceneNpc *)entry)->getPetType())
			return isAttackMe(((SceneNpc *)entry)->getMaster());
		else
			return false;
	}
	else if (zSceneEntry::SceneEntry_Player==entry->getType())
	{
		SceneUser * pUser = (SceneUser *)entry;
		TeamManager * team = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);

		if (team)
		{
			NpcHpHashmap_iterator iter;
			iter = expmap.find(team->getTeamtempId());

			if(iter != expmap.end())
				return true;
		}
		else
		{
			NpcHpHashmap_iterator iter;
			iter = expmap.find(pUser->tempid);

			if (iter != expmap.end())
				return true;
		}
	}
	return false;
}
void SceneNpc::showHP(SceneUser *pUser,DWORD npchp)
{
  if (!pUser)
  {
    return ;
  }
  Cmd::stNPCHPMapScreenUserCmd ret;
  ret.dwMapNpcDataPosition = tempid;
  ret.dwHP = npchp;//this->hp;        
  ret.dwMaxHP =getMaxHP();        
  pUser->sendCmdToMe(&ret,sizeof(ret));
}
/**
 * \brief 通知客户端生命值的变化
 */
void SceneNpc::attackRTHpAndMp()
{
    notifyHMS = false;
}


/**
 * \brief 构造函数
 * \param scene npc所在的场景
 * \param npc 基本数据
 * \param define 定义数据
 * \param type 类型
 * \param entrytype 物件类型(玩家、npc、建筑等)
 * \param anpc 用于强化的npc base
 */
SceneNpc::SceneNpc(Scene *scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype,zNpcB *anpc) : SceneEntryPk(entrytype,SceneEntry_Hide),npc(npc),anpc(anpc),define(define),_half_sec(0.3f),_one_sec(1),_3_sec(3),lockedUserTime(),nextMoveTime(npc->distance),nextAttackTime(npc->adistance),type(type),isUse(false)
{
  id = npc->id;
  this->scene = scene;
  if (type == STATIC)
  {
    tempid = ++serialID;
  }
  else if (type == GANG)
  {
    tempid = uniqueID.get();
    if (tempid==uniqueID.invalid())
      Zebra::logger->fatal("SceneNpc::SceneNpc 临时编号分配错误");
  }
  if ((char)define->name[0]) {
    strncpy(name,define->name,MAX_NAMESIZE);
  }else {
    strncpy(name,npc->name,MAX_NAMESIZE);
  }

  catchme = 0; //吸引敌人攻击标志
  boostupPet =0; // 宠物增强
  boostupPetMDef = 0; //增强宠物的法术防御
  boostupSummon = 0; //召唤兽攻击加强
  dwReduceDam = 0; //召唤兽伤害扣减
  giddy =0; //怒吼训练使对方眩晕的几率
  boostupHpMaxP =0; // 增强宠的生命最大值百分比

  hp = this->getMaxHP();//npc->hp+(anpc?anpc->hp:0);
  chaseMode = CHASE_NONE;
  curTargetType = 0;
  curTargetID = 0;
  defTargetType = 0;
  defTargetID = 0;
  dwNpcLockedUser = 0;
  lostObject = false;
  notifyHMS = false;
  clearMe = false;
  backOffing = 0;
  dwStandTime =SceneTimeTick::currentTime.sec();
  dwStandTimeCount=0;
  skillStatusM.initMe(this);
  //dupIndex = 0;

  speedRate = 1.0;
  aspeedRate = 1.0;
  speedUpUnder20 = false;
  aspeedUpUnder50 = false;
  recoverUnder30 = false;
  //master = NULL;

  appendMinDamage = 0;
  appendMaxDamage = 0;

  AIC = new NpcAIController(this);

  AIC->setActRegion(define->region.c,define->width/2,define->height/2);
  AIC->loadScript(define->scriptID);

  //[sky]初始化AI变量
  m_dwNowTime = 0;
  m_mobType	  = false;
  m_bFighting = false;
  m_dwGtime = 0;

  //[sky]先冲AI容器里去找自己的AI
  //GetNpcAi();

  isRushNpc = false;
  lockTarget = false;

  needRecover = false;

  notifystep=0; // 绿boss 通知步骤

  //petAI = 0;
  //aif = npc->ai;
#ifdef _DEBUG
  //aif = npc->ai | AIF_LIMIT_REGION;
#endif
  //宠物
  pet = 0;
  summon = 0;
  totems.clear();
  MirageSummon.clear();

  summoned = false;

  switch(this->npc->kind)
  {
    case NPC_TYPE_TOTEM:
      {
        switch(this->npc->id)
        {
          case 300:
            {
              this->setUState(39);
            }
            break;
          default:
            {
              this->setUState(39);
            }
            break;
        }
      }
      break;
    default:
      break;
  }

  ///加载npc技能
  if (!npc->skillMap.empty())
  {
    std::map<int,std::vector<npcSkill> >::iterator map_it;
    for (map_it=npc->skillMap.begin();map_it!=npc->skillMap.end();map_it++)
    {
      std::vector<npcSkill>::iterator skill_it;
      for (skill_it=map_it->second.begin();skill_it!=map_it->second.end();skill_it++)
      {
        //zSkill::create(this,skill_it->id,1);

        zSkill *skill = zSkill::create(this,skill_it->id,1);
        if (!skill)
          Zebra::logger->error("SceneNpc::SnceneNpc():无法加载技能 name=%s skill=%u level=%u",npc->name,skill_it->id,npc->level);
      }
    }
  }
  if (npc->skill>0)
  {
    zSkill::create(this,npc->skill,npc->level) ;
  }

  //sky 初始化建造时间
  MakeTime = 0;

  //sky 每申请完毕一个NPC就把NPC的记数器加1
  MapdwNpcNum++;
}

/**
 * \brief 析构函数
 *
 */
SceneNpc::~SceneNpc()
{
  if (type == GANG)
  {
    uniqueID.put(tempid);
    SAFE_DELETE(define);
  }
  SAFE_DELETE(AIC);
}

/**
 * \brief 返回Npc跟踪状态
 * \return 跟踪状态
 */
SceneNpc::SceneNpcChase SceneNpc::getChaseMode() const
{
    return chaseMode;
}

/**
 * \brief 获取跟踪用户编号
 * \return 用户编号
 */
SceneEntryPk* SceneNpc::getChaseSceneEntry() const
{
  switch (curTargetType)
  {
    case zSceneEntry::SceneEntry_Player:
      {
        return SceneUserManager::getMe().getUserByTempID(curTargetID);
      }
      break;
    case zSceneEntry::SceneEntry_NPC:
      {
        return SceneNpcManager::getMe().getNpcByTempID(curTargetID);
      }
      break;
    default:
      {
        return NULL;
      }
      break;
  }
  return NULL;
}

/**
 * \brief 设置跟踪用户
 * \param type 跟踪对象的类型
 * \param entryid 要跟踪用户的编号
 * \return 是否跟踪成功
 */
bool SceneNpc::chaseSceneEntry(const DWORD type,const DWORD entryid)
{
    //不设置自己
    if (type==(DWORD)getType()&&entryid==tempid) return false;

  if (curTargetID!=0)
      return false;
  else
  {
      chaseMode = CHASE_ATTACK;
      //curTargetType = type;
      //curTargetID = entryid;
      setCurTarget(entryid,type);
      if (aif&AIF_GIVEUP_10_SEC)
          setEndBattleTime(SceneTimeTick::currentTime,10*1000);
      return true;
  }
}

/**
 * \brief 强制跟踪用户,如果怪已经在跟踪用户,那么有45%的几率将目标转换成目前的用户
 * \param userid 要跟踪用户的编号
 * \return 是否跟踪成功
 */
bool SceneNpc::forceChaseUser(SceneEntryPk *pAtt)
{
  //if (chaseMode != CHASE_NONE)
  if (curTargetID!=0)
  {
    if (selectByPercent(30))
    {
      chaseMode = CHASE_ATTACK;
      setCurTarget(pAtt);
      //Channel::sendNine(this,"更换攻击目标%s",pAtt->name);
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    chaseMode = CHASE_ATTACK;
    setCurTarget(pAtt);
    AIC->on_find_enemy(pAtt);
    if (aif&AIF_GIVEUP_10_SEC)
      setEndBattleTime(SceneTimeTick::currentTime,10*1000);

    return true;
  }
}

/**
 * \brief 取消对用户进行跟踪
 *
 */
void SceneNpc::unChaseUser()
{
  }
}

/**
 * \brief 更新锁定用户,在物品保护的时候使用
 * \param ct 当前时间
 */
bool SceneNpc::checkLockUserOverdue(const zRTime &ct)
{
  if (getState() == zSceneEntry::SceneEntry_Normal
      && dwNpcLockedUser
      && ct > lockedUserTime)
  {
    dwNpcLockedUser = 0;
    lockedUserTime = ct;
    return true;
  }
  return false;
}

/**
 * \brief 设置锁定用户
 * \param dwID 被保护的用户
 */
void SceneNpc::setLockUser(const DWORD dwID)
{
}

/**
 * \brief 检查Npc是否已经到达可以移动时间
 * \param ct 当前时间
 * \return 判断是否成功
 */
bool SceneNpc::checkMoveTime(const zRTime &ct)
{
    return ct >= nextMoveTime;
}

/**
 * \brief 设置Npc下一次移动时间
 * \param ct 当前时间
 */
void SceneNpc::setMoveTime(const zRTime &ct)
{
}

/**
 * \brief 设置Npc下一次移动时间
 * \param ct 当前时间
 * \param delay 延迟时间,毫秒
 */
void SceneNpc::setMoveTime(const zRTime &ct,const int delay)
{
}

/**
 * \brief 延长下次移动的时间
 *
 *
 * \return 
 */
void SceneNpc::delayMoveTime(const int delay)
{
}

/**
 * \brief 检查Npc是否已经到达可以攻击时间
 * \param ct 当前时间
 * \return 判断是否成功
 */
bool SceneNpc::checkAttackTime(const zRTime &ct) const
{
    return ct >= nextAttackTime;
}

/**
 * \brief 设置Npc下一次攻击时间
 * \param ct 当前时间
 */
void SceneNpc::setAttackTime(const zRTime &ct)
{
}

/**
 * \brief 设置Npc下一次攻击时间
 * \param ct 当前时间
 * \param delay 延迟时间,毫秒
 */
void SceneNpc::setAttackTime(const zRTime &ct,const int delay)
{
    nextAttackTime = ct;
    nextAttackTime.addDelay((int)(delay/aspeedRate));
}

/**
 * \brief 判断是否可以掉落物品
 * \param ct 当前时间
 * \return 是否可以掉落物品
 */
bool SceneNpc::canLostObject(const zRTime &ct)
{
  if (lostObject)
  {
    reliveTime = SceneTimeTick::currentTime;

	//sky 根据复活修正系数来休正复活的延迟时间
	DWORD interval = 0;

	if((define->interval * 1000) > ((t_NpcDefine *)define)->GetIntervalAmendmaent(false))
		interval = (define->interval * 1000) - ((t_NpcDefine *)define)->GetIntervalAmendmaent(false);

    reliveTime.addDelay(interval);  //sky 设置复活时间

    if (NPC_TYPE_BBOSS==npc->kind || NPC_TYPE_PBOSS==npc->kind)
      Zebra::logger->debug("[BOSS]%s(%u) 死亡",name,tempid);

    lostObject = false;
    return true;
  }
  else
    return false;
}

/**
 * \brief 判断是否可以重生
 * \param ct 当前时间
 * \return 是否可以重生
 */
bool SceneNpc::canRelive(const zRTime &ct)
{
  if (!lostObject
      && ct > reliveTime)
  {
    setMoveTime(ct);
    setAttackTime(ct);
    notifystep=0;
    if (this->npc->kind == NPC_TYPE_BBOSS)
    {
      Cmd::stRefreshBossUserCmd send;
      strncpy(send.mapName,this->scene->getName(),sizeof(send.mapName));
      //send.x = this->getPos().x;
      //send.y = this->getPos().y;
      send.time = 0;
      send.npcid = this->npc->id;
      send.country = (BYTE)this->scene->getCountryID();
      this->forwardSession(&send,sizeof(send));
    }

	//sky 重新把系数设置一下
	((t_NpcDefine *)define)->GetIntervalAmendmaent(true);

    return true;
  }
  else
  {
    switch(this->npc->kind)
    {
      case NPC_TYPE_BBOSS:
        {
          int timegap=500;
          switch(notifystep)
          {
            case 0:
              timegap = 20;
              break;
            case 1:
              timegap = 10;
              break;
            case 3:
              timegap = 500;
              break;
            default:
              break;
          }
          if ((int)((reliveTime.sec()-ct.sec())/60) == timegap)
          {
            Cmd::stRefreshBossUserCmd send;
            strncpy(send.mapName,this->scene->getName(),sizeof(send.mapName));
            //send.x = this->getPos().x;
            //send.y = this->getPos().y;
            send.time = timegap;
            send.npcid = this->npc->id;
            send.country = (BYTE)this->scene->getCountryID();
            this->forwardSession(&send,sizeof(send));
            notifystep++;
          }
        }
        break;
      default:
        break;
    }
    return false;
  }
}

/**
 * \brief 将命令转发到会话服务器
 *
 * \param pNullCmd 待转发的命令
 * \param nCmdLen 命令长度
 */
bool SceneNpc::forwardSession(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
  if (nCmdLen > zSocket::MAX_USERDATASIZE)
  {
    Zebra::logger->debug("消息越界(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
  }
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::Session::t_Session_ForwardUser *sendCmd=(Cmd::Session::t_Session_ForwardUser *)buf;
  constructInPlace(sendCmd);
  sendCmd->dwID=0;
  sendCmd->size=nCmdLen;
  bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Session::t_Session_ForwardUser));
  return sessionClient->sendCmd(buf,sizeof(Cmd::Session::t_Session_ForwardUser)+nCmdLen);
}

/**
 * \brief 获取Npc类型
 * 静态的还是动态分配的
 * \return Npc类型
 */
const SceneNpc::SceneNpcType &SceneNpc::getSceneNpcType() const
{
    return type;
}

/**
 * \brief 填充NPC自己的信息到结构中
 *
 * \param data 要填充的结构
 */
void SceneNpc::full_t_NpcData(Cmd::t_NpcData &data)
{
}
/**
 * \brief 填充NPC自己的信息到结构中
 *
 * \param data 要填充的结构
 */
void SceneNpc::full_t_MapNpcData(Cmd::t_MapNpcData &data)
{
}
/**
 * \brief 填充NPC自己的信息到结构中
 *
 * \param data 要填充的结构
 */
void SceneNpc::full_t_MapNpcDataAndPos(Cmd::t_MapNpcDataPos &data)
{
}
/**
 * \brief 填充NPC自己的信息到结构中
 *
 * \param data 要填充的结构
 */
void SceneNpc::full_t_MapNpcDataState(Cmd::t_MapNpcDataState &data)
{
  full_t_NpcData(*((Cmd::t_NpcData*)&data));
  data.num = full_UState(data.state);
}
/**
 * \brief 填充NPC自己的信息到结构中
 *
 * \param data 要填充的结构
 */
void SceneNpc::full_t_MapNpcDataAndPosState(Cmd::t_MapNpcDataPosState &data)
{
  full_t_NpcData(*((Cmd::t_NpcData*)&data));
  //坐标
  data.x=getPos().x;
  data.y=getPos().y;
  data.byDir=getDir();
  data.num = full_UState(data.state);
}
/**
 * \brief 填充魔法位置信息到结构中
 *
 */
void SceneNpc::full_stRTMagicPosUserCmd(Cmd::stRTMagicPosUserCmd &ret) const
{
}

/**
 * \brief Npc动作
 * \return 动作执行是否成功
 */
void SceneNpc::action(const zRTime& ctv)
{
}

/**
 * \brief 场景上Npc的AI主处理函数
 * \param ctv 当前时间
 * \param affectNpc 要处理的npc列表
 */
void SceneNpc::AI(const zRTime& ctv,MonkeyNpcs &affectNpc,const DWORD group,const bool every)
{
}

/**
 * \brief 查找正在跟踪的敌人
 *
 * \param entry 输出,找到的对象
 * \return 是否查找成功
 */
bool SceneNpc::checkChaseAttackTarget(SceneEntryPk *&entry)
{
	if (getPetType()!=Cmd::PET_TYPE_NOTPET && getPetType()!=Cmd::PET_TYPE_SEMI)
		((ScenePet *)this)->checkMasterTarget(entry);
	if (entry) return true;

	if (curTargetID!=0 || defTargetID!=0)
	{
		entry = getChaseSceneEntry();
		SceneEntryPk * dt = 0;
		bool switchTarget = false;
		if (getPetType()==Cmd::PET_TYPE_NOTPET || getPetType()==Cmd::PET_TYPE_SEMI)
		{
			dt = getDefTarget();
			if (!entry || (dt && selectByPercent(1)))
			{
				entry = dt;
				switchTarget = true;
			}
		}
		if (NULL == entry//找不到目标,下线或者换地图
			|| entry->getState() != zSceneEntry::SceneEntry_Normal//隐身或死亡
			//sky 炮塔的攻击范围限定为索敌范围,超过这个范围就寻找新的目标
			|| ((npc->kind == NPC_TYPE_TURRET || npc->kind == NPC_TYPE_BARRACKS || npc->kind == NPC_TYPE_CAMP)&&(!scene->zPosShortRange(getPos(),entry->getPos(),npc_search_region)))
			|| (!scene->zPosShortRange(getPos(),entry->getPos(),npc_lost_target_region) && !lockTarget)
			|| !canReach(entry) //即使往前走也攻击不到
			|| entry->angelMode //sky 目标变成无敌的拉
			|| (entry->getType() == zSceneEntry::SceneEntry_Player && NPC_AI_PATROL==AIDefine.type && !entry->isRedNamed())
			|| (!(canFight()))
			|| entry->hideme)  //不能战斗
		{
#ifdef _DEBUG
			//if (entry && !scene->zPosShortRange(getPos(),entry->getPos(),npc_lost_target_region))
			//Zebra::logger->debug("%s->%s 距离超过 %u 格",name,entry->name,npc_lost_target_region);
#endif
			if( npc->kind == NPC_TYPE_TURRET ) // 炮台类的NPC直接放弃跟踪
				unChaseUser();//放弃追踪
			else if (!chaseSecondTarget()&&!chaseItsMaster())
				unChaseUser();//放弃追踪			
		}
		else
		{
			if (1==isEnemy(entry))
			{
				if (switchTarget)
				{
					setCurTarget(entry,true);
					clearDefTarget();
				}
				return true;
			}
			else
			{
				unChaseUser();//放弃追踪
			}
		}
	}

	entry = NULL;
	return false;
}


/**
 * \brief 判断敌人是否在攻击范围内
 *
 *
 * \param entry 敌人指针
 * \return 是否在攻击范围内
 */
bool SceneNpc::inRange(SceneEntryPk * entry)
{
  BYTE atype = 0;
  BYTE action = 0;
  npc->getATypeAndAction(atype,action);

  switch(atype)
  {
    case NPC_ATYPE_NEAR:  /// 近距离攻击
    case NPC_ATYPE_MNEAR:  /// 近距离法术攻击
      if (!scene->zPosShortRange(getPos(),entry->getPos(),1))
      {
        //Zebra::logger->debug("近距离攻击不成功");
        return false;
      }
      break;
    case NPC_ATYPE_MFAR:  /// 法术远距离攻击
    case NPC_ATYPE_FAR:    /// 远距离攻击
      if (!scene->zPosShortRange(getPos(),entry->getPos(),6))
      {
        if (getPetType()!=Cmd::PET_TYPE_NOTPET && scene->zPosShortRange(getPos(),entry->getPos(),10))
          return true;

        return false;
      }
      else
      {
        if ((Cmd::PET_TYPE_NOTPET==getPetType())&&
            (!scene->zPosShortRange(getPos(),entry->getPos(),1))&&
            selectByPercent(1)) return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

/**
 * \brief 判断移动后敌人是否在攻击范围内
 *
 *
 * \param entry 敌人指针
 * \return 是否在攻击范围内
 */
bool SceneNpc::canReach(SceneEntryPk *entry)
{
  //只有宠物判断这个范围
  switch (getPetType())
  {
    case Cmd::PET_TYPE_NOTPET:
    case Cmd::PET_TYPE_SEMI:
    case Cmd::PET_TYPE_TOTEM:
	case Cmd::PET_TYPE_TURRET:  //sky 新增炮台类型处理
      {
        switch (this->npc->kind)
        {
          case NPC_TYPE_TRAP:
            break;
          default:
            {
              return true;
            }
            break;
        }
      }
      break;
    default:
      break;
  }
  if (!entry) return false;

  zPos c(0,0);
  int x=0,y=0;
  AIC->getActRegion(c,x,y);
  /*
     if (getPetType()!=Cmd::PET_TYPE_NOTPET)
     {
     x += 10;
     y += 10;
     }
     else
     */
  {
    x = 7;//宠物就是7
    y = 7;
  }

  //被包围或无法走动
  if (!canMove() || isSurrounded())
  {
    c = getPos();
    x = 0;
    y = 0;
  }

  int atkRange = 0;
  BYTE atype=0,action=0;
  npc->getATypeAndAction(atype,action);
  switch(atype)
  {
    case NPC_ATYPE_NEAR:  /// 近距离攻击
    case NPC_ATYPE_MNEAR:  /// 法术近距离攻击
      atkRange = 1;
      break;
    case NPC_ATYPE_MFAR:  /// 法术远距离攻击
    case NPC_ATYPE_FAR:  /// 远距离攻击
      atkRange = 6;
      break;
    default:
      return false;
  }

  x += atkRange;
  y += atkRange;

  const zPos &pos = entry->getPos();
  return (pos.x>=c.x-x)
    &&(pos.x<=c.x+x)
    &&(pos.y>=c.y-y)
    &&(pos.y<=c.y+y);
}

/**
 * \brief 判断是否可以攻击该对象
 *
 *
 * \param entry 对手
 * \return 是否可以攻击
 */
bool SceneNpc::canAttack(SceneEntryPk *entry)
{
  //检查Npc是否可以攻击
  if (!checkAttackTime(SceneTimeTick::currentTime))// || !selectByPercent(50))
    return false;

  if (!attackAction) return false; //不攻击但是追踪
  return true;
}

/**
 * \brief Npc攻击玩家
 * \param entry 玩家
 * \return 攻击是否成功
 */
bool SceneNpc::attackTarget(SceneEntryPk *entry)
{
  if (this->assault)
  {
    this->skillStatusM.clearRecoveryElement(121);
  }
  if (!canAttack(entry)) return false;

  if (entry->getTopMaster() &&
      entry->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player &&
      this->getTopMaster() &&
      this->getTopMaster()->getType() == zSceneEntry::SceneEntry_Player)
  {
    if (!(this->isPkZone(entry) && entry->isPkZone(this))) // 新加&&this->isPkZone(pDef)
    {
      return false;
    }
  }

  BYTE atype = 0;
  BYTE action = 0;
  npc->getATypeAndAction(atype,action);

  //成功攻击用户
  if (entry && this->npc->kind != NPC_TYPE_SURFACE) 
  {
    setDir(getPos().getDirect(entry->getPos()));
#ifdef _DEBUG
    //if (getPetType()!=Cmd::PET_TYPE_NOTPET)
    //  Zebra::logger->debug("%s 设置方向 %s",name,entry->name);
#endif
  }

  using namespace Cmd;
  stAttackMagicUserCmd att;
  att.dwUserTempID = tempid;
  if (entry) att.dwDefenceTempID = entry->tempid;

  switch(atype)
  {
    case NPC_ATYPE_NEAR:  /// 近距离攻击
      {
        if (npc->skill)
          att.wdMagicType = npc->skill;
        else
		{
			if(npc->kind == NPC_TYPE_GHOST) //sky 元神特殊处理下
			{
				//sky 获取到主人的指针
				SceneUser *entry = (SceneUser *)getMaster();
				if(entry)
				{
					switch(entry->getWeaponType()) //sky 检测主人的武器类型
					{
					case ItemType_Blade:		// sky 104代表单手刀
					case ItemType_Sword:		// sky 105代表左手剑
						att.wdMagicType = SERVER_SKILL_ATTACK_NORMAL;
						break;
					case ItemType_Crossbow:	// sky 109代表法杖类武器
					case ItemType_Axe:		// sky 106代表双手类武器
						att.wdMagicType = SERVER_SKILL_HANDS_ATTACK_NORMAL;
						break;
					case ItemType_Hammer:		// sky 107代表匕首类武器
						att.wdMagicType = SERVER_SKILL_DAGGER_ATTACK_NORMAL;
						break;
					default:
						att.wdMagicType = SERVER_SKILL_ATTACK_NORMAL;
						break;
					}
				}
				else
					att.wdMagicType = SERVER_SKILL_ATTACK_NORMAL;
			}
			else
				att.wdMagicType = SERVER_SKILL_ATTACK_NORMAL;
		}

        att.byAction = Ani_Attack;
      }
      break;
    case NPC_ATYPE_FAR:    /// 远距离攻击
      {
        if (npc->skill)
          att.wdMagicType = npc->skill;
        else
          att.wdMagicType = SERVER_SKILL_DART_ATTACK_NORMAL;
        //att.wdMagicType = SKILLNORMAL;
        att.byAction = Ani_Attack;
        //att.byAction = Ani_Attack;
      }
      break;
    case NPC_ATYPE_MFAR:
    case NPC_ATYPE_MNEAR:
      {
        if (!this->skillAction) return false;
        att.wdMagicType = npc->skill;
        att.byAction = 0;
      }
      break;
    default:
      break;
  }
  //att.byAction = action;
  /*
  */

  if (entry)
  {
    att.xDes = (WORD)entry->getPos().x;
    att.yDes = (WORD)entry->getPos().y;
    att.byDirect = getDir();
    switch (entry->getType())
    {
      case zSceneEntry::SceneEntry_Player:
        {
			if( npc->kind == NPC_TYPE_GHOST )  //sky元神特殊处理
				att.byAttackType = ATTACKTYPE_U2U;
			else
				att.byAttackType = ATTACKTYPE_N2U;
        }
        break;
      case zSceneEntry::SceneEntry_NPC:
        {
			if( npc->kind == NPC_TYPE_GHOST )
				att.byAttackType = ATTACKTYPE_U2N;
			else
				att.byAttackType = ATTACKTYPE_N2N;
        }
        break;
      default:
        {
			if( npc->kind == NPC_TYPE_GHOST )
				att.byAttackType = ATTACKTYPE_U2U;
			else
				att.byAttackType = ATTACKTYPE_N2U;
        }
        break;
    }

  }
  else
  {
    att.xDes = (WORD)getPos().x;
    att.yDes = (WORD)getPos().y;
    att.byDirect = getDir();
  }
  //scene->sendCmdToNine(getPosI(),&att,sizeof(att),false);

  this->skillValue.init();
  this->skillStatusM.processPassiveness();// 处理我的被动状态影响

  //if (selectByPercent(this->npc->rating))
  {
    //ScenePk::physicalMagicN2U(this,sceneUser);

    switch(atype)
    {
      case NPC_ATYPE_NEAR:  /// 近距离攻击
      case NPC_ATYPE_FAR:    /// 远距离攻击
        {
          if (entry)
          {
            if (this->checkMagicFlyRoute(entry,(atype==2||atype==3)?AttackFly:AttackNear))
            {
              if (entry->preAttackMe(this,&att))
              {
                scene->sendCmdToNine(getPosI(),&att,sizeof(att),this->dupIndex);
                entry->AttackMe(this,&att);
              }
            }
          }
        }
        break;
      case NPC_ATYPE_MFAR:
      case NPC_ATYPE_MNEAR:
        {
          //走魔法攻击
          if (entry)
          {
            if (this->checkMagicFlyRoute(entry,(atype==2||atype==3)?AttackFly:AttackNear))
            {
              if (entry->preAttackMe(this,&att,false))
              {
                scene->sendCmdToNine(getPosI(),&att,sizeof(att),this->dupIndex);
                entry->AttackMe(this,&att,false);
              }
            }
            /*
               else
               {
               if (getPetType()==Cmd::PET_TYPE_NOTPET)
               scene->sendCmdToNine(getPosI(),&att,sizeof(att),false);
               }
               */
          }

          /*
             zSkill *s = this->usm.findSkill(att.wdMagicType);
             if (s)
             {
             if (skillAction)
             {
             if (s->action(&att))
             {
             break;
             }
             }
             }
             if (entry)
             {
             entry->preAttackMe(this,NULL);
             entry->AttackMe(this,NULL);
             }
             */
          //sceneUser->preAttackMe(this,NULL,false,npc->five);
          //sceneUser->AttackMe(this,NULL,false);
        }
        break;
      default:
        break;
    }

    if (entry)
    {
      if (this->giddy>0) //怒吼训练处理
      {
        if (selectByPercent(giddy))
        {
          Cmd::stAttackMagicUserCmd cmd;

          if (entry->getType() == zSceneEntry::SceneEntry_Player)
          {
            cmd.dwDefenceTempID = entry->tempid;
            cmd.byAttackType = Cmd::ATTACKTYPE_N2U;
            cmd.byAction = Cmd::Ani_Null;
          }
          else
          {
            cmd.dwDefenceTempID = entry->tempid;
            cmd.byAttackType = Cmd::ATTACKTYPE_N2N;
            cmd.byAction = Cmd::Ani_Null;
          }

          cmd.dwUserTempID = this->tempid;
          cmd.wdMagicType = 386;
          cmd.byDirect = this->getDir();

          zSkill *s = NULL;

          s = zSkill::createTempSkill(this,386,1);
          if (s)
          {
            s->action(&cmd,sizeof(cmd));
            SAFE_DELETE(s);
          }
        }
      }



      if (entry->getType() == zSceneEntry::SceneEntry_Player) ((SceneUser *)entry)->mask.on_defence();
    }
  }

  SceneEntryPk *pMaster = this->getTopMaster();
  if (pMaster&&pMaster->getType() == zSceneEntry::SceneEntry_Player)
  {
    SceneUser *pUser = (SceneUser *)pMaster;
    pUser->packs.equip.costAttackDurByPet(pUser);
  }

  //设置下一次攻击的时间
  setAttackTime(SceneTimeTick::currentTime);
  if (nextMoveTime>SceneTimeTick::currentTime)
    delayMoveTime(720);
  else
    setMoveTime(SceneTimeTick::currentTime,720);
  setEndBattleTime(SceneTimeTick::currentTime,10*1000);

  return true;
}

bool is_boss(int type)
{
  switch (type)
  {
    case NPC_TYPE_BACKBONE:
    case NPC_TYPE_BBOSS:
    case NPC_TYPE_LBOSS:
    case NPC_TYPE_GOLD:
    case NPC_TYPE_PBOSS:
    case NPC_TYPE_DUCKHIT:
    case NPC_TYPE_RESOURCE:
      return true;
      break;  
    default:
      break;
  }
  return false;
}

//sky 判断杀死怪物的角色和NPC之间的等级差避免高级玩家刷低级怪
int drop_odds(WORD player_level,WORD npc_level,DWORD npc_kind)
{
  int odds = 100;

  if (is_boss(npc_kind))
  {
    if (player_level>npc_level+20)
    {
      DWORD sub = (player_level-(npc_level+20))*10;
      odds = (sub>=100)?0:(100-sub);
    }
  }
  else
  {
    int diff = player_level - npc_level;
    if (diff >= 6 && diff < 10)
    {
      odds = 50;
    }
    else if (diff >= 10)
    {
      odds = 0;
    }
  }
  if (odds>100) odds=100;
  if (npc_kind==NPC_TYPE_DUCKHIT || npc_kind==NPC_TYPE_RESOURCE) odds = 100;
#ifdef _DEBUG
  Zebra::logger->debug("爆率损耗 %u",100-odds);
#endif
  return odds;
}

/**
* \brief 死亡Npc的动作
 * \return 动作执行是否成功
 */
bool SceneNpc::deathAction()
{
	Zebra::logger->debug("user kill a NPC");
	if (canLostObject(SceneTimeTick::currentTime))
	{
		//Zebra::logger->debug("%s 死亡",name);
		int value=1;
		int value1=0;
		int value2=0;
		int player_level = 0;
		int vcharm = 0;
		int vlucky = 0;
		SceneUser *pUser = SceneUserManager::getMe().getUserByID(dwNpcLockedUser);
		if (pUser)
		{

			TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);

			if (teamMan) 
			{
				Team& team = const_cast<Team&>(teamMan->getTeam());

				team.rwlock.rdlock();
				std::vector<TeamMember>::iterator it = team.member.begin();;
				for(; it!=team.member.end(); ++it) 
				{
					SceneUser* member = SceneUserManager::getMe().getUserByTempID(it->tempid);
					if (member && (member->charbase.level < 20 || 
						scene->checkTwoPosIInNine(pUser->getPosI(),member->getPosI()) ||
						scene->checkTwoPosIInNine(this->getPosI(),member->getPosI()) )) {
							//moved from attackDeathNpc by lqy,avoid unfair bonus
							OnKill event(id) ;
							EventTable::instance().execute(*member,event);
							OnKillByLevel e(npc->level);
							EventTable::instance().execute(*member,e);
							if (dwNpcLockedUser == member->id)
							{
								OnKillBySelf es(id);
								EventTable::instance().execute(*member,es);
							}

							if (ScriptQuest::get_instance().has(ScriptQuest::NPC_KILL,npc->id)) { 
								char func_name[32];
								sprintf(func_name,"%s_%d","kill",npc->id);
								execute_script_event(member,func_name,this);
							}
					}
				}
				team.rwlock.unlock();
			}
			else 
			{
				OnKill event(id) ;
				EventTable::instance().execute(*pUser,event);
				OnKillByLevel e(npc->level);
				EventTable::instance().execute(*pUser,e);
				OnKillBySelf es(id);
				EventTable::instance().execute(*pUser,es);

				if (ScriptQuest::get_instance().has(ScriptQuest::NPC_KILL,npc->id)) { 
					char func_name[32];
					sprintf(func_name,"%s_%d","kill",npc->id);
					execute_script_event(pUser,func_name,this);
				}
			}

			value1=pUser->packs.equip.getEquips().get_mf();
			value2=pUser->packs.equip.getEquips().get_incgold();
			vcharm = pUser->charstate.charm;
			//vlucky = pUser->charstate.lucky;
			player_level = pUser->charbase.level;
		}

		NpcLostObject nlo;
		if (npc->kind == NPC_TYPE_BBOSS && selectByPercent(50))
		{
			npc->nco.lostGreen(nlo,value,value1,value2,vcharm,vlucky);
		}
		else if (selectByOneHM(100))
		{
			npc->nco.lostAll(nlo);
		}
		else
		{
			npc->nco.lost(nlo,value,value1,value2,vcharm,vlucky,player_level,ScenesService::getInstance().DropRate,ScenesService::getInstance().DropRateLevel);
		}

		for(NpcLostObject::const_iterator it = nlo.begin(); it != nlo.end(); it++)
		{
			/*if (!((npc->id>=21501 && npc->id<=21511) ||
				(npc->id>=26501 && npc->id<=26511) ||
				(npc->id>=30051 && npc->id<=30102)))
			{*/
			if (!selectByPercent(drop_odds(player_level,npc->level,npc->kind))) continue;
			/*}*/

			zObjectB *ob = objectbm.get((*it).id);
			if (ob)
			{
				int num = (*it).minnum;
				if ((*it).minnum != (*it).maxnum)
					num = randBetween((*it).minnum,(*it).maxnum);
				if (num > 0)
				{
					if (ob->id == 665)
					{
						//分配银子
						distributeMoney(num);
					}
					else
					{
						SceneUser *pUser = SceneUserManager::getMe().getUserByID(dwNpcLockedUser);
						if (pUser && (pUser->TeamThisID != 0) && npc->kind != NPC_TYPE_RESOURCE)
						{
							TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);

							if (teamMan && teamMan->isSpecialObj())
							{
								//sky最后的参数换成极品倍率来适应新的极品算法
								scene->addObject(dupIndex,ob,num,getPos(),teamMan->getNextObjOwnerID(),npc->Need_Probability, teamMan->getTeamtempId() );
							}
							else if(teamMan && teamMan->isCaptainObj())
							{
								zObject *o = zObject::create(ob,num);
								if (o)
								{
									Captain_ID = teamMan->getLeader();

									EquipMaker maker(NULL);
									maker.NewAssign(NULL,o,o->base,npc->Need_Probability);

									//sky 判断物品如果是非极品的话就直接掉在地上按轮流拾取模式分配
									if( o->data.kind != 1 &&
										o->data.kind != 2 &&
										o->data.kind != 4 &&
										o->data.kind != 8 )
									{
										scene->addObject(dupIndex,o,pos,0,teamMan->getNextObjOwnerID(),30);
									}
									else
										m_TemObj.push_back( o );
								}
							}
							else
							{
								//sky最后的参数换成极品倍率来适应新的极品算法
								scene->addObject(dupIndex,ob,num,getPos(),dwNpcLockedUser,npc->Need_Probability);
							}
						}
						else
						{
							if (this->npc->kind == NPC_TYPE_RESOURCE && pUser)
							{
								zObject *o = zObject::create(ob,num);
								if (o)
								{
									DWORD addnum=o->data.dwNum;
									Combination callback(pUser,o);
									pUser->packs.main.execEvery(callback);
									if (pUser->packs.equip.pack(EquipPack::L_PACK) && pUser->packs.equip.pack(EquipPack::L_PACK)->can_input()) pUser->packs.equip.pack(EquipPack::L_PACK)->execEvery(callback);
									if (pUser->packs.equip.pack(EquipPack::R_PACK) && pUser->packs.equip.pack(EquipPack::R_PACK)->can_input()) pUser->packs.equip.pack(EquipPack::R_PACK)->execEvery(callback);

									bool added = false;
									if (o->data.dwNum)
									{
										if (pUser->packs.uom.space(pUser) >= 1 && pUser->packs.addObject(o,true,AUTO_PACK))
										{
											//如果是双倍经验道具和荣誉道具需要绑定
											if (o->base->kind == ItemType_DoubleExp || o->base->kind == ItemType_Honor || o->base->kind == ItemType_ClearProperty)
											{
												o->data.bind=1;
											}
											added = true;
											Cmd::stAddObjectPropertyUserCmd status;
											status.byActionType = Cmd::EQUIPACTION_OBTAIN;
											bcopy(&o->data,&status.object,sizeof(t_Object),sizeof(status.object));
											pUser->sendCmdToMe(&status,sizeof(status));
										}
										else
										{
											addnum -= o->data.dwNum;
											Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"你的包裹已满");
											//sky已经没有灵魂石的概念拉把最后个参数转变成极品倍率用来计算极品
											scene->addObject(dupIndex,ob,o->data.dwNum,getPos(),dwNpcLockedUser,npc->Need_Probability);



											if (callback.num() || added)
											{
												zObject::logger(o->createid,o->data.qwThisID,o->data.strName,addnum,addnum,1,this->scene->id,pUser->scene->name,pUser->id,pUser->name,"采集得到",o->base,o->data.kind,o->data.upgrade);
												OnGet event(o->data.dwObjectID);
												EventTable::instance().execute(*pUser,event);
												if (ScriptQuest::get_instance().has(ScriptQuest::OBJ_GET,o->data.dwObjectID))
												{ 
													char func_name[32];
													sprintf(func_name,"%s_%d","get",o->data.dwObjectID);
													execute_script_event(pUser,func_name,o);
												}                
											}
										}
									}
									if (!o->data.dwNum || !added)
									{
										zObject::destroy(o);
									}
								}
							}
							else
							{
								//sky已经没有灵魂石的概念拉把最后个参数转变成极品倍率用来计算极品
								scene->addObject(dupIndex,ob,num,getPos(),dwNpcLockedUser,npc->Need_Probability);
							}
						}
					}
				}
			}
		}

		//触发攻城
		if (canRush())
			createRush();

	}
	else if (canRelive(SceneTimeTick::currentTime))
	{
		zPos pos;

		//sky 如果是动态NPC就设置清除标准
		if (getSceneNpcType() == SceneNpc::GANG)
		{
			if (!isRushNpc)
				setClearState();
		}
		else if (scene->randPosByRegion(define->region.index,pos))
		{
			//重生
			isUse = false; // 把尸体使用状态洗掉。
			m_TemObj.clear(); //sky将残留的掉落物品清除掉
			clearUState(Cmd::USTATE_DEATH);
			Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
			removeNpc.dwMapNpcDataPosition = tempid;
			scene->sendCmdToNine(getPosI(),&removeNpc,sizeof(removeNpc),this->dupIndex);

			hp = this->getMaxHP();
			recoverUnder30 = false;
			//从头开始按照路线移动
			if (AIC) AIC->on_relive();

			if (define->initstate == zSceneEntry::SceneEntry_Normal)
			{
				//查找非阻挡点成功
				scene->setBlock(pos);
				setState(zSceneEntry::SceneEntry_Normal);
			}
			else
				setState(zSceneEntry::SceneEntry_Hide);

			if (scene->refresh(this,pos))
			{
				if (getState() == zSceneEntry::SceneEntry_Normal)
				{
					this->sendMeToNine();
				}
				//召唤npc的宠物
				if (!define->petList.empty())
				{
					for (std::map<DWORD,std::pair<DWORD,DWORD> >::const_iterator it=define->petList.begin(); it!=define->petList.end(); it++)
					{
						for (DWORD i=0; i<it->second.first; i++)
						{
							if (selectByPercent(it->second.second))
							{
								ScenePet * pet = summonPet(it->first,Cmd::PET_TYPE_TOTEM,0,0,0,0);
								if (pet)
									pet->setPetAI(Cmd::PETAI_ATK_ACTIVE);
							}
						}
					}
				}
				//半宠物
				if (!define->dieList.empty())
				{
					for (std::list< std::pair<DWORD,zPos> >::const_iterator it=define->dieList.begin(); it!=define->dieList.end(); it++)
					{
						ScenePet * pet = summonPet(it->first,Cmd::PET_TYPE_SEMI,0,0,0,0,it->second);
						if (pet)
						{
							pet->setPetAI(Cmd::PETAI_ATK_ACTIVE);
							Zebra::logger->debug("%s 召唤宠物 %s",name,pet->name);
						}
					}
				}

				if (npc->kind==NPC_TYPE_BBOSS || npc->kind==NPC_TYPE_LBOSS || npc->kind==NPC_TYPE_PBOSS)
					Zebra::logger->debug("[BOSS]%s(%u) 复活 pos=(%u,%u)",name,tempid,pos.x,pos.y);
			}
			else
			{
				setState(zSceneEntry::SceneEntry_Death);
				Zebra::logger->debug("[BOSS]%s(%u) 复活失败 pos=(%u,%u)",name,tempid,pos.x,pos.y);
			}

			if (this->id == COUNTRY_MAIN_FLAG)
			{
				CountryDareM::getMe().reliveSecondFlag(this->scene,COUNTRY_SEC_FLAG);
			}

			if (this->id == COUNTRY_KING_MAIN_FLAG)
			{
				CountryDareM::getMe().reliveSecondFlag(this->scene,COUNTRY_KING_SEC_FLAG);
			}


		}
	}

	return true;
}

/*
 * \brief 隐藏npc
 *
 * \showDelay 隐藏的时间
 */
void SceneNpc::hideMe(int showDelay)
{
    Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
    removeNpc.dwMapNpcDataPosition = tempid;
    scene->sendCmdToNine(getPosI(),&removeNpc,sizeof(removeNpc),this->dupIndex);

    setState(zSceneEntry::SceneEntry_Hide);
    showTime = SceneTimeTick::currentTime;
    showTime.addDelay(showDelay);
#ifdef _DEBUG
    Zebra::logger->debug("npc %s 隐藏%d毫秒",npc->name,showDelay);
#endif
}

/*
 * \brief npc隐身状态的行为
 *
 * \return 是否执行成功
 */
bool SceneNpc::hideAction()
{
    if (showTime<=SceneTimeTick::currentTime)
  {
    setState(zSceneEntry::SceneEntry_Normal);
    scene->setBlock(getPos());
    sendMeToNine();
#ifdef _DEBUG
    Zebra::logger->debug("npc %s 结束隐身",name);
#endif
  }
    return true;
}

/**
 * \brief 寻路过程中判断中间点是否可达目的地
 * \param tempPos 寻路过程的中间点
 * \param destPos 目的点坐标
 * \param radius 寻路范围,超出范围的视为目的地不可达
 * \return 返回是否可到达目的地
 */
bool SceneNpc::moveable(const zPos &tempPos,const zPos &destPos,const int radius)
{
  return (scene->zPosShortRange(tempPos,destPos,radius)
      && (!scene->checkBlock(tempPos) //目标点可达,或者是最终目标点
        || tempPos == destPos));
}

/**
 * \brief Npc向某一个方向移动
 * \param direct 方向
 * \param step 移动的步长
 * \return 移动是否成功
 */
bool SceneNpc::move(const int direct,const int step)
{
  if (!canMove()) return false;
  //if ((aif&AIF_NO_MOVE)||(petAI&Cmd::PETAI_MOVE_STAND)) return false;

  const int walk_adjust[9][2]= { {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,0} };

  if (backOffing)
  {
    return true;
  }
  //Zebra::logger->debug("移动步长：%u",step);
  lastPos2 = lastPos1;
  lastPos1 = pos;
  zPosI oldPosI = getPosI();
  zPos oldPos = getPos(),newPos = getPos();
  newPos.x += (step*walk_adjust[direct][0]);
  newPos.y += (step*walk_adjust[direct][1]);

  if( (dread || blind) && (RandPos.x!=0 || RandPos.y!=0) )
  {
	  //sky 超过设置的范围就返回
	  if(!outOfRandRegion(&newPos))
		  return false;
  }
  
  //检查目标点阻挡以及是否合法
  if (scene->checkBlock(newPos))
  {
    //Zebra::logger->debug("%u,%u,%s,%u,%u,%u",id,tempid,name,direct,getPos().x,getPos().y);
    return false;
  }
  if (scene->refresh(this,newPos))
  {
    if (getPetType()!=Cmd::PET_TYPE_CARTOON && getPetType()!=Cmd::PET_TYPE_RIDE)
      scene->setBlock(newPos);
    scene->clearBlock(oldPos);
    setDir(direct);

	if( npc->kind == NPC_TYPE_GHOST )  //sky元神特殊处理
	{
		Cmd::stUserMoveMoveUserCmd cmd;
		cmd.dwUserTempID = tempid;
		cmd.byDirect = direct;
		cmd.bySpeed = 2;

		cmd.x = newPos.x;
		cmd.y = newPos.y;
		scene->sendCmdToNine(oldPosI,&cmd,sizeof(cmd),this->dupIndex);
	}
	else
	{
		Cmd::stNpcMoveMoveUserCmd cmd;
		cmd.dwNpcTempID = tempid;
		cmd.byDirect = direct;
		cmd.bySpeed = step;
		if (Cmd::PET_TYPE_RIDE==getPetType()&&speedRate>1.5)
		{
			cmd.bySpeed = 3;
#ifdef _DEBUG
			//Zebra::logger->debug("%s 移动步长 2",name);
#endif
		}
		cmd.x = newPos.x;
		cmd.y = newPos.y;
		scene->sendCmdToNine(oldPosI,&cmd,sizeof(cmd),this->dupIndex);
	}

    if (oldPosI != getPosI())
    {
		if( npc->kind == NPC_TYPE_GHOST )		//sky元神特殊处理
		{
			SceneUser *entry = (SceneUser *)getMaster();
			Cmd::stRemoveUserMapScreenUserCmd remove;
			remove.dwUserTempID = tempid;
			scene->sendCmdToReverseDirect(oldPosI,
				scene->getScreenDirect(oldPosI,getPosI()),&remove,sizeof(remove),this->dupIndex);

			BUFFER_CMD(Cmd::stAddUserAndPosMapScreenStateUserCmd,send,zSocket::MAX_USERDATASIZE);
			((SceneGhost *)this)->full_t_MapUserDataPosState( send->data, entry );
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),send,send->size(),this->dupIndex);
			Cmd::stRTMagicPosUserCmd ret;
			full_stRTMagicPosUserCmd(ret);
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),&ret,sizeof(ret),this->dupIndex);

			attackRTHpAndMp();
		}
		else
		{
			Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
			removeNpc.dwMapNpcDataPosition = tempid;
			scene->sendCmdToReverseDirect(oldPosI,
				scene->getScreenDirect(oldPosI,getPosI()),&removeNpc,sizeof(removeNpc),this->dupIndex);
			BUFFER_CMD(Cmd::stAddMapNpcMapScreenStateUserCmd,send,zSocket::MAX_USERDATASIZE);
			this->full_t_MapNpcDataState(send->data);
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),send,send->size(),this->dupIndex);
			Cmd::stRTMagicPosUserCmd ret;
			full_stRTMagicPosUserCmd(ret);
			scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),&ret,sizeof(ret),this->dupIndex);
			attackRTHpAndMp();
		}
    }
  }

  //设置下一次移动时间
  setMoveTime(SceneTimeTick::currentTime);

  if(!dread && !blind)
  {
	  //对下次攻击时间造成的影响：普通怪移动时间； 宠物：移动时间的一半
	  if (getPetType()==Cmd::PET_TYPE_NOTPET || getPetType()==Cmd::PET_TYPE_SEMI)
	  {
		  if (nextAttackTime>SceneTimeTick::currentTime)
			  nextAttackTime.addDelay((int)(npc->distance/speedRate));
		  else
			  setAttackTime(SceneTimeTick::currentTime);
	  }
	  else
	  {
		  if (nextAttackTime>SceneTimeTick::currentTime)
			  nextAttackTime.addDelay((int)(npc->distance/speedRate/2));
		  else
			  setAttackTime(SceneTimeTick::currentTime,(int)(npc->distance/speedRate/2));
	  }
  }
  
  return true;
}

/**
 * \brief 瞬间移动
 *
 *
 * \param newPos 目标位置
 * \return 是否成功
 */
bool SceneNpc::warp(const zPos &newPos,bool ignore)
{
    if (!ignore)
  if (!canMove()) return false;

    zPosI oldPosI = getPosI();
    zPos oldPos = getPos();

    zPos findedPos;
    bool founded = scene->findPosForUser(newPos,findedPos);
    if (scene->refresh(this,founded ? findedPos : newPos))
  {
    if (SceneEntry_Normal==getState() && getPetType()!=Cmd::PET_TYPE_CARTOON && getPetType()!=Cmd::PET_TYPE_RIDE)
      scene->setBlock(newPos);
    scene->clearBlock(oldPos);


    Cmd::stRTMagicPosUserCmd ret;
    full_stRTMagicPosUserCmd(ret);
    if (oldPosI != getPosI())
    {
		if( npc->kind == NPC_TYPE_GHOST )		//sky元神特殊处理
		{
			SceneUser *entry = (SceneUser *)getMaster();
			Cmd::stRemoveUserMapScreenUserCmd remove;
			remove.dwUserTempID = tempid;
			Cmd::stAddUserAndPosMapScreenStateUserCmd send;
			((SceneGhost *)this)->full_t_MapUserDataPosState( send.data, entry );
			if (scene->checkTwoPosIInNine(oldPosI,getPosI()))
			{
				scene->sendCmdToReverseDirect(oldPosI,
					scene->getScreenDirect(oldPosI,getPosI()),&remove,sizeof(remove),this->dupIndex);
				scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),&send,sizeof(send),this->dupIndex);
			}
			else
			{
				scene->sendCmdToNine(oldPosI,&remove,sizeof(remove),this->dupIndex);
				sendMeToNine();
			}
		}
		else
		{
			Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
			removeNpc.dwMapNpcDataPosition = tempid;
			Cmd::stAddMapNpcMapScreenUserCmd addNpc;
			full_t_MapNpcData(addNpc.data);
			if (scene->checkTwoPosIInNine(oldPosI,getPosI()))
			{
				scene->sendCmdToReverseDirect(oldPosI,scene->getScreenDirect(oldPosI,getPosI()),&removeNpc,sizeof(removeNpc),this->dupIndex);
				scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),&addNpc,sizeof(addNpc),this->dupIndex);
				//scene->sendCmdToDirect(getPosI(),scene->getScreenDirect(oldPosI,getPosI()),&ret,sizeof(ret));
			}
			else
			{
				scene->sendCmdToNine(oldPosI,&removeNpc,sizeof(removeNpc),this->dupIndex);
				sendMeToNine();
			}
		}
    }
    scene->sendCmdToNine(getPosI(),&ret,sizeof(ret),this->dupIndex);
    //setMoveTime(SceneTimeTick::currentTime);
    return true;
  }
    else return false;
}

/**
 * \brief 使物件向某一个点移动
 * 带寻路算法的移动,不过这里只是封装了一下,由于不同的npc步长不同,在这里调用不同步长的A*算法
 * \param srcPos 起点坐标
 * \param destPos 目的地坐标
 * \return 移动是否成功
 */
bool SceneNpc::gotoFindPath(const zPos &srcPos,const zPos &destPos)
{
    if (id==27 || id==28 || id==29)
  //铁甲蛙一次跳两格,所以步长为2
  return zAStar<2>::gotoFindPath(srcPos,destPos);
    else
  //普通怪物一次只行走一步,所以步长为1
  return zAStar<>::gotoFindPath(srcPos,destPos);
}

/**
 * \brief Npc向某一个点移动
 * 不带寻路算法的移动,不过这里只是封装了一下,由于不同的npc步长不同,在这里调用不同步长的A*算法
 * \param pos 目的地坐标
 * \return 移动是否成功
 */
bool SceneNpc::goTo(const zPos &pos)
{
    if (id==27 || id==28 || id==29)
  //铁甲蛙一次跳两格,所以步长为2
  return zAStar<2>::goTo(getPos(),pos);
    else
  //普通怪物一次只行走一步,所以步长为1
  return zAStar<>::goTo(getPos(),pos);
}

/**
 * \brief Npc随机向某一个方向移动
 * 不过这里只是封装了一下,由于不同的npc步长不同,在这里调用不同步长的A*算法
 * \param direct 随机方向
 * \return 移动是否成功
 */
bool SceneNpc::shiftMove(const int direct)
{
    if (id==27 || id==28 || id==29)
  //铁甲蛙一次跳两格,所以步长为2
  return zAStar<2>::shiftMove(direct);
    else
  //普通怪物一次只行走一步,所以步长为1
  return zAStar<>::shiftMove(direct);
}

void SceneNpc::clearStateToNine(WORD state)
{
  if (!scene) return;

  if( npc->kind == NPC_TYPE_GHOST )
  {

	  Cmd::stClearStateMapScreenUserCmd send;
	  send.type=Cmd::MAPDATATYPE_USER;
	  send.dwTempID = this->tempid;
	  send.wdState =state;

	  scene->sendCmdToNine(getPosI(),&send,sizeof(send),this->dupIndex);
  }
  else
  {
	  Cmd::stClearStateMapScreenUserCmd send;
	  send.type=Cmd::MAPDATATYPE_NPC;
	  send.dwTempID = this->tempid;
	  send.wdState =state;
	  if (npc->kind == NPC_TYPE_TRAP)
	  {
		  SceneEntryPk *entry = getMaster();
		  if (entry && entry->getType() == zSceneEntry::SceneEntry_Player)
		  {
			  ((SceneUser*)entry)->sendCmdToMe(&send,sizeof(send));
		  }    
	  }
	  else
	  {
		  this->scene->sendCmdToNine(getPosI(),&send,sizeof(send),this->dupIndex);
	  }
  }
}
/**
 * \brief 设置某个状态给用户
 *
 */

void SceneNpc::setStateToNine(WORD state)
{
	if( npc->kind == NPC_TYPE_GHOST )
	{

		Cmd::stSetStateMapScreenUserCmd send;
		send.type=Cmd::MAPDATATYPE_USER;
		send.dwTempID = this->tempid;
		send.wdState =state;

		this->scene->sendCmdToNine(getPosI(),&send,sizeof(send),this->dupIndex);
	}
	else
	{
		Cmd::stSetStateMapScreenUserCmd send;
		send.type=Cmd::MAPDATATYPE_NPC;
		send.dwTempID = this->tempid;
		send.wdState =state;
		if (npc->kind == NPC_TYPE_TRAP)
		{
			SceneEntryPk *entry = getMaster();
			if (entry && entry->getType() == zSceneEntry::SceneEntry_Player)
			{
				((SceneUser*)entry)->sendCmdToMe(&send,sizeof(send));
			}    
		}
		else
		{
			this->scene->sendCmdToNine(getPosI(),&send,sizeof(send),this->dupIndex);
		}
	}
}
/**
 * \brief 通知客户端显示状态特效
 * \param state 状态特效编号
 * \param isShow 出现还是消失
 */
void SceneNpc::showCurrentEffect(const WORD &state,bool isShow,bool notify)
{
  if (isShow)
  {
    if (this->setUState(state) && notify)
      this->setStateToNine(state);
  }
  else
  {
    if (this->clearUState(state) && notify)
      this->clearStateToNine(state);
  }
}

/**
 * \brief 角色被击退N格
 * \param dwAttTempID 攻击者的临时ID
 * \param grids
 */
void SceneNpc::standBack(const DWORD dwAttTempID,DWORD grids)
{
  SceneUser *att = SceneUserManager::getMe().getUserByTempID(dwAttTempID);
  if (att)
  {
    backoff(Scene::getCompDir(att->getPos(),this->pos),grids);
  }
  else
  {
    backoff(getDir(),grids);
  }
}

/**
 * \brief 将攻击目标换成dwTempID所指向的角色玩家
 * \param dwTempID 目标角色的临时ID
 */
void SceneNpc::changeAttackTarget(const DWORD &dwTempID)
{
  SceneUser *targetUser = SceneUserManager::getMe().getUserByTempID(dwTempID);
  if (targetUser) 
  {
    if (canChaseTarget(targetUser))
    {
      unChaseUser();
      chaseSceneEntry(targetUser->getType(),targetUser->tempid);
    }
  }
}

/**
 * \brief 让角色死亡
 */
void SceneNpc::toDie(const DWORD &dwTempID)
{
}

/**
 * \brief 判断角色是否死亡
 * \return true为死亡
 */
bool SceneNpc::isDie()
{
    if (this->getState() == SceneEntry_Death) return true;
    return false;
}


/**
 * \brief 得到npc的等级
 *
 *
 * \return 等级
 */
DWORD SceneNpc::getLevel() const
{
    return npc->level;
}

/**
 * \brief 需要的职业类型,决定可以使用的技能类型
 */
bool SceneNpc::needType(const DWORD &needtype)
{
    return true;
}

/**
 * \brief 需要的职业类型,决定可以使用的技能类型
 */
bool SceneNpc::addSkillToMe(zSkill *skill)
{
    return usm.addSkill(skill);;
}

/**
 * \brief 是否有该技能需要的武器
 * \return true 有 false 没有
 */
bool SceneNpc::needWeapon(DWORD skillid)
{
    return true;
}

/**
 * \brief 是否Pk区域
 * \param other PK相关人
 * \return true 是 false 否
 */
bool SceneNpc::isPkZone(SceneEntryPk *other)
{
    return true;
}

/**
 * \brief 依赖物品消耗型法术
 * \param object 消耗物品的类型
 * \param num 消耗物品的数量
 * \return true 消耗成功 false 失败
 */
bool SceneNpc::reduce(const DWORD &object,const BYTE num)
{
    return true;
}

/**
 * \brief 检查可消耗物品是否足够
 * \param object 消耗物品的类型
 * \param num 消耗物品的数量
 * \return true 足够 false 不够
 */
bool SceneNpc::checkReduce(const DWORD &object,const BYTE num)
{
    return true;
}

/**
 * \brief 施放技能所导致的消耗MP,HP,SP
 * \param base 技能基本属性对象
 * \return true 消耗成功 false 失败
 */
bool SceneNpc::doSkillCost(const zSkillB *base)
{
    return true;
}

/**
 * \brief 检查施放技能所导致的消耗MP,HP,SP是否足够
 * \param base 技能基本属性对象
 * \return true 消耗成功 false 失败
 */
bool SceneNpc::checkSkillCost(const zSkillB *base)
{
    return true;
}

/**
 * \brief 检查自身的施放成功几率,决定这次技能是否可以施放
 * \return true 成功 false 失败
 */
bool SceneNpc::checkPercent()
{
    return true;
}

/**
 * \brief 改变角色的hp
 * \param hp 变更的HP
 */
SWORD SceneNpc::directDamage(SceneEntryPk *pAtt,const SDWORD &dam,bool notify)
{

	SDWORD attHp = 0;

	attHp = SceneEntryPk::directDamage(pAtt,dam,notify);

	SDWORD reduceHP=0;
	if ((SDWORD)hp - attHp>=0)
	{
		hp -= attHp;
		reduceHP = attHp;
	}
	else
	{
		reduceHP = hp;
		hp=0;
	}

	if (reduceHP !=0 && notify)
	{
		if( npc->kind == NPC_TYPE_GHOST )
		{
			Cmd::stObjectHpMpPopUserCmd ret;
			ret.dwUserTempID = this->tempid;
			ret.byTarget = Cmd::MAPDATATYPE_USER;
			ret.vChange = 0-(int)reduceHP;
			ret.type = Cmd::POP_HP;
			this->scene->sendCmdToNine(getPosI(),&ret,sizeof(ret),this->dupIndex);
		}
		else
		{
			Cmd::stObjectHpMpPopUserCmd ret;
			ret.dwUserTempID = this->tempid;
			ret.byTarget = Cmd::MAPDATATYPE_NPC;
			ret.vChange = 0-(int)reduceHP;
			ret.type = Cmd::POP_HP;
			this->scene->sendCmdToNine(getPosI(),&ret,sizeof(ret),this->dupIndex);
		}
	}


	SceneEntryPk *pEntry = pAtt->getMaster();
	if (pEntry&&(pEntry->getType() == zSceneEntry::SceneEntry_Player))
	{
		SceneUser * master = (SceneUser *)pEntry;
		if (master)
		{
			this->reduceHP(master,reduceHP);
			this->setLockUser(master->id);
		}
	}
	return reduceHP;
}

/**
 * \brief 处理npc死亡
 *
 *
 * \param pAtt 凶手
 * \return 是否死亡
 */
bool SceneNpc::processDeath(SceneEntryPk *pAtt)
{
  SceneEntryPk *mymaster=NULL;

  if (hp ==0 && getState()!=zSceneEntry::SceneEntry_Death)
  {
    SceneEntryPk *temp = NULL;
    DWORD attCountryID = 0;
    DWORD attUnionID   = 0;
    DWORD dwAttUserID = 0;
    bool isAntiAtt = false;

	//[sky]NPC死亡把AI相关的数据清除
	m_dwNowTime = 0;
	m_mobType	  = false;
	m_bFighting = false;
	m_dwGtime = 0;

	Zebra::logger->debug("%s 死亡！初始化AI定时系统", name );

    // 如果召唤
    mymaster = this->getTopMaster();
    if (mymaster->summon != this) mymaster = NULL;
    switch(pAtt->getType())
    {
      case zSceneEntry::SceneEntry_NPC:
        {
          temp = pAtt->getTopMaster();
          if (!temp) temp = pAtt;

          if (((SceneUser*)temp)->getType() == zSceneEntry::SceneEntry_Player)
          {
            attUnionID   = ((SceneUser*)temp)->charbase.unionid;
            attCountryID = ((SceneUser*)temp)->charbase.country;
            dwAttUserID = ((SceneUser*)temp)->id;
            isAntiAtt = ((SceneUser*)temp)->isWarRecord(Cmd::COUNTRY_FORMAL_ANTI_DARE,
                this->scene->getCountryID());
          }
        }
        break;
      case zSceneEntry::SceneEntry_Player:
        {
          temp = pAtt;
          attCountryID = ((SceneUser*)temp)->charbase.country;
          attUnionID = ((SceneUser*)temp)->charbase.unionid;
          dwAttUserID = ((SceneUser*)temp)->id;

          isAntiAtt = ((SceneUser*)temp)->isWarRecord(Cmd::COUNTRY_FORMAL_ANTI_DARE,
                this->scene->getCountryID());

          switch ( npc->kind)
          {
            case NPC_TYPE_TRADE:
            case NPC_TYPE_TASK:
            case NPC_TYPE_LIVENPC:
            case NPC_TYPE_MAILBOX:
            case NPC_TYPE_AUCTION:
              if (attCountryID!=scene->getCountryID())
              {
                Cmd::Session::t_countryNotify_SceneSession send;
                send.infoType = Cmd::INFO_TYPE_EXP;
                send.dwCountryID = scene->getCountryID();
                _snprintf(send.info,MAX_CHATINFO,"%sNPC %s(%u,%u) 被 %s(%s) 杀死",scene->getRealName(),name,pos.x,pos.y,temp->name,SceneManager::getInstance().getCountryNameByCountryID(attCountryID));
                sessionClient->sendCmd(&send,sizeof(send));
              }
              break;
            default:
              break;
          }

          if (npc->id==30034)//杀死典狱官
          {
            SceneUser * u = (SceneUser *)pAtt;
            if (u->isRedNamed())
            {
              u->charbase.goodness = (DWORD)Cmd::GOODNESS_2_1;
              while (!u->pkState.cancelProtect(u));

              Channel::sendSys(u,Cmd::INFO_TYPE_EXP,"你杀死典狱官,清除了自己所有的犯罪记录");
              Zebra::logger->debug("[监狱]%s 杀死典狱官",u->name);
            }
          }
        }
        break;
      default:
        {
          temp = pAtt;
        }
        break;
    }

    resetSpeedRate();
    resetAspeedRate();

    if (AIC) AIC->on_die();

    if (this->id == COUNTRY_MAIN_FLAG || this->isMainGeneral() || this->id == COUNTRY_KING_MAIN_FLAG || this->id == COUNTRY_EMPEROR_MAIN_GEN)
    {
      Cmd::Session::t_countryDareResult_SceneSession send;
      send.dwAttCountryID = attCountryID;
      send.dwDefCountryID = this->scene->getCountryID();

      if (attCountryID>0)
      {
        strncpy(send.attCountryName,
            SceneManager::getInstance().getCountryNameByCountryID(attCountryID),
            sizeof(send.attCountryName));
      }
      else
      {
        bzero(send.attCountryName,sizeof(send.attCountryName));
      }

      strncpy(send.defCountryName,
          SceneManager::getInstance().getCountryNameByCountryID(this->scene->getCountryID()),
          sizeof(send.defCountryName));

      if (this->id == COUNTRY_MAIN_FLAG || this->id == COUNTRY_KING_MAIN_FLAG)
      {
        send.byType = Cmd::Session::COUNTRY_ANNOY_DARE;
        send.dwAttUserID = dwAttUserID;
      }
      else if (this->isMainGeneral())
      {
        if (isAntiAtt)
        {
          send.byType = Cmd::Session::COUNTRY_FORMAL_ANTI_DARE;
        }
        else
        {
          send.byType = Cmd::Session::COUNTRY_FORMAL_DARE;
        }

        send.dwAttUserID = dwAttUserID;
      }
      else if (this->id == COUNTRY_EMPEROR_MAIN_GEN)
      {
        send.byType = Cmd::Session::EMPEROR_DARE;
        send.dwAttUserID = dwAttUserID;
      }

      sessionClient->sendCmd(&send,sizeof(send));
    }

    if (this->id == COUNTRY_SEC_GEN)
    {// 通知守方,大将军将受到攻击
      Cmd::Session::t_countryNotify_SceneSession send;
      bzero(send.info,sizeof(send.info));
      sprintf(send.info,"%s","禁卫队长死亡,大将军将受到攻击!");
      send.dwCountryID = this->scene->getCountryID();
      sessionClient->sendCmd(&send,sizeof(send));
    }

    if (this->id == COUNTRY_SEC_FLAG)
    {// 通知守方,副旗受到攻击
      Cmd::Session::t_countryNotify_SceneSession send;
      bzero(send.info,sizeof(send.info));
      sprintf(send.info,"本国 %s 副旗(%u,%u)倒地",this->scene->getRealName(),this->getPos().x,this->getPos().y);
      send.dwCountryID = this->scene->getCountryID();
      sessionClient->sendCmd(&send,sizeof(send));
    }

    if (this->id == 58101 && attUnionID>0 && attCountryID > 0) //如果死亡的是城旗,则通知会话进行城主变换
    {
      Cmd::Session::t_UnionCity_DareResult_SceneSession send;
      send.dwUserID = pAtt->id;
      send.dwCountryID = this->scene->getCountryID();
      send.dwCityID = pAtt->scene->getRealMapID();
      send.dwUnionID = attUnionID;
      sessionClient->sendCmd(&send,sizeof(send));
    }

    ScenePk::attackDeathNpc(temp,this);
    if (this->summonsplit)
    {
      if (mymaster&&selectByPercent(this->summonsplit))
      {
        mymaster->summonPet(this->npc->id,Cmd::PET_TYPE_TOTEM,30,0,0,0);
        mymaster->summonPet(this->npc->id,Cmd::PET_TYPE_SUMMON,0,0,0,0);
      }
    }
    return true;
  }
  return false;
}

/**
 * \brief 设置任务状态
 *
 *
 * \param user 玩家
 */
void SceneNpc::set_quest_status(SceneUser* user)
{
  clearUState(Cmd::USTATE_START_QUEST);
  clearUState(Cmd::USTATE_DOING_QUEST);  
  clearUState(Cmd::USTATE_FINISH_QUEST);  

  if (ScriptQuest::get_instance().has(ScriptQuest::NPC_VISIT,id)) { 
    char func_name[32];
    sprintf(func_name,"%s_%d","state",id);
    int state = execute_script_event(user,func_name,this);
    if (state) {
      setUState(state);
      return;
    }
  }

  OnVisit event(id);
  int state = EventManager<OnVisit>::instance().state(*user,event) ;
  if (state != -1) {
    setUState(state);
    //Zebra::logger->debug("NPC(%s),任务(%d,%d) ",name,id,state);

  }
}

/**
 * \brief 向9屏发送自己的信息
 */
void SceneNpc::reSendMyMapData()
{
    this->sendMeToNine();
}

/**
 * \brief npc切换地图
 * 只能在同一服务器
 *
 *
 * \param newScene 要去的地图
 * \param pos 要去的位置
 * \return 是否切化成功
 */
bool SceneNpc::changeMap(Scene * newScene,const zPos &pos)
{
  if (!newScene)
  {
    Zebra::logger->error("SceneNpc::changeMap(): 尝试跳转不存在的地图 newScene=0");
    return false;
  }
  scene->removeNpc(this);
  //通知客户端删除NPC
  zPosI oldPosI = getPosI();
  Scene *oldScene=scene;

  //this->pos = pos;


  if (!newScene->refreshNpc(this,pos))
  {
    Zebra::logger->debug("%s 跳转地图 %s 失败",name,newScene->name);
    return false;
  }
  scene = newScene;
  //删除旧地图上的我
  Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
  removeNpc.dwMapNpcDataPosition = tempid;
  oldScene->sendCmdToNine(oldPosI,&removeNpc,sizeof(removeNpc),this->dupIndex);
  //添加新地图的我
  this->sendMeToNine();

  Zebra::logger->debug("%s 跳转地图 %s (x=%u,y=%u)",name,newScene->name,getPos().x,getPos().y);
  return true;
}

/**
 * \brief 向选中该npc的玩家发送该npc的状态信息
 *
 *
 * \param state npc状态
 * \param value 具体数值
 * \param time 持续时间
 * \return 
 */
void SceneNpc::sendtoSelectedState(DWORD state,WORD value,WORD time)
{
  //Zebra::logger->debug("NPC(%s),状态(%d,%d,%d)",name,state,value,time);
  using namespace Cmd;
  char Buf[200]; 
  bzero(Buf,sizeof(Buf));
  stSelectReturnStatesPropertyUserCmd *srs=(stSelectReturnStatesPropertyUserCmd*)Buf;
  constructInPlace(srs);
  if( npc->kind == NPC_TYPE_GHOST )
	  srs->byType = MAPDATATYPE_USER;
  else
	  srs->byType = MAPDATATYPE_NPC;
  srs->dwTempID = this->tempid;
  srs->states[0].state = state;
  srs->states[0].result = value;
  srs->states[0].time = time;
  srs->size=1;
  //selected_lock.lock();
  SelectedSet_iterator iter = selected.begin();
  for(; iter != selected.end() ;)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(*iter);
    if (pUser)
    {
      if (this->scene->checkTwoPosIInNine(this->getPosI(),pUser->getPosI()))
      {
        pUser->sendCmdToMe(srs,sizeof(stSelectReturnStatesPropertyUserCmd) + sizeof(srs->states[0]));
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

/**
 * \brief 向选种该npc的玩家发送该npc的hp和np
 */
void SceneNpc::sendtoSelectedHpAndMp()
{
}

/**
 * \brief 设置图腾系的持续时间
 *
 * \param standTime 延长的时间
 */
void SceneNpc::setStandingTime(DWORD standTime)
{
    dwStandTime += standTime;
    dwStandTimeCount=standTime;
#ifdef _DEBUG
    if (standTime>0) Zebra::logger->debug("图腾系的持续时间%u 当前时间[%u]",standTime,dwStandTime);
#endif
}


/**
 * \brief 判断npc的五行相克
 *
 *
 * \param five 要判断的五行数值
 * \return 1：克 2：被克 0：无关
 */
int SceneNpc::IsOppose(DWORD five)
{
  if (this->npc->five == (five + 4)%5)
  {
    return 1;
  }
  else
  {
    if (this->npc->five == (five + 1)%5)
    {
      return 2;
    }

  }
  if ((this->npc->five != 5) &&(five == 5))
    return 1;
  if ((this->npc->five == 5) &&(five != 5))
    return 2;
  return 0;
}

/**
 * \brief 设置npc的移动速度倍率
 *
 * \param rate 倍率
 */
void SceneNpc::setSpeedRate(float rate)
{
  speedRate = rate;

  //hp20%以下速度加倍
  if (speedUpUnder20)
    speedRate *= 2.0;

  //this->sendMeToNine();
  //Channel::sendNine(this,"移动速度 %f",speedRate);
}

/**
 * \brief 设置npc原始速度
 */
void SceneNpc::resetSpeedRate()
{
  DWORD value = (DWORD)((640-skillValue.movespeed)*(1+this->skillValue.array_dmvspeed/100.0f));
  if (value == 0)
  {
    speedRate = 6;
  }
  else
  {
    speedRate = 640.0f/((float)value);
  }
  if (this->assault) speedRate =4;

  //hp20%以下速度加倍
  if (speedUpUnder20)
    speedRate *= 2.0;

  //this->sendMeToNine();
  //Channel::sendNine(this,"移动速度 %f",speedRate);
}

/**
 * \brief 设置npc的攻击速度倍率
 *
 * \param rate 倍率
 */
void SceneNpc::setAspeedRate(float rate)
{
    aspeedRate = rate;

    //hp50%以下攻击速度加倍
    if (aspeedUpUnder50)
    aspeedRate *= 2.0;

    //Channel::sendNine(this,"攻击速度 %f",aspeedRate);
    //Zebra::logger->debug("%s 设置攻击速度倍率 %f",name,rate);
}

/**
 * \brief 设置npc原始攻击速度
 */
void SceneNpc::resetAspeedRate()
{
    aspeedRate = 1.0;

    //hp50%以下攻击速度加倍
    if (aspeedUpUnder50)
    aspeedRate *= 2.0;

    //Channel::sendNine(this,"攻击速度 %f",aspeedRate);
}

/**
 * \brief 取消npc的主人
 */
void SceneNpc::clearMaster()
{
    //master = NULL;
}

//主动寻敌时判断
/**
 * \brief 判断是否可锁定目标
 *
 *
 * \param entry 要判断的目标
 * \return 是否可以锁定
 */
bool SceneNpc::canChaseTarget(const SceneEntryPk * entry)
{
  switch (entry->getType())
  {
    case zSceneEntry::SceneEntry_Player:
      {
        if (NPC_AI_PATROL==AIDefine.type)
        {
          if (((SceneUser *)entry)->isRedNamed())
            return true;
          else
            return false;
        }
        else
          return true;
      }
    case zSceneEntry::SceneEntry_NPC:
      {
        //宠物暂时被动
        if (NPC_TYPE_PET!=/* ((SceneNpc *)entry)-> */npc->kind) return false;
        if (NPC_AI_PATROL==AIDefine.type)
        {
          if (((SceneNpc *)entry)->isRedNamed()) return true;

          SceneEntryPk * pk = ((SceneNpc *)entry)->getChaseSceneEntry();
          if ((pk) && (zSceneEntry::SceneEntry_Player==pk->getType())
              &&(NPC_TYPE_PET!=((SceneNpc *)entry)->npc->kind))
            return true;
        }
        else
          if (NPC_TYPE_PET==((SceneNpc *)entry)->npc->kind)
            return true;
      }
    default:
      return false;
  }
  return false;
}

/**
 * \brief npc是否红名
 *
 * \return 是否红名
 */
bool SceneNpc::isRedNamed(bool allRedMode)
{
    return false;
}

/**
 * \brief 设置npc脚本
 *
 * \param id 脚本id
 * \return 是否设置成功
 */
bool SceneNpc::setScript(int id)
{
    if (!AIC) return false;

    AIC->loadScript(id);
    return AIC->isActive();
}

/**
 * \brief 清除npc脚本
 *
 */
void SceneNpc::clearScript()
{
    if (!AIC) return;
    AIC->unloadScript();
}

/**
 * \brief 攻击者冲向我
 * \param attacktype 攻击者的类型
 * \param tempid 攻击者的临时id
 */
void SceneNpc::assaultMe(BYTE attacktype,DWORD tempid)
{
  if (attacktype == zSceneEntry::SceneEntry_Player)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(tempid);
    if (pUser)
    {
      zPos tmppos = pos;
      const zPos &pos1 = pUser->getPos();
      if (pos1.x>pos.x)
      {
        tmppos.x++;
      }
      else if (tmppos.x>0)
      {
        tmppos.x--;
      }

      if (pos1.y>pos.y)
      {
        tmppos.y++;
      }
      else if (tmppos.y>0)
      {
        tmppos.y--;
      }
      pUser->goTo(tmppos);
    }
  }
}

/**
 * \brief 对目标使用技能[sky]技能具体使用函数
 *
 *
 * \param target 目标
 * \param id 技能id
 * \return 是否使用成功
 */
bool SceneNpc::useSkill(SceneEntryPk * target, DWORD id, WORD level) //[sky]添加技能等级参数
{
  //return false;
  if (!target) return false;
  if (!canAttack(target)) return false;

  zSkill *s = usm.findSkill(id);
  if (!s)
  {
	  if( level <= 0 )
		  level = 1;
	  s = zSkill::create(this,id,level);
  }
  if(!s) return false;
  if (!s->canUse()) return false;

  BYTE atype = 0;
  BYTE action = 0;
  npc->getATypeAndAction(atype,action);

  if (this->npc->kind != NPC_TYPE_SURFACE) setDir(getPos().getDirect(target->getPos()));

  using namespace Cmd;
  stAttackMagicUserCmd att;
  att.dwUserTempID = tempid;
  att.dwDefenceTempID = target->tempid;

  att.wdMagicType = id;
  switch(atype)
  {
    case NPC_ATYPE_NEAR:  /// 近距离攻击
      {
        att.byAction = Ani_Attack;
      }
      break;
    case NPC_ATYPE_FAR:    /// 远距离攻击
      {
        att.byAction = Ani_Attack;
      }
      break;
    case NPC_ATYPE_MFAR:
    case NPC_ATYPE_MNEAR:
      {
        att.byAction = Ani_Null;
      }
      break;
    case NPC_ATYPE_NOACTION:
      {
        att.byAction = Ani_Num;
      }
      break;
    default:
      break;
  }
  //att.byAction = action;

  att.xDes = (WORD)target->getPos().x;
  att.yDes = (WORD)target->getPos().y;
  att.byDirect = getDir();
  switch (target->getType())
  {
  case zSceneEntry::SceneEntry_Player:
	  {
		  if( npc->kind == NPC_TYPE_GHOST )  //sky元神特殊处理
			  att.byAttackType = ATTACKTYPE_U2U;
		  else
			  att.byAttackType = ATTACKTYPE_N2U;
	  }
	  break;
  case zSceneEntry::SceneEntry_NPC:
	  {
		  if( npc->kind == NPC_TYPE_GHOST )
			  att.byAttackType = ATTACKTYPE_U2N;
		  else
			  att.byAttackType = ATTACKTYPE_N2N;
	  }
	  break;
  default:
	  {
		  if( npc->kind == NPC_TYPE_GHOST )
			  att.byAttackType = ATTACKTYPE_U2U;
		  else
			  att.byAttackType = ATTACKTYPE_N2U;
	  }
  }

  s->action(&att,sizeof(att));
#ifdef _DEBUG
  //Channel::sendNine(this,"%s!",s->name);
#endif
  setAttackTime(SceneTimeTick::currentTime);
  setMoveTime(SceneTimeTick::currentTime);
  setEndBattleTime(SceneTimeTick::currentTime,10*1000);

  return true;
}


/**
 * \brief SKY 增加NPC召唤属下处理
 *
 *
 * \param target 目标
 * \param id 技能id
 * \param mobID 要召唤的怪物ID
 * \return 是否使用成功
 */
bool SceneNpc::useMobSkill(SceneEntryPk * target, DWORD id, DWORD mobID) 
{
  //return false;
  if (!target) return false;

  zSkill *s = usm.findSkill(id);
  if (!s)
  {
	  s = zSkill::create(this,id,1);

	  if(!s)
		  return false;

	  (DWORD)(s->actionbase->skillStatus[0]._StatusElementList[0].value) = mobID;   //将技能效果设置为配置文件里的怪物ID
  }

  if (!s->canUse()) return false;

  BYTE atype = 0;
  BYTE action = 0;
  npc->getATypeAndAction(atype,action);

  if (this->npc->kind != NPC_TYPE_SURFACE) setDir(getPos().getDirect(target->getPos()));

  using namespace Cmd;
  stAttackMagicUserCmd att;
  att.dwUserTempID = tempid;
  att.dwDefenceTempID = target->tempid;

  att.wdMagicType = id;
  switch(atype)
  {
    case NPC_ATYPE_NEAR:  /// 近距离攻击
      {
        att.byAction = Ani_Attack;
      }
      break;
    case NPC_ATYPE_FAR:    /// 远距离攻击
      {
        att.byAction = Ani_Attack;
      }
      break;
    case NPC_ATYPE_MFAR:
    case NPC_ATYPE_MNEAR:
      {
        att.byAction = Ani_Null;
      }
      break;
    case NPC_ATYPE_NOACTION:
      {
        att.byAction = Ani_Num;
      }
      break;
    default:
      break;
  }
  //att.byAction = action;

  att.xDes = (WORD)target->getPos().x;
  att.yDes = (WORD)target->getPos().y;
  att.byDirect = getDir();
  switch (target->getType())
  {
    case zSceneEntry::SceneEntry_Player:
      {
        att.byAttackType = ATTACKTYPE_N2U;
      }
      break;
    case zSceneEntry::SceneEntry_NPC:
      {
        att.byAttackType = ATTACKTYPE_N2N;
      }
      break;
    default:
      {
        att.byAttackType = ATTACKTYPE_N2U;
      }
      break;
  }

  s->action(&att,sizeof(att));
#ifdef _DEBUG
  //Channel::sendNine(this,"%s!",s->name);
#endif
  setAttackTime(SceneTimeTick::currentTime);
  setMoveTime(SceneTimeTick::currentTime);
  setEndBattleTime(SceneTimeTick::currentTime,10*1000);

  return true;
}




/**
 * \brief npc随机说话
 *
 * \param type 说话的类型
 */
void SceneNpc::randomChat(NpcChatType type)
{
  if ((npc->kind!=0)
      &&(npc->kind!=6)
      &&(npc->kind!=7)
      &&(npc->kind!=19)
      &&(npc->kind!=20)
      &&(npc->kind!=24)
      &&(npc->kind!=25))
    return;

  char str[MAX_CHATINFO];
  if (SceneNpcManager::getMe().getNpcCommonChat(type,str))
    Channel::sendNine(this,str);
  //else
  //  Zebra::logger->debug("取得说话内容失败 type=%d",type);
}

/**
 * \brief 角色中了恐惧效果
 */
bool SceneNpc::dreadProcess()
{
  if (dread)
  {
    int count=10;
    int curDir=0;
    do {
      if (count==10)
        curDir = getDir()+randBetween(-1,1);
      else
        curDir++;
      if (curDir <0) curDir = 7;
      count--;
    }while(!move(curDir%8,1)&&count>0);
    return true;
  }
  return false;
}

/**
* \brief 角色中了失明效果
*/
bool SceneNpc::blindProcess()
{
	if (blind)
	{
		int count=10;
		int curDir=0;
		do {
			if (count==10)
				curDir = getDir()+randBetween(-1,1);
			else
				curDir++;
			if (curDir <0) curDir = 7;
			count--;
		}while(!move(curDir%8,1)&&count>0);
		return true;
	}
	return false;
}

/**
 * \brief 得到npc的攻击类型
 *
 * \return 攻击类型
 */
BYTE SceneNpc::getAType()
{
    BYTE atype = 0;
    BYTE action = 0;
    npc->getATypeAndAction(atype,action);
    return atype;
}

/**
 * \brief 判断是否红名
 *
 * \return 是否红名
 */
bool SceneNpc::isRedNamed(bool allRedMode) const
{
    return false;
}
/**
 * \brief 填充宠物信息的结构
 *
 *
 * \param data 结构地址
 */     
void SceneNpc::full_PetDataStruct(Cmd::t_PetData & data)
{
}

/**
 * \brief 是否主动攻击
 *
 * \return 是否主动攻击
 */
bool SceneNpc::isActive()
{
    return aif&AIF_ACTIVE_MODE;
}

/**
 * \brief 是否可以战斗
 *
 * \return 是否可以战斗
 */
bool SceneNpc::canFight()
{
    return !(aif&AIF_NO_BATTLE);
}

/**
 * \brief 是否可以移动
 *
 * \return 是否可以移动
 */
bool SceneNpc::canMove()
{
	if(npc->kind == NPC_TYPE_TURRET ||
		npc->kind == NPC_TYPE_BARRACKS ||
		npc->kind == NPC_TYPE_CAMP)
		return false;

    return ( !(aif&AIF_NO_MOVE) && moveAction);
}

/**
 * \brief 得到宠物的类型
 *
 * \return 宠物类型
 */
Cmd::petType SceneNpc::getPetType()
{
    return Cmd::PET_TYPE_NOTPET;
}

/**
 * \brief 标记npc为清除状态
 * 下次循环时清除
 */
void SceneNpc::setClearState()
{
    //if (isSpecialNpc())
    //  SceneNpcManager::getMe().removeSpecialNpc(this);
    clearMe = true;

  if (isMainGeneral())
    scene->bossMap.erase(COUNTRY_MAIN_GEN);
  if (scene->bossMap[id]==this)
    scene->bossMap.erase(id);
    //Zebra::logger->info("标记npc %s(%u)",name,tempid);
}

/**
 * \brief 检查该npc是否要清除
 *
 * \return 是否要清除
 */
bool SceneNpc::needClear()
{
    return clearMe;
}

/**
 * \brief 判断是否是任务npc
 *
 * \return 是否是任务npc
 */
bool SceneNpc::isTaskNpc()
{
    if (npc->kind == NPC_TYPE_TRADE 
      || npc->kind == NPC_TYPE_TASK 
      || npc->kind == NPC_TYPE_LIVENPC)
    {
  return true;
    }
    return false;
}
/**
 * \brief 判断是否是功能npc
 *
 * \return 是否是功能npc
 */
bool SceneNpc::isFunctionNpc()
{
    if (npc->kind == NPC_TYPE_ROADSIGN 
      || npc->kind == NPC_TYPE_TRADE 
      || npc->kind == NPC_TYPE_TASK 
      || npc->kind == NPC_TYPE_MOBILETRADE
      || npc->kind == NPC_TYPE_MAILBOX
      || npc->kind == NPC_TYPE_AUCTION)
    {
  return true;
    }
    return false;
}
/**
 * \brief 判断是否是特殊npc
 *
 * 特殊npc包括宠物、boss、有固定脚本的npc
 * \return 是否是特殊npc
 */
bool SceneNpc::isSpecialNpc()
{
  if (Cmd::PET_TYPE_NOTPET!=getPetType())
  {
    SceneEntryPk *p = getTopMaster();
    if (0==p) return false;
    if (p==this) return false;
    if (p->getType() == zSceneEntry::SceneEntry_Player) return true;
    if (p->getType() == zSceneEntry::SceneEntry_NPC) return ((SceneNpc *)p)->isSpecialNpc();
  }
  if (AIC->isActive()) return true;
  if (NPC_TYPE_BBOSS==npc->kind || NPC_TYPE_PBOSS==npc->kind || NPC_TYPE_BARRACKS==npc->kind) return true;
  return false;
}

/**
 * \brief 得到速度倍率
 *
 * \return 速度倍率
 */
float SceneNpc::getSpeedRate()
{
    return speedRate;
}

/**
 * \brief 改变并刷新角色属性
 */
void  SceneNpc::changeAndRefreshHMS(bool lock,bool sendData)
{
    this->resetSpeedRate();
    this->reSendData = false;
}

/**
 * \brief 设置攻击目标
 * 
 *
 * \param target 攻击目标
 * \param force 强制设置目标
 * \return 是否成功
 */
bool SceneNpc::setCurTarget(SceneEntryPk * target,bool force)
{
  if (!target) return false;
  if (!canFight()) return false;
  if (!canReach(target)) return false;
  if (curTargetID!=0 && !force)
  {
    if (lockTarget||aif&AIF_LOCK_TARGET) return false;
    return forceChaseUser(target);
  }

  /*if (aif&AIF_CALL_FELLOW_7 || aif&AIF_CALL_FELLOW_9)
    AIC->on_find_enemy(target);*/

  if (SceneEntryPk::setCurTarget(target))
  {
    closeCount = 0;
    return true;
  }
  return false;
}

/**
 * \brief 设置攻击目标
 *
 *
 * \param type 攻击目标类型
 * \param id 攻击目标临时ID
 * \param force 强制设置目标
 * \return 是否成功
 */
bool SceneNpc::setCurTarget(DWORD id,DWORD type,bool force)
{
  //if (!canFight()) return false;
  SceneEntryPk * target = 0;
  switch (type)
  {
    case zSceneEntry::SceneEntry_Player:
      {
        target = scene->getUserByTempID(id);
        break;
      }
    case zSceneEntry::SceneEntry_NPC:
      {
        target = SceneNpcManager::getMe().getNpcByTempID(id);
        break;
      }
    default:
      return false;
  } 
  return setCurTarget(target);
}

/**
 * \brief 招唤宠物
 *
 * \param id 宠物ID
 * \param type 宠物类型
 * \param standTime 该npc的持续时间
 * \param sid 脚本ID
 * \param petName 指定宠物的名字
 * \param anpcid 强化的附加npcID
 *
 * \return 如果该宠物存在,返回宠物对象的指针,否则为NULL
 */
ScenePet * SceneNpc::summonPet(DWORD id,Cmd::petType type,DWORD standTime,DWORD sid,const char * petName,DWORD anpcid,zPos pos,BYTE vdir)
{
  if ((Cmd::PET_TYPE_PET>type)||(Cmd::PET_TYPE_SEMI<type))
  {
    Zebra::logger->info("SceneNpc::summonPet(): %s 召唤未知类型的宠物 type=%d",name,type);
    return 0;
  }

  zNpcB *base = npcbm.get(id);
  zNpcB *abase = NULL;
  if (anpcid>0) abase = npcbm.get(anpcid);
  if (NULL == base) return 0;

  t_NpcDefine define;
  //  zPos pos = getPos();
  define.id = base->id;
  strncpy(define.name,base->name,MAX_NAMESIZE-1);
  define.pos = getPos();
  define.num = 1;
  define.interval = 5;
  define.initstate = zSceneEntry::SceneEntry_Normal;
  define.width = SceneUser::CALL_PET_REGION;
  define.height = SceneUser::CALL_PET_REGION;
  define.pos -= zPos(SceneUser::CALL_PET_REGION/2,SceneUser::CALL_PET_REGION/2);
  define.scriptID = sid;
  scene->initRegion(define.region,define.pos,define.width,define.height);

  ScenePet * newPet = scene->summonOneNpc<ScenePet>(define,pos,base,dupIndex,standTime,abase,vdir);

  if (newPet)
  {
    /*
       char n[MAX_NAMESIZE];
       bzero(n,MAX_NAMESIZE);
       if (0==strncmp(petName,"",MAX_NAMESIZE))
       _snprintf(n,MAX_NAMESIZE-1,"(%s)%s",name,newPet->name);
       else
       _snprintf(n,MAX_NAMESIZE-1,"(%s)%s",name,petName);
       strncpy(newPet->name,,MAX_NAMESIZE-1);
       */

    newPet->setMaster(this);
	newPet->setPetAI( Cmd::PETAI_MOVE_FOLLOW );
    newPet->setPetType(type);
    newPet->setDir(vdir);
    if (isSpecialNpc())
      SceneNpcManager::getMe().addSpecialNpc(newPet);

    using namespace Cmd;
    switch (type)
    {
      case PET_TYPE_PET:
        {
          if (pet)
          {
            pet->death(SceneTimeTick::currentTime);
            killOnePet(pet);
          }
          if (summon)
          {
            summon->death(SceneTimeTick::currentTime);
            killOnePet(summon);
          }
          pet = newPet;
        }
        break;
      case PET_TYPE_SUMMON:
        {
          if (pet)
          {
            pet->death(SceneTimeTick::currentTime);
            killOnePet(pet);
          }
          if (summon)
          {
            summon->death(SceneTimeTick::currentTime);
            killOnePet(summon);
          }
          summon = newPet;
        }
        break;
      case PET_TYPE_TOTEM:
#ifdef _DEBUG
        //Zebra::logger->debug("SceneNpc::summonPet(): lock %s",name);
#endif
        totems.push_back(newPet);
#ifdef _DEBUG
        //Zebra::logger->debug("SceneNpc::summonPet(): unlock %s",name);
#endif
        break;
      case PET_TYPE_SEMI:
        semipetList.push_back(newPet);
        break;
      default:
        Zebra::logger->info("SceneNpc::summonPet(): 未知的宠物类型 %d",type);
        break;
    }

    Zebra::logger->debug("%s 增加宠物 %s 类型 %d",name,newPet->name,type);
  }

  return newPet;
}


/**
 * \brief 删除一个宠物
 *
 */
bool SceneNpc::killOnePet(ScenePet * kill)
{
  if (!kill) return false;
  if (kill->getMaster()!=this)
  {
    Zebra::logger->error("[宠物]%s(%u) 不是NPC %s(%u) 的宠物,kill->getMaster()=%u",kill->name,kill->tempid,name,tempid,kill->getMaster());
    return false;
  }

  using namespace Cmd;

#ifdef _DEBUG
  //Zebra::logger->debug("SceneNpc::killOnePet(): lock %s",name);
#endif
  switch (kill->getPetType())
  {
    case PET_TYPE_PET:
      {
        pet = 0;
      }
      break;
    case PET_TYPE_SUMMON:
      {
		  if( !MirageSummon.empty() )
			  MirageSummon.clear();
		  summon = 0;
	  }
      break;
    case PET_TYPE_TOTEM:
      {
        totems.remove(kill);
      }
      break;
    case PET_TYPE_SEMI:
      {
        semipetList.remove(kill);
      }
      break;
    default:
      break;
  }

  kill->clearMaster();

  return true;
#ifdef _DEBUG
  //Zebra::logger->debug("SceneNpc::killOnePet(): unlock %s",name);
#endif
}

/**
 * \brief 删除所有宠物
 *
 */
void SceneNpc::killAllPets()
{
  //删除所有宠物
#ifdef _DEBUG
  //Zebra::logger->debug("SceneNpc::killAllPets(): lock %s type=%u",name,getPetType());
#endif
  std::list<ScenePet *> copy(totems);
  for (std::list<ScenePet *>::iterator it=copy.begin(); it!=copy.end(); it++)
  {
    (*it)->skillStatusM.clearActiveSkillStatus();

    (*it)->killAllPets();
    //petDeath();
    (*it)->leaveBattle();
    (*it)->scene->clearBlock((*it)->getPos());
    (*it)->setState(SceneEntry_Death);

    Cmd::stNpcDeathUserCmd death;
    death.dwNpcTempID = (*it)->tempid;
    (*it)->scene->sendCmdToNine((*it)->getPosI(),&death,sizeof(death),this->dupIndex);

    (*it)->clearMaster();
  }
  totems.clear();
  MirageSummon.clear();
  copy.clear();
  copy = semipetList;
  for (std::list<ScenePet *>::iterator it=copy.begin(); it!=copy.end(); it++)
  {
    (*it)->skillStatusM.clearActiveSkillStatus();

    (*it)->killAllPets();
    (*it)->leaveBattle();
    (*it)->scene->clearBlock((*it)->getPos());
    (*it)->setState(SceneEntry_Death);

    Cmd::stNpcDeathUserCmd death;
    death.dwNpcTempID = (*it)->tempid;
    (*it)->scene->sendCmdToNine((*it)->getPosI(),&death,sizeof(death),this->dupIndex);

    (*it)->clearMaster();
  }
  semipetList.clear();
#ifdef _DEBUG
  //Zebra::logger->debug("SceneNpc::killAllPets(): unlock %s type=%u",name,getPetType());
#endif

  if (pet)
  {
    pet->skillStatusM.clearActiveSkillStatus();

    pet->killAllPets();
    //petDeath();
    pet->leaveBattle();
    pet->scene->clearBlock(pet->getPos());
    pet->setState(SceneEntry_Death);

    Cmd::stNpcDeathUserCmd death;
    death.dwNpcTempID = pet->tempid;
    pet->scene->sendCmdToNine(pet->getPosI(),&death,sizeof(death),this->dupIndex);

    pet->clearMaster();
    pet = 0;

  }
  if (summon)
  {
    summon->skillStatusM.clearActiveSkillStatus();

    summon->killAllPets();
    //petDeath();
    summon->leaveBattle();
    summon->scene->clearBlock(summon->getPos());
    summon->setState(SceneEntry_Death);

    Cmd::stNpcDeathUserCmd death;
    death.dwNpcTempID = summon->tempid;
    summon->scene->sendCmdToNine(summon->getPosI(),&death,sizeof(death),this->dupIndex);

    summon->clearMaster();
    summon = 0;

  }
}

/**
 * \brief 检查是否到了回血的时间
 *
 *
 * \param ct 用于比较的时间
 * \return 是否到时间
 */
bool SceneNpc::checkRecoverTime(const zRTime& ct)
{
    return ct>=rcvTimePet;
}

/**
 * \brief 设置下次回血的时间
 *
 * \param ct 延迟开始的时间
 * \param delay 延迟
 */
void SceneNpc::setRecoverTime(const zRTime& ct,int delay)
{
    rcvTimePet = ct;
    rcvTimePet.addDelay(delay);
}

/**
 * \brief 设置主人棍类附加攻击力
 * \param mindamage 最小攻击力
 * \param maxdamage 最大攻击力
 */
void SceneNpc::setAppendDamage(WORD mindamage,WORD maxdamage)
{
    appendMinDamage = mindamage;
    appendMaxDamage = maxdamage;
}

/**
 * \brief 获取最小法术攻击力
 * \return 最小法术攻击力
 */
DWORD SceneNpc::getMinMDamage()
{
    SDWORD value = (SDWORD)(this->npc->mdamage+appendMinDamage+skillValue.uppetdamage+skillValue.theurgy_umdam-skillValue.dmdam-skillValue.theurgy_dmdam);
    if (value <0) value =0;
    return value;
}

/**
 * \brief 获取最大法术攻击力
 * \return 最大法术攻击力
 */
DWORD SceneNpc::getMaxMDamage() 
{
    SDWORD value = (SDWORD)(this->npc->maxmdamage+appendMaxDamage+skillValue.uppetdamage+skillValue.theurgy_umdam-skillValue.dmdam-skillValue.theurgy_dmdam);
    if (value <0) value =0;
    return value;
}

/**
 * \brief 获取最小物理攻击力
 * \return 最小物理攻击力
 */
DWORD SceneNpc::getMinPDamage()
{
    SDWORD value = (SDWORD)(this->npc->damage+appendMinDamage+skillValue.uppetdamage+skillValue.theurgy_updam-skillValue.dpdam-skillValue.theurgy_dpdam);
    if (value <0) value =0;
    return value;
}

/**
 * \brief 获取最大物理攻击力
 * \return 最大物理攻击力
 */
DWORD SceneNpc::getMaxPDamage()
{
    SDWORD value = (SDWORD)(this->npc->maxdamage+appendMaxDamage+skillValue.uppetdamage+skillValue.theurgy_updam-skillValue.dpdam-skillValue.theurgy_dpdam);
    if (value <0) value =0;
    return value;
}

/**
 * \brief 获取最小法术防御力
 * \return 最小法术防御力
 */
DWORD SceneNpc::getMinMDefence()
{
    SDWORD value = this->npc->mdefence+skillValue.uppetdefence+skillValue.umdef+skillValue.theurgy_umdef - skillValue.theurgy_dmdef;
    value = (SDWORD)(value *( 1 - skillValue.dmdefp/100.0f));
    if (value < 0) value = 0;
    return value;
}

/**
 * \brief 获取最大法术防御力
 * \return 最大法术防御力
 */
DWORD SceneNpc::getMaxMDefence() 
{
    SDWORD value = this->npc->maxmdefence+skillValue.uppetdefence+skillValue.umdef+skillValue.theurgy_umdef - skillValue.theurgy_dmdef;
    value = (SDWORD)(value *( 1 - skillValue.dmdefp/100.0f));
    if (value < 0) value = 0;
    return value;
}

/**
 * \brief 获取最小物理防御力
 * \return 最小物理防御力
 */
DWORD SceneNpc::getMinPDefence()
{
    SDWORD value = this->npc->pdefence+skillValue.uppetdefence+skillValue.updef+skillValue.theurgy_updef - skillValue.theurgy_dpdef;
    if (value < 0) value = 0;
    return value;
}

/**
 * \brief 获取最大物理防御力
 * \return 最大物理防御力
 */
DWORD SceneNpc::getMaxPDefence()
{
    SDWORD value = this->npc->maxpdefence+skillValue.uppetdefence+skillValue.updef+skillValue.theurgy_updef - skillValue.theurgy_dpdef;
    if (value < 0) value = 0;
    return value;
}

/**
 * \brief 获得最大的hp
 * \return 返回最大值
 */
DWORD SceneNpc::getMaxHP()
{
#ifdef _DEBUG
  if (boostupHpMaxP)
    Zebra::logger->debug("boostupHpMaxP=%u",boostupHpMaxP);
#endif
    return (DWORD)((npc->hp+(anpc?anpc->hp:0)+skillValue.maxhp)*(1.0f+(float)(boostupHpMaxP/100.0f)));;
}

/**
 * \brief 获得最大的hp
 * \return 返回最大值
 */
DWORD SceneNpc::getBaseMaxHP()
{
    return npc->hp+(anpc?anpc->hp:0);
}

/**
 * \brief 向9屏发送自己的数据
 *
 */
void SceneNpc::sendMeToNine()
{
  if (npc->kind == NPC_TYPE_TRAP)
  {
    SceneEntryPk *entry = getMaster();
    if (entry && entry->getType() == zSceneEntry::SceneEntry_Player)
    {
      SceneUser *pUser = (SceneUser *)entry;
    BUFFER_CMD(Cmd::stAddMapNpcAndPosMapScreenStateUserCmd,send,zSocket::MAX_USERDATASIZE);
    this->full_t_MapNpcDataAndPosState(send->data);
      pUser->sendCmdToMe(send,send->size());
    }    
  }
  else
  {
    BUFFER_CMD(Cmd::stAddMapNpcAndPosMapScreenStateUserCmd,send,zSocket::MAX_USERDATASIZE);
    this->full_t_MapNpcDataAndPosState(send->data);
    this->scene->sendCmdToNine(getPosI(),send,send->size(),this->dupIndex);
  }
}

/**
 * \brief npc回血
 *
 */
bool SceneNpc::recover()
{
  if (!npc->recover.num) return 0;

  bool ret = false;
  DWORD tMaxHP = getMaxHP();

  if (!needRecover && (npc->recover.start*tMaxHP/100 > hp))
    needRecover = true;
  if (!needRecover) return 0;

  if (_3_sec(SceneTimeTick::currentTime))
  {
    if (npc->recover.type==1)//按百分比回
      hp += tMaxHP*npc->recover.num/100;
    else if (npc->recover.type==2)//按数量
      hp += npc->recover.num;

    ret = true;
  }

  if (hp>tMaxHP) hp = tMaxHP;
  if (hp==tMaxHP)
    needRecover = false;

  return ret;
}

/**
 * \brief 脱离战斗 
 *
 */
void SceneNpc::leaveBattle(bool bDie)
{
	//if (aif&AIF_RCV_REST)//脱离战斗回血
	//{
	//	rcvTimeRest.now();
	//	rcvTimeRest.addDelay(30*1000);
	//}

	//sky 现在脱离战斗直接回满血
	if(!bDie)
		this->hp = getMaxHP();

	//[sky]脱离战斗就杀死BB和清除AI相关变量
	killAllPets();

	m_dwNowTime = 0;
	m_bFighting = false;
	m_dwGtime = 0;

	Zebra::logger->debug("%s 脱离战斗！初始化AI定时系统", name );

	clearDefTarget();
	lockTarget = false;
	SceneEntryPk::leaveBattle();
#ifdef _DEBUG
	//Zebra::logger->debug("%s 脱离战斗",name);
#endif
}

/**
 * \brief 设置第二攻击目标
 * \param target 目标指针
 * \return 是否成功
 */
bool SceneNpc::setSecondTarget(SceneEntryPk * target)
{
  if (!target) return false;
  if (!canFight()) return false;
  if (curTargetType==(DWORD)target->getType() && curTargetID==target->tempid) return false;
  if (!canReach(target)) return false;

  if (target==this) return false;//不设置自己
  if (!isEnemy(target)) return false;//不设置朋友

  secondTargetType = target->getType();
  secondTargetID = target->tempid;
#ifdef _DEBUG
  //  Zebra::logger->debug("%s 设置第二目标 id=%u type=%u",name,secondTargetID,secondTargetType);
#endif
  return true;
}

/**
 * \brief 设置第二攻击目标
 * \param id 临时id
 * \param type 类型
 * \return 是否成功
 */
bool SceneNpc::setSecondTarget(DWORD id,DWORD type)
{
  SceneEntryPk * def = NULL;
  if (zSceneEntry::SceneEntry_Player==type)
    def = scene->getUserByTempID(tempid);
  else if (zSceneEntry::SceneEntry_NPC==type)
    def = SceneNpcManager::getMe().getNpcByTempID(tempid);

  if (def)
    return setSecondTarget(def);
  else
    return false;
}

/**
 * \brief 得到次攻击目标
 *
 * \return 次攻击目标
 * 
 */
SceneEntryPk * SceneNpc::getSecondTarget()
{
  switch (secondTargetType)
  {
    case zSceneEntry::SceneEntry_Player:
      return scene->getUserByTempID(secondTargetID);
    case zSceneEntry::SceneEntry_NPC:
      return SceneNpcManager::getMe().getNpcByTempID(secondTargetID);
    default:
      return 0;
  }     
}

/**
 * \brief 设置第二攻击目标为当前目标
 *
 * \return 是否成功
 * 
 */
bool SceneNpc::chaseSecondTarget()
{
    if (secondTargetID==0) return false;
    //setCurTarget(secondTargetID,secondTargetType);
    curTargetID = secondTargetID;
    curTargetType = secondTargetType;
#ifdef _DEBUG
    //Zebra::logger->debug("%s 攻击第二目标 id=%u type=%u",name,secondTargetID,secondTargetType);
#endif
    secondTargetID = 0;
    secondTargetType = 0;
    return true;
}

/**
 * \brief 设置当前目标的主人当前目标
 *
 * \return 是否成功
 * 
 */
bool SceneNpc::chaseItsMaster()
{
    SceneEntryPk * c = getCurTarget();
    if (!c) return false;

    SceneEntryPk * m = c->getTopMaster();
    if (!m || m==c) return false;

#ifdef _DEBUG
    //Zebra::logger->debug("%s 攻击其主人 id=%u type=%u",name,m->tempid,m->getType());
#endif
    return setCurTarget(m,true);
}

void SceneNpc::goToRandomScreen()
{
  zPos randPos;
  if (this->scene->randzPosOnRect(getPos(),randPos,SCREEN_WIDTH,SCREEN_HEIGHT))
  {
    warp(randPos);
  }
}

bool SceneNpc::isMainGeneral()
{
  return (id>=58200 && id<58300 && 0==id%10);
}

/**
 * \brief 根据npcMap召唤npc（宠物、HP60%以下、死亡后召唤的npc）
 *
 * \return 召唤的数量
 */
int SceneNpc::summonByNpcMap(std::map<DWORD,std::pair<DWORD,DWORD> > map)
{
  DWORD count = 0;
  for (std::map<DWORD,std::pair<DWORD,DWORD> >::iterator it=map.begin(); it!=map.end(); it++)
  {
    zNpcB *base = npcbm.get(it->first);

    if (base)
    {
      t_NpcDefine define;
      //const zPos &pos = getPos();
      define.id = base->id;
      strncpy(define.name,base->name,MAX_NAMESIZE-1);
      define.pos = getPos();
      define.num = 1;
      define.interval = 5;
      define.initstate = zSceneEntry::SceneEntry_Normal;
      define.width = 10;
      define.height = 10;
      define.pos -= zPos(5,5);
      define.scriptID = 0;
      scene->initRegion(define.region,define.pos,define.width,define.height);

      for (DWORD i=0; i<it->second.first; i++)
      {
        if (selectByPercent(it->second.second))
        {
          SceneNpc * newPet = scene->summonOneNpc<SceneNpc>(define,zPos(0,0),base,dupIndex,0,0);    
          if (newPet)
            count++;
        }
      }
    }
  }
  return count;
}

  /**
   * \brief 在屏幕内定点移动
   */
  void SceneNpc::jumpTo(zPos &newPos)
  {
    warp(newPos);
  }

///////////////////////////////////////////////////////////////////////////////////

bool SceneNpc::ChangeNpc()
{
	// sky 获取NPC变身NPC的ID
	DWORD dwID = npc->ChangeNpcID;
	zNpcB * npcbuff;

	if(dwID > 0)
		npcbuff = npcbm.get(dwID);
	else
		return false;

	// sky 检测获取的NPC指针不能为NULL
	if(npcbuff)
	{
		// sky 将NPC隐藏用来显示变身动画
		hideMe( 3000 );

		npc = npcbuff; //sky 把新的NPC基本数据赋给当前NPC
		this->id = dwID;
		memcpy(name, npc->name, sizeof(name), sizeof(name));
	}
	else
		return false;

	del_ai(); //sky 删除旧的AI列表

	laod_ai(); //sky 读取新的AI列表

	//sky 通知客户端跟新怪物的形象
	Cmd::stNpcChangeUserCmd newNpc;
	full_t_MapNpcData(newNpc.data);
	scene->sendCmdToNine(getPosI(),&newNpc,sizeof(newNpc),this->dupIndex);

	return true;
}

//sky 计算伤害反射
void SceneNpc::reflectDam(int &dwDamDef,int &dwDamSelf,DWORD skillID)
{
	zSkill *s = NULL;

	//sky 反弹魔法攻击伤害百分比
	if (this->skillValue.reflect2 > 0)
	{
		if(	!(skillID == SERVER_SKILL_ATTACK_NORMAL) &&
			!(skillID == SERVER_SKILL_DAGGER_ATTACK_NORMAL) &&
			!(skillID == SERVER_SKILL_DART_ATTACK_NORMAL) &&
			!(skillID == SERVER_SKILL_HANDS_ATTACK_NORMAL) )
		{
			s = zSkill::createTempSkill(this,skillID,1);

			if(s && s->IsMagicSkill() && !s->IsBuffSkill())
				dwDamSelf += (int)(dwDamDef * (this->skillValue.reflect2 / 100.0f ));
		}

	}

	//sky 反弹物理攻击伤害百分比
	if (this->skillValue.reflectp > 0)
	{
		if(	skillID == SERVER_SKILL_ATTACK_NORMAL ||
			skillID == SERVER_SKILL_DAGGER_ATTACK_NORMAL ||
			skillID == SERVER_SKILL_DART_ATTACK_NORMAL ||
			skillID == SERVER_SKILL_HANDS_ATTACK_NORMAL)
		{
			dwDamSelf += (int)(dwDamDef * (this->skillValue.reflectp / 100.0f ));
		}
		else
		{
			s = zSkill::createTempSkill(this,skillID,1);

			if(s && s->IsPhysicsSkill() && !s->IsBuffSkill())
				dwDamSelf += (int)(dwDamDef * (this->skillValue.reflectp / 100.0f ));
		}

	}

	//sky 反弹伤害固定值
	if (this->skillValue.reflect > 0)
	{
		dwDamSelf += this->skillValue.reflect;
	}

}

//sky 反射技能
void SceneNpc::reflectSkill(SceneEntryPk *pAtt,const Cmd::stAttackMagicUserCmd *rev)
{
	Cmd::stAttackMagicUserCmd cmd;

	//zSkill::createTempSkill(this,rev->wdMagicType,skillValue.reflect_ardor);

	if (pAtt && skillValue.MagicReflex>0)
	{
		zSkill *s = NULL;

		s = pAtt->usm.findSkill(rev->wdMagicType);

		if(s && s->IsMagicSkill())
		{
			memcpy(&cmd,rev,sizeof(cmd),sizeof(cmd));

			switch (pAtt->getType())
			{
			case zSceneEntry::SceneEntry_Player:
				{
					cmd.byAttackType = Cmd::ATTACKTYPE_N2U;
				}
				break;
			case zSceneEntry::SceneEntry_NPC:
				{
					cmd.byAttackType = Cmd::ATTACKTYPE_N2N;
				}
				break;
			default:
				{
					cmd.byAttackType = Cmd::ATTACKTYPE_N2U;
				}
				break;
			}

			cmd.dwDefenceTempID = pAtt->tempid;
			cmd.dwUserTempID = this->tempid;
			cmd.wdMagicType = rev->wdMagicType;
			cmd.byAction = Cmd::Ani_Run;
			cmd.xDes = pAtt->getPos().x;
			cmd.yDes = pAtt->getPos().y;
			cmd.byDirect = getDir();

			if (s)
			{
				zSkill * useSkill = NULL;
				useSkill = zSkill::createTempSkill(this,s->data.skillid,s->data.level);

				if(useSkill)
				{
					useSkill->action(&cmd,sizeof(cmd));
					SAFE_DELETE(useSkill);
				}
			}
		}
	}
}