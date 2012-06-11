/**
 * \brief ʵ�ֻỰ��
 *
 * 
 */

#include <stdarg.h>

#include <zebra/SessionServer.h>

/**
 * \brief ���캯��
 * \param task �ûỰ������
 */
UserSession::UserSession(SessionTask *task):zUser(),Session(task)
{
	teamid = 0; //sky �Ѷ���ΨһID��ʼ��Ϊ0
	regTime = SessionTimeTick::currentTime;
	user_count++;
	accid=0;
	scene=NULL;
	face = 1;
	nextBlessTime.now();
	septExp =0;

	reqAdopter = 0;
}

/**
 * \brief ��������
 */
UserSession::~UserSession()
{
  Zebra::logger->info("�û�%s(%u)��Ϸʱ��:%u����",name,id,(regTime.elapse(SessionTimeTick::currentTime)/1000)/60); 
  UserSession::country_map[this->country]--;
  if ((int)UserSession::country_map[this->country] < 0)
  {
    UserSession::country_map.erase(this->country);
  }
  Zebra::logger->debug("SessionĿǰ��������:%u",--user_count);
  SessionChannelManager::getMe().removeUser(this);
}
/**
 * \brief �����Ϣ���ͣ�����ϵͳ���ý��й���
 * \param pstrCmd ��������Ϣ
 * \param nCmdLen ��Ϣ����
 * \return ����Ϣ�Ƿ�ͨ�����
 */
bool UserSession::checkChatCmd(const Cmd::stNullUserCmd *pstrCmd,const DWORD nCmdLen) const
{
  using namespace Cmd;
  switch (pstrCmd->byCmd)
  {
  case CHAT_USERCMD:
    {
      switch (pstrCmd->byParam)
      {
      case REQUEST_TEAM_USERCMD_PARA://�������
        {
          if (!isset_state(sysSetting,USER_SETTING_TEAM))
            return false;
        }
        break;
      default:
        break;
      }
    }
    break;
  case TRADE_USERCMD:
    {
      //������
      if (REQUEST_TRADE_USERCMD_PARAMETER==pstrCmd->byParam)
        if (!isset_state(sysSetting,USER_SETTING_TRADE))
          return false;
    }
    break;
  case SCHOOL_USERCMD://�������ʦ��
    {
      if (ADD_MEMBER_TO_SCHOOL_PARA==pstrCmd->byParam)
      {
        stAddMemberToSchoolCmd * rev = (stAddMemberToSchoolCmd *)pstrCmd;
        if (TEACHER_QUESTION==rev->byState)
          if (!isset_state(sysSetting,USER_SETTING_SCHOOL))
          {
            UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->memberName);
            if (pUser)
                pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"%s ����ʦ��δ����",name);
            return false;
          }
      }
    }
    break;
  case UNION_USERCMD://���������
    //by RAY
    //return false;
//#if 0
    {
      if (ADD_MEMBER_TO_UNION_PARA==pstrCmd->byParam)
        if (QUESTION==((stAddMemberToUnionCmd*)pstrCmd)->byState)
          if (!isset_state(sysSetting,USER_SETTING_UNION))
            return false;
    }
//#endif    
    break;
  case SEPT_USERCMD://����������
    //by RAY
    //return false;
//#if 0
    {
      if (ADD_MEMBER_TO_SEPT_PARA==pstrCmd->byParam)
        if (SEPT_QUESTION==((stAddMemberToSeptCmd*)pstrCmd)->byState)
          if (!isset_state(sysSetting,USER_SETTING_FAMILY))
            return false;
    }
//#endif
    break;
  case RELATION_USERCMD://�����Ϊ����
    //by RAY
    //return false;
//#if 0
        {
      if (RELATION_STATUS_PARA==pstrCmd->byParam)
      {
        stRelationStatusCmd * rev = (stRelationStatusCmd *)pstrCmd;
        if (RELATION_QUESTION==rev->byState)
          if (!isset_state(sysSetting,USER_SETTING_FRIEND))
          {
            UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->name);
            if (pUser)
                pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"%s ��Ӻ���δ����",name);
            return false;
          }
      }
    }
