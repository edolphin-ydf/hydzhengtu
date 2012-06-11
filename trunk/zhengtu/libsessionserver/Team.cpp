#include <zebra/SessionServer.h>

/**
 * \brief 添加成员
 * \param userid 角色id
 * \return true 成功 false 失败
 */
bool Team::addMember(const DWORD userid)
{
  UserSession *pUser=UserSessionManager::getInstance()->getUserByID(userid);
  if (!pUser)
  {
    return false;
  }
  bool bret = false;
  bret = member.insert(userid).second;
  if (bret)
  {
    pUser->teamid = dwTeam_tempid;

	MapIDSet::iterator iter;
	iter = MapID.find(pUser->scene);

	if(iter == MapID.end())
	{
		char * buf = new char[sizeof(Cmd::Session::t_Team_Data)+(sizeof(Cmd::Session::stMember)*member.size())];
		bzero(buf,sizeof(Cmd::Session::t_Team_Data)+(sizeof(Cmd::Session::stMember)*member.size()));
		Cmd::Session::t_Team_Data * TeamData = (Cmd::Session::t_Team_Data*)buf;
		constructInPlace(TeamData);

		TeamData->LeaderID = leaderid;
		TeamData->dwTeamThisID = dwTeam_tempid;
		TeamData->dwSize = member.size();

		int i = 0;
		MemberSet::iterator iter;
		for(iter=member.begin(); iter!=member.end(); iter++)
		{
			UserSession * User = UserSessionManager::getInstance()->getUserByID((*iter));

			if(pUser)
			{
				TeamData->Member[i].dwID = User->id;
				strncpy(TeamData->Member[i].name, User->name, MAX_NAMESIZE);
				TeamData->Member[i].face = User->face;
				i++;
			}
		}

		pUser->scene->sendCmd(TeamData, sizeof(Cmd::Session::t_Team_Data)+(sizeof(Cmd::Session::stMember)*i));
		SAFE_DELETE_VEC(buf);

		//sky 便历队伍的多个场景通知添加队友
		TeamAddMemberSceneExec add(leaderid, dwTeam_tempid, pUser->name, pUser->id, pUser->face);
		execEvery(add);

		MapID.insert(pUser->scene);
	}
  }
  return bret;
}

/**
 * \brief 删除成员
 * \param userid 角色id
 * \return true 删除成功(队伍还存在别的队员) false 队伍已经没人拉(要管理器删除这个队伍)
 */
bool Team::delMember(const DWORD userid)
{
	member.erase(userid);
	UserSession *pUser=UserSessionManager::getInstance()->getUserByID(userid);
	if (pUser)
	{
		pUser->teamid = 0;

		//sky 遍历通知多个场景删除队友
		TeamDelMemberSceneExec Del(this->dwTeam_tempid, pUser->name);
		execEvery(Del);

		UpDataMapID(userid);
	}

	//sky 该队伍没有队员的时候,删除队伍
	if(member.empty())
		return false;

	return true;
}

/**
 * \brief 设置队长
 * \param leader 队长id
 * \return true 成功 false 失败
 */
bool Team::setLeader(const DWORD leader)
{
  leaderid = leader;
  return addMember(leaderid);
}

DWORD Team::GetLeader()
{
	return leaderid;
}

/**
 * \brief 删除队伍
 * \return true 成功 false 失败
 */
bool Team::delTeam()
{
  MemberSet_iter iter;
  for(iter = member.begin() ; iter != member.end(); iter++)
  {
    UserSession *pUser=UserSessionManager::getInstance()->getUserByID(*iter);
    if (pUser)
    {
      pUser->teamid = 0;
    }
  }
  member.clear();

  TeamDelTeamSceneExec Del(dwTeam_tempid);

  execEvery(Del);

  return true;
}

/// 全局队伍索引
GlobalTeamIndex *GlobalTeamIndex::instance = NULL;

/**
 * \brief 获取全局队伍索引唯一实例
 * \return 全局队伍索引对象
 */
GlobalTeamIndex *GlobalTeamIndex::getInstance()
{
  if (!instance)
  {
    instance = new GlobalTeamIndex;
  }
  return instance;
}

/**
 * \brief 删除实例
 */
void GlobalTeamIndex::delInstance()
{
  SAFE_DELETE(instance);
}

