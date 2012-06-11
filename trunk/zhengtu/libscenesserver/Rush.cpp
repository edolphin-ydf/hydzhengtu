/**
 * \brief ���Ǵ������
 *
 * 
 */
#include <zebra/ScenesServer.h>
  
/**
 * \brief ���캯��,�����г�ʼ��
 *
 * \param rushID ���ǽű���id
 * \param rushDelay �����ӳ�ʱ�䣨boss����ʱ�䣩
 * \param countryID Ŀ�����id
 */
Rush::Rush(DWORD rushID,DWORD rushDelay,DWORD countryID)
:curPhase(0),id(rushID),countryID(countryID),bossID(0),boss(0),summonTime(0),lastBossHp(0),canSummon(true),clearDelay(0),end(false),lasttime(0),rushDelay(rushDelay)
{
  bzero(rushName,sizeof(rushName));
  bzero(bossName,sizeof(bossName));
  bzero(mapName,sizeof(mapName));
  bzero(text,sizeof(text));

  //summonCount = 0;
}

/**
 * \brief ��ʼ������
 * ��ȡ�ű�����Ŀ�곡����ӱ��ι���
 *
 * \return ��ʼ���Ƿ�ɹ�
 */
bool Rush::init(Scene * s)
{
  if (!loadRushData(id,rushDelay,countryID)) return false;
  setEndTime((rushDelay+lasttime)*1000);

  if (s)
  {
    strncpy(mapName,s->name,MAX_NAMESIZE);
    s->addRush(this);
  }
  else
  {
    Scene *scene=SceneManager::getInstance().getSceneByName(mapName);
    if (!scene) return false;
    scene->addRush(this);
  }

  if (0==strncmp("",text,128))
  {
    /*
    Cmd::Session::t_cityRush_SceneSession send;
    bzero(send.bossName,MAX_NAMESIZE);
    bzero(send.rushName,MAX_NAMESIZE);
    bzero(send.mapName,MAX_NAMESIZE);
    strncpy(send.bossName,bossName,MAX_NAMESIZE-1);
    strncpy(send.rushName,rushName,MAX_NAMESIZE-1);
    strncpy(send.mapName,mapName,MAX_NAMESIZE-1);
    send.delay = rushDelay;
    send.countryID = countryID;
    sessionClient->sendCmd(&send,sizeof(send));
    */
  }
  else
  {
    Cmd::Session::t_cityRushCust_SceneSession send;
    bcopy(text,send.text,128,sizeof(send.text));
    Zebra::logger->debug("%s",text);
    send.countryID = countryID;
    sessionClient->sendCmd(&send,sizeof(send));
  }

  return true;
}

/**
 * \brief ��������
 * npc��ɾ����SceneNpc::AI����
 *
 */
Rush::~Rush()
{
  /*
  if (boss)
  {
    SceneNpcManager::getMe().removeSceneNpc(boss);
    boss->scene->removeNpc(boss);
    SAFE_DELETE(boss);
  }
  */
}

/**
 * \brief �����ַ����õ����ǵĶ���
 *
 * \param action ��������
 * \return ����ö��ֵ
 */
Rush::rushAction Rush::parseAction(const char * action)
{
  if (0 == strcmp(action,"moveto")) return RUSH_MOVETO;
  if (0 == strcmp(action,"attack")) return RUSH_ATTACK;
  if (0 == strcmp(action,"summon")) return RUSH_SUMMON;
  if (0 == strcmp(action,"summon_rush")) return RUSH_SUMMON_RUSH;
  if (0 == strcmp(action,"recover")) return RUSH_RECOVER;
  if (0 == strcmp(action,"summon_on")) return RUSH_SUMMON_ON;
  if (0 == strcmp(action,"summon_off")) return RUSH_SUMMON_OFF;
  if (0 == strcmp(action,"clear")) return RUSH_CLEAR;
  if (0 == strcmp(action,"summon_pet")) return RUSH_SUMMON_PET;
  if (0 == strcmp(action,"broadcast")) return RUSH_BROADCAST;
  return RUSH_NORMAL;
}

/**
 * \brief ��ȡ���ǽű�
 *
 * \param rushID �ű�id
 * \param rushDelay �����ӳ�ʱ��
 * \param countryID Ŀ�����id
 * \return ��ȡ�Ƿ�ɹ�,���ɹ��򲻿���
 */
