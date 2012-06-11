#include <zebra/ScenesServer.h>

/*
 * \brief ����
 *
 */
CartoonPet::CartoonPet(Scene *scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype,zNpcB *abase) : ScenePet(scene,npc,define,type,entrytype,abase),_5_sec(5)
{
  //needSave = false;

  bzero(&cartoonData,sizeof(cartoonData));
  cartoonData.npcID = npc->id;
  cartoonID = 0;
  expRate = 3;

  _5_sec_count = 0;

  setPetAI((Cmd::petAIMode)(Cmd::PETAI_MOVE_FOLLOW|Cmd::PETAI_ATK_NOATK));
}

/*
 * \brief ���ñ�������
 * 
 * \param master ����
 */
void CartoonPet::setMaster(SceneEntryPk * master)
{
  if (!master) return;
  ScenePet::setMaster(master);
  cartoonData.masterID = master->id;
}

/*
 * \brief ���ñ�������
 * 
 * \param id ����ID
 * \param type ��������
 */
void CartoonPet::setMaster(DWORD id,DWORD type)
{
  ScenePet::setMaster(id,type);

  cartoonData.masterID = id;
}

/*
 * \brief ���ñ���ID
 * 
 * \param id ID
 */
void CartoonPet::setCartoonID(DWORD id)
{
  cartoonID = id;
}

/*
 * \brief �����ͷž��������
 * 
 * \param rate ����,һ���ͷ��������ȼ�*����
 */
void CartoonPet::setExpRate(BYTE rate)
{
  expRate = rate;
  if (3==rate)
    Channel::sendNine(this,"HOHO~�ҵ�����������,��Ҫ����,���1.5���ľ���~");
  else if (2==rate)
    Channel::sendNine(this,"OH~����������,��û��~");
#ifdef _DEBUG
  //Zebra::logger->debug("[����]%s ���þ��鱶�� %u",name,expRate);
#endif
}

/*
 * \brief ����,����ʱҪ����
 * 
 * \param saveType ��������
 */
void CartoonPet::putAway(Cmd::Session::saveType saveType)
{
  SceneUser * master = (SceneUser *)getMaster();
  if (!master) return;

  cartoonData.repair = 0;
  if (saveType!=Cmd::Session::SAVE_TYPE_DONTSAVE)
    save(saveType);
  master->killOnePet(this);
  setClearState();
}

/*
 * \brief �Ǳ������Ļ����Լ���
 * 
 * \return �Ǳ������Ļ����Լ���
 */
bool CartoonPet::isAdopted()
{
  return cartoonData.state == Cmd::CARTOON_STATE_ADOPTED;
}

/*
 * \brief �������ݵ����ݿ�
 * \param type ��������
 * 
 */
void CartoonPet::save(Cmd::Session::saveType type)
{
  SceneUser * master = (SceneUser *)getMaster();
  if (!master) return;

  Cmd::Session::t_saveCartoon_SceneSession send;
  strncpy(send.userName,master->name,MAX_NAMESIZE);
  send.type = type;
  send.cartoonID = cartoonID;
  send.data = cartoonData;
  switch (type)
  {
    case Cmd::Session::SAVE_TYPE_RETURN:
      {
        send.data.state = Cmd::CARTOON_STATE_WAITING;
        bzero(send.data.adopter,MAX_NAMESIZE);
      }
      break;
    case Cmd::Session::SAVE_TYPE_TIMEOVER:
    case Cmd::Session::SAVE_TYPE_PUTAWAY:
      send.data.state = Cmd::CARTOON_STATE_PUTAWAY;
      break;
    default:
      break;
  }

  if (!isAdopted())
    master->cartoonList[cartoonID] = cartoonData;
  sessionClient->sendCmd(&send,sizeof(send));

  switch (type)
  {
    case Cmd::Session::SAVE_TYPE_RETURN:
    case Cmd::Session::SAVE_TYPE_TIMEOVER:
    case Cmd::Session::SAVE_TYPE_SYN:
      cartoonData.addExp = 0;//�������
      break;
    default:
      break;
  }
}

