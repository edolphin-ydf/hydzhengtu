/**
 * \brief   马匹相关
 *
 * 
 */

#include <zebra/ScenesServer.h>

/**     
 * \brief 构造函数
 *
 * 初始化相关变量
 *      
 * \param user: 马匹拥有者
 */  
Horse::Horse(SceneUser& user) : _owner(user),_horse(0),_mount(false)
{
  bzero(&pkData,sizeof(pkData));
  bzero(&data,sizeof(data));
}

/**     
 * \brief 析构函数
 *
 */  
Horse::~Horse()
{

}

/**     
 * \brief 设置马匹的种类
 *
 * \param horse_id: 马匹种类
 * \return 无
 */  
void Horse::horse(DWORD horse_id)
{
  fprintf(stderr,"\n用户上马 %u\n",horse_id);
  using namespace Cmd;

  /*if (horseType(horse_id)==HORSE_TYPE_NOTHORSE)
    horse_id = 0;*/

  _horse = horse_id;

  if (_horse)
  {
    data.id = horse_id;
    zNpcB *b = npcbm.get(data.id);
    if (!b)
    {
      _horse = 0;
      data.id = 0;
      return;
    }

    if (_owner.ridepet)
      _owner.killOnePet(_owner.ridepet);

    data.horseid = horse_id;
    if (b)
    {
      data.lv = b->level;
      interval = b->adistance;
      data.state = Cmd::HORSE_STATE_PUTUP;
      data.callTime = 0;
      summonTime = SceneTimeTick::currentTime;
      bcopy(b->name,data.name,MAX_NAMESIZE-1,sizeof(data.name));

      if (HORSE_TYPE_SUPER!=horseType(horse_id))
      {
        bzero(&data.str,sizeof(DWORD)*5);
        bzero(&data.poisonRes,sizeof(DWORD)*4);
      }
      if (_owner.emperor && HORSE_TYPE_SUPER==horseType(horse_id))
        data.speed += 50;

      sendData();
    }
    else
    {
      Zebra::logger->debug("没有找到id=%d的马",horse_id);
    }
  }
  else
  {
    bzero(&data,sizeof(data));
    bzero(&pkData,sizeof(pkData));

    Cmd::stDelPetPetCmd del;                             
    del.type = Cmd::PET_TYPE_RIDE;                  
    _owner.sendCmdToMe(&del,sizeof(del));
    _owner.clearUState(Cmd::USTATE_RIDE);
  }

  if (_owner.scene)
  {
    _owner.sendMeToNine();
  }
}

/**     
 * \brief 设置马匹的种类
 *
 * \param horse_id: 马匹种类
 * \return 无
 */  
bool Horse::horse(t_Object & obj)
{
	return true;
}

/**     
 * \brief 取得马匹的种类
 *
 * \return 马匹种类
 */  
DWORD Horse::horse() const
{
  return _horse;
}

/**     
 * \brief 切换用户的骑马状态
 *
 * \param flag: 用户要求上马还是下马
 * \param send: 是否需要通知客户端,死亡下马不需要通知客户端,要在重生的时候刷新下去
 * \param speed: 提升移动速度倍率(百分比)
 * \return 无
 */ 
void Horse::mount(bool flag, WORD speed, bool send)
{
  //ride down
  if (!flag && _mount) 
  {
    _owner.clearUState(Cmd::USTATE_RIDE);
	_owner.charstate.movespeed = 640; //sky 还原移动速度
    _mount = false;
  }

  //ride up
  if (flag && !_mount) 
  {
    _owner.setUState(Cmd::USTATE_RIDE);
	_owner.charstate.movespeed = (WORD)(_owner.charstate.movespeed/(1+speed/100.0f));
    _mount = true;
  }

  if (send)
  {
    _owner.sendMeToNine();
  }

  Cmd::stMainUserDataUserCmd  userinfo;
  _owner.full_t_MainUserData(userinfo.data);
  _owner.sendCmdToMe(&userinfo,sizeof(userinfo));
}

/**     
 * \brief 返回用户的骑马状态
 *
 * \return 用户是否骑马
 */ 
bool Horse::mount() const
{
  return _mount;
}

/**     
 * \brief 读取用户的马匹信息
 *
 * \param d 二进制档案内容
 * \return 读档的二进制长度
 */ 