bool Rush::loadRushData(DWORD rushID,DWORD rushDelay,DWORD countryID)
{
  zXMLParser xml;
  if (!xml.initFile(Zebra::global["confdir"] +"city_rush.xml"))
  {
    Zebra::logger->error("��city_rush.xmlʧ��");
    return false;
  }

  xmlNodePtr root = xml.getRootNode("info");
  if (!root) return false;
  xmlNodePtr rushNode = xml.getChildNode(root,"rush");

  while (rushNode)
  {
    DWORD id=0;
    xml.getNodePropNum(rushNode,"id",&id,sizeof(id));
    if (rushID==id)
    {
      xml.getNodePropNum(rushNode,"boss",&bossID,sizeof(bossID));
      xml.getNodePropStr(rushNode,"bossname",bossName,sizeof(bossName));
      xml.getNodePropStr(rushNode,"name",rushName,sizeof(rushName));
      xml.getNodePropNum(rushNode,"cleardelay",&clearDelay,sizeof(clearDelay));
      xml.getNodePropNum(rushNode,"lasttime",&lasttime,sizeof(bossID));
      
      zXMLParser parser = zXMLParser();
      parser.initFile(Zebra::global["confdir"] + "scenesinfo.xml");
      xmlNodePtr root2 = parser.getRootNode("ScenesInfo");
      if (0==root2) return false;
      xmlNodePtr infoNode = parser.getChildNode(root2,"countryinfo");
      if (0==infoNode) return false;
      
      char str[MAX_NAMESIZE];
      xmlNodePtr dataNode = parser.getChildNode(infoNode,"country");
      while (dataNode)
      {                       
        DWORD id=0;
        parser.getNodePropNum(dataNode,"id",&id,sizeof(id));
        if (id==countryID)
        {
          parser.getNodePropStr(dataNode,"name",str,sizeof(str));
          break;
        }
        dataNode = parser.getNextNode(dataNode,"country");
      }
      if (!dataNode)
      {
        Zebra::logger->debug("��ȡ��������ʱ,δ�ҵ�Ŀ����� id=%d",countryID);
        return false;
      }
      strncpy(mapName,str,sizeof(mapName));
      
      DWORD mapID;
      xml.getNodePropNum(rushNode,"mapid",&mapID,sizeof(mapID));
      infoNode = parser.getChildNode(root2,"mapinfo");
      if (0==infoNode) return false;
      dataNode = parser.getChildNode(infoNode,"map");
      while (dataNode)
      {                       
        DWORD id=0;
        parser.getNodePropNum(dataNode,"mapID",&id,sizeof(id));
        if (id==mapID)
        {
          parser.getNodePropStr(dataNode,"name",str,sizeof(str));
          break;
        }
        dataNode = parser.getNextNode(dataNode,"map");
      }
      if (!dataNode)
      {
        Zebra::logger->debug("��ȡ��������ʱ,δ�ҵ�Ŀ���ͼ id=%d",mapID);
        return false;
      }
      strcat(mapName,"��");
      strcat(mapName,str);

      dataNode = xml.getChildNode(rushNode,"boss");
      while (dataNode)
      {
        bossDefine bd;
        xml.getNodePropNum(dataNode,"x",&bd.pos.x,sizeof(bd.pos.x));
        xml.getNodePropNum(dataNode,"y",&bd.pos.y,sizeof(bd.pos.y));
        xml.getNodePropNum(dataNode,"num",&bd.num,sizeof(bd.num));
        xml.getNodePropNum(dataNode,"region",&bd.region,sizeof(bd.region));
        xml.getNodePropNum(dataNode,"script",&bd.script,sizeof(bd.script));

        bossVector.push_back(bd);

        dataNode = xml.getNextNode(dataNode,"boss");
      }

      dataNode = xml.getChildNode(rushNode,"servant");
      while (dataNode)
      {
        servantDefine sd;
        xml.getNodePropNum(dataNode,"id",&sd.id,sizeof(sd.id));
        xml.getNodePropNum(dataNode,"num",&sd.num,sizeof(sd.num));
        xml.getNodePropNum(dataNode,"rate",&sd.rate,sizeof(sd.rate));
        xml.getNodePropNum(dataNode,"interval",&sd.interval,sizeof(sd.interval));

        servantVector.push_back(sd);

        dataNode = xml.getNextNode(dataNode,"servant");
      }

      dataNode = xml.getChildNode(rushNode,"notify");
      if (dataNode)
        xml.getNodePropStr(dataNode,"str",text,sizeof(text));

      char t[128];
      std::string::size_type pos=0;
      dataNode = xml.getChildNode(rushNode,"starttext");
      if (dataNode)
      {
        xml.getNodePropStr(dataNode,"str",t,sizeof(t));
        startText = t;
        pos = startText.find("time",0);
        if (pos!=std::string::npos)
          startText.replace(pos,4,"%s");
      }

      dataNode = xml.getChildNode(rushNode,"endtext");
      if (dataNode)
      {
        xml.getNodePropStr(dataNode,"str",t,sizeof(t));
        endText = t;
        pos = endText.find("time",0);
        if (pos!=std::string::npos)
          endText.replace(pos,4,"%s");
      }

      dataNode = xml.getChildNode(rushNode,"phase");
      phaseDefine pd;
      pd.action = RUSH_RELIVE;
      pd.lasttime = rushDelay;
      setPhaseTime(pd.lasttime*1000);
      phaseVector.push_back(pd);
      while (dataNode)
      {
        char action[32];
        bzero(action,sizeof(action));
        bzero(pd.say,sizeof(pd.say));
        xml.getNodePropStr(dataNode,"action",action,sizeof(action));
        pd.action = parseAction(action);
        if (pd.action)
        {
          xml.getNodePropNum(dataNode,"x",&pd.pos.x,sizeof(pd.pos.x));
          xml.getNodePropNum(dataNode,"y",&pd.pos.y,sizeof(pd.pos.y));
          xml.getNodePropNum(dataNode,"region",&pd.region,sizeof(pd.region));
          xml.getNodePropStr(dataNode,"say",pd.say,sizeof(pd.say));
          xml.getNodePropNum(dataNode,"lasttime",&pd.lasttime,sizeof(pd.lasttime));
          xml.getNodePropNum(dataNode,"id",&pd.x,sizeof(pd.x));
          xml.getNodePropNum(dataNode,"num",&pd.y,sizeof(pd.y));
          xml.getNodePropNum(dataNode,"script",&pd.z,sizeof(pd.z));

          phaseVector.push_back(pd);
        }

        dataNode = xml.getNextNode(dataNode,"phase");
      }
      pd.action = RUSH_END;
      bzero(pd.say,sizeof(pd.say));
      phaseVector.push_back(pd);

      //Zebra::logger->info("��ȡ�������ݳɹ�:name=%s mapname=%s boss=%d servant=%d phase=%d",rushName,mapName,bossVector.size(),servantVector.size(),phaseVector.size());
      return true;
      break;
    }
    rushNode = xml.getNextNode(rushNode,"rush");
  }
  Zebra::logger->info("��ȡ��������ʧ��:rushID=%d countryID=%d",rushID,countryID);
  return false;
}

