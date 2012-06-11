/**
 * \brief  脚本辅助函数
 * 
 */

#include <zebra/ScenesServer.h>
#include <zebra/csCommon.h>
#include "meterialsManager.h"

SceneUser* current_user = NULL;


/**     
 * \brief 取得当前用户
 *
 * \return: 当前用户
 */  
SceneUser* me()
{
  return current_user;
}

/**     
 * \brief 设定当前用户
 *
 * \param user: 目标用户
 */  
void set_me(SceneUser* user)
{
  current_user = user;
}


/**     
 * \brief 脚本接口,传送信息到用户窗口
 *
 * \param target : 目标用户
 * \param type : 讯息类型,see enumSysInfoType from Command.h for details
 * \param msg: 信息内容
 * \return: 传送成功返回true,否则返回false
 */  
bool sys(SceneUser* target,int type,const char* msg)
{
  return Channel::sendSys(target,type,msg);
}


//全世界发消息
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

//装备打造
void equip_make(SceneUser* user,DWORD thisID,bool drop,int flag)
{
	EquipMaker maker(NULL);
    zObject* o = goi->getObjectByThisid(thisID);
	if(o)
		maker.assign(user,o,o->base,1);
}

//创建物品
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
      memcpy(&ret.object,&(o->data),sizeof(t_Object),sizeof(ret.object));
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
 * \brief 脚本接口,弹出对话框
 *
 * \param npc : 对话的npc
 * \param menu : 对话框内容
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
 * \brief 脚本接口,设定一个变量值
 *
 * \param vars : 目标变量
 * \param name : 变量名称
 * \param value: 变量值
 */ 
void set_var(Vars* vars,const char* name,int value)
{
  Op::Set<int> op;
  vars->set_value(op,name,value,0,current_user);
  
}

/**     
 * \brief 脚本接口,设定一个变量值
 *
 * \param vars : 目标变量
 * \param name : 变量名称
 * \param value: 变量值
 */ 
void set_varS(Vars* vars,const char* name,const char * value)
{
  Op::Set< std::string> op;
  std::string s=value;
  vars->set_value(op,name,s,0,current_user);
}

/**     
 * \brief 脚本接口,取得一个变量值
 *
 * \param vars : 目标变量
 * \param name : 变量名称
 * \return: 取得的变量值,若变量不存在返回0
 */ 
int get_var(Vars* vars,const char* name)
{
  int value;
  if (vars->get_value(name,value)) return value;
  
  return 0;
}

/**     
 * \brief 脚本接口,取得一个变量值
 *
 * \param vars : 目标变量
 * \param name : 变量名称
 * \return: 取得的变量值,若变量不存在返回0
 */ 
const char *  get_varS(Vars* vars,const char* name)
{
  std::string value;
  if (vars->get_value(name,value)) return value.c_str();
  
  return 0;
}

/**     
 * \brief 脚本接口,刷新npc状态
 *
 * \param npc : 目标npc
 */ 
void refresh_status(SceneNpc* npc)
{
  npc->set_quest_status(current_user);

  Cmd::stAddMapNpcMapScreenUserCmd cmd;
  npc->full_t_MapNpcData(cmd.data);
  current_user->sendCmdToMe(&cmd,sizeof(Cmd::stAddMapNpcMapScreenUserCmd));
}

/**     
 * \brief 脚本接口,刷新npc状态
 *
 * \param npc : 目标npc的tempid
 */ 
void refresh_npc(int id)
{
  SceneNpc *sceneNpc = SceneNpcManager::getMe().getNpcByTempID((DWORD)id);  
  if (sceneNpc) refresh_status(sceneNpc);

}

/**     
 * \brief 脚本接口,取得某npc的tempid
 *
 * \param npc : 目标npc
 *\return : 该npc的tempid
 */ 
int npc_tempid(SceneNpc* npc)
{
  return npc->tempid;
}

/**     
 * \brief 脚本接口,取得某npc的id
 *
 * \param npc : 目标npc
 *\return : 该npc的id
 */ 
int npc_id(SceneNpc* npc)
{
  return npc->id;
}

/**     
 * \brief 脚本接口,刷新任务信息
 *
 * \param npc : 目标任务id
 */ 
void refresh_quest(int id)
{
  Cmd::stAbandonQuestUserCmd ret;
  ret.id = id;
  current_user->sendCmdToMe(&ret,sizeof(ret));
}

#define U_ID 98765

/**     
 * \brief 脚本接口,取得全局变量,若变量不存在,则新建
 *
 *\return : 得到的全局变量
 */ 
