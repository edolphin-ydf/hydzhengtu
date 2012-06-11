/**
 * \brief  ����ϵͳ
 * 
 */
#include <zebra/ScenesServer.h>

/**     
 * \brief  ���캯��
 *
 * ��ʼ����ر���
 *      
 */    
Event::Event() : _id(0)
{
  
}

/**     
 * \brief ��������
 *
 */
Event::~Event()
{

}

/**     
 * \brief  ��������ű�
 *
 * ��������ű�,���ɶ�Ӧ���¼��ṹ
 *      
 * \param file_name: �ļ���
 * \return true��ʾ�����ű��ɹ�,false��ʾ�����ű�ʧ��
 */  
bool Event::parse(const std::string& file_name)
{  
  zXMLParser xml;
  if (!xml.initFile(file_name))
  {
    Zebra::logger->error("���������ļ� %s ʧ��",file_name.c_str());
    return false;
  }

  xmlNodePtr root = xml.getRootNode("event");
  
  if (root)
  {
    if (!xml.getNodePropNum(root,"id",&_id,sizeof(_id))) {
      return false;
    }
    bzero(_npc_name,MAX_NAMESIZE);
    xml.getNodePropStr(root,"name",_npc_name,MAX_NAMESIZE);

    xmlNodePtr node = xml.getChildNode(root,NULL);  
    while (node) {
      //parse quest
      if (0 == strcmp((char *)node->name,"quest")) {
        DWORD quest_id;
        if (!xml.getNodePropNum(node,"id",&quest_id,sizeof(quest_id))) {
          return false;
        }
        
        int new_quest;
        if (xml.getNodePropNum(node,"new",&new_quest,sizeof(new_quest))) {
          if (new_quest) {
            quest_id |= (0x1 << 31);  
          } 
        }        
        
        //parse embranchment
        xmlNodePtr embranchment_node = xml.getChildNode(node,NULL);
        EMBRANCHMENT* embranchments = new EMBRANCHMENT;
        while (embranchment_node) {
          if (0 == strcmp((char *)embranchment_node->name,"embranchment")  ) {  
            DWORD embranchment_id;
            if (!xml.getNodePropNum(embranchment_node,"id",&embranchment_id,sizeof(embranchment_id))) {
              return false;
            }
            
            Embranchment* embranchment = new Embranchment;
            int active;
            if (xml.getNodePropNum(embranchment_node,"active",&active,sizeof(active))) {
              embranchment->active(true);
            }

            //conditions and actions
            
            xmlNodePtr c_a_node = xml.getChildNode(embranchment_node,NULL);
            while (c_a_node) {
              //parse conditions
              
              if (0 == strcmp((char *)c_a_node->name,"conditions")  ) {  
                
                xmlNodePtr conditions_node = xml.getChildNode(c_a_node,NULL);
                while (conditions_node) {
                  
                  MakeFunction::Maker<Condition>* maker = ConditionFactory::instance().get_creator((char *)conditions_node->name);
                  if (maker) {
                    Condition* condition = maker->make(xml,conditions_node);
                    embranchment->add(condition);
                  }

                  conditions_node = xml.getNextNode(conditions_node,NULL);
                }
              } //endif conditions

              if (0 == strcmp((char *)c_a_node->name,"actions")  ) {  

                xmlNodePtr actions_node = xml.getChildNode(c_a_node,NULL);
                while (actions_node) {

                  MakeFunction::Maker<Action>* maker = ActionFactory::instance().get_creator((char *)actions_node->name);
                  if (maker) {
                    Action* action = maker->make(xml,actions_node);
                    embranchment->add(action);
                  }
                  
                  actions_node = xml.getNextNode(actions_node,NULL);
                }
              } //endif actions              
              
              (*embranchments)[embranchment_id] = embranchment;
              c_a_node = xml.getNextNode(c_a_node,NULL);
            } //endif c_a
            
            
          } //endif embranchment
          embranchment_node = xml.getNextNode(embranchment_node,NULL);
        }
        _quests[quest_id] = embranchments;

      }
      
      node = xml.getNextNode(node,NULL);
    }
    
    return true;
  }

  return false;
}

/**     
 * \brief  ����״̬
 *
 * ȡ���û��������״̬
 *      
 * \param user: ������û�
 * \return ����״̬
 */
