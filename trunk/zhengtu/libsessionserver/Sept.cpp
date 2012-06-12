/**
 * \brief ʵ�ּ��������
 *
 */

#include <zebra/SessionServer.h>

#include <stdarg.h>

using namespace SeptDef;

class CSeptSort
{
  public:
    char  septName[MAX_NAMESIZE];           // ��������
    DWORD dwRepute;                         // ��������
    DWORD dwOrder;                          // ��������
    DWORD dwCountryID;      // ��������

    CSeptSort()
    {
      dwRepute = 0;
      dwOrder  = 0;
      dwCountryID = 0;
      bzero(septName,MAX_NAMESIZE);
    }

    CSeptSort(const CSeptSort& ref)
    {
      dwRepute = ref.dwRepute;
      dwOrder = ref.dwOrder;
      dwCountryID = ref.dwCountryID;
      strncpy(septName,ref.septName,MAX_NAMESIZE);
    }

    ~CSeptSort()
    {
    }
    
    CSeptSort& operator =(const CSeptSort& ref)
    {
      this->dwRepute = ref.dwRepute;
      this->dwOrder = ref.dwOrder;
      this->dwCountryID = ref.dwCountryID;
      strncpy(this->septName,ref.septName,MAX_NAMESIZE);
      return *this;
    }

    friend bool operator<(const CSeptSort& lhs,const CSeptSort& rhs)
    {
      return lhs.dwRepute > rhs.dwRepute;
    }
};

//==[CSeptMember]==================================================
/**
* \brief �����Ա���캯��,��ʼ����������
*/
CSeptMember::CSeptMember()
{
  destroy  = false;
  mnRight = 9;	// Ĭ��Ϊ��Ա...
  //user = NULL;
}

/**
* \brief �����Ա��ʼ��
* \param info ��Ա��Ϣ�ṹ
*/
void CSeptMember::init(const stSeptMemberInfo& info)
{
  rwlock.wrlock();

  id = info.dwCharID;
  strncpy(name,info.name,MAX_NAMESIZE);

  if(info.aliasname[0] == '\0' )
	  strncpy(aliasname, "��Ա", MAX_NAMESIZE);
  else
	  strncpy(aliasname,info.aliasname,MAX_NAMESIZE);

  mnRight = info.nRight;				//ְλ���.
  wdOccupation = info.wdOccupation;
  byStatus = CSeptMember::Offline;                  // ��Ա״̬
  rwlock.unlock();
}

/**
* \brief ��ȡ��Ա�Ļ�����Ϣ
* \param info ���ص���Ϣ�ṹ
*/
void CSeptMember::getMemberBaseInfo(struct Cmd::stSeptRember& info)
{
  rwlock.rdlock();
  strncpy(info.memberName,name,MAX_NAMESIZE);
  strncpy(info.memberAliasName,aliasname,MAX_NAMESIZE);
  
  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);
  if (pUser)
  {
    info.level = pUser->level;
    info.exploit = pUser->dwExploit;
    info.useJob = pUser->dwUseJob;
	//Shx Add ���Ȩ��ְλ;
	info.nRight = this->mnRight;
  }
  else
  {
	  
    info.level = 0;
    info.exploit = 0;
    info.useJob = 0;
	//Shx Add Ĭ����9 ,��Ҳ������޷���ȡ��Ȩ��,�����ԱȨ��..;
	info.nRight = 9;
  }

  info.byOnline = byStatus;
  info.occupation = wdOccupation;
  rwlock.unlock();
}

/**
* \brief ������Ϣ����Ա��Ӧ�Ŀͻ���
* \param  pstrCmd ��Ϣ��
* \param  nCmdLen ��Ϣ����
*/
void CSeptMember::sendCmdToMe(const Cmd::stNullUserCmd *pstrCmd,const DWORD nCmdLen)
{
  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);
  if (pUser)
  {
    pUser->sendCmdToMe(pstrCmd,nCmdLen);
  }
  else
  {
    if ((pstrCmd->byCmd == Cmd::CHAT_USERCMD) && (pstrCmd->byParam == ALL_CHAT_USERCMD_PARAMETER))
    {
      Cmd::stChannelChatUserCmd *pCmd = (Cmd::stChannelChatUserCmd *)pstrCmd;
      if (Cmd::CHAT_TYPE_FAMILY_AFFICHE == pCmd->dwType)
      {
        COfflineMessage::writeOfflineMessage(pCmd->dwType,this->id,pstrCmd,nCmdLen);
      }
    }
  }
}

/**
* \brief ���³�Ա�����ݿ�浵
*/
void CSeptMember::writeDatabase()
{
  static const dbCol septmember_define[] = {
    { "`OCCUPATION`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "ALIASNAME",    zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { NULL,0,0}
  };
  struct {
    WORD  wdOccupation;            // ��Ա��ְҵ
    char  aliasname[MAX_NAMESIZE+1];
  }
  updateseptmember_data;
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  rwlock.rdlock();
  
  updateseptmember_data.wdOccupation = wdOccupation;
  strncpy(updateseptmember_data.aliasname,aliasname,MAX_NAMESIZE);
  
  rwlock.unlock();
  
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"SEPTID = %u AND CHARID = %u",mySept->getID(),id);
  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`SEPTMEMBER`",septmember_define,(BYTE*)(&updateseptmember_data),where);
  SessionService::dbConnPool->putHandle(handle);

  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("CSeptMember �޸Ļ�Ա����ʧ�ܣ�SEPTID=%u CHARID=%u retcode=%u",mySept->getID(),id,retcode);
  }
}

