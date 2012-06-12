/**
 * \brief    ����ϵͳ
 * 
 */
#include <zebra/ScenesServer.h>

/**     
 * \brief  ������������
 *
 * ����ͻ������������ָ��,������������ͷ�������
 *      
 * \param user: ����������û�
 * \param cmd: ����ָ������
 * \param len: ����ָ���
 * \return ����ɹ�����true,���򷵻�false
 */
bool Quest::execute(SceneUser& user,Cmd::stQuestUserCmd* cmd,DWORD len)
{
  if (user.tradeorder.hasBegin()) {
    user.tradeorder.cancel();
  }
  
  switch (cmd->byParam)  
  {
    case Cmd::REQUEST_QUEST_PARA :
      {      
        Cmd::stRequestQuestUserCmd *request=(Cmd::stRequestQuestUserCmd *)cmd;
        char target = request->target[0];
        DWORD event_id = atoi((char *)&request->target[1]);
        
        if (target == 'v') {
          OnVisit event(event_id,request->id,request->offset);
          EventTable::instance().execute(user,event);
        }else {
          OnUse event(event_id,request->id,request->offset);
          EventTable::instance().execute(user,event);
        }

        BYTE buf[zSocket::MAX_DATASIZE];
        Cmd::stVisitNpcTradeUserCmd *cmd=(Cmd::stVisitNpcTradeUserCmd *)buf;
        bzero(buf,sizeof(buf));
        constructInPlace(cmd);
        
        SceneNpc *sceneNpc = SceneNpcManager::getMe().getNpcByTempID(user.npc_dwNpcTempID);
        if (sceneNpc)
        {
//          if (execute_script_event(request->target,request->id,request->offset)) return true;
          if (request->offset > 0 && ScriptQuest::get_instance().has(ScriptQuest::NPC_VISIT,sceneNpc->id))
            execute_script_event(&user,request->target,sceneNpc,request->id,request->offset);
          
          int status;
          int len = user.quest_list.get_menu(cmd->menuTxt,status);
          
          if (status > 0 && NpcTrade::getInstance().getNpcMenu(sceneNpc->id,cmd->menuTxt+len))
          {
            cmd->byReturn = 1;
            user.sendCmdToMe(cmd,sizeof(Cmd::stVisitNpcTradeUserCmd) + strlen(cmd->menuTxt));
          }
        }else {
          if ((signed char)request->offset < 0) {
            execute_script_event(&user,request->target,sceneNpc,request->id,request->offset);
          }
        }

        return true;

      }
      break;
    
    
    case Cmd::ABANDON_QUEST_PARA :
      {
        Cmd::stAbandonQuestUserCmd *request=(Cmd::stAbandonQuestUserCmd *)cmd;
        if (request->id >= 21000 && request->id <= 22000) {
          return Channel::sendSys(&user,Cmd::INFO_TYPE_SYS,"�������ܷ���!");
        }
        Quest::abandon(user,request->id);
        user.sendNineToMe(); //refresh quest state
        //user.sendMeToNine();
        /*Cmd::stMainUserDataUserCmd  userinfo;
        user->full_t_MainUserData(userinfo.data);
        user->sendCmdToMe(&userinfo,sizeof(userinfo));
        user->sendMeToNine();*/
        return true;
      }
    case Cmd::CART_CONTROL_QUEST_PARA:
      {
        if (user.guard)
        {
          user.guard->moveAction = !(user.guard->moveAction);
          if (user.guard->canMove())
            Zebra::logger->debug("%s ��ʼ����",user.guard->name);
          else
            Zebra::logger->debug("%s ֹͣ����",user.guard->name);
        }
        return true;
      }
      break;
    case Cmd::CHECK_VALID_QUEST_PARA:
      {
        EventManager<OnVisit>::instance().get_valid_quest(user); 
        return true;
      }
      break;
    case Cmd::REQ_VALID_QUEST_PARA:
      {
        EventManager<OnVisit>::instance().get_valid_quest_str(user); 
        return true;
      }
      break;
    default:
      break;
  }
  
  return false;
}

/**     
 * \brief  ��ȡ����
 *
 * ���û������ж�ȡ�����б�
 *      
 * \param user: ����������û�
 * \param dest: ���񵵰�
 * \return ��ȡ��������Ŀ
 */
