/**
 * \brief  �ű���������
 * 
 */

#include <zebra/ScenesServer.h>
#include <zebra/csCommon.h>
#include "meterialsManager.h"

SceneUser* current_user = NULL;


/**     
 * \brief ȡ�õ�ǰ�û�
 *
 * \return: ��ǰ�û�
 */  
SceneUser* me()
{
  return current_user;
}

/**     
 * \brief �趨��ǰ�û�
 *
 * \param user: Ŀ���û�
 */  
void set_me(SceneUser* user)
{
  current_user = user;
}


/**     
 * \brief �ű��ӿ�,������Ϣ���û�����
 *
 * \param target : Ŀ���û�
 * \param type : ѶϢ����,see enumSysInfoType from Command.h for details
 * \param msg: ��Ϣ����
 * \return: ���ͳɹ�����true,���򷵻�false
 */  
bool sys(SceneUser* target,int type,const char* msg)
{
  return Channel::sendSys(target,type,msg);
}


//ȫ���緢��Ϣ
void globalSys(SceneUser* sender,const char* msg)
{
	DWORD size = sizeof(Cmd::Session::t_GlobalMessage_SceneSession) + strlen(msg);
	char *buf = new char[size];

	Cmd::Session::t_GlobalMessage_SceneSession *send = new(buf) Cmd::Session::t_GlobalMessage_SceneSession;

	strncpy(send->msg,msg,sizeof(send->msg));
	send->dwUserID = sender->getID();
	sessionClient->sendCmd(send,size);
	delete[] buf;
}

//װ������
void equip_make(SceneUser* user,DWORD thisID,bool drop,int flag)
{
	EquipMaker maker(NULL);
    zObject* o = goi->getObjectByThisid(thisID);
	if(o)
		maker.assign(user,o,o->base,1);
}

//������Ʒ
zObject *CreateObject(DWORD objID,DWORD level,DWORD quantity)
{
	zObjectB *base = objectbm.get(objID);

	if(NULL == base)
		return NULL;

	zObject* o = NULL;
	o = zObject::create(base,quantity,level);

	return o;
}

zObject *getObjByTempidFromPackage(SceneUser* user,DWORD thisid)
{
	return user->packs.uom.getObjectByThisID(thisid);
}


void makerName(const char *name,DWORD thisid)
{
	zObject* o = goi->getObjectByThisid(thisid);
	if(o)
	{

	  strncpy(o->data.maker,name,MAX_NAMESIZE);	
	  //Cmd::stAddObjectPropertyUserCmd ret;
      //memcpy(&ret.object,&(o->data),sizeof(t_Object));
      //user->sendCmdToMe(&ret,sizeof(ret));
	}
}


zObject *getObjByIdFromPackage(SceneUser* user,DWORD id)
{
	return user->packs.uom.getObjectByID(id);
}

void infoUserObjUpdate(SceneUser *user,DWORD thisid)
{

    zObject* o = goi->getObjectByThisid(thisid);
	if(o)
	{


	  Cmd::stAddObjectPropertyUserCmd ret;
      memccpy(&ret.object,&(o->data),sizeof(t_Object),sizeof(ret.object));
      user->sendCmdToMe(&ret,sizeof(ret));
	}
}

void assign_set(DWORD thisid)
{
	zObject* o = goi->getObjectByThisid(thisid);
	if(o)
	{
		EquipMaker maker(NULL);
		maker.assign_set(o);
	}
}



/**     
 * \brief �ű��ӿ�,�����Ի���
 *
 * \param npc : �Ի���npc
 * \param menu : �Ի�������
 */  
void show_dialog(SceneNpc* npc,const char* menu)
{
  BYTE buf[4096];
  Cmd::stVisitNpcTradeUserCmd *cmd=(Cmd::stVisitNpcTradeUserCmd *)buf;
  memset(buf,0,sizeof(buf));
  constructInPlace(cmd);
  
  strncpy(cmd->menuTxt,menu,strlen(menu));  
  if (npc && NpcTrade::getInstance().getNpcMenu(npc->id,cmd->menuTxt+strlen(menu))) {
    //intended to be blank
  }
  cmd->byReturn = 1;
  
  if (current_user) {
    current_user->sendCmdToMe(cmd,sizeof(Cmd::stVisitNpcTradeUserCmd) + strlen(cmd->menuTxt));
  }
}

/**     
 * \brief �ű��ӿ�,�趨һ������ֵ
 *
 * \param vars : Ŀ�����
 * \param name : ��������
 * \param value: ����ֵ
 */ 
void set_var(Vars* vars,const char* name,int value)
{
  Op::Set<int> op;
  vars->set_value(op,name,value,0,current_user);
  
}

