/**
 * \brief   ��ƥ���
 *
 * 
 */

#include <zebra/ScenesServer.h>

/**     
 * \brief ���캯��
 *
 * ��ʼ����ر���
 *      
 * \param user: ��ƥӵ����
 */  
Horse::Horse(SceneUser& user) : _owner(user),_horse(0),_mount(false)
{
  bzero(&pkData,sizeof(pkData));
  bzero(&data,sizeof(data));
}

/**     
 * \brief ��������
 *
 */  
Horse::~Horse()
{

}

/**     
 * \brief ������ƥ������
 *
 * \param horse_id: ��ƥ����
 * \return ��
 */  
void Horse::horse(DWORD horse_id)
{
  fprintf(stderr,"\n�û����� %u\n",horse_id);
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
      Zebra::logger->debug("û���ҵ�id=%d����",horse_id);
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
 * \brief ������ƥ������
 *
 * \param horse_id: ��ƥ����
 * \return ��
 */  
bool Horse::horse(t_Object & obj)
{
	return true;
}

/**     
 * \brief ȡ����ƥ������
 *
 * \return ��ƥ����
 */  
DWORD Horse::horse() const
{
  return _horse;
}

/**     
 * \brief �л��û�������״̬
 *
 * \param flag: �û�Ҫ������������
 * \param send: �Ƿ���Ҫ֪ͨ�ͻ���,����������Ҫ֪ͨ�ͻ���,Ҫ��������ʱ��ˢ����ȥ
 * \param speed: �����ƶ��ٶȱ���(�ٷֱ�)
 * \return ��
 */ 
void Horse::mount(bool flag, WORD speed, bool send)
{
  //ride down
  if (!flag && _mount) 
  {
    _owner.clearUState(Cmd::USTATE_RIDE);
	_owner.charstate.movespeed = 640; //sky ��ԭ�ƶ��ٶ�
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
 * \brief �����û�������״̬
 *
 * \return �û��Ƿ�����
 */ 
bool Horse::mount() const
{
  return _mount;
}

/**     
 * \brief ��ȡ�û�����ƥ��Ϣ
 *
 * \param d �����Ƶ�������
 * \return �����Ķ����Ƴ���
 */ 
int Horse::load(BYTE* d)
{	
	return 0;
}

/**     
 * \brief �洢�û�����ƥ��Ϣ
 *
 * \param d: �����Ƶ�������
 * \return �浵�Ķ����Ƴ���
 */ 
int Horse::save(BYTE *d)
{
  return 0;
}

/**
 * \brief �����´��ٻ���ƥ���ӳ�
 *
 *
 * \param delay �ӳ�ʱ��
 */
void Horse::setSummonTime(int delay)
{

}

/**
 * \brief �����´��ٻ���ƥ���ӳ�
 *
 * \return ����Ƿ�����ٻ���ƥ
 */
bool Horse::checkSummonTime()
{
  return true;
}

/**
 * \brief �ٻ�������������
 * \return �Ƿ�ɹ�
 */
bool Horse::comeOut()
{
  return false;
}

/**
 * \brief ������ƥ
 *
 * \return �Ƿ�ɹ���û����ƥ����ʧ��
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
 * \brief ������Ƿ��ڸ���״̬
 *
 * \return �Ƿ��ڸ���
 */
bool Horse::isFollowing()
{
  return Cmd::HORSE_STATE_FOLLOW==data.state;
}

/**
 * \brief ��������Ϣ
 *
 *
 * \param d ��Ϣ�ṹ��ַ
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
 * \brief ��ͻ��˷�������
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
  Zebra::logger->debug("������ƥ��Ϣ name=%s id=%u horseid=%u state=%u time=%u",data.name,data.id,data.horseid,data.state,data.callTime);
}

/**
 * \brief �жϸ����Ƿ����ս��
 *
 * \return �Ƿ����ս��
 */
bool Horse::canFight()
{
	//sky ���ڵ���������ս��
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
		pkData.poisonRes = data.poisonRes; //����
		pkData.lullRes = data.lullRes;     //�����
		pkData.faintRes = data.faintRes;   //����ѣ
		pkData.chaosRes = data.chaosRes;   //������
		pkData.freezeRes = data.freezeRes; //������
		pkData.petrifyRes = data.petrifyRes; //��ʯ��
		pkData.blindRes = data.blindRes;    //��ʧ��
		pkData.slowRes = data.slowRes;      //������
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
