/**
 * \brief  ����ϵͳ
 * 
 */

#include <zebra/ScenesServer.h>

Mask::Mask() : _drop(false),_mask(NULL),_user(NULL)
{

}

Mask::~Mask()
{

}

bool Mask::is_use(zObject* ob) const
{
  return _mask == ob;
}
/**
 * \brief ʹ�������
 *
 *
 * \param user: ʹ���������û�
 * \param ob: �������Ʒ
 * \return ��Ʒ���Ͳ���ȷ����1,���������ȴ�ڷ���-1,�ɹ�����0
 */
int Mask::on_use(SceneUser* user,zObject* ob)
{
  if (!ob || ob->base->kind!=ItemType_MASK) {
    return 1;
  }

  if (_drop && _last > SceneTimeTick::currentTime) {
    Channel::sendSys(user,Cmd::INFO_TYPE_FAIL,"��ʱ����ʹ��!");
    return -1;
  }

  _drop = false;
  bool notify=true;

  if (_mask && _mask->data.qwThisID == ob->data.qwThisID) {
    _mask = NULL;
    _user = NULL;
    user->sendMeToNine();
    user->sendPetDataToNine();
    Channel::sendSys(user,Cmd::INFO_TYPE_GAME,"���%s!",ob->data.strName);
  }else {
    if (_mask)
    {
      notify=false;
    }
    _mask = ob;
    _mask->data.dur -= TIME_CONSUME_PERMANENCE;
    if ((short)_mask->data.dur < 0)
    {
      _mask->data.dur = 0;
    }
    Channel::sendSys(user,Cmd::INFO_TYPE_GAME,"ʹ��%s!",ob->data.strName);  
    if (user->tradeorder.hasBegin() ) {
      user->tradeorder.cancel();
    }
    if ((short)_mask->data.dur > 0 && notify)
    {
      user->sendMeToNine();
      user->sendPetDataToNine();
    }

    _user = user;
    reduce_permanence();
  }
  
  return 0;
}

/**
 * \brief �ж��Ƿ�������״̬
 *
 *
 * \return ���淵��true,���򷵻�false
 */
bool Mask::is_masking() const
{
  return (_mask)?true:false;  
}

/**
 * \brief ����ʱ���н�����ȡ������״̬
 *
 *
 * \return 0
 */
int Mask::on_trade()
{
  _mask = NULL;
  if (_user) _user->sendMeToNine();
  _user = NULL;

  return 0;
}

/**
 * \brief ����ʱ��û���״̬��ȡ������״̬
 *
 *
 * \return 0
 */
int Mask::on_gem()
{
  _mask = NULL;
  if (_user) _user->sendMeToNine();
  _user = NULL;

  return 0;
}

/**
 * \brief ��ʱ�������;�
 *
 *
 * \return ��������״̬����0,���򷵻�-1
 */
int Mask::on_timer()
{
  if (_mask && _user) {
    _mask->data.dur -= TIME_CONSUME_PERMANENCE;
    if ((short)_mask->data.dur < 0)
    {
      _mask->data.dur = 0;
    }
      
    
    return reduce_permanence();    
  }
  
  return -1;
}

/**
 * \brief ����״̬ʱ������,���������������
 *
 *
 * \return ��������״̬����0,���򷵻�-1
 */
int Mask::on_defence()
{
  if (_mask && _user) {
    _mask->data.dur -= ATTACKED_CONSUME_PERMANENCE;
    
    reduce_permanence();
  
    if (!_mask) return 0;  

    return 0; //Ŀǰ���������,����������ֱ�ӷ����ˡ�

    int drop = drop_odds();
    Zebra::logger->debug("������������:%d",drop);
    if (selectByTenTh(drop)) {
      Channel::sendSys(_user,Cmd::INFO_TYPE_GAME,"���%s�������!",_mask->data.strName);
      _drop = true;
      _last = SceneTimeTick::currentTime;
      _last.addDelay(ATTACK_USE_TIME);
      _mask = NULL;
      //refresh right now
      _user->sendMeToNine();
      _user = NULL;
    }
    
    return 0;
    
  }
  
  return -1;  
}


/**
 * \brief ��������״̬ʱ�������˷���ϵͳ��ʾ
 *
 *
 * \param victim: �������û�
 * \return 0
 */
int Mask::on_attack(SceneUser* victim)
{
  if (_mask && _user) {
    Channel::sendSys(victim,Cmd::INFO_TYPE_GAME,"�����ܵ������˵Ķ��⹥��,����Զ�����������!");
  }
  
  return 0;
}

/**
 * \brief ���������־�
 *
 *
 * \return 0
 */
int Mask::reduce_permanence()
{
  if ((SWORD)_mask->data.dur > 0) {
    Cmd::stDurabilityUserCmd ret;
    ret.dwThisID = _mask->data.qwThisID;
    ret.dwDur = _mask->data.dur;
    ret.dwMaxDur = _mask->data.maxdur;
    _user->sendCmdToMe(&ret,sizeof(ret));  
  }else {
    _user->packs.removeObject(_mask); //notify and delete
    _mask = NULL;    
//refresh right now
    _user->sendMeToNine();    
    //_user->sendPetDataToNine();
    _user = NULL;
  }
  
  return 0;
}

/**
 * \brief ����ʱ�����˹��������;�
 *
 *
 * \return ���ĵ��;�ֵ
 */
int Mask::drop_odds() const
{
  return 100*DROP_ODDS/_mask->data.maxdur;
}
