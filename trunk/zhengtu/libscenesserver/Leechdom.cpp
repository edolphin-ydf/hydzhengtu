/**
 * \brief ҩƷ,ʳ������Ʒʹ��
 *
 * �ָ�����ֵ,����ֵ,����ֵ����Ʒ
 */

#include <zebra/ScenesServer.h>


/**
 * \brief ʹ��ҩƷ
 *
 *
 * \param obj: ʹ����Ʒָ�� 
 * \return ʹ���Ƿ�ɹ�
 */
bool SceneUser::useLeechdom(zObject *obj)
{
  if (!useRes) return true;

  DWORD old_num=obj->data.dwNum;
  DWORD update = 0;
  switch(obj->base->leechdom.id)
  {
    case Leechdom_dam:
      {
      }
      break;
    case Leechdom_def:
      {
      }
      break;
    case Leechdom_poison:
      {
      }
      break;
    case Leechdom_sppersist:
      {
        leechdom.add(Leechdom_sppersist,0,obj->base->leechdom.time);
      }
      break;
    case Leechdom_spup:
      {
        DWORD temp = (DWORD)(charstate.maxsp * (obj->base->leechdom.effect/100.0f));
        charbase.sp=(charbase.sp+temp) >
          charstate.maxsp?charstate.maxsp:(charbase.sp+temp);
        update |= 0x04;
      }
      break;
    case Leechdom_spcostdown:
      {
      }
      break;
    case Leechdom_sp:
      {
        DWORD temp = obj->base->leechdom.effect;
        charbase.sp=(charbase.sp+temp) >
          charstate.maxsp?charstate.maxsp:(charbase.sp+temp);
        update |= 0x04;
      }
      break;
    case Leechdom_spresumeup:
      {
        leechdom.add(Leechdom_spresumeup,obj->base->leechdom.effect,obj->base->leechdom.time);
      }
      break;
    case Leechdom_hp:
    case Leechdom_hp5:
      {
        charbase.hp=(charbase.hp+obj->base->leechdom.effect) >
          charstate.maxhp?charstate.maxhp:(charbase.hp+obj->base->leechdom.effect);
        update |= 0x01;
      }
      break;
    case Leechdom_hppersist:
    case Leechdom_hppersist5:
      {
        leechdom.add(Leechdom_hppersist,obj->base->leechdom.effect,obj->base->leechdom.time);
      }
      break;
    case Leechdom_mp:
      {
        charbase.mp=(charbase.mp+obj->base->leechdom.effect) >
          charstate.maxmp?charstate.maxmp:(charbase.mp+obj->base->leechdom.effect);
        update |= 0x02;
      }
      break;
    case Leechdom_mppersist:
      {
        leechdom.add(Leechdom_mppersist,obj->base->leechdom.effect,obj->base->leechdom.time);
      }
      break;
    case Leechdom_hpmax:
      {
        DWORD need = 0;
        if (obj->base->id == 881)
        {
          need = (charstate.maxhp-charbase.hp>3000)?3000:charstate.maxhp-charbase.hp;
        }
        else
        {
          need = (charstate.maxhp-charbase.hp>2500)?2500:charstate.maxhp-charbase.hp;
        }
        if (0==need)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����ڲ���Ҫ");
          return true;
        }

        charbase.hp += ((DWORD)(obj->data.dur*100)>need)?need:obj->data.dur*100;

        DWORD minus = need>200?need:200;
        if (minus%100>50)
          minus = minus/100+1;
        else
          minus = minus/100;

        if (obj->data.dur>minus)
          obj->data.dur -= minus;
        else
        {
          obj->data.dur = 0;
          obj->data.dwNum = 0;
        }

        Cmd::stDurabilityUserCmd ret;
        ret.dwThisID = obj->data.qwThisID;
        ret.dwDur = obj->data.dur;
        ret.dwMaxDur = obj->data.maxdur;
        sendCmdToMe(&ret,sizeof(ret));

        update |= 0x01;
      }
      break;
    case Leechdom_mpmax:
      {
        DWORD need = charstate.maxmp-charbase.mp;
        if (0==need)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����ڲ���Ҫ");
          return true;
        }

        charbase.mp += ((DWORD)(obj->data.dur*50)>need)?need:obj->data.dur*50;

        DWORD minus = need>50?need:50;
        if (minus%50>25)
          minus = minus/25+1;
        else
          minus = minus/25;

        if (obj->data.dur>minus)
          obj->data.dur -= minus;
        else
        {
          obj->data.dur = 0;
          obj->data.dwNum = 0;
        }

        Cmd::stDurabilityUserCmd ret;
        ret.dwThisID = obj->data.qwThisID;
        ret.dwDur = obj->data.dur;
        ret.dwMaxDur = obj->data.maxdur;
        sendCmdToMe(&ret,sizeof(ret));

        update |= 0x01;
      }
      break;
    case Leechdom_chocolate:
      {
        leechdom.add(Leechdom_hppersist,obj->base->leechdom.effect,obj->base->leechdom.time);
        leechdom.add(Leechdom_mppersist,obj->base->leechdom.effect,obj->base->leechdom.time);
      }
      break;
    default:
      {
        Zebra::logger->debug("����ʶ�����Ʒ����:%u",obj->base->leechdom.id);
        return false;
      }
      break;
  }

  if (obj->base->leechdom.id!=Leechdom_hpmax && obj->base->leechdom.id!=Leechdom_mpmax && (int)obj->data.dwNum > 0)
    obj->data.dwNum--;

  if (obj->data.dwNum==0)//����0
  {
    zObject::logger(obj->createid,obj->data.qwThisID,obj->data.strName,obj->data.dwNum,obj->data.dwNum,0,this->id,this->name,0,NULL,"��ҩ",NULL,0,0);
    packs.removeObject(obj); //notify and delete
  }
  else
  {
    //zObject::logger(obj->createid,obj->data.qwThisID,obj->base->name,obj->data.dwNum,this->id,this->name,this->id,this->name,"��ҩ");
    if (old_num != obj->data.dwNum)
    {
      Cmd::stRefCountObjectPropertyUserCmd send;
      send.qwThisID=obj->data.qwThisID;
      send.dwNum=obj->data.dwNum;
      sendCmdToMe(&send,sizeof(send));
    }
  }

  if (update) {
    //notify me
    Cmd::stSetHPAndMPDataUserCmd ret;
    ret.dwHP = charbase.hp;
    ret.dwMP = charbase.mp;
    //ret.dwSP = charbase.sp;
    this->sendCmdToMe(&ret,sizeof(ret));
  }
  if (update & 0x01) {
	  TeamManager * team = SceneManager::getInstance().GetMapTeam(TeamThisID);

	  if(team)
		  team->sendtoTeamCharData(this);
  }

