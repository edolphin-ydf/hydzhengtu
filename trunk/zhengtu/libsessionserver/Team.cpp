#include <zebra/SessionServer.h>

/**
 * \brief ��ӳ�Ա
 * \param userid ��ɫid
 * \return true �ɹ� false ʧ��
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

		//sky ��������Ķ������֪ͨ��Ӷ���
		TeamAddMemberSceneExec add(leaderid, dwTeam_tempid, pUser->name, pUser->id, pUser->face);
		execEvery(add);

		MapID.insert(pUser->scene);
	}
  }
  return bret;
}

/**
 * \brief ɾ����Ա
 * \param userid ��ɫid
 * \return true ɾ���ɹ�(���黹���ڱ�Ķ�Ա) false �����Ѿ�û����(Ҫ������ɾ���������)
 */
bool Team::delMember(const DWORD userid)
{
	member.erase(userid);
	UserSession *pUser=UserSessionManager::getInstance()->getUserByID(userid);
	if (pUser)
	{
		pUser->teamid = 0;

		//sky ����֪ͨ�������ɾ������
		TeamDelMemberSceneExec Del(this->dwTeam_tempid, pUser->name);
		execEvery(Del);

		UpDataMapID(userid);
	}

	//sky �ö���û�ж�Ա��ʱ��,ɾ������
	if(member.empty())
		return false;

	return true;
}

/**
 * \brief ���öӳ�
 * \param leader �ӳ�id
 * \return true �ɹ� false ʧ��
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
 * \brief ɾ������
 * \return true �ɹ� false ʧ��
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

/// ȫ�ֶ�������
GlobalTeamIndex *GlobalTeamIndex::instance = NULL;

/**
 * \brief ��ȡȫ�ֶ�������Ψһʵ��
 * \return ȫ�ֶ�����������
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
 * \brief ɾ��ʵ��
 */
void GlobalTeamIndex::delInstance()
{
  SAFE_DELETE(instance);
}

/**
 * \brief ���ӳ�Ա
 * \param leaderid �ӳ�id
 * \param userid ��ԱID
 * \return true �ɹ� false ʧ��
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
			t.setLeader(leaderid); //sky ���ö���ӳ�
			//t.MapID.insert(pUser->scene);
			bret = team.insert(TeamMap_value_type(tempid,t)).second; //sky ���������Ͷ���ΨһID�ŵ���������
		}
	}
	else
	{
		TeamMap_iterator iter = team.find(tempid); //SKY ���ݶ����ΨһID�����Ҷ������
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
 * \brief ɾ����Ա
 * \param leaderid ����Ψһid
 * \param userName ��Ա����
 * \return true �ɹ� false ʧ��
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
				//sky �����Ѿ���ɾ���� ����ʱ�б�������������ص�����ȫ�������
				for(iter1=MoveSceneMemberMap.begin(); iter1!=MoveSceneMemberMap.end();)
				{
					if(iter1->second == tempid)
						iter1 = MoveSceneMemberMap.erase(iter1);
					else
						iter1++;
				}
				g_MoveSceneMemberMapLock.unlock();

				team.erase(iter); //sky ��������Ѿ�û����,�ʹӹ�����ɾ����
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
 * \brief sky �����ӳ�
 * \param leaderid �ӳ�id
 * \return true �ɹ� false ʧ��
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
			buff->ChangeLeader(leaberName); //sky �Ѷӳ���ID����
		else
			buff->ChangeLeader(); //sky �Ѷӳ���ID����

		bret = true;
	}
	else
	{
		bret = false;
	}
	mlock.unlock();
	return bret;
}

//sky ��������ϵ��

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
					leaderid = pUser->id;	//sky ����Ա˳������һ���¶ӳ�
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

//sky�����������ڵ�ȫ������
bool Team::execEvery(TeamSceneExec &callback)
{
	MapIDSet::iterator iter;

	for(iter = MapID.begin() ; iter != MapID.end() ; iter ++)
	{
		callback.exec(*iter);
	}

	return true;
}


//sky ���ö����ΨһID
void Team::SetTeamThisID(DWORD TeamThisID)
{
	dwTeam_tempid = TeamThisID;
}


//sky ɾ������
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

//sky ����MapID����
void Team::UpDataMapID(DWORD useID)
{
	//sky ֪ͨ��Ϻ������û��Ŀ糡�������������MapID����
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

//sky ��ȡ�ض���ԱID
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

//sky ��ȡ��Ա��Ŀ
DWORD Team::GetMemberNum()
{
	return member.size();
}

//sky ����MapID����
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