int Quest::load(SceneUser& user,BYTE* dest,unsigned long &dest_size)
{
  return user.quest_list.load(dest,dest_size);  
}

/**     
 * \brief �洢����
 *
 *���������б��û�������
 *      
 * \param user: ����������û�
 * \param dest: ���񵵰�
 * \return �洢�Ķ����Ƶ�������
 */
int Quest::save(SceneUser& user,BYTE* dest)
{
  return user.quest_list.save(dest);
}

/**     
 * \brief ֪ͨ����
 *
 *����������Ϣ���û�
 *      
 * \param user: ����������û�
 * \return ��ǰ���Ƿ���0
 */
int Quest::notify(SceneUser& user)
{
  return user.quest_list.notify(user);
}

/**     
 * \brief ��������
 *
 *����һ������
 *      
 * \param user: ����������û�
 * \param id: ����id
 * \return �ɹ�����0,ʧ�ܷ���-1
 */
int Quest::abandon(SceneUser& user,DWORD id)
{
  return user.quest_list.abandon(user,id);
}

const std::string Quest::FINISHED_NAME = "state";

/**     
 * \brief ����ʱ��
 *
 *����һ�������ʱ������
 *      
 * \return ��ǰ���Ƿ���0
 */
int Vars::set_timer()
{
  _start_time = time(NULL);
  
  return 0;
}

int Vars::set_timer(int start)
{
  _start_time = start;
  
  return 0;
}

/**     
 * \brief ����ʼʱ��
 *
 *ȡ������Ŀ�ʼʱ��
 *      
 * \return ����ʼʱ��
 */
int Vars::start_time() const
{
  return _start_time;
}

/**     
 * \brief �����Ƿ�ʱ
 *
 *�ж������Ƿ񳬹�ʱ������
 *      
 * \return ���񳬹�ʱ�����Ʒ���true,���򷵻�false
 */
bool Vars::is_timeout(int timeout) const
{
  if (_start_time && (timeout + _start_time > time(NULL)) ) {
    return true;
  }
  
  return false;
}

/**     
 * \brief �洢ʱ��
 *
 *�洢����Ŀ�ʼʱ���ʱ������
 *      
 * \param dest: ���񵵰�
 * \return �洢�Ķ����Ƶ�������
 */
int Vars::save_timer(BYTE* dest) const
{
  int len = 0;
  memccpy(dest+len,&_timeout,sizeof(_timeout),sizeof(_timeout));
  len += sizeof(_timeout);
  memccpy(dest+len,&_start_time,sizeof(_start_time),sizeof(_start_time));
  len += sizeof(_start_time);
  
  return len;
}

/**     
 * \brief ��ȡʱ��
 *
 *��ȡ����Ŀ�ʼʱ���ʱ������
 *      
 * \param dest: ���񵵰�
 * \return ��ȡ�Ķ����Ƶ�������
 */
int Vars::load_timer(BYTE* dest)
{
  int len = 0;
  memccpy(&_timeout,(dest+len),sizeof(int),sizeof(int));
  len += sizeof(int);
  memccpy(&_start_time,(dest+len),sizeof(int),sizeof(int));
  len += sizeof(int);
  
  return len;
}

/**     
 * \brief �洢����
 *
 *�洢�������
 *      
 * \param dest: ���񵵰�
 * \return �洢�Ķ����Ƶ�������
 */
int Vars::save(BYTE* dest) const
{
  int length = save_timer(dest);
  
  int len = length + sizeof(int);
  int count = 0;
  for (const_var_iterator it=_vars.begin(); it!=_vars.end(); ++it) {
    if (!it->second.is_tmp()) {
      ++count;
      int tmp = it->first.length();
      memccpy(dest+len,&tmp,sizeof(int),sizeof(int));
      len += sizeof(int);
      memccpy(dest+len,it->first.c_str(),it->first.length(),it->first.length());
      len += it->first.length();

      tmp = it->second.value().length();
      memccpy(dest+len,&tmp,sizeof(int),sizeof(int));
      len += sizeof(int);
      memccpy(dest+len,it->second.value().c_str(),it->second.value().length(),it->second.value().length());
      len += it->second.value().length();      
      
      //Zebra::logger->debug("�洢����(%s:%s)",it->first.c_str(),it->second.value().c_str());
    }
  }
  //store count
  memccpy(dest + length,&count,sizeof(int),sizeof(int));  

  return len;
}

