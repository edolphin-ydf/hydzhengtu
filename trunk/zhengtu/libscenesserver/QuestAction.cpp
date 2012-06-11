/**
 * \brief  ����ϵͳ
 * 
 */

#include <zebra/ScenesServer.h>

/**     
 * \brief  ִ�нű�����Ķ���
 *
 * template methodģʽ,���в������,��ִ�нű�����Ķ���
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */     
int Action::do_it (SceneUser* user,Vars* vars)
{
  if (!check_args(user,vars)) return Action::FAILED;  
  
  return done(user,vars);
}

/**     
 * \brief  �������
 *
 * �ṩ��һ��Ĭ�ϵĲ���������,�̳�����Ҫ�����Լ���Ҫ�����ش˺���
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return true��ʾ�����Ϸ�,false��ʾ�����Ƿ�
 */     
bool Action::check_args(SceneUser* user,Vars* vars) const
{
  if (user && vars) return true;
  
  return false;
}

/**     
 * \brief  ִ�нű�����Ķ���
 *
 * template methodģʽ,���в������,�����ڶ����е�ÿһ����Աִ�нű�����Ķ���
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */   
int TeamAction::do_it(SceneUser* user,Vars* vars)
{
	if (!check_args(user,vars) ) return false;

	if (_team) 
	{
		TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(user->TeamThisID);

		bool result = false;

		if (teamMan) 
		{
			Team& team = const_cast<Team&>(teamMan->getTeam());

			team.rwlock.rdlock();
			std::vector<TeamMember>::iterator it = team.member.begin();;
			for(; it!=team.member.end(); ++it) 
			{
				SceneUser* member = SceneUserManager::getMe().getUserByTempID(it->tempid);
				if (member) 
				{
					Vars* v = member->quest_list.vars(vars->quest_id());
					if (v) result |= done(member,v);
				}
			}
			team.rwlock.unlock();
		}
		else
		{
			result = done(user,vars);    
		}

		return result;  
	}  

	return done(user,vars);
}

/**     
 * \brief  ϵͳ����
 *
 * ������done����,����ϵͳ������Ϣ���ض��û�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */       
int NotifyAction::done (SceneUser* user,Vars* vars)
{
  Channel::sendSys(user,Cmd::INFO_TYPE_GAME,_info.c_str());
  return Action::SUCCESS;
}

/**     
 * \brief  ������־
 *
 * ������done����,���������־��Log�С�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */       
int LogAction::done (SceneUser* user,Vars* vars)
{
  Zebra::logger->info("������־: ��ɫ%s,%s",user->name,_info.c_str());
  return Action::SUCCESS;
}

int BulletinAction::done (SceneUser* user,Vars* vars)
{
  Cmd::Session::t_QuestBulletinUserCmd cmd;
  cmd.kind = _kind;
  strncpy(cmd.content,_info.c_str(),_info.length());
  if (_kind == 1) {
    cmd.id = user->charbase.unionid;
  }
  if (_kind == 2) {
    cmd.id = user->charbase.septid;
  }
  
  sessionClient->sendCmd(&cmd,sizeof(cmd));
  return Action::SUCCESS;
}

/**     
 * \brief  ϵͳ����
 *
 * ������done����,����ϵͳ������Ϣ���ض��û�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */       
int Notify1Action::done (SceneUser* user,Vars* vars)
{
  Channel::sendSys(user,Cmd::INFO_TYPE_EXP,_info.c_str());
  return Action::SUCCESS;
}

/**     
 * \brief  �˵�
 *
 * ������done����,�����û����ʵ�npc������˵�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */  
     
int MenuAction::done (SceneUser* user,Vars* vars)
{
  user->quest_list.set_menu(_menu);
  return Action::SUCCESS;
}

/**     
 * \brief  �˵�
 *
 * ������done����,�����û����ʵ�npc������˵�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */  
     
int SubMenuAction::done (SceneUser* user,Vars* vars)
{
  user->quest_list.add_menu(_menu);
  return Action::SUCCESS;
}

/**     
 * \brief  �˵�
 *
 * ������done����,�����û����ʵ�npc������˵�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */       
int MainMenuAction::done (SceneUser* user,Vars* vars)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::stVisitNpcTradeUserCmd *cmd=(Cmd::stVisitNpcTradeUserCmd *)buf;
  bzero(buf,sizeof(buf));
  constructInPlace(cmd);

  strcpy(cmd->menuTxt,_menu.c_str());  
  cmd->byReturn = 1;
  user->sendCmdToMe(cmd,sizeof(Cmd::stVisitNpcTradeUserCmd) + strlen(cmd->menuTxt));

  return Action::SUCCESS;
}


