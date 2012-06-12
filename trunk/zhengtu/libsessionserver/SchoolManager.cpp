/**
 * \brief ʵ��ʦ�����ɹ�����
 *
 * 
 */

#include <zebra/SessionServer.h>


/**
 * \brief ȡ��ʦ�ų�Ա�Ļص�����
 * \param member ��Աָ��
 */
void CSchoolMemberListCallback::exec(CSchoolMember *member)
{
  memberList.push_back(member);
}

/**
 * \brief ɾ����Ա���������������Ա�Ĺ�ϵ
 */
void CSchoolMemberListCallback::clearInValidNodeRelation()
{
  std::list<CSchoolMember *>::iterator tIterator;

  for(tIterator = memberList.begin(); tIterator != memberList.end(); tIterator++)
  {
    CSchoolMember *member = (*tIterator);
    if (member)
    {
      if (!member->isValid()||member->isClean())
      {
        CSchoolMember *preNode = member->getPreLevelNode();
        if (preNode) preNode->directRemovePrentice(member);
        member->clearAllNextLevelRelation();
      }
    }
  }
}

/**
 * \brief ��һ����Ա����������Ա����Ϣ
 */
void CSchoolMemberListCallback::sendNotifyToMember()
{
  std::list<CSchoolMember *>::iterator tIterator;

  clearInValidNodeRelation();

  for(tIterator = memberList.begin(); tIterator != memberList.end(); tIterator++)
  {
    if ((*tIterator))
    {
      if (!(*tIterator)->isValid()||(*tIterator)->isClean())
      {
        (*tIterator)->deleteMe();
      }
      else
      {
        (*tIterator)->updateRecord();
        (*tIterator)->sendInfomationToMe();
      }
    }
  }
}

/**
 * \brief ��Ҫ����ĳ�Ա��ӽ��б�
 * sendNotifyToMember��clearInValidNodeRelation�����б���ĳ�Ա
 * \param member ��Աָ��
 * \param tag ��־
 */
void CSendSchoolCallback::exec(CSchoolMember *member,const BYTE tag)
{
  Cmd::stSchoolMember temp;

  strncpy(temp.name,member->name,MAX_NAMESIZE);
  temp.tag = tag;
  temp.online = member->isOnline();
  temp.occupation = member->getOccupation();
  temp.level = member->getLevel();
  memberList.push_back(temp);
  //  Zebra::logger->debug("tag = [%u] name = [%s] online = [%u]",temp.tag,temp.name,temp.online);
}

/**
 * \brief ���Ա���ͳ�Ա�б�
 *
 * \param member ��Աָ��
 */
void CSendSchoolCallback::sendListToMember(CSchoolMember *member)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  DWORD count;
  Cmd::stSchoolMember *tempPoint;
  std::list<struct Cmd::stSchoolMember>::iterator tIterator;

  Cmd::stSendListSchoolCmd *retCmd=(Cmd::stSendListSchoolCmd *)buf;
  constructInPlace(retCmd);
  tempPoint = (Cmd::stSchoolMember *)retCmd->list;
  count = 0;
  retCmd->byPackageTag=Cmd::SCHOOL_PACKAGE_HEAD;

  CSchool *tempSchool = member->getSchool();
  DWORD masterid = 0;
  if (tempSchool)   masterid = tempSchool->getMasterNode()->getCharID();
  if (member->getCharID() == masterid)
  {
    UserSession *master = NULL;
    master = UserSessionManager::getInstance()->getUserByID(masterid);
    if (master)
      retCmd->dwMasterTempID = master->tempid;
    else
      retCmd->dwMasterTempID = 0;
  }
  else
  {
    retCmd->dwMasterTempID = 0;
  }

  tIterator = memberList.begin();
  while(tIterator != memberList.end())
  {
    if (count > 1500)
    {
      retCmd->size = count;
      member->sendCmdToMe(retCmd,(count*sizeof(Cmd::stSchoolMember)+sizeof(Cmd::stSendListSchoolCmd)));

      tempPoint = (Cmd::stSchoolMember *)retCmd->list;
      count = 0;
      retCmd->byPackageTag=Cmd::SCHOOL_PACKAGE_BODY;
    }
    strncpy(tempPoint->name,tIterator->name,MAX_NAMESIZE);
    tempPoint->tag = tIterator->tag;
    tempPoint->online = tIterator->online;
    tempPoint->occupation = tIterator->occupation;
    tempPoint->level  = tIterator->level;

    count++;
    tempPoint++;
    tIterator++;
  }

  if (count>0)
  {
    if (retCmd->byPackageTag == Cmd::SCHOOL_PACKAGE_HEAD)
    {
      retCmd->byPackageTag=Cmd::SCHOOL_PACKAGE_HEAD|Cmd::SCHOOL_PACKAGE_TAIL;
    }
    else
    {
      retCmd->byPackageTag = Cmd::SCHOOL_PACKAGE_TAIL;
    }
    retCmd->size = count;
    member->sendCmdToMe(retCmd,(count*sizeof(Cmd::stSchoolMember)+sizeof(Cmd::stSendListSchoolCmd)));
  }
}

//---------------------------------------------------------------------

/**
 * \brief ���ɹ��������캯��
 */
CSchoolM *CSchoolM::sm(NULL);

/**
 * \brief ��������
 */
CSchoolM::~CSchoolM()
{
  rwlock.wrlock();
  for(zEntryID::hashmap::iterator it=zEntryID::ets.begin();it!=zEntryID::ets.end();it++)
  {
    CSchool *temp =(CSchool *)it->second;
    SAFE_DELETE(temp);
  }
  clear();
  rwlock.unlock();
}

/**
 * \brief ��ʼ��������
 * \return ��ʼ���Ƿ�ɹ�
 */
bool CSchoolM::init()
{
  CSchool *pSchool = new CSchool();
  if (pSchool)
  {
    pSchool->initToNoneSchool();
    rwlock.wrlock();
    addEntry(pSchool);
    rwlock.unlock();
  }
  else
  {
    Zebra::logger->error("��������ʦ�Ź���������");
    return false;
  }


  Zebra::logger->debug("loadSchoolFromDB()");
  if (!loadSchoolFromDB()) return false;
  Zebra::logger->debug("loadSchoolMemberFromDB()");
  if (!loadSchoolMemberFromDB()) return false;
  return true;
}

/**
 * \brief �õ�������ʵ��
 * \return ���ɹ�����ʵ��
 */
CSchoolM &CSchoolM::getMe()
{
  if (sm==NULL)
  {
    sm=new CSchoolM();
  }
  return *sm;
}

/**
 * \brief ɾ��������ʵ��
 */
void CSchoolM::destroyMe()
{
  SAFE_DELETE(sm);
}

/**
* \brief �ڳ�Ա����������ҳ�Ա����
* return ����ָ��
*/
std::map<std::string,CSchoolMember *>::iterator  CSchoolM::findMemberIndex(const char *pName)
{
        char temp_name[MAX_NAMESIZE];
        bzero(temp_name,MAX_NAMESIZE);
        strncpy(temp_name,pName,MAX_NAMESIZE);
        return memberIndex.find(temp_name);
}


/**
 * \brief �������ֵõ�����ָ��
 * \param name ��������
 * \return ����ָ��
 */
CSchool * CSchoolM::getSchoolByName( const char * name)
{
  rwlock.rdlock();
  CSchool *ret =(CSchool *)getEntryByName(name);
  rwlock.unlock();
  return ret;
}

/**
 * \brief �����ݿ��ȡ������Ϣ
 * \return �Ƿ��ȡ�ɹ�
 */
bool CSchoolM::loadSchoolFromDB()
{
  const dbCol school_read_define[] = {
    { "SCHOOLID",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "NAME",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "MASTERSERIAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL,0,0}
  };

  stSchoolInfo *schoolList,*tempPoint;
  char *point;

  schoolList = NULL;
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`SCHOOL`",school_read_define,NULL,NULL,(BYTE **)&schoolList);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode
      || 0 == retcode)
  {
    return true;
  }

  if (schoolList)
  {
    tempPoint = &schoolList[0];
    point = (char *)&schoolList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      if (!createSchoolFromDB(*tempPoint))
      {
        Zebra::logger->error("�޷��������ɼ�¼�������ɹ�����");
        SAFE_DELETE_VEC(schoolList);
        return false;
      }
      point +=sizeof(stSchoolInfo)+tempPoint->size;//MAX_BULLETIN_CHAR_NUMBER;
      tempPoint = (stSchoolInfo *)point;
    }
    SAFE_DELETE_VEC(schoolList);
    return true;
  }
  else
  {
    Zebra::logger->error("�������ݳ�ʼ��ʧ��,exeSelect ������Чbufָ��");
  }
  return false;
}

/**
 * \brief �����ݿ��ȡ���ɳ�Ա
 * \return �Ƿ�ɹ�
 */
bool CSchoolM::loadSchoolMemberFromDB()
{
  bool ret = true;

  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSchool *temp =(CSchool *)it->second;
    if (!temp->loadSchoolMemberFromDB()) ret = false;
  }
  return ret;
}

/**
 * \brief �������ݿ������Ϣ�������ɶ���
 * \param info ������Ϣ
 * \return  �Ƿ�ɹ�
 */
bool CSchoolM::createSchoolFromDB(const stSchoolInfo &info)
{
  CSchool *pSchool = new CSchool();
  if (pSchool)
  {
    //    Zebra::logger->debug("�������ɶ���id=[%u]name=[%s]",info.dwSchoolID,info.name);
    pSchool->initSchool(info);
    rwlock.wrlock();
    addEntry(pSchool);
    rwlock.unlock();
    return true;
  }
  else
  {
    Zebra::logger->error("�������ɶ����ʱ���޷������ڴ�");
    return false;
  }
}

/**
 * \brief �����Ա����
 *
 * \param pUser ���ߵĳ�Ա
 */
void CSchoolM::userOnline(UserSession *pUser)
{
  std::map <std::string,CSchoolMember*>::iterator sIterator;

  rwlock.wrlock();
  sIterator = findMemberIndex(pUser->name);
  rwlock.unlock();
  if (sIterator != memberIndex.end())
  {
    if (sIterator->second) sIterator->second->online(pUser);
  }
  else
  {
    Cmd::stMemberStatusSchool ret;
    ret.byStatus = Cmd::SCHOOL_STATUS_NONE;
    pUser->sendCmdToMe(&ret,sizeof(ret));
    if (pUser->schoolid != 0)
    {
      pUser->schoolid = 0;
      Cmd::Session::t_sendUserRelationID send;
      send.dwUserID = pUser->id;
      send.caption = 0;
      send.dwID = 0;
      send.type = Cmd::Session::RELATION_TYPE_SCHOOL;
      pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
    }
  }
}

/**
 * \brief �����ɫ����,�������ɷ�����Ϣ
 *
 * \param pUser ���ߵĳ�Ա
 */
void CSchoolM::userOffline(UserSession *pUser)
{
  std::map <std::string,CSchoolMember*>::iterator sIterator;

  rwlock.wrlock();
  sIterator = findMemberIndex(pUser->name);
  rwlock.unlock();
  if (sIterator != memberIndex.end())
  {
    if (sIterator->second) sIterator->second->offline();
  }
}

/**
 * \brief ���һ�����ɳ�Ա����
 * \param pName ��������
 * \param pSchoolMember ��Աָ��
 * \return �Ƿ�ɹ�
 */
bool CSchoolM::addMemberIndex(const char *pName,CSchoolMember *pSchoolMember)
{
  std::pair<std::map<std::string,CSchoolMember*>::iterator,bool> retval;
        char temp_name[MAX_NAMESIZE];
        bzero(temp_name,MAX_NAMESIZE);
        strncpy(temp_name,pName,MAX_NAMESIZE);

  rwlock.wrlock();
  retval = memberIndex.insert(memberIndexValueType(temp_name,pSchoolMember));
  rwlock.unlock();
  return retval.second;
}

/**
 * \brief ��ӳ����ɾ��һ����Ա 
 * \param pName Ҫɾ��������
 * \return ɾ���Ƿ�ɹ� 
 */
bool CSchoolM::removeMemberIndex(const char *pName)
{
  bool ret;
  std::map <std::string,CSchoolMember*>::iterator sIterator;
  rwlock.wrlock();
  ret = true;
  sIterator = findMemberIndex(pName);
  if (sIterator != memberIndex.end()) memberIndex.erase(sIterator);
  else ret = false;
  rwlock.unlock();
  return ret;
}