/*
 * \brief ��ȡ���鵽��������
 * 
 */
void CartoonPet::drawExp()
{
  SceneUser * master = (SceneUser *)getMaster();
  if (!master) return;

  master->addExp(cartoonData.addExp,true);
  cartoonData.addExp = 0;
}

/*
 * \brief �ж�,��ս��,ֻ����
 * ÿ15���ͷ�һ�ξ���
 * 
 */
bool CartoonPet::normalAction()
{
  if (_5_sec(SceneTimeTick::currentTime))
  {
    _5_sec_count++;

    if (isAdopted())
    {
      if (cartoonData.time<=5)
      {
        cartoonData.time = 0;
        bzero(cartoonData.adopter,MAX_NAMESIZE);
        putAway(Cmd::Session::SAVE_TYPE_TIMEOVER);
        return true;
      }
      cartoonData.time -= 5;

      if (0==_5_sec_count%3)//��������,ÿ15���ͷž���
        releaseExp();

      if (0==_5_sec_count%60)//5���Ӵ浵
      {
        save(Cmd::Session::SAVE_TYPE_SYN);
      }
    }
    /* //�����㾫��ֵ��
    else if (0==_5_sec_count%12)//������,ÿ1���Ӽ�sp1
    {
      if (cartoonData.sp)
      {
        cartoonData.sp--;
        if (0==cartoonData.sp)
        {
          putAway(Cmd::Session::SAVE_TYPE_PUTAWAY);
          return true;
        }
        else
        {
          SceneUser * master = (SceneUser *)getMaster();
          master->cartoonList[cartoonID] = cartoonData;
          sendHpExp();
        }
      }
    }
    */

  }
  return SceneNpc::normalAction();
}

/*
 * \brief �ͷž���
 * 
 */
void CartoonPet::releaseExp()
{
  //�̶�����=0.15*(2*���˽�ɫ��ǰ�ȼ�^2)/4 +1  ȡ��
  if (0==cartoonData.time) return;
  DWORD n = (2*cartoonData.masterLevel*cartoonData.masterLevel)*15/100/4+1;
  cartoonData.addExp += n;

#ifdef _DEBUG
  //Zebra::logger->debug("[����]%s �ͷž��� %u,���� %u",name,n,cartoonData.addExp);
#endif
}

/*
 * \brief ���ͱ�������
 * 
 */
void CartoonPet::sendData()
{
  if (!cartoonID) return;
  SceneUser * master = (SceneUser *)getMaster();
  if (!master) return;

  if (!isAdopted())
  {
    Cmd::stAddCartoonCmd send;
    send.isMine = true;
    send.cartoonID = cartoonID;
    send.data = cartoonData;
    master->sendCmdToMe(&send,sizeof(send));
  }
  else
  {
    Cmd::stAddWaitingCartoonCmd send;
    send.cartoonID = cartoonID;
    send = cartoonData;
    master->sendCmdToMe(&send,sizeof(send));
  }
}

/*
 * \brief ���ͱ�������
 * 
 */
void CartoonPet::sendHpExp()
{
  if (!cartoonID) return;
  SceneUser * master = (SceneUser *)getMaster();
  if (!master) return;

  if (!isAdopted())
  {
    Cmd::stHpExpCartoonCmd send;
    send.cartoonID = cartoonID;
    send.sp = cartoonData.sp;
    send.exp = cartoonData.exp;
    master->sendCmdToMe(&send,sizeof(send));
  }
  else
  {
  }
}

/*
 * \brief �����Ӿ���
 * \param num ����
 * 
 * \return �Ƿ�����(δ��)
 */
bool CartoonPet::addExp(DWORD num)
{
  if (0==num) num = 1;

  cartoonData.exp += num;
  if (cartoonData.exp>=cartoonData.maxExp)
  {
    levelUp();
    sendData();
  }
  else
    sendHpExp();
  petData.exp = cartoonData.exp;

  return false;
}

/*
 * \brief ��������
 * 
 */