/**
* \brief ����Ա��¼�������ݿ�
*/
void CSeptMember::insertDatabase()
{
  static const dbCol septmember_define[] = {
    { "`SEPTID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { "`ALIASNAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { "`OCCUPATION`",zDBConnPool::DB_WORD,sizeof(WORD) },
	{ "`MBRIGHT`", zDBConnPool::DB_WORD, sizeof(WORD) },
    { NULL,0,0}
  };
  struct {
    DWORD dwSeptID;            // ������
    DWORD  dwCharID;            // ��Ա��ɫID
    char  name[MAX_NAMESIZE+1];           // ��Ա����
    char  aliasname[MAX_NAMESIZE+1];		// ��Ա����
    WORD  wdOccupation;            // ��Ա��ɫ
	WORD	wdnRight;
  }
  createseptmember_data;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
          Zebra::logger->error("���ܻ�ȡ���ݿ���");
          return;
  }

  //�������ݿ��ɫ��Ϣ
  bzero(&createseptmember_data,sizeof(createseptmember_data));

  rwlock.rdlock();
  
  createseptmember_data.dwSeptID = mySept->getID();
  createseptmember_data.dwCharID = id;
  strncpy(createseptmember_data.name,name,MAX_NAMESIZE);
  strncpy(createseptmember_data.aliasname,aliasname,MAX_NAMESIZE);
  createseptmember_data.wdOccupation = wdOccupation;
  createseptmember_data.wdnRight = mnRight;

  rwlock.unlock();

  DWORD retcode = SessionService::dbConnPool->exeInsert(handle,"`SEPTMEMBER`",septmember_define,(const BYTE *)(&createseptmember_data));
    SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("��������Ա���ݿ���� %u %u",mySept->getID(),id);
  }
  return;
}

void CSeptMember::change_aliasname(const char* aliasname)
{
  rwlock.wrlock();
  strncpy(this->aliasname,aliasname,MAX_NAMESIZE);
  rwlock.unlock();

  this->online(Cmd::SEPT_MEMBER_STATUS_ALIASCHANGE);
}

void CSeptMember::update_data()
{
	Cmd::Session::t_sendUserRelationID send;
	UserSession* pUser  = UserSessionManager::getInstance()->getUserByID(id);

	if (this->mySept)
	{
		send.dwUserID = this->id;
		send.dwID = this->mySept->id;
		send.type = Cmd::Session::RELATION_TYPE_SEPT;
		bzero(send.name,sizeof(send.name));
		strncpy(send.name,this->mySept->name,sizeof(send.name));

		send.septmaster = (this == this->mySept->master);
		send.dwRepute = this->mySept->getRepute();
		send.dwSeptLevel = this->mySept->dwLevel;




		if (pUser)  pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_sendUserRelationID));
	}
//[Shx Delete ��սNPC]
//	if (pUser) CNpcDareM::getMe().sendUserData(pUser);
}

void CSeptMember::update_normal_data()
{
  Cmd::Session::t_SendSeptNormal_SceneSession send;
  UserSession* pUser  = UserSessionManager::getInstance()->getUserByID(id);

  if (this->mySept)
  {
    send.dwUserID = this->id;
    send.dwRepute = this->mySept->getRepute();

    if (pUser && pUser->scene)  pUser->scene->sendCmd(&send,sizeof(send));
  }
}


/**
* \brief ��������,������ʱ��ˢ�³�Ա�����ݿ�浵
*/
CSeptMember::~CSeptMember()
{
//  if (!destroy) writeDatabase();
}

/**
* \brief ���ó�Ա�������������
* \param pSept �������ָ��
*/
void CSeptMember::setSept(CSept * pSept)
{
   rwlock.wrlock();
   mySept = pSept;
   rwlock.unlock();
}

/**
* \brief ����������Ϣ����Ա�Ŀͻ���
* \param type ��Ϣ����
* \param message ��Ϣ��
*/
void CSeptMember::sendMessageToMe(int type,const char *message)
{
  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);
  if (NULL !=pUser) pUser->sendSysChat(type,message);
}

/**
* \brief ������Ա�����ݿ��¼�ӿ���ɾ��
* \return true �ɹ�,false ʧ��
*/
bool CSeptMember::deleteMeFromDB()
{
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"SEPTID = %u AND CHARID = %u",mySept->getID(),id);
  DWORD retcode = SessionService::dbConnPool->exeDelete(handle,"`SEPTMEMBER`",where);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->debug("ɾ�������Աʧ�� %u",id);
    return false;
  }
  else
  {
    destroy = true;
  }
  return true;
}

/**
* \brief �����Ա�����Ĺ�ϵ
* \param notify ֪ͨ��־,Ϊtrue��ʾҪ֪ͨ���е����߳�Ա,Ϊflase��ʾ��֪ͨ
*/
void CSeptMember::fireMe(const bool notify,const bool checkunion)
{
  if (deleteMeFromDB())
  {
    CSeptM::getMe().removeMemberIndex(name);
    if (notify) 
    {
      mySept->notifyMemberFire(name);
      mySept->sendSeptNotify("%s �˳��˼���",name);
    }
    
    CUnion* pUnion = CUnionM::getMe().getUnionByID(this->mySept->dwUnionID);
    
    if (pUnion && checkunion)
    {
      if (this->id == this->mySept->master->id)
      {
         pUnion->fireSeptFromUnion(this->mySept->id);
      }
      else
      {
        pUnion->fireUnionMemberDirect(id,false);
      }
    }
    
    UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);
    if (pUser)
    {
      Cmd::Session::t_fireSeptMember_SceneSession send;
      send.dwCharID = id;
      
      send.dwMapTempID = pUser->scene->tempid;
      pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_fireSeptMember_SceneSession)); /// ֪ͨ����������
      pUser->septid = 0;
      CNpcDareM::getMe().sendUserData(pUser);
    }
  }
  else
  {
    Zebra::logger->error("[����]:���� %s ��Աʧ��",this->name);
  }
}

/**
* \brief �����û��ļ����Ա����
*/
void CSeptMember::sendUserSeptData()
{
  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);
  if (mySept && pUser)
  {
    mySept->sendSeptInfoToUser(pUser); // ���ͼ������Ϣ����ǰ��Ա
    mySept->sendSeptMemberList(pUser); // ���ͼ���ĳ�Ա�б����ǰ��Ա
  }
}

/**
* \brief �жϳ�Ա�Ƿ�����
* \return true����  false ������
*/
bool CSeptMember::isOnline()
{
    return (byStatus==CSeptMember::Online)?true:false;
}

/** 
* \brief ��Ա���ߴ���
* \param status ��Ա������״̬
*/
void CSeptMember::online(const DWORD status)
{
  Cmd::stBroadcastSeptMemberInfo send;
  
   rwlock.wrlock();
   UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);

  if (pUser)
  {
    byStatus = CSeptMember::Online; 
    wdOccupation = pUser->occupation;
    send.level = pUser->level;
    send.exploit = pUser->dwExploit;
    send.useJob = pUser->dwUseJob;
  }
  send.nRight = mnRight;
  send.byStatus = status;
  send.wdOccupation = wdOccupation;
  strncpy(send.name,name,MAX_NAMESIZE);
  strncpy(send.aliasName,aliasname,MAX_NAMESIZE);
  if (pUser) wdOccupation = pUser->occupation;
  
   rwlock.unlock();


  if (mySept && mySept->master) 
  {
    if (mySept->master->id == this->id && pUser)
    {
      mySept->dwCountryID = pUser->country;
      mySept->writeDatabase();
   }
    mySept->sendCmdToAllMember(&send,sizeof(send));
  }
  else
  {
    Zebra::logger->error("�ڼ����Ա%s�Ҳ����Լ��ļ������",send.name);
  }
}

/**
* \brief ��Ա���ߴ���
*/
void CSeptMember::offline()
{
  Cmd::stBroadcastSeptMemberInfo send;
  
   rwlock.wrlock();
   byStatus = CSeptMember::Offline; 
//   user = NULL;
  send.byStatus = Cmd::SEPT_MEMBER_STATUS_OFFLINE;
  strncpy(send.name,name,MAX_NAMESIZE);
   rwlock.unlock();
  if (mySept) mySept->sendCmdToAllMember(&send,sizeof(send));
  else
  {
    Zebra::logger->error("�ڼ����Ա%s�Ҳ����Լ��ļ������",send.name);
  }
}

/** 
* \brief ֪ͨ�����Առ��NPC����Ϣ
* \param status ��Ա������״̬
*/
void CSeptMember::notifyNpcHoldData()
{
   UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);

  if (pUser)
  {
    CNpcDareM::getMe().sendUserData(pUser);
  }
}

void CSeptMember::sendGoldToMember(DWORD userID,DWORD num)
{
   UserSession *pUser = UserSessionManager::getInstance()->getUserByID(id);

  if (pUser)
  {
    Cmd::Session::t_notifyAddIntegral send;
    send.dwGoldUser = userID;
    send.dwUserID = id;
    send.dwNum = num;
    pUser->scene->sendCmd(&send,sizeof(send));
  }
}

//---------------------------------------new---------------------------------------------

/**
* \brief ɾ��ʵ���������
* \param name ʵ������
*/
void CSept::removeEntryByName(const char * name)
{
   zEntry *rm=getEntryByName(name);
   removeEntry(rm);
}

/**
* \brief �����ʼ��,���ݴ������ݳ�ʼ������Ļ�����Ϣ
* \param info �������ݽṹ
*/
void CSept::init(const stSeptInfo & info)
{
  rwlock.wrlock();    
  id = info.dwSeptID;
  strncpy(name,info.name,MAX_NAMESIZE);
  byVote = info.byVote;
  tempid = info.dwCharID;
  dwCreateTime = info.dwCrTime;
  dwRepute = info.dwRepute;
  dwCountryID = info.dwCountryID;
  dwUnionID = info.dwUnionID;
  dwLevel = info.dwLevel;
  dwSpendGold = info.dwSpendGold;
  dwIsExp = info.dwIsExp;
  strncpy(note,info.note,255);
  master = NULL;
  calltimes = info.calltimes;
  calldaytime = info.calldaytime;
  normal_exp_time = info.normalexptime;


//[Shx Add] ��ʼ������ ְλ+Ȩ��;
  ZeroMemory(RightList, sizeof(stStepRight) * 10 );
  std::string str = info.ExterData;
  std::string ts = "";
  for(int i = 0; i < 10; i ++)
  {
	  int pos = str.find_first_of(';');
	  if(pos == -1 )
	  {
		  break;
	  }
	  ts = str.substr(0, pos);	  
	  sscanf(ts.c_str(), "%[^,],%X", RightList[i].RightName, &(RightList[i].dwRight));
	  str.erase(0, pos + 1 );
  }
//End Shx;

  rwlock.unlock();
}

/**
* \brief ����һ������İ�����Ա,����Ȩ��
* \param info �����Ա���ݽṹ
*/
CSeptMember * CSept::addSeptMaster(const stSeptInfo& info)
{
  stSeptMemberInfo masterInfo;
//Shx Modify ��ʼ���᳤��Ϣ...
  masterInfo.dwCharID = info.dwCharID;
  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(masterInfo.dwCharID);
  if (pUser) masterInfo.wdOccupation = pUser->occupation;
  strncpy(masterInfo.name,info.masterName,MAX_NAMESIZE);
  strncpy(masterInfo.aliasname,"�᳤",MAX_NAMESIZE);
  masterInfo.nRight = 0;
  return addSeptMember(masterInfo);  

}

/**
* \brief ��������ʼ�������Ա
* \param info �����Ա���ݽṹ
* \return �ɹ������´����ļ����Ա����,ʧ�ܷ���NULL
*/
CSeptMember * CSept::addSeptMember(const stSeptMemberInfo& info)
{
  CSeptMember *pMember = NULL;
  
  pMember = new CSeptMember();
  pMember->init(info);
  if (pMember)
  {
    pMember->setSept(this);
    rwlock.wrlock();
    addEntry(pMember);
    rwlock.unlock();

    CSeptM::getMe().addMemberIndex(info.name,pMember);  // ��CSeptM�м���������������ʹ����ķ�����ͨ��CSeptM�ҵ��Լ���
  }
  return pMember;
}

/**
* \brief �����ʽ�����ĺ�
*/
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
* \brief ֪ͨ����ս���
* \param  msg ... ֪ͨ����
*/
void CSept::notifyWarResult(const char* msg,...)
{
   char buf[1024+1];
   bzero(buf,sizeof(buf));
   
   struct findall : public execEntry<CSeptMember> 
   {
       const char* msg;

       findall(const char* pMsg)
       {
     msg = pMsg;
       }

       ~findall(){}
       
       bool exec(CSeptMember* pMember)
       {
      UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
      if (pUser)
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_SYS,msg);
      }
       return true;
       }
   };

   getMessage(buf,1024,msg);

   findall myList(buf);
   execEveryOne(myList);

}

/**
  * \brief ���ͼ���֪ͨ
  *
  * \param message ��Ϣ
  *
  */
void CSept::sendSeptNotify(const char* message,...)
{
  if (message == NULL)
  {
    return;
  }

  char buf[1024+1];
  bzero(buf,sizeof(buf));
  getMessage(buf,1024,message);

  Cmd::stChannelChatUserCmd send;
//  send.dwType=Cmd::CHAT_TYPE_FAMILY;
  send.dwType = Cmd::CHAT_TYPE_SYSTEM;
  send.dwSysInfoType = Cmd::INFO_TYPE_EXP;
  bzero(send.pstrName,sizeof(send.pstrName));
  bzero(send.pstrChat,sizeof(send.pstrChat));

  sprintf((char*)send.pstrName,"����֪ͨ");
  sprintf((char*)send.pstrChat,"%s",buf);

  this->sendCmdToAllMember(&send,sizeof(send));

}

bool  CSept::isMember(DWORD dwUserID)
{
  CSeptMember * pMember = NULL;
  pMember = (CSeptMember *)getEntryByID(dwUserID);

  if (pMember)
  {
    return true;
  }

  return false;
}

void CSept::delSeptAllMember()
{
  rwlock.rdlock();
  
  Zebra::logger->info("[����]����������г�Ա:%s",this->name);
  std::vector<DWORD> vMemberID;

  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSeptMember *temp=(CSeptMember *)it->second;
    if (temp && temp->id != this->tempid)
    {
      vMemberID.push_back(temp->id);
    }
  }
  rwlock.unlock();

  for (std::vector<DWORD>::iterator vIter = vMemberID.begin(); vIter!=vMemberID.end(); vIter++)  
  {
    
     CSeptM::getMe().processMemberLeave(this->id,*vIter);
  }
  
  this->writeDatabase();
}

/**
* \brief ����������������г�Ա
* \param pNullCmd ����ս��״̬֪ͨ��Ϣ
* \param cmdLen ��Ϣ����
*/
void CSept::sendNpcDareCmdToScene(Cmd::Session::t_NpcDare_NotifyScene_SceneSession* pCmd,const DWORD cmdLen)
{
    struct findall : public execEntry<CSeptMember>
    {
    Cmd::Session::t_NpcDare_NotifyScene_SceneSession* cmd;
    DWORD cmdLen;

    findall(Cmd::Session::t_NpcDare_NotifyScene_SceneSession* ptCmd,const DWORD nCmdLen)
    {
      cmd = ptCmd;
      cmdLen = nCmdLen;
    }

    ~findall(){}
    bool exec(CSeptMember* pMember)
    {
      if (pMember)
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
        if (pUser && pUser->scene)
        {
          cmd->dwUserID = pUser->id;
          pUser->scene->sendCmd(cmd,cmdLen);
        }
      }
      return true;
    }
  };

    findall myList(pCmd,cmdLen);
    execEveryOne(myList);
}

/**
* \brief ��������������������г�Ա��ս��״̬��֪ͨ�ͻ���
* \param ptEnterWarCmd ����ս��״̬֪ͨ��Ϣ
* \param cmdLen ��Ϣ����
*/
void CSept::sendCmdToAllMemberScene(Cmd::Session::t_enterWar_SceneSession* ptEnterWarCmd,const DWORD cmdLen)
{
    struct findall : public execEntry<CSeptMember>
    {
  Cmd::Session::t_enterWar_SceneSession* cmd;
  DWORD cmdLen;

  findall(Cmd::Session::t_enterWar_SceneSession* ptCmd,const DWORD nCmdLen)
  {
      cmd = ptCmd;
      cmdLen = nCmdLen;
  }

  ~findall(){}
  bool exec(CSeptMember* pMember)
  {
    if (pMember)
    {
      UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
      if (pUser && pUser->scene)
      {
        cmd->dwUserID = pUser->id;
        cmd->dwSceneTempID = pUser->scene->tempid;

        if (cmd->dwStatus)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ѽ�������ս״̬��");
        }
        else
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����˳������ս״̬��");
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
void CSept::sendCallCmdToAllMemberScene(Cmd::Session::t_GoTo_Leader_Check_SceneSession* ptCmd,const DWORD cmdLen,DWORD expect)
{
    struct findall : public execEntry<CSeptMember>
  {
    Cmd::Session::t_GoTo_Leader_Check_SceneSession* cmd;
    DWORD cmdLen;
    DWORD expect;

    findall(Cmd::Session::t_GoTo_Leader_Check_SceneSession* ptCmd,const DWORD nCmdLen,DWORD exp)
    {
      cmd = ptCmd;
      cmdLen = nCmdLen;
      expect = exp;
    }

    ~findall(){}
    bool exec(CSeptMember* pMember)
    {
      if (pMember)
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
        if (pUser && pUser->scene)
        {
          if (pUser->tempid != expect)
          {
            cmd->userTempID = pUser->tempid;
            pUser->scene->sendCmd(cmd,cmdLen);
          }
        }
      }

      return true;
    }
  };

    findall myList(ptCmd,cmdLen,expect);
    execEveryOne(myList);

}

/**
* \brief ������������еĳ�Ա���ͻ��ˣ�
* \param ptCmd ��Ϣ��
* \param nCmdLen ��Ϣ����
*/
void CSept::sendCmdToAllMember(const Cmd::stNullUserCmd * ptCmd,const DWORD nCmdLen )
{
  struct findall : public execEntry<CSeptMember>
  {
    const Cmd::stNullUserCmd * cmd;
    DWORD cmdLen;
    findall(const Cmd::stNullUserCmd * ptCmd,const DWORD nCmdLen)
    {
      cmd = ptCmd;
      cmdLen = nCmdLen;
    }
    ~findall(){}

    bool exec(CSeptMember *pMember)
    {
      if (pMember)
      {
        pMember->sendCmdToMe(cmd,cmdLen);
      }

      return true;
    }
  };

//  if (1==byVote) return;
  findall myList(ptCmd,nCmdLen);
  execEveryOne(myList);
}

/**
* \brief ������������еĳ�Ա���ͻ��ˣ�
* \param ptCmd ��Ϣ��
* \param nCmdLen ��Ϣ����
*/
void CSept::sendCmdToAllMemberExcept(const Cmd::stNullUserCmd * ptCmd,const DWORD nCmdLen,const char * except)
{
  struct findall : public execEntry<CSeptMember>
  {
    const Cmd::stNullUserCmd * cmd;
    DWORD cmdLen;
    const char * except;
    findall(const Cmd::stNullUserCmd * ptCmd,const DWORD nCmdLen,const char * e)
    {
      cmd = ptCmd;
      cmdLen = nCmdLen;
      except = e;
    }
    ~findall(){}

    bool exec(CSeptMember *pMember)
    {
      if (pMember && strncmp(pMember->name,except,MAX_NAMESIZE))
      {
        pMember->sendCmdToMe(cmd,cmdLen);
      }

      return true;
    }
  };

  if (1==byVote) return;
  findall myList(ptCmd,nCmdLen,except);
  execEveryOne(myList);
}

/**
* \brief ���徭����������͵�����
* \param dwUserID �����ṩ�ߵ�ID
* \param ptCmd ���������Ϣ
*/
void CSept::sendDistributeSeptExpToScene(const DWORD dwUserID,const Cmd::Session::t_distributeSeptExp_SceneSession * ptCmd)
{
  struct findAllExceptMe : public execEntry<CSeptMember>
  {
    Cmd::Session::t_distributeSeptExp_SceneSession send;
    const Cmd::Session::t_distributeSeptExp_SceneSession * cmd;
    DWORD userid;
    findAllExceptMe(const DWORD dwUserID,const Cmd::Session::t_distributeSeptExp_SceneSession * ptCmd)
    {
      cmd = ptCmd;
      userid = dwUserID;
    }
    ~findAllExceptMe(){}

    bool exec(CSeptMember *pMember)
    {
      if (pMember->id != userid)
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
        if (pUser && pUser->scene)
        {
#ifdef _DEBUG
          Zebra::logger->debug("���;������֪ͨ��%s����Ϊ%u",pUser->name,cmd->dwExp);
#endif
          pUser->septExp +=cmd->dwExp;
          if (pUser->septExp >=30)
          {
            send.dwUserID = pMember->id;
            send.dwExp = pUser->septExp;
            pUser->septExp=0;
            pUser->scene->sendCmd(&send,sizeof(send));
          }
        }
      }
      return true;
    }
  };

  if (1==byVote) return;
  findAllExceptMe myList(dwUserID,ptCmd);
  execEveryOne(myList);
}


/**
* \brief ֪ͨ�������г�Ա������
* \param pName �������ߵ�����
*/
void CSept::notifyMemberFire(const char * pName)
{
  Cmd::stBroadcastSeptMemberInfo send;

  send.byStatus = Cmd::SEPT_MEMBER_STATUS_FIRE;
  strncpy(send.name,pName,MAX_NAMESIZE);
  sendCmdToAllMember(&send,sizeof(send));
}

/**
* \brief �����û��ļ������ݵ��ͻ���
* \param pName ������Ϣ���û�
*/
void CSept::sendUserSeptData(const char *pName)
{
  CSeptMember *pMember;

  pMember = (CSeptMember *)getEntryByName(pName);
  if (pMember)
  {
    pMember->sendUserSeptData();
  }
}

/**
* \brief ���ͼ�����Ϣ��ָ���û��Ŀͻ���,�����û����߳�ʼ����ʱ��
* \param pUser �����û�
*/
void CSept::sendSeptInfoToUser(UserSession *pUser)
{
  Cmd::stSeptBaseInfoCmd retSept;
 
  rwlock.rdlock();
  strncpy(retSept.septName,name,MAX_NAMESIZE);     // ��������
  strncpy(retSept.master,master->name,MAX_NAMESIZE); // ����᳤
  
  if (master->id == pUser->id) 
    retSept.dwMasterTempID = pUser->tempid;
  else
    retSept.dwMasterTempID = 0;
  
  retSept.dwRepute = this->getRepute();
  retSept.dwLevel = this->dwLevel;
  strncpy(retSept.note,this->note,sizeof(retSept.note));
  bcopy(this->RightList, retSept.RightList, sizeof(stStepRight) * 10,  sizeof(stStepRight) * 10);
  rwlock.unlock();
  pUser->sendCmdToMe(&retSept,sizeof(retSept));
}

void CSept::full_SeptMember_Data(Cmd::stUnionRember*& tempPoint,DWORD& count)
{
  rwlock.rdlock();
  for (zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSeptMember *temp=(CSeptMember *)it->second;
    UserSession *pUser = UserSessionManager::getInstance()->getUserByID(temp->id);

    strncpy(tempPoint->memberName,temp->name,MAX_NAMESIZE);
    strncpy(tempPoint->aliasname,temp->aliasname,MAX_NAMESIZE);
    tempPoint->byOnline = temp->byStatus;
    tempPoint->occupation = temp->wdOccupation;
    strncpy(tempPoint->septName,this->name,MAX_NAMESIZE);

    if (pUser)
    {
      tempPoint->exploit = pUser->dwExploit;
      tempPoint->level = pUser->level;
    }
    tempPoint++;
    count++;
  }

}

CSeptMember* CSept::getMemberByName(const char* pName)
{
  CSeptMember *pSeptMember=NULL;
    
  if (pName)
  {
    pSeptMember = (CSeptMember *)getEntryByName(pName);
  }

  return pSeptMember;
}

/**
* \brief ���ͼ����Ա�б�,�������������,���ֻ��100�������Ա��¼��
* \param pUser ���ݽ����߶���
*/
void CSept::sendSeptMemberList(UserSession *pUser)
{
  BYTE buf[zSocket::MAX_DATASIZE];
  DWORD count;
  Cmd::stSeptRember *tempPoint;

  rwlock.rdlock();
  Cmd::stAllSeptMemberCmd *retCmd=(Cmd::stAllSeptMemberCmd *)buf;
  constructInPlace(retCmd);
  tempPoint = (Cmd::stSeptRember *)retCmd->memberList;
  DWORD len = sizeof(Cmd::stSeptRember);
  count = 0;
  for (zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    struct Cmd::stSeptRember mInfo;
    CSeptMember *temp=(CSeptMember *)it->second;

	temp->getMemberBaseInfo(mInfo);
	bcopy(&mInfo,tempPoint,len,sizeof(buf));

    tempPoint++;
    count++;
    if (1000==count)		//[Shx ����] ����100����1000?
    {
      goto breakfor; // ����¼����100��ʱ��ᳬ������͵��������
    }
  }
breakfor:
  rwlock.unlock();
  retCmd->size = count;
  pUser->sendCmdToMe(retCmd,(count*sizeof(Cmd::stSeptRember)+sizeof(Cmd::stAllSeptMemberCmd)));
}

/**
* \brief ���������Ա
* \param master ��ʹȨ���ߵ����� 
* \param member �������ߵ�����
* \return 
*/
void  CSept::fireSeptMember(const char * master,const char *member)
{
  CSeptMember *pMaster = (CSeptMember *)getEntryByName(master);
  CSeptMember *pMember = (CSeptMember *)getEntryByName(member);
  if (pMaster)
  {
    if (pMember)
    {
      if (pMaster->id == pMember->id)
      {
        pMaster->sendMessageToMe(Cmd::INFO_TYPE_FAIL,"���ܿ����Լ�");
        return;
      }

      if (pMaster->id == this->master->id) //ֻ�л᳤���ʸ�
      {
        removeEntryByName(member);
        pMember->fireMe();
        SAFE_DELETE(pMember);
      }
      else
      {
        pMaster->sendMessageToMe(Cmd::INFO_TYPE_FAIL,"��û�п�����Ա��Ȩ��,����㷢�ִ����뱨���GM");
      }
    }
    else
    {
      pMaster->sendMessageToMe(Cmd::INFO_TYPE_FAIL,"�����ڵļ�����û�д���,��ȷ�������Ƿ���ȷ");
    }
  }
  else
  {
    Zebra::logger->debug("���������Ա�����г��ִ����״̬");
  }
}

/**
* \brief ֱ�ӿ��������Ա
* \param dwSeptID �������ļ����ԱID
* \return 1 �ɹ� 2 ʧ��
*/
int CSept::fireSeptMemberDirect(const DWORD dwCharID,const bool checkunion)
{
  int ret = 0;

  rwlock.wrlock();
  CSeptMember * pMember = NULL;
  pMember = (CSeptMember *)getEntryByID(dwCharID);
  removeEntry(pMember);
  rwlock.unlock();
  if (pMember)
  {
    pMember->fireMe(true,checkunion);
    SAFE_DELETE(pMember);
    ret = 1;
  }
  else
  {
    ret = 2;
  }

  return ret;
}

/**
* \brief �������ݿ��¼
*/
void CSept::writeDatabase()
{
  const dbCol sept_update_define[] = {
    { "`CHARID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`VOTE`",      zDBConnPool::DB_BYTE,  sizeof(BYTE) },
    { "`REPUTE`",               zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`COUNTRYID`",               zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`UNIONID`",               zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`LEVEL`",               zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`SPENDGOLD`",               zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`ISEXP`",               zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`NOTE`",      zDBConnPool::DB_STR,  sizeof(char[254+1])},
    { "`CALLTIMES`",      zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`CALLDAYTIME`",      zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`NORMALEXPTIME`",      zDBConnPool::DB_DWORD, sizeof(DWORD) },
	{ "EXTERDATA",		zDBConnPool::DB_STR, sizeof(char[512]) },
    { NULL,0,0}
  };
#pragma pack(1)
  struct {
    DWORD dwCharid;
    BYTE  byVote;
    DWORD dwRepute;
    DWORD dwCountryID;
    DWORD dwUnionID;
    DWORD dwLevel;
    DWORD dwSpendGold;
    DWORD dwIsExp;
    char  note[254+1];
    DWORD calltimes;
    DWORD calldaytime;
    DWORD normalexptime;
	char  ExtraData[512];
  }
  updatesept_data;
#pragma pack(1)
  char where[128];

  if (!master) return;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  bzero(&updatesept_data,sizeof(updatesept_data));
  rwlock.rdlock();  

  if (master)
  {
    updatesept_data.dwCharid = master->id;
  }

  updatesept_data.dwLevel = this->dwLevel;
  updatesept_data.dwUnionID = this->dwUnionID;
  updatesept_data.dwCountryID = this->dwCountryID;
  updatesept_data.dwRepute = this->dwRepute;
  updatesept_data.byVote = byVote;
  strncpy(updatesept_data.note,note,255);
  updatesept_data.dwSpendGold = dwSpendGold;
  updatesept_data.dwIsExp = this->dwIsExp;
  updatesept_data.calltimes = this->calltimes;
  updatesept_data.calldaytime = this->calldaytime;
  updatesept_data.normalexptime = this->normal_exp_time;
  bzero(updatesept_data.ExtraData, 512);
  //Shx Add...Ȩ��...
  for(int i = 0; i <  10; i ++)
  {
	  char _ts[64] = {0};
	  _snprintf(_ts, 50, "%s,%u;", this->RightList[i].RightName, this->RightList[i].dwRight);
	  strcat(updatesept_data.ExtraData, _ts);
  }

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"`SEPTID` = %u",id);
  rwlock.unlock();
  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`SEPT`",sept_update_define,(BYTE*)(&updatesept_data),where);
  SessionService::dbConnPool->putHandle(handle);

  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("CSept �޸Ļ�Ա����ʧ�ܣ�SEPTID=%u retcode=%u",id,retcode);
  }
}