/**
 * \brief ���������̵�������
 * ��鹥�ǽ���ʱ�䡢�׶ν���ʱ�䡢�׶ν�����־
 * ���������������һ�׶�
 *
 */
void Rush::process()
{
  if (end) return;

  if (checkEndTime())
    if (boss && boss->AIC->isActive())
      terminate();
    else
      onTimeOver();
  else
  {
    DWORD t = endTime.sec()-SceneTimeTick::currentTime.sec();
    if (t>=900 && t<=3600 && t%900<2)
    {
      char tex[32];
      _snprintf(tex,sizeof(tex)-1,"%u����",t/60);
      Channel::sendCountryInfo(countryID,Cmd::CHAT_TYPE_GM,endText.c_str(),tex);
      Zebra::logger->debug(endText.c_str(),tex);
    }
  }

  if (boss)
  {
    if (boss->hp<lastBossHp)
      if (checkSummonTime())
        summonServant();
    lastBossHp = boss->hp;

    if (zSceneEntry::SceneEntry_Death==boss->getState())
    {
      if (curPhase != phaseVector.size()-1)
      {
        curPhase = phaseVector.size()-1;
        setPhaseTime(clearDelay*1000);
        //if (boss->AIC) boss->AIC->setNormalAI();
        Zebra::logger->debug("boss����,�׶�%d",curPhase);
      }
    }
  }

  if (checkPhaseTime())
    onPhaseTimeOver();

  switch (phaseVector[curPhase].action)
  {
    case RUSH_RELIVE:
      {
        if (rushDelay>=900
            && endTime.sec()-SceneTimeTick::currentTime.sec()>3600
            && rushDelay>=nextPhaseTime.sec()-SceneTimeTick::currentTime.sec())
        {  
          char tex[32];
          _snprintf(tex,sizeof(tex)-1,"%u����",rushDelay/60);
          Channel::sendCountryInfo(countryID,Cmd::CHAT_TYPE_GM,startText.c_str(),tex);
          Zebra::logger->debug(startText.c_str(),tex);
          rushDelay -= 900;
        }
      }
      break;
    case RUSH_MOVETO:
      {
        if (!boss) return;
        if (boss->scene->zPosShortRange(boss->getPos(),phaseVector[curPhase].pos,phaseVector[curPhase].region))
          enterNextPhase();
      }
      break;
    default:
      break;
  }
}