#ifdef _TEST_DATA_LOG
  switch(obj->data.maxhp)
  {
    case Leechdom_sppersist:
    case Leechdom_spup:
    case Leechdom_sp:
    case Leechdom_spresumeup:
      {
        writeCharTest(Cmd::Record::SP_WRITEBACK);
      }
      break;
    case Leechdom_hp:
    case Leechdom_hp5:
    case Leechdom_hppersist:
    case Leechdom_hppersist5:
    case Leechdom_hpmax:
      {
        writeCharTest(Cmd::Record::HP_WRITEBACK);
      }
      break;
    case Leechdom_mp:
    case Leechdom_mppersist:
    case Leechdom_mpmax:
      {
        writeCharTest(Cmd::Record::MP_WRITEBACK);
      }
      break;
    default:
      break;
  }
#endif // _TEST_DATA_LOG��������

  return true;
}
/**
 * \brief ��ʱ������һ�ӳ�������
 *
 *
 * \param type: ҩƷ����
 * \param value: ��ֵ
 * \param times: ����ʱ��
 */
void Leechdom::add(LeechdomType type,WORD value,WORD times)
{
  LeechdomElement ele;
  ele.type = type;
  ele.value = value;
  ele.times = times;
  mlock.lock();
  element.push_back(ele);
  mlock.unlock();
}

/**
 * \brief ����ʱ��Ҫ�������ҩƷЧ��
 *
 */