/**     
 * \brief  ˢ�±���
 *
 * ������done����,ˢ���������ֵ�����û�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */    
int RefreshAction::done (SceneUser* user,Vars* vars)
{
  if (_id) {
    Vars* v = user->quest_list.vars(_id);
    if (v) {
      v->notify(*user,_name);
    }
  }else {
    vars->notify(*user,_name);
  }

  return Action::SUCCESS;
}

/**     
 * \brief  ���Ӿ���
 *
 * ������done����,�����û��ľ���
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */    
int ExpAction::done (SceneUser* user,Vars* vars)
{
  Vars* vs = vars;
  if (_id) vs = user->quest_list.vars(_id);

  if (!vs) {
    user->addExp(_exp);
    return Action::SUCCESS;
  }

  int ratio;
  if (!vs->get_value(_name,ratio)) {
    user->addExp(_exp);
    return Action::SUCCESS;
  }
  
  //add exp bonus  
  user->addExp(int(_exp*(ratio/100.0)));  
  return Action::SUCCESS;
}

int EnterSeptGuardAction::done(SceneUser* user,Vars* vars)
{
  user->enterSeptGuard();
  return Action::SUCCESS;
}

int FinishSeptGuardAction::done(SceneUser* user,Vars* vars)
{
  user->finishSeptGuard();
  return Action::SUCCESS;
}

int Exp1Action::done (SceneUser* user,Vars* vars)
{
  Vars* vs = vars;
  if (_id) vs = user->quest_list.vars(_id);

  if (!vs) {
    return Action::SUCCESS;
  }

  int exp;
  if (!vs->get_value(_name,exp)) {
    return Action::SUCCESS;
  }
  
  //add exp bonus  
  user->addExp(exp);  
  return Action::SUCCESS;
}

/**     
 * \brief  ���Ӽ��ܵȼ�
 *
 * ������done����,�����û�ĳһ���ܵĵȼ�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int SkillAction::done (SceneUser* user,Vars* vars)
{
  user->upgradeSkill(_id,false);
  return Action::SUCCESS;
}

/**     
 * \brief  ��ֹ����
 *
 * ������done����,��ֹ�û����ض��Ķ���
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int DisableAction::done (SceneUser* user,Vars* vars)
{
  return Action::DISABLE;
}

/**     
 * \brief  ��ת
 *
 * ������done����,ʹ�û���ת���ض���ͼλ��
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int GotoAction::done (SceneUser* user,Vars* vars)
{
  
  if (_name.length()/*strcmp(user->scene->name,_name.c_str())*/)  {
    
    //Zebra::logger->debug("����ͼ��%s",_name.c_str());
    std::string tmpStr = "name=" + _name + " pos=" +  _pos;
    bool bret=false;
    switch(randBetween(1,4))
    {
      case 1:
        if (_pos1.length() >3) {tmpStr = "name=" + _name + " pos=" +  _pos1;bret=true;}
        break;
      case 2:
        if (_pos2.length() >3) {tmpStr = "name=" + _name + " pos=" +  _pos2;bret=true;}
        break;
      case 3:
        if (_pos3.length() >3) {tmpStr = "name=" + _name + " pos=" +  _pos3;bret=true;}
        break;
      case 4:
        if (_pos4.length() >3) {tmpStr = "name=" + _name + " pos=" +  _pos4;bret=true;}
        break;
    }
    if (!bret)
    {
      if (_cpos.length() >4 )
      {
        if (_rlen.length( ) > 4)
          tmpStr = "name=" + _name + " cpos=" + _cpos + " rlen=" + _rlen;
        else
          tmpStr = "name=" + _name + " cpos=" + _cpos;
      }
    }
    // */
    Gm::gomap(user,tmpStr.c_str());
  }
  //Zebra::logger->debug("�����꣺%s",_pos.c_str());
  else
  {
    std::string tmpStr = _pos;

    switch(randBetween(1,4))
    {
      case 1:
        if (_pos1.length() >3) tmpStr = _pos1;
        break;
      case 2:
        if (_pos2.length() >3) tmpStr = _pos2;
        break;
      case 3:
        if (_pos3.length() >3) tmpStr = _pos3;
        break;
      case 4:
        if (_pos4.length() >3) tmpStr = _pos4;
        break;
    }
    Gm::goTo(user,tmpStr.c_str());  
  }
  
  return Action::SUCCESS;
}