/**     
 * \brief �ű��ӿ�,�趨һ������ֵ
 *
 * \param vars : Ŀ�����
 * \param name : ��������
 * \param value: ����ֵ
 */ 
void set_varS(Vars* vars,const char* name,const char * value)
{
  Op::Set< std::string> op;
  std::string s=value;
  vars->set_value(op,name,s,0,current_user);
}

/**     
 * \brief �ű��ӿ�,ȡ��һ������ֵ
 *
 * \param vars : Ŀ�����
 * \param name : ��������
 * \return: ȡ�õı���ֵ,�����������ڷ���0
 */ 
int get_var(Vars* vars,const char* name)
{
  int value;
  if (vars->get_value(name,value)) return value;
  
  return 0;
}

/**     
 * \brief �ű��ӿ�,ȡ��һ������ֵ
 *
 * \param vars : Ŀ�����
 * \param name : ��������
 * \return: ȡ�õı���ֵ,�����������ڷ���0
 */ 
const char *  get_varS(Vars* vars,const char* name)
{
  std::string value;
  if (vars->get_value(name,value)) return value.c_str();
  
  return 0;
}

/**     
 * \brief �ű��ӿ�,ˢ��npc״̬
 *
 * \param npc : Ŀ��npc
 */ 
void refresh_status(SceneNpc* npc)
{
  npc->set_quest_status(current_user);

  Cmd::stAddMapNpcMapScreenUserCmd cmd;
  npc->full_t_MapNpcData(cmd.data);
  current_user->sendCmdToMe(&cmd,sizeof(Cmd::stAddMapNpcMapScreenUserCmd));
}

/**     
 * \brief �ű��ӿ�,ˢ��npc״̬
 *
 * \param npc : Ŀ��npc��tempid
 */ 
void refresh_npc(int id)
{
  SceneNpc *sceneNpc = SceneNpcManager::getMe().getNpcByTempID((DWORD)id);  
  if (sceneNpc) refresh_status(sceneNpc);

}

/**     
 * \brief �ű��ӿ�,ȡ��ĳnpc��tempid
 *
 * \param npc : Ŀ��npc
 *\return : ��npc��tempid
 */ 
int npc_tempid(SceneNpc* npc)
{
  return npc->tempid;
}

/**     
 * \brief �ű��ӿ�,ȡ��ĳnpc��id
 *
 * \param npc : Ŀ��npc
 *\return : ��npc��id
 */ 
int npc_id(SceneNpc* npc)
{
  return npc->id;
}

/**     
 * \brief �ű��ӿ�,ˢ��������Ϣ
 *
 * \param npc : Ŀ������id
 */ 
void refresh_quest(int id)
{
  Cmd::stAbandonQuestUserCmd ret;
  ret.id = id;
  current_user->sendCmdToMe(&ret,sizeof(ret));
}

#define U_ID 98765

/**     
 * \brief �ű��ӿ�,ȡ��ȫ�ֱ���,������������,���½�
 *
 *\return : �õ���ȫ�ֱ���
 */ 
Vars* GlobalVars::add_g()
{
  return GlobalVar::instance().add(U_ID);
}

/**     
 * \brief �ű��ӿ�,ȡ�ð�����,������������,���½�
 *
 *\return : �õ��İ�����
 */ 
Vars* GlobalVars::add_t()
{
  return TongVar::instance().add(current_user->charbase.unionid);
}

/**     
 * \brief �ű��ӿ�,ȡ�ü������,������������,���½�
 *
 *\return : �õ��ļ������
 */ 
Vars* GlobalVars::add_f()
{
  return FamilyVar::instance().add(current_user->charbase.septid);
}

/**     
 * \brief �ű��ӿ�,�ж��û����ϴ��еĽ�Ǯ�Ƿ�����Ҫ��
 *
 * \param user: Ŀ���û�
 * \param money: ��Ҫ�Ľ�Ǯ
 * \return : ������������ture,���򷵻�false
 */ 
bool check_money(SceneUser* user,int money)
{
  return user->packs.checkMoney(money);
}

/**     
 * \brief �ű��ӿ�,�����û����ϴ��еĽ�Ǯ����
 *
 * \param user: Ŀ���û�
 * \param money: ��Ҫ�Ľ�Ǯ
 * \return : ���ٳɹ�����ture,���򷵻�false
 */ 
bool remove_money(SceneUser* user,int money)
{
  return user->packs.removeMoney(money,"����");
}

/**     
 * \brief �ű��ӿ�,�����û����ϴ��еĽ�Ǯ����
 *
 * \param user: Ŀ���û�
 * \param money: ���ӵĽ�Ǯ
 */ 