/**
 * \brief �ܹ���ʱ�����ʱ�Ĵ���
 * ֻҪ�����Ѿ����ڽ����׶�,��ֱ����ת�������׶�
 * ���������npc���ӳ���Ϊ�����׶ε�ʱ��
 *
 */
void Rush::onTimeOver()
{
  if (curPhase != phaseVector.size()-1)
  {
    curPhase = phaseVector.size()-1;
    setPhaseTime(clearDelay*1000);
    //if (boss) boss->AIC->setNormalAI();
  }
}

/**
 * \brief ��������
 * 
 */
void Rush::terminate()
{
  if (curPhase != phaseVector.size()-1)
  {
    curPhase = phaseVector.size()-1;
    //if (boss) boss->AIC->setNormalAI();
  }
  setPhaseTime(0);
}

/**
 * \brief �����Ƿ��Ѿ�����
 *
 * \return �Ƿ����
 */
bool Rush::isEnd()
{
  return end;
}

/**
 * \brief �����ܽ���ʱ��
 *
 * \param delay �ӳ�
 */
void Rush::setEndTime(const int delay)
{
  endTime = SceneTimeTick::currentTime;
  endTime.addDelay(delay);
}

/**
 * \brief ���ý׶ν���ʱ��
 *
 * \param delay �ӳ�
 */
void Rush::setPhaseTime(const int delay)
{
  nextPhaseTime = SceneTimeTick::currentTime;
  nextPhaseTime.addDelay(delay);
}

/**
 * \brief ��鹥��ʱ���Ƿ��ѵ�
 *
 * \return ʱ���Ƿ��ѵ� 
 */
bool Rush::checkEndTime()
{
  return SceneTimeTick::currentTime >= endTime;
}

/**
 * \brief ���׶ν���ʱ��
 *
 * \return ʱ���Ƿ��ѵ�
 */
bool Rush::checkPhaseTime()
{
	return SceneTimeTick::currentTime >= nextPhaseTime;
}

/**
 * \brief �׶ν����¼��Ĵ���
 * ��ͬ�׶ν���ʱ����ͬ
 * �������Ķ������ڽ׶ν���ʱ����
 * ����������ƶ�,�ڽ׶ν���ʱδ����Ŀ�ĵ�,��ֱ����ת��ȥ
 *
 */
void Rush::onPhaseTimeOver()
{
  switch (phaseVector[curPhase].action)
  {
    case RUSH_RELIVE:
      {
        //Zebra::logger->debug("%s : �ٻ�boss",rushName);
        if (!summonBoss())
        {
          Zebra::logger->error("���﹥��: %s �ٻ�boss %s ʧ��,��������",rushName,bossName);
          end = true;
        }
      }
      break;
    case RUSH_MOVETO:
      {
        //if (!boss) return;
        if (!boss->scene->zPosShortRange(boss->getPos(),phaseVector[curPhase].pos,phaseVector[curPhase].region))              
          boss->warp(phaseVector[curPhase].pos);
      }
      break;
    case RUSH_CLEAR:
      {
        clearServants();
      }
      break;
    case RUSH_END:
      {
        deleteBoss();
        clearServants();
        end=true;
      }
      break;
    default:
      break;
  }
  enterNextPhase();
}

/**
 * \brief �������׶ε��¼�
 * �ָ����ٻ������ء�˵���ȶ����ڽ׶ο�ʼʱ����
 * ��Щ������˲���,����׶γ���ʱ�䲻Ϊ0,�������µ�ʱ����boss����ͨ����
 *
 */
