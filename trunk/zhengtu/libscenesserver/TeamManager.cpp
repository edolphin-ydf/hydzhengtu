#include <zebra/ScenesServer.h>

/**
 * \brief  遍历组队将指定新增队员user的数据发送给队伍所有成员的客户端
 */
struct SendTeamDataExec : public TeamMemExec
{

  /// 队长
  SceneUser *leader;

  /// 增加组队成员消息定义
  Cmd::stFreshTeamMemberUserCmd ret;

  /**
   * \brief  构造函数初始化初始化将要发送的消息
   * \param  l 队长的指针
   * \param  user 新增队员的对象指针
   * \return 
   */
  SendTeamDataExec(SceneUser * l,SceneUser *user)
  {
    leader = l; 
    ret.dwTempID = user->tempid;
    ret.dwMaxHealth = user->charstate.maxhp;
    ret.dwHealth = user->charbase.hp;
    ret.dwMaxMp = user->charstate.maxmp;
    ret.dwMp = user->charbase.mp;
  }

  /**
   * \brief  回调方法
   * \param  member 遍历成员
   * \return false 处理终止 true 处理继续
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().
            getUserByTempID(member.tempid);
    if (pUser)
    {
      if (ret.dwTempID != pUser->tempid)
        pUser->sendCmdToMe(&ret,sizeof(ret));
      return true;
    }
    return false;
  }
};

/**
 * \brief  发送组队指定成员数据
 * \param  leader 队长
 * \param  user 队员
 */
void TeamManager::sendtoTeamCharData(SceneUser *leader,SceneUser *user)
{
    if (leader)
    {
      SendTeamDataExec exec(leader,user);
      team.execEvery(exec);
    }
}

/**
 * \brief  发送组队指定成员数据
 * \param  user 需要发送的成员对象
 */
void TeamManager::sendtoTeamCharData(SceneUser *user)
{
    SceneUser *leader = SceneUserManager::getMe().getUserByTempID(getLeader());
    if (leader)
    {
		sendtoTeamCharData(leader,user);
    }
}

/**
 * \brief  遍历结构,用来给组队中所有队员广播消息
 */
struct SendCmdExec : public TeamMemExec
{
  /// 需要广播的消息
  void *cmd;

  /// 消息长度
  DWORD cmdLen;

  /**
   * \brief  构造初始化属性
   * \param  data 消息体
   * \param  dataLen 消息长度
   */
  SendCmdExec(void *data,DWORD dataLen)
  {
    cmd = data;
    cmdLen = dataLen;
  }

  /**
   * \brief  回调方法
   * \param  member 成员
   * \return false 终止遍历 true 继续遍历
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().
            getUserByTempID(member.tempid);
    if (pUser)
    {
      pUser->sendCmdToMe(cmd,cmdLen);
      return true;
    }
    return true;
  }
};

/**
 * \brief  发送消息给组队
 * \param  user   消息发送者
 * \param  cmd    消息体
 * \param  cmdLen 消息长度
 */
void TeamManager::sendCmdToTeam(SceneUser *user,void *cmd,DWORD cmdLen)
{
	SendCmdExec exec(cmd,cmdLen);
	execEveryOne(exec);
}

/**
 * \brief  用来检查所有队员是否都在一屏内
 */
struct CheckAllInOneScreenExec : public TeamMemExec
{
  bool isOk;
  SceneUser *leader;

  /**
   * \brief  构造初始化属性
   * \param  data 消息体
   * \param  dataLen 消息长度
   */
  CheckAllInOneScreenExec(SceneUser *pUser)
  {
    isOk = true;
    leader = pUser;
  }

  /**
   * \brief  回调方法
   * \param  member 成员
   * \return false 终止遍历 true 继续遍历
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().
            getUserByTempID(member.tempid);
    if (pUser)
    {
      if (!(pUser->scene == leader->scene && pUser->scene->zPosShortRange(pUser->getPos(),leader->getPos(),11,11)))
      {
        isOk = false;
      }
      return true;
    }
    else
    {
      isOk = false;
    }
    return true;
  }
};


bool TeamManager::canPutSkill()
{
	SceneUser *leader = SceneUserManager::getMe().getUserByTempID(getLeader());
	if (leader)
	{
		CheckAllInOneScreenExec exec(leader);
		execEveryOne(exec);
		if (!exec.isOk)
		{
			if (!giveupstatus)
			{
				giveupstatus = true;
				giveuptime.now();
				giveuptime.addDelay(120000);
				return true;
			}
			else
			{
				if (SceneTimeTick::currentTime.msecs() > giveuptime.msecs())
				{
					giveupstatus = false;///将状态初始化回去。
					return false;
				}
				else
					return true;
			}
		}
		else
		{
			giveupstatus = false;
			return true;
		}
	}

	return false;
}

/**
 * \brief   增加一个新的队员处理
 */
struct AddNewMemberExec : public TeamMemExec
{
  /// 新成员
  SceneUser *nm;

  DWORD LeaberID;

  /// 队员新增消息实例1
  Cmd::stAddTeamMemberUserCmd ret_1;

  /// 队员新增消息实例2
  Cmd::stAddTeamMemberUserCmd ret_2;

  /**
   * \brief  构造初始化消息实例
   * \param  u 队长角色对象
   * \param n 新增队员角色对象
   */
  AddNewMemberExec(TeamManager* team, SceneUser *n)
  {
	  LeaberID = team->getLeader();
	  nm = n;
	  if(team)
	  {
		  ret_1.dwTeamID = team->getTeamtempId(); 
		  ret_1.dwHeadID = team->getLeader();
		  ret_2.dwTeamID = team->getTeamtempId();
		  ret_2.dwHeadID = team->getLeader();
		  ret_2.data.dwTempID = nm->tempid;
		  ret_2.data.dwMaxHealth = nm->charstate.maxhp;
		  ret_2.data.dwHealth = nm->charbase.hp;
		  ret_2.data.dwMaxMp = nm->charstate.maxmp;
		  ret_2.data.dwMp = nm->charbase.mp;
		  ret_2.data.wdFace = nm->charbase.face;
		  strncpy(ret_2.data.pstrName,nm->name,MAX_NAMESIZE);
		  ret_2.data.byHead = false;
	  }
  }

