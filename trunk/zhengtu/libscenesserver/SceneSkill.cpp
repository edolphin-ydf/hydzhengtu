/**
 * \brief ʵ�ּ�������Ĵ���
 *
 */
#include <zebra/ScenesServer.h>

using namespace Cmd;


struct TeamSkillExec : public TeamMemExec
{
  SceneUser *leader;
  zSkill *skill;
  TeamSkillExec(SceneUser *user,zSkill *s)
  {
    leader = user;
    skill = s;
  }
  bool exec(TeamMember &member)
  {
    SceneUser *u = leader->scene->getUserByTempID(member.tempid);
    if (u)
    {
      /*
      std::vector<DWORD>::const_iterator iter = skill->base->benignID.gid.begin();
      std::vector<zSkillB::Odds>::const_iterator iter_1 = skill->base->benignrating.gr.begin();
      for(;iter != skill->base->benignID.gid.end(),iter_1 != skill->base->benignrating.gr.end();iter++,iter_1++)
      {
        if (selectByPercent(iter_1->percent))
        {
          u->skillState.plus.setCharState(u,*iter,false);
          u->skillState.plus.good[*iter].value = iter_1->value;
          u->skillState.plus.good[*iter].leavetime = skill->base->ptime + 1;
          u->skillState.plus.setCharState(u,*iter,true);
        }
      }
      Cmd::stRefreshStateMapScreenUserCmd ret;
      ret.dwUserTempID = u->tempid;
      bcopy(u->getByState(),ret.state,sizeof(ret.state));
      u->scene->sendCmdToNine(u->getPosI(),&ret,sizeof(ret),false);
      // */
      return true;
    }
    return false;
  }
};
/**
  * \brief �����Ҽ���
  *
  * \param rev ��Ӽ�������
  *
  * \return ��ӳɹ�����TRUE,���򷵻�FALSE
  *
  */
bool SceneUser::addSkillData(const Cmd::stAddUserSkillPropertyUserCmd *rev)
{
  zSkill::create(this,rev->dwSkillID,rev->wdLevel) ;
  charbase.skillpoint --;

  //ˢ���û�����
  Cmd::stMainUserDataUserCmd ret;
  full_t_MainUserData(ret.data);
  sendCmdToMe(&ret,sizeof(ret));
  return true;
}
/**
  * \brief ɾ����Ҽ���
  *
  * \param rev ɾ����������
  *
  * \return ɾ���ɹ�����TRUE,���򷵻�FALSE
  *
  */
bool SceneUser::removeSkill(const Cmd::stRemoveUserSkillPropertyUserCmd *rev)
{
  zSkill *skill = usm.findSkill(rev->dwSkillID);
  if (!skill)
  {
    return false;
  }
  usm.removeSkill(skill);

  //ˢ���û�����
  Cmd::stMainUserDataUserCmd  userinfo;
  full_t_MainUserData(userinfo.data);
  sendCmdToMe(&userinfo,sizeof(userinfo));
  return true;
}
/**
  * \brief ������Ҽ���
  *
  * \param dwSkillID ����ID
  * \param needSkillPoint ���輼�ܵ�
  *
  * \return ���³ɹ�����TRUE,���򷵻�FALSE
  *
  */
bool SceneUser::upgradeSkill(DWORD dwSkillID,bool needSkillPoint)
{
  if (charbase.skillpoint < 1 && needSkillPoint)
  {
    return false;
  }

  zSkill *skill = usm.findSkill(dwSkillID);
  if (!skill)
  {
    skill = zSkill::create(this,dwSkillID,1);
    if (!skill)
    {
      return false;
    }

	//sky �ж��¼��ܵ�ְҵҪ���Ƿ���û���ְҵһ��
	if(skill->base->kind != charbase.useJob)
	{
		Zebra::logger->error("�û�:[%s]��ͼѧϰ���û�ְҵ������ѧϰ�ļ���:[%s]", charbase.name, skill->base->name);
		return false;
	}

    if (needSkillPoint)
      charbase.skillpoint --;

    Cmd::stAddUserSkillPropertyUserCmd ret;
    ret.dwSkillID = skill->data.skillid;
    ret.wdLevel = skill->data.level;
    ret.wdUpNum = this->skillUpLevel(skill->base->skillid,skill->base->kind);
    if (ret.wdUpNum+ret.wdLevel>10) ret.wdUpNum = 10 - ret.wdLevel;
    ret.dwExperience = 0;
    ret.dwMaxExperience = 0;
    sendCmdToMe(&ret,sizeof(ret));

    this->setupCharBase();
    //ˢ���û�����
    Cmd::stMainUserDataUserCmd ret_1;
    full_t_MainUserData(ret_1.data);
    sendCmdToMe(&ret_1,sizeof(ret_1));
    return true;
  }

  if (skill->data.level >= MAX_SKILLLEVEL)
  {
    return false;
  }

  if (!skill->checkSkillStudy(true))
  {
    return false;
  }

  if (!skill->checkSkillBook(true))
  {
    return false;
  }

  zSkillB *tempbm = skillbm.get(skill_hash(skill->base->skillid,skill->base->level+1));
  if (tempbm)
  {
    if (this->usm.getPointInTree(skill->base->subkind) < tempbm->needpoint)
    {
      return false;
    }
  }
  else
  {
    return false;
  }
#ifdef _DEBUG
  Zebra::logger->error("�����������ǰ�Ἴ�ܵ��� ��ǰ��[%u] ��Ҫ��[%u] ��ǰBASE���ܵȼ�[%u] dwSkillID=[%u]",this->usm.getPointInTree(skill->base->subkind),skill->base->needpoint,skill->base->level,dwSkillID);
#endif

  if ((int)charbase.skillpoint - 1 >=0)
  {
    charbase.skillpoint -=1;
  }
  else
  {
    return false;
  }

  skill->data.level ++;
  skill->setupSkillBase(this);
  zRTime ctv;
  skill->lastUseTime = ctv.msecs() - skill->base->dtime;
  this->setupCharBase();

  Cmd::stAddUserSkillPropertyUserCmd ret;
  ret.dwSkillID = skill->data.skillid;
  ret.wdLevel = skill->data.level;
  ret.wdUpNum = this->skillUpLevel(skill->base->skillid,skill->base->kind);
  if (ret.wdUpNum+ret.wdLevel>10) ret.wdUpNum = 10 - ret.wdLevel;
  ret.dwExperience = 0;
  ret.dwMaxExperience = 0;
  sendCmdToMe(&ret,sizeof(ret));
  
  //ˢ���û�����
  Cmd::stMainUserDataUserCmd ret_1;
  full_t_MainUserData(ret_1.data);
  sendCmdToMe(&ret_1,sizeof(ret_1));
  return true;
}

int getCharType(DWORD type)
{
  int ret = 0;
  if (type != 0)
  {
    for( ; type > 0; type = type >> 1,ret ++);
    ret --;
  }
  else
  {
    return -1;
  }
  return ret;
}