int Event::state(SceneUser& user)
{
  int status = -1;
  for (quest_iterator it=_quests.begin(); it!=_quests.end(); ++it) {
    DWORD quest_id = (it->first & ~(0x1 << 31));
    //hide status
    if (quest_id >= 100000) continue;

    Vars* vars = user.quest_list.vars(quest_id);
    if (vars) {
      int var_state = vars->state();
      if (var_state > Quest::DOING) {
        status = Cmd::USTATE_DOING_QUEST;
        return status;
      }
    }
    
    bool new_vars = false;

    if (!vars && ( (it->first >> 31) & 0x1) ) {
      new_vars = true;
      vars = new Vars(quest_id);
    }
    if (vars ) {

      embranchment_iterator e_it = it->second->begin();
      while (e_it!= it->second->end()) {
        if (e_it->second->is_valid(&user,vars)) {
          int var_state = vars->state();
          if (var_state == Quest::FINISHED_SAVE) {
            ++e_it;
            continue;
          }
          if (var_state == Quest::FINISHED) {
            status = Cmd::USTATE_FINISH_QUEST;
/*            
          } else if (var_state > Quest::DOING) {
            status = Cmd::USTATE_DOING_QUEST;
*/
          } else {
            status = Cmd::USTATE_START_QUEST;
          }

          if (new_vars) SAFE_DELETE(vars);
          return status;
        }
        
        ++e_it;  
      }
    }
    
    if (new_vars) SAFE_DELETE(vars);
  }
  
  return status;
}

/**     
 * \brief  ִ������
 *
 * ִ��ȫ�������¼�
 *      
 * \return ������
 */
int Event::execute()
{
  int result = 0;
  
  for (quest_iterator it=_quests.begin(); it!=_quests.end(); ++it) {
    DWORD quest_id = (it->first & ~(0x1 << 31));
    Vars vars(quest_id);
    embranchment_iterator e_it = it->second->begin();
    while (e_it!= it->second->end()) {
      if (!e_it->second->active() && e_it->second->is_valid(NULL,&vars)) {
        result |= e_it->second->do_it(NULL,&vars);
      }
      
      ++e_it;  
    }
  }
  
  return result;
}

/**     
 * \brief  ִ������
 *
 * ִ���û��������б��������¼�
 *      
 * \param user: ������û�
 * \return ������
 */
int Event::execute(SceneUser& user)
{
  int result = 0;

  int old_state = state(user);
      
  for (quest_iterator it=_quests.begin(); it!=_quests.end(); ++it) {
    
    DWORD quest_id = (it->first & ~(0x1 << 31));
    
    bool new_vars = false;
    //user have started this quest
    Vars* vars = user.quest_list.vars(quest_id);
    if (vars ) {
      embranchment_iterator e_it = it->second->begin();
      while (e_it!= it->second->end()) {
        if (!e_it->second->active() && e_it->second->is_valid(&user,vars)) {
          result |= e_it->second->do_it(&user,vars);
          if (vars->update()) vars->update(0);
          //break;
        }        
          
        ++e_it;
      }
    }else {
      bool new_quest = (it->first >> 31) & 0x1;
      if ((!new_quest) ||(new_quest && user.quest_list.count() < QuestList::MAX_NUM) ) {
        vars = new Vars(quest_id);
        new_vars = true;

        embranchment_iterator e_it = it->second->begin();
        while (e_it != it->second->end()) {
          if (!e_it->second->active() && e_it->second->is_valid(&user,vars)) {
            if (new_quest) {
              user.quest_list.add_quest(quest_id,*vars,user);
              Vars* vars = user.quest_list.vars(quest_id);
              result |= e_it->second->do_it(&user,vars);  
            }else {
              result |= e_it->second->do_it(&user,vars);  
            }
/*
            result |= e_it->second->do_it(&user,vars);
            //new quest
            if (new_quest ) user.quest_list.add_quest(quest_id,*vars,user);
            //break;

*/
          }        
          ++e_it;
        }
        
      }
    }
    if (new_vars) SAFE_DELETE(vars);
  }

  int new_state = state(user);
  user.quest_list.update(user,old_state!=new_state);
  
  return result;  
}

/**     
 * \brief  ִ������
 *
 * ִ���û����������֧���¼�
 *      
 * \param user: ������û�
 * \param quest_id: ����id
 * \param offset: �����֧ 
 * \return ������
 */