  /**
   * \brief  回调方法将新成员发送给每个队员,将每个队员发送给新成员
   * \param  member 队员
   * \return false 终止遍历 true 继续遍历
   */
  bool exec(TeamMember &member)
  {
	  if(member.tempid == MEMBER_BEING_OFF)
		  return true;

	  SceneUser *pUser = SceneUserManager::getMe().
		  getUserByTempID(member.tempid);
	  if (pUser)
	  {
		  TeamManager * team = SceneManager::getInstance().GetMapTeam(pUser->TeamThisID);
		  ret_1.dwTeamID = team->getTeamtempId(); //getLeader();
		  ret_1.dwHeadID = team->getLeader();
		  strncpy(ret_1.data.pstrName,pUser->name,MAX_NAMESIZE);
		  if (LeaberID == pUser->tempid)
		  {
			  ret_1.data.byHead = true;
		  }
		  else
		  {
			  ret_1.data.byHead = false;
		  }
		  ret_1.data.dwTempID = pUser->tempid;
		  ret_1.data.dwMaxHealth = pUser->charstate.maxhp;
		  ret_1.data.dwHealth = pUser->charbase.hp;
		  ret_1.data.dwMaxMp = pUser->charstate.maxmp;
		  ret_1.data.dwMp = pUser->charbase.mp;
		  ret_1.data.wdFace = pUser->charbase.face;
		  strncpy(ret_1.data.pstrName,pUser->name,MAX_NAMESIZE);
		  pUser->sendCmdToMe(&ret_2,sizeof(ret_2));
		  Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"%s加入队伍",ret_2.data.pstrName);
		  nm->sendCmdToMe(&ret_1,sizeof(ret_1));
		  //Zebra::logger->debug("队伍(%ld)发出添加队员指令(%s,%ld)",ret_2.dwTeamID,nm->name,nm->id);
		  return true;
	  }
	  return true;
  }
};


/**
* \brief   sky 增加一个新的跨场景队员处理
*/
struct AddAwayNewMemberExec : public TeamMemExec
{

	/// 新的跨场景队员
	Cmd::Session::stMember * AddUser;

	/// 队员新增消息实例1
	Cmd::stAddTeamMemberUserCmd ret_1;

	/**
	* \brief  构造初始化消息实例
	* \param  team 队伍对象
	* \param AwayUser 新增虚假队员
	* \param Leaber 是否是队长
	*/
	AddAwayNewMemberExec(TeamManager *team, Cmd::Session::stMember * AwayUser)
	{
		AddUser = AwayUser;
		if(team)
		{
			ret_1.dwTeamID = team->getTeamtempId();
			ret_1.dwHeadID = team->getLeader();
			ret_1.data.dwTempID = MEMBER_BEING_OFF;
			ret_1.data.dwMaxHealth = 1;
			ret_1.data.dwHealth = 1;
			ret_1.data.dwMaxMp = 1;
			ret_1.data.dwMp = 1;
			ret_1.data.wdFace = AwayUser->face;
			strncpy(ret_1.data.pstrName,AwayUser->name,MAX_NAMESIZE);
			ret_1.data.byHead = AwayUser->leaber;
		}		
	}

	/**
	* \brief  回调方法将新成员发送给每个队员,将每个队员发送给新成员
	* \param  member 队员
	* \return false 终止遍历 true 继续遍历
	*/
	bool exec(TeamMember &member)
	{
		if(member.tempid == MEMBER_BEING_OFF)
			return true;

		SceneUser *pUser = SceneUserManager::getMe().
			getUserByTempID(member.tempid);
		if (pUser)
		{
			pUser->sendCmdToMe(&ret_1,sizeof(ret_1));
			return true;
		}
		return true;
	}
};


/**
 * \brief  判断角色是否是自己的队友
 * \param  pUser 被判断对象
 * \return true 是 false 否
 */
bool TeamManager::IsOurTeam(SceneUser *pUser)
{
	return IsOurTeam(pUser->id);	
}

/**
 * \brief  判断角色是否是自己的队友
 * \param  dwID 被判断对象的id
 * \return true 是 false 否
 */
bool TeamManager::IsOurTeam(DWORD dwID)
{
	std::vector<TeamMember>::iterator iter;
	//sky 遍历队友靠是否有判断用户存在
	for(iter=team.member.begin(); iter!=team.member.end(); iter++)
	{
		if(iter->id == dwID)
			return true;
	}

	return false;
}

/**
 * \brief  增加一个新的队员
 * \param  pUser 准队员
 * \param  rev 组队邀请应答消息
 * \return true 加入成功 false 加入失败
 */
bool TeamManager::addNewMember(SceneUser *pUser,Cmd::stAnswerNameTeamUserCmd *rev, Cmd::Session::t_Team_AnswerTeam * rev1)
{
    SceneUser *nm = SceneUserManager::getMe().
            getUserByName(rev->byAnswerUserName);
  if (nm)	//sky 如果在本场景能够找到该用户
  {
    if (nm->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER)
        ||  nm->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON))
    {
      return false;
    }

	nm->TeamThisID = getTeamtempId();  //sky 把队伍的唯一ID赋予队员

	if (addMemberByTempID(pUser,nm->tempid))
	{
		Cmd::stTeamModeUserCmd tmu; 
		tmu.byType = pUser->team_mode;
		nm->sendCmdToMe(&tmu,sizeof(tmu));
		Cmd::stObjModeTeamMemberUserCmd objmode; 
		objmode.byType = getObjMode();
		nm->sendCmdToMe(&objmode,sizeof(objmode));
		Cmd::stExpModeTeamMemberUserCmd expmode; 
		expmode.byType = getExpMode();
		nm->sendCmdToMe(&expmode,sizeof(expmode));
		AddNewMemberExec add(this,nm);
		team.execEvery(add);
		pUser->reSendMyMapData();

		addMemberToSession(nm->name);
		return true;

	}
  }
  else	//否则就填充个默认的数据在丢给网关去处理
  {
	  if(rev1)
	  {
		  Cmd::Session::stMember member;
		  member.dwID = rev1->dwAnswerID;
		  member.face = rev1->face;
		  strncpy(member.name, rev1->byAnswerUserName, MAX_NAMESIZE);

		  addWAwayMember(&member);

		  addMemberToSession(member.name);
	  }
  }
  return false;
}

/**
 * \brief  增加一个新的队员
 * \param  leader 队长
 * \param  pUser 准队员
 * \return true 增加成功  false 增加失败
 */
bool TeamManager::addNewMember(SceneUser *leader,SceneUser *pUser)
{
  if (pUser->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER)
      ||  pUser->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON))
  {
	  return false;
  }

  pUser->TeamThisID = getTeamtempId();  //sky 把队伍的唯一ID赋予队员

  AddNewMemberExec add(this,pUser);

  if (addMemberByTempID(leader,pUser->tempid))
  {
    Cmd::stTeamModeUserCmd tmu; 
    tmu.byType=leader->team_mode;
    pUser->sendCmdToMe(&tmu,sizeof(tmu));

    Cmd::stObjModeTeamMemberUserCmd objmode; 
    objmode.byType=getObjMode();
    pUser->sendCmdToMe(&objmode,sizeof(objmode));

    Cmd::stExpModeTeamMemberUserCmd expmode; 
    expmode.byType=getExpMode();
    pUser->sendCmdToMe(&expmode,sizeof(expmode));
    team.execEvery(add);
    pUser->reSendMyMapData();

    addMemberToSession(pUser->name);
    return true;

  }

  return false;
}

//sky 重载一个不需要通知sess的添加队员函数(跨场景用)
bool TeamManager::addNewMember(Cmd::Session::stMember Member, SceneUser *pUser)
{
	if(pUser)
	{
		AddNewMemberExec add(this,pUser);

		if (addMemberByTempID(pUser,pUser->tempid))
		{
			Cmd::stTeamModeUserCmd tmu; 
			tmu.byType = Cmd::TEAM_NORMAL;
			pUser->sendCmdToMe(&tmu,sizeof(tmu));

			Cmd::stObjModeTeamMemberUserCmd objmode; 
			objmode.byType=getObjMode();
			pUser->sendCmdToMe(&objmode,sizeof(objmode));

			Cmd::stExpModeTeamMemberUserCmd expmode; 
			expmode.byType=getExpMode();
			pUser->sendCmdToMe(&expmode,sizeof(expmode));
			team.execEvery(add);
			pUser->reSendMyMapData();

			return true;
		}
	}
	else
	{
		addWAwayMember(&Member);
		return true;
	}

	return false;
}

