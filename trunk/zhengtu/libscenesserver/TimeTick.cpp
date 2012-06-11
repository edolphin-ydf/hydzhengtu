/**
 * \brief ʱ��ص�����
 */
//#include <zebra/ScenesServer.h>
#include "scriptTickTask.h"
#include "duplicateManager.h"

#define MAX_CMD_GROUP 10
zRTime SceneTimeTick::currentTime;
SceneTimeTick *SceneTimeTick::instance = NULL;

/**
 * \brief ��ͼ�ص�����
 */
struct EverySceneEntryAction : public SceneCallBack
{
  const DWORD group;
  EverySceneEntryAction(const DWORD group) : group(group) {}
  bool exec(Scene *scene)
  {
    if (scene)
    {
      if (scene->SceneEntryAction(SceneTimeTick::currentTime,group))
      {
        /**
         * ���������Ҫ��̬����ж�ص�ͼ������Ŷ
         * ^-^ ��������,���ڵ��߳�,����������û����,ֻ��Ҫע������������ĵ�����
         */
        SceneTaskManager::getInstance().execEvery();
      }
    }
    return true;
  }
};

/**
 * \brief ������npc�ӵ�ai�����б���
 */
/*class AddSpecialNpcCallBack : public specialNpcCallBack
{
  private:
    MonkeyNpcs &affectNpc;
    const DWORD group;
    const bool _every;
  public:
    AddSpecialNpcCallBack(MonkeyNpcs &affectNpc,const DWORD g,const bool every):affectNpc(affectNpc),group(g),_every(every){}
    bool exec(SceneNpc * npc)
    {
      if (_every || npc->tempid%MAX_NPC_GROUP==group)
      {
        affectNpc.insert(npc);
      }
      return true;
    }
};*/

/**
 * \brief ʱ��ѭ��,���Ͷ�ʱ�¼�,������Ӱ��npc ai��
 */
