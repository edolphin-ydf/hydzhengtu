/**
 * \brief ��ս������
 *
 * 
 */
#include <zebra/ScenesServer.h>

CountryDareM *CountryDareM::_instance = 0;


void CountryDareM::timer()
{
  for (DWORD i=0; i<dares.size(); i++)
  {
    int time_diff = abs((long)(SceneTimeTick::currentTime.sec() - dares[i].last_attack_time));
    int king_time_diff = abs((long)(SceneTimeTick::currentTime.sec() - dares[i].last_king_attack_time));

    int gen_time_diff = abs((long)(SceneTimeTick::currentTime.sec() - dares[i].last_gen_attack_time));
    int gen_att_time_diff  = abs((long)(SceneTimeTick::currentTime.sec() - dares[i].last_att_gen_attack_time));
    
    if (dares[i].last_attack_time!=0 && time_diff < 120)
    {
      Cmd::Session::t_countryNotify_SceneSession send;
      bzero(send.info,sizeof(send.info));
      sprintf(send.info,"%s","��˳����ܹ����У����ٻ�Ԯ");
      send.dwCountryID = dares[i].id;
      send.infoType = Cmd::INFO_TYPE_ATT_FLAG;
      sessionClient->sendCmd(&send,sizeof(send));
    }

    if (dares[i].last_king_attack_time!=0 && king_time_diff < 120)
    {
      Cmd::Session::t_countryNotify_SceneSession send;
      bzero(send.info,sizeof(send.info));
      sprintf(send.info,"%s","�������ܹ����У����ٻ�Ԯ");
      send.infoType = Cmd::INFO_TYPE_ATT_FLAG;
      send.dwCountryID = dares[i].id;
      sessionClient->sendCmd(&send,sizeof(send));
    }
    if (dares[i].last_gen_attack_time!=0 && gen_time_diff < 600)
    {
      Cmd::Session::t_countryNotify_SceneSession send;
      bzero(send.info,sizeof(send.info));
      sprintf(send.info,"%s","�ҹ������⵽�˹��������ٻ�Ԯ");
      send.dwCountryID = dares[i].id;
      sessionClient->sendCmd(&send,sizeof(send));
    }
    
    if (dares[i].last_att_gen_attack_time!=0 && gen_att_time_diff < 600)
    {
      Cmd::Session::t_countryNotify_SceneSession send;
      bzero(send.info,sizeof(send.info));
      sprintf(send.info,"%s","�ҹ������⵽�˹��������ٻ�Ԯ");
      send.dwCountryID = dares[i].id;
      sessionClient->sendCmd(&send,sizeof(send));
    }

  }
}

/**
 * \brief ͳ�ƴ���ָ��NPC�ĸ���
 */
struct TotalAllNpc : public zSceneEntryCallBack
{
  int count;  /// ����NPC����
  DWORD dwNpcID; // Ҫͳ�Ƶ�NPCID
  TotalAllNpc() : count(0),dwNpcID(0) {};
  
  /**
   * \brief �ص�����
   * \param entry ��ͼ���,���������
   * \return �ص��Ƿ��ͳɹ�
   */
  bool exec(zSceneEntry *entry)
  {
    if (((SceneNpc *)entry)->getState() != zSceneEntry::SceneEntry_Death)
    {
      count++;
    }
    
    return true;
  }
};

/**
 * \brief �������и���
 */
struct ReliveSpecNpc : public zSceneEntryCallBack
{
  ReliveSpecNpc() {};
  
  /**
   * \brief �ص�����
   * \param entry ��ͼ���,���������
   * \return �ص��Ƿ��ͳɹ�
   */
  bool exec(zSceneEntry *entry)
  {
    zRTime cur;
    ((SceneNpc*)entry)->reliveTime = cur;
    //((SceneNpc*)entry)->setMoveTime(cur);
    //((SceneNpc*)entry)->setAttackTime(cur);

    return true;
  }
};

/**
 * \brief �޸����и��츴��ʱ��
 */
struct DelaySpecNpc : public zSceneEntryCallBack
{
  DelaySpecNpc() {};
  
  /**
   * \brief �ص�����
   * \param entry ��ͼ���,���������
   * \return �ص��Ƿ��ͳɹ�
   */
  bool exec(zSceneEntry *entry)
  {
    return true;
  }
};

bool CountryDareM::isAttackMainGen(Scene* scene)
{
  TotalAllNpc totalnpc;

  if (scene)
  {
    scene->execAllOfScene_npc(COUNTRY_SEC_GEN,totalnpc);
    if (totalnpc.count==0)
      return true;
  }

  return false;
}

bool CountryDareM::isAttackMainFlag(Scene* scene,DWORD dwNpcID)
{
  TotalAllNpc totalnpc;
  if (scene)
  {
    scene->execAllOfScene_npc(dwNpcID,totalnpc);
    if (totalnpc.count==0)
      return true;
  }

  return false;
}

void CountryDareM::updateAttGenAttackTime(DWORD countryid,time_t uptime)
{
  for (DWORD i=0; i<dares.size(); i++)
  {
    if (dares[i].id == countryid)
    {
      rwlock.wrlock();
      dares[i].last_att_gen_attack_time = uptime;
      rwlock.unlock();
    }
  }
}

void CountryDareM::updateKingAttackTime(DWORD countryid,time_t uptime)
{
  for (DWORD i=0; i<dares.size(); i++)
  {
    if (dares[i].id == countryid)
    {
      rwlock.wrlock();
      dares[i].last_king_attack_time = uptime;
      rwlock.unlock();
    }
  }
}
void CountryDareM::updateAttackTime(DWORD countryid,time_t uptime)
{
  for (DWORD i=0; i<dares.size(); i++)
  {
    if (dares[i].id == countryid)
    {
      rwlock.wrlock();
      dares[i].last_attack_time = uptime;
      rwlock.unlock();
    }
  }
}
void CountryDareM::updateGenAttackTime(DWORD countryid,time_t uptime)
{
  for (DWORD i=0; i<dares.size(); i++)
  {
    if (dares[i].id == countryid)
    {
      rwlock.wrlock();
      dares[i].last_gen_attack_time = uptime;
      rwlock.unlock();
    }
  }
}

void CountryDareM::reliveSecondFlag(Scene* scene,DWORD dwNpcID)
{
  ReliveSpecNpc relivenpc;
  if (scene)
    scene->execAllOfScene_npc(dwNpcID,relivenpc);
}

void CountryDareM::delaySecondFlag(Scene* scene)
{
  DelaySpecNpc delaynpc;
  if (scene)
    scene->execAllOfScene_npc(COUNTRY_SEC_FLAG,delaynpc);

}


void CountryDareM::init()
{
  rwlock.wrlock();
  
  if (!dares.empty())
    return;

  for (SceneManager::CountryMap_iter iter=SceneManager::getInstance().country_info.begin(); iter!=SceneManager::getInstance().country_info.end(); iter++)
  {
    CountryDare temp;
    temp.id = iter->second.id;

    dares.push_back(temp);
  }
  
  rwlock.unlock();
}