/**     
 * \brief ��ȡ����
 *
 *��ȡ�������
 *      
 * \param dest: ���񵵰�
 * \return ��ȡ�Ķ����Ƶ�������
 */
int Vars::load(BYTE* dest)
{
  int length = load_timer(dest);

  int* count = (int*)(dest + length);
  length += sizeof(int);

  while ( (*count)-- > 0) {
    int* len = (int*)(dest + length);
    length += sizeof(int);

    std::string name((char *)dest+length,*len);
    length += *len;
    
    len = (int *)(dest + length);
    length += sizeof(int);

    std::string value((char *)dest+length,*len);
    length += *len;
    
    _vars[name] = VAR(value);
    
    //Zebra::logger->debug("��ȡ����(%s:%s)",name.c_str(),value.c_str());
  }

  int states = state();
  if (states == Quest::FINISHED_NOT_SAVE || states == Quest::FINISHED_SAVE) update(1);
  
  return length;
}

/**     
 * \brief �����Ƿ���Ҫ����
 *
 *�ж��Ƿ���Ҫ֪ͨ�ͻ������������Ϣ
 *      
 * \return 1��ʾ��Ҫ֪ͨ�ͻ���,0��ʾ����Ҫ
 */
int Vars::update() const
{
  return _update;
}

/**     
 * \brief �����Ƿ���Ҫ����
 *
 *�����Ƿ���Ҫ֪ͨ�ͻ������������Ϣ
 *      
 * \param value: �Ƿ���Ҫ֪ͨ�ͻ���
 * \return ��
 */
void Vars::update(int value)
{
  _update = value;
}

/**     
 * \brief ֪ͨ�������
 *
 *�����������������Ϣ���û�
 *      
 * \param user: ����������û�
 * \return ��ǰ���Ƿ���0
 */
int Vars::notify(SceneUser& user) const
{
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::stQuestVarsUserCmd* vars = (Cmd::stQuestVarsUserCmd*)buf;
  constructInPlace(vars);
  vars->id = _quest_id;
  vars->count = _vars.size();
  
  int offset=0;
  for (const_var_iterator it=_vars.begin(); it!=_vars.end(); ++it) {
    if (! it->first.compare(Quest::FINISHED_NAME)) continue;
    if (offset >= (int)(zSocket::MAX_DATASIZE - sizeof(Cmd::stQuestVarsUserCmd)
          - (Cmd::stQuestVarsUserCmd::MAX_NSIZE + Cmd::stQuestVarsUserCmd::MAX_VSIZE) ) ) {
      Zebra::logger->error("����(%d)������̫��",_quest_id);
      break;
    }
    //name
    strncpy((char*)vars->vars_list+offset,it->first.c_str(),Cmd::stQuestVarsUserCmd::MAX_NSIZE);
    offset += Cmd::stQuestVarsUserCmd::MAX_NSIZE;
    //value
    strncpy((char*)vars->vars_list+offset,it->second.value().c_str(),Cmd::stQuestVarsUserCmd::MAX_VSIZE);
    offset += Cmd::stQuestVarsUserCmd::MAX_VSIZE;
    //Zebra::logger->debug("var:(%s,%s)",it->first.c_str(),it->second.value().c_str());
  }
  
  user.sendCmdToMe(vars,sizeof(Cmd::stQuestVarsUserCmd)+ vars->count*sizeof(Cmd::stQuestVarsUserCmd::Var));
  
  return 0;  
}

/**     
 * \brief ֪ͨ�������
 *
 *�����ض����������Ϣ���û�
 *      
 * \param user: ����������û�
 * \param name: ������
 * \return 0 ��ʾ���ͳɹ�,-1��ʾʧ��
 */
