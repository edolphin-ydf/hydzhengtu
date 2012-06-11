/**
 * \brief  ����ϵͳ
 * 
 */

#include <zebra/ScenesServer.h>

/**     
 * \brief  �������
 *
 * �ṩ��һ��Ĭ�ϵĲ���������,�̳�����Ҫ�����Լ���Ҫ�����ش˺���
 *      
 * \param user: ���������ж����û�
 * \param vars: �û������ĸ�������ر���
 * \return true��ʾ�����Ϸ�,false��ʾ�����Ƿ�
 */   
bool Condition::check_args(const SceneUser* user,const Vars* vars) const
{
  if (user && vars) return true;
  
  return false;
}

/**     
 * \brief  �ű������ж�
 *
 * template methodģʽ,���в�����飬��ִ�нű�����������ж�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return true��ʾ��������,false��ʾ����������
 */    
bool Condition::is_valid(const SceneUser* user,const Vars* vars) const
{
  if (!check_args(user,vars) ) return false;
  
  return check_valid(user,vars);
}

/**     
 * \brief  �ű������ж�
 *
 * template methodģʽ,���в�����飬����ÿһ���û�ִ�нű�����������ж�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return true��ʾ�����û���������,���򷵻�false
 */    
bool TeamCondition::is_valid(const SceneUser* user,const Vars* vars) const
{
	if (!check_args(user,vars) ) return false;

	if (_team) 
	{
		TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(user->TeamThisID);
		bool result = false;

		if(teamMan)
		{
			DWORD leader_id = teamMan->getLeader();

			Team& team = const_cast<Team&>(teamMan->getTeam());

			team.rwlock.rdlock();

			std::vector<TeamMember>::iterator it = team.member.begin();;
			for(; it!=team.member.end(); ++it) 
			{
				SceneUser* member = SceneUserManager::getMe().getUserByTempID(it->tempid);
				if (member)
				{
					Vars* v = member->quest_list.vars(vars->quest_id());
					result = check_valid(member,v);
					if (!result) break;
				}
			}

			team.rwlock.unlock();
		}
		else 
		{
			result = check_valid(user,vars);    
		}

		return result;
	}

	return check_valid(user,vars);
}

/**     
 * \brief  ��������ж�
 *
 * ������check_valid����,�ж��û��Ķ����Ƿ�����ű�Ҫ��
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return true��ʾ��������,false��ʾ����������
 */    
bool TeamedCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
	if (!check_args(user,vars) ) return false;

	TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(user->TeamThisID);

	if (teamMan) 
	{
		if (_number && teamMan->getSize() != _number) return false;

		int male = 0,female = 0;
		Team& team = const_cast<Team&>(teamMan->getTeam());
		team.rwlock.rdlock();
		std::vector<TeamMember>::iterator it = team.member.begin();;
		for(; it!=team.member.end(); ++it) 
		{
			SceneUser* member = SceneUserManager::getMe().getUserByTempID(it->tempid);
			if (member) 
			{
				if (member->charbase.type == PROFESSION_1) ++male;
				if (member->charbase.type == PROFESSION_2) ++female;
			}
		}
		team.rwlock.unlock();

		if (_male && _male != male) 
			return false;
		if (_female && _female != female) 
			return false;

		return true;
	}

	return false;
}

bool MapCountryCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
  Vars* vs = const_cast<Vars *>(vars);
  if (_id) vs = user->quest_list.vars(_id);

  if (!vs) {
    return false;
  }

  int country;
  if (!vs->get_value(_name,country)) {
    return false;
  }
  
  return (int)user->scene->getCountryID() == country;
}

bool HonorCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
  return user->charbase.honor >= (DWORD)_value;
}

bool MaxHonorCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
  return user->charbase.maxhonor >= (DWORD)_value;
}

bool SelfCountryCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
  bool flag = ( user->charbase.country == user->scene->getCountryID() 
    || (CountryAllyM::getMe().getFriendLevel(user->charbase.country,user->scene->getCountryID())>0));
  if (!_value) flag = !flag;

  return flag;
}

bool CountryPowerCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
#ifdef _DEBUG
  Zebra::logger->debug("��[%s countryid=%d]���ڵĹ�����%s",user->name,user->charbase.country,(ScenesService::getInstance().countryPower[user->charbase.country] == 0)?"����":"ǿ��");
#endif
  return (ScenesService::getInstance().countryPower[user->charbase.country] == 0);
}

bool WeekCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
  struct tm tm_1;
  time_t timValue = time(NULL);
  zRTime::getLocalTime(tm_1,timValue);
  BYTE tempvalue = (BYTE)_value;
  return (Cmd::isset_state(&tempvalue,tm_1.tm_wday));
}

bool CaptionCondition::check_valid(const SceneUser* user,const Vars* vars) const
{
  return strstr(user->caption,"����") != NULL;
}


