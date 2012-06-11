/**
 * \brief Session队伍信息
 *
 * 
 */
#include <zebra/ScenesServer.h>

/**
 * \brief 重新计算经验分配方案
 * \param pos 队伍分配经验的中心点 
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
 * \brief 用户升级后经验分配方案随之变化
 *
 *
 * \param pUser: 升级的用户
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
 * \brief 减少经验分配比率
 *
 *
 * \param pUser: 引起减少的用户
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
 * \brief 增加经验分配比率
 *
 *
 * \param pUser:引起增加的用户
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
 * \brief 增加队员
 *
 *
 * \param id: 用户id
 * \param tempid: 用户tempid
 * \param pname: 用户名称
 */
bool Team::addMember(DWORD id,DWORD tempid,const char *pname)
{
  zRTime curTim;
  TeamMember m;
  m.id = id;
  m.tempid = tempid;
  strncpy(m.name,pname,MAX_NAMESIZE);
  m.offtime = 0;
  m.begintime = curTim.sec()/60; // 计算友好度用的，记录登入时间

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
 * \brief 删除队员 
 *
 *
 * \param id: 用户id
 * \param tempid: 用户tempid
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
 * \brief 删除队员 
 *
 *
 * \param id: 用户id
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
 * \brief 删除队员 
 *
 *
 * \param pname: 用户名称
 * \return 如果删除成功返回被删除者的临时 id 否则为0
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
 * \brief 得到下一个物品拥有者
 *
 *
 * \return 下一个物品拥有者
 */
DWORD Team::getNextObjOwnerID()
{
  DWORD tempid=0;
  //rwlock.rdlock();
  if (member.size())  //sky 获取队伍中的总人数(用来计算下标)
  {
    current_obj_owner=current_obj_owner%member.size(); //sky 用物品掉落的序号来取模总人数得到当前获取物品的角色
    tempid = member[current_obj_owner].id;				//sky 设置物品主人
    current_obj_owner++;								//sky 掉落的序号自加以保证可以循环到下个队员
  }
  else
  {
    current_obj_owner=0; 
  }
  //rwlock.unlock();
  return tempid;
}

/**
 * \brief 删除队员 
 *
 *
 * \param tempid: 用户tempid
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
 * \brief 遍历队员
 *
 *
 * \param callback: 回调操作
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
 * \brief 遍历队员
 *
 *
 * \param callback: 回调操作
 * \param tempid: 不参加回调的用户tempid
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
 * \brief 计算友好度
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