int Vars::notify(SceneUser& user,const std::string& name) const
{  
  const_var_iterator it=_vars.find(name);
  if (it != _vars.end()) {
    BYTE buf[sizeof(Cmd::stQuestVarsUserCmd) + (Cmd::stQuestVarsUserCmd::MAX_NSIZE + Cmd::stQuestVarsUserCmd::MAX_VSIZE)];
    Cmd::stQuestVarsUserCmd* vars = (Cmd::stQuestVarsUserCmd*)buf;
    constructInPlace(vars);
    vars->id  = _quest_id;
    vars->count = 1;
    
    int offset = 0;
    //name
    strncpy((char *)vars->vars_list+offset,it->first.c_str(),Cmd::stQuestVarsUserCmd::MAX_NSIZE);
    offset += Cmd::stQuestVarsUserCmd::MAX_NSIZE;
    //value
    strncpy((char *)vars->vars_list+offset,it->second.value().c_str(),Cmd::stQuestVarsUserCmd::MAX_VSIZE);
    offset += Cmd::stQuestVarsUserCmd::MAX_VSIZE;
    user.sendCmdToMe(vars,sizeof(Cmd::stQuestVarsUserCmd)+ vars->count*sizeof(Cmd::stQuestVarsUserCmd::Var));
    //Zebra::logger->debug("var:(%s,%s)",it->first.c_str(),it->second.value().c_str());
    return 0;
  }
  
  
  return -1;  
}

std::string Vars::info() const
{
  std::ostringstream os;
  for (const_var_iterator it=_vars.begin(); it!=_vars.end(); ++it) {
    os << it->first << ":" << it->second.value() << "\t";
  }

  return os.str();
}

bool Vars::reserve()
{
  bool flag = false;
  for (var_iterator it=_vars.begin(); it!=_vars.end(); ) {
    const std::string& name = it->first;
    if (!strncmp(name.c_str(),"r",1) || !strncmp(name.c_str(),"r_",2)) {
      ++it;
      flag = true;
    }else {
      _vars.erase(it++);
    }
  }

  return flag;
}

/**     
 * \brief ����״̬
 *
 *ȡ������ĵ�ǰ״̬
 *      
 * \return ����״̬
 */
int Vars::state() const
{
  const_var_iterator it = _vars.find(Quest::FINISHED_NAME);
  if (it != _vars.end() ) {
    return atoi(it->second.value().c_str());
  }
  
  return 0;
}

#if 0
GlobalVar* GlobalVar::_instance = NULL;

/**     
 * \brief ����ģʽ,��֤ȫ�ֱ����б�Ψһ
 *
 * \return ȫ�ֱ����б��Ψһʵ��
 */
GlobalVar& GlobalVar::instance()
{
  if (!_instance) {
    static GlobalVar new_instance;
    _instance = &new_instance;
  }
  
  return *_instance;
}

/**     
 * \brief ��ѯȫ�ֱ���
 *
 *��ȫ�ֱ����б��в����ض�id��һ������
 *      
 * \param id: ����id
 * \return ȫ�ֱ���,û�ҵ�����NULL
 */
Vars* GlobalVar::vars(DWORD id) const
{
  const_vars_iterator it = _vars.find(id);
  if   (it != _vars.end() ) {
    return it->second;
  }
  
  return NULL;
}

/**     
 * \brief ���ȫ�ֱ���
 *
 *��ȫ�ֱ����б������һ���ض�id�ı���,����ñ����Ѿ�����,����±�����ֵ
 *      
 * \param id: ����id
 * \return ȫ�ֱ���
 */
Vars* GlobalVar::add(DWORD id)
{
  Vars* v = vars(id);
  if (!v) v = new Vars(id);

  _vars[id] = v;

  return v;  
}
#endif

UserVar* UserVar::_instance = NULL;
int UserVar::SERVER_ID = 0;

/**     
 * \brief ����ģʽ,��֤�û������б�Ψһ
 *
 * \return �û������б��Ψһʵ��
 */
UserVar& UserVar::instance()
{
  if (!_instance) {
    static UserVar new_instance;
    _instance = &new_instance;
  }
  
  return *_instance;
}

/**     
 * \brief ��ѯ�û�����
 *
 *���û������б��в����ض���һ������
 *      
 * \param id: ����id
 * \param key: �����û���Ϣhash����һ��keyֵ,Ψһʶ��һ��ie�û�
 * \return �û�����,û�ҵ�����NULL
 */
Vars* UserVar::vars(DWORD id,QWORD key) const
{
  const_vars_iterator it = _vars.find(id);
  if   (it != _vars.end() ) {
    return it->second->vars(key);
  }
  
  return NULL;
}