/**
 * \brief ����������ص���Ϣ
 *
 * \param pUser ��һỰ����ָ�� 
 * \param pNullCmd ����������Ϣ
 * \param cmdLen ��Ϣ����
 * \return �����Ƿ�ɹ�
 */
bool CSchoolM::processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  switch(pNullCmd->byCmd)
  {
    case Cmd::SCHOOL_USERCMD:
      {
        switch(pNullCmd->byParam)
        {
          case REQ_MASTER_BOUNTY_PARA:
            {
              CSchoolMember *member = (CSchoolMember *)getMember(pUser->name);
              
              if (!member)
                return true;
              
              Cmd::stReqMasterBountyUserCmd* cmd = (Cmd::stReqMasterBountyUserCmd*)pNullCmd;

              if (cmd->byState == Cmd::QUERY_SCHOOL_BOUNTY)
              {
                Cmd::stRtnMasterBountyUserCmd send;
                send.dwBalance = member->master_balance;
                send.dwTotal = member->master_total;
                pUser->sendCmdToMe(&send,sizeof(send));
              }
              else if (cmd->byState == Cmd::GET_SCHOOL_BOUNTY)
              {
                Cmd::Session::t_PickupMaster_SceneSession send;

                if (member->master_balance>0)
                {
                  send.dwUserID = pUser->id;
                  send.dwMoney = member->master_balance;
                  pUser->scene->sendCmd(&send,sizeof(send));
                  member->master_total += member->master_balance;
                  member->master_balance = 0;
                  member->updateRecord();
                }
              }

              return true;
            }
            break;
          case REQ_PRENTICE_BOUNTY_PARA:
            {
              CSchoolMember *member = (CSchoolMember *)getMember(pUser->name);
              if (!member || !member->getTeacher())
                return true;
              
              Cmd::stReqPrenticeBountyUserCmd* cmd = 
                (Cmd::stReqPrenticeBountyUserCmd*)pNullCmd;

              if (cmd->byState == Cmd::QUERY_SCHOOL_BOUNTY)
              {
                Cmd::stRtnPrenticeBountyUserCmd send;
                send.dwBalance = member->queryBounty();
                send.dwTotal = member->prentice_total;
                send.dwLastLevel = member->prentice_total;
                strncpy(send.name,member->getTeacher()->name,MAX_NAMESIZE);
                pUser->sendCmdToMe(&send,sizeof(send));
              }
              else if (cmd->byState == Cmd::PUT_SCHOOL_BOUNTY)
              {
                if (member->putBounty())
                {
                  pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����ɹ�");
                }
              }
              
              return true;
            }
            break;
          case Cmd::ADD_MEMBER_TO_SCHOOL_PARA: // ����ͽ��
            {
              Cmd::stAddMemberToSchoolCmd *ptCmd=(Cmd::stAddMemberToSchoolCmd *)pNullCmd;
              switch(ptCmd->byState)
              {
                case Cmd::TEACHER_QUESTION:
                  {
                    if (getUserPrenticeInfo(pUser->name,ptCmd->schoolName))
                    {
                      UserSession *otherUser = NULL;
                      otherUser = UserSessionManager::getInstance()->getUserSessionByName(ptCmd->memberName);
                      if (otherUser)
                      {
                        if (!isset_state(otherUser->sysSetting,Cmd::USER_SETTING_SCHOOL))
                        {
                          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է�����ʦ��δ����");
                          return true;
                        }
                        if (otherUser->level >=10)
                        {
                          if (isClean(otherUser->name))
                          {
                            ptCmd->memberID = pUser->tempid;
                            strncpy(ptCmd->memberName,pUser->name,MAX_NAMESIZE);
                            otherUser->sendCmdToMe(ptCmd,cmdLen);
                          }
                          else
                          {
                            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻������Ϊͽ,�����Ѿ�����ʦͽ��ϵ");
                          }
                        }
                        else
                        {
                          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻������Ϊͽ,����û��10����");
                        }
                      }
                      else
                      {
                        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��Ҳ�����,�޷���Ӧ����");
                      }
                    }
                    else
                    {
                      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��������,������������");
                    }
                    return true;
                  }
                  break;
                case Cmd::TEACHER_ANSWER_YES:
                  {
                    UserSession *master = NULL;
                    master = UserSessionManager::getInstance()->getUserByTempID(ptCmd->memberID);
                    if (isClean(pUser->name))
                    {
                      if (master)
                      {
                        Zebra::logger->info("[ʦͽ]�յ�%s����%s��ָ��",master->name,pUser->name);
                        addMember(master,pUser);
                      }
                    }
                    else
                    {
                      if (master) master->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻������Ϊͽ,�����Ѿ�����ʦͽ��ϵ");
                    }
                    return true;
                  }
                  break;
                case Cmd::TEACHER_ANSWER_NO:
                  {
                    UserSession *master = NULL;
                    master = UserSessionManager::getInstance()->getUserByTempID(ptCmd->memberID);
                    if (master) master->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ܾ���������飡");
                    return true;
                  }
                  break;
                default:
                  break;
              }
              return true;
            }
            break;
          case Cmd::FIRE_MEMBER_FROM_SCHOOL_PARA: //����ĳ��
            {
              Cmd::stFireMemberFromSchoolCmd *ptCmd=(Cmd::stFireMemberFromSchoolCmd *)pNullCmd;
              Zebra::logger->info("[ʦͽ]�յ�%s����%s��ָ��",pUser->name,ptCmd->memberName);
              frieMember(pUser,ptCmd->memberName);
              return true;
            }
            break;
          case Cmd::SCHOOL_STATUS_CHECK_PARA: //���ɴ���ǰ���������
            {
              if (checkSchoolCreateCondition(pUser))
              {
                Cmd::stSchoolStatusCheckCmd ret;
                pUser->sendCmdToMe(&ret,sizeof(ret));
              }
              return true;
            }
            break;
          case Cmd::CREATE_SCHOOL_PARA: // ���ɴ���
            {
              Cmd::stCreateSchoolCmd *ptCmd=(Cmd::stCreateSchoolCmd *)pNullCmd;
              if (checkSchoolCreateCondition(pUser))
              {
                if (createNewSchool(pUser->name,ptCmd->SchoolName))
                {
                  Cmd::Session::t_SchoolCreateSuccess_SceneSession send;
                  send.dwID = pUser->id;
                  send.dwSchoolID = pUser->schoolid;
                  strncpy(send.schoolName,ptCmd->SchoolName,MAX_NAMESIZE);
                  pUser->scene->sendCmd(&send,sizeof(send));
                }
                else
                {
                  pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�ź���֪ͨ�����������ѱ�ռ��,�뻻������������!");
                }
              }
              return true;
            }
            break;
          case Cmd::SCHOOL_BULLETIN__PARA: // ���ɹ���
            {
              Cmd::stSchoolBulletinCmd *ptCmd=(Cmd::stSchoolBulletinCmd *)pNullCmd;
              processBulletin(pUser,ptCmd);
              return true;
            }
            break;
          case Cmd::SCHOOLMEMBER_LEAVE_SCHOOL_PARA: // ��Ա���������ϵ
            {
              processLeaveGroup(pUser);
              return true;
            }
            break;
          case Cmd::DESTROY_SCHOOL_PARA: // ��ɢ����
            {
              if (!destroySchool(pUser->name))
              {
                pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ɢ��Ч!");
              }
              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
  return false;
}

/**
 * \brief ������������Ϣ
 * һ��ʧ��
 *
 * \param pNullCmd ����������Ϣ
 * \param cmdLen ��Ϣ����
 * \return �����Ƿ�ɹ�
 */
bool CSchoolM::processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
  return false;
}

/**
 * \brief Ϊ��ҷ���ʦ��������Ϣ
 *
 * \param pUser ����Ϣ�����
 * \param rev ��Ϣ
 * \param cmdLen ��Ϣ����
 * \return 
 */
void CSchoolM::sendSchoolChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen)
{
  CSchoolMember *member = getMember(pUser->name);
  if (member) 
    member->sendChatMessages(rev,cmdLen);
  else
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"û�м���ʦ�Ż������ɲ���ʹ�ô�Ƶ��");
}

/**
 * \brief Ϊ��ҷ���ʦ��˽����Ϣ
 *
 * \param pUser ����Ϣ�����
 * \param rev ��Ϣ
 * \param cmdLen ��Ϣ����
 */
void CSchoolM::sendSchoolPrivateChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen)
{
  rwlock.rdlock();

  CSchoolMember *member = (CSchoolMember *)getMember(rev->pstrName);
  if (member)
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stChannelChatUserCmd *chatCmd;

    chatCmd = (Cmd::stChannelChatUserCmd *)buf;
    memccpy(chatCmd,rev,cmdLen,sizeof(buf));
    strncpy(chatCmd->pstrName,pUser->name,MAX_NAMESIZE);

    if (member->getUser())
    {
      member->getUser()->sendCmdToMe(chatCmd,cmdLen);
    }
    else
    {
      COfflineMessage::writeOfflineMessage(chatCmd->dwType,member->id,chatCmd,cmdLen);
    }
  }
  rwlock.unlock();
}

/**
 * \brief ��������뿪����
 *
 * \param pUser �������ɵ����
 */
void CSchoolM::processLeaveGroup(UserSession *pUser)
{
  CSchoolMember *member = NULL;
  member = getMember(pUser->name);
  if (member)
  {
    CSchool *school = NULL;
    school = member->getSchool();
    if (school)
    {
      CSchoolMember *teacher = member->getTeacher();
      if (school->processLeaveSchool(member))
      {
        if (school->id >0)
        {
          Cmd::stChannelChatUserCmd send;
          send.dwType = Cmd::CHAT_TYPE_SYSTEM;
          send.dwChannelID = 0;
          strncpy(send.pstrName,pUser->name,MAX_NAMESIZE);
          _snprintf(send.pstrChat,sizeof(send.pstrChat) - 1,"%sѡ�����뿪����",pUser->name);
          school->sendCmdToSchool(&send,sizeof(send));

          /// ��ʼ�����ع�ϵID
          pUser->schoolid = 0;
          Cmd::Session::t_sendUserRelationID sendrl;
          sendrl.dwUserID = pUser->id;
          sendrl.caption = 0;
          sendrl.dwID = 0;
          sendrl.type = Cmd::Session::RELATION_TYPE_SCHOOL;
          pUser->scene->sendCmd(&sendrl,sizeof(Cmd::Session::t_sendUserRelationID));
        }
        else
        {
          if (teacher) teacher->sendSysChat(Cmd::INFO_TYPE_FAIL,"%sѡ�����뿪ʦ��!",pUser->name);
        }
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻��ʹ�ô˹���,��Ǹ��");
      }
    }
    else
    {
      Zebra::logger->error("CSchoolM::processLeaveGroup(): %s��Ӧ�ڵ��޷�ȡ���ڵ�������Ķ���",member->name);
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲢û�м����κε�ʦ�Ż������ɰ�����");
  }

}

/**
 * \brief ��������뿪����
 *
 * \param roleName �뿪���ɵ��������
 */
void CSchoolM::processLeaveGroupDirect(const char* roleName)
{
  CSchoolMember *member = NULL;
  member = getMember(roleName);
  
  if (member)
  {
    CSchool *school = NULL;
    school = member->getSchool();
    if (school)
    {
      CSchoolMember *teacher = member->getTeacher();
      if (school->processLeaveSchool(member))
      {
        if (school->id >0)
        {
          Cmd::stChannelChatUserCmd send;
          send.dwType = Cmd::CHAT_TYPE_SYSTEM;
          send.dwChannelID = 0;
          strncpy(send.pstrName,roleName,MAX_NAMESIZE);
          _snprintf(send.pstrChat,sizeof(send.pstrChat) - 1,"%sѡ�����뿪����",roleName);
          school->sendCmdToSchool(&send,sizeof(send));
        }
        else
        {
          if (teacher) teacher->sendSysChat(Cmd::INFO_TYPE_FAIL,"%sѡ�����뿪ʦ��!",roleName);
        }
      }
    }
    else
    {
      Zebra::logger->error("CSchoolM::processLeaveGroup(): %s��Ӧ�ڵ��޷�ȡ���ڵ�������Ķ���",member->name);
    }
  }
}
/**
 * \brief ��������
 *
 * \param  userName ����������
 * \param schoolName ��������
 * \return �Ƿ񴴽��ɹ�
 */
bool CSchoolM::createNewSchool(const char *userName,const char *schoolName)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  rwlock.rdlock();
  tIterator = findMemberIndex(userName);
  rwlock.unlock();
  if (tIterator != memberIndex.end())
  {
    CSchool *pSchool = new CSchool();
    if (pSchool)
    {
      pSchool->setSchoolName(schoolName);
      pSchool->setMasterSerialID(tIterator->second->getSerialID());
      if (pSchool->insertSchoolToDB())
      {
        rwlock.rdlock();
        CSchool *nonSchool = (CSchool *)getEntryByID(0); //���ʦ�Ź�����
        rwlock.unlock();
        if (nonSchool)
        {
          if (nonSchool->moveMemberToNewSchool(userName,pSchool))
          {
            pSchool->getMasterNode()->getUser()->schoolid = pSchool->id; // ��ʼ������Session��schoolid
            rwlock.wrlock();
            addEntry(pSchool);
            rwlock.unlock();
            return true;
          }
        }
        else
        {
          Zebra::logger->error("CSchoolM::createNewSchool �޷�ȡ�����ɹ�����");
        }
      }
      SAFE_DELETE(pSchool);
    }
    else
    {
      Zebra::logger->error("�������ɶ����ʱ���޷������ڴ�");
    }
  }
  return false;
}