//#endif
    break;
  default:
    break;
  }
  return true;
}

/**
 * \brief �����ҷ�����Ϣ
 * \param pstrCmd �����͵���Ϣ
 * \param nCmdLen ��Ϣ����
 */
void UserSession::sendCmdToMe(const void *pstrCmd,const DWORD nCmdLen) const 
{
  using namespace Cmd::Session;
  using namespace Cmd;

  if (!checkChatCmd((stNullUserCmd *)pstrCmd,nCmdLen)) return;

  BYTE buf[zSocket::MAX_DATASIZE];
  t_Session_ForwardUser *scmd=(t_Session_ForwardUser *)buf;
  constructInPlace(scmd);
  scmd->dwID=id;
  scmd->size=nCmdLen;
  bcopy(pstrCmd,scmd->data,nCmdLen,sizeof(buf) - sizeof(t_Session_ForwardUser));
  sendCmd(scmd,sizeof(t_Session_ForwardUser)+nCmdLen);
}

#define getMessage(msg,msglen,pat)  \
do  \
{  \
  va_list ap;  \
  bzero(msg,msglen);  \
  va_start(ap,pat);    \
  vsnprintf(msg,msglen - 1,pat,ap);  \
  va_end(ap);  \
}while(false)

/**
 * \brief ����ҷ���ϵͳ������Ϣ
 * \param type ϵͳ��Ϣ����
 * \param pattern ����
 */
void UserSession::sendSysChat(int type,const char *pattern,...) const
{
  char buf[MAX_CHATINFO];

  getMessage(buf,MAX_CHATINFO,pattern);
  Cmd::stChannelChatUserCmd send;
  send.dwType=Cmd::CHAT_TYPE_SYSTEM;
  send.dwSysInfoType = type;
  bzero(send.pstrName,sizeof(send.pstrName));
  bzero(send.pstrChat,sizeof(send.pstrChat));
  strncpy((char *)send.pstrChat,buf,MAX_CHATINFO-1);
  sendCmdToMe(&send,sizeof(send));
}

/**
 * \brief ����ҷ���GM������Ϣ
 * \param type ϵͳ��Ϣ����
 * \param pattern ����
 */
void UserSession::sendGmChat(int type,const char *pattern,...) const
{
  char buf[MAX_CHATINFO];

  getMessage(buf,MAX_CHATINFO,pattern);
  Cmd::stChannelChatUserCmd send;
  send.dwType=Cmd::CHAT_TYPE_GM;
  send.dwSysInfoType = type;
  bzero(send.pstrName,sizeof(send.pstrName));
  bzero(send.pstrChat,sizeof(send.pstrChat));
  strncpy((char *)send.pstrChat,buf,MAX_CHATINFO-1);
  sendCmdToMe(&send,sizeof(send));
}

/**
 * \brief ������Ϣ������ҵ��Ѻö�
 * \param rev ����Ѻö���Ϣ����Ϣ
 */
void UserSession::setFriendDegree(Cmd::Session::t_CountFriendDegree_SceneSession *rev)
{
  relationManager.setFriendDegree(rev);
}

/**
 * \brief �򳡾����ͺ��ѹ�ϵ��Ϣ
 * �������Ϣ���������б�������Һ���Щ���ֵ��Ѻö�
 * \param rev ���������б����Ϣ
 */