void Leechdom::clear()
{
  mlock.lock();
  element.clear();
  mlock.unlock();
}
/**
 * \brief ��ʱˢ����ʱ��
 *
 */
void Leechdom::checkCooling()
{
  if ((int)damcooling > 0)
    damcooling --;
  if ((int)defcooling > 0)
    defcooling --;
  if ((int)hpcooling > 0)
    hpcooling --;
  if ((int)spcooling > 0)
    spcooling --;
  if ((int)mpcooling > 0)
    mpcooling --;
}
/**
 * \brief ��ʱˢ��ҩƷ��ֵ����
 *
 *
 * \param pUser: �û�
 * \param update: �ı���û�����ֵ(�����Ʊ�ʾ)
 * \return update
 */
DWORD Leechdom::fresh(SceneUser *pUser,DWORD &update)
{
  checkCooling();
  Leechdom_iterator iter;
  mlock.lock();
  for(iter = element.begin() ; iter != element.end() ; )
  {
    if ((*iter).times == 0)
    {
      iter = element.erase(iter);
      continue;
    }
    LeechdomElement &ele = *iter;
    if ((int)ele.times > 0) 
    {
      ele.times --;
    }
    switch(ele.type)
    {
      case Leechdom_dam:
        {
        }
        break;
      case Leechdom_def:
        {
        }
        break;
      case Leechdom_poison:
        {
        }
        break;
      case Leechdom_sppersist:
        {
          if (ele.times == 0)
          {
            sppersist = 0;
          }
        }
        break;
      case Leechdom_spcostdown:
        {
        }
        break;
      case Leechdom_spresumeup:
        {
          pUser->charbase.sp=(pUser->charbase.sp+ele.value) >
            pUser->charstate.maxsp?pUser->charstate.maxsp:(pUser->charbase.sp+ele.value);
          update |= 0x04;
        }
        break;
      case Leechdom_hppersist:
        {
          pUser->charbase.hp=(pUser->charbase.hp+ele.value) >
            pUser->charstate.maxhp?pUser->charstate.maxhp:(pUser->charbase.hp+ele.value);
          update |= 0x01;
        }
        break;
      case Leechdom_mppersist:
        {
          pUser->charbase.mp=(pUser->charbase.mp+ele.value) >
            pUser->charstate.maxmp?pUser->charstate.maxmp:(pUser->charbase.mp+ele.value);
          update |= 0x02;
        }
        break;
      default:
        break;
    }
    iter ++;
  }
  mlock.unlock();
  return update;
}

/**
 * \brief ����Ƿ���ȴ��
 *
 *
 * \param type: ����
 * \return �Ƿ�����ٴ�ʹ�ø���ҩƷ
 */
bool Leechdom::isCooling(DWORD type)
{
  switch(type)
  {
    case Leechdom_dam:
      {
        if (damcooling == 0)
        {
          damcooling = 3;
          return false;
        }
      }
      break;
    case Leechdom_def:
      {
        if (defcooling == 0)
        {
          defcooling = 3;
          return false;
        }
      }
      break;
    case Leechdom_poison:
      {
      }
      break;
    case Leechdom_sppersist:
    case Leechdom_spup:
    case Leechdom_spcostdown:
    case Leechdom_sp:
    case Leechdom_spresumeup:
      {
        if (spcooling == 0)
        {
          spcooling = 3;
          return false;
        }
      }
      break;
    case Leechdom_hp:
    case Leechdom_hppersist:
    case Leechdom_hpmax:
    case Leechdom_chocolate:
      {
        if (hpcooling == 0)
        {
          hpcooling = 3;
          return false;
        }
      }
      break;
    case Leechdom_hp5:
    case Leechdom_hppersist5:
      {
        if (hpcooling == 0)
        {
          hpcooling = 5;
          return false;
        }
      }
      break;
    case Leechdom_mp:
    case Leechdom_mppersist:
      {
        if (mpcooling == 0)
        {
          mpcooling = 3;
          return false;
        }
      }
      break;
    case Leechdom_mpmax:
      {
        return false;
      }
      break;
    default:
      break;
  }
  return true;
}
// */