/**
* \brief �������,��ʼ������
*/
CSept::CSept()
{
  destroy = false;
  byVote  = 0;
  dwIsExp = 0;
  calltimes=0; 
  calldaytime=0; 
}

/**
* \brief �������弰�����г�Ա
*/
CSept::~CSept()
{
  if (!destroy)
  {
    //writeDatabase();

    rwlock.wrlock();
    for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
    {
      CSeptMember *temp = (CSeptMember *)it->second;
      SAFE_DELETE(temp);
    }
    clear();
    rwlock.unlock();
  }
}

/**
* \brief ��ɢ����,�����г�Ա�ļ����ϵ������������Լ�
*/
void CSept::disbandSept()
{
  rwlock.wrlock();
  
  Zebra::logger->info("[����]:%s �����ɢ",this->name);
  destroy = true;
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSeptMember *temp=(CSeptMember *)it->second;
    if (temp)
    {
      temp->fireMe(false);
      SAFE_DELETE(temp);
    }
  }
  clear();
  deleteMeFromDB();
  rwlock.unlock();
}

/**
* \brief ɾ������������ݿ��¼
* \return true �ɹ� false ʧ��
*/
bool CSept::deleteMeFromDB()
{
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"SEPTID = %u ",id);
  DWORD retcode = SessionService::dbConnPool->exeDelete(handle,"`SEPT`",where);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("ɾ������ʧ�� %u",id);
    return false;
  }
  else
  {
    destroy = true;
  }
  return true;
}

/**
* \brief �����ݿ��м��ؼ����Ա
* \return true ���سɹ�,false ����ʧ��
*/
bool CSept::loadSeptMemberFromDB()
{
  static const dbCol septmember_define[] = {
    { "CHARID",			zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "NAME",			zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "ALIASNAME",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "`OCCUPATION`",	zDBConnPool::DB_WORD,  sizeof(WORD) },	
//[Shx Add ��Աְλ]
	{ "MBRIGHT",		zDBConnPool::DB_WORD,  sizeof(WORD) },
    { NULL,0,0}
  };

  stSeptMemberInfo *memberList,*tempPoint;
  stSeptMemberInfo info;
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"SEPTID = %u",id);
  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`SEPTMEMBER`",septmember_define,where,NULL,(BYTE **)&memberList);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode
      || 0 == retcode)
  {
    Zebra::logger->debug("û���ҵ������Ա��¼");
    return true;
  }

  if (memberList)
  {
    tempPoint = &memberList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      bcopy(tempPoint,&info,sizeof(stSeptMemberInfo),sizeof(stSeptMemberInfo));
      CSeptMember *member = addSeptMember(info);
      if (info.dwCharID == tempid) 
      {
        //Zebra::logger->info("[�������]:%s(%d) �峤���سɹ�",member->name,this->id);
        master = member; // masterid��������tempid�У������ʼ��master����
      }
      tempPoint++;
    }
    SAFE_DELETE_VEC(memberList);
  }
  else
  {
    Zebra::logger->error("�������ݳ�ʼ��ʧ��,exeSelect ������Чbufָ��");
  }

  if (this->master==NULL)
  {
    this->byVote = 1;
    Zebra::logger->info("[�������]:(%d) �峤����ʧ��",this->id);
    return false;
  }
  
  return true;
}