void CartoonPet::levelUp()
{
  SceneEntryPk * master = getMaster();
  if (!master || master->getType()!=zSceneEntry::SceneEntry_Player) return;

  DWORD maxlv = ((SceneUser *)master)->charbase.level;
  bool notify = false;
  while ((cartoonData.exp>=cartoonData.maxExp)&&(cartoonData.lv<maxlv))
  {
    cartoonData.exp -= cartoonData.maxExp;
    cartoonData.maxSp += 10;
    cartoonData.sp += 10;
    cartoonData.lv++; 
    zExperienceB *base_exp = experiencebm.get(cartoonData.lv);
    if (base_exp)
      cartoonData.maxExp = base_exp->nextexp/10;

    notify = true;
  }
  if (cartoonData.exp>cartoonData.maxExp)
    cartoonData.exp = cartoonData.maxExp;
  petData.lv = cartoonData.lv;
  petData.maxexp = cartoonData.maxExp;

  save(Cmd::Session::SAVE_TYPE_TIMETICK);

  sendMeToNine();

  if (notify)
  {
    if (id==9005)
      Channel::sendNine(this,"����~�Ҷ�����%u����,�ø����Ҹ����˰�~~",cartoonData.lv);
    else
      Channel::sendNine(this,"����~������%u������",cartoonData.lv);
  }
}

/*
 * \brief �õ������ĵȼ�
 * 
 * \return �ȼ�
 */
DWORD CartoonPet::getLevel() const
{
  return cartoonData.lv;
}

/*
 * \brief �õ�����������
 * 
 * \return ����
 */
Cmd::t_CartoonData& CartoonPet::getCartoonData()
{
  return cartoonData;
}

/*
 * \brief ���ñ���������
 * 
 * \param data ����
 *
 */
void CartoonPet::setCartoonData(Cmd::t_CartoonData& data)
{
  cartoonData = data;

  strncpy(petData.name,cartoonData.name,MAX_NAMESIZE);
  strncpy(name,cartoonData.name,MAX_NAMESIZE);
  petData.lv = cartoonData.lv;
  petData.exp = cartoonData.exp;
  petData.maxexp = cartoonData.maxExp;
  petData.hp = cartoonData.sp;
  petData.maxhp = cartoonData.maxSp;

  sendData();
  sendMeToNine();
}

/*
 * \brief ���˿��ʱ,�����������ı�������ʱ����
 * 
 * \param dest �����ַ
 *
 * \return ���ݴ�С
 */
DWORD CartoonPet::save(BYTE * dest)
{
  *((DWORD *)dest) = cartoonID;//ID
  bcopy(&cartoonData,dest+sizeof(DWORD),sizeof(Cmd::t_CartoonData),sizeof(Cmd::t_CartoonData));//����
  return sizeof(DWORD)+sizeof(Cmd::t_CartoonData);
}

/*
 * \brief �ָ�SP
 * 
 * \param num ����
 *
 */
void CartoonPet::recoverSp(DWORD num)
{
  if (cartoonData.sp==cartoonData.maxSp) return;
  cartoonData.sp += num;
  if (cartoonData.sp>cartoonData.maxSp)
    cartoonData.sp=cartoonData.maxSp;
  sendHpExp();
}

/*
 * \brief ���ñ���������
 * 
 * \param name ����
 *
 */
void CartoonPet::setName(char * n)
{
  strncpy(name,n,MAX_NAMESIZE);
  strncpy(cartoonData.name,n,MAX_NAMESIZE);
  strncpy(petData.name,n,MAX_NAMESIZE);
  sendMeToNine();
}

/*
 * \brief �������ĳ������ӿ���ȡ�ľ���
 *
 * \param num ����
 *
 */
void CartoonPet::releaseExp(DWORD num)
{
  if (0==cartoonData.time) return;
  if (!num) num = 1;
  cartoonData.addExp += num;
}

void CartoonPet::delMyself()
{
  Zebra::logger->warn("[����]cartoon %s ���Ҳ������˶�ɾ��",name);
  save(isAdopted()?Cmd::Session::SAVE_TYPE_RETURN:Cmd::Session::SAVE_TYPE_TIMETICK);
  setClearState();
}