/**
 * \brief 获取队伍人数
 */
int TeamManager::getSize()
{
	return team.getSize();
}

/**
 * \brief  删除组队
 * \param  pUser 队长
 * \param  tempid 队长的临时id
 */
//void TeamManager::removeTeam(SceneUser *pUser,DWORD tempid)
//{
//	DeleteTeamExec DelTeam;
//
//	team.execEvery(DelTeam)
//}

/**
 * \brief  队伍解散遍历处理每个队员的退出事宜
 */
struct DeleteTeamExec : public TeamMemExec
{
  /// 队长
  //SceneUser *leader;

  /// 删除组队消息
  Cmd::stRemoveTeamUserCmd ret;

  /**
   * \brief  构造初始化
   * \param  u 队长
   */
  DeleteTeamExec(/*SceneUser *u*/)
  {
    //leader = u;
  }

  /**
   * \brief  回调方法,发送队伍解散消息给每个成员
   * \param  member 队员
   * \return true 继续遍历 false 终止遍历
   */
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().
            getUserByTempID(member.tempid);
    if (pUser)
    {
      pUser->TeamThisID = 0;
      pUser->sendCmdToMe(&ret,sizeof(ret));
      pUser->reSendMyMapData();
      Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"队伍解散");
    }
    return true;
  }

};

/**
 * \brief  删除组队
 * \param  leader 队长
 */
void TeamManager::deleteTeam()
{
	setLeader(0);
	setTeamtempId(0);
	DeleteTeamExec del;
	team.execEvery(del);
	team.Clear();
}

/**
 * \brief  删除成员遍历通知所有队伍成员
 */
struct RemoveMemberExec : public TeamMemExec
{
	/// 删除成员消息
	Cmd::stRemoveTeamMemberUserCmd ret;


	/**
	* \brief  构造函数初始化成员删除消息
	* \param  u 队长
	* \param  rem  队员删除通知消息
	*/
	RemoveMemberExec(const Cmd::stRemoveTeamMemberUserCmd *rem)
	{
		ret.dwTeamID = rem->dwTeamID;
		strncpy(ret.pstrName,rem->pstrName,MAX_NAMESIZE);
	}

	/**
	* \brief  回调方法通知所有队员有成员离开
	* \param  member 队员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		if(member.tempid == MEMBER_BEING_OFF)
			return true;

		SceneUser *pUser = SceneUserManager::getMe().
			getUserByTempID(member.tempid);

		if (pUser)
		{
			pUser->sendCmdToMe(&ret,sizeof(ret));
		}
		return true;
	}

};

/**
* \brief  sky 删除成员遍历通知所有队伍成员
*/
bool TeamManager::T_DelTeamExec(const Cmd::stRemoveTeamMemberUserCmd *rev)
{
	RemoveMemberExec rem(rev);
	team.execEvery(rem);

	return true;
}


/**
 * \brief  sky 跟换队长历通知所有队伍成员
 */
struct newChangeLeaderExec : public TeamMemExec
{

	/// 删除成员消息
	Cmd::stRemoveTeamChangeLeaderUserCmd ret;


	/**
	* \brief  构造函数初始化成员删除消息
	* \param  u 队长
	* \param  rem  队员删除通知消息
	*/
	newChangeLeaderExec( const char * LeaderName )
	{
		strncpy(ret.LeaderName ,LeaderName ,MAX_NAMESIZE);
	}

	/**
	* \brief  回调方法通知所有队员有成员离开
	* \param  member 队员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		if(member.tempid == MEMBER_BEING_OFF)
			return true;

		SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
		if (pUser)
		{
			pUser->sendCmdToMe(&ret,sizeof(ret));
		}
		return true;
	}

};

/**
* \brief  sky 删除成员遍历通知所有队伍成员
*/
bool TeamManager::T_ChangeLeaderExec( const char * LeaderName)
{
	newChangeLeaderExec Change(LeaderName);
	team.execEvery(Change);

	return true;
}


/**
 * \brief  sky 比较所有成员的ROLL点数找出最大数的成员地址
 */
struct ComparisonRollnumExec : public TeamMemExec
{

	BYTE rolltype;		//sky 当前最大ROLL类型
	int Maxrollnum;		//sky 当前最大的ROLL点数
	
	TeamMember * maxRollmember;	//sky 在ROLL中胜出的成员地址

	/**
	* \brief  构造函数初始化ROLL保存数据
	*/
	ComparisonRollnumExec()
	{
		rolltype = 0;
		Maxrollnum = 0;
		maxRollmember = NULL;
	}

	/**
	* \brief  回调方法计算出所有成员的随机数
	* \param  member 队员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		//sky 如果是放弃或者是被排除就丢弃 未选择只是就说明时间已经到拉也认为是放弃
		if( member.tempid == MEMBER_BEING_OFF || member.rollitem == Roll_GiveUp || member.rollitem == Roll_Null || member.rollitem == Roll_Exclude )
		{
			return true;
		}

		//sky 如果是贪婪或者需求就计算随机数并每次保留最大的数字和最大的ROLL模式还有成员地址
		if( member.rollitem == Roll_Greed || member.rollitem == Roll_Need )
		{
			if( member.rollitem > rolltype )
			{
				rolltype = member.rollitem;
				Maxrollnum = member.rollnum;

				maxRollmember = &member;
			}
			else if( member.rollitem == rolltype && member.rollnum >= Maxrollnum )
			{
				Maxrollnum = member.rollnum;
				maxRollmember = &member;
			}
		}
		return true;
	}

};


/**
 * \brief  sky 遍历成员发送ROLL开始的消息
 */
struct NoticeRollExec : public TeamMemExec
{	
	Cmd::stTeamRollItemNoticeUserCmd pCmd;

	/**
	* \brief  构造函数初始化ROLL保存数据
	*/
	NoticeRollExec( zObject * obj )
	{
		memcpy( &(pCmd.object), &(obj->data), sizeof(t_Object),sizeof(pCmd.object) );
	}