/**
* \brief ��ȡ�����Ա��Ŀ
* \return �����Ա����
*/
DWORD CSept::size()
{
  return zEntryName::size();
}

/**
* \brief �ж��Ƿ���ͶƱ״̬
* \return true ����ͶƱ�� false ͶƱ�ѽ���
*/
bool CSept::isVote()
{
  return 1==byVote;
}

/**
* \brief ͶƱ��������
*/
void CSept::letVoteOver()
{
  struct findList : public execEntry<CSeptMember>
  {
    findList()
    {
    }
    ~findList(){}
    bool exec(CSeptMember *pMember)
    {
//Shx Remove ����ͶƱϵͳ
//       UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
//       if (pUser)
//       {
//         //pMember->online();
//         pMember->sendUser.........SeptData();
//         pMember->update_data();
//         pUser->septid = pMember->mySept->id;
//       }
      return true;
    }
  };

  byVote = 0;
  findList myList;
  execEveryOne(myList);
  writeDatabase();
}

/**
* \brief ���ü���Ľ��ܣ�����⣩
* \param pCmd ����������Ϣ
*/
void CSept::setNote(Cmd::stNoteSeptCmd *pCmd)
{
  rwlock.wrlock();
  strncpy(note,pCmd->noteBuf,255);
  rwlock.unlock();
  writeDatabase();
  sendCmdToAllMember(pCmd,sizeof(Cmd::stNoteSeptCmd));
}

DWORD CSept::getRepute()
{
  if (this->dwUnionID>0)
  {
    CUnion* pUnion = CUnionM::getMe().getUnionByID(this->dwUnionID);
    if (pUnion)
    {
      // TODO:�ж��Ƿ��ǳ�������� 
      if (pUnion->master && this->master && (pUnion->master->id == this->master->id))
      {//�����峤�ǰ���
        if (CCityM::getMe().findByUnionID(pUnion->id) !=NULL)
        {//�ǹ��������
          return (DWORD)(this->dwRepute*1.1);
        }
      }
    }
  }

  return this->dwRepute;
}

void CSept::sendSeptReputeToAll()
{
  rwlock.rdlock();
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSeptMember *pMember=(CSeptMember *)it->second;
    if (pMember)
    {
      UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
      if (pMember->mySept && pUser)
      {
        pMember->mySept->sendSeptInfoToUser(pUser); // ���ͼ������Ϣ����ǰ��Ա
      }

      pMember->update_normal_data();
    }
    
  }
  rwlock.unlock();
}

void CSept::sendSeptInfoToAll()
{
  rwlock.rdlock();
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSeptMember *pMember=(CSeptMember *)it->second;
    if (pMember)
    {
      pMember->sendUserSeptData();
      pMember->update_data();
    }
  }
  rwlock.unlock();
}

//[Shx Delete ���������ı�....��ʱ����Ҫ..]
void CSept::changeRepute(int repute)
{  
	/*
  if (this->isVote()) return;
  if (repute>0)
  {
    this->dwRepute+=repute;
    this->sendSeptNotify("�������������� %d ��",repute);
  }
  else
  {
    this->sendSeptNotify("�������������� %d ��",::abs(repute));
    
    if ((int)this->dwRepute>::abs(repute))
    {
      this->dwRepute = (int)this->dwRepute - ::abs(repute);
    }
    else
    {
      this->dwRepute = 0;
    }
  }

  this->sendSeptReputeToAll();
  if (this->dwUnionID>0)
  {
    CUnion* pUnion = CUnionM::getMe().getUnionByID(this->dwUnionID);
    if (pUnion)
    {  
      pUnion->sendUnionManaToAll();
    }
  }

//  if (this->master) this->master->update_data();
  this->writeDatabase();

   */
}

void CSept::changeLevel(int level)
{
  if (this->isVote()) return;
  if (level>0)
  {
    this->dwLevel+=level;
  }
  else
  {
    if ((int)this->dwLevel>::abs(level))
    {
      this->dwLevel = (int)this->dwLevel - ::abs(level);
    }
    else
    {
      this->dwLevel = 0;
    }
  }

  this->sendSeptInfoToAll();
  this->writeDatabase();
 
}

/**
* \brief ֪ͨ�����Ա�µ�NPC��������
*/
void CSept::notifyNpcHoldData()
{
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSeptMember *temp=(CSeptMember *)it->second;
    if (temp)
    {
      temp->notifyNpcHoldData();
    }
  }
}

void CSept::sendGoldToMember(DWORD userID,DWORD num)
{
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSeptMember *temp=(CSeptMember *)it->second;
    if (temp)
    {
      temp->sendGoldToMember(userID,num);
    }
  }
}

//-------------------------------------------------------------------------------------

/**
* \brief ����������ʵ��
*/
CSeptM *CSeptM::um(NULL);

/**
* \brief ���������캯��
*/
CSeptM::CSeptM()
{
}

/**
* \brief ��������������
*/
CSeptM::~CSeptM()
{
  rwlock.wrlock();
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSept *temp =(CSept *)it->second;
    SAFE_DELETE(temp);
  }
  clear();
  rwlock.unlock();
}

/**
* \brief �����ͷŹ�����
*/
void CSeptM::destroyMe()
{
  SAFE_DELETE(um);
}

/**
* \brief �ڳ�Ա����������ҳ�Ա����
* return ����ָ��
*/
std::map<std::string,CSeptMember *>::iterator  CSeptM::findMemberIndex(const char *pName)
{
  char temp_name[MAX_NAMESIZE];
  bzero(temp_name,MAX_NAMESIZE);
  strncpy(temp_name,pName,MAX_NAMESIZE);
  return memberIndex.find(temp_name);
}

/**
* \brief ����ʵ������ɾ������ʵ��
* \param name ʵ�������
*/
void CSeptM::removeEntryByName(const char * name)
{
  zEntry *rm=getEntryByName(name);
  removeEntry(rm);
}

/**
  * \brief ������ȫ�����߳�Ա������Ϣ
  *
  * \param septID ����ID
  * \param message ��Ϣ����
  *
  */
void CSeptM::sendNpcDareCmdToScene(const DWORD septID,Cmd::Session::t_NpcDare_NotifyScene_SceneSession* ptCmd,DWORD nCmdLen)
{
  CSept *pSept = NULL;

  pSept = this->getSeptByID(septID);
  if (pSept)
  {       
    pSept->sendNpcDareCmdToScene(ptCmd,nCmdLen);
  }
  else
  {       
    Zebra::logger->error("�������������ȫ���Աʱ,δ�ҵ�����:%d",septID);
  }
}

/**
  * \brief ���ͼ���֪ͨ
  *
  * \param septID ����ID
  * \param message ��Ϣ����
  *
  */
void CSeptM::sendSeptNotify(const DWORD septID,const char* message,...)
{
  CSept *pSept = NULL;
  char buf[1024+1];
  bzero(buf,sizeof(buf));

  if (message==NULL)
  {
    return;
  }

  pSept = this->getSeptByID(septID);
  getMessage(buf,1024,message);


  Cmd::stChannelChatUserCmd send;
  send.dwType=Cmd::CHAT_TYPE_SYSTEM;
  send.dwSysInfoType = Cmd::INFO_TYPE_EXP;
  bzero(send.pstrName,sizeof(send.pstrName));
  bzero(send.pstrChat,sizeof(send.pstrChat));

  sprintf((char*)send.pstrName,"����֪ͨ");
  sprintf((char*)send.pstrChat,"%s",buf);

  if (pSept)
  {       
    pSept->sendCmdToAllMember(&send,sizeof(send));
  }
  else
  {       
    Zebra::logger->error("���ͼ���֪ͨʱ,δ�ҵ�����:%d",septID);
  }
}

/**
* \brief ��ʼ�����������,�����ݿ��м������еļ�������
* \return true ���سɹ�  false ����ʧ��
*/
bool CSeptM::init()
{
  /*
     Cmd::Record::t_GetSept_SeptRecord tCmd;
     return RecordClientManager::getInstance().get().sendCmd(&tCmd,sizeof(tCmd));
   */
  const dbCol sept_read_define[] = {
    { "SEPTID",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "NAME",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "CHARID",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "MASTER",      zDBConnPool::DB_STR,  sizeof(char[MAX_NAMESIZE+1]) },
    { "`VOTE`",      zDBConnPool::DB_BYTE,  sizeof(BYTE) },
    { "`CREATETIME`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`REPUTE`",                  zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`COUNTRYID`",               zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`UNIONID`",                 zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`LEVEL`",              zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`SPENDGOLD`",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`ISEXP`",      zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`NOTE`",      zDBConnPool::DB_STR,  sizeof(char[254+1]) },
    { "`CALLTIMES`",      zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`CALLDAYTIME`",      zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`NORMALEXPTIME`",      zDBConnPool::DB_DWORD, sizeof(DWORD) },
	{ "EXTERDATA",		zDBConnPool::DB_STR, sizeof(char[512]) },
    { NULL,0,0}
  };

  stSeptInfo *septList,*tempPoint;
  stSeptInfo info;

  septList = NULL;
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`SEPT`",sept_read_define,NULL,NULL,(BYTE **)&septList);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode
      || 0 == retcode)
  {
    return true;
  }
  if (septList)
  {
    tempPoint = &septList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      bcopy(tempPoint,&info,sizeof(stSeptInfo),sizeof(stSeptInfo));
      CSept *pSept = createSeptByDBRecord(info);
      if (!pSept->loadSeptMemberFromDB())
      {
//Shx Del Union���.
//         if (pSept->byVote && pSept->dwUnionID)
//         {
//           delSept(pSept->id);  // ������峤�ͽ�ɢ����
//         }			
      }

      tempPoint++;
    }
    SAFE_DELETE_VEC(septList);
    return true;
  }
  else
  {
    Zebra::logger->error("�������ݳ�ʼ��ʧ��,exeSelect ������Чbufָ��");
  }
  return false;
}

/**
* \brief ��ȡ�����������Ψһʵ��
* \return ���������ʵ��
*/
CSeptM &CSeptM::getMe()
{
  if (um==NULL)
  {
    um=new CSeptM();
  }
  return *um;
}

/**
* \brief ��ʼ�����������Ӱ�����Ա ��createNewUnion()���ô˷�����
* \param info ��������ݽṹ
* \return ��������ʼ���õļ������
*/
CSept* CSeptM::createSeptAndAddMaster(const stSeptInfo & info)
{
  CSept *pSept = NULL;
//Shx Modfify �������Ტ��ӻ᳤

  pSept = new CSept();
  pSept->init(info);
  pSept->master = pSept->addSeptMaster(info);
  pSept->master->insertDatabase();

  rwlock.wrlock();
  addEntry(pSept);
  rwlock.unlock();

  if (pSept->master)
  {
	  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pSept->master->id);
	  if (pUser)
	  {
		  pUser->septid = pSept->id;
	  }

	  //[Shx] Ȼ���ٷ��û�������Ϣ...
	  pSept->master->sendUserSeptData();
	  pSept->master->online(Cmd::SEPT_MEMBER_STATUS_NEWMEMBER); 
	  pSept->master->update_data(); 
  }  




  Zebra::logger->info("[����]:%s(%u) ���� �� %s �����ɹ�",pSept->name,pSept->id,pSept->master->name);
  SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_EXP,pSept->dwCountryID,"��ϲ%s������%s����",pSept->master->name,pSept->name);
  return pSept;
}

/**
* \brief �������ݿ��¼�����������,ϵͳ���ص�ʱ��ʹ��
* \param info ��������ݽṹ
* \return �����ɹ��ļ������ָ��
*/
CSept* CSeptM::createSeptByDBRecord(const stSeptInfo & info)
{
  CSept *pSept = NULL;
  bool ret = false;

  pSept = new CSept();
  pSept->init(info);
  rwlock.wrlock();
  ret = addEntry(pSept);
  rwlock.unlock();

  if (!ret)
  {
    Zebra::logger->info("[����]: %s(%u) ������ӽ�CSeptM������ʧ��",pSept->name,pSept->id);
  }
  
  return pSept;
}