/**     
 * \brief ����û�����
 *
 *���û������б�������ض���һ������,��������Ѵ��������
 *      
 * \param id: ����id
 * \param key: �����û���Ϣhash����һ��keyֵ,Ψһʶ��һ��ie�û�
 * \return ��ӵ��û�����
 */
Vars* UserVar::add(DWORD id,QWORD key)
{
  VAR* var = NULL; 
  const_vars_iterator it = _vars.find(id);
  if   (it != _vars.end() ) {
    var = it->second;
  }
  
  if (var) {
    return var->add(id,key);
  }
  
  var = new VAR;
  Vars* v = var->add(id,key);
  _vars[id] = var;

  return v;  
}

bool UserVar::save() const
{
  std::ofstream of(_file.c_str(),std::ios::binary);

  char buf[MAX_BUF_SIZE*30];
  bzero(buf,sizeof(buf));

  int count = _vars.size();
  memccpy(buf,&count,sizeof(count),sizeof(buf));
  int len = sizeof(count);
    
  for (const_vars_iterator it=_vars.begin(); it!=_vars.end(); ++it) {
    QWORD id = it->first;
    memccpy(buf+len,&id,sizeof(id),sizeof(buf) - len);
    len += sizeof(id);

    len += it->second->save((BYTE*)buf+len);
    if (len >= (MAX_BUF_SIZE*30 - 1024))
    {
      Zebra::logger->fatal("�洢ȫ�ֱ���ʱ������������Խ�磨%u,%u��",MAX_BUF_SIZE,len);
      of.write(buf,len);
      bzero(buf,sizeof(buf));
      len = 0;
    }
  }

  if (len > 0) of.write(buf,len);
      
  of.flush();
  of.close();
    
  return true;
}

bool UserVar::load()
{
  bool ret = false;
  std::ifstream inf(_file.c_str(),std::ios::binary);

  //����ļ���С
  inf.seekg(0,std::ios::end);
  int length = inf.tellg();
  inf.seekg(0,std::ios::beg);

  if (length > 0)
  {
    char *buf = new char[length];
    if (buf)
    {
      bzero(buf,length);

      inf.read(buf,sizeof(buf));
      int size =*((int*)buf);
      int len = sizeof(int);

      while (size-- > 0) {
        QWORD id = *((QWORD*)(buf+len));
        len += sizeof(id);
        VAR* vars = new VAR;
        len += vars->load((BYTE*)buf+len);
        _vars[id] = vars;
      }
      SAFE_DELETE_VEC(buf);
      ret = true;
    }
    else
      Zebra::logger->fatal("������������ļ������ڴ�ʧ�ܣ�%u",length);
  }

  inf.close();
  return ret;
}

/**     
 * \brief �洢����
 *
 *�洢�������
 *      
 * \param dest: ���񵵰�
 * \return �洢�Ķ����Ƶ�������
 */
int UserVar::VAR::save(BYTE* dest) const
{
  int len = sizeof(int);
  int count = 0;
  for (const_var_iterator it=_vars.begin(); it!=_vars.end(); ++it) {
      ++count;
      memccpy(dest+len,&(it->first),sizeof(it->first),sizeof(it->first));
      len += sizeof(it->first);

      int tmp = it->second->save(dest);
      len += tmp;
  //Zebra::logger->debug("�洢����(%s:%s)",it->first.c_str(),it->second.value().c_str());
  }
  //store count
  memccpy(dest,&count,sizeof(int),sizeof(int));  

  return len;
}

/**     
 * \brief ��ȡ����
 *
 * ��ȡ�������
 *      
 * \param dest: ���񵵰�
 * \return ��ȡ�Ķ����Ƶ�������
 */
int UserVar::VAR::load(BYTE* dest)
{
  int* count = (int*)(dest);
  int length = sizeof(int);

  while ( (*count)-- > 0) {
    QWORD id=(QWORD)(*(dest+length));
    length += sizeof(QWORD);
    Vars* vars= new Vars;
    length +=vars->load((BYTE*)dest+length);
    _vars[id] = vars;
  }

  return length;
}

/**     
 * \brief ��ѯ�������
 *
 *�������б��в����ض�id��һ������
 *      
 * \param id: ����id
 * \return �������,û�ҵ�����NULL
 */