	/**
	* \brief  回调方法计算出所有成员的随机数
	* \param  member 队员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		if(member.tempid == MEMBER_BEING_OFF)
			return true;

		SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
		if (pUser)
		{
			pUser->sendCmdToMe(&pCmd,sizeof(pCmd));
			//Channel::sendSys( pUser,Cmd::INFO_TYPE_GAME,"为拉 %s 大家一起来掷色子！哈哈",pCmd.object.strName );
		}
		else
		{
			member.rollitem = Roll_Exclude;
		}
		return true;
	}

};

//sky Roll控制函数
bool TeamManager::RollItem_A()
{
	--dwRollTime;

	if( GetNoRollUser() || dwRollTime == 0 ) //sky 判断是否所有队伍成员都已经选择拉ROLL选项
	{
		RollItemPos * p = GetRollItem();

		if(!p)
		{
			DelRoll();
			return false;
		}

		Scene * scene = SceneManager::getInstance().getSceneByTempID(p->SceneID);
		if(!scene)
		{
			DelRoll();
			return false;
		}

		zSceneObject *ret = NULL;
		ret = scene->getSceneObjectByPos( p->ItemPos );

		char msg[MAX_PATH];

		TeamMember * memberBuff = ComparisonRollnum();

		if( !memberBuff )
		{
			if( ret )
			{
				sprintf(msg, "【ROLL信息】:全部队员都选择的放弃！物品没人获得！" );
				Channel::sendTeam(getTeamtempId(), msg);

				//sky 既然队伍里的人都不要这个物品就清除掉这个物品的保护
				ret->DelProtectOverdue();
				Cmd::stClearObjectOwnerMapScreenUserCmd  eret;
				eret.dwMapObjectTempID=ret->id;
				scene->sendCmdToNine(ret->getPosI(),&eret,sizeof(eret),ret->dupIndex);

				//sky 清掉队长的相关ROLL信息
				DelRoll();
			}
			else
			{
				//sky 物品都没拉更加要清掉队长的相关ROLL信息撒`=。=|||
				DelRoll();
			}
		}
		else
		{
			if (ret)
			{
				zObject *o=ret->getObject();

				if (!o)
				{
					//sky 清掉队长的相关ROLL信息
					DelRoll();
					return false;
				}
				else
				{
					SceneUser * rolluser = SceneUserManager::getMe().getUserByTempID( memberBuff->tempid );

					if(rolluser)
					{
						sprintf(msg, "【ROLL信息】:%s ROLL胜利 获取物品:%s", rolluser->charbase.name, o->data.strName );
						Channel::sendTeam(getTeamtempId(), msg);

						if(rolluser->packs.uom.space(rolluser) >= 1)
						{
							//sky 通知客户端跟新地面物品被删除
							Cmd::stRemoveMapObjectMapScreenUserCmd re;
							re.dwMapObjectTempID=ret->id;
							scene->sendCmdToNine(ret->getPosI(),&re,sizeof(re), ret->dupIndex);

							//sky 先冲地面上删除这个物品
							scene->removeObject(ret);

							//sky 在把他添加到玩家的包袱中
							rolluser->packs.addObject( o, true, AUTO_PACK );

							//sky 通知玩家客户端跟新包袱里的物品信息
							Cmd::stAddObjectPropertyUserCmd send;
							bcopy(&o->data,&send.object,sizeof(t_Object),sizeof(send.object));
							rolluser->sendCmdToMe(&send,sizeof(send));

							//sky 清掉队长的相关ROLL信息
							DelRoll();
						}
						else
						{
							Channel::sendSys(rolluser,Cmd::INFO_TYPE_FAIL,"包裹里没有空位置拉!无法获取ROLL胜利物品");

							//sky 修改物品的保护对象为ROLL的胜利者并重新设置保护时间为60秒
							ret->setOwner( rolluser->getID(), 60 );

							//sky 通知客户端全场景修改保护对象
							Cmd::stClearObjectOwnerMapScreenUserCmd  eret;
							eret.dwMapObjectTempID=ret->id;
							eret.tempid = rolluser->tempid;
							strncpy(eret.strName, rolluser->charbase.name, (MAX_NAMESIZE+1));
							scene->sendCmdToNine(ret->getPosI(),&eret,sizeof(eret),ret->dupIndex);

							DelRoll();

						}
					}
					else
					{
						//sky 清掉队长的相关ROLL信息
						DelRoll();
						sprintf(msg, "【ROLL信息】:ROLL胜利者离线！请重新ROLL物品!!");
						Channel::sendTeam(getTeamtempId(), msg);
					}
				}
			}
		}
		//sky 最后不管什么情况都把ROLL信息清掉
		DelRoll();
	}

	return true;
}


/**
 * \brief  sky 返回队伍中在线的人数
 */
struct GetTeamMemberNumExec : public TeamMemExec
{	
	int MemberNum;

	GetTeamMemberNumExec( )
	{
		MemberNum = 0;
	}

	/**
	* \brief  回调方法计算出在线成员数
	* \param  member 队员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		if(member.tempid == MEMBER_BEING_OFF)
			return true;

		SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
		if (pUser)
		{
			MemberNum++;
		}
		return true;
	}

};

/**
 * \brief  sky 查看是否还有人没有选择ROLL选项
 */
struct GetNoRollUserExec : public TeamMemExec
{	
	bool NoRollUser;
	/**
	* \brief  构造函数初始化ROLL保存数据
	*/
	GetNoRollUserExec( )
	{
		NoRollUser = true;
	}

	/**
	* \brief  回调方法计算出在线成员数
	* \param  member 队员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		if( member.tempid != MEMBER_BEING_OFF && member.rollitem == Roll_Null )
			NoRollUser = false;

		return true;
	}

};


/**
 * \brief  sky 根据唯一ID设置队伍中的队员ROLL参数
 */
struct SetMemberRollExec : public TeamMemExec
{	
	DWORD userTempid;
	BYTE  rolltype;

	int  RollNum;
	/**
	* \brief  构造函数初始化ROLL保存数据
	*/
	SetMemberRollExec( DWORD tempid, BYTE type )
	{
		userTempid	= tempid;
		rolltype	= type;
	}