/**
* \brief ����ָ���ļ����Ա
* \param master ��ʹ����Ȩ�ļ����Ա,һ���ǻ᳤
* \param member �������ĳ�Ա
* \return 
*/
void CSeptM::fireSeptMember(UserSession * master,const char * member)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember=NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(master->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember) 
  {
    CSept *pSept = pSeptMember->mySept;
    if (pSept)
    {
      pSept->fireSeptMember(master->name,member);
    }
  }
  else
  {
    master->sendSysChat(Cmd::INFO_TYPE_FAIL,"��û�м������");
  }
}

int CSeptM::fireSeptMember(DWORD dwUserID,bool find)
{
  struct findList : public execEntry<CSept>
  {
    DWORD _dwUserID;
    int _status;
    CSept* _pSept;

    findList(DWORD dwUserID)
    {
      _dwUserID = dwUserID;
      _status = 2;
      _pSept = NULL;
    }
    ~findList(){}

    void removeSeptMember()
    {
      if (_status == 1)
      {
#ifdef _DEBUG
        Zebra::logger->debug("%d ����Ա,���������ϵ",_dwUserID);
#endif
        if (_pSept)
        {
          _status = _pSept->fireSeptMemberDirect(_dwUserID);
        }
        else
        {
          _status = -1;
        }
      }

      if (_status == 3)
      {
        if (_pSept)
        {
          CSeptM::getMe().delSept(_pSept->id);
        }
        else
        {
          _status = -1;
        }
        
        //_status = -1;
      }
    }

    bool exec(CSept *pSept)
    {
      if (pSept)
      {
        if (pSept->master)
        {
          if (pSept->master->id == _dwUserID)
          {
#ifdef _DEBUG
            Zebra::logger->debug("%d���峤,���ܽ������ϵ",_dwUserID);
#endif
            _pSept = pSept;
            _status = 3;
            return false;
          }
          else
          {
            if (pSept->isMember(_dwUserID))
            {
#ifdef _DEBUG
              Zebra::logger->debug("%d �� %s ��Ա,�ܽ������ϵ",_dwUserID,pSept->name);
#endif
              _pSept = pSept;
              _status = 1;
              return false;
            }
            else
            {
#ifdef _DEBUG
              Zebra::logger->debug("%d ���� %s ����Ա",_dwUserID,pSept->name);
#endif
              _status = 2;
            }
          }
        }
        else
        {
          Zebra::logger->error("%s û���峤��Ϣ,��������Ϣ�������ԡ�",pSept->name);
        }
      }

      return true;
    }
  };

  findList myList(dwUserID);
  execEveryOne(myList);

  if (!find)
  {
    myList.removeSeptMember();
  }

  return myList._status;
}

/**
* \brief ���ݽ�ɫ����������Ͻ�ļ���
* \param dwUserID ��ɫID
* \return �����Ϊ����ID����Ϊ0
*/
DWORD CSeptM::findUserSept(DWORD dwUserID)
{
  struct findList : public execEntry<CSept>
  {
    DWORD _dwUserID;
    CSept* _pSept;

    findList(DWORD dwUserID)
    {
      _dwUserID = dwUserID;
      _pSept = NULL;
    }
    ~findList(){}

    bool exec(CSept *pSept)
    {
      if (pSept)
      {
        if (pSept->master)
        {
          if (pSept->master->id == _dwUserID)
          {
            _pSept = pSept;
            return false;
          }

        }
      }

      return true;
    }
  };

  findList myList(dwUserID);
  execEveryOne(myList);

  if (myList._pSept) return myList._pSept->getID();
  else return 0;
}

/**
* \brief ������м���ĳ�Ա
*/
void CSeptM::delSeptAllMember()
{
  struct findList : public execEntry<CSept>
  {
    std::vector<CSept*> vCSept;
    
    findList()
    {
    }
    ~findList(){}

    void removeAllMember()
    {
      for (std::vector<CSept*>::iterator vIter=vCSept.begin(); vIter!=vCSept.end(); vIter++)
      {
        (*vIter)->delSeptAllMember();
      }
    }
    
    bool exec(CSept *pSept)
    {
      if (pSept)
      {
        vCSept.push_back(pSept);
      }

      return true;
    }
  };

  findList myList;
  execEveryOne(myList);
  myList.removeAllMember();
}

/**
* \brief �����³�Ա������
* \param dwSeptID ����ID
* \param info �³�Ա����Ϣ�ṹ
* \return true �ɹ� false ʧ��
*/
bool CSeptM::addNewMemberToSept(const DWORD dwSeptID,const stSeptMemberInfo& info)
{
  CSept * pSept = NULL;

  rwlock.rdlock();
  pSept = (CSept *)getEntryByID(dwSeptID);
  rwlock.unlock();

  if (pSept)
  {
    if (pSept->master)
    {
      /*DWORD value = pSept->master->user->level - 50;
        value = (DWORD(value/5))*PER_LEVEL_MAN_NUM;
        value +=10;*/
      if (pSept->size()<CREATE_SEPT_NEED_MAN_NUM)//value)
      {
		  CSeptMember *pMember = pSept->addSeptMember(info);
		if (pMember)
		{
          Zebra::logger->info("[����]: %s ���� %s �������",pSept->name,pMember->name);
          pMember->insertDatabase();      
          pMember->sendUserSeptData();
		  pMember->online(Cmd::SEPT_MEMBER_STATUS_NEWMEMBER);

          UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
          if (pUser)
          {
            pUser->septid = pMember->mySept->id;

            pMember->update_data();
            return true;
          }
        }
      }
      else
      {
        UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pSept->master->id);
        if (pUser)  pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��Ŀǰ����������������%u��,�Ѿ����˲���������",CREATE_SEPT_NEED_MAN_NUM);//value);
      }
    }
    else
    {
      Zebra::logger->error("�޷�����Ա%u�������%u��",info.dwCharID,dwSeptID);
    }

  }
  else
  {
    Zebra::logger->error("���ݲ����������������û��%u����,����Ա%u�������ü���",dwSeptID,info.dwCharID);
  }
  return false;
}

/**
* \brief �û����ߴ���,��������û���ĳ������ĳ�Ա������Ӧ�����߳�ʼ��
* \param pUser �����û�
*/
void CSeptM::userOnline(UserSession * pUser)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  
  if (pSeptMember)
  {
//	[Shx Delete ��ҹ�������岻һ��,�������ѹ�,�������...���ǲ���Ҫ��.]
//     if (pSeptMember->mySept && pSeptMember->mySept->dwCountryID>1 && (pSeptMember->mySept->dwCountryID != pUser->country))
//     {
//       this->fireSeptMember(pUser->id,false);
//       return;
//     }
//End Delete;

	  //CDareM::getMe().userOnline(pUser);

    if(!pSeptMember->isOnline())
    {
  //    if (pSeptMember->mySept->isVote()) return;	//Shx Delete ����ͶƱ, ѡ����Ա��????;
      pSeptMember->sendUserSeptData();
	  pSeptMember->online();
    }

    pSeptMember->update_data();
    //if ((0 == pUser->septid) && pSeptMember->user)
    if (pUser)
    {
      pUser->septid = pSeptMember->mySept->id;
      if (pSeptMember->id == pSeptMember->mySept->tempid)
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����ߵ���ʮ��,���Դ�����Ա,��ȡ���徭��");
      }
    }
  }
  else
  {
    if (pUser->septid!=0)
    {
      Cmd::Session::t_fireSeptMember_SceneSession send;
      send.dwCharID = pUser->id;
      send.dwMapTempID = pUser->scene->tempid;
      pUser->scene->sendCmd(&send,sizeof(Cmd::Session::t_fireSeptMember_SceneSession)); /// ֪ͨ����������
      pUser->septid = 0;
    }
  }
}

/**
* \brief ��Ա���ߴ���,�����ж�ָ�����û��Ƿ��Ǽ����Ա,���������ߴ���
* \param pUser �����û�
*/
void CSeptM::userOffline(const UserSession * pUser)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    pSeptMember->offline();
  }
}

/**
* \brief �����µļ������
* \param data ���崴����Ϣ
*/
void CSeptM::createNewSept(Cmd::Session::t_addSept_SceneSession *data)
{
//[Shx Modify �򻯼�....]
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(data->info.masterName);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pSeptMember->id);
    if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�㲻���ٴ��������ˣ�");
    return;
  }

  zRTime ctv;
  data->byRetcode =0;     
  data->info.dwCrTime = ctv.sec();

  if (createSeptDBRecord(data->info))  /// �������ݿ��¼��������
  {
	  createSeptAndAddMaster(data->info);  /// ��ʼ������������еļ������
	  data->byRetcode =1;            /// ��������ʱ��0 ��ʾ����ʧ�������ظ�,1��ʾ�ɹ�
  }

 /// ��������ʱ��0 ��ʾ����ʧ�������ظ�,1��ʾ�ɹ� ֪ͨ����������
  //[Shx Modify ]
  SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByTempID(data->dwMapTempID);
  if (pScene)
  {
	  pScene->sendCmd(data,sizeof(Cmd::Session::t_addSept_SceneSession)); 
  }
}