Vars* QuestList::vars(DWORD id) const
{
  const_quest_iterator it = _quests.find(id);
  if (it != _quests.end()) {
    return const_cast<Vars *>(&it->second);
  }

  return NULL;  
}

/**     
 * \brief ��������
 *
 *�������б�������һ������
 *      
 * \param id: ����id
 * \param vars: �������
 * \param user:  Я��������û�
 * \param notify:  �������ʱ�Ƿ�֪ͨ�ͻ���
 * \return ��
 */
void QuestList::add_quest(DWORD id,const Vars& vars,SceneUser& user,bool notify)
{
  _quests[id] = vars;
  if (!notify) return;
  
  const Quest* quest = QuestTable::instance().quest(id);
  if (quest ) {

    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stQuestInfoUserCmd* info = (Cmd::stQuestInfoUserCmd*)buf;
    constructInPlace(info);
    info->id = id;
    int len = (quest->title().length()<63)?quest->title().length():63;
    memccpy(info->name,quest->title().c_str(),len,sizeof(info->name));
    info->name[len] = '\0';
    //info->length = quest->description().length();
    info->start = start_time(id);
    //Zebra::logger->debug("quest time(%d)",info->start);
    memccpy(info->info,quest->description().c_str(),quest->description().length(),sizeof(buf) - sizeof(Cmd::stQuestInfoUserCmd));
    info->info[quest->description().length()] = '\0';
    //Zebra::logger->debug("quest info(%s)",info->info);
    user.sendCmdToMe(info,sizeof(Cmd::stQuestInfoUserCmd)+sizeof(BYTE)*(quest->description().length()+1));
  }  
}

/**     
 * \brief ������Ŀ
 *
 *ȡ�������б���δ��ɵ���������
 *      
 * \return ��������
 */
int QuestList::count() const
{
//  return _quests.size();
  int number = 0;
  for (const_quest_iterator it=_quests.begin(); it!=_quests.end(); ++it) {
    int state = it->second.state();
    if (it->first > 100000 ||state == Quest::FINISHED_NOT_SAVE || state == Quest::FINISHED_SAVE) continue;
    ++number;
  }
  
  return number;
}

#define MARK "%%"
/**     
 * \brief ��������˵�
 *
 *����npc��������Ϣ
 *      
  * \param menu: �˵�����
 * \return �ɹ�����true,ʧ�ܷ���false
 */
bool QuestList::set_menu(const std::string& menu)
{
  if (_menu.empty()  ) {
    if (!_subs.empty() ) {
      std::ostringstream head;
      std::ostringstream tail;
      head << "function TaskDialog()";
      std::list<std::string>::iterator it;
      for (it=_subs.begin(); it!=_subs.end(); ++it) {
        std::string::size_type pos = (*it).find(MARK);
        if (pos != std::string::npos) {
          head << (*it).substr(0,pos);
          tail << (*it).substr(pos+strlen(MARK));
        }
      }
      head << "end";
      _menu = head.str() + tail.str();  
    }else {
      _menu = menu;
    }    

    return true;

  }
  
  return false;
}

/**     
 * \brief ��������˵�
 *
 *����npc��������Ϣ
 *      
  * \param menu: �˵�����
 * \return �ɹ�����true,ʧ�ܷ���false
 */
void QuestList::add_menu(const std::string& menu)
{
  _subs.push_back(menu);
}


#define HAS_TASK_DIALOG "function IsHasTask()\n\treturn true\nend\n"
#define NO_TASK_DIALOG "function IsHasTask()\n\treturn false\nend\nfunction TaskDialog()\nend\n"

/**     
 * \brief ��ѯ����˵�
 *
 *��ѯnpc��������Ϣ
 *      
 * \param menu: ��ѯ���Ľ��
 * \param status: ��ѯ״̬
 * \return �˵��ĳ���
 */