/**
 * \brief ��ɢ����
 *
 *
 * \param pName Ҫ��ɢ����������
 * \return �Ƿ�ɹ�
 */
bool CSchoolM::destroySchool(const char *pName)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  rwlock.rdlock();
  tIterator = findMemberIndex(pName);
  rwlock.unlock();
  if (tIterator != memberIndex.end())
  {
    CSchoolMember *member = tIterator->second;
    if (member)
    {
      if (member->getSchoolID() > 0)
      {
        CSchool *school = NULL;
        school = member->getSchool();
        if (school)
        {
          if (school->getMasterNode() == member)
          {
            if (school->getMasterNode()
                &&  CDareM::getMe().findDareRecordByID
                (Cmd::SCHOOL_DARE,school->getID()) == NULL)
            {
              UserSession* pUser = school->getMasterNode()->getUser();
              if (school->moveMemberToTeacherGroup())
              {
                //school->getMasterNode()->getUser()->schoolid = 0;
                school->deleteSchoolFromDB();
                SAFE_DELETE(school);
                if (pUser)
                {
                  pUser->sendSysChat(Cmd::INFO_TYPE_GAME,
                      "ʦ���ѽ�ɢ");
                }

                return true;
              }
            }
            else
            {
              if (school->getMasterNode() && school->getMasterNode()->getUser())  
              {
                school->getMasterNode()->getUser()->sendSysChat(Cmd::INFO_TYPE_FAIL,
                    "ʦ�Ŷ�ս״̬,�������ɢʦ��");
              }

            }
          }
        }
      }
    }
  }
  return false;
}

/**
 * \brief ����ɫ�Ƿ���Դ�������
 *
 * \param pUser �봴������
 * \return �Ƿ���Դ���
 */
bool CSchoolM::checkSchoolCreateCondition(const UserSession *pUser)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  rwlock.rdlock();
  tIterator = findMemberIndex(pUser->name);
  rwlock.unlock();
  if (tIterator != memberIndex.end())
  {
    CSchoolMember *me = tIterator->second;
    if (!me->havePreNode())
    {
      if (me->getSchoolID() == 0)
      {
        if (me->checkSchoolCreateCondition())
        {
          return true;
        }
        else
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"Ҫ����������ͽ�ܺ�ͽ�ܵ�ͽ�ܶ���������%u��,����ͽ�ܵȼ�Ҫȫ���ﵽ%u��,ͽ��ȼ�Ҫȫ���ﵽ%u��,��ȥ���һ��������",
              MAX_PRENTICE,
              FIRST_LAYER_PRENTICE_REQUEST_LEVEL,
              SECOND_LAYER_PRENTICE_REQUEST_LEVEL);
        }
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���Ѿ���ʦ����,�����Ը��Ͱ����ɽ�ɢ������������!");
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʦ�����ܳ�������,�Ⱥ����ʦ�������ϵ������!");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㻹û��ͽ��,��������ͽ��������!");
  }
  return false;
}

/**
 * \brief �ж��û���ͽ���Ƿ���
 *
 * \param master �����Ľ�ɫ����
 * param schoolName ��������,������˵�ͽ��δ���Ҵ����������ɵ���ô��������ͨ�����ﷵ��
 * \return ����true��ʾmaster��ͽ��û����,����false��ʾmaster��ͽ���Ѿ���������
 */
bool CSchoolM::getUserPrenticeInfo(const char *master,char *schoolName)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  bool ret;

  rwlock.rdlock();
  tIterator = findMemberIndex(master);
  if (tIterator != memberIndex.end())
  {
    if (tIterator->second->getPrenticeCount() <MAX_PRENTICE)
    {
      ret = true;
      char *point = tIterator->second->getSchoolName();
      if (point)
      {
        strncpy(schoolName,point,MAX_NAMESIZE);
      }
    }
    else
      ret = false;
  }
  else
  {
    ret = true;
  }
  rwlock.unlock();
  return ret;
}

/**
 * \brief �ж�ĳ�����Ƿ�Ϊ�ɾ���,��˼����û��ʦ����ͽ�ܵĹ�ϵ,�������ڴ˹�������
 *
 * \param name �����Ľ�ɫ������
 * \return trueΪ�ɾ���,false Ϊ�Ѿ���ʦ��ͽ�ܹ�ϵ
 */
bool CSchoolM::isClean(const char *name)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  rwlock.rdlock();
  tIterator = findMemberIndex(name);
  rwlock.unlock();
  if (tIterator == memberIndex.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * \brief Ϊ master ����һ��ͽ�� prentice
 * �ں����л�������˵�������о���Ĳ���,������ɹ������Ӧ��ɫ������Ϣ
 *
 * \param master ʦ��
 * \param prentice ͽ��
 */
/*void CSchoolM::addMember(UserSession *master,UserSession *prentice)
{
  //free ��ֹʦͽϵͳ
  master->sendSysChat(Cmd::INFO_TYPE_FAIL, "ʦͽϵͳ���ڿ� ���У�");
  return;
}*/

void CSchoolM::addMember(UserSession *master, UserSession *prentice)
{
	std::map<std::string, CSchoolMember*>::iterator tIterator;
	CSchoolMember *pPrentice = NULL;
	rwlock.rdlock();
	tIterator = findMemberIndex(master->name);
	rwlock.unlock();
	if (tIterator != memberIndex.end())
	{
		CSchoolMember *pMember = tIterator->second;
		if (NULL != (pPrentice = pMember->addPrentice(prentice)))
		{
			pPrentice->notifyNewMemberAdd(); // ֪ͨ�ܱ��³�Ա����
			if (prentice)
			{
				prentice->schoolid = master->schoolid;
				Cmd::Session::t_sendUserRelationID send;
				send.dwUserID = prentice->id;
				send.caption = master->id;
				send.dwID = master->schoolid;
				send.type = Cmd::Session::RELATION_TYPE_SCHOOL;
				prentice->scene->sendCmd(&send, sizeof(Cmd::Session::t_sendUserRelationID));
			}
		}
		else
		{
			master->sendSysChat(Cmd::INFO_TYPE_FAIL, "��֪ͨGM��֤����ʱ�޷�Ϊ�����ͽ����������");
		}
	}
	else
	{
		CSchool *noneSchool = (CSchool *)getEntryByID(0); // ȡ��ʦ�Ź�ϵ������
		CSchoolMember *masterMember = NULL;
		masterMember = noneSchool->addTeacher(master);
		if (masterMember)
		{
			if (NULL == (pPrentice = masterMember->addPrentice(prentice)))
			{
				//Zebra::logger->trace("[ʦͽ]����ͽ�ܽڵ�[%s]ʧ��ɾ��ʦ���ڵ�[%s]", master->name, prentice->name);
				masterMember->deleteMe();
				SAFE_DELETE(masterMember);
				return;
			}
			else
			{
				if (masterMember)	masterMember->sendInfomationToMe();
				if (pPrentice)
				{
					pPrentice->notifyNewMemberAdd();
					pPrentice->sendNotifyToScene();
				}
			}
		}
		else
		{
			master->sendSysChat(Cmd::INFO_TYPE_FAIL, "��֪ͨGM��֤����ʱ�޷�Ϊ�����ͽ����������");
			//Zebra::logger->error("�޷���Schoolģ��������ӽڵ�addMember����ʧ��");
			return;
		}
	}
}

/**
 * \brief �������ɳ�Ա
 *
 *
 * \param roleName ��Ա����
 * \param find ���ֻ�ǲ���,�򲻿�����Ա
 * \return 1:�ɹ� 0�������Խ������ϵ -1��û�ҵ����� 2�����Ǹ����ɵĳ�Ա
 */
int CSchoolM::fireSchoolMember(const char* roleName,bool find)
{
  struct findList : public execEntry<CSchool>
  {
    const char* _roleName;
    int _status;
    CSchool* _pSchool;

    findList(const char* roleName)
    {
      _roleName = roleName;
      _status = 2;
      _pSchool = NULL;
    }
    ~findList(){}

    void removeSchoolMember()
    {
      if (_status == 1)
      {
        Zebra::logger->info("%s ��ʦ�ų�Ա,���������ϵ",_roleName);

        if (_pSchool)
        {

          CSchoolMember *pMember = _pSchool->getMember(_roleName);
          if (pMember)
          {
            if (pMember->getTeacher())
            {
              _pSchool->processLeaveSchool(pMember);
            }
            else
            {
              pMember->fireAllPrentice();
              if (pMember->isClean())
              {
                pMember->sendDestroyNotifyToMe();
                pMember->deleteMe();
                SAFE_DELETE(pMember);
              }
            }
          }
          
          /*if (CSchoolM::getMe().processLeaveGroupDirect(_roleName))
          {
            _status = 1;
          }
          else
          {
            _status = -1;
          }
          CSchoolM::getMe().processLeaveGroupDirect(_roleName);*/
          _status = 1;
        }
        else
        {
          _status = -1;
        }
      }

      if (_status == 3)
      {
        Zebra::logger->debug("%s ��ʦ��,���������ϵ",_roleName);

        if (_pSchool)
        {
          CSchoolMember *pMember = _pSchool->getMember(_roleName);

          if (pMember)
          {
            pMember->fireAllPrentice();
          }

          CSchoolM::getMe().destroySchool(_roleName);
        
          if (pMember)
            pMember->deleteMe();
        }
      }
    }

    bool exec(CSchool *pSchool)
    {
      if (pSchool)
      {
        if (pSchool->getMasterNode() && pSchool->getMasterNode()->isMe(_roleName))
        {
#ifdef _DEBUG
          Zebra::logger->debug("%s��ʦ��,�������ϵ",_roleName);
#endif
          _pSchool = pSchool;
          _status = 3;
          return false;
        }
        else
        {
          if (pSchool->isMember(_roleName))
          {
#ifdef _DEBUG
            Zebra::logger->debug("%s �� %s ʦ�ų�Ա,�ܽ������ϵ",_roleName,pSchool->getSchoolName());
#endif
            _pSchool = pSchool;
            _status = 1;
            return false;
          }
          else
          {
#ifdef _DEBUG
            Zebra::logger->debug("%s ���� %s ��ʦ�ų�Ա",_roleName,pSchool->getSchoolName());
#endif
            _status = 2;
          }
        }
      }

      return true;
    }
  };

  findList myList(roleName);
  execEveryOne(myList);

  if (!find)
  {
    myList.removeSchoolMember();
  }

  return myList._status;
}

/**
 * \brief master ����һ��ͽ�� prentice
 * �ں����л�������˵�������о���Ĳ���,������ɹ������Ӧ��ɫ������Ϣ
 *
 * \param master ʦ��
 * \param prentice ͽ��
 * \return 
 */
void CSchoolM::frieMember(UserSession *master,const char *prentice)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  //  CSchoolMember *pPrentice = NULL;
  rwlock.rdlock();
  tIterator = findMemberIndex(master->name);
  rwlock.unlock();
  if (tIterator != memberIndex.end())
  {
    if (tIterator->second->firePrentice(prentice))
    {
      UserSession *pUser = UserSessionManager::getInstance()->getUserSessionByName(prentice);
      if (pUser)
      {
        pUser->schoolid = 0;
        Cmd::Session::t_sendUserRelationID send;
        send.dwUserID = pUser->id;
        send.caption = 0;
        send.dwID = 0;
        send.type = Cmd::Session::RELATION_TYPE_SCHOOL;
        pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
      }
      if ((tIterator->second->getSchoolID() == 0) &&(tIterator->second->isClean()))
      {
        CSchoolMember *pMember = tIterator->second;
        pMember->sendDestroyNotifyToMe();
        pMember->deleteMe();
        SAFE_DELETE(pMember);
      }
    }
    else
    {
      master->sendSysChat(Cmd::INFO_TYPE_FAIL,"���޷����������");
    }
  }
  else
  {
    master->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻��ʦ���㲻�ܿ����κ���!");
  }
}