	/**
	* \brief  回调方法设置队伍中的特定队员ROLL参数
	* \param  member 队员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		if( member.tempid != MEMBER_BEING_OFF && member.tempid == userTempid )
		{
			if(rolltype == Roll_Greed || rolltype == Roll_Need )
			{
				RollNum = randBetween(1, 100);
				member.rollnum = RollNum;
				member.rollitem = rolltype;
			}
			else if( rolltype == Roll_GiveUp )
			{
				member.rollitem = rolltype;
			}
			return true;
		}
		return false;
	}

};

/**
 * \brief  在队伍中踢人
 * \param  pUser 队长
 * \param rev 队员删除消息
 */
void TeamManager::kickoutMember(SceneUser *pUser,Cmd::stRemoveTeamMemberUserCmd *rev)
{
	RemoveMemberExec rem(rev);
	team.execEvery(rem);

	SceneUser * LpUser = SceneUserManager::getMe().getUserByName(rev->pstrName);

	if(pUser)
	{
		team.removeMemberByTempID(LpUser->tempid);

		if (LpUser)
		{
			LpUser->TeamThisID = 0;
			LpUser->reSendMyMapData();
			// Session队伍
			delMemberToSession(LpUser->name);
		}
	}
}

/**
 * \brief  响应消息删除队伍成员
 * \param  pUser 被删除的成员
 * \param  rev 删除队伍成员消息
 */
void TeamManager::removeMember(Cmd::stRemoveTeamMemberUserCmd *rev)
{
	//sky 退出前先看看队伍中还有没有人
	if (getSize() <= 2)
	{
		//sky 如果没人拉就直接把队伍给删除掉
		SceneManager::getInstance().SceneDelTeam(getTeamtempId());

		SceneUser * pUser = SceneUserManager::getMe().getUserByName(rev->pstrName);
		if(pUser)
			pUser->TeamThisID = 0;

		return;
	}

	//sky 删除队员的事情Scene就不处理拉,统一丢给Session去处理,保持统一性
	delMemberToSession(rev->pstrName);
}

/**
 * \brief  删除队伍成员
 * \param  mem 被删除成员
 */
void TeamManager::removeMember(SceneUser *mem)
{
  //TODO 删除队友
  Cmd::stRemoveTeamMemberUserCmd ret;
  ret.dwTeamID = getTeamtempId();
  strncpy(ret.pstrName, mem->name, MAX_NAMESIZE);
  removeMember(&ret);
}        

/**
 * \brief  遍历
 * \param  callback 回调方法
 */
void TeamManager::execEveryOne(TeamMemExec &callback)
{
  team.execEvery(callback);
}

/**
 * \brief  遍历除了特定成员以外的所有队员
 * \param  callback 回调方法
 * \param  tempid 被排除队员的 id
 */
void TeamManager::execEveryOneExceptMe(TeamMemExec &callback,DWORD tempid)
{
  team.execEveryExceptMe(callback,tempid);
}

/**
 * \brief  根据id增加一个新的成员
 * \param  pUser 角色指针,无特定要求
 * \param  id 新增成员的 id
 * \return true 增加成功 false 增加失败
 */
bool TeamManager::addMemberByID(SceneUser *pUser,DWORD id)
{
  if (pUser)
  {
    SceneUser *u = SceneUserManager::getMe().getUserByID(id);
    if (u)
    {
      if (team.addMember(id,u->tempid,u->name))
      {
        return true;
      }
    }
  }
  return false;
}

/**
* \brief  sky 添加跨场景成员
* \param  AwayUser 成员的基本数据
* \param  leaber 是否是队长
*/
bool TeamManager::addWAwayMember(Cmd::Session::stMember * AwayUser)
{
	if(AwayUser)
	{
		TeamMember member;
		member.id = AwayUser->dwID;
		SceneUser * pUser = SceneUserManager::getMe().getUserByID(AwayUser->dwID);

		if(pUser)
		{
			member.tempid = pUser->tempid;
			strncpy(member.name, AwayUser->name, MAX_NAMESIZE);

			team.member.push_back(member);

			AddNewMemberExec add(this, pUser);
			team.execEvery(add);
		}
		else
		{
			member.tempid = MEMBER_BEING_OFF;

			strncpy(member.name, AwayUser->name, MAX_NAMESIZE);

			team.member.push_back(member);

			AddAwayNewMemberExec add(this, AwayUser);
			team.execEvery(add);
		}

		return true;
	}

	return false;
}

/**
 * \brief  遍历检查离线成员状态,未离线成员计算离线时间,超过规定时间的离线成员从队伍中删除
 */
struct CheckOfflineExec : public TeamMemExec
{
	/// 队长
	SceneUser *leader;

	/// 类型定义
	typedef std::vector<Cmd::stRemoveTeamMemberUserCmd> Remove_vec;

	/// 类型定义
	typedef Remove_vec::iterator Remove_vec_iterator;

	/// 清除管理器
	Remove_vec del_vec;

	/// 类型定义
	typedef std::vector<DWORD> Online_vec;

	/// 定义迭代指针
	typedef Online_vec::iterator Online_vec_iterator;

	/// 重新上线管理器
	Online_vec add_vec;

	/**
	* \brief  构造函数初始化属性
	* \param  u 队长
	*/
	CheckOfflineExec(SceneUser *u)
	{
		leader = u;
	}

	/**
	* \brief  遍历处理离线成员
	* \param  member 成员
	* \return true 继续遍历 false 终止遍历
	*/
	bool exec(TeamMember &member)
	{
		SceneUser *pUser = SceneUserManager::getMe().
			getUserByID(member.id);
		if (pUser)
		{
			if (!pUser->TeamThisID != 0)
			{
				add_vec.push_back(pUser->id);
			}
		}
		else
		{
			member.offtime += 10;
			if (member.offtime >= 120)
			{
				//TODO 删除队友
				Cmd::stRemoveTeamMemberUserCmd ret;
				ret.dwTeamID = leader->TeamThisID;
				strncpy(ret.pstrName ,member.name, MAX_NAMESIZE);
				del_vec.push_back(ret);
			}
		}
		return true;
	}
};

/**
 * \brief  检查离线成员状态,删除超时的,恢复上线的
 * \param  pUser 队长
 */
void TeamManager::checkOffline(SceneUser *pUser)
{
  CheckOfflineExec exec(pUser);
  team.execEvery(exec);
  if (!exec.del_vec.empty())
  {
    for(CheckOfflineExec::Remove_vec_iterator del_iter=exec.del_vec.begin();del_iter != exec.del_vec.end();del_iter ++)
    {
      removeMember(&*del_iter);
    }
    exec.del_vec.clear();
    if (getSize() == 1)
    {
      deleteTeam();
    }
  }
  if (!exec.add_vec.empty())
  {
    for(CheckOfflineExec::Online_vec_iterator add_iter=exec.add_vec.begin();add_iter !=exec.add_vec.end();add_iter ++)
    {
      SceneUser *tuser = SceneUserManager::getMe().
        getUserByID(*add_iter);
      if (tuser)
      {
        addNewMember(pUser,tuser);
      }
    }
    exec.add_vec.clear();
  }
}

/**
 * \brief  遍历队伍查找一个现任队长以外的成员使其成为新队长
 */
//struct FindLeaderExec : public TeamMemExec
//{
//  /// 现任队长
//  SceneUser *leader;
//
//  /// 新队长
//  SceneUser *newleader;
//
//  /**
//   * \brief  构造函数初始化变量
//   * \param  u 当前队长
//   */
//  FindLeaderExec(SceneUser *u)
//  {
//    leader = u;
//    newleader = NULL;
//  }
//
//  /**
//   * \brief  遍历查找新的队长
//   * \param  member 成员
//   * \return true 表示继续遍历 false 终止遍历
//   */
//  bool exec(TeamMember &member)
//  {
//    SceneUser *pUser = SceneUserManager::getMe().
//      getUserByTempID(member.tempid);
//    if (pUser && pUser->tempid != leader->tempid)
//    {
//      newleader = pUser;
//      return true;
//    }
//    else
//    {
//      return false;
//    }
//    return true;
//  }
//};
//
///**
// * \brief  查找一个新的队长
// * \param  pUser 当前队长
// * \return 新的队长对象角色指针
// */
//SceneUser *TeamManager::findNewLeader(SceneUser *pUser)
//{
//  FindLeaderExec exec(pUser);
//  team.execEvery(exec);
//  return exec.newleader;
//}

/**
 * \brief  SKY 改变队长函数
 * \param  pUser 当前队长
 * \param  pNewUse 指定跟换的队长 
 * \return true 改变成功  false 改变失败
 */
bool TeamManager::changeLeader(char * NewLeaberName)
{
	// 给Session发送队长跟换的消息
	if(NewLeaberName)
		ChangeLeaderToSession(NewLeaberName);
	else
		ChangeLeaderToSession();

	return true;
}

/**
 * \brief  SKY 通知所有队伍成员开始ROLL
 * \param  obj 被争夺的物品
 * \return true 改变成功  false 改变失败
 */
bool TeamManager::NoticeRoll(zObject * obj)
{
	if ( obj )
	{
		NoticeRollExec rem( obj );
		team.execEvery(rem);

		return true;
	}
	return false;
}

/**
 * \brief  sky 返回队伍中在线的人数处理函数
 */
int TeamManager::GetTeamMemberNum()
{
	GetTeamMemberNumExec rem;
	team.execEvery(rem);

	return rem.MemberNum;
}

/**
 * \brief  sky 查看是否还有人没有选择ROLL选项
 */
bool TeamManager::GetNoRollUser()
{
	GetNoRollUserExec rem;
	team.execEvery(rem);

	return rem.NoRollUser;
}

/**
 * \brief  sky 根据唯一ID设置队伍中的队员ROLL参数
 */
int TeamManager::SetMemberRoll( DWORD tempid, BYTE rolltype )
{
	SetMemberRollExec rem(tempid, rolltype);
	team.execEvery(rem);

	return rem.RollNum;
}

/**
 * \brief  sky 全部队员选择ROLL完毕后开始计算ROLL最大的队员
 */
TeamMember * TeamManager::ComparisonRollnum()
{
	ComparisonRollnumExec rem;
	team.execEvery(rem);

	return rem.maxRollmember;
}

/**
* \brief   给某个单独的队员发送整个队伍的成员消息
*/
struct SendMemberDataExec : public TeamMemExec
{
	/// 接受消息的成员
	SceneUser *nm;

