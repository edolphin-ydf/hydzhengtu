#include <zebra/ScenesServer.h>

/**
 * \brief  ������ӽ�ָ��������Աuser�����ݷ��͸��������г�Ա�Ŀͻ���
 */
struct SendTeamDataExec : public TeamMemExec
{

  /// �ӳ�
  SceneUser *leader;

  /// ������ӳ�Ա��Ϣ����
  Cmd::stFreshTeamMemberUserCmd ret;

  /**
   * \brief  ���캯����ʼ����ʼ����Ҫ���͵���Ϣ
   * \param  l �ӳ���ָ��
   * \param  user ������Ա�Ķ���ָ��
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
   * \brief  �ص�����
   * \param  member ������Ա
   * \return false ������ֹ true �������
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
 * \brief  �������ָ����Ա����
 * \param  leader �ӳ�
 * \param  user ��Ա
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
 * \brief  �������ָ����Ա����
 * \param  user ��Ҫ���͵ĳ�Ա����
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
 * \brief  �����ṹ,��������������ж�Ա�㲥��Ϣ
 */
struct SendCmdExec : public TeamMemExec
{
  /// ��Ҫ�㲥����Ϣ
  void *cmd;

  /// ��Ϣ����
  DWORD cmdLen;

  /**
   * \brief  �����ʼ������
   * \param  data ��Ϣ��
   * \param  dataLen ��Ϣ����
   */
  SendCmdExec(void *data,DWORD dataLen)
  {
    cmd = data;
    cmdLen = dataLen;
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
      pUser->sendCmdToMe(cmd,cmdLen);
      return true;
    }
    return true;
  }
};

/**
 * \brief  ������Ϣ�����
 * \param  user   ��Ϣ������
 * \param  cmd    ��Ϣ��
 * \param  cmdLen ��Ϣ����
 */
void TeamManager::sendCmdToTeam(SceneUser *user,void *cmd,DWORD cmdLen)
{
	SendCmdExec exec(cmd,cmdLen);
	execEveryOne(exec);
}

/**
 * \brief  ����������ж�Ա�Ƿ���һ����
 */
struct CheckAllInOneScreenExec : public TeamMemExec
{
  bool isOk;
  SceneUser *leader;