#define getMessage(msg,msglen,pat)      \
do      \
{       \
  va_list ap;     \
    bzero(msg,msglen);     \
    va_start(ap,pat);              \
    vsnprintf(msg,msglen - 1,pat,ap);    \
    va_end(ap);     \
}while(false)


/**
 * \brief �����г�Ա����ս�����
 *
 * \param msg Ҫ������Ϣ
 */
void CSchool::notifyWarResult(const char* msg,...)
{
  char buf[1024+1];
  bzero(buf,sizeof(buf));

  struct findall : public execEntry<CSchoolMember> 
  {
    const char* msg;

    findall(const char* pMsg)
    {
      msg = pMsg;
    }

    ~findall(){}

    bool exec(CSchoolMember* pMember)
    {
      if (pMember)
      {
        if (pMember->getUser())
        {
          pMember->getUser()->sendSysChat(Cmd::INFO_TYPE_SYS,msg);
        }
      }

      return true;
    }
  };

  getMessage(buf,1024,msg);

  findall myList(buf);
  execEveryOne(myList);

}

/**
 * \brief �����г�Ա���Ͷ�ս״̬��Ϣ
 *
 * \param ptEnterWarCmd ��ս״̬��Ϣ
 * \param cmdLen ��Ϣ����
 */
void CSchool::sendCmdToAllMemberScene(Cmd::Session::t_enterWar_SceneSession* ptEnterWarCmd,const DWORD cmdLen)
{

  struct findall : public execEntry<CSchoolMember>
  {
    Cmd::Session::t_enterWar_SceneSession* cmd;
    DWORD cmdLen;

    findall(Cmd::Session::t_enterWar_SceneSession* ptCmd,const DWORD nCmdLen)
    {
      cmd = ptCmd;
      cmdLen = nCmdLen;
    }

    ~findall(){}

    bool exec(CSchoolMember* pMember)
    {
      if (pMember)
      {
        UserSession* pUser = pMember->getUser();
        if (pUser && pUser->scene)
        {
          cmd->dwUserID = pUser->id;
          cmd->dwSceneTempID = pUser->scene->tempid;
          if (cmd->dwStatus)
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ѽ���ʦ�Ŷ�ս״̬��");
          }
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����˳�ʦ�Ŷ�ս״̬��");
          }
          pUser->scene->sendCmd(cmd,cmdLen);
        }
      }

      return true;

    }
  };

  findall myList(ptEnterWarCmd,cmdLen);
  execEveryOne(myList);
}

/**
 * \brief ��Ӧ���湦�����ù�����߶�ȡ����
 *
 * \param pUser ������
 * \param rev ������Ϣ
 */
void CSchoolM::processBulletin(const UserSession *pUser,const Cmd::stSchoolBulletinCmd *rev)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  CSchoolMember *me = NULL;
  rwlock.rdlock();
  tIterator = findMemberIndex(pUser->name);
  rwlock.unlock();
  if (tIterator != memberIndex.end())
  {
    me = tIterator->second;
    if (me)
    {
      if (me->getSchoolID() >0)
      {
        switch(rev->byState)
        {
          case Cmd::SCHOOL_BULLETIN_GET:
            {
              me->sendBulletinToMe();
            }
            break;
          case Cmd::SCHOOL_BULLETIN_SET:
            {
              char buf[zSocket::MAX_DATASIZE];

              bzero(buf,zSocket::MAX_DATASIZE);
              strncpy(buf,rev->data,rev->wdSize);
              me->setBulletin(buf);
            }
            break;
        }
        return;
      }
    }
  }

  pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��û�м������ɣ���Ȩʹ�ù�����ع��ܣ�");
}

/**
 * \brief �������ֵõ���Ҷ���
 *
 *
 * \param pName �������
 * \return �ҵ��Ķ���,ʧ�ܷ���0
 */
CSchoolMember *CSchoolM::getMember(const char *pName)
{
  std::map<std::string,CSchoolMember*>::iterator tIterator;
  CSchoolMember *member = NULL;
  rwlock.rdlock();
  tIterator = findMemberIndex(pName);
  if (tIterator != memberIndex.end()) member = tIterator->second;
  rwlock.unlock();
  return member;
}

/**
 * \brief ����id�õ����ɶ���
 *
 * \param id ����id
 * \return �ҵ��Ķ���,ʧ�ܷ���0
 */
CSchool * CSchoolM::getSchool(DWORD id)
{
  CSchool *school = NULL;
  rwlock.rdlock();
  school = (CSchool *)getEntryByID(id);
  rwlock.unlock();
  return school;
}


/**
 * \brief ������ҵļ���
 *
 * \param pName �������
 * \param level ����
 */
void CSchoolM::setUserLevel(const char *pName,const WORD &level)
{
  CSchoolMember *member = getMember(pName);

  if (!member)
    return;

  if (member)
  {
    member->setLevel(level);
  }

  if (member->getTeacher()!=NULL)
  {
    if ((::abs((int)member->getTeacher()->getLevel() - (int)member->getLevel()) >= 20)  || (member->getLevel()>=80))
    {//ʦ����ͽ�ܵȼ����20��,��ͽ���Լ�������80��,����ʦͽ��ϵ
      CSchoolMember* pTeacher = member->getTeacher();
      if (pTeacher->firePrentice(pName))
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserSessionByName(pName);
        if (pUser)
        {
          pUser->schoolid = 0;
          Cmd::Session::t_sendUserRelationID send;
          send.dwUserID = pUser->id;
          send.caption = 0;
          send.dwID = 0;
          send.type = Cmd::Session::RELATION_TYPE_SCHOOL;
          pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
        }

        if ((pTeacher->getSchoolID() == 0) && (pTeacher->isClean()))
        {
          pTeacher->sendDestroyNotifyToMe();
          pTeacher->deleteMe();
          SAFE_DELETE(pTeacher);
        }
      }
    }
  }
}

//--[CSchool]----------------------------------------------------------

/**
 * \brief ���ɹ��캯��
 */
CSchool::CSchool()
{
  destroy = false;
}

/**
 * \brief ��������
 */
CSchool::~CSchool()
{
  if (!destroy)
  {
    updateSchoolInDB();
    rwlock.wrlock();
    for(zEntryID::hashmap::iterator it=zEntryID::ets.begin();it!=zEntryID::ets.end();it++)
    {
      CSchoolMember *temp = (CSchoolMember *)it->second;
      SAFE_DELETE(temp);
    }
    clear();
    rwlock.unlock();
  }
}

/**
 * \brief ������������
 *
 * \param pName ����
 */
void CSchool::setSchoolName(const char *pName)
{
  strncpy(name,pName,MAX_NAMESIZE);
}

/**
 * \brief �õ���������
 *
 * \return ��������
 */
char *CSchool::getSchoolName()
{
  return name;
}

/**
 * \brief ����ʦ��Ľڵ�id
 *
 * \param id ʦ��Ľڵ�ID
 */
void CSchool::setMasterSerialID(const DWORD &id)
{
  dwMasterSerialID = id;
}

/**
 * \brief ����������ʼ����ʦ�Ź�ϵ������
 *
 * \return ����NULL��ʾ���ͽ��ʧ��,���򷵻�ͽ�ܵĽڵ����
 */
void CSchool::initToNoneSchool()
{
  id =0;
  bzero(name,sizeof(name));
}

/**
 * \brief ��ʼ��������,������Ϣ�ṹ,�����ݿ��ʼ����ʱ����
 *
 * \param info ��������Ϣ�ṹ
 */
void CSchool::initSchool(const stSchoolInfo &info)
{
  rwlock.wrlock();
  id = info.dwSchoolID;
  strncpy(name,info.name,MAX_NAMESIZE);
  dwMasterSerialID = info.dwMasterSerialID;
  //bulletin = info.data;
  rwlock.unlock();
}

bool CSchool::addNode(const stSchoolMemberInfo &info)
{
  CSchoolMember *member = new CSchoolMember(this,NULL);

  if (member)
  {
    member->initByDBRecord(info);
    if (member->havePreNode())
    {
      CSchoolMember *pPreNode = (CSchoolMember *)getEntryByID(info.dwPreSerialID);
      if (pPreNode)
      {
        if (addMember(member))
        {
          if (pPreNode->addNextLevelNode(member))
          {
            member->setPreLevelNode(pPreNode);
          }
          else
          {
            removeMember(member);
            SAFE_DELETE(member);
          }
          return true;
        }
        else
        {
          SAFE_DELETE(member);
          Zebra::logger->error("%s�ڵ����������%s�ڵ�ʧ��idΪ%u",name,info.name,info.dwSerialID);
        }
      }
      else
      {
        SAFE_DELETE(member);
        Zebra::logger->error("%s�ڵ��������%s�ڵ�Ѱ��ʦ���ڵ�%uʧ��",name,info.name,info.dwMasterID);
      }
    }
    else
    {
      if (addMember(member))
      {
        return true;
      }
      else
      {
        SAFE_DELETE(member);
        Zebra::logger->error("%s�ڵ����������%s�ڵ�ʧ��idΪ%u",name,info.name,info.dwSerialID);
      }
    }
  }
  else
  {
    Zebra::logger->error("��ʼ����ʱ���ڴ����ʧ�� CSchool::addNode");
  }
  return false;
}

bool CSchool::updateSchoolInDB()
{
  /*
     const dbCol school_update_define[] = {
     { "BULLETIN",    zDBConnPool::DB_BIN2,  0},//MAX_BULLETIN_CHAR_NUMBER+2 },
     { NULL,0,0}
     };

     BYTE buf[zSocket::MAX_DATASIZE];
     struct stSchoolWrite
     {
     DWORD  size;
     char data[0];
     } __attribute__ ((packed))
   * schooldata;
   char where[128];

   bzero(buf,zSocket::MAX_DATASIZE);
   schooldata = (stSchoolWrite *)buf;
   connHandleID handle = SessionService::dbConnPool->getHandle();
   if ((connHandleID)-1 == handle)
   {
   Zebra::logger->error("���ܻ�ȡ���ݿ���");
   return false;
   }

   rwlock.rdlock();

   schooldata->size = bulletin.size();
   if (schooldata->size>0) strncpy(schooldata->data,bulletin.data(),schooldata->size);

   rwlock.unlock();

   bzero(where,sizeof(where));
   _snprintf(where,sizeof(where) - 1,"SCHOOLID = %u",id);
   SessionService::dbConnPool->exeUpdate(handle,"`SCHOOL`",school_update_define,(BYTE*)(schooldata),where);
   SessionService::dbConnPool->putHandle(handle);
   */
return true;
}


bool CSchool::insertSchoolToDB()
{
  const dbCol school_write_define[] = {
    { "NAME",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "MASTERSERIAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL,0,0}
  };
  static const dbCol verifyname_define[] = {
    { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { NULL,0,0}
  };

  //  BYTE buf[zSocket::MAX_DATASIZE];
  char where[128];
  char strName[MAX_NAMESIZE+1];

  struct stSchoolWrite
  {
    char  name[MAX_NAMESIZE+1];
    DWORD dwMasterSerialID;
    //    WORD  size;
    //    char data[0];
  }
  schooldata;

  //  bzero(buf,zSocket::MAX_DATASIZE);
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  //������֤�����Ƿ��ظ�
  std::string escapeName;
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"NAME = '%s'",SessionService::dbConnPool->escapeString(handle,name,escapeName).c_str());
  DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`SCHOOL`",verifyname_define,where,NULL,1,(BYTE*)(strName));
  if (retcode == 1)
  {
    SessionService::dbConnPool->putHandle(handle);
    return false;
  }

  rwlock.rdlock();

  strncpy(schooldata.name,name,MAX_NAMESIZE);
  schooldata.dwMasterSerialID = dwMasterSerialID;
  //  schooldata->size = bulletin.size();
  //  if (schooldata->size>0) strncpy(schooldata->data,bulletin.c_str(),schooldata->size);

  rwlock.unlock();

  retcode = SessionService::dbConnPool->exeInsert(handle,"`SCHOOL`",school_write_define,(const BYTE *)(&schooldata));
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("����SCHOOL�����ݿ���� name = %s",schooldata.name);
    return false;
  }
  else
  {
    id = retcode;
  }
  return true;
}