int QuestList::get_menu(char* menu,int& status)
{
  int len;
  if (!_menu.empty() ) {
    len =  zSocket::MAX_DATASIZE - strlen(menu) - sizeof(Cmd::stVisitNpcTradeUserCmd) - strlen(HAS_TASK_DIALOG);
    len = ((int)_menu.length()>len)?len:_menu.length();
    strncpy(menu,_menu.c_str(),len);
    strncpy(menu+len,HAS_TASK_DIALOG,strlen(HAS_TASK_DIALOG));
    len += strlen(HAS_TASK_DIALOG);
    _menu.clear();
    _subs.clear();
    status = 1;
  }else {
    //client request this
    status = 0;
    len = strlen(NO_TASK_DIALOG);
    strncpy(menu,NO_TASK_DIALOG,len);
  }
  
  return len;
}

/**     
 * \brief �洢����
 *
 *���������б��û�������
 *      
 * \param dest: ���񵵰�
 * \return �洢�Ķ����Ƶ�������
 */
int QuestList::save(BYTE* dest) const
{
  int len = 0;
  int tmp = _quests.size();
  memccpy(dest,&tmp,sizeof(int),sizeof(int));
  len += sizeof(int);
  
  for (const_quest_iterator it=_quests.begin(); it!=_quests.end(); ++it) {
    memccpy(dest+len,&it->first,sizeof(DWORD),sizeof(DWORD));
    //Zebra::logger->debug("�洢����(%d)",it->first);
    len += sizeof(DWORD);
    len += it->second.save(dest+len);
  }
    
  return len;
}

/**     
 * \brief  ��ȡ����
 *
 * ���û������ж�ȡ�����б�
 *      
 * \param dest: ���񵵰�
 * \param dest_size: ���񵵰���С
 * \return ��ȡ��������Ŀ
 */
int QuestList::load(BYTE* dest,unsigned long &dest_size)
{
  BYTE* data=dest;
  int* size = (int *)data;
  int count = *size;
  
  data += sizeof(int);
  
  while ((*size)-- > 0) {
    int* quest_id = (int *)data;
    data += sizeof(int);
    //Zebra::logger->debug("��ȡ����(%d)",*quest_id);
    
    Vars vars(*quest_id);
    data += vars.load(data);
    
    _quests[*quest_id] = vars;
  }
  
  dest_size =data - dest;
  return count;    
}

/**     
 * \brief ����״̬
 *
 *��ѯ�����б��е�һ�������״̬
 *      
 * \param id: ����id
 * \return ����״̬,û�ҵ�����0
 */
int QuestList::state(DWORD id) const
{
  Vars* var = vars(id);
  if (!var) return 0;
  
  return var->state();
}

/**     
 * \brief ����ʼʱ��
 *
 *��ѯ�����б��е�һ������Ŀ�ʼʱ��
 *      
 * \param id: ����id
 * \return ����ʼʱ��,û�ҵ����ص�ǰʱ��
 */
int QuestList::start_time(DWORD id) const
{
  Vars* var = vars(id);
  if (!var) return time(NULL);
  
  return var->start_time();
}

/**     
 * \brief ֪ͨ����
 *
 *����������Ϣ���û�
 *      
 * \param user: ����������û�
 * \return ��ǰ���Ƿ���0
 */
int QuestList::notify(SceneUser& user) const
{
  for (const_quest_iterator it=_quests.begin(); it!=_quests.end(); ++it) {
    const Quest* quest = QuestTable::instance().quest(it->first);
    if (quest ) {
      //finished request will not info client
      if (state(it->first) == Quest::FINISHED_SAVE || state(it->first) == Quest::FINISHED_NOT_SAVE) continue;
      
      BYTE buf[zSocket::MAX_DATASIZE];
      Cmd::stQuestInfoUserCmd* info = (Cmd::stQuestInfoUserCmd*)buf;
      constructInPlace(info);
      info->id = it->first;
      int len = (quest->title().length()<63)?quest->title().length():63;
      memccpy(info->name,quest->title().c_str(),len,sizeof(info->name));
      info->name[len] = '\0';
      //info->length = quest->description().length();
      info->start = start_time(it->first);
      //Zebra::logger->debug("quest time(%d)",info->start);
      memccpy(info->info,quest->description().c_str(),quest->description().length(),sizeof(buf) - sizeof(Cmd::stQuestInfoUserCmd));
      info->info[quest->description().length()] = '\0';
      user.sendCmdToMe(info,sizeof(Cmd::stQuestInfoUserCmd)+sizeof(BYTE)*(quest->description().length()+1));

      //Zebra::logger->debug("quest info(%s)",info->info);
      it->second.notify(user);
      
    }else {      
      //Zebra::logger->error("�û�(%d,%d)���в����ڵ�����(%d)",user.accid,user.charbase.id,it->first);
    }
  }
  
  return 0;
}