  /**
   * \brief  �����ʼ������
   * \param  data ��Ϣ��
   * \param  dataLen ��Ϣ����
   */
  CheckAllInOneScreenExec(SceneUser *pUser)
  {
    isOk = true;
    leader = pUser;
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
					giveupstatus = false;///��״̬��ʼ����ȥ��
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
 * \brief   ����һ���µĶ�Ա����
 */
struct AddNewMemberExec : public TeamMemExec
{
  /// �³�Ա
  SceneUser *nm;

  DWORD LeaberID;

  /// ��Ա������Ϣʵ��1
  Cmd::stAddTeamMemberUserCmd ret_1;

  /// ��Ա������Ϣʵ��2
  Cmd::stAddTeamMemberUserCmd ret_2;

  /**
   * \brief  �����ʼ����Ϣʵ��
   * \param  u �ӳ���ɫ����
   * \param n ������Ա��ɫ����
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
   * \brief  �ص��������³�Ա���͸�ÿ����Ա,��ÿ����Ա���͸��³�Ա
   * \param  member ��Ա
   * \return false ��ֹ���� true ��������
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
		  Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"%s�������",ret_2.data.pstrName);
		  nm->sendCmdToMe(&ret_1,sizeof(ret_1));
		  //Zebra::logger->debug("����(%ld)������Ӷ�Աָ��(%s,%ld)",ret_2.dwTeamID,nm->name,nm->id);
		  return true;
	  }
	  return true;
  }
};


/**
* \brief   sky ����һ���µĿ糡����Ա����
*/
struct AddAwayNewMemberExec : public TeamMemExec
{

	/// �µĿ糡����Ա
	Cmd::Session::stMember * AddUser;

	/// ��Ա������Ϣʵ��1
	Cmd::stAddTeamMemberUserCmd ret_1;

	/**
	* \brief  �����ʼ����Ϣʵ��
	* \param  team �������
	* \param AwayUser ������ٶ�Ա
	* \param Leaber �Ƿ��Ƕӳ�
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
	* \brief  �ص��������³�Ա���͸�ÿ����Ա,��ÿ����Ա���͸��³�Ա
	* \param  member ��Ա
	* \return false ��ֹ���� true ��������
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
 * \brief  �жϽ�ɫ�Ƿ����Լ��Ķ���
 * \param  pUser ���ж϶���
 * \return true �� false ��
 */
bool TeamManager::IsOurTeam(SceneUser *pUser)
{
	return IsOurTeam(pUser->id);	
}

/**
 * \brief  �жϽ�ɫ�Ƿ����Լ��Ķ���
 * \param  dwID ���ж϶����id
 * \return true �� false ��
 */
bool TeamManager::IsOurTeam(DWORD dwID)
{
	std::vector<TeamMember>::iterator iter;
	//sky �������ѿ��Ƿ����ж��û�����
	for(iter=team.member.begin(); iter!=team.member.end(); iter++)
	{
		if(iter->id == dwID)
			return true;
	}

	return false;
}

/**
 * \brief  ����һ���µĶ�Ա
 * \param  pUser ׼��Ա
 * \param  rev �������Ӧ����Ϣ
 * \return true ����ɹ� false ����ʧ��
 */
bool TeamManager::addNewMember(SceneUser *pUser,Cmd::stAnswerNameTeamUserCmd *rev, Cmd::Session::t_Team_AnswerTeam * rev1)
{
    SceneUser *nm = SceneUserManager::getMe().
            getUserByName(rev->byAnswerUserName);
  if (nm)	//sky ����ڱ������ܹ��ҵ����û�
  {
    if (nm->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER)
        ||  nm->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON))
    {
      return false;
    }

	nm->TeamThisID = getTeamtempId();  //sky �Ѷ����ΨһID�����Ա

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
  else	//���������Ĭ�ϵ������ڶ�������ȥ����
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
 * \brief  ����һ���µĶ�Ա
 * \param  leader �ӳ�
 * \param  pUser ׼��Ա
 * \return true ���ӳɹ�  false ����ʧ��
 */
bool TeamManager::addNewMember(SceneUser *leader,SceneUser *pUser)
{
  if (pUser->issetUState(Cmd::USTATE_TOGETHER_WITH_TIGER)
      ||  pUser->issetUState(Cmd::USTATE_TOGETHER_WITH_DRAGON))
  {
	  return false;
  }

  pUser->TeamThisID = getTeamtempId();  //sky �Ѷ����ΨһID�����Ա

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

//sky ����һ������Ҫ֪ͨsess����Ӷ�Ա����(�糡����)
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
 * \brief ��ȡ��������
 */
int TeamManager::getSize()
{
	return team.getSize();
}

/**
 * \brief  ɾ�����
 * \param  pUser �ӳ�
 * \param  tempid �ӳ�����ʱid
 */
//void TeamManager::removeTeam(SceneUser *pUser,DWORD tempid)
//{
//	DeleteTeamExec DelTeam;
//
//	team.execEvery(DelTeam)
//}

/**
 * \brief  �����ɢ��������ÿ����Ա���˳�����
 */
struct DeleteTeamExec : public TeamMemExec
{
  /// �ӳ�
  //SceneUser *leader;

  /// ɾ�������Ϣ
  Cmd::stRemoveTeamUserCmd ret;

  /**
   * \brief  �����ʼ��
   * \param  u �ӳ�
   */
  DeleteTeamExec(/*SceneUser *u*/)
  {
    //leader = u;
  }

  /**
   * \brief  �ص�����,���Ͷ����ɢ��Ϣ��ÿ����Ա
   * \param  member ��Ա
   * \return true �������� false ��ֹ����
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
      Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"�����ɢ");
    }
    return true;
  }

};

/**
 * \brief  ɾ�����
 * \param  leader �ӳ�
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
 * \brief  ɾ����Ա����֪ͨ���ж����Ա
 */
struct RemoveMemberExec : public TeamMemExec
{
	/// ɾ����Ա��Ϣ
	Cmd::stRemoveTeamMemberUserCmd ret;


	/**
	* \brief  ���캯����ʼ����Աɾ����Ϣ
	* \param  u �ӳ�
	* \param  rem  ��Աɾ��֪ͨ��Ϣ
	*/
	RemoveMemberExec(const Cmd::stRemoveTeamMemberUserCmd *rem)
	{
		ret.dwTeamID = rem->dwTeamID;
		strncpy(ret.pstrName,rem->pstrName,MAX_NAMESIZE);
	}

	/**
	* \brief  �ص�����֪ͨ���ж�Ա�г�Ա�뿪
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
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
* \brief  sky ɾ����Ա����֪ͨ���ж����Ա
*/
bool TeamManager::T_DelTeamExec(const Cmd::stRemoveTeamMemberUserCmd *rev)
{
	RemoveMemberExec rem(rev);
	team.execEvery(rem);

	return true;
}


/**
 * \brief  sky �����ӳ���֪ͨ���ж����Ա
 */
struct newChangeLeaderExec : public TeamMemExec
{

	/// ɾ����Ա��Ϣ
	Cmd::stRemoveTeamChangeLeaderUserCmd ret;


	/**
	* \brief  ���캯����ʼ����Աɾ����Ϣ
	* \param  u �ӳ�
	* \param  rem  ��Աɾ��֪ͨ��Ϣ
	*/
	newChangeLeaderExec( const char * LeaderName )
	{
		strncpy(ret.LeaderName ,LeaderName ,MAX_NAMESIZE);
	}

	/**
	* \brief  �ص�����֪ͨ���ж�Ա�г�Ա�뿪
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
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
* \brief  sky ɾ����Ա����֪ͨ���ж����Ա
*/
bool TeamManager::T_ChangeLeaderExec( const char * LeaderName)
{
	newChangeLeaderExec Change(LeaderName);
	team.execEvery(Change);

	return true;
}


/**
 * \brief  sky �Ƚ����г�Ա��ROLL�����ҳ�������ĳ�Ա��ַ
 */
struct ComparisonRollnumExec : public TeamMemExec
{

	BYTE rolltype;		//sky ��ǰ���ROLL����
	int Maxrollnum;		//sky ��ǰ����ROLL����
	
	TeamMember * maxRollmember;	//sky ��ROLL��ʤ���ĳ�Ա��ַ

	/**
	* \brief  ���캯����ʼ��ROLL��������
	*/
	ComparisonRollnumExec()
	{
		rolltype = 0;
		Maxrollnum = 0;
		maxRollmember = NULL;
	}

	/**
	* \brief  �ص�������������г�Ա�������
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
	*/
	bool exec(TeamMember &member)
	{
		//sky ����Ƿ��������Ǳ��ų��Ͷ��� δѡ��ֻ�Ǿ�˵��ʱ���Ѿ�����Ҳ��Ϊ�Ƿ���
		if( member.tempid == MEMBER_BEING_OFF || member.rollitem == Roll_GiveUp || member.rollitem == Roll_Null || member.rollitem == Roll_Exclude )
		{
			return true;
		}

		//sky �����̰����������ͼ����������ÿ�α����������ֺ�����ROLLģʽ���г�Ա��ַ
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
 * \brief  sky ������Ա����ROLL��ʼ����Ϣ
 */
struct NoticeRollExec : public TeamMemExec
{	
	Cmd::stTeamRollItemNoticeUserCmd pCmd;

	/**
	* \brief  ���캯����ʼ��ROLL��������
	*/
	NoticeRollExec( zObject * obj )
	{
		memccpy( &(pCmd.object), &(obj->data), sizeof(t_Object),sizeof(pCmd.object) );
	}

	/**
	* \brief  �ص�������������г�Ա�������
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
	*/
	bool exec(TeamMember &member)
	{
		if(member.tempid == MEMBER_BEING_OFF)
			return true;

		SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(member.tempid);
		if (pUser)
		{
			pUser->sendCmdToMe(&pCmd,sizeof(pCmd));
			//Channel::sendSys( pUser,Cmd::INFO_TYPE_GAME,"Ϊ�� %s ���һ������ɫ�ӣ�����",pCmd.object.strName );
		}
		else
		{
			member.rollitem = Roll_Exclude;
		}
		return true;
	}

};

//sky Roll���ƺ���
bool TeamManager::RollItem_A()
{
	--dwRollTime;

	if( GetNoRollUser() || dwRollTime == 0 ) //sky �ж��Ƿ����ж����Ա���Ѿ�ѡ����ROLLѡ��
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
				sprintf(msg, "��ROLL��Ϣ��:ȫ����Ա��ѡ��ķ�������Ʒû�˻�ã�" );
				Channel::sendTeam(getTeamtempId(), msg);

				//sky ��Ȼ��������˶���Ҫ�����Ʒ������������Ʒ�ı���
				ret->DelProtectOverdue();
				Cmd::stClearObjectOwnerMapScreenUserCmd  eret;
				eret.dwMapObjectTempID=ret->id;
				scene->sendCmdToNine(ret->getPosI(),&eret,sizeof(eret),ret->dupIndex);

				//sky ����ӳ������ROLL��Ϣ
				DelRoll();
			}
			else
			{
				//sky ��Ʒ��û������Ҫ����ӳ������ROLL��Ϣ��`=��=|||
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
					//sky ����ӳ������ROLL��Ϣ
					DelRoll();
					return false;
				}
				else
				{
					SceneUser * rolluser = SceneUserManager::getMe().getUserByTempID( memberBuff->tempid );

					if(rolluser)
					{
						sprintf(msg, "��ROLL��Ϣ��:%s ROLLʤ�� ��ȡ��Ʒ:%s", rolluser->charbase.name, o->data.strName );
						Channel::sendTeam(getTeamtempId(), msg);

						if(rolluser->packs.uom.space(rolluser) >= 1)
						{
							//sky ֪ͨ�ͻ��˸��µ�����Ʒ��ɾ��
							Cmd::stRemoveMapObjectMapScreenUserCmd re;
							re.dwMapObjectTempID=ret->id;
							scene->sendCmdToNine(ret->getPosI(),&re,sizeof(re), ret->dupIndex);

							//sky �ȳ������ɾ�������Ʒ
							scene->removeObject(ret);

							//sky �ڰ�����ӵ���ҵİ�����
							rolluser->packs.addObject( o, true, AUTO_PACK );

							//sky ֪ͨ��ҿͻ��˸��°��������Ʒ��Ϣ
							Cmd::stAddObjectPropertyUserCmd send;
							bcopy(&o->data,&send.object,sizeof(t_Object),sizeof(send.object));
							rolluser->sendCmdToMe(&send,sizeof(send));

							//sky ����ӳ������ROLL��Ϣ
							DelRoll();
						}
						else
						{
							Channel::sendSys(rolluser,Cmd::INFO_TYPE_FAIL,"������û�п�λ����!�޷���ȡROLLʤ����Ʒ");

							//sky �޸���Ʒ�ı�������ΪROLL��ʤ���߲��������ñ���ʱ��Ϊ60��
							ret->setOwner( rolluser->getID(), 60 );

							//sky ֪ͨ�ͻ���ȫ�����޸ı�������
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
						//sky ����ӳ������ROLL��Ϣ
						DelRoll();
						sprintf(msg, "��ROLL��Ϣ��:ROLLʤ�������ߣ�������ROLL��Ʒ!!");
						Channel::sendTeam(getTeamtempId(), msg);
					}
				}
			}
		}
		//sky ��󲻹�ʲô�������ROLL��Ϣ���
		DelRoll();
	}

	return true;
}


/**
 * \brief  sky ���ض��������ߵ�����
 */
struct GetTeamMemberNumExec : public TeamMemExec
{	
	int MemberNum;

	GetTeamMemberNumExec( )
	{
		MemberNum = 0;
	}

	/**
	* \brief  �ص�������������߳�Ա��
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
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
 * \brief  sky �鿴�Ƿ�����û��ѡ��ROLLѡ��
 */
struct GetNoRollUserExec : public TeamMemExec
{	
	bool NoRollUser;
	/**
	* \brief  ���캯����ʼ��ROLL��������
	*/
	GetNoRollUserExec( )
	{
		NoRollUser = true;
	}

	/**
	* \brief  �ص�������������߳�Ա��
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
	*/
	bool exec(TeamMember &member)
	{
		if( member.tempid != MEMBER_BEING_OFF && member.rollitem == Roll_Null )
			NoRollUser = false;

		return true;
	}

};


/**
 * \brief  sky ����ΨһID���ö����еĶ�ԱROLL����
 */
struct SetMemberRollExec : public TeamMemExec
{	
	DWORD userTempid;
	BYTE  rolltype;

	int  RollNum;
	/**
	* \brief  ���캯����ʼ��ROLL��������
	*/
	SetMemberRollExec( DWORD tempid, BYTE type )
	{
		userTempid	= tempid;
		rolltype	= type;
	}

	/**
	* \brief  �ص��������ö����е��ض���ԱROLL����
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
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
 * \brief  �ڶ���������
 * \param  pUser �ӳ�
 * \param rev ��Աɾ����Ϣ
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
			// Session����
			delMemberToSession(LpUser->name);
		}
	}
}

/**
 * \brief  ��Ӧ��Ϣɾ�������Ա
 * \param  pUser ��ɾ���ĳ�Ա
 * \param  rev ɾ�������Ա��Ϣ
 */
void TeamManager::removeMember(Cmd::stRemoveTeamMemberUserCmd *rev)
{
	//sky �˳�ǰ�ȿ��������л���û����
	if (getSize() <= 2)
	{
		//sky ���û������ֱ�ӰѶ����ɾ����
		SceneManager::getInstance().SceneDelTeam(getTeamtempId());

		SceneUser * pUser = SceneUserManager::getMe().getUserByName(rev->pstrName);
		if(pUser)
			pUser->TeamThisID = 0;

		return;
	}

	//sky ɾ����Ա������Scene�Ͳ�������,ͳһ����Sessionȥ����,����ͳһ��
	delMemberToSession(rev->pstrName);
}

/**
 * \brief  ɾ�������Ա
 * \param  mem ��ɾ����Ա
 */
void TeamManager::removeMember(SceneUser *mem)
{
  //TODO ɾ������
  Cmd::stRemoveTeamMemberUserCmd ret;
  ret.dwTeamID = getTeamtempId();
  strncpy(ret.pstrName, mem->name, MAX_NAMESIZE);
  removeMember(&ret);
}        

/**
 * \brief  ����
 * \param  callback �ص�����
 */
void TeamManager::execEveryOne(TeamMemExec &callback)
{
  team.execEvery(callback);
}

/**
 * \brief  ���������ض���Ա��������ж�Ա
 * \param  callback �ص�����
 * \param  tempid ���ų���Ա�� id
 */
void TeamManager::execEveryOneExceptMe(TeamMemExec &callback,DWORD tempid)
{
  team.execEveryExceptMe(callback,tempid);
}

/**
 * \brief  ����id����һ���µĳ�Ա
 * \param  pUser ��ɫָ��,���ض�Ҫ��
 * \param  id ������Ա�� id
 * \return true ���ӳɹ� false ����ʧ��
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
* \brief  sky ��ӿ糡����Ա
* \param  AwayUser ��Ա�Ļ�������
* \param  leaber �Ƿ��Ƕӳ�
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
 * \brief  ����������߳�Ա״̬,δ���߳�Ա��������ʱ��,�����涨ʱ������߳�Ա�Ӷ�����ɾ��
 */
struct CheckOfflineExec : public TeamMemExec
{
	/// �ӳ�
	SceneUser *leader;

	/// ���Ͷ���
	typedef std::vector<Cmd::stRemoveTeamMemberUserCmd> Remove_vec;

	/// ���Ͷ���
	typedef Remove_vec::iterator Remove_vec_iterator;

	/// ���������
	Remove_vec del_vec;

	/// ���Ͷ���
	typedef std::vector<DWORD> Online_vec;

	/// �������ָ��
	typedef Online_vec::iterator Online_vec_iterator;

	/// �������߹�����
	Online_vec add_vec;

	/**
	* \brief  ���캯����ʼ������
	* \param  u �ӳ�
	*/
	CheckOfflineExec(SceneUser *u)
	{
		leader = u;
	}

	/**
	* \brief  �����������߳�Ա
	* \param  member ��Ա
	* \return true �������� false ��ֹ����
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
				//TODO ɾ������
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
 * \brief  ������߳�Ա״̬,ɾ����ʱ��,�ָ����ߵ�
 * \param  pUser �ӳ�
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
 * \brief  �����������һ�����ζӳ�����ĳ�Աʹ���Ϊ�¶ӳ�
 */
//struct FindLeaderExec : public TeamMemExec
//{
//  /// ���ζӳ�
//  SceneUser *leader;
//
//  /// �¶ӳ�
//  SceneUser *newleader;
//
//  /**
//   * \brief  ���캯����ʼ������
//   * \param  u ��ǰ�ӳ�
//   */
//  FindLeaderExec(SceneUser *u)
//  {
//    leader = u;
//    newleader = NULL;
//  }
//
//  /**
//   * \brief  ���������µĶӳ�
//   * \param  member ��Ա
//   * \return true ��ʾ�������� false ��ֹ����
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
// * \brief  ����һ���µĶӳ�
// * \param  pUser ��ǰ�ӳ�
// * \return �µĶӳ������ɫָ��
// */
//SceneUser *TeamManager::findNewLeader(SceneUser *pUser)
//{
//  FindLeaderExec exec(pUser);
//  team.execEvery(exec);
//  return exec.newleader;
//}

/**
 * \brief  SKY �ı�ӳ�����
 * \param  pUser ��ǰ�ӳ�
 * \param  pNewUse ָ�������Ķӳ� 
 * \return true �ı�ɹ�  false �ı�ʧ��
 */
bool TeamManager::changeLeader(char * NewLeaberName)
{
	// ��Session���Ͷӳ���������Ϣ
	if(NewLeaberName)
		ChangeLeaderToSession(NewLeaberName);
	else
		ChangeLeaderToSession();

	return true;
}

/**
 * \brief  SKY ֪ͨ���ж����Ա��ʼROLL
 * \param  obj ���������Ʒ
 * \return true �ı�ɹ�  false �ı�ʧ��
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
 * \brief  sky ���ض��������ߵ�����������
 */
int TeamManager::GetTeamMemberNum()
{
	GetTeamMemberNumExec rem;
	team.execEvery(rem);

	return rem.MemberNum;
}

/**
 * \brief  sky �鿴�Ƿ�����û��ѡ��ROLLѡ��
 */
bool TeamManager::GetNoRollUser()
{
	GetNoRollUserExec rem;
	team.execEvery(rem);

	return rem.NoRollUser;
}

/**
 * \brief  sky ����ΨһID���ö����еĶ�ԱROLL����
 */
int TeamManager::SetMemberRoll( DWORD tempid, BYTE rolltype )
{
	SetMemberRollExec rem(tempid, rolltype);
	team.execEvery(rem);

	return rem.RollNum;
}

/**
 * \brief  sky ȫ����Աѡ��ROLL��Ϻ�ʼ����ROLL���Ķ�Ա
 */
TeamMember * TeamManager::ComparisonRollnum()
{
	ComparisonRollnumExec rem;
	team.execEvery(rem);

	return rem.maxRollmember;
}

/**
* \brief   ��ĳ�������Ķ�Ա������������ĳ�Ա��Ϣ
*/
struct SendMemberDataExec : public TeamMemExec
{
	/// ������Ϣ�ĳ�Ա
	SceneUser *nm;

	DWORD LeaberID;

	/// ��Ա������Ϣʵ��1
	Cmd::stAddTeamMemberUserCmd ret_1;

	/**
	* \brief  �����ʼ����Ϣʵ��
	* \param  u �ӳ���ɫ����
	* \param n ������Ա��ɫ����
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
	* \brief  �ص��������³�Ա���͸�ÿ����Ա,��ÿ����Ա���͸��³�Ա
	* \param  member ��Ա
	* \return false ��ֹ���� true ��������
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
* \brief  sky ���ö�Ա�Ĵ���״̬
* \memberID Ҫ���õĶ�Ա��ID
* \leaberID �ӳ�ID
* \Being true:����Ϊ����״̬ false:����Ϊ�����ڵ�״̬
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
 * \brief  ������ʱid���Ӷ����Ա
 * \param  pUser δʹ��
 * \param tempid �����ӳ�Ա����ʱid
 * \return true ���ӳɹ� false ����ʧ��
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
 * \brief  ����id ɾ����Ա
 * \param  id ��ɫ��id
 */
void TeamManager::removeMemberByID(DWORD id)
{
  team.removeMemberByID(id);
}

/**
 * \brief ������ʱidɾ����Ա 
 * \param  tempid ��ʱid
 */
void TeamManager::removeMemberByTempID(DWORD tempid)
{
  team.removeMemberByTempID(tempid);
}

/**
 * \brief �����Ѻöȹ�ϵ
 */
void TeamManager::requestFriendDegree()
{
  team.requestFriendDegree();
}

/**
 * \brief  ���͵�ǰ�Ѻöȹ�ϵ�б��Ự�浵
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
 * \brief  �����Ѻö����̰ѵ�ǰ���Ѻö�ֵ�滻���������Ĺ�ϵ�嵥��,��Ϊ��ǰֵ�����µ�,
       Ȼ�����֮ǰ�Ĺ�ϵ�б�,�ٰ��յ��Ĺ�ϵ�嵥���뵽��ϵ�б���
 * \param  rev �Ѻöȹ�ϵ�б���Ϣ
 */
void TeamManager::setFriendDegree(Cmd::Session::t_ReturnFriendDegree_SceneSession *rev)
{
	relationlock.wrlock();

	if (NULL == rev)
	{
		friendList.clear();   // �����б�
		consortList.clear();  // �����б�
		teacherList.clear();  // ʦ���б�
		prenticeList.clear(); // ͽ���б�

	}
	else if (0 == rev->size)
	{
		friendList.clear();   // �����б�
		consortList.clear();  // �����б�
		teacherList.clear();  // ʦ���б�
		prenticeList.clear(); // ͽ���б�
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

		if (!friendList.empty())   // �����б�
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
		if (!consortList.empty())  // �����б�
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
		if (!teacherList.empty())  // ʦ���б�
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
		if (!prenticeList.empty()) // ͽ���б�
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

		friendList.clear();   // �����б�
		consortList.clear();  // �����б�
		teacherList.clear();  // ʦ���б�
		prenticeList.clear(); // ͽ���б�

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
 * \brief  ʵʱ�ۼӼ������й�ϵ���Ѻöȹ�ϵ
 */
void TeamManager::countFriendDegree()
{
  if (0 == team.leader) return;

  bool oper = false;

  relationlock.rdlock();
  if (!friendList.empty())   // �����б�
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
  if (!consortList.empty())  // �����б�
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
  if (!teacherList.empty())  // ʦ���б�
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
  if (!prenticeList.empty()) // ͽ���б�
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
 * \brief  ������ʱ�浵���ݼ��ض���
 * \param  leader �ӳ�
 * \param  data ��ʱ�浵����
 * \return true ���سɹ�  false ����ʧ��
 */
bool TeamManager::loadTeam(SceneUser *leader,TempArchiveMember *data)
{
  ////����ͼ�������
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
  //      //Zebra::logger->debug("����%u�ָ���Ա�ҵ���Ա,id=%u,tempid=%u,name=%s",leader->tempid,u->id,u->tempid,u->name);
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
  //  //Zebra::logger->debug("�ָ������Ա,id=%u,name=%s",pair->id,pair->name);
  //  //size -= (int)((size - sizeof(DWORD)) > 0) ? size - sizeof(DWORD) : 0;
  //}
  //if (leader->team->getSize() == 1)
  //{
  //  leader->team->deleteTeam(leader);
  //}
  ////Zebra::logger->debug("load�����Ա����=%u",leader->team->getSize());
  return true;
}

/**
 * \brief  ����һ����Ա���Ự
 * \param  userid ���ӵĳ�Ա�� id
 * \return true Ϊ��Ϣ���ͳɹ� false ��Ϣ����ʧ��
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
 * \brief  ��Ự���ͳ�Աɾ����Ϣ
 * \param  leaderid �ӳ���id
 * \param  userid ��ɾ����ɫ��id
 * \return true Ϊ��Ϣ���ͳɹ� false ��Ϣ����ʧ��
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
 * \brief  ��Ự���Ͷӳ���������Ϣ
 * \param  leaderid �϶ӳ���id
 * \param  newleaderid �¶ӳ��ĵ�id
 * \return true Ϊ��Ϣ���ͳɹ� false ��Ϣ����ʧ��
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
        //Zebra::logger->debug("���9����Ա����ɹ�(%d,%d)(%d,%d)",pos,pUser->getPosI(),pUser->getPos().x,pUser->getPos().y);
        size++;
      }
      else
      {
        //Zebra::logger->debug("���9����Ա����ʧ��(%d,%d)(%d,%d)",pos,pUser->getPosI(),pUser->getPos().x,pUser->getPos().y);
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