bool CSchool::deleteSchoolFromDB()
{
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"SCHOOLID = %u ",id);
  DWORD retcode = SessionService::dbConnPool->exeDelete(handle,"`SCHOOL`",where);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("ɾ��SCHOOL��¼ʧ�� SCHOOLID=%u",id);
    return false;
  }
  destroy = true;
  return true;
}

bool CSchool::loadSchoolMemberFromDB()
{
  const dbCol schoolmember_read_define[] = {
    { "SERIALID",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "MASTERID",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "PRESERIALID",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "CHARID",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "NAME",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "LEVEL",      zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "JOINTIME",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "DEGREE",      zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "LASTTIME",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "SCHOOLID",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "TAG",      zDBConnPool::DB_BYTE,  sizeof(BYTE) },
    { "`OCCUPATION`",  zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "MASTERTOTAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "MASTERBALANCE",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "PRENTICELASTLVL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "PRENTICETOTAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL,0,0}
  };

  stSchoolMemberInfo *memberList,*tempPoint;
  char where[128];

  memberList = NULL;
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  _snprintf(where,sizeof(where) - 1,"SCHOOLID = %u ",id);
  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`SCHOOLMEMBER`",schoolmember_read_define,where,"SERIALID ASC",(BYTE **)&memberList);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode
      || 0 == retcode)
  {
    return true;
  }

  bool ret = true;
  if (memberList)
  {
    tempPoint = &memberList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      if (!addNode(*tempPoint)) ret = false;
      tempPoint++;
    }
    SAFE_DELETE_VEC(memberList);
  }
  else
  {
    Zebra::logger->error("�������ݳ�ʼ��ʧ��,exeSelect ������Чbufָ��");
    ret = false;
  }
  return true; //��������ֵ��
}

// ��һ��ʦ���ڵ�,������ǰ����û�����չ�ͽ��,Ҳû�б�����Ϊͽ��,��һ��ʦ���ڵ�,���Ƿ����ɽڵ�
CSchoolMember * CSchool::addTeacher(UserSession *master)
{
  if (id != 0) return NULL; // �������һ�����ɹ���������ʹ�ô˷���
  CSchoolMember *member = new CSchoolMember(this,master);
  if (member)
  {
    member->initRootMember();
    if (member->insertRecord())
    {
      rwlock.wrlock();
      addEntry(member);
      rwlock.unlock();
      CSchoolM::getMe().addMemberIndex(master->name,member);
    }
    else
    {
      SAFE_DELETE(member);
      master->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ʱ�޷�Ϊ������ͽ��,��֪ͨGM��֤");
    }
  }
  else
  {
    Zebra::logger->error("CSchool::addTeacher():�ڴ����ʧ��,�޷������µĳ�Ա�ڵ�");
  }
  return member;
}

bool CSchool::addMember(CSchoolMember *member)
{
  bool ret = false;
  rwlock.wrlock();
  ret = addEntry(member);
  rwlock.unlock();
  if (ret) CSchoolM::getMe().addMemberIndex(member->name,member);
  return ret;
}

void CSchool::directAdddMember(CSchoolMember *member)
{
  rwlock.wrlock();
  addEntry(member);
  rwlock.unlock();
}

void CSchool::removeMember(CSchoolMember *member)
{
  CSchoolM::getMe().removeMemberIndex(member->name);
  rwlock.wrlock();
  removeEntry(member);
  rwlock.unlock();
}

void CSchool::directRemoveMember(CSchoolMember *member)
{
  rwlock.wrlock();
  removeEntry(member);
  rwlock.unlock();
}

void CSchool::sendCmdToSchool(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  rwlock.rdlock();
  for(zEntryID::hashmap::iterator it=zEntryID::ets.begin();it!=zEntryID::ets.end();it++)
  {
    CSchoolMember *temp =(CSchoolMember *)it->second;
    temp->sendCmdToMe(pNullCmd,cmdLen);
  }
  rwlock.unlock();
}

bool CSchool::moveMemberToNewSchool(const char *userName,CSchool *pSchool)
{
  bool ret = true;
  if (id != 0) return false; // ���ɹ���������ʹ�ô˷���
  rwlock.wrlock();
  CSchoolMember *master = getMember(userName);
  rwlock.unlock();
  if (master)
  {
    master->moveAllToSchool(pSchool);
    CSendSchoolCallback sscallback;
    master->getSchoolTree(sscallback);
    master->notifyAllReSendInitData(&sscallback);
    ret = true;
  }
  else
  {
    ret = false;
  }
  return ret;
}

bool CSchool::moveMemberToTeacherGroup()
{
  CSchoolMember *master = getMasterNode();
  CSchool *nonSchool = (CSchool *)CSchoolM::getMe().getSchool(0);
  if (nonSchool)
  {
    if (master)
    {
      CSchoolMemberListCallback memberSet;
      master->moveAllToSchool(nonSchool,&memberSet);
      memberSet.sendNotifyToMember();
      return true;
    }
  }
  return false;
}

CSchoolMember * CSchool::getMasterNode()
{
  CSchoolMember *member = NULL;
  rwlock.rdlock();
  member = (CSchoolMember *)getEntryByID(dwMasterSerialID);
  rwlock.unlock();
  return member;
}

void CSchool::setBulletin(const char *buf)
{
  rwlock.wrlock();
  bulletin = buf;
  rwlock.unlock();
  updateSchoolInDB();
}

const char * CSchool::getBulletin()
{
  return bulletin.data();
}

CSchoolMember *CSchool::getMember(const char *pName)
{
  CSchoolMember *member = NULL;

  member = CSchoolM::getMe().getMember(pName);
  if (member)
  {
    if (member->getSchool() != this) member = NULL;
  }
  return member;
}

bool  CSchool::isMember(const char* memberName)
{
  CSchoolMember *member = NULL;
  member = this->getMember(memberName);

  if (member == NULL)
  {
    return false;
  }

  return true;
}

bool CSchool::processLeaveSchool(CSchoolMember * member,bool deleteTeacher)
{
  if (member)
  {
    if (member->getSchool() != this) return false;
    if (member->getSchoolID() == 0)
    {
      Zebra::logger->info("[ʦͽ]processLeaveSchool(%s)",member->name);
      CSchoolMember *teacher = NULL;
      teacher = member->getTeacher();
      if (teacher)
      {
        teacher->directRemovePrentice(member);
        member->clearTeacherRelation();
        if (member->isClean())
        {
          member->sendDestroyNotifyToMe();
          member->deleteMe();
          SAFE_DELETE(member);
        }
        else
        {
          member->sendInfomationToMe();
          member->updateRecord();
        }
        if (teacher->isClean())
        {
          if (deleteTeacher)
          {
            teacher->sendDestroyNotifyToMe();
            teacher->deleteMe();
            SAFE_DELETE(teacher);
          }
        }
        else
        {
          teacher->notifyTeacherGroup();
          teacher->updateRecord();
        }
      }
      else
      {
        return false;
      }
    }
    else
    {
      if (member->getSerialID() == dwMasterSerialID) return false;
      if (member->getNextLevelNodeCount()>0)
      {
        member->notifySchoolMemberRemove();
        member->sendDestroyNotifyToMe();
        member->setInValid();
        member->updateRecord();
      }
      else
      {
        CSchoolMember *master = member->getPreLevelNode();
        member->notifySchoolMemberRemove();
        member->sendDestroyNotifyToMe();
        if (master) master->directRemovePrentice(member);
        member->deleteMe();
        SAFE_DELETE(member);

        // ����Ĳ��������ɾ���ڵ�������Ч�ڵ�ֱ��������Ч�ڵ�Ϊֹ
        CSchoolMember *curMember = master;  
        CSchoolMember *preMember = NULL;

        while((!curMember->isValid())&&(curMember->getNextLevelNodeCount()==0))
        {
          preMember = curMember->getPreLevelNode();
          if (preMember) preMember->directRemovePrentice(curMember);
          curMember->deleteMe();
          SAFE_DELETE(curMember);
          if (preMember)
          {
            curMember = preMember;
          }
          else
          {
            break;
          }
        }
        if ((curMember != master) && (NULL != curMember))
        {
          curMember->updateRecord();
        }
      }
    }
  }
  else
  {
    return false;
  }
  return true;
}

//--[CSchoolMember]----------------------------------------------------
CSchoolMember::CSchoolMember(CSchool *pSchool,UserSession *pUser)
{
  school = pSchool;
  user = pUser;
  preLevelNode = NULL;    // ʦ��
  id        = 0;
  dwMasterID    = 0;
  dwPreSerialID  = 0;
  dwCharID    = 0;
  bzero(name,sizeof(name));
  wdLevel      = 0;
  dwJoinTime    = 0;
  wdDegree    = 0;
  dwLastTime    = 0;
  dwSchoolID    = 0;
  byTag      = 0;
  wdOccupation  = 0;

  destroy = false;
}

DWORD CSchoolMember::getCharID()
{
  return dwCharID;
}

DWORD CSchoolMember::getSerialID()
{
  return id;
}

DWORD CSchoolMember::getSchoolID()
{
  return dwSchoolID;
}

char *CSchoolMember::getSchoolName()
{
  if (school)
  {
    return school->getSchoolName();
  }
  else
  {
    return NULL;
  }
}

CSchool * CSchoolMember::getSchool()
{
  return school;
}

BYTE CSchoolMember::getPrenticeCount()
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  BYTE count = 0;

  rwlock.rdlock();
  for(tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    if (tIterator->second->isValid()) count++;
  }
  rwlock.unlock();
  return count;
}

BYTE CSchoolMember::getNextLevelNodeCount()
{
  BYTE count =0;
  rwlock.rdlock();
  count = prenticeList.size();
  rwlock.unlock();
  return count;
}


WORD CSchoolMember::getDegree()
{
  return wdDegree;
}

DWORD CSchoolMember::getJoinTime()
{
  return dwJoinTime;
}

DWORD CSchoolMember::getLastTime()
{
  return dwLastTime;
}

WORD CSchoolMember::getLevel()
{
  return wdLevel;
}

CSchoolMember *CSchoolMember::addPrentice(UserSession *pUser)
{
  CSchoolMember *returnMember = NULL;
  std::pair<std::map<DWORD,CSchoolMember*,ltword>::iterator,bool> retval;

  if (getPrenticeCount() == MAX_PRENTICE) return NULL;
  if (prenticeList.size() > getPrenticeCount())
  {
    CSchoolMember *member = getFirstInValideNode();
    if (member)
    {
      member->setUser(pUser);
      member->initInValidNode(this);
      if (member->updateRecord())
      {
        directRemovePrentice(member);
        rwlock.wrlock();
        retval = prenticeList.insert(prenticeListValueType(member->getJoinTime(),member));
        rwlock.unlock();
        if (!retval.second)
        {
          Zebra::logger->error("[ʦͽ]: %s ��� %s ͽ�ܽڵ�ʧ��(%u)--A",this->name,member->name,member->getJoinTime());
        }
        CSchoolM::getMe().addMemberIndex(member->name,member);
        returnMember = member;
      }
      else
      {
        member->setInValid();
      }
    }
    else
    {
      Zebra::logger->error("CSchoolMember::addPrentice():%s����Ч�¼��ڵ��������,���֤",name);
    }
  }
  else
  {
    CSchoolMember *member = new CSchoolMember(school,pUser);

    if (member)
    {
      member->initGeneralMember(this);
      if (member->insertRecord())
      {
        rwlock.wrlock();
        retval = prenticeList.insert(prenticeListValueType(member->getJoinTime(),member));
        rwlock.unlock();
        if (!retval.second)
        {
          Zebra::logger->error("[ʦͽ]: %s ��� %s ͽ�ܽڵ�ʧ��(%u)--B",this->name,member->name,member->getJoinTime());
          member->deleteRecord();
          SAFE_DELETE(member);
        }
        else
        {
          member->setPreLevelNode(this);
          school->addMember(member);
          returnMember = member;
        }
      }
      else
      {
        SAFE_DELETE(member);
      }
    }
    else
    {
      Zebra::logger->error("CSchoolMember::addPrentice():�ڴ����ʧ��,�޷������µĳ�Ա�ڵ�");
    }
  }
  if (returnMember) updateRecord();
  return returnMember;
}