void Rush::enterNextPhase()
{
  if (!end && curPhase<phaseVector.size()-1)
  {
    curPhase++;
    Zebra::logger->debug("%s : �׶�%d",rushName,curPhase);
    setPhaseTime(phaseVector[curPhase].lasttime*1000);
    if (strcmp(phaseVector[curPhase].say,""))
      Channel::sendNine(boss,phaseVector[curPhase].say);

    //���սű�������
    if (boss->AIC->isActive()
        && phaseVector[curPhase].action!=RUSH_SUMMON_PET
        && phaseVector[curPhase].action!=RUSH_SUMMON_RUSH
        && phaseVector[curPhase].action!=RUSH_END)
      return;

    switch (phaseVector[curPhase].action)
    {
      case RUSH_RELIVE:
        break;
      case RUSH_MOVETO:
        {
          t_NpcAIDefine d(NPC_AI_PATROL,phaseVector[curPhase].pos,phaseVector[curPhase].region,phaseVector[curPhase].region,phaseVector[curPhase].lasttime);
          boss->AIC->setAI(d);
        }
        break;
      case RUSH_ATTACK:
        {
          t_NpcAIDefine d(NPC_AI_ATTACK,phaseVector[curPhase].pos,phaseVector[curPhase].region,phaseVector[curPhase].region,phaseVector[curPhase].lasttime);
          boss->AIC->setAI(d);
        }
        break;
      case RUSH_RECOVER:
        {
          boss->hp = boss->getMaxHP();
          //boss->AIC->setNormalAI();
        }
        break;
      case RUSH_SUMMON:
        {
          summonServant();
          //boss->AIC->setNormalAI();
        }
        break;
      case RUSH_BROADCAST:
        {
          Channel::sendCountryInfo(countryID,Cmd::CHAT_TYPE_GM,phaseVector[curPhase].say);
        }
        break;
      case RUSH_SUMMON_RUSH:
        {
          zNpcB *base = npcbm.get(phaseVector[curPhase].x);

          if (base)
          {
            t_NpcDefine define;
            zPos pos = phaseVector[curPhase].pos;
            define.id = base->id;
            strncpy(define.name,base->name,MAX_NAMESIZE-1);
            define.pos = phaseVector[curPhase].pos;
            define.num = 1;
            define.interval = 0x0fffffff;
            define.initstate = zSceneEntry::SceneEntry_Normal;
            define.width = phaseVector[curPhase].region;
            define.height = phaseVector[curPhase].region;
            define.pos -= zPos(phaseVector[curPhase].region/2,phaseVector[curPhase].region/2);
            define.scriptID = phaseVector[curPhase].z;

            Scene * scene = SceneManager::getInstance().getSceneByName(mapName);
            if (!scene)
            {
              Zebra::logger->debug("�ٻ�����servantʱ,δ�ҵ��õ�ͼ name=%s",mapName);
              return;
            }
            scene->initRegion(define.region,define.pos,define.width,define.height);

            for (DWORD i=0; i<phaseVector[curPhase].y; i++)
            {
              SceneNpc * servant = scene->summonOneNpc<SceneNpc>(define,zPos(0,0),base,0,0,0);
              //summonCount++;
              if (servant)
              {
                servant->aif |= AIF_ACTIVE_MODE;
                servant->isRushNpc = true;
                servants.push_back(servant);
#ifdef _DEBUG
                Zebra::logger->debug("[���﹥��]%s (%u,%u)",servant->name,servant->getPos().x,servant->getPos().y);
#endif
              }
            }
          }
          //boss->AIC->setNormalAI();
        }
        break;
      case RUSH_SUMMON_PET:
        {
          summonPet();
        }
        break;
      case RUSH_SUMMON_ON:
        {
          canSummon = true;
          //boss->AIC->setNormalAI();
        }
        break;
      case RUSH_SUMMON_OFF:
        {
          canSummon = false;
          //boss->AIC->setNormalAI();
        }
        break;
      case RUSH_CLEAR:
        {
          //boss->AIC->setNormalAI();
        }
        break;
      case RUSH_END:
        {
          setPhaseTime(clearDelay*1000);
          //boss->AIC->setNormalAI();
        }
        break;
      default:
        break;
    }
  }
#if 0
  else
    Zebra::logger->debug("%s : ���ǽ���",rushName);
#endif
}

