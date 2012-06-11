/**
 * \brief Session������Ϣ
 *
 * 
 */
#include <zebra/ScenesServer.h>

/**
 * \brief ���¼��㾭����䷽��
 * \param pos ������侭������ĵ� 
 */
void Team::calAverageExp(zPosI pos,DWORD sceneid)
{
  averageExp = 0;
  std::vector<TeamMember>::iterator iter;
  //rwlock.rdlock();
  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(iter->tempid);
    if (pUser)
    {
      if (pUser->scene->id == sceneid && pUser->scene->checkTwoPosIInNine(pUser->getPosI(),pos))
      {
        //averageExp += (DWORD)(pUser->charbase.level /30.0f + 1.0f);
        averageExp += pUser->charbase.level;
        if (averageExp < 0)
        {
          averageExp = 0;
        }
      }
    }
  }
  //rwlock.unlock();
  
}


/**
 * \brief �û�����������䷽����֮�仯
 *
 *
 * \param pUser: �������û�
 */
void Team::userupgrade(SceneUser *pUser)
{
	/*averageExp -= (WORD)((pUser->charbase.level - 1.0f) /30.0f + 1.0f);
	averageExp += (DWORD)(pUser->charbase.level /30.0f + 1.0f);
	if (averageExp < 0)
	{
	averageExp = 0;
	}
	*/
}


const TeamMember *Team::getTeamMember(DWORD index) const 
{
	DWORD _size = member.size();
	if(index < 0 || index >= member.size())
		return NULL;
	return &member[index];
}

const TeamMember *Team::getTeamMember(const char * name) const
{
	for(int i=0; i<member.size(); i++)
	{
		if(strcmp(name, member[i].name) == 0)
			return &member[i];
	}

	return NULL;
}

/**
 * \brief ���پ���������
 *
 *
 * \param pUser: ������ٵ��û�
 */
void Team::decreaseAverageExp(SceneUser *pUser)
{
  averageExp -= (DWORD)(pUser->charbase.level /30.0f + 1.0f);
  if (averageExp < 0)
  {
    averageExp = 0;
  }
}
/**
 * \brief ���Ӿ���������
 *
 *
 * \param pUser:�������ӵ��û�
 */
void Team::increaseAverageExp(SceneUser *pUser)
{
  averageExp += (DWORD)(pUser->charbase.level /30.0f + 1.0f);
  if (averageExp < 0)
  {
    averageExp = 0;
  }
}
/**
 * \brief ���Ӷ�Ա
 *
 *
 * \param id: �û�id
 * \param tempid: �û�tempid
 * \param pname: �û�����
 */
bool Team::addMember(DWORD id,DWORD tempid,const char *pname)
{
  zRTime curTim;
  TeamMember m;
  m.id = id;
  m.tempid = tempid;
  strncpy(m.name,pname,MAX_NAMESIZE);
  m.offtime = 0;
  m.begintime = curTim.sec()/60; // �����Ѻö��õģ���¼����ʱ��

  std::vector<TeamMember>::iterator iter;
  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    if (iter->id == m.id)
    {
      iter->id = m.id;
      iter->tempid = m.tempid;
      strncpy(iter->name,m.name,MAX_NAMESIZE);
      iter->offtime = m.offtime;
      iter->begintime = m.begintime;
      break;
    }
  }
  if (iter == member.end())
  {
    if (member.size() >= (DWORD)MaxMember)
    {
      return false;
    }
    member.push_back(m);
  }

  SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(tempid);
  if (pUser)
  {
    increaseAverageExp(pUser);
    requestFriendDegree();
  }

  return true;
}

/**
 * \brief ɾ����Ա 
 *
 *
 * \param id: �û�id
 * \param tempid: �û�tempid
 */
void Team::removeMember(DWORD id,DWORD tempid)
{
  std::vector<TeamMember>::iterator iter;

  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    if (iter->id == id && iter->tempid == tempid)
    {
      SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(tempid);
      if (pUser)
      {
        decreaseAverageExp(pUser);
      }
      member.erase(iter);
      if (current_obj_owner) current_obj_owner--; 
      break;
    }
  }

  requestFriendDegree();
}