bool CSchoolMember::isClean()
{
  bool ret = true;

  if (dwMasterID >0) ret = false;
  if (getPrenticeCount()>0) ret = false;
  return ret;
}

void CSchoolMember::deleteMe()
{
  Zebra::logger->info("[ʦͽ]�ڵ㱻�Ƴ����ݿ��¼��ɾ�� Name=[%s] CharID=[%u] MasterID=[%u] PreSerialID=[%u]",this->name,this->dwCharID,this->dwMasterID,this->dwPreSerialID);
  school->removeMember(this);
  deleteRecord();
  destroy = true;
}

void CSchoolMember::online(UserSession *pUser)
{
  zRTime ctv;

  rwlock.wrlock();
  user = pUser;
  wdLevel = pUser->level;
  wdOccupation = pUser->occupation;

  //if (pUser->schoolid != school->id)
  //{
    pUser->schoolid = dwSchoolID;;
    Cmd::Session::t_sendUserRelationID send;
    send.dwUserID = pUser->id;
    CSchoolMember * pTeacher = this->getTeacher();
    if (pTeacher)
      send.caption = pTeacher->getCharID();
    else
      send.caption = 0;
    send.dwID = dwSchoolID;
    send.type = Cmd::Session::RELATION_TYPE_SCHOOL;
    pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
  //}

  DWORD curTime = ctv.sec();

  if (curTime - dwLastTime >= MAX_GROUP_TIME_GAP)
  {
    if (wdDegree - DEDUCT_POINT >=0)
      wdDegree-=DEDUCT_POINT;
    else
      wdDegree = 0;
    dwLastTime = curTime;
  }
  rwlock.unlock();
  sendInfomationToMe();
  sendOnlineStatusMessage(Cmd::SCHOOL_MEMBER_ONLINE);
}

void CSchoolMember::sendOnlineStatusMessage(BYTE onlineStatus)
{
  Cmd::stSchoolMemberStatusCmd send;
  send.byState = onlineStatus;
  send.level = wdLevel;  
  strncpy(send.name,name,MAX_NAMESIZE);


  char *tempStr = CUnionM::getMe().getUnionNameByUserName(name);
  if (tempStr) strncpy(send.unionName,tempStr,MAX_NAMESIZE);
  else strncpy(send.unionName,"��",MAX_NAMESIZE);
  tempStr = NULL;
  tempStr = CSeptM::getMe().getSeptNameByUserName(name);
  if (tempStr) strncpy(send.septName,tempStr,MAX_NAMESIZE);
  else strncpy(send.septName,"��",MAX_NAMESIZE);
  send.country = CUnionM::getMe().getCountryIDByUserName(name);
  if (send.country == 0)
  {
    if (this->user) send.country = user->country;
  }

  if (dwSchoolID >0)
  {
    if (school) school->sendCmdToSchool(&send,sizeof(send));
  }
  else
  {
    if (preLevelNode) preLevelNode->sendCmdToTeacherGroup(&send,sizeof(send));
    sendCmdToTeacherGroup(&send,sizeof(send),true);
  }
}

void CSchoolMember::prenticeRequestMemberInfo(DWORD &count,Cmd::stTeacherMember *point,CSchoolMember *me)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  bool brother = true;

  if (byTag)
  {
    point->degree = wdDegree;
    strncpy(point->name,name,MAX_NAMESIZE);

    char *tempStr = CUnionM::getMe().getUnionNameByUserName(name);
    if (tempStr) strncpy(point->unionName,tempStr,MAX_NAMESIZE);
    else strncpy(point->unionName,"��",MAX_NAMESIZE);
    tempStr = NULL;
    tempStr = CSeptM::getMe().getSeptNameByUserName(name);
    if (tempStr) strncpy(point->septName,tempStr,MAX_NAMESIZE);
    else strncpy(point->septName,"��",MAX_NAMESIZE);
    point->country = CUnionM::getMe().getCountryIDByUserName(name);
    if (point->country == 0)
    {
      if (this->user) point->country = user->country;
    }

    point->tag = Cmd::TEACHER;
    point->online = isOnline();
    point->occupation = wdOccupation;
    point->level = wdLevel;
    count++;
    point++;
  }

  for(tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    if (tIterator->second->byTag == 0) continue;
    if (tIterator->second == me)
    {
      brother = false;
      continue;
    }

    char *tempStr = CUnionM::getMe().getUnionNameByUserName(tIterator->second->name);
    if (tempStr) strncpy(point->unionName,tempStr,MAX_NAMESIZE);
    else strncpy(point->unionName,"��",MAX_NAMESIZE);
    tempStr = NULL;
    tempStr = CSeptM::getMe().getSeptNameByUserName(tIterator->second->name);
    if (tempStr) strncpy(point->septName,tempStr,MAX_NAMESIZE);
    else strncpy(point->septName,"��",MAX_NAMESIZE);
    point->country = CUnionM::getMe().getCountryIDByUserName(tIterator->second->name);
    if (point->country == 0)
    {
      if (tIterator->second->user) point->country = tIterator->second->user->country;
    }

    point->degree = tIterator->second->getDegree();
    strncpy(point->name,tIterator->second->name,MAX_NAMESIZE);
    if (brother)
      point->tag = Cmd::BIGBROTHER;
    else
      point->tag = Cmd::LITTLEBROTHER;
    point->online = tIterator->second->isOnline();
    point->occupation = tIterator->second->getOccupation();
    count++;
    point++;
  }
}

void CSchoolMember::sendInfomationToMe(CSendSchoolCallback *callback)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  Cmd::stMemberStatusSchool ret;

  if (NULL == user && byTag) return;
  //--[��ͻ��˷������ͳ�ʼ����Ϣ]------------------------
  if (0 == dwSchoolID)
  {
    ret.byStatus = Cmd::SCHOOL_STATUS_TEACHER;
  }
  else
  {
    ret.byStatus = Cmd::SCHOOL_STATUS_SCHOOL;
    strncpy(ret.schoolName,school->name,MAX_NAMESIZE);
  }
  user->sendCmdToMe(&ret,sizeof(ret));
  ////////////////////////////////////////////////////////

  BYTE buf[zSocket::MAX_DATASIZE];
  DWORD count;
  Cmd::stTeacherMember *tempPoint;

  Cmd::stSendMemberInfoCmd *retCmd=(Cmd::stSendMemberInfoCmd *)buf;
  constructInPlace(retCmd);
  tempPoint = (Cmd::stTeacherMember *)retCmd->list;
  count = 0;
  if (preLevelNode)
  {
    preLevelNode->prenticeRequestMemberInfo(count,tempPoint,this);
    tempPoint+=count;
  }

  rwlock.rdlock();
  for(tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    if (tIterator->second->byTag == 0) continue;

    char *tempStr = CUnionM::getMe().getUnionNameByUserName(tIterator->second->name);
    if (tempStr) strncpy(tempPoint->unionName,tempStr,MAX_NAMESIZE);
    else strncpy(tempPoint->unionName,"��",MAX_NAMESIZE);
    tempStr = NULL;
    tempStr = CSeptM::getMe().getSeptNameByUserName(tIterator->second->name);
    if (tempStr) strncpy(tempPoint->septName,tempStr,MAX_NAMESIZE);
    else strncpy(tempPoint->septName,"��",MAX_NAMESIZE);
    tempPoint->country = CUnionM::getMe().getCountryIDByUserName(tIterator->second->name);
    if (tempPoint->country == 0)
    {
      if (tIterator->second->user) tempPoint->country = tIterator->second->user->country;
    }

    tempPoint->degree = tIterator->second->getDegree();
    strncpy(tempPoint->name,tIterator->second->name,MAX_NAMESIZE);
    tempPoint->online = tIterator->second->isOnline();
    tempPoint->occupation = tIterator->second->getOccupation();
    tempPoint->level = tIterator->second->getLevel();
    tempPoint->tag = Cmd::PRENTICE;
    count++;
    tempPoint++;
  }

  rwlock.unlock();
  retCmd->size = count;
  user->sendCmdToMe(retCmd,(count*sizeof(Cmd::stTeacherMember)+sizeof(Cmd::stSendMemberInfoCmd)));

  if (0 != dwSchoolID)
  {
    if (callback == NULL)
    {
      CSchoolMember * master = school->getMasterNode();
      if (master)
      {
        CSendSchoolCallback sscallback;
        master->getSchoolTree(sscallback);
        sscallback.sendListToMember(this);
      }
    }
    else
    {
      callback->sendListToMember(this);
    }
  }
}

void CSchoolMember::offline()
{
  user = NULL;
  sendOnlineStatusMessage(Cmd::SCHOOL_MEMBER_OFFLINE);
}

void CSchoolMember::getMyBigBrother(CSchoolMember *member,char *pName)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  CSchoolMember *brother = NULL;

  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    if (!tIterator->second->isValid()) continue;
    if (tIterator->second == member)
    {
      break;
    }
    else
    {
      brother = tIterator->second;
    }
  }
  if (tIterator == prenticeList.end()) brother = NULL;
  if (brother)
  {
    strncpy(pName,brother->name,MAX_NAMESIZE);
  }
  else
  {
    bzero(pName,MAX_NAMESIZE);
  }
}

void CSchoolMember::notifyNewMemberAdd()
{
  if (dwSchoolID == 0)
  {// ��ʦ���еĴ�����
    if (preLevelNode) preLevelNode->notifyTeacherGroup();

    CSchoolMember *myTeacher = getTeacher();
    if (myTeacher)
    {
      char buf[MAX_CHATINFO];
      Cmd::stChannelChatUserCmd send;
      send.dwType=Cmd::CHAT_TYPE_SYSTEM;
      send.dwSysInfoType = Cmd::INFO_TYPE_GAME;
      bzero(send.pstrName,sizeof(send.pstrName));
      bzero(send.pstrChat,sizeof(send.pstrChat));
      strncpy(send.pstrName,"����",MAX_NAMESIZE);
      sprintf(buf,"��ɹ�����%sΪͽ��",this->name);
      strncpy((char *)send.pstrChat,buf,MAX_CHATINFO-1);
      myTeacher->sendCmdToMe(&send,sizeof(send));
      sprintf(buf,"���Ϊ��%s��ͽ��",myTeacher->name);
      strncpy((char *)send.pstrChat,buf,MAX_CHATINFO-1);
      sendCmdToMe(&send,sizeof(send));
    }
  }
  else
  {// �������еĴ�����
    Cmd::stAddMemberToSchoolCmd ret;
    strncpy(ret.memberName,name,MAX_NAMESIZE);
    ret.byState = Cmd::TEACHER_ANSWER_YES;
    ret.degree = 0;
    ret.wdOccupation = wdOccupation;
    if (user)
    {
      ret.level = user->level;
    }
    
    bzero(ret.prename,sizeof(ret.prename));
    if (preLevelNode)
    {
      preLevelNode->getMyBigBrother(this,ret.prename);
      preLevelNode->getMyLayer(ret.degree);
    }
    school->sendCmdToSchool(&ret,sizeof(ret));
    if (preLevelNode) 
      preLevelNode->notifyTeacherGroup();
    else 
      sendInfomationToMe();
  }
}

void CSchoolMember::initInValidNode(CSchoolMember * master)
{
  zRTime ctv;
  rwlock.wrlock();
  dwMasterID = master->getCharID();      // ʦ����ID
  dwPreSerialID = master->getSerialID();    // ǰһ���ڵ��ID
  dwCharID = user->id;            // �Լ��Ľ�ɫID
  strncpy(name,user->name,MAX_NAMESIZE);  // �Լ��Ľ�ɫ����
  wdLevel = user->level;            // ��ɫ�ĵ�ǰ����
  dwJoinTime = ctv.sec();            // �ڵ㴴��ʱ��,�������ʦ�ֵ����������λ�Ĺؼ�
  wdDegree = 0;                // �Ѻö�Ϊ��,�����Ѻö���ָ�������Լ�ʦ�����Ѻö�
  dwLastTime = 0;                // ������ʱ��,Ĭ������Ϊ0
  dwSchoolID = master->getSchoolID();      // ��ȡʦ����������Ϣ
  byTag = 1;                  // �ڵ�����Ч��,�����־ֻ���������в�������
  wdOccupation = 0;              // ��ɫ��ְҵ
  rwlock.unlock();
}