int Event::execute(SceneUser& user,DWORD quest_id,DWORD offset)
{
  int result = 0;

  int old_state = state(user);
  
  quest_iterator it = _quests.find(quest_id);
  if (it != _quests.end()) {
    Vars* vars = user.quest_list.vars(quest_id);
    bool new_vars = false;
    if (!vars && ( (it->first >> 31) & 0x1) ) {
      if ((user.quest_list.count() > QuestList::MAX_NUM) ) return 0;      
      new_vars = true;
      vars = new Vars(quest_id);
    }
    if (vars ) {
      embranchment_iterator e_it = it->second->find(offset);
      if (e_it != it->second->end()) {
        if (e_it->second->is_valid(&user,vars)) {
          if ((it->first >> 31) & 0x1) {
            user.quest_list.add_quest(quest_id,*vars,user);
            Vars* vars = user.quest_list.vars(quest_id);
            result = e_it->second->do_it(&user,vars);  
            if (vars->update()) vars->update(0);
          }else {
            result = e_it->second->do_it(&user,vars);  
          }
        }
      }

    }
    if (new_vars) SAFE_DELETE(vars);
  }

  int new_state = state(user);
  user.quest_list.update(user,old_state!=new_state);
  
  return result;
}

/**     
 * \brief  ���캯��
 *
 * ��ʼ����ر���
 *      
 */    
Event::Embranchment::Embranchment() : _active(false)
{

}

/**     
 * \brief ��������
 *
 */
Event::Embranchment::~Embranchment()
{
  //free memory
//  for_each(_conditions.begin(),_conditions.end(),FreeMemory());
//  for_each(_actions.begin(),_actions.end(),FreeMemory());
}  

/**     
 * \brief  �ж�һ�������¼���֧�Ƿ���Ч
 *      
 * \param user: ������û�
 * \param vars: �û������ĸ�������ر���
 * \return ��Ч����true,���򷵻�false
 */
bool Event::Embranchment::is_valid(SceneUser* user,const Vars* vars)
{
  for (std::vector<Condition* >::const_iterator it=_conditions.begin(); it!=_conditions.end(); ++it) {
    if (!(*it)->is_valid(user,vars)) {
      return false;
    }    
  }

  return true;
}

/**     
 * \brief  �ж�һ�������¼��Ƿ������¼�
 *      
 * \return �����¼�����true,�����¼�����false
 */
bool Event::Embranchment::active() const
{
  return _active;
}

/**     
 * \brief  ����һ�������¼��Ƿ�Ϊ�����¼�
 *      
 * \param flag: �Ƿ������¼�
 * \return ��
 */
void Event::Embranchment::active(bool flag)
{
  _active = flag;
}

/**     
 * \brief  ִ��һ�������¼����з�֧
 *      
 * \param user: ������û�
 * \param vars: �û������ĸ�������ر���
 * \return ִ�н��
 */
int Event::Embranchment::do_it(SceneUser* user,Vars* vars)
{
  int result = 0;
  for (std::vector<Action* >::iterator it=_actions.begin(); it!=_actions.end(); ++it) {
    result |= (*it)->do_it(user,vars);
    if (result & Action::FAILED) break;
  }

  return result;
}

/**     
 * \brief �������֧�����һ����������
 *      
 * \param condition: ��������
 * \return ��
 */
void Event::Embranchment::add(Condition* condition)
{
  if (condition) _conditions.push_back(condition);
}

/**     
 * \brief �������֧�����һ����������
 *      
 * \param action: ��������
 * \return ��
 */
void Event::Embranchment::add(Action* action)
{
  if (action) _actions.push_back(action);
}

/**     
 * \brief �õ�һ���û����Խӵ������б�
 *      
 * \param user: �û�
 * \return ��
 */