/**
 * \brief 增加成员
 * \param leaderid 队长id
 * \param userid 队员ID
 * \return true 成功 false 失败
 */
bool GlobalTeamIndex::addMember(const DWORD tempid, const DWORD leaderid,const DWORD userid)
{
	bool bret = false;
	mlock.lock();
	if (leaderid == userid)
	{
		UserSession * pUser = UserSessionManager::getInstance()->getUserByID(leaderid);

		if(pUser)
		{
			Team t;
			t.SetTeamThisID(tempid);
			t.setLeader(leaderid); //sky 设置队伍队长
			//t.MapID.insert(pUser->scene);
			bret = team.insert(TeamMap_value_type(tempid,t)).second; //sky 将队伍对象和队伍唯一ID放到管理器中
		}
	}
	else
	{
		TeamMap_iterator iter = team.find(tempid); //SKY 根据队伍的唯一ID来查找队伍对象
		if (iter != team.end())
		{
			bret = iter->second.addMember(userid);
		}
		else
		{
			bret = false;
		}
	}
	mlock.unlock();
	return bret;
}

/**
 * \brief 删除成员
 * \param leaderid 队伍唯一id
 * \param userName 队员名字
 * \return true 成功 false 失败
 */
bool GlobalTeamIndex::delMember(const DWORD tempid,const char * userName)
{
	bool bret = false;
	mlock.lock();
	TeamMap_iterator iter = team.find(tempid);
	if (iter != team.end())
	{
		UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName(userName);

		if(pUser)
		{
			if(!iter->second.delMember(pUser->id))
			{
				std::map<DWORD,DWORD>::iterator iter1;

				g_MoveSceneMemberMapLock.lock();
				//sky 队伍已经被删除拉 把临时列表里和这个队伍相关的数据全部清理掉
				for(iter1=MoveSceneMemberMap.begin(); iter1!=MoveSceneMemberMap.end();)
				{
					if(iter1->second == tempid)
						iter1 = MoveSceneMemberMap.erase(iter1);
					else
						iter1++;
				}
				g_MoveSceneMemberMapLock.unlock();

				team.erase(iter); //sky 如果队伍已经没人拉,就从管理器删除他
			}

			bret = true;
		}
	}
	else
	{
		bret = false;
	}
	mlock.unlock();
	return bret;
}


/**
 * \brief sky 跟换队长
 * \param leaderid 队长id
 * \return true 成功 false 失败
 */
bool GlobalTeamIndex::ChangeLeader( DWORD tempid , const char * leaberName)
{
	bool bret = false;
	mlock.lock();
	TeamMap_iterator iter = team.find(tempid);
	if (iter != team.end())
	{
		Team * buff = &(iter->second);
		if(leaberName)
			buff->ChangeLeader(leaberName); //sky 把队长的ID换掉
		else
			buff->ChangeLeader(); //sky 把队长的ID换掉

		bret = true;
	}
	else
	{
		bret = false;
	}
	mlock.unlock();
	return bret;
}

//sky 新增函数系列

bool Team::ChangeLeader(const char * leaberName)
{
	MemberSet::iterator iter;
	char name[MAX_NAMESIZE];

	if(leaberName)
	{
		UserSession *pUser = UserSessionManager::getInstance()->
			getUserSessionByName(leaberName);

		if(pUser)
		{
			leaderid = pUser->id;
			strncpy(name, leaberName, MAX_NAMESIZE);
			TeamChangeLeaberSceneExec New(dwTeam_tempid, name);
			execEvery(New);
		}
	}
	else
	{
		for(iter=member.begin(); iter!=member.end(); iter++)
		{
			UserSession *pUser = UserSessionManager::getInstance()->
				getUserByID(*iter);

			if(pUser)
			{
				if(pUser->id != leaderid)
				{
					leaderid = pUser->id;	//sky 按成员顺序设置一个新队长
					strncpy(name, pUser->name, MAX_NAMESIZE);
					break;
				}
			}
		}

		TeamChangeLeaberSceneExec New(dwTeam_tempid, name);
		execEvery(New);
	}

	return true;
}

