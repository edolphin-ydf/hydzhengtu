/**
 * \brief �ʳ�����ս
 *
 * 
 */

#include <zebra/SessionServer.h>

CDareEmperor::CDareEmperor() : CDare(0,0)
{
}

CDareEmperor::CDareEmperor(DWORD active_time,DWORD ready_time) : CDare(active_time,ready_time)
{
}

CDareEmperor::~CDareEmperor()
{
}

void CDareEmperor::setSecondID(DWORD dwID)
{
  this->secondID = dwID;
}

void CDareEmperor::addFirstID(DWORD dwID)
{
}

bool CDareEmperor::isInvalid()
{
  return false;
}

void CDareEmperor::timer()
{
}

void CDareEmperor::setActiveState()
{
  DWORD dwMapID = (NEUTRAL_COUNTRY_ID<<16) + 134;
  Cmd::Session::t_setEmperorDare_SceneSession send;

  // ����״̬
  rwlock.wrlock();
  this->state = CDare::DARE_ACTIVE;
  rwlock.unlock();   

  send.byState = 1;
  send.dwDefCountryID = this->secondID;
  SceneSession *pScene = SceneSessionManager::getInstance()->getSceneByID(dwMapID);
  pScene->sendCmd(&send,sizeof(send));
  SessionChannel::sendAllInfo(Cmd::INFO_TYPE_EXP,"�ʳ�����ս���ڿ�ʼ��������׼����");
}

void CDareEmperor::setReadyOverState()
{
  DWORD dwMapID = (NEUTRAL_COUNTRY_ID<<16) + 134;
  Cmd::Session::t_setEmperorDare_SceneSession send;
  send.byState = 0;
  send.dwDefCountryID = this->secondID;
  SceneSession *pScene = SceneSessionManager::getInstance()->getSceneByID(dwMapID);
  pScene->sendCmd(&send,sizeof(send));

  if (this->dwWinnerID >0)
  {
    CCountry* pCountry = CCountryM::getMe().find(NEUTRAL_COUNTRY_ID);
    pCountry->changeEmperor(this->dwWinnerID);
  
    // ���������ʳ�ռ���߰��,��ΪӮ�ҹ��ҹ����������
    SessionChannel::sendAllInfo(Cmd::INFO_TYPE_EXP,"%s ��ûʳǿ���Ȩ",
        CCountryM::getMe().find(this->dwWinnerID)->name);
  }
  else
  {
    SessionChannel::sendAllInfo(Cmd::INFO_TYPE_EXP,"%s ����ԡѪ��ս�ɹ���ס�ʳ�",
        CCountryM::getMe().find(this->secondID)->name);
  }

  // ͨ���ս���
  this->setOverState();
}