void CSchoolMember::initGeneralMember(CSchoolMember * master)
{
  zRTime ctv;
  rwlock.wrlock();
  id = 0;                    // ���ڵ��ID
  dwMasterID = master->getCharID();      // ʦ����ID
  dwPreSerialID = master->getSerialID();    // ǰһ���ڵ��ID
  dwCharID = user->id;            // �Լ��Ľ�ɫID
  strncpy(name,user->name,MAX_NAMESIZE);  // �Լ��Ľ�ɫ����
  wdLevel = user->level;            // ��ɫ�ĵ�ǰ����
  dwJoinTime = ctv.sec();            // �ڵ㴴��ʱ��,�������ʦ�ֵ����������λ�Ĺؼ�
  wdDegree = 0;                // �Ѻö�Ϊ��,�����Ѻö���ָ�������Լ�ʦ�����Ѻö�
  dwLastTime = 0;                // ������ʱ��,Ĭ������Ϊ0
  dwSchoolID = master->getSchoolID();      // ��ȡʦ����������Ϣ
  byTag = 1;                  // �ڵ�����Ч��,�����־ֻ���������в�������
  wdOccupation = user->occupation;      // ��ɫ��ְҵ
  rwlock.unlock();
}

void CSchoolMember::initRootMember()
{
  zRTime ctv;
  rwlock.wrlock();
  id = 0;                      // ���ڵ��ID
  dwMasterID = 0;                  // ʦ����ID
  dwPreSerialID = 0;                // ǰһ���ڵ��ID
  dwCharID = user->id;              // �Լ��Ľ�ɫID
  strncpy(name,user->name,MAX_NAMESIZE);    // �Լ��Ľ�ɫ����
  wdLevel = user->level;              // ��ɫ�ĵ�ǰ����
  dwJoinTime = ctv.sec();              // �ڵ㴴��ʱ��,�������ʦ�ֵ����������λ�Ĺؼ�
  wdDegree = 0;                  // �Ѻö�Ϊ��,�����Ѻö���ָ�������Լ�ʦ�����Ѻö�,���ڵ���Ѻö����ô�
  dwLastTime = 0;                  // ������ʱ��,Ĭ������Ϊ0
  dwSchoolID = 0;                  // ��ʾ������
  byTag = 1;                    // �ڵ�����Ч��,�����־ֻ���������в�������
  wdOccupation = user->occupation;        // ��ɫ��ְҵ
  rwlock.unlock();
}

bool CSchoolMember::insertRecord()
{
  static const dbCol createschoolmember_define[] = {
    { "`MASTERID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`PRESERIALID`",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`CHARID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`NAME`",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "`LEVEL`",    zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "`JOINTIME`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`DEGREE`",    zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "LASTTIME",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`SCHOOLID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`TAG`",      zDBConnPool::DB_BYTE,  sizeof(BYTE) },
    { "`OCCUPATION`",    zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "MASTERTOTAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "MASTERBALANCE",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "PRENTICELASTLVL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "PRENTICETOTAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL,0,0}
  };
  struct {
    DWORD dwMasterID;
    DWORD dwPreSerialID;
    DWORD dwCharID;
    char  name[MAX_NAMESIZE+1];
    WORD  wdLevel;
    DWORD dwJoinTime;
    WORD  wdDegree;
    DWORD dwLastTime;
    DWORD dwSchoolID;
    BYTE  byTag;
    WORD  wdOccupation;
    DWORD master_total;
    DWORD master_balance;
    DWORD prentice_lastlevel;
    DWORD prentice_total;
  }
  createschoolmember_data;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }
  //�������ݿ��ɫ��Ϣ
  bzero(&createschoolmember_data,sizeof(createschoolmember_data));

  rwlock.wrlock();
  createschoolmember_data.dwMasterID    = dwMasterID;
  createschoolmember_data.dwPreSerialID  = dwPreSerialID;
  createschoolmember_data.dwCharID    = dwCharID;
  strncpy(createschoolmember_data.name,name,MAX_NAMESIZE);
  createschoolmember_data.wdLevel      = wdLevel;
  createschoolmember_data.dwJoinTime    = dwJoinTime;
  createschoolmember_data.wdDegree    = wdDegree;
  createschoolmember_data.dwLastTime    = dwLastTime;
  createschoolmember_data.dwSchoolID    = dwSchoolID;
  createschoolmember_data.byTag      = byTag;
  createschoolmember_data.wdOccupation  = wdOccupation;

  createschoolmember_data.master_total  = master_total;
  createschoolmember_data.master_balance  = master_balance;
  createschoolmember_data.prentice_lastlevel  = prentice_lastlevel;
  createschoolmember_data.prentice_total  = prentice_total;
  rwlock.unlock();

  DWORD retcode = SessionService::dbConnPool->exeInsert(handle,"`SCHOOLMEMBER`",createschoolmember_define,(const BYTE *)(&createschoolmember_data));
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("����SCHOOLMEMBER���ݿ���� %u,%s",dwCharID,name);
    return false;
  }

  id = retcode;
  return true;
}

bool CSchoolMember::updateRecord(bool locked)
{
  static const dbCol updatechoolmember_define[] = {
    { "`MASTERID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`PRESERIALID`",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`CHARID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`NAME`",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "`LEVEL`",    zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "`JOINTIME`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`DEGREE`",    zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "LASTTIME",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`SCHOOLID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`TAG`",      zDBConnPool::DB_BYTE,  sizeof(BYTE) },
    { "`OCCUPATION`",  zDBConnPool::DB_WORD,  sizeof(WORD) },
    { "MASTERTOTAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "MASTERBALANCE",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "PRENTICELASTLVL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "PRENTICETOTAL",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL,0,0}
  };
  struct {
    DWORD dwMasterID;
    DWORD dwPreSerialID;
    DWORD dwCharID;
    char  name[MAX_NAMESIZE+1];
    WORD  wdLevel;
    DWORD dwJoinTime;
    WORD  wdDegree;
    DWORD dwLastTime;
    DWORD dwSchoolID;
    BYTE  byTag;
    WORD  wdOccupation;
    DWORD master_total;
    DWORD master_balance;
    DWORD prentice_lastlevel;
    DWORD prentice_total;
  }
  updateschoolmember_data;
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  bzero(&updateschoolmember_data,sizeof(updateschoolmember_data));

  if (!locked) rwlock.rdlock();  

  updateschoolmember_data.dwMasterID    = dwMasterID;
  updateschoolmember_data.dwPreSerialID  = dwPreSerialID;
  updateschoolmember_data.dwCharID    = dwCharID;
  strncpy(updateschoolmember_data.name,name,MAX_NAMESIZE);
  updateschoolmember_data.wdLevel      = wdLevel;
  updateschoolmember_data.dwJoinTime    = dwJoinTime;
  updateschoolmember_data.wdDegree    = wdDegree;
  updateschoolmember_data.dwLastTime    = dwLastTime;
  updateschoolmember_data.dwSchoolID    = dwSchoolID;
  updateschoolmember_data.byTag      = byTag;
  updateschoolmember_data.wdOccupation  = wdOccupation;
  
  updateschoolmember_data.master_total  = master_total;
  updateschoolmember_data.master_balance  = master_balance;
  updateschoolmember_data.prentice_lastlevel  = prentice_lastlevel;
  updateschoolmember_data.prentice_total  = prentice_total;

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"`SERIALID` = %u",id);

  if (!locked) rwlock.unlock();

  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`SCHOOLMEMBER`",updatechoolmember_define,(BYTE*)(&updateschoolmember_data),where);
  SessionService::dbConnPool->putHandle(handle);

  if (retcode > 1)
  {
    Zebra::logger->error("�޸����ɳ�Ա����ʧ�ܣ�serialid = %u,retcode = %u",id,retcode);
    return false;
  }
  return true;
}

bool CSchoolMember::deleteRecord()
{
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"SERIALID = %u ",id);
  DWORD retcode = SessionService::dbConnPool->exeDelete(handle,"`SCHOOLMEMBER`",where);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("ɾ������SCHOOLMEMBER��¼ʧ�� SERIALID=%u",id);
    return false;
  }
  return true;
}

void CSchoolMember::initByDBRecord(const stSchoolMemberInfo &info)
{
  id        = info.dwSerialID;
  dwMasterID    = info.dwMasterID;
  dwPreSerialID  = info.dwPreSerialID;
  dwCharID    = info.dwCharID;
  strncpy(name,info.name,MAX_NAMESIZE);
  wdLevel      = info.wdLevel;
  dwJoinTime    = info.dwJoinTime;
  wdDegree    = info.wdDegree;
  dwLastTime    = info.dwLastTime;
  dwSchoolID    = info.dwSchoolID;
  byTag      = info.byTag;
  wdOccupation  = info.wdOccupation;
}

bool CSchoolMember::havePreNode()
{
  return (dwPreSerialID !=0 );
}

bool CSchoolMember::haveTeacher()
{
  return (dwMasterID != 0);
}

bool CSchoolMember::addNextLevelNode(CSchoolMember *member)
{
  std::pair<std::map<DWORD,CSchoolMember*,ltword>::iterator,bool> retval;
  retval = prenticeList.insert(prenticeListValueType(member->getJoinTime(),member));

  if (!retval.second)
  {
    Zebra::logger->error("[ʦͽ]: %s ��� %s ͽ�ܽڵ�ʧ��(%u)",this->name,member->name,member->getJoinTime());
  }
  
  return retval.second;
}

void CSchoolMember::setPreLevelNode(CSchoolMember *pPreNode)
{
  preLevelNode = pPreNode;
}

bool CSchoolMember::fireAllPrentice()
{
  bool ret = false;
  std::map<DWORD,CSchoolMember*,ltword> tmp_bak = prenticeList;
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

  for (tIterator = tmp_bak.begin(); tIterator != tmp_bak.end(); tIterator++)
  {
    if (school->processLeaveSchool(tIterator->second,false))
    {
    }
  }
  return ret;
}

bool CSchoolMember::firePrentice(const char *prenticeName)
{
  bool ret = false;

  if (dwSchoolID == 0)
  {
    CSchoolMember * member = school->getMember(prenticeName);
    if (member)
    {
      UserSession * pUser = member->getUser();
      if (member->getTeacher() == this) // ������˵�ʦ���Ǳ��ڵ�
      {
        if (school->processLeaveSchool(member,false))
        {
          if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㱻���ʦ��%s���ʦ��!",name);
          Cmd::stChannelChatUserCmd send;
          send.dwType = Cmd::CHAT_TYPE_SYSTEM;
          send.dwChannelID = 0;
          strncpy(send.pstrName,name,MAX_NAMESIZE);
          _snprintf(send.pstrChat,sizeof(send.pstrChat) - 1,"%s�������ʦ��",prenticeName);
          this->sendCmdToTeacherGroup(&send,sizeof(send));
          //school->sendCmdToSchool(&send,sizeof(send));
          Zebra::logger->info("[ʦͽ]%s��%s���ʦ��,ʣ��ͽ����Count=[%u] Size=[%u]",prenticeName,name,
                this->getPrenticeCount(),prenticeList.size());
        }
      }
      else
      {
        if (user) user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻����ʦ�����ܿ�������");
      }

      ret = true;
    }
  }
  else
  {
    CSchoolMember *prentice = school->getMember(prenticeName);
    if (prentice)
    {
      UserSession * pUser = prentice->getUser();
      if (prentice->getTeacher() == this) // ������˵�ʦ���Ǳ��ڵ�
      {
        if (school->processLeaveSchool(prentice))
        {
          if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㱻���ʦ��%s�������!",name);
        }
        ret = true;
      }
      else if (school->getMasterNode() == this) // ������ڵ���ʦ��
      {
        if (school->processLeaveSchool(prentice))
        {
          if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㱻���ʦ��%s�������!",name);
        }
        ret = true;
      }
    }
  }

  if (ret) updateRecord();
  return ret;
}

bool CSchoolMember::isMe(const char *pName)
{
  return (strncmp(pName,name,MAX_NAMESIZE) == 0);
}

bool CSchoolMember::isOnline()
{
  return (NULL != user);
}

bool CSchoolMember::isValid()
{
  return (byTag == 1);
}

void CSchoolMember::getMyLayer(DWORD &layer)
{
  if (preLevelNode)
  {
    preLevelNode->getMyLayer(layer);
  }
  layer++;
}

void CSchoolMember::notifyTeacherGroup()
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  sendInfomationToMe();
  rwlock.rdlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    tIterator->second->sendInfomationToMe();
  }
  rwlock.unlock();
}