/**
 * \brief �ٻ�һ��boss
 * �ű��пɶ�����boss,�ٻ�ʱѡ��һ��,�Ա�Ӳ�ͬ��λ�ó���,���������
 *
 * \return �ٻ��Ƿ�ɹ�
 */
bool Rush::summonBoss()
{
  zNpcB *base = npcbm.get(bossID);
  if (NULL == base)
  {
    //Zebra::logger->debug("�ٻ�����bossʱ,δ�ҵ��ù��� id=%d",bossID);
    return false;
  }

  int index = randBetween(0,bossVector.size()-1);

  t_NpcDefine define;
  define.id = base->id;
  strncpy(define.name,base->name,sizeof(define.name));
  define.pos = bossVector[index].pos;
  define.num = bossVector[index].num;
  define.interval = 0x0fffffff;//���ʱ��һ��Ҫ�� �������rush��cleartime,�ͻ���ǰdelete boss,clearʱ�䵽��ʱ��ͻ�coredump
  define.initstate = zSceneEntry::SceneEntry_Normal;
  define.width = bossVector[index].region*2;
  define.height = bossVector[index].region*2;
  define.pos -= zPos(bossVector[index].region,bossVector[index].region);
  define.scriptID = bossVector[index].script;

  Scene * scene = SceneManager::getInstance().getSceneByName(mapName);
  if (!scene)
  {
    Zebra::logger->debug("�ٻ�����bossʱ,δ�ҵ��õ�ͼ name=%s",mapName);
    return false;
  }
  scene->initRegion(define.region,define.pos,define.width,define.height);

  boss = scene->summonOneNpc<SceneNpc>(define,bossVector[index].pos,base,0);
  //summonCount++;
  if (!boss)
  {
    Zebra::logger->debug("�ٻ�����bossʧ�� id=%d",base->id);
    return false;
  }
  //SceneNpcManager::getMe().addSpecialNpc(boss);

  boss->isRushNpc = true;
  summonTime = SceneTimeTick::currentTime;

  return true;
}

/**
 * \brief ����Ƿ��˿��ٻ����˵�ʱ��
 *
 * \return �Ƿ��˿��ٻ���ʱ��
 */
bool Rush::checkSummonTime()
{
  return SceneTimeTick::currentTime >= summonTime;
}

/**
 * \brief �ٻ�����
 * �ű��п��Զ���������,ÿ�������ٻ����ʺͼ��
 * ÿ���ٻ�ʱ�ж����е�����,����һ���ٻ�����������,Ҳ����һ��Ҳû�ٻ�
 * �ٻ�ʱ���ۼ�,��һ���ٻ���1����30������˺�һ����20�������,����50����boss�����ٻ�
 *
 * \return �ٻ��Ƿ�ɹ�
 */