void add_money(SceneUser* user,int money)
{
  user->packs.addMoney(money,"����");
}

/**     
 * \brief �ű��ӿ�,�ж��û������Ƿ����ĳ��Ʒ
 *
 * \param user: Ŀ���û�
 * \param id: ��Ʒ��objectid
 * \param number: Ҫ�������
 * \param level: ��Ʒ�ȼ�������
 * \return ��������Ʒ�������ڵ��ڸ�������������true,���򷵻�false
 */ 
bool have_ob(SceneUser* user,int id,int num,int level,int type)
{
  return user->packs.uom.exist(id,num,level,type);
}

/**     
 * \brief �ű��ӿ�,�ж��û������Ƿ����ĳ��Ʒ
 *
 * \param user: Ŀ���û�
 * \param id: ��Ʒ��objectid
 * \param level: ��Ʒ����
 * \return ����������Ʒ������Ʒthisid,���򷵻�0
 */ 
DWORD get_ob(SceneUser* user,int id,int level )
{
  return user->packs.uom.exist(id,1,level,1);
}

/**     
 * \brief �ű��ӿ�,ɾ����Ʒ
 *
 * \param user: Ŀ���û�
 * \param id: ��Ʒ��thisid
 * \return ɾ���Ƿ�ɹ�
 */ 
bool del_ob(SceneUser* user,DWORD id)
{
  zObject *delObj=user->packs.uom.getObjectByThisID(id);
  return user->packs.removeObject(delObj,true,true);
}

/**     
 * \brief �ű��ӿ�,ȡ���û������е�ʣ��ռ�
 *
 * \param user: Ŀ���û�
 * \return �û�������ӵ�е�ʣ��ռ�
 */ 
int  space(SceneUser* user)
{
  return user->packs.uom.space(user);
}

/**
 * \brief �ű��ӿ�,ȡ��ָ���û��������,������������,���½�
 *
 *\return : �õ����û��������
 */
Vars * get_familyvar(SceneUser* user,int dummy)
{
  return FamilyVar::instance().add(user->charbase.septid);
}

/**
 * \brief �ű��ӿ�,ȡ��ָ���û�����,������������,���½�
 *
 *\return : �õ����û�����
 */
Vars * get_uservar(SceneUser* user,int dummy)
{
  return UsersVar::instance().add(((QWORD)user->charbase.accid << 32) |user->charbase.id);
}

/**
 * \brief �ű��ӿ�,ȡ��ָ���û��л����,������������,���½�
 *
 *\return : �õ����û��л����
 */
Vars * get_tongvar(SceneUser* user,int dummy)
{
  return TongVar::instance().add(user->charbase.unionid);
}

void add_exp(SceneUser* user,DWORD num,bool addPet,DWORD dwTempID,BYTE byType,bool addCartoon)
{
  if (user)
  {
    user->addExp(num,addPet,dwTempID,byType,addCartoon);
    Zebra::logger->debug("[�õ���ʽ������][%s(%u)][%ld]",user->charbase.name,user->id,num);
  }
}

/**     
 * \brief �ű��ӿ�,ȡ��ϵͳ��ǰʱ��
 *
 * \return ϵͳ��ǰʱ��
 */ 
int get_time()
{
  tzset();
  return time(NULL)-_timezone;
}

/**     
 * \brief �ű��ӿ�,ȡ������ʱ��Ĳ�
 *
 * \param t1 : ���Ƚ�ʱ��1
 * \param t2 : ���Ƚ�ʱ��2
 * \return ����ʱ��Ĳ�
 */ 
double diff_time(int t1,int t2)
{
  return difftime(t1,t2);
}


//��ת����ͼ
bool gomap(SceneUser *pUser,const char *para)
{
	return Gm::gomap_Gm(pUser,para); 
}


//��ת������
bool goTo(SceneUser *pUser,const char *para)
{
	return Gm::goTo_Gm(pUser,para);
	//return false;
}


bool addExp(SceneUser *pUser,const char *para)
{
	return Gm::addExp(pUser,para);
}


void deleteChar(const char *p)
{
	delete[] p;
	p = NULL;
}

char * getMeterials(DWORD id)
{
	//zObject* o = goi->getObjectById(id);

	return meterialsManager::getInstance().get(id);

}

DWORD getIdByThisId(DWORD thisid)
{
	zObject* o = goi->getObjectByThisid(thisid);
	if(o)
	{
		return o->id;
		//EquipMaker maker(NULL);
		//maker.assign_set(o);
	}

	return 0;
}