/**
* \brief �����µļ������ݿ��¼
* \param info �µļ���ṹ��Ϣ
* \return true �ɹ�  false ʧ��
*/
bool CSeptM::createSeptDBRecord(stSeptInfo& info)
{
  static const dbCol createsept_define[] = {
    { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`MASTER`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { "`VOTE`",  zDBConnPool::DB_BYTE,  sizeof(BYTE) },
    { "`CREATETIME`",zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`REPUTE`",zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`COUNTRYID`",zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`UNIONID`",zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`LEVEL`",zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`SPENDGOLD`",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`ISEXP`",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`NORMALEXPTIME`",    zDBConnPool::DB_DWORD, sizeof(DWORD) },
    { "`NOTE`",  zDBConnPool::DB_STR,  sizeof(char[254+1]) },
	{ "EXTERDATA", zDBConnPool::DB_STR, sizeof(char[510])} ,
    { NULL,0,0}
  };

#pragma pack(1)
  struct {
    char name[MAX_NAMESIZE+1];
    DWORD charid;
    char master[MAX_NAMESIZE+1];
    BYTE vote;
    DWORD dwCrTime;
    DWORD dwRepute;
    DWORD dwCountryID;
    DWORD dwUnionID;
    DWORD dwLevel;
    DWORD dwSpendGold;
    DWORD dwIsExp;
    DWORD dwNormalExpTime;
    char note[254+1];
	char right[510];
  }

  createsept_data;
#pragma pack()
  static const dbCol verifyname_define[] = {
    { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { NULL,0,0}
  };
  
  static const dbCol verifymasterid_define[] = {
    { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { NULL,0,0}
  };
  
  char strName[MAX_NAMESIZE+1];
  DWORD dwMasterID;
  char where[128];
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  //������֤�����Ƿ��ظ�
  bzero(where,sizeof(where));
  std::string escapeName;
  _snprintf(where,sizeof(where) - 1,"NAME = '%s'",SessionService::dbConnPool->escapeString(handle,info.name,escapeName).c_str());
  DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`SEPT`",verifyname_define,where,NULL,1,(BYTE*)(strName));
  if (retcode == 1)
  {
    SessionService::dbConnPool->putHandle(handle);
    return false;
  }

  retcode = 0;
  // ��֤�峤�Ƿ��ظ�
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"CHARID = '%d'",info.dwCharID);
  retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`SEPT`",verifymasterid_define,
      where,NULL,1,(BYTE*)&dwMasterID);

  if (retcode == 1)
  {
    SessionService::dbConnPool->putHandle(handle);
    return false;
  }

  UserSession *pUser = NULL;
  pUser = UserSessionManager::getInstance()->getUserByID(info.dwCharID);
  
  //�������ݿ��ɫ��Ϣ
  bzero(&createsept_data,sizeof(createsept_data));
  
  if (pUser) 
  {
    createsept_data.dwCountryID = pUser->country;
  }       
  else
  {
    createsept_data.dwCountryID = 0;
  }
  
  createsept_data.dwLevel = 1;
  createsept_data.dwUnionID = 0;
  createsept_data.dwRepute = info.dwRepute;
  strncpy(createsept_data.name,info.name,MAX_NAMESIZE);
  strncpy(createsept_data.master,info.masterName,MAX_NAMESIZE);
  createsept_data.vote = info.byVote;
  createsept_data.charid = info.dwCharID;
  createsept_data.dwCrTime = info.dwCrTime;
  strncpy(createsept_data.note,info.note,255);
  createsept_data.dwIsExp = 0;
  createsept_data.dwNormalExpTime = 0;
  createsept_data.dwSpendGold = 0;
  strncpy(createsept_data.right, info.ExterData , 510);


  retcode = SessionService::dbConnPool->exeInsert(handle,"`SEPT`",createsept_define,(const BYTE *)(&createsept_data));
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("��������������ݿ���� %u,%s",info.dwCharID,info.name);
    return false;
  }

  info.dwSeptID = retcode;
  return true;
}

/**
* \brief ��ӳ�Ա����
* \param pName ���ӵĳ�Ա������
* \param pSeptMember ��Ա����
* \return true ��ӳɹ� false ���ʧ��
*/
bool CSeptM::addMemberIndex(const char *pName,CSeptMember *pSeptMember)
{
  std::pair<std::map<std::string,CSeptMember*>::iterator,bool> retval;
  char temp_name[MAX_NAMESIZE];
  bzero(temp_name,MAX_NAMESIZE);
  strncpy(temp_name,pName,MAX_NAMESIZE);

  rwlock.wrlock();
  retval = memberIndex.insert(memberIndexValueType(temp_name,pSeptMember));
  rwlock.unlock();
  
  if (!retval.second)
  {
    Zebra::logger->error("[����]: %s ��ӽ�memberIndexʧ��",pName);
    CSeptMember *pTemp = NULL;

    std::map <std::string,CSeptMember*>::iterator sIterator;
    rwlock.rdlock();
    sIterator = findMemberIndex(pName);
    if (sIterator != memberIndex.end()) pTemp = (*sIterator).second;
    rwlock.unlock();
    if (pTemp)
    {
      Zebra::logger->error("[����]: %s ռ���� %s ������λ��",pTemp->name,pName);
    }
    else
    {
      Zebra::logger->error("[����]: û����ռ��,��������� %s ʧ����",temp_name);
    }
  }
  else
  {
#ifdef _DEBUG    
      Zebra::logger->error("[����]: ��� %s �������memberIndex�ɹ�",temp_name);
#endif      
  }
  
  return retval.second;
}

/**
* \brief ɾ����Ա�������еļ�¼
* \param pName ��Ա����
* \return true ɾ���ɹ� false ɾ��ʧ��
*/
bool CSeptM::removeMemberIndex(const char *pName)
{
  bool ret;
  std::map <std::string,CSeptMember*>::iterator sIterator;
  rwlock.wrlock();
  ret = true;
  sIterator = findMemberIndex(pName);
  if (sIterator != memberIndex.end()) memberIndex.erase(sIterator);
  else ret = false;
  rwlock.unlock();
  return ret;
}

/**
* \brief ��ɢ����
* \param dwSeptID ����ɢ�ļ����ID
*/
void CSeptM::delSept(const DWORD dwSeptID)
{
  CSept *pSept = NULL;
  pSept = (CSept *)getEntryByID(dwSeptID);

  if (pSept)
  {
    //CDare* pDare = CDareM::getMe().findDareRecordByRelation(Cmd::SEPT_DARE,dwSeptID,0);
    //CDare* pNpcDare = CDareM::getMe().findDareRecordByRelation(Cmd::SEPT_NPC_DARE,dwSeptID,0);
    
    //if (pDare==NULL && pNpcDare==NULL)
    //{
      pSept->sendSeptNotify("%s �����ɢ",pSept->name);
      //SessionChannel::sendCountryInfo(Cmd::INFO_TYPE_EXP,pSept->dwCountryID,
      //    "���ź� %s �����ɢ",pSept->name);
      
      rwlock.wrlock();
      removeEntry(pSept);
      rwlock.unlock();
      
      pSept->disbandSept();
      SAFE_DELETE(pSept);
    //}
    //else
    //{
    //  UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pSept->master->id);
    //  if (pSept->master && pUser)
    //  {
    //    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���ڶ�ս״̬,�������ɢ����!");
    //  }  
    
    //}
  }
}

bool lessSeptSort(const CSeptSort& p1,const CSeptSort& p2)
{
  return p1.dwRepute > p2.dwRepute;
}

void CSeptM::processRequestSeptNormalExpMessage(UserSession* pUser,
    const Cmd::stRequestSeptNormalExpCmd* ptCmd)
{
  CSept *pSept = NULL;
  pSept = (CSept *)getEntryByID(pUser->septid);
  struct tm tv1;
  time_t timValue = time(NULL);
  zRTime::getLocalTime(tv1,timValue);

  
  if (pSept && !pSept->isVote())
  {
    if (pSept->master && pSept->master->id == pUser->id)
    {
#ifdef _DEBUG      
      if ((timValue - pSept->normal_exp_time)>20*60*60 && tv1.tm_hour>=9 && tv1.tm_hour<=22)
#else
      if ((timValue - pSept->normal_exp_time)>20*60*60 && tv1.tm_hour>=19 && tv1.tm_hour<=22)
#endif        
      {
        Cmd::Session::t_GetSeptNormalExp_SceneSession send;
        send.dwSeptID = pSept->id;
        send.dwUserID = pUser->id;

        if (pUser->scene)
        {
          pUser->scene->sendCmd(&send,sizeof(send));
          Zebra::logger->info("[����]: %s(%u) ��ȡ %s ��ͨ���徭��",
              pUser->name,pUser->id,pSept->name);
        }

        pSept->normal_exp_time = timValue;
        pSept->writeDatabase();
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��������ȡ,����19�㵽22��֮��");
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������峤������ȡ���徭��");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������峤������ȡ���徭��");
  }
}

void CSeptM::processRequestSeptExpMessage(UserSession* pUser,const Cmd::stRequestSeptExpCmd* ptCmd)
{
  CSept *pSept = NULL;
  pSept = (CSept *)getEntryByID(pUser->septid);
  
  if (pSept && !pSept->isVote())
  {
    if (pSept->master && pSept->master->id == pUser->id)
    {
      if (pSept->dwUnionID && CCityM::getMe().findByUnionID(pSept->dwUnionID) != NULL)
      {
        if (pSept->getExp())
        {
          Cmd::Session::t_GetSeptExp_SceneSession send;
          send.dwSeptID = pSept->id;
          send.dwUserID = pUser->id;
          if (pUser->scene)
          {
            pUser->scene->sendCmd(&send,sizeof(send));
            pSept->clearExp();
            Zebra::logger->info("[����]: %s(%u) ��ȡ %s ���徭��",
                pUser->name,pUser->id,pSept->name);
          }
        }
        else
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ļ��徭���Ѿ���ȡ��");
        }
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���Ǽ��廹û�����κΰ������ڰ�Ტδռ�����");
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������峤������ȡ���徭��");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������峤������ȡ���徭��");
  }
}

void CSeptM::processSeptSortMessage(UserSession* pUser,const Cmd::stReqSeptSort* ptCmd)
{
  struct findList : public execEntry<CSept>
  {
    DWORD _dwCountry;
    DWORD _byType;
    std::vector<CSeptSort> _vSept;
    
    findList(DWORD dwCountry,BYTE byType)
    {       
      _dwCountry = dwCountry;
      _byType = byType;
    }
    ~findList(){}

    void sendSeptSort(UserSession* pUser)
    {
      BYTE buf[zSocket::MAX_DATASIZE];
      Cmd::stSeptSortInfo* tempPoint;
      
      Cmd::stRtnSeptSort* retCmd = (Cmd::stRtnSeptSort*)buf;
      constructInPlace(retCmd);
      
      std::sort(_vSept.begin(),_vSept.end());
      
      tempPoint = (Cmd::stSeptSortInfo *)retCmd->data;
      retCmd->byType = _byType;

      for (DWORD i=0; i<_vSept.size(); i++)
      {
        if (i<10)
        {
          tempPoint->dwRepute = _vSept[i].dwRepute;
          strncpy(tempPoint->septName,_vSept[i].septName,MAX_NAMESIZE);
          tempPoint->dwCountryID = _vSept[i].dwCountryID;
          tempPoint++;
          retCmd->dwSize++;  
        }
        else{
          break;
        }
      }
      
      pUser->sendCmdToMe(retCmd,(retCmd->dwSize*sizeof(Cmd::stSeptSortInfo)+
            sizeof(Cmd::stRtnSeptSort)));
    }
      
    
    bool exec(CSept *psept)
    {      
      if (psept && !psept->byVote && (_byType != Cmd::COUNTRY_SEPT_SORT || psept->dwCountryID==_dwCountry))
      {
        CSeptSort temp;
        strncpy(temp.septName,psept->name,MAX_NAMESIZE);

        temp.dwRepute = psept->getRepute();
        temp.dwOrder  = 0;
        temp.dwCountryID = psept->dwCountryID;
        _vSept.push_back(temp);
      }
      
      return true;
    }
  };
  
  findList myList(pUser->country,ptCmd->byType);
  execEveryOne(myList);
  myList.sendSeptSort(pUser);
}

/**
* \brief �����Ա���
* \param dwSeptID ����ID
* \param dwCharID �뿪�ߵ�ID
*/
void CSeptM::processMemberLeave(const DWORD dwSeptID,const DWORD dwCharID)
{
  CSept *pSept = NULL;
  pSept = (CSept *)getEntryByID(dwSeptID);
  if (pSept)
  {
    CSeptMember *pMember;
    pMember = pSept->master;
    if (pMember)
    {
      if (pMember->id != dwCharID)
      {
        pSept->fireSeptMemberDirect(dwCharID);
      }
      else
      {
//Shx Modify ȥ�����ͼ���ĸ�����ϵ,
//         if (pSept->dwUnionID >0)
//         {
//           UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pMember->id);
//           if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"���������˳����,���ܽ�ɢ����");
//         }
//         else
//         {
         delSept(dwSeptID);  // ������峤�ͽ�ɢ����
//       }
//End;
	  }
    }
    else
    {
      Zebra::logger->error("����%sû����ȷ�Ļ᳤����,�������ݵ�������",pSept->name);
    }
  }

}

/**
* \brief ���ͼ���������Ϣ
* \param pUser ��Ϣ������
* \param pCmd ������Ϣ��
* \param cmdLen ��Ϣ����
*/
void CSeptM::sendSeptChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *pCmd,const DWORD cmdLen)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pCmd->pstrName);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    if (pSeptMember->mySept)
    {
      pSeptMember->mySept->sendCmdToAllMember(pCmd,cmdLen);
    }
  }
  else
  {
    if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����ȼ���������ʹ�ü�������");
  }
}

/**
* \brief ���ͼ���˽����Ϣ
* \param pUser ��Ϣ������
* \param rev ������Ϣ��
* \param cmdLen ��Ϣ����
*/
void CSeptM::sendSeptPrivateChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(rev->pstrName);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stChannelChatUserCmd *chatCmd;

    chatCmd = (Cmd::stChannelChatUserCmd *)buf;
    memccpy(chatCmd,rev,cmdLen,sizeof(buf));
    strncpy(chatCmd->pstrName,pUser->name,MAX_NAMESIZE);

//    UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pSeptMember->id);
//    if (pUser)
//    {
      if (!pSeptMember->mySept->isVote()) pSeptMember->sendCmdToMe(chatCmd,cmdLen);
//    }
//    else
//    {
//      COfflineMessage::writeOfflineMessage(chatCmd->dwType,pSeptMember->id,chatCmd,cmdLen);
//    }
  }
}