int Horse::load(BYTE* d)
{	
	return 0;
}

/**     
 * \brief 存储用户的马匹信息
 *
 * \param d: 二进制档案内容
 * \return 存档的二进制长度
 */ 
int Horse::save(BYTE *d)
{
  return 0;
}

/**
 * \brief 设置下次召唤马匹的延迟
 *
 *
 * \param delay 延迟时间
 */
void Horse::setSummonTime(int delay)
{

}

/**
 * \brief 设置下次召唤马匹的延迟
 *
 * \return 检查是否可以召唤马匹
 */
bool Horse::checkSummonTime()
{
  return true;
}

/**
 * \brief 召唤出来跟随主人
 * \return 是否成功
 */
bool Horse::comeOut()
{
  return false;
}

/**
 * \brief 收起马匹
 *
 * \return 是否成功，没有马匹返回失败
 */
bool Horse::putAway()
{
	if (_owner.ridepet)
	{
		_owner.killOnePet(_owner.ridepet);
	}

	return false;
}

/**
 * \brief 检查马是否在跟随状态
 *
 * \return 是否在跟随
 */
bool Horse::isFollowing()
{
  return Cmd::HORSE_STATE_FOLLOW==data.state;
}

/**
 * \brief 填充马的信息
 *
 *
 * \param d 信息结构地址
 */
bool Horse::full_HorseDataStruct(Cmd::t_HorseData *d)
{
  if (!_horse) return false;

  /*
  if (summonTime>SceneTimeTick::currentTime)
  {
    data.callTime = summonTime.sec()-SceneTimeTick::currentTime.sec();
  }
  else
  */
   data.callTime = 0;

  bcopy(&data,d,sizeof(data),sizeof(Cmd::t_HorseData));

  if (_owner.king)
  {
      d->id = KING_HORSE_ID;
      d->horseid = KING_HORSE_ID;
  }

  if (_owner.emperor)
  {
      d->id = EMPEROR_HORSE_ID;
      d->horseid = EMPEROR_HORSE_ID;
  }

  return true;
}

/**
 * \brief 向客户端发送数据
 *
 */
void Horse::sendData()
{
  if (!_horse) return;
  Cmd::stHorseDataPetCmd ret;
  ret.type = Cmd::PET_TYPE_RIDE;
  full_HorseDataStruct(&ret.data);
  ret.id = ret.data.id;
  _owner.sendCmdToMe(&ret,sizeof(ret));
  Zebra::logger->debug("发送马匹信息 name=%s id=%u horseid=%u state=%u time=%u",data.name,data.id,data.horseid,data.state,data.callTime);
}

/**
 * \brief 判断该马是否可以战斗
 *
 * \return 是否可以战斗
 */
bool Horse::canFight()
{
	//sky 现在的马都不可以战斗
    return false;
}

void Horse::getPkData()
{
    if (_horse && HORSE_TYPE_SUPER==horseType() && mount())
	{
        bcopy(&data,&pkData,sizeof(pkData),sizeof(pkData));

        pkData.pdam = data.pdam;
		pkData.pdef = data.pdef;
		pkData.mdam = data.mdam;
		pkData.mdef = data.mdef;
		pkData.maxhp = data.maxhp; 
		pkData.maxmp = data.maxmp;
		pkData.speed = data.speed;
		pkData.poisonRes = data.poisonRes; //抗毒
		pkData.lullRes = data.lullRes;     //抗麻痹
		pkData.faintRes = data.faintRes;   //抗晕眩
		pkData.chaosRes = data.chaosRes;   //抗混乱
		pkData.freezeRes = data.freezeRes; //抗冰冻
		pkData.petrifyRes = data.petrifyRes; //抗石化
		pkData.blindRes = data.blindRes;    //抗失明
		pkData.slowRes = data.slowRes;      //抗减速
	}
    else
        bzero(&pkData,sizeof(pkData));
}

DWORD Horse::horseType()
{
  return horseType(_horse);
}

DWORD Horse::horseType(DWORD type)
{
    if (3000==type) return HORSE_TYPE_NORMAL;
    if (3200==type) return HORSE_TYPE_BATTLE;
    if (type>=3201 && type<=3500) return HORSE_TYPE_SUPER;
    return HORSE_TYPE_NOTHORSE;
}