/**     
 * \brief ��������
 *
 *����һ������
 *      
 * \param user: ����������û�
 * \param id: ����id
 * \param force: �Ƿ�ǿ�Ʒ���
 * \param destroy: �Ƿ�ɾ������
 * \return �ɹ�����0,ʧ�ܷ���-1
 */
int QuestList::abandon(SceneUser& user,DWORD id,bool force,bool destroy)
{
  if (!force &&  (state(id)  ==  Quest::FINISHED_SAVE || state(id) == Quest::FINISHED_NOT_SAVE) ) {
    return -1;
  }

  if (destroy) _quests.erase(id);
  
  Cmd::stAbandonQuestUserCmd ret;
  ret.id = id;
  user.sendCmdToMe(&ret,sizeof(ret));
  
  //need delete quest object in package?
  
  return 0;
}

/**     
 * \brief ������������
 *
 *      
 * \param user: ����������û�
 */

void QuestList::clear(SceneUser* pUser)
{
  for (quest_iterator it=_quests.begin(); it!=_quests.end(); it++) 
  {
    this->abandon(*pUser,it->first,true,false);
  }
  
  _quests.clear();
}

/**     
 * \brief ��������
 *
 *����������Ϣ
 *      
 * \param user: ����������û�
 * \param refresh: �Ƿ���Ҫˢ��
 * \return ��ǰ���Ƿ���0
 */
int QuestList::update(SceneUser& user,bool refresh)
{
  for (quest_iterator it=_quests.begin(); it!=_quests.end();) {
    int state = it->second.state();
    int update =  it->second.update();
    if (state == Quest::FINISHED_NOT_SAVE && !update) {
      //not save,clear all infomation about this quest
      //quest_iterator old_it = it;
      //++old_it;
      DWORD id = it->first;
      int start = it->second.start_time();
      abandon(user,it->first,true,false);
      Vars vars(id);
      vars.set_timer(start);
      Op::Set<int> op;
      vars.set_value(op,Quest::FINISHED_NAME,Quest::FINISHED_NOT_SAVE,0,&user);
      vars.update(1);  //set updated
      add_quest(id,vars,user,false);
      //it = old_it;
      //continue;
    }

    if (state == Quest::FINISHED_SAVE && !update) {      

      //quest_iterator old_it = it;
      //++old_it;
      DWORD id = it->first;
      abandon(user,it->first,true,false);
      //the quest must save,only keep that infomation
      Vars vars(id);
      Op::Set<int> op;
      vars.set_value(op,Quest::FINISHED_NAME,Quest::FINISHED_SAVE,0,&user);
      vars.update(1);
      add_quest(id,vars,user,false);
      //it = old_it;
      //continue;
    }
    
    ++it;
  }

  if (refresh) 
  {
    //Zebra::logger->debug("����״̬�仯�����������������!");

	  TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(user.TeamThisID);

	  if (teamMan) 
	  {
		  Team& team = const_cast<Team&>(teamMan->getTeam());

		  team.rwlock.rdlock();
		  std::vector<TeamMember>::iterator it = team.member.begin();;
		  for(; it!=team.member.end(); ++it) 
		  {
			  SceneUser* member = SceneUserManager::getMe().getUserByTempID(it->tempid);
			  if (member) {
				  member->quest_list.update(*member,false);
				  member->sendNineToMe();
			  }
		  }
		  team.rwlock.unlock();
	  }
	  else 
	  {
		  user.sendNineToMe();      
	  }

  }
  return 0;
}


std::string QuestList::info(int id) const
{
  std::ostringstream os;

  os << "���������б�:\n";
  for (const_quest_iterator it=_quests.begin(); it!=_quests.end(); ++it) {
    std::string name = "δ֪";
    const Quest* quest = QuestTable::instance().quest(it->first);
    if (quest) name = quest->title();
    os << "����(" << name << "," << it->first << ")" << "\n";
    os << it->second.info() << "\n";
  }

  return os.str();
}