void CSchoolMember::sendCmdToTeacherGroup(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen,bool exceptMe)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

  if (!exceptMe) sendCmdToMe(pNullCmd,cmdLen);
  rwlock.rdlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    tIterator->second->sendCmdToMe(pNullCmd,cmdLen);
  }
  rwlock.unlock();
}

void CSchoolMember::sendCmdToMe(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  if (1==byTag)
  {
    if (user)
    {
      user->sendCmdToMe(pNullCmd,cmdLen);
    }
    else
    {
      if (pNullCmd->byCmd == Cmd::CHAT_USERCMD && pNullCmd->byParam == ALL_CHAT_USERCMD_PARAMETER)
      {
        Cmd::stChannelChatUserCmd *pCmd = (Cmd::stChannelChatUserCmd *)pNullCmd;
        if (Cmd::CHAT_TYPE_OVERMAN_AFFICHE == pCmd->dwType)
        {
          COfflineMessage::writeOfflineMessage(pCmd->dwType,this->id,pNullCmd,cmdLen);
        }
      }
    }
  }
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

void CSchoolMember::sendSysChat(int type,const char *pattern,...)
{
  if (user && (byTag == 1))
  {
    char buf[MAX_CHATINFO];
    getMessage(buf,MAX_CHATINFO,pattern);
    user->sendSysChat(type,buf);
  }
}

void CSchoolMember::clearTeacherRelation()
{
  rwlock.wrlock();
  preLevelNode = NULL;
  dwMasterID = 0;
  if (dwSchoolID == 0) dwPreSerialID =0;
  rwlock.unlock();
}

bool CSchoolMember::checkSchoolCreateCondition()
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  bool ret = true;
  int count = 0;

  rwlock.rdlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    count++;
    if (!tIterator->second->checkMeAndPrenticeNumberAndLevel()) ret = false;
  }
  if (count<MAX_PRENTICE) ret = false;
  rwlock.unlock();
  return ret;
}

bool CSchoolMember::checkMeAndPrenticeNumberAndLevel()
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  bool ret = true;
  int count = 0;

  rwlock.rdlock();
  if (wdLevel < FIRST_LAYER_PRENTICE_REQUEST_LEVEL) ret = false;
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    count++;
    if (tIterator->second->getLevel() < SECOND_LAYER_PRENTICE_REQUEST_LEVEL) ret = false;
  }
  if (count<MAX_PRENTICE) ret = false;
  rwlock.unlock();
  return ret;
}

void CSchoolMember::moveAllToSchool(CSchool *pSchool,CSchoolMemberListCallback *memberSet)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

  rwlock.rdlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    tIterator->second->moveAllToSchool(pSchool,memberSet);
  }

  pSchool->directAdddMember(this);
  school->directRemoveMember(this);
  school = pSchool;
  dwSchoolID = pSchool->id;
  if (this->user)
  {
    user->schoolid = pSchool->id;
    Cmd::Session::t_sendUserRelationID send;
    send.dwUserID = user->id;
    CSchoolMember *pTeacher = this->getTeacher();
    if (pTeacher)
    {
      send.caption = pTeacher->getCharID();
    }
    else
    {
      send.caption = 0;
    }
    send.dwID = pSchool->id;
    send.type = Cmd::Session::RELATION_TYPE_SCHOOL;
    user->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
  }
  updateRecord();
  if (memberSet) memberSet->exec(this);
  rwlock.unlock();
}

void CSchoolMember::notifyAllReSendInitData(CSendSchoolCallback *callback)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

  rwlock.rdlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    tIterator->second->notifyAllReSendInitData(callback);
  }
  rwlock.unlock();
  sendInfomationToMe(callback);
}

void CSchoolMember::getSchoolTree(CSendSchoolCallback &callback)
{
  DWORD curLevel = 0;
  bool condition;
  BYTE tag;

  do {
    condition = false;
    tag = Cmd::SCHOOL_NEWLAYER;
    schoolTreeCallback(0,curLevel,condition,callback,tag);
    curLevel++;
  }while(condition);
}

void CSchoolMember::schoolTreeCallback(DWORD level,DWORD tgLevel,bool &condition,CSendSchoolCallback &callback,BYTE &tag)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

  if (level >= tgLevel)
  {
    if (!prenticeList.empty()) condition = true;
    callback.exec(this,tag);
  }
  else
  {
    level++;
    for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
    {
      tIterator->second->schoolTreeCallback(level,tgLevel,condition,callback,tag);
      if (level == tgLevel) tag = Cmd::SCHOOL_NONE;
    }
    if (!prenticeList.empty()&&(level == tgLevel))tag = Cmd::SCHOOL_NEWLINE;
  }
}

void CSchoolMember::sendBulletinToMe()
{
  if (dwSchoolID <0) return;
  if (school)
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    const char *point;

    point = school->getBulletin();

    bzero(buf,zSocket::MAX_DATASIZE);
    Cmd::stSchoolBulletinCmd *retCmd=(Cmd::stSchoolBulletinCmd *)buf;
    constructInPlace(retCmd);
    retCmd->byState = Cmd::SCHOOL_BULLETIN_GET;
    if (point)
    {
      retCmd->wdSize = strlen(point);
      strncpy(retCmd->data,point,retCmd->wdSize);
    }
    else
    {
      retCmd->wdSize = 0;
    }
    if (user) user->sendCmdToMe(retCmd,sizeof(retCmd));
  }
}

void CSchoolMember::setBulletin(const char *buf)
{
  if (dwSchoolID <0) return;
  if (school)
  {
    if (school->getMasterNode() == this)
    {
      school->setBulletin(buf);
    }
    else
    {
      if (user) user->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻��ʦ������д����!");
    }
  }
}

CSchoolMember * CSchoolMember::getPreLevelNode()
{
  return preLevelNode;
}

CSchoolMember * CSchoolMember::getTeacher()
{
  if (preLevelNode)
  {
    if (preLevelNode->getCharID() == dwMasterID)
    {
      return preLevelNode;
    }
  }
  return NULL;
}

void CSchoolMember::setInValid()
{
  if (dwSchoolID > 0)
  {
    std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

    CSchoolM::getMe().removeMemberIndex(name);

    rwlock.wrlock();
    dwMasterID =0;
    dwCharID = 0;
    user = NULL;
    byTag = 0;
    //    dwJoinTime = (DWORD)-1;  ���µ���Ա�����ս���֮ǰ���ı���λ�á�
    bzero(name,MAX_NAMESIZE);
    wdLevel = 0;
    wdDegree = 0;
    dwLastTime = 0;
    wdOccupation = 0;
    for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
    {
      if (tIterator->second->isValid())
      {
        tIterator->second->clearMaster();
        tIterator->second->updateRecord();
      }
    }
    rwlock.unlock();
  }
}

void CSchoolMember::clearMaster()
{
  dwMasterID = 0;
}

void CSchoolMember::sendDestroyNotifyToMe()
{
  Cmd::stMemberStatusSchool ret;
  ret.byStatus = Cmd::SCHOOL_STATUS_NONE;
  if (user) user->sendCmdToMe(&ret,sizeof(ret));
}

void CSchoolMember::directRemovePrentice(const CSchoolMember *member)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

  rwlock.wrlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    if (tIterator->second == member)
    {
      prenticeList.erase(tIterator);
      break;
    }
  }
  rwlock.unlock();
}

void CSchoolMember::notifySchoolMemberRemove()
{
  Cmd::stFireMemberFromSchoolCmd ret;
  strncpy(ret.memberName,name,MAX_NAMESIZE);
  school->sendCmdToSchool(&ret,sizeof(ret));
}

CSchoolMember* CSchoolMember::getFirstInValideNode()
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  CSchoolMember *member = NULL;

  rwlock.rdlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    if (!tIterator->second->isValid())
    {
      member = tIterator->second;
      break;
    }
  }
  rwlock.unlock();
  return member;
}

void CSchoolMember::setUser(UserSession *pUser)
{
  user = pUser;
}

void CSchoolMember::clearPreLevelNode()
{
  rwlock.wrlock();
  dwPreSerialID = 0;
  preLevelNode = NULL;
  rwlock.unlock();
}

void CSchoolMember::clearAllNextLevelRelation()
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;

  rwlock.rdlock();
  for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
  {
    tIterator->second->clearPreLevelNode();
  }
  prenticeList.clear();
  rwlock.unlock();
}

void CSchoolMember::setFriendDegree(const Cmd::Session::t_CountFriendDegree_SceneSession *rev)
{
  std::map<DWORD,CSchoolMember*,ltword>::iterator tIterator;
  zRTime ctv;

  rwlock.wrlock();
  for (int i=0; i<rev->size; i++)
  {
    if (rev->namelist[i].byType == Cmd::RELATION_TYPE_TEACHER)
    {
      if (preLevelNode)
      {
        if (preLevelNode->getCharID() == rev->namelist[i].dwUserID)
        {
          if (wdDegree < rev->namelist[i].wdDegree)
          {
            wdDegree = rev->namelist[i].wdDegree;
            dwLastTime = ctv.sec();
            updateRecord(true);
          }
        }
      }
      for (tIterator = prenticeList.begin(); tIterator != prenticeList.end(); tIterator++)
      {
        if (tIterator->second->getCharID() == rev->namelist[i].dwUserID)
        {
          if (tIterator->second->getDegree() < rev->namelist[i].wdDegree)
          {
            tIterator->second->setDegree(rev->namelist[i].wdDegree);
            tIterator->second->setLastTime(ctv.sec());
            tIterator->second->updateRecord(true);
          }
        }
      }
    }
  }
  rwlock.unlock();
}

void CSchoolMember::setDegree(const WORD &degree)
{
  wdDegree = degree;
}

void CSchoolMember::setLastTime(const DWORD &lasttime)
{
  dwLastTime =lasttime;
}

void CSchoolMember::sendChatMessages(const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen)
{
  if (dwSchoolID == 0)
  {
    CSchoolMember *myTeacher = getTeacher();
    if (rev->dwType == Cmd::CHAT_TYPE_OVERMAN_AFFICHE)
    {
      sendCmdToTeacherGroup(rev,cmdLen);
    }
    else
    {
      if (myTeacher)
      {
        //myTeacher->sendCmdToTeacherGroup(rev,cmdLen);  �°汾������Ϣ�������Լ���ʦ�ֵ�ֻ�����Լ���ʦ���Լ���ͽ��
        //sendCmdToTeacherGroup(rev,cmdLen,true);
        myTeacher->sendCmdToMe(rev,cmdLen);
        sendCmdToTeacherGroup(rev,cmdLen);
      }
      else
      {
        sendCmdToTeacherGroup(rev,cmdLen);
      }
    }
  }
  else
  {
    if (school) school->sendCmdToSchool(rev,cmdLen);
  }
}

CSchoolMember::~CSchoolMember()
{
  if (!destroy) updateRecord();
}

UserSession *CSchoolMember::getUser()
{
  return user;
}

void CSchoolMember::setLevel(const WORD level)
{
  this->wdLevel = level;
  updateRecord();
}

WORD CSchoolMember::getOccupation()
{
  return wdOccupation;
}

void CSchoolMember::sendNotifyToScene()
{
  if (user)
  {
    Cmd::Session::t_sendUserRelationID send;
    send.dwUserID = user->id;
    CSchoolMember * pTeacher = this->getTeacher();
    if (pTeacher)
      send.caption = pTeacher->getCharID();
    else
      send.caption = 0;
    send.dwID = 0;
    send.type = Cmd::Session::RELATION_TYPE_SCHOOL;
    if (user->scene) user->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
  }
}

DWORD CSchoolMember::queryBounty()
{
  double ret = 0;
  if (getUser() && getUser()->level>=10)
  {
    if (this->prentice_lastlevel==0)
    {
      ret = (0.1*(getUser()->level*getUser()->level - 19*getUser()->level + 90)) * 100;
      if (ret < 0) ret = 0;
    }
    else
    {
      ret = (0.1 * (getUser()->level*getUser()->level - 19*getUser()->level - this->prentice_lastlevel*this->prentice_lastlevel + 19*this->prentice_lastlevel)) * 100;
      
    }
  }
  
  return 0;
}

bool CSchoolMember::putBounty()
{
  if (getUser()!=NULL && getTeacher()!=NULL)
  {
    DWORD bounty = this->queryBounty();

    getTeacher()->master_balance += bounty;
    getTeacher()->updateRecord();
    
    this->prentice_lastlevel = getUser()->level;
    this->prentice_total += bounty;
    
    this->updateRecord();
    return true;
  }

  return false;
}