/**
 * \brief ɾ����Ա 
 *
 *
 * \param id: �û�id
 */
void Team::removeMemberByID(DWORD id)
{
  std::vector<TeamMember>::iterator iter;
  //rwlock.wrlock();
  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    if (iter->id == id)
    {
      SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(iter->tempid);
      if (pUser)
      {
        decreaseAverageExp(pUser);
      }
      member.erase(iter);
      if (current_obj_owner) current_obj_owner--; 
      break;
    }
  }

  requestFriendDegree();
}

/**
 * \brief ɾ����Ա 
 *
 *
 * \param pname: �û�����
 * \return ���ɾ���ɹ����ر�ɾ���ߵ���ʱ id ����Ϊ0
 */
DWORD Team::removeMemberByName(const char *pname)
{
  DWORD tempid = 0;
  std::vector<TeamMember>::iterator iter;
  //rwlock.wrlock();
  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    if (0 == strcmp(iter->name,pname))
    {
      tempid = iter->tempid;
      SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(iter->tempid);
      if (pUser)
      {
        decreaseAverageExp(pUser);
      }
      member.erase(iter);
      if (current_obj_owner) current_obj_owner--; 
      break;
    }
  }

  requestFriendDegree();
  return tempid;
}
/**
 * \brief �õ���һ����Ʒӵ����
 *
 *
 * \return ��һ����Ʒӵ����
 */
DWORD Team::getNextObjOwnerID()
{
  DWORD tempid=0;
  //rwlock.rdlock();
  if (member.size())  //sky ��ȡ�����е�������(���������±�)
  {
    current_obj_owner=current_obj_owner%member.size(); //sky ����Ʒ����������ȡģ�������õ���ǰ��ȡ��Ʒ�Ľ�ɫ
    tempid = member[current_obj_owner].id;				//sky ������Ʒ����
    current_obj_owner++;								//sky ���������Լ��Ա�֤����ѭ�����¸���Ա
  }
  else
  {
    current_obj_owner=0; 
  }
  //rwlock.unlock();
  return tempid;
}

/**
 * \brief ɾ����Ա 
 *
 *
 * \param tempid: �û�tempid
 */
void Team::removeMemberByTempID(DWORD tempid)
{
  std::vector<TeamMember>::iterator iter;

  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    if (iter->tempid == tempid)
    {
      member.erase(iter);
      if (current_obj_owner) current_obj_owner--; 
      break;
    }
  }

  requestFriendDegree();
}


/**
 * \brief ������Ա
 *
 *
 * \param callback: �ص�����
 */
void Team::execEvery(TeamMemExec &callback)
{
  std::vector<TeamMember>::iterator iter;

  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    callback.exec(*iter);
  }
}

/**
 * \brief ������Ա
 *
 *
 * \param callback: �ص�����
 * \param tempid: ���μӻص����û�tempid
 */
void Team::execEveryExceptMe(TeamMemExec &callback,DWORD tempid)
{
  std::vector<TeamMember>::iterator iter;

  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    if (iter->tempid != tempid)
    {
      callback.exec(*iter);
    }
  }
}

/**
 * \brief �����Ѻö�
 *
 *
 */
void Team::requestFriendDegree()
{
  std::vector<TeamMember>::iterator iter;
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::Session::stRequestMember * temp = NULL;

  Cmd::Session::t_RequestFriendDegree_SceneSession *retCmd=(Cmd::Session::t_RequestFriendDegree_SceneSession *)buf;
  constructInPlace(retCmd);
  retCmd->size = 0;
  temp = retCmd->namelist;
  //rwlock.rdlock();
  for(iter = member.begin() ; iter != member.end() ; iter ++)
  {
    strncpy(temp->name,iter->name,MAX_NAMESIZE);
    retCmd->size++;
    temp++;
  }
  //rwlock.unlock();

  sessionClient->sendCmd(retCmd,sizeof(Cmd::Session::t_RequestFriendDegree_SceneSession)+retCmd->size*sizeof(Cmd::Session::stRequestMember));
}