bool Rush::summonServant()
{
  if (!boss) return false;
  if (!canSummon) return false;

  int die = randBetween(0,100);

  for (DWORD index=0; index<servantVector.size(); index++)
  {
    if (servantVector[index].rate<die)
      continue;
    
    zNpcB *base = npcbm.get(servantVector[index].id);
    if (NULL == base)
    {
      Zebra::logger->debug("�ٻ� %s ������ʱ,δ�ҵ��ù��� id=%d",boss->name,servantVector[index].id);
      return false;
    }

    t_NpcDefine define;
    define.id = base->id;
    strncpy(define.name,base->name,sizeof(define.name));
    zPos pos(boss->getPos());
    pos -= zPos(summon_servant_region*2,summon_servant_region*2);
    define.pos = pos;
    define.num = servantVector[index].num;
    define.interval = 0x0fffffff;
    define.initstate = zSceneEntry::SceneEntry_Normal;
    define.width = summon_servant_region*4;
    define.height = summon_servant_region*4;
    //define.pos -= zPos(summon_servant_region,summon_servant_region);

    Scene * scene = SceneManager::getInstance().getSceneByName(mapName);
    if (!scene)
    {
      Zebra::logger->debug("�ٻ� %s ������ʱ,δ�ҵ��õ�ͼ name=%s",boss->name,mapName);
      return false;
    }
    scene->initRegion(define.region,define.pos,define.width,define.height);

    SceneNpc * servant;
    int count = 0;
    for (int i=0; i<servantVector[index].num; i++)
    {
      servant = scene->summonOneNpc<SceneNpc>(define,pos,base,0);
      if (servant)
      {
        servant->aif |= AIF_ACTIVE_MODE;
        servant->isRushNpc = true;
        servants.push_back(servant);
        count++;
      }
    }

    summonTime = SceneTimeTick::currentTime;
    summonTime.addDelay(servantVector[index].interval*1000);
    Zebra::logger->debug("%s summonServant:summon %u ��,servant %u ��",rushName,count,servants.size());
  }
  
  return true;
}
bool Rush::summonPet()
{
  if (!boss) return false;
  if (!canSummon) return false;

  int die = randBetween(0,100);

  for (DWORD index=0; index<servantVector.size(); index++)
  {
    if (servantVector[index].rate<die)
      continue;
    
    zNpcB *base = npcbm.get(servantVector[index].id);
    if (NULL == base)
    {
      Zebra::logger->debug("�ٻ� %s �ĳ���ʱ,δ�ҵ��ù��� id=%d",boss->name,servantVector[index].id);
      return false;
    }

    t_NpcDefine define;
    define.id = base->id;
    strncpy(define.name,base->name,sizeof(define.name));
    zPos pos(boss->getPos());
    pos -= zPos(summon_servant_region*2,summon_servant_region*2);
    define.pos = pos;
    define.num = servantVector[index].num;
    define.interval = 0x0fffffff;
    define.initstate = zSceneEntry::SceneEntry_Normal;
    define.width = summon_servant_region*4;
    define.height = summon_servant_region*4;
    //define.pos -= zPos(summon_servant_region,summon_servant_region);

    Scene * scene = SceneManager::getInstance().getSceneByName(mapName);
    if (!scene)
    {
      Zebra::logger->debug("�ٻ� %s ������ʱ,δ�ҵ��õ�ͼ name=%s",boss->name,mapName);
      return false;
    }
    scene->initRegion(define.region,define.pos,define.width,define.height);

    ScenePet * servant;
    int count = 0;
    for (int i=0; i<servantVector[index].num; i++)
    {
      servant = scene->summonOneNpc<ScenePet>(define,pos,base,0);
      if (servant)
      {
        servant->isRushNpc = true;
        servants.push_back(servant);

        servant->setMaster(boss);
        boss->totems.push_back(servant);
        count++;
      }
    }

    summonTime = SceneTimeTick::currentTime;
    summonTime.addDelay(servantVector[index].interval*1000);
    Zebra::logger->debug("%s summonPet:summon %u ��,servant %u ��",rushName,count,servants.size());
  }
  
  return true;
}

/**
 * \brief ɾ��boss
 * ����clearme�ı�־λ,delete������SceneNpc::AI����
 *
 */
void Rush::deleteBoss()
{
  if (boss)
  {
    /*
    Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
    removeNpc.dwMapNpcDataPosition = boss->tempid;
    boss->scene->sendCmdToNine(boss->getPosI(),&removeNpc,sizeof(removeNpc),false);

    SceneNpcManager::getMe().removeSceneNpc(boss);
    boss->scene->removeNpc(boss);

    SAFE_DELETE(boss);
    */
    boss->setClearState();
    SceneNpcManager::getMe().addSpecialNpc(boss,true);
    boss = 0;
    Zebra::logger->debug("%s : ɾ��boss",rushName);
  }
  else
    Zebra::logger->debug("%s : boss�Ѿ�ɾ��",rushName);
}

/**
 * \brief ɾ���������� 
 * ����clearme�ı�־λ,delete������SceneNpc::AI����
 *
 */
void Rush::clearServants()
{
  for (std::list<SceneNpc *>::iterator it=servants.begin(); it!=servants.end(); it++)
  {
    /*
    Cmd::stRemoveMapNpcMapScreenUserCmd removeNpc;
    removeNpc.dwMapNpcDataPosition = (*it)->tempid;
    (*it)->scene->sendCmdToNine((*it)->getPosI(),&removeNpc,sizeof(removeNpc),false);

    SceneNpcManager::getMe().removeSceneNpc((*it));
    (*it)->scene->removeNpc((*it));

    SceneNpc *npc = *it;
    SAFE_DELETE(npc);
    */
    (*it)->setClearState();
    SceneNpcManager::getMe().addSpecialNpc((*it),true);
  }
  Zebra::logger->debug("%s : ɾ�� %u ��servants",rushName,servants.size());
  servants.clear();
}