void SceneTimeTick::run()
{
  const int timeout_value = 500;
  const int timeout_value2 = 300;
  DWORD step = 0;
  int t = 0;
  while(!isFinal())
  {
    zThread::msleep((10-t)>0?(10-t):1);
    //��ȡ��ǰʱ��
    currentTime.now();

	//sky һ���ʱ��ѭ���������鴦��Roll����
	if(_one_sec(currentTime)) {
		SceneManager::getInstance().TeamRollItme();
	}

    if (_five_sec(currentTime)) {
      OnTimer event(1);
      EventTable::instance().execute(event);
      ScenesService::getInstance().checkAndReloadConfig();
    }
    sessionClient->doCmd();
    recordClient->doCmd();
    SceneTaskManager::getInstance().execEvery();

    //specialNpc
    //MonkeyNpcs affectNpc;
    //AddSpecialNpcCallBack asncb(affectNpc,step,t > timeout_value2);
    //SceneNpcManager::getMe().execAllSpecialNpc(asncb);
    SceneNpc::AI(currentTime,SceneNpcManager::getMe().getSepcialNpc(),step,t > timeout_value2);

    //250 usec
    EverySceneEntryAction esea(step);
    //�����е�ͼ���ûص�����
    SceneManager::getInstance().execEveryScene(esea);

#if 0
    if (0==step)
    {
      //20-25 usec
      sessionClient->doCmd();
      recordClient->doCmd();
      SceneTaskManager::getInstance().execEvery();
    }
#endif

    if (_one_min(currentTime))
    {//��������,һ�����ж�һ��ȫ������
      CountryDareM::getMe().timer();

      if (Zebra::global["world_quiz"] == "true")
      {
        struct tm tv1;
        time_t timValue = time(NULL);
        zRTime::getLocalTime(tv1,timValue);

        if (tv1.tm_hour==19)
        {
          if (tv1.tm_min==5 || tv1.tm_min == 10 || tv1.tm_min==15)
          {
            for (SceneManager::CountryMap_iter iter=SceneManager::getInstance().
                country_info.begin(); iter!=SceneManager::getInstance().country_info.end(); 
                iter++)
            {       
              Cmd::Session::t_countryNotify_SceneSession send;
              bzero(send.info,sizeof(send.info));
              sprintf(send.info,"%d ���Ӻ�ٰ���������",abs(20-tv1.tm_min));
              send.dwCountryID = iter->second.id;
              sessionClient->sendCmd(&send,sizeof(send));  
            }
          }

          if (!quiz && tv1.tm_min>=20 && tv1.tm_min<25)
          {
            quiz = true;
            Cmd::Session::t_createQuiz_SceneSession send;

            send.active_time  = 30;
            send.ready_time   = 1;
            send.type = 0; 
            send.subject_type = 0;
            sessionClient->sendCmd(&send,sizeof(send));  
          }
        }

        if (tv1.tm_hour==12)
        {
          if (tv1.tm_min==5 || tv1.tm_min==10 || tv1.tm_min == 15)
          {
            for (SceneManager::CountryMap_iter iter=SceneManager::getInstance().
                country_info.begin(); iter!=SceneManager::getInstance().country_info.end(); 
                iter++)
            {       
              Cmd::Session::t_countryNotify_SceneSession send;
              bzero(send.info,sizeof(send.info));
              sprintf(send.info,"%d ���Ӻ�ٰ���������",abs(20-tv1.tm_min));
              send.dwCountryID = iter->second.id;
              sessionClient->sendCmd(&send,sizeof(send));  
            }
          }

          if (!quiz && tv1.tm_min>=20 && tv1.tm_min<25)
          {
            quiz = true;
            Cmd::Session::t_createQuiz_SceneSession send;

            send.active_time  = 30;
            send.ready_time   = 1;
            send.type = 0; 
            send.subject_type = 0;
            sessionClient->sendCmd(&send,sizeof(send));  
          }
        }

#if 0
        if (tv1.tm_hour==22)
        {
          if (tv1.tm_min==35 || tv1.tm_min==40 || tv1.tm_min == 45)
          {
            for (SceneManager::CountryMap_iter iter=SceneManager::getInstance().
                country_info.begin(); iter!=SceneManager::getInstance().country_info.end(); 
                iter++)
            {       
              Cmd::Session::t_countryNotify_SceneSession send;
              bzero(send.info,sizeof(send.info));
              sprintf(send.info,"%d ���Ӻ�ٰ���������",abs(50-tv1.tm_min));
              send.dwCountryID = iter->second.id;
              sessionClient->sendCmd(&send,sizeof(send));  
            }
          }

          if (!quiz && tv1.tm_min>=50 && tv1.tm_min<55)
          {
            quiz = true;
            Cmd::Session::t_createQuiz_SceneSession send;

            send.active_time  = 30;
            send.ready_time   = 1;
            send.type = 0; 
            send.subject_type = 0;
            sessionClient->sendCmd(&send,sizeof(send));  
          }
        }
#endif

        if (tv1.tm_hour==13 || tv1.tm_hour==20 || tv1.tm_hour==0)
        {
          quiz = false;
        }
      }

      //ˢ������ȫ�ֱ���
      if (GlobalVar::server_id()) { //ugly,TO BE FIXED

        ALLVARS(update);
        ALLVARS(save);
      }

      SceneManager::getInstance().checkUnloadOneScene();
    }

    step = (++step) % MAX_NPC_GROUP;

    zRTime e;
    t = currentTime.elapse(e);
    if (t > timeout_value)
    {
      Zebra::logger->debug("---------- 1��ѭ����ʱ %u ����----------",t);
    }

	scriptTaskManagement::getInstance().execAll();

	duplicateManager::getInstance().doClear();
	/*scriptTaskManagement::iterator it = _tasklist->begin();
	scriptTaskManagement::iterator end = _tasklist->end();
	for( ; it != end; ++it)
	{
		time_t t = (time(NULL) - it->second->lastTime);
		if(it->second->doTask(t))
		{
			it->second->lastTime = time(NULL);
		}
	}*/

  }
}