bool Team::MemberMoveScen(SceneSession * scene)
{
	MapIDSet::iterator iter;
	iter = MapID.find(scene);

	if(iter == MapID.end())
	{
		MapID.insert( scene );

		char * buf = new char[sizeof(Cmd::Session::t_Team_Data)+(sizeof(Cmd::Session::stMember)*member.size())];
		bzero(buf,sizeof(Cmd::Session::t_Team_Data)+(sizeof(Cmd::Session::stMember)*member.size()));
		Cmd::Session::t_Team_Data * TeamData = (Cmd::Session::t_Team_Data*)buf;
		constructInPlace(TeamData);

		TeamData->LeaderID = leaderid;
		TeamData->dwTeamThisID = dwTeam_tempid;
		TeamData->dwSize = member.size();

		int i = 0;
		MemberSet::iterator iter;
		for(iter=member.begin(); iter!=member.end(); iter++)
		{
			UserSession * pUser = UserSessionManager::getInstance()->getUserByID((*iter));

			if(pUser)
			{
				TeamData->Member[i].dwID = pUser->id;
				strncpy(TeamData->Member[i].name, pUser->name, MAX_NAMESIZE);
				TeamData->Member[i].face = pUser->face;
				i++;
			}
		}

		scene->sendCmd(TeamData, sizeof(Cmd::Session::t_Team_Data)+(sizeof(Cmd::Session::stMember)*i));
		SAFE_DELETE_VEC(buf);
	}

	return true;
}

bool GlobalTeamIndex::MemberMoveScen(const DWORD tempid, SceneSession * scene)
{
	bool bret = false;

	mlock.lock();

	TeamMap_iterator iter = team.find(tempid);

	if(iter != team.end())
	{
		iter->second.MemberMoveScen(scene);
		bret = true;
	}

	mlock.unlock();

	return bret;
}

Team * GlobalTeamIndex::GetpTeam(const DWORD tempid)
{
	mlock.lock();

	Team * buff = NULL;

	TeamMap_iterator iter = team.find(tempid);

	if(iter != team.end())
		buff = &(iter->second);

	mlock.unlock();

	return buff;
}

//sky遍历队伍所在的全部场景
bool Team::execEvery(TeamSceneExec &callback)
{
	MapIDSet::iterator iter;

	for(iter = MapID.begin() ; iter != MapID.end() ; iter ++)
	{
		callback.exec(*iter);
	}

	return true;
}


//sky 设置队伍的唯一ID
void Team::SetTeamThisID(DWORD TeamThisID)
{
	dwTeam_tempid = TeamThisID;
}


//sky 删除队伍
bool GlobalTeamIndex::DelTeam(DWORD TeamThisID)
{
	mlock.lock();

	TeamMap_iterator iter = team.find(TeamThisID);

	if(iter != team.end())
	{
		Team * buff = &(iter->second);
		buff->delTeam();
		team.erase(iter);
	}

	mlock.unlock();

	return true;
}

//sky 跟新MapID容器
void Team::UpDataMapID(DWORD useID)
{
	//sky 通知完毕后检测下用户的跨场景的情况跟新下MapID容器
	UserSession * pUser = UserSessionManager::getInstance()->getUserByID(useID);

	if(pUser)
	{
		MemberSet_iter iter;
		int num = 0;	
		for(iter = member.begin() ; iter != member.end(); iter++)
		{
			UserSession *pMember=UserSessionManager::getInstance()->getUserByID(*iter);
			if (pMember && (pMember->id != pUser->id) && (pMember->scene == pUser->scene))
				num++;
		}

		if(num == 0)
			MapID.erase(pUser->scene);
	}
}

//sky 获取特定队员ID
DWORD Team::GetMemberID(int i)
{
	DWORD userID = 0;
	int index = 0;

	if(i < GetMemberNum())
	{
		MemberSet_iter it;
		for(it=member.begin(); it!=member.end(); it++)
		{
			if(index == i)
			{
				userID = *it;
				break;
			}

			index++;
		}
	}

	return userID;
}

//sky 获取队员数目
DWORD Team::GetMemberNum()
{
	return member.size();
}

//sky 跟新MapID容器
void GlobalTeamIndex::UpDataMapID(DWORD useID, DWORD TeamThisID)
{
	mlock.lock();

	TeamMap_iterator iter = team.find(TeamThisID);

	if(iter != team.end())
	{
		Team * buff = &(iter->second);
		buff->UpDataMapID(useID);
	}

	mlock.unlock();	
}