/**     
 * \brief  ������Ʒ
 *
 * ������done����,���û������������ض���Ʒ
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int AddItemAction::done (SceneUser* user,Vars* vars)
{
  if (!_odds || selectByTenTh(_odds)) {
    user->addObjectNum(_id,_value);
  }
  return Action::SUCCESS;
}

/**     
 * \brief  ���Ӱ���Ʒ
 *
 * ������done����,���û������������ض�����Ʒ
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int AddBindItemAction::done (SceneUser* user,Vars* vars)
{
  if (!_odds || selectByTenTh(_odds)) {
    user->addObjectNum(_id,_value,0,0,true);
  }
  return Action::SUCCESS;
}

/**     
 * \brief  ������ɫ����Ʒ
 *
 * ������done����,���û������������ض���ɫ����Ʒ
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int AddGreenBindItemAction::done (SceneUser* user,Vars* vars)
{
  if (!_odds || selectByTenTh(_odds)) {
    user->addGreenObjectNum(_id,_value,0,0,true);
  }
  return Action::SUCCESS;
}

/**     
 * \brief  ɾ����Ʒ
 *
 * ������done����,���û�������ɾ���ض���Ʒ
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int RemoveItemAction::done (SceneUser* user,Vars* vars)
{
  user->reduceObjectNum(_id,_value);
  return Action::SUCCESS;
}

/**     
 * \brief  ������Ʒ
 *
 * ������done����,���û������ж����ض���Ʒ
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int DropItemAction::done (SceneUser* user,Vars* vars)
{
  zObject* ob = user->packs.uom.getObjectByID(_id,_level,true);
  if (ob) user->packs.moveObjectToScene(ob,user->getPos());
  
  return Action::SUCCESS;
}

/**     
 * \brief δʵ��
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int DropAction::done (SceneUser* user,Vars* vars)
{
/*
  zObject* ob = user->packs.main.getObjectByID(_id,_level,true);
  if (ob) user->packs.moveObjectToScene(ob,user->getPos());
*/  
  return Action::SUCCESS;
}

/**     
 * \brief  ����
 *
 * ������done����,����û���������ʹ�û�����
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int RideDownAction::done (SceneUser* user,Vars* vars)
{
//  Cmd::stRideMapScreenUserCmd ret;
//  ret.dwUserTempID = user->tempid;
//  ret.bySwitch = 0;
//  clear_state(user->getByState(),Cmd::USTATE_RIDE);
//  user->dwHorseID = 0;
//  user->sendMeToNine();
  user->horse.mount(false);
  
//  user->scene->sendCmdToNine(user->getPosI(),&ret,sizeof(ret));
  
  return Action::SUCCESS;
}

/**     
 * \brief  ��ʱ��
 *
 * ������done����,�����û���ɸ������ʱ������
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int TimeoutsAction::done (SceneUser* user,Vars* vars)
{

  if (_id) {
    Vars* v = user->quest_list.vars(_id);
    if (v) {
      v->set_timer();
    }
  }else {
    vars->set_timer();  
  }

  return Action::SUCCESS;
}

/**     
 * \brief  �趨״̬
 *
 * ������done����,�趨�û���״̬
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int SetStateAction::done (SceneUser* user,Vars* vars)
{
  user->setUState(_state);
  return Action::SUCCESS;
}

/**     
 * \brief  ���״̬
 *
 * ������done����,����û���״̬
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int ClearStateAction::done (SceneUser* user,Vars* vars)
{
  user->clearUState(_state);
  return Action::SUCCESS;
}

/**     
 * \brief  ��ƥ
 *
 * ������done����,�����û���ƥ
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int HorseAction::done (SceneUser* user,Vars* vars)
{
  user->horse.horse(_id);
  return Action::SUCCESS;
}


/**     
 * \brief  ���NPC
 *
 * ������done����,�ڵ�ͼ������һ��NPC
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int AddNpcAction::done (SceneUser* user,Vars* vars)
{
  zNpcB *base = npcbm.get(_id);
  if (!base) return  Action::FAILED;

  t_NpcDefine define;
  define.id = base->id;
  strcpy(define.name,base->name);
  define.pos = _ltpos;
  define.width = _rbpos.x - _ltpos.x;
  define.height = _rbpos.y - _ltpos.y;
  define.num = _num;
  define.interval = 30;
  define.initstate = zSceneEntry::SceneEntry_Normal;

  if (!_s) {
    return  Action::FAILED;
  }

  _s->zPosRevaluate(define.pos);
  _s->initRegion(define.region,define.pos,define.width,define.height);

  _s->summonNpc(define,define.pos,base,user->dupIndex);

  //Zebra::logger->debug("����npc�ɹ�");  
  
  return Action::SUCCESS;
}

/**     
 * \brief  ɾ��NPC
 *
 * ������done����,�ڵ�ͼ��ָ����Χɾ��һ��NPC
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int RemoveNpcAction::done (SceneUser* user,Vars* vars)
{  
  SceneNpcManager::getMe().removeNpc_if (_remove);
  
  return Action::SUCCESS;
}

/**     
 * \brief  ����ڳ�
 *
 * ������done����,�ڵ�ͼ�����һ���ڳ�
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int AddGuardAction::done(SceneUser* user,Vars* vars)
{
  DWORD i=_id;

  DWORD o = randBetween(1,100);
  if (_id3 && (o>(_odds1+_odds2)*100/(_odds1+_odds2+_odds3)))
    i=_id3;
  else if (_id2 && (o>_odds1*100/(_odds1+_odds2+_odds3)))
  {
    i=_id2;
    Channel::sendSys(user,Cmd::INFO_TYPE_GAME,"��ϲ����ӵ���һ���޵��ڳ���");
  }

  zNpcB *base = npcbm.get(i);
  if (!base) return  Action::FAILED;

  if (user->guard) {
    Channel::sendSys(user,Cmd::INFO_TYPE_FAIL,"�㻹�ڻ�����!");
    return  Action::FAILED;
  }

  t_NpcDefine define;
  define.id = base->id;
  //_snprintf(define.name,MAX_NAMESIZE,"%s��%s",user->name,base->name );
  strcpy(define.name,base->name);
  define.pos = _ltpos;
  define.width = _rbpos.x - _ltpos.x;
  define.height = _rbpos.y - _ltpos.y;
  define.num = _num;
  define.interval = 30;
  define.initstate = zSceneEntry::SceneEntry_Normal;
  //define.scriptID = _script;
  //define.setPath(_path.c_str());
  user->scene->initRegion(define.region,define.pos,define.width,define.height);

  //if (!_s) {
  //  return  Action::FAILED;
  //}

  user->scene->zPosRevaluate(define.pos);

  GuardNpc* npc = user->scene->summonOneNpc<GuardNpc>(define,define.pos,base,user->dupIndex);
  if (!npc)
  {
    Zebra::logger->error("�ٻ� %s ʧ��",define.name);
    return Action::FAILED;
  }
  npc->setMaster(user);
  npc->setPetType(Cmd::PET_TYPE_GUARDNPC);
  user->guard = npc;
  //t_NpcAIDefine  ai;
  //ai.type = NPC_AI_PATROL;
  //npc->setAI(ai);
  //npc->sendData();
  /*
  GuardNpc* npc = user->scene->summonPet(user,Cmd::PET_TYPE_GUARDNPC,define,define.pos,base);
  if (!npc) {
    Zebra::logger->error("�ٻ� %s ʧ��",define.name);
    return Action::FAILED;
  }
  npc->setMaster(user);
  user->pets.push_back(npc);

  //set owner
  */
  npc->gold(_gold);
  if (user->venterSeptGuard.size())//��������
  {
    DWORD m=0;
    for (DWORD i=0; i<user->venterSeptGuard.size(); i++)
      m += user->venterSeptGuard[i].money;
    npc->gold(m);
    npc->isSeptGuard = true;
    Zebra::logger->debug("%s(%u) ���Ӽ����ڳ� gold=%u",user->name,user->id,m);
  }
  npc->owner(user);
  npc->dest(_dest);
  npc->map(_map);
  npc->exp(_exp);
  SceneNpcManager::getMe().addSpecialNpc(npc);

  return Action::SUCCESS;
}

/**     
 * \brief  ������������
 *
 * ������done����,�����û���������
 *      
 * \param user: �����������û�
 * \param vars: �û������ĸ�������ر���
 * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
 */
int FiveTypeAction::done (SceneUser* user,Vars* vars)
{
  user->charbase.fivetype = _type;
  Cmd::stMainUserDataUserCmd  userinfo;
  user->full_t_MainUserData(userinfo.data);
  user->sendCmdToMe(&userinfo,sizeof(userinfo));
  
  return Action::SUCCESS;
}

int UseSkillAction::done (SceneUser* user,Vars* vars)
{
  user->sendSkill(_id,_level);
  
  return Action::SUCCESS;
}