/**
* \brief ���� Gateway ת�������Ŀͻ�����Ϣ
* \param pUser ��Ϣ������
* \param pNullCmd ��Ϣ����
* \param cmdLen ��Ϣ����
* \return true ����ɹ� false ��Ϣ���ڴ���Χ֮��
*/
bool CSeptM::processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  switch(pNullCmd->byCmd)
  {
    case Cmd::SEPT_USERCMD:
      {
        switch(pNullCmd->byParam)
        {
          case Cmd::REQUEST_SEPT_NORMAL_EXP_PARA:
            {
              this->processRequestSeptNormalExpMessage(pUser,
                  (Cmd::stRequestSeptNormalExpCmd*)pNullCmd);
              return true;
            }
            break;
          case Cmd::REQ_SEPT_NAME_LIST_PARA:
            {
              Cmd::stReqSeptNameListCmd* rev = (Cmd::stReqSeptNameListCmd*)pNullCmd;
              char buffer[zSocket::MAX_DATASIZE];
              Cmd::stRtnSeptNameListCmd *send = (Cmd::stRtnSeptNameListCmd*)buffer;
              constructInPlace(send);
              int i=0;
              while(i < rev->num && i <= 200)
              {
                CSept* pSept = CSeptM::getMe().getSeptByID(rev->dwSeptID[i]);
                if (pSept)
                {
#ifdef _DEBUG
                  Zebra::logger->debug("[�Ż�]: %s ���� %s ��������",pUser->name,pSept->name);
#endif                

                  strncpy(send->list[send->num].name,pSept->name,MAX_NAMESIZE);
                  send->list[send->num].dwSeptID = pSept->id;
                  send->num++;
                }
                i ++;
              }
              if (send->num)
              {
                pUser->sendCmdToMe(send,sizeof(Cmd::stRtnSeptNameListCmd) + (send->num * sizeof(send->list[0])));
              }
              
              return true;
            }
            break;
          case Cmd::REQ_SEPT_NAME_PARA:
            {
              Cmd::stReqSeptNameCmd* rev = (Cmd::stReqSeptNameCmd*)pNullCmd;
              CSept* pSept = CSeptM::getMe().getSeptByID(rev->dwSeptID);
              Cmd::stRtnSeptNameCmd send;
              
              if (pSept)
              {
#ifdef _DEBUG
                Zebra::logger->debug("[�Ż�]: %s ���� %s ��������",pUser->name,pSept->name);
#endif                

                strncpy(send.name,pSept->name,MAX_NAMESIZE);
                send.dwSeptID = rev->dwSeptID;
                pUser->sendCmdToMe(&send,sizeof(send));
              }
              
              return true;
            }
            break;
          case Cmd::REQUEST_ABJURATION_SEPTNPC_PARA:
            {
              CSept* pSept = CSeptM::getMe().getSeptByID(pUser->septid);
              if (pSept && pSept->tempid == pUser->id)
              {
                CNpcDareObj *npcdare = CNpcDareM::getMe().searchSeptHold(pUser->septid);
                if (npcdare)
                {
                  npcdare->abandon_npc();
                }
                else
                {
                  pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
                    "����廹δռ���κ�����");
                }
              }
              else
              {
                pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
                    "�������峤,���ܷ���");
              }
              
              return true;
            }
            break;
          case Cmd::REQUEST_SEPT_EXP_PARA:
            {
              Cmd::stRequestSeptExpCmd* ptCmd = 
                (Cmd::stRequestSeptExpCmd*)pNullCmd;
              this->processRequestSeptExpMessage(pUser,ptCmd);
              return true;
            }
            break;
          case Cmd::REQ_SEPT_SORT_PARA:
            {
              Cmd::stReqSeptSort* ptCmd = 
                (Cmd::stReqSeptSort*)pNullCmd;
              this->processSeptSortMessage(pUser,ptCmd);
              return true;
            }
            break;
          case Cmd::CHANGE_SEPT_MEMBER_ALIASNAME_PARA:
            {
              Cmd::stChangeSeptMemberAliasName* ptCmd = 
                (Cmd::stChangeSeptMemberAliasName*)pNullCmd;
              
              change_aliasname(pUser,ptCmd);
              return true;
            }
            break;
          case Cmd::FIRE_MEMBER_FROM_SEPT_PARA:
            {
              Cmd::stFireMemberFromSeptCmd *ptCmd=(Cmd::stFireMemberFromSeptCmd *)pNullCmd;
              fireSeptMember(pUser,ptCmd->memberName);
              return true;
            }
            break;
          case Cmd::SEPTMEMBER_LEAVE_SEPT_PARA:
            {
              processMemberLeave(pUser->septid,pUser->id);
              return true;
            }
            break;
          case Cmd::LIST_SEPT_PARA:
            {
              sendListToUser(pUser);
              return true;
            }
            break;
          case Cmd::VOTELIST_SEPT_PARA:
            {
              /*if (pUser->level<20)
              {
                pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ȼ�����20�����ܲ���ͶƱ");
                return true;
              }*/

              sendVoteListToUser(pUser);
              return true;
            }
            break;
          case Cmd::REQUEST_NPCINFO_SEPT_PARA:
            {
              if (pUser)
              {
                CNpcDareObj *npcdare = CNpcDareM::getMe().searchSeptHold(pUser->septid);
                if (npcdare)
                {
                  SceneSession *scene= SceneSessionManager::getInstance()->getSceneByID((npcdare->get_country()<<16)+npcdare->get_mapid());
                  if (scene)
                    pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"��ļ������ڿ�����%s(%u,%u)�������ˣ�",scene->name,npcdare->get_posx(),npcdare->get_posy());
                  else
                    pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"�޷��˲���ļ�����Ƶ�����������");
                }
                else
                {
                  pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"��ļ���û�п��Ƶ�����");
                }
              }
              return true;
            }
            break;
          case Cmd::REQUEST_JOIN_SEPT_PARA:
            {
              Cmd::stRequestJoinSeptCmd *ptCmd=(Cmd::stRequestJoinSeptCmd *)pNullCmd;
              switch(ptCmd->status)
              {
                case Cmd::REQUEST_JOIN_OK:
                  {
                    UserSession *master = UserSessionManager::getInstance()->getUserSessionByName(ptCmd->name);
                    if (master)
                    {
                      strncpy(ptCmd->name,pUser->name,MAX_NAMESIZE);
                      master->sendCmdToMe(ptCmd,sizeof(Cmd::stRequestJoinSeptCmd));
                    }
                    else
                    {
                      pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"�峤%sĿǰ������",ptCmd->name);
                    }
                  }
                  break;
                case Cmd::REQUEST_JOIN_CANCEL:
                case Cmd::REQUEST_JOIN_TIMEOUT:
                default:
                  {
                    UserSession *tUser = UserSessionManager::getInstance()->getUserSessionByName(ptCmd->name);
                    if (tUser)
                    {
                      strncpy(ptCmd->name,pUser->name,MAX_NAMESIZE);
                      tUser->sendCmdToMe(ptCmd,sizeof(Cmd::stRequestJoinSeptCmd));
                    }
                  }
                  break;
              }
            }
            break;
          case Cmd::VOTE_SEPT_PARA:
            {
              if (pUser->level<20)
              {
                pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ȼ�����20�����ܲ���ǩ��");
                return true;
              }

              Cmd::stVoteSeptCmd *ptCmd=(Cmd::stVoteSeptCmd *)pNullCmd;
              if (ptCmd->bySuccess)
              {
                userVote(pUser,ptCmd->septName);
              }
              else
              {
                userAboutVote(pUser,ptCmd->septName);
              }
              return true;
            }
            break;
          case Cmd::NOTE_SEPT_PARA:
            {
              Cmd::stNoteSeptCmd *ptCmd=(Cmd::stNoteSeptCmd *)pNullCmd;
              setSeptNote(pUser,ptCmd);
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

bool CSeptM::processSceneSeptMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
  switch(pNullCmd->para)
  {
    case Cmd::Session::OP_LEVEL_PARA:
      {
        Cmd::Session::t_OpLevel_SceneSession* ptCmd=
        (Cmd::Session::t_OpLevel_SceneSession*)pNullCmd;

        this->changeLevel(ptCmd->dwSeptID,ptCmd->dwLevel);
        return true;
      }
      break;
    case Cmd::Session::OP_REPUTE_PARA:
      {
        Cmd::Session::t_OpRepute_SceneSession* ptCmd=
          (Cmd::Session::t_OpRepute_SceneSession*)pNullCmd;

        CSept* pSept = CSeptM::getMe().getSeptByID(ptCmd->dwSeptID);
        
        if (pSept)          
        {
          pSept->changeRepute(ptCmd->dwRepute);
        }

        return true;
      }
      break;
    default:
      break;
  }

  return false;
}
/**
* \brief ����ӳ�����������Ϣ
* \param cmd ��Ϣ��
* \param cmdLen ��Ϣ����
* \return true ����ɹ� false ��Ϣ���ڴ���Χ֮��
*/
bool CSeptM::processSceneMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
  switch(cmd->para)
  {
    case Cmd::Session::PARA_SEPT_ADDMEMBER:
      {
        Cmd::Session::t_addSeptMember_SceneSession *ptCmd=(Cmd::Session::t_addSeptMember_SceneSession *)cmd;
        std::map<std::string,CSeptMember *>::iterator sIterator;
              CSeptMember *pSeptMember = NULL;
                                    
              rwlock.rdlock();        
              sIterator = findMemberIndex(ptCmd->member.name);
              if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
              rwlock.unlock();
        
        CSept * pSept = NULL;

        rwlock.rdlock();
        pSept = (CSept *)getEntryByID(ptCmd->dwSeptID);
        rwlock.unlock();
        if (!pSept || !pSept->master)
        {
          Zebra::logger->error("[����]:�����³�Ա,��δ�ҵ�ָ���Ĺ����᳤");
          return true;
        }

        if (pSeptMember != NULL)
        {
          /*if (pSeptMember->mySept && pSeptMember->user && pSeptMember->mySept->isVote())
          {
            userAboutVote(pSeptMember->user,pSeptMember->mySept->name);
          }*/
//           if (!pSept || !pSept->master)
//           {
//             return true;
//           }
          UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pSept->master->id);

//           if (pSept && pSept->master && pUser)
//           {
//             pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է��Ѽ��빤��,��������");
//           }

          pUser = UserSessionManager::getInstance()->getUserByID(pSeptMember->id);
          if (pSeptMember && pUser)
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,
                "���Ѽ������,���������ܼ�������ļ���");
          }
        }
        else
        {
//           if (!pSept || !pSept->master)
//           {
//             return true;
//           }
            
          UserSession *pUser = UserSessionManager::getInstance()->getUserByID(pSept->master->id);
          if (/*pSept && pSept->master &&*/ pUser)  
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"%s ������������������˼���",ptCmd->member.name);
          }

          if (addNewMemberToSept(ptCmd->dwSeptID,ptCmd->member))
          {
			  pSept->sendSeptNotify("��ӭ %s �������",ptCmd->member.name);
          }
        }
          
        return true;
      }
      break;
    case Cmd::Session::PARA_SEPT_ADDSEPT:
      {
        Cmd::Session::t_addSept_SceneSession *ptCmd=(Cmd::Session::t_addSept_SceneSession *)cmd;
        createNewSept(ptCmd);
        return true;
      }
      break;
    case Cmd::Session::PARA_SEPT_EXP_DISTRIBUTE:
      {
        Cmd::Session::t_distributeSeptExp_SceneSession *ptCmd=(Cmd::Session::t_distributeSeptExp_SceneSession *)cmd;
        disributeExp(ptCmd);
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

/**
* \brief �����Gateway��������Ϣ
* \param cmd ��Ϣ��
* \param cmdLen ��Ϣ����
* \return true ����ɹ� false ��Ϣ���ڴ���Χ֮��
*/
bool CSeptM::processGateMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
  switch(cmd->para)
  {
    case Cmd::Session::PARA_SEPT_DISBAND:
      {
        Cmd::Session::t_disbandSept_GateSession *ptCmd=(Cmd::Session::t_disbandSept_GateSession *)cmd;
        processMemberLeave(ptCmd->dwSeptID,ptCmd->dwCharID);
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

/**
* \brief ���͵�ǰϵͳ���ڵȴ�ͶƱ�ļ����б��ָ�����û�
* \param pUser ����ͶƱ�б���
*/
void CSeptM::sendVoteListToUser(const UserSession *pUser)
{/*
  struct findList : public execEntry<CSept>
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stVoteListSeptCmd *retCmd;
    Cmd::stSeptVoteRecord *tempPoint;
    std::vector<DWORD> removed;
    UserSession *user;
    findList(char *pName,UserSession *pUser)
    {
      user = pUser;
      retCmd=(Cmd::stVoteListSeptCmd *)buf;
      constructInPlace(retCmd);
      tempPoint = (Cmd::stSeptVoteRecord *)retCmd->data;
      retCmd->dwSize = 0;
      retCmd->flag =0;

      strncpy(retCmd->septName,pName,MAX_NAMESIZE);
    }
    ~findList(){}

    void sendList(const UserSession *pUser)
    {
      pUser->sendCmdToMe(retCmd,(retCmd->dwSize*sizeof(Cmd::stSeptVoteRecord)+sizeof(Cmd::stVoteListSeptCmd)));
    }

    void removeInvalidSept()
    {
      std::vector<DWORD>::iterator tIterator;

      for(tIterator = removed.begin(); tIterator != removed.end(); tIterator++)
      {
        CSeptM::getMe().delSept(*tIterator);
      }
    }

    bool exec(CSept *psept)
    {
      if (psept&&psept->isVote())
      {
        zRTime ctv;
        if (ctv.sec() - psept->dwCreateTime > DESTROYTIME)
        {
          removed.push_back(psept->id);
          return true;
        }

        if (retCmd->dwSize>100)
        {
          this->sendList(user);
          char tName[MAX_NAMESIZE];
          strncpy(tName,retCmd->septName,MAX_NAMESIZE);
          tempPoint = (Cmd::stSeptVoteRecord *)retCmd->data;
          retCmd->dwSize = 0;
          retCmd->flag=1;
        }

        
        if (user && user->country == psept->dwCountryID)
        {
          strncpy(tempPoint->septName,psept->name,MAX_NAMESIZE);
          if (psept->master)
            strncpy(tempPoint->master,psept->master->name,MAX_NAMESIZE);
          else
            strncpy(tempPoint->master,"δ֪",MAX_NAMESIZE);
          strncpy(tempPoint->note,psept->note,255);
          tempPoint->wdVoteNumber = psept->size();
          tempPoint++;
          retCmd->dwSize++;
        }
      }
      return true;
    }
  };

  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;
  char tempName[MAX_NAMESIZE];

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  bzero(tempName,MAX_NAMESIZE);
  if (pSeptMember)
  {
    if (pSeptMember->mySept->isVote())
    {
      strncpy(tempName,pSeptMember->mySept->name,MAX_NAMESIZE);
    }
  }



  findList myList(tempName,(UserSession *)pUser);
  execEveryOne(myList);
  myList.sendList(pUser);
  myList.removeInvalidSept();
  */
}


/**
* \brief ���͵�ǰϵͳ�еļ����б��ָ�����û�
* \param pUser �����б���
*/
void CSeptM::sendListToUser(const UserSession *pUser)
{
  struct findList : public execEntry<CSept>
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stListSeptCmd *retCmd;
    Cmd::stSeptRecord *tempPoint;
    UserSession *user;
    findList(UserSession *pUser)
    {
      user = pUser;
      retCmd=(Cmd::stListSeptCmd *)buf;
      constructInPlace(retCmd);
      tempPoint = (Cmd::stSeptRecord *)retCmd->data;
      retCmd->dwSize = 0;
      retCmd->flag =0;
    }
    ~findList(){}

    void sendList(const UserSession *pUser)
    {
      pUser->sendCmdToMe(retCmd,(retCmd->dwSize*sizeof(Cmd::stSeptRecord)+sizeof(Cmd::stListSeptCmd)));
    }

    bool exec(CSept *psept)
    {
      if (psept)
      {
        if (retCmd->dwSize>100)
        {
          this->sendList(user);
          tempPoint = (Cmd::stSeptRecord *)retCmd->data;
          retCmd->dwSize = 0;
          retCmd->flag=1;
        }

        
        if (user && user->country == psept->dwCountryID)
        {
          strncpy(tempPoint->septName,psept->name,MAX_NAMESIZE);
          if (psept->master)
            strncpy(tempPoint->master,psept->master->name,MAX_NAMESIZE);
          else
            strncpy(tempPoint->master,"δ֪",MAX_NAMESIZE);
          strncpy(tempPoint->note,psept->note,255);
          //tempPoint->wdVoteNumber = psept->size();
          tempPoint++;
          retCmd->dwSize++;
        }
      }
      return true;
    }
  };

  findList myList((UserSession *)pUser);
  execEveryOne(myList);
  myList.sendList(pUser);
}

/**
 * \brief ���͵�ǰϵͳ�п��Ա���ս�İ���б�
 * \param pUser ����ͶƱ��
 */
void CSeptM::sendDareListToUser(const UserSession *pUser)
{
  struct findList : public execEntry<CSept>
  {
    BYTE buf[zSocket::MAX_DATASIZE];
    Cmd::stSendDareList *send;
    Cmd::stDareList* tempPoint;
    const UserSession* _pUser;

    findList(const UserSession* pUser)
    {       
      send = (Cmd::stSendDareList *)buf;
      constructInPlace(send);
      tempPoint = (Cmd::stDareList *)send->dare_list;
      send->dwSize = 0;

      send->byType = Cmd::SEPT_DARE;
      _pUser = pUser;
    }
    ~findList(){}

    void sendList()
    {       
      if (_pUser) _pUser->sendCmdToMe(send,(send->dwSize*sizeof(Cmd::stDareList)+sizeof(Cmd::stSendDareList)));
    }

    bool exec(CSept *psept)
    {      
      if (_pUser)  
      {
        if (psept && psept->master)
        {   
          UserSession *pUser = UserSessionManager::getInstance()->getUserByID(psept->master->id);
          if (pUser && pUser->country==_pUser->country
              && psept->id!=_pUser->septid)
          {
            strncpy(tempPoint->name,psept->name,MAX_NAMESIZE);
            tempPoint++;
            send->dwSize++;
          }
        }
      }

      return true;
    }
  };


  findList myList(pUser);
  execEveryOne(myList);
  myList.sendList();
}
/**
* \brief �û�ͶƱ��ָ���ļ���
* \param pUser ͶƱ��
* \param pName ��������
*/
void CSeptM::userVote(const UserSession *pUser,const char *pName)
{
	//[Shx Add]
	return;
	//End Shx 


  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    if (pUser)
    {
      if (pSeptMember->mySept->isVote())
      {
        if (pSeptMember == pSeptMember->mySept->master)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����%s���Ԥ���峤������ǩ��",pSeptMember->mySept->name);
        }
        else
        {
          if (strncmp(pName,pSeptMember->mySept->name,MAX_NAMESIZE)!=0)
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���Ѷ�%s��ǩ��,��֮ǰ��%s���ǩ����������.",pName,pSeptMember->mySept->name);
            processMemberLeave(pSeptMember->mySept->id,pUser->id);
            addVoteMemberToSept(pUser,pName);
            return;
          }
        }
      }
      else
      {
        pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���Ѿ���������������,û��ǩ�����ʸ�");
      }
    }
    else
    {
      Zebra::logger->error("CSeptM::userVote():һ�����ߵļ����Աû����Ч���û�ָ��");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���%s�������ǩ��",pName);
    addVoteMemberToSept(pUser,pName);
    return;
  }

  // ֪ͨͶƱʧ��
  Cmd::stVoteSeptCmd send;
  strncpy(send.septName,pName,MAX_NAMESIZE);
  send.bySuccess = 0;
  pUser->sendCmdToMe(&send,sizeof(send));
}