template<>
void EventManager<OnVisit>::get_valid_quest(SceneUser& user,bool showID)
{
  bool have = false;
  std::stringstream menu;
  //menu<<"function MainDialog() this:AddDialogItem(\"�鿴��������\",\"Dialog63\") end function ";
  //menu<<"Dialog63() this:AddTalk(\"<p><n color=\"255,239,196,0\">����NPC�б�</n>";
  menu<<"function IsHasTask() return false end function TaskDialog() end ";
  menu<<"function MainDialog() ";
  menu<<"this:AddTalk(\"<p><n>ȥ����Щ�ˣ��������µ��������:\\n\\n</n> ";
  for (hash_map<DWORD,Event>::iterator it=_events.begin(); it!=_events.end(); it++)
  {
    if (it->second.state(user)==Cmd::USTATE_START_QUEST)
    {
      //Channel::sendSys(&user,Cmd::INFO_TYPE_GAME,"����:npc=%s id=%u",it->second.npc_name(),it->first);
      if (it->second.npc_name()[0]!='-')
      {
        if (showID)
          menu<<"<n>\\t"<<it->second.npc_name()<<"\\t(id="<<it->first<<")"<<"\\n</n>";
        else
          menu<<"<n>\\t"<<it->second.npc_name()<<"\\n</n>";
        have = true;
      }
    }
  }
  menu<<"</p> \") this:AddCloseItem(\"֪����\")end this:AddDialog(\"MainDialog\")";

  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::stVisitNpcTradeUserCmd *cmd=(Cmd::stVisitNpcTradeUserCmd *)buf;
  bzero(buf,sizeof(buf));
  constructInPlace(cmd);

  if (have)
    strncpy(cmd->menuTxt,menu.str().c_str(),sizeof(cmd->menuTxt));
  else
    strncpy(cmd->menuTxt,"����û���µ��������",sizeof(cmd->menuTxt));
  cmd->byReturn = 1;
  user.sendCmdToMe(cmd,sizeof(Cmd::stVisitNpcTradeUserCmd) + strlen(cmd->menuTxt));
#ifdef _DEBUG
  Zebra::logger->debug(cmd->menuTxt);
#endif
}

template<>
void EventManager<OnVisit>::get_valid_quest_str(SceneUser &user)
{
  bool have = false;
  std::stringstream menu;
  char tmp[MAX_NAMESIZE];
  int x=0,y=0;
  char *tok = 0;

  menu<<"<?xml version=\"1.0\" encoding=\"GB2312\"?>";
  menu<<"<body>";
  menu<<"<p><n>ȥ����Щ�ˣ��������µ��������:</n></p>";
  for (hash_map<DWORD,Event>::iterator it=_events.begin(); it!=_events.end(); it++)
  {
    if (it->second.state(user)==Cmd::USTATE_START_QUEST)
    {
      if (it->second.npc_name()[0]!='&')
      {
        bzero(tmp,sizeof(tmp));
        strncpy(tmp,it->second.npc_name(),MAX_NAMESIZE-1);
        tok = strtok(tmp,"-(,)");
        if (tok)
        {
          if (0==strcmp(user.scene->getRealName(),tok))
          {
            menu<<"<p><n color=\"255,0,250,0\">"<<tok;
            tok = strtok(NULL,"-(,)");
            menu<<"-"<<tok<<"</n>";
            tok = strtok(NULL,"-(,)");
            if (tok)
            {
              x = atoi(tok);
              tok = strtok(NULL,"(,)");
              if (tok) y = atoi(tok);

              menu<<"<a href=\"goto "<<x<<","<<y<<"\">("<<x<<","<<y<<")</a></p>";
            }
          }
          else
            menu<<"<p><n color=\"255,0,250,0\">"<<it->second.npc_name()<<"</n></p>";

          tok = 0;
          have = true;
        }
      }
    }
  }
  menu<<"</body>";

  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::stRetValidQuestUserCmd *cmd=(Cmd::stRetValidQuestUserCmd *)buf;
  bzero(buf,sizeof(buf));
  constructInPlace(cmd);

  if (have)
    strcpy_s(cmd->content, sizeof(buf)-(DWORD)((DWORD)(&(cmd->content))-(DWORD)cmd), menu.str().c_str());
  else
    strcpy_s(cmd->content,sizeof(buf)-(DWORD)((DWORD)(&(cmd->content))-(DWORD)cmd),"����û���µ��������");
  cmd->size = strlen(cmd->content) + 1;
  user.sendCmdToMe(cmd,sizeof(Cmd::stRetValidQuestUserCmd) + strlen(cmd->content) + 1);
#ifdef _DEBUG
  Zebra::logger->debug(cmd->content);
#endif
}

const std::string Trigger::Use::DIR = "on_use/";
const std::string Trigger::Kill::DIR = "on_kill/";
const std::string Trigger::KillByLevel::DIR = "on_kill_by_level/";
const std::string Trigger::KillBySelf::DIR = "on_kill_by_self/";
const std::string Trigger::Get::DIR = "on_get/";
const std::string Trigger::Visit::DIR = "on_visit/";
const std::string Trigger::Die::DIR = "on_die/";
const std::string Trigger::Ride::DIR = "on_ride/";
const std::string Trigger::Quit::DIR = "on_quit/";
const std::string Trigger::Timer::DIR = "on_timer/";
const std::string Trigger::Drop::DIR = "on_drop/";
const std::string Trigger::Enter::DIR = "on_enter/";
const std::string Trigger::Other::DIR = "on_other/";