void UserSession::sendFriendDegree(Cmd::Session::t_RequestFriendDegree_SceneSession *rev)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  Cmd::Session::stRequestReturnMember * temp = NULL;

  Cmd::Session::t_ReturnFriendDegree_SceneSession *retCmd=(Cmd::Session::t_ReturnFriendDegree_SceneSession *)buf;
  constructInPlace(retCmd);
  retCmd->size = 0;
  retCmd->dwID = id;
  retCmd->dwMapTempID = scene->tempid;
  temp = retCmd->memberlist;

  CSchoolMember *me = CSchoolM::getMe().getMember(name);

  for (int j=0; j< rev->size; j++)
  {
    CRelation *rel = NULL;
    rel = relationManager.getRelationByName(rev->namelist[j].name);
    CSchoolMember *member = CSchoolM::getMe().getMember(rev->namelist[j].name);

    if (NULL != rel)
    {
      temp->dwUserID = rel->id;
      switch(rel->type)
      {
        case Cmd::RELATION_TYPE_LOVE:
          temp->byType = Cmd::Session::TYPE_CONSORT;
          break;
        case Cmd::RELATION_TYPE_FRIEND:
          temp->byType = Cmd::Session::TYPE_FRIEND;
          break;
        default:
          Zebra::logger->error("���ͺ��ѹ�ϵ�в���ʶ������� %u",rel->type);
          break;
      }
      temp->wdDegree = rel->level;
      retCmd->size++;
      temp++;
    }

    if (member && me)
    {
      if (member->getTeacher() == me)
      {
        temp->dwUserID = member->getCharID();
        temp->byType = Cmd::Session::TYPE_PRENTICE;
        temp->wdDegree = member->getDegree();
        retCmd->size++;
        temp++;
      }
      else if (me->getTeacher() == member)
      {
        temp->dwUserID = member->getCharID();
        temp->byType = Cmd::Session::TYPE_TEACHER;
        temp->wdDegree = me->getDegree();
        retCmd->size++;
        temp++;
      }
    }
  }

  scene->sendCmd(retCmd,sizeof(Cmd::Session::t_ReturnFriendDegree_SceneSession)+retCmd->size*sizeof(Cmd::Session::stRequestReturnMember));
}

/**
 * \brief ��sceneת����Ϣ
 * \param pNullCmd ��ת������Ϣ
 * \param nCmdLen ��Ϣ����
 * \return ת���Ƿ�ɹ�
 */
bool UserSession::forwardScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
  
  if (scene)
  {
    Zebra::logger->error("����forwardScene");
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::Scene::t_Scene_ForwardScene *sendCmd=(Cmd::Scene::t_Scene_ForwardScene *)buf;
    constructInPlace(sendCmd);
    sendCmd->dwUserID=id;
    sendCmd->dwSceneTempID=scene->tempid;;
    sendCmd->size=nCmdLen;
    bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Scene::t_Scene_ForwardScene));
    scene->sendCmd(buf,sizeof(Cmd::Scene::t_Scene_ForwardScene)+nCmdLen);
    //Zebra::logger->debug("ת��%ld����Ϣ��%ld����",pUser->id,pUser->sceneTempID);
    return true;
  }
  return false;
}

void UserSession::updateConsort()
{
  CRelation* relation = NULL;
  relation = relationManager.getMarryRelation();
  Cmd::Session::t_updateConsort send;
  
  if (relation)
  {
    send.dwUserID = id;
    send.dwConsort = relation->charid;
    CCountry* pCountry = CCountryM::getMe().find(this->country);
    if (pCountry && strncmp(pCountry->kingName,relation->name,MAX_NAMESIZE) == 0)
    {
      CCountry* c = CCountryM::getMe().find(NEUTRAL_COUNTRY_ID);
      if (c && strncmp(c->kingName,relation->name,MAX_NAMESIZE) == 0)
      {
        send.byKingConsort = 2;
      }
      else
      {
        send.byKingConsort = 1;
      }
    }
  }
  else
  {
    send.dwUserID = id;
    send.dwConsort = 0;
  }

  if (this->scene)
  {
    scene->sendCmd(&send,sizeof(send));
  }
}

void UserSession::updateCountryStar()
{
  Cmd::Session::t_updateCountryStar send;
  send.dwUserID = id;
  CCountry* pCountry = CCountryM::getMe().find(this->country);

  if (pCountry)
  {
    send.dwCountryStar = pCountry->getStar();
  }
  else
  {
    send.dwCountryStar = 0;
  }

  if (this->scene && send.dwCountryStar>0)
  {
    scene->sendCmd(&send,sizeof(send));
  }
}