/**
* \brief �û�����ͶƱ
* \param pUser ����ͶƱ�ĳ�Ա
* \param pName ��������������
* \return 
*/
void CSeptM::userAboutVote(const UserSession *pUser,const char *pName)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {

    if (pUser)
    {
      if (pSeptMember->mySept->isVote())
      {
        if (pSeptMember == pSeptMember->mySept->master)
        {
          
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����%s���Ԥ���峤,����ǩ��,�����ɢ",
              pSeptMember->mySept->name);

          processMemberLeave(pSeptMember->mySept->id,pUser->id);
          sendVoteListToUser(pUser);

          return;
        }
        else
        {
          if (strncmp(pName,pSeptMember->mySept->name,MAX_NAMESIZE)==0)
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������%s���Ʊ.",pName);
            processMemberLeave(pSeptMember->mySept->id,pUser->id);
            Cmd::stVoteSeptCmd send;
            strncpy(send.septName,pName,MAX_NAMESIZE);
            send.bySuccess = 2;
            pUser->sendCmdToMe(&send,sizeof(send));
            return;
          }
        }
      }
      else
      {
        Zebra::logger->error("[%s]�����Ѿ�����,����ǩ����ûȡ��",pName);
      }
    }
    else
    {
      Zebra::logger->error("CSeptM::userVote():һ�����ߵļ����Աû����Ч���û�ָ��");
    }
  }

  // ֪ͨͶƱʧ��
  Cmd::stVoteSeptCmd send;
  strncpy(send.septName,pName,MAX_NAMESIZE);
  send.bySuccess = 0;
  pUser->sendCmdToMe(&send,sizeof(send));
}

/**
* \brief ͶƱ��ָ���ļ���
* \param pUser ͶƱ��
* \param pName ��������
*/
void CSeptM::addVoteMemberToSept(const UserSession *pUser,const char *pName)
{
  rwlock.rdlock();
  CSept *mySept = (CSept *)getEntryByName(pName);
  rwlock.unlock();

  if (mySept)
  {
    if (mySept->isVote())
    {
      stSeptMemberInfo member;
      member.dwCharID = pUser->id;
      strncpy(member.name,pUser->name,MAX_NAMESIZE);
      member.wdOccupation = pUser->occupation;
      addNewMemberToSept(mySept->id,member);

      Cmd::stVoteSeptCmd send;
      strncpy(send.septName,pName,MAX_NAMESIZE);
      send.bySuccess = 1;
      pUser->sendCmdToMe(&send,sizeof(send));

      if (mySept->size()>=CREATE_SEPT_NEED_MAN_NUM)//10
      {
        mySept->letVoteOver();
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"%s���ǩ�����Ѿ�����,��ֱ��ȥ���峤Э���й��������.",pName);
      Cmd::stVoteSeptCmd send;
      strncpy(send.septName,pName,MAX_NAMESIZE);
      send.bySuccess = 0;
      pUser->sendCmdToMe(&send,sizeof(send));
    }
  }
}

/**
* \brief �����ڷ��侭��
* \param cmd ���徭�������Ϣ
*/
void CSeptM::disributeExp(Cmd::Session::t_distributeSeptExp_SceneSession *cmd)
{
  UserSession *pUser=UserSessionManager::getInstance()->getUserByID(cmd->dwUserID);
  if (!pUser) return;

  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    CSept *mySept = pSeptMember->mySept;
    if (mySept)
    {
      mySept->sendDistributeSeptExpToScene(cmd->dwUserID,cmd);
    }
  }
}

/**
* \brief �������ƻ�ȡ�������
* \param name ���������
* \return �ɹ����ؼ������ ʧ�ܷ���NULL
*/
CSept * CSeptM::getSeptByName( const char * name)
{
  rwlock.rdlock();
  CSept *ret =(CSept *)getEntryByName(name);
  rwlock.unlock();
  return ret;
}

/**
* \brief ����ID��ȡ�������
* \param dwSeptID �����ID
* \return �ɹ����ؼ������,ʧ�ܷ��� NULL
*/
CSept* CSeptM::getSeptByID(DWORD dwSeptID)
{
  rwlock.rdlock();
  CSept* ret = (CSept*)getEntryByID(dwSeptID);
  rwlock.unlock();
  return ret;
}

/**
* \brief ���ü������
* \param pUser �����ߵ� UserSession����
* \param pCmd ����������Ϣ
*/
void CSeptM::setSeptNote(UserSession *pUser,Cmd::stNoteSeptCmd *pCmd)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    CSept *mySept = pSeptMember->mySept;
    if (mySept && mySept->master == pSeptMember)
    {
      mySept->setNote(pCmd);
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���޷����ü������");
    }
  }
}

void CSeptM::change_aliasname(UserSession* pUser,Cmd::stChangeSeptMemberAliasName* pCmd)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember=NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(pUser->name);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  
  if (pSeptMember)
  {
    CSept* pSept = pSeptMember->mySept;
    if (pSept && pSept->master == pSeptMember)
    {
      pSeptMember = NULL;
      rwlock.rdlock();
      sIterator = findMemberIndex(pCmd->name);
      if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
      rwlock.unlock();

      if (pSeptMember && pSeptMember->mySept == pSept)
      {
        pSeptMember->change_aliasname(pCmd->aliasname);
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����Ȩ���ĳƺ�");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���������κμ���");
  }

}

void CSeptM::changeAllRepute(DWORD countryid,int repute)
{
  rwlock.rdlock();
  for(zEntryName::hashmap::iterator it=zEntryName::ets.begin();it!=zEntryName::ets.end();it++)
  {
    CSept *pSept =(CSept*)it->second;
    if (pSept && pSept->dwCountryID == countryid)
    {
      pSept->changeRepute(repute);
    }
  }
  rwlock.unlock();
}

DWORD CSeptM::getRepute(DWORD dwSeptID)
{
  CSept* pSept = this->getSeptByID(dwSeptID);
  if (pSept)
    return pSept->getRepute();

  return 0;
}

void CSeptM::changeRepute(DWORD dwSeptID,int repute)
{
  CSept* pSept = this->getSeptByID(dwSeptID);
  if (pSept)
    return pSept->changeRepute(repute);
}

void CSeptM::changeLevel(DWORD dwSeptID,int level)
{
  CSept* pSept = this->getSeptByID(dwSeptID);
  if (pSept)
    return pSept->changeLevel(level);
}
/**
 * \brief ���ݽ�ɫ���ֻ�ȡ��������������
 *
 *
 * \param Name ��ɫ����
 * \return ���ذ�����ƻ���NULL
 */
char * CSeptM::getSeptNameByUserName(char *Name)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(Name);

  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    if (pSeptMember->mySept)
      return pSeptMember->mySept->name;
  }
  return NULL;
}

/**
 * \brief ���ݽ�ɫ���ֻ�ȡ����������ID
 *
 *
 * \param Name ��ɫ����
 * \return ���ذ�����ƻ���NULL
 */
DWORD CSeptM::getSeptIDByUserName(char *Name)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember = NULL;

  rwlock.rdlock();
  sIterator = findMemberIndex(Name);

  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  if (pSeptMember)
  {
    if (pSeptMember->mySept)
      return pSeptMember->mySept->id;
  }
  return 0;
}

/**
 * \brief ���ݼ���ID��ָ֪ͨ�������NPC��������
 * \param septid ����ID
 */
void CSeptM::notifyNpcHoldData(DWORD septid)
{
  CSept* pSept = this->getSeptByID(septid);
  if (pSept)
    return pSept->notifyNpcHoldData();
}

CSeptMember * CSeptM::getMemberByName(const char * pName)
{
  std::map<std::string,CSeptMember *>::iterator sIterator;
  CSeptMember *pSeptMember=NULL;

  rwlock.rdlock();
  sIterator = this->findMemberIndex(pName);
  if (sIterator != memberIndex.end()) pSeptMember = (*sIterator).second;
  rwlock.unlock();
  return pSeptMember;
}