	DWORD LeaberID;

	/// 队员新增消息实例1
	Cmd::stAddTeamMemberUserCmd ret_1;

	/**
	* \brief  构造初始化消息实例
	* \param  u 队长角色对象
	* \param n 新增队员角色对象
	*/
	SendMemberDataExec(SceneUser *n, DWORD leaberID)
	{
		LeaberID = leaberID;
		nm = n;
		TeamManager * team = SceneManager::getInstance().GetMapTeam(nm->TeamThisID);
		if(team)
		{
			ret_1.dwTeamID = team->getTeamtempId(); //getLeader();
			ret_1.dwHeadID = team->getLeader();
		}
		else
		{
			ret_1.dwTeamID = 0;
			ret_1.dwHeadID = 0;
		}
	}

	/**
	* \brief  回调方法将新成员发送给每个队员,将每个队员发送给新成员
	* \param  member 队员
	* \return false 终止遍历 true 继续遍历
	*/
	bool exec(TeamMember &member)
	{
		if(ret_1.dwTeamID != 0)
		{
			SceneUser * pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
			if(pUser)
			{
				strncpy(ret_1.data.pstrName,pUser->name,MAX_NAMESIZE);
				if (LeaberID == pUser->id)
				{
					ret_1.data.byHead = true;
				}
				else
				{
					ret_1.data.byHead = false;
				}
				ret_1.data.dwTempID = pUser->tempid;
				ret_1.data.dwMaxHealth = pUser->charstate.maxhp;
				ret_1.data.dwHealth = pUser->charbase.hp;
				ret_1.data.dwMaxMp = pUser->charstate.maxmp;
				ret_1.data.dwMp = pUser->charbase.mp;
				ret_1.data.wdFace = pUser->charbase.face;
			}
			else
			{
				if (LeaberID == member.id)
				{
					ret_1.data.byHead = true;
				}
				else
				{
					ret_1.data.byHead = false;
				}
				strncpy(ret_1.data.pstrName,member.name,MAX_NAMESIZE);
				ret_1.data.dwTempID = MEMBER_BEING_OFF;
				ret_1.data.dwMaxHealth = 1;
				ret_1.data.dwHealth = 1;
				ret_1.data.dwMaxMp = 1;
				ret_1.data.dwMp = 1;
				ret_1.data.wdFace = nm->charbase.face;
			}

			nm->sendCmdToMe(&ret_1,sizeof(ret_1));
		}
		
		return true;
	}
};

/**
* \brief  sky 设置队员的存在状态
* \memberID 要设置的队员的ID
* \leaberID 队长ID
* \Being true:设置为存在状态 false:设置为不存在的状态
*/
bool TeamManager::SetMemberType(DWORD memberID, DWORD leaberID, bool Being)
{
	int size = team.member.size();

	for(int i=0; i<size; i++)
	{
		if(team.member[i].id == memberID)
		{
			if(Being)
			{
				if(team.member[i].tempid == MEMBER_BEING_OFF)
				{
					SceneUser * pUser = SceneUserManager::getMe().getUserByID(memberID);
					if(pUser)
					{
						team.member[i].tempid = pUser->tempid;
						SendMemberDataExec Send(pUser, leaberID);
						team.execEvery(Send);
						return true;
					}
					else
						return false;
				}
			}
			else
			{
				if(team.member[i].tempid != MEMBER_BEING_OFF)
				{
					team.member[i].tempid = MEMBER_BEING_OFF;
					team.member[i].rollitem = 0;
					team.member[i].rollnum = 0;
					team.member[i].begintime = 0;
					team.member[i].offtime = 0;
					return true;
				}
			}
		}
	}

	return false;
}

/**
 * \brief  根据临时id增加队伍成员
 * \param  pUser 未使用
 * \param tempid 新增加成员的临时id
 * \return true 增加成功 false 增加失败
 */
bool TeamManager::addMemberByTempID(SceneUser *pUser,DWORD tempid)
{
  if (pUser)
  {
    SceneUser *u = SceneUserManager::getMe().
            getUserByTempID(tempid);
    if (u )
    {
      if (team.addMember(u->id,tempid,u->name))
      {
        return true;
      }
    }
  }
  return false;
}

/**
 * \brief  根据id 删除成员
 * \param  id 角色的id
 */
void TeamManager::removeMemberByID(DWORD id)
{
  team.removeMemberByID(id);
}

/**
 * \brief 根据临时id删除成员 
 * \param  tempid 临时id
 */
void TeamManager::removeMemberByTempID(DWORD tempid)
{
  team.removeMemberByTempID(tempid);
}

/**
 * \brief 请求友好度关系
 */
void TeamManager::requestFriendDegree()
{
  team.requestFriendDegree();
}

/**
 * \brief  发送当前友好度关系列表到会话存档
 */
void TeamManager::sendFriendDegreeToSession()
{
  /*BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::Session::stCountMember * temp = NULL;

  Cmd::Session::t_CountFriendDegree_SceneSession *retCmd=(Cmd::Session::t_CountFriendDegree_SceneSession *)buf;
  strncpy(retCmd->name,me->name,MAX_NAMESIZE);
  constructInPlace(retCmd);
  retCmd->size = 0;
  temp = retCmd->namelist;

  if (!friendList.empty())
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = friendList.begin(); sIterator!= friendList.end(); sIterator++)
    {
      temp->dwUserID = sIterator->second.dwUserID;
      temp->wdDegree = sIterator->second.wdDegree;
      temp->byType   = Cmd::RELATION_TYPE_FRIEND;
      retCmd->size++;
      temp++;
    }
  }

  if (!consortList.empty())
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = consortList.begin(); sIterator!= consortList.end(); sIterator++)
    {
      temp->dwUserID = sIterator->second.dwUserID;
      temp->wdDegree = sIterator->second.wdDegree;
      temp->byType   = Cmd::RELATION_TYPE_LOVE;
      retCmd->size++;
      temp++;
    }
  }

  if (!teacherList.empty())
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = teacherList.begin(); sIterator!= teacherList.end(); sIterator++)
    {
      temp->dwUserID = sIterator->second.dwUserID;
      temp->wdDegree = sIterator->second.wdDegree;
      temp->byType   = Cmd::RELATION_TYPE_TEACHER;
      retCmd->size++;
      temp++;
    }
  }

  if (!prenticeList.empty())
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = prenticeList.begin(); sIterator!= prenticeList.end(); sIterator++)
    {
      temp->dwUserID = sIterator->second.dwUserID;
      temp->wdDegree = sIterator->second.wdDegree;
      temp->byType   = Cmd::RELATION_TYPE_TEACHER;
      retCmd->size++;
      temp++;
    }
  }
  if (0 != retCmd->size) sessionClient->sendCmd(retCmd,sizeof(Cmd::Session::t_CountFriendDegree_SceneSession)+retCmd->size*sizeof(Cmd::Session::stCountMember));*/
}

/**
 * \brief  设置友好度流程把当前的友好度值替换到发过来的关系清单中,因为当前值是最新的,
       然后清空之前的关系列表,再把收到的关系清单插入到关系列表中
 * \param  rev 友好度关系列表消息
 */
void TeamManager::setFriendDegree(Cmd::Session::t_ReturnFriendDegree_SceneSession *rev)
{
	relationlock.wrlock();

	if (NULL == rev)
	{
		friendList.clear();   // 朋友列表
		consortList.clear();  // 夫妻列表
		teacherList.clear();  // 师傅列表
		prenticeList.clear(); // 徒弟列表

	}
	else if (0 == rev->size)
	{
		friendList.clear();   // 朋友列表
		consortList.clear();  // 夫妻列表
		teacherList.clear();  // 师傅列表
		prenticeList.clear(); // 徒弟列表
	}
	else
	{
		std::list<stTempDegreeMember> templist;

		for(int i=0; i<rev->size; i++)
		{
			stTempDegreeMember unit;
			unit.dwUserID = rev->memberlist[i].dwUserID;
			unit.wdDegree = rev->memberlist[i].wdDegree;
			unit.byType   = rev->memberlist[i].byType;
			unit.wdTime   = 0;
			templist.push_back(unit);
		}

		if (!friendList.empty())   // 朋友列表
		{
			std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
			for(sIterator = friendList.begin(); sIterator!= friendList.end(); sIterator++)
			{
				std::list<stTempDegreeMember>::iterator tIterator;
				for(tIterator = templist.begin(); tIterator!=templist.end(); tIterator++)
				{
					if ((sIterator->second.dwUserID == tIterator->dwUserID) &&
						(Cmd::Session::TYPE_FRIEND == tIterator->byType))
					{
						tIterator->wdDegree = sIterator->second.wdDegree;
						tIterator->wdTime    = sIterator->second.wdTime;
					}
				}
			}
		}
		if (!consortList.empty())  // 夫妻列表
		{
			std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
			for(sIterator = consortList.begin(); sIterator!= consortList.end(); sIterator++)
			{
				std::list<stTempDegreeMember>::iterator tIterator;
				for(tIterator = templist.begin(); tIterator!=templist.end(); tIterator++)
				{
					if ((sIterator->second.dwUserID == tIterator->dwUserID) &&
						(Cmd::Session::TYPE_CONSORT == tIterator->byType))
					{
						tIterator->wdDegree = sIterator->second.wdDegree;
						tIterator->wdTime    = sIterator->second.wdTime;
					}
				}
			}
		}
		if (!teacherList.empty())  // 师傅列表
		{
			std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
			for(sIterator = teacherList.begin(); sIterator!= teacherList.end(); sIterator++)
			{
				std::list<stTempDegreeMember>::iterator tIterator;
				for(tIterator = templist.begin(); tIterator!=templist.end(); tIterator++)
				{
					if ((sIterator->second.dwUserID == tIterator->dwUserID) &&
						(Cmd::Session::TYPE_TEACHER == tIterator->byType))
					{
						tIterator->wdDegree = sIterator->second.wdDegree;
						tIterator->wdTime    = sIterator->second.wdTime;
					}
				}
			}
		}
		if (!prenticeList.empty()) // 徒弟列表
		{
			std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
			for(sIterator = prenticeList.begin(); sIterator!= prenticeList.end(); sIterator++)
			{
				std::list<stTempDegreeMember>::iterator tIterator;
				for(tIterator = templist.begin(); tIterator!=templist.end(); tIterator++)
				{
					if ((sIterator->second.dwUserID == tIterator->dwUserID) &&
						(Cmd::Session::TYPE_PRENTICE == tIterator->byType))
					{
						tIterator->wdDegree = sIterator->second.wdDegree;
						tIterator->wdTime    = sIterator->second.wdTime;
					}
				}
			}
		}

		friendList.clear();   // 朋友列表
		consortList.clear();  // 夫妻列表
		teacherList.clear();  // 师傅列表
		prenticeList.clear(); // 徒弟列表

		std::list<stTempDegreeMember>::iterator tIterator;
		for(tIterator = templist.begin(); tIterator!=templist.end(); tIterator++)
		{
			stDegreeMember member;

			member.dwUserID = tIterator->dwUserID;
			member.wdDegree = tIterator->wdDegree;
			member.wdTime   = tIterator->wdTime;
			switch(tIterator->byType)
			{
			case Cmd::Session::TYPE_FRIEND:
				{
					friendList.insert(insValueType(member.wdDegree,member));
				}
				break;
			case Cmd::Session::TYPE_CONSORT:
				{
					consortList.insert(insValueType(member.wdDegree,member));
				}
				break;
			case Cmd::Session::TYPE_TEACHER:
				{
					teacherList.insert(insValueType(member.wdDegree,member));
				}
				break;
			case Cmd::Session::TYPE_PRENTICE:
				{
					prenticeList.insert(insValueType(member.wdDegree,member));
				}
				break;
			default:
				break;
			}
		}
	}
	relationlock.unlock();
}


/**
 * \brief  实时累加计算所有关系的友好度关系
 */
void TeamManager::countFriendDegree()
{
  if (0 == team.leader) return;

  bool oper = false;

  relationlock.rdlock();
  if (!friendList.empty())   // 朋友列表
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = friendList.begin(); sIterator!= friendList.end(); sIterator++)
    {
      sIterator->second.wdTime++;
      if (60 <= sIterator->second.wdTime)
      {
        sIterator->second.wdTime=0;
        sIterator->second.wdDegree+=60;
        oper = true;
      }
    }
  }
  if (!consortList.empty())  // 夫妻列表
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = consortList.begin(); sIterator!= consortList.end(); sIterator++)
    {
      sIterator->second.wdTime++;
      if (60 <= sIterator->second.wdTime)
      {
        sIterator->second.wdTime=0;
        sIterator->second.wdDegree+=60;
        oper = true;
      }
    }
  }
  if (!teacherList.empty())  // 师傅列表
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = teacherList.begin(); sIterator!= teacherList.end(); sIterator++)
    {
      sIterator->second.wdTime++;
      if (60 <= sIterator->second.wdTime)
      {
        sIterator->second.wdTime=0;
        sIterator->second.wdDegree+=60;
        oper = true;
      }
    }
  }
  if (!prenticeList.empty()) // 徒弟列表
  {
    std::map<WORD,struct stDegreeMember,ltword>::iterator sIterator;
    for(sIterator = prenticeList.begin(); sIterator!= prenticeList.end(); sIterator++)
    {
      sIterator->second.wdTime++;
      if (60 <= sIterator->second.wdTime)
      {
        sIterator->second.wdTime=0;
        sIterator->second.wdDegree+=60;
        oper = true;
      }
    }
  }
  if (oper) sendFriendDegreeToSession();
  relationlock.unlock();
}

/**
 * \brief  根据临时存档数据加载队伍
 * \param  leader 队长
 * \param  data 临时存档对象
 * \return true 加载成功  false 加载失败
 */
bool TeamManager::loadTeam(SceneUser *leader,TempArchiveMember *data)
{
  ////本地图不能组队
  //if (leader->scene->noTeam())
  //{
  //  return false;
  //}

  //int size = data->size - sizeof(DWORD) - sizeof(DWORD) - sizeof(BYTE) - sizeof(BYTE);
  //leader->team_mode = *(DWORD*)&data->data[size];
  //Cmd::stTeamModeUserCmd tmu; 
  //tmu.byType=leader->team_mode;
  //leader->sendCmdToMe(&tmu,sizeof(tmu));
  //leader->team->setNextObjOwnerID(*(DWORD*)&data->data[size+sizeof(DWORD)]);
  //leader->team->setObjMode(*(BYTE*)&data->data[size+sizeof(DWORD)+sizeof(DWORD)]);
  //leader->team->setExpMode(*(BYTE*)&data->data[size+sizeof(DWORD)+sizeof(DWORD)+sizeof(BYTE)]);
  //struct PairIDAndName
  //{
  //  DWORD id;
  //  char name[MAX_NAMESIZE];
  //};
  //PairIDAndName *pair = NULL;
  //while((int)size > 0)
  //{
  //  pair = (PairIDAndName*)&data->data[data->size -sizeof(DWORD) - sizeof(DWORD) - sizeof(BYTE) - sizeof(BYTE) - size]; 
  //  size -= sizeof(PairIDAndName);
  //  SceneUser *u = SceneUserManager::getMe().
  //    getUserByID(pair->id);
  //  if (u)
  //  {
  //    if (!u->team->IsTeamed())
  //    {
  //      //Zebra::logger->debug("队伍%u恢复成员找到成员,id=%u,tempid=%u,name=%s",leader->tempid,u->id,u->tempid,u->name);
  //      if (leader->team->IsFull())
  //      {
  //        break;
  //      }
  //      addNewMember(leader,u);
  //    }
  //    //team->addMember(u->id,u->tempid,u->name);
  //  }
  //  else if (pair->id)
  //  {
  //    team.addMember(pair->id,0,pair->name);
  //  }
  //  //Zebra::logger->debug("恢复队伍成员,id=%u,name=%s",pair->id,pair->name);
  //  //size -= (int)((size - sizeof(DWORD)) > 0) ? size - sizeof(DWORD) : 0;
  //}
  //if (leader->team->getSize() == 1)
  //{
  //  leader->team->deleteTeam(leader);
  //}
  ////Zebra::logger->debug("load队伍成员数量=%u",leader->team->getSize());
  return true;
}

/**
 * \brief  增加一个成员到会话
 * \param  userid 增加的成员的 id
 * \return true 为消息发送成功 false 消息发送失败
 */
bool TeamManager::addMemberToSession(char * userName)
{
  Cmd::Session::t_Team_AddMember add;
  add.dwTeam_tempid = getTeamtempId();

  SceneUser * leader = SceneUserManager::getMe().getUserByTempID(this->getLeader());

  if(leader)
	  add.dwLeaderID = leader->id;
  else
	  return false;

  strncpy(add.AddMember.name, userName, MAX_NAMESIZE);
  sessionClient->sendCmd(&add,sizeof(Cmd::Session::t_Team_AddMember));
  return true;
}

/**
 * \brief  向会话发送成员删除消息
 * \param  leaderid 队长的id
 * \param  userid 被删除角色的id
 * \return true 为消息发送成功 false 消息发送失败
 */
bool TeamManager::delMemberToSession(char * memberName)
{
  Cmd::Session::t_Team_DelMember del;
  del.dwTeam_tempid = getTeamtempId();
  strncpy(del.MemberNeam, memberName, MAX_NAMESIZE);
  sessionClient->sendCmd(&del,sizeof(Cmd::Session::t_Team_DelMember));
  return true;
}
void TeamManager::decreaseAverageExp(SceneUser *pUser)
{
  team.decreaseAverageExp(pUser);
}

/**
 * \brief  向会话发送队长跟换的消息
 * \param  leaderid 老队长的id
 * \param  newleaderid 新队长的的id
 * \return true 为消息发送成功 false 消息发送失败
 */
bool TeamManager::ChangeLeaderToSession(char * NewLeaberName)
{
  Cmd::Session::t_Team_ChangeLeader change;
  change.dwTeam_tempid = getTeamtempId();
  if(NewLeaberName)
	  strncpy(change.NewLeaderName, NewLeaberName, MAX_NAMESIZE);

  sessionClient->sendCmd(&change,sizeof(Cmd::Session::t_Team_ChangeLeader));
  return true;
}

struct ExpSizeExec : public TeamMemExec
{
  int size;
  zPosI pos;
  DWORD sceneid;
  ExpSizeExec(zPosI p,DWORD s)
  {
    pos = p;
    size = 0;
    sceneid = s;
  }
  bool exec(TeamMember &member)
  {
    SceneUser *pUser = SceneUserManager::getMe().
            getUserByTempID(member.tempid);
    if (pUser)
    {
      if (pUser->scene->id == sceneid && pUser->scene->checkTwoPosIInNine(pUser->getPosI(),pos))
      {
        //Zebra::logger->debug("检查9屏成员坐标成功(%d,%d)(%d,%d)",pos,pUser->getPosI(),pUser->getPos().x,pUser->getPos().y);
        size++;
      }
      else
      {
        //Zebra::logger->debug("检查9屏成员坐标失败(%d,%d)(%d,%d)",pos,pUser->getPosI(),pUser->getPos().x,pUser->getPos().y);
      }
    }
    return true;
  }
};
int TeamManager::getExpSize(zPosI pos,DWORD sceneid)
{
  ExpSizeExec exec(pos,sceneid);
  team.execEvery(exec);
  return exec.size;
}
void TeamManager::calAverageExp(zPosI pos,DWORD sceneid)
{
	team.calAverageExp(pos,sceneid);
}