Vars* GlobalVars::add_g()
{
  return GlobalVar::instance().add(U_ID);
}

/**     
 * \brief 脚本接口,取得帮会变量,若变量不存在,则新建
 *
 *\return : 得到的帮会变量
 */ 
Vars* GlobalVars::add_t()
{
  return TongVar::instance().add(current_user->charbase.unionid);
}

/**     
 * \brief 脚本接口,取得家族变量,若变量不存在,则新建
 *
 *\return : 得到的家族变量
 */ 
Vars* GlobalVars::add_f()
{
  return FamilyVar::instance().add(current_user->charbase.septid);
}

/**     
 * \brief 脚本接口,判断用户身上带有的金钱是否满足要求
 *
 * \param user: 目标用户
 * \param money: 需要的金钱
 * \return : 满足条件返回ture,否则返回false
 */ 
bool check_money(SceneUser* user,int money)
{
  return user->packs.checkMoney(money);
}

/**     
 * \brief 脚本接口,较少用户身上带有的金钱数量
 *
 * \param user: 目标用户
 * \param money: 需要的金钱
 * \return : 较少成功返回ture,否则返回false
 */ 
bool remove_money(SceneUser* user,int money)
{
  return user->packs.removeMoney(money,"任务");
}

/**     
 * \brief 脚本接口,增加用户身上带有的金钱数量
 *
 * \param user: 目标用户
 * \param money: 增加的金钱
 */ 
void add_money(SceneUser* user,int money)
{
  user->packs.addMoney(money,"任务");
}

/**     
 * \brief 脚本接口,判断用户身上是否带有某物品
 *
 * \param user: 目标用户
 * \param id: 物品的objectid
 * \param number: 要求的数量
 * \param level: 物品等级或类型
 * \return 包裹中物品数量大于等于给定的数量返回true,否则返回false
 */ 
bool have_ob(SceneUser* user,int id,int num,int level,int type)
{
  return user->packs.uom.exist(id,num,level,type);
}

/**     
 * \brief 脚本接口,判断用户身上是否带有某物品
 *
 * \param user: 目标用户
 * \param id: 物品的objectid
 * \param level: 物品类型
 * \return 包裹中有物品返回物品thisid,否则返回0
 */ 
DWORD get_ob(SceneUser* user,int id,int level )
{
  return user->packs.uom.exist(id,1,level,1);
}

/**     
 * \brief 脚本接口,删除物品
 *
 * \param user: 目标用户
 * \param id: 物品的thisid
 * \return 删除是否成功
 */ 
bool del_ob(SceneUser* user,DWORD id)
{
  zObject *delObj=user->packs.uom.getObjectByThisID(id);
  return user->packs.removeObject(delObj,true,true);
}

/**     
 * \brief 脚本接口,取得用户包裹中的剩余空间
 *
 * \param user: 目标用户
 * \return 用户包裹所拥有的剩余空间
 */ 
int  space(SceneUser* user)
{
  return user->packs.uom.space(user);
}

/**
 * \brief 脚本接口,取得指定用户家族变量,若变量不存在,则新建
 *
 *\return : 得到的用户家族变量
 */
Vars * get_familyvar(SceneUser* user,int dummy)
{
  return FamilyVar::instance().add(user->charbase.septid);
}

/**
 * \brief 脚本接口,取得指定用户变量,若变量不存在,则新建
 *
 *\return : 得到的用户变量
 */
Vars * get_uservar(SceneUser* user,int dummy)
{
  return UsersVar::instance().add(((QWORD)user->charbase.accid << 32) |user->charbase.id);
}

/**
 * \brief 脚本接口,取得指定用户行会变量,若变量不存在,则新建
 *
 *\return : 得到的用户行会变量
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
    Zebra::logger->debug("[得到环式任务经验][%s(%u)][%ld]",user->charbase.name,user->id,num);
  }
}

/**     
 * \brief 脚本接口,取得系统当前时间
 *
 * \return 系统当前时间
 */ 
int get_time()
{
  tzset();
  return time(NULL)-_timezone;
}

/**     
 * \brief 脚本接口,取得两个时间的差
 *
 * \param t1 : 待比较时间1
 * \param t2 : 待比较时间2
 * \return 两个时间的差
 */ 
double diff_time(int t1,int t2)
{
  return difftime(t1,t2);
}


//跳转到地图
bool gomap(SceneUser *pUser,const char *para)
{
	return Gm::gomap_Gm(pUser,para); 
}


//跳转到坐标
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



