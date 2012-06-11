/**
 * \brief ʵ�ֻỰ������
 *
 * 
 */
#include <zebra/SessionServer.h>

///�û��Ự������ʵ��
UserSessionManager * UserSessionManager::sm(NULL);
DWORD UserSession::user_count=0;
//��������
std::map<DWORD,DWORD> UserSession::country_map;
class CGraceUser
{
  public:
    DWORD dwUserID;
    DWORD dwUseJob; //sky ְҵ
    DWORD dwExploit;
    char  szName[MAX_NAMESIZE];

    CGraceUser()
    {
      dwUserID = 0;
      dwUseJob = 0;
      dwExploit = 0;
      bzero(szName,MAX_NAMESIZE);
    }

    CGraceUser(const CGraceUser& ref)
    {
      dwUserID = ref.dwUserID;
      dwUseJob = ref.dwUseJob;
      dwExploit = ref.dwExploit;
      strncpy(szName,ref.szName,MAX_NAMESIZE);
    }

    ~CGraceUser()
    {
    }
    
    CGraceUser& operator =(const CGraceUser& ref)
    {
      this->dwUserID = ref.dwUserID;
      this->dwUseJob = ref.dwUseJob;
      this->dwExploit = ref.dwExploit;
      strncpy(this->szName,ref.szName,MAX_NAMESIZE);
      return *this;
    }

};

//sky dwUseJob�Ѿ���ְҵ�������Ĳ���`��������Ƚ��Ѿ�û������
bool lessGraceScore(const CGraceUser& p1,const CGraceUser& p2)
{
 /* return p1.dwUseJob > p2.dwUseJob;*/
	return true;
}

bool lessExploit(const CGraceUser& p1,const CGraceUser& p2)
{
  return p1.dwExploit > p2.dwExploit;
}

/**
 * \brief ���캯��
 */
UserSessionManager::UserSessionManager():zUserManager()
{
  inited=false;
}

/**
 * \brief ��������
 */
UserSessionManager::~UserSessionManager()
{
  final();
}

/**
 * \brief ȡ�ù�����ʵ��
 * �����û�г�ʼ�������������
 * \return ������ʵ��ָ��
 */
UserSessionManager *UserSessionManager::getInstance()
{
  if (sm==NULL)
    sm=new UserSessionManager();
  return sm;
}

/**
 * \brief ɾ��������ʵ��
 */
void UserSessionManager::delInstance()
{
  SAFE_DELETE(sm);
}

/**
 * \brief ��ȡһ��ΨһID
 * \return �Ƿ�ɹ�
 */
bool UserSessionManager::getUniqeID(DWORD &tempid)
{
  return true;
}

/**
 * \brief �ͷ�һ��ΨһID
 * \return 
 */
void UserSessionManager::putUniqeID(const DWORD &tempid)
{
}

/**
 * \brief ��ʼ���û�������
 * \return ��ʼ���Ƿ�ɹ�
 */
bool UserSessionManager::init()
{
  if (inited)
  {
    Zebra::logger->warn("User Session�������Ѿ�����ʼ����...");
    return true;
  }
  inited=true;
  Zebra::logger->info("��ʼ��User Session�������ɹ�...");
  return inited;
}

/**
 * \brief �����û�������
 */
void UserSessionManager::final()
{
  if (inited)
  {
    inited=false;
  }
}

/**
 * \brief �������ֵõ��û��Ự��ָ��
 * \param name �û�����
 * \return �ҵ��ĻỰָ�룬ʧ�ܷ���0
 */
UserSession * UserSessionManager::getUserSessionByName( const char * name)
{
  UserSession *ret =(UserSession *)getUserByName(name);
  /*
  mlock.lock();
  UserSession *ret =(UserSession *)getEntryByName(name);
  mlock.unlock();
  // */
  return ret;
}

/**
 * \brief ������ʱID�õ��û��Ự��ָ��
 * \param tempid �û���ʱid
 * \return �ҵ��ĻỰָ�룬ʧ�ܷ���0
 */
UserSession *UserSessionManager::getUserByTempID(DWORD tempid)
{
  return (UserSession *)zUserManager::getUserByTempID(tempid);
}

/**
 * \brief ����ID�õ��û��Ự��ָ��
 * \param id �û�id
 * \return �ҵ��ĻỰָ�룬ʧ�ܷ���0
 */
UserSession *UserSessionManager::getUserByID(DWORD id)
{
  return (UserSession *)zUserManager::getUserByID(id);
}

void UserSessionManager::sendGraceSort(UserSession* pUser)
{
  struct sortAllUser: public execEntry<UserSession>
  {
    std::vector<CGraceUser> vGraceList;
    DWORD _dwCountryID;

    sortAllUser(DWORD dwCountryID)
    {
      _dwCountryID = dwCountryID;
    }

    ~sortAllUser()
    {
    }

    void sendSort(UserSession* user)
    {
      BYTE buf[zSocket::MAX_DATASIZE];
      Cmd::stWaitOfficialItem* tempPoint;
      Cmd::stRtnWaitOfficialUserCmd* retCmd = (Cmd::stRtnWaitOfficialUserCmd*)buf;
      constructInPlace(retCmd);
      std::sort(vGraceList.begin(),vGraceList.end(),lessGraceScore);
      tempPoint = (Cmd::stWaitOfficialItem *)retCmd->data;

      for (DWORD i=0; i<vGraceList.size(); i++)
      {
        if (i<10)
        {
          tempPoint->dwCharID = vGraceList[i].dwUserID;
          strncpy(tempPoint->szName,vGraceList[i].szName,MAX_NAMESIZE);
          tempPoint++;
          retCmd->dwSize++;  
        }
        else{
          break;
        }
      }
      
      user->sendCmdToMe(retCmd,(retCmd->dwSize*sizeof(Cmd::stWaitOfficialItem)+sizeof(Cmd::stRtnWaitOfficialUserCmd)));
    }
    
    bool exec(UserSession* user)
    {
      if (user && user->dwUseJob>0 && !CCountryM::getMe().isKing(user)
          && user->country == _dwCountryID && !CCountryM::getMe().isOfficial(user))
      {
        CGraceUser temp;
        temp.dwUserID = user->id;
        temp.dwUseJob = user->dwUseJob;
        strncpy(temp.szName,user->name,MAX_NAMESIZE);
        vGraceList.push_back(temp);
      }
      
      return true;
    }
  };

  struct sortAllUser allUser(pUser->country);
  execEveryUser(allUser);
  allUser.sendSort(pUser);
}

void UserSessionManager::notifyOnlineToGate()
{
	struct checkOnline: public execEntry<UserSession>
	{
		std::map<DWORD,Cmd::Session::Country_Online> vCountryMap;
		void sendToGate()
		{
			BYTE buf[zSocket::MAX_DATASIZE];
			Cmd::Session::t_updateOnline_SessionGate* pCmd = (Cmd::Session::t_updateOnline_SessionGate*)buf;
			constructInPlace(pCmd);
			pCmd->size = vCountryMap.size();
			int i = 0;
			for(std::map<DWORD,Cmd::Session::Country_Online>::iterator it = vCountryMap.begin();
				it != vCountryMap.end(); it ++)
			{
				pCmd->info[i].country_id = it->first;
				pCmd->info[i].Online_Now = it->second.Online_Now;
				i++;
			}
			
			SessionTaskManager::getInstance().broadcastGateway(pCmd, sizeof(Cmd::Session::t_updateOnline_SessionGate) + vCountryMap.size() * sizeof(Cmd::Session::Country_Online));
		}
		bool exec(UserSession* user)
		{
			if( user && user->country )
			{
				vCountryMap[user->country].Online_Now ++;
			}
			return true;
		}
	};
	struct checkOnline allUser;
	execEveryUser(allUser);
	allUser.sendToGate();
}

void UserSessionManager::sendExploitSort(UserSession* pUser)
{
  struct sortAllUser: public execEntry<UserSession>
  {
    std::vector<CGraceUser> vExploitList;
    DWORD _dwCountryID;

    sortAllUser(DWORD dwCountryID)
    {
      _dwCountryID = dwCountryID;
    }

    ~sortAllUser()
    {
    }

    void sendSort(UserSession* user)
    {
      BYTE buf[zSocket::MAX_DATASIZE];
      Cmd::stWaitGenItem* tempPoint;
      Cmd::stRtnWaitGenUserCmd* retCmd = (Cmd::stRtnWaitGenUserCmd*)buf;
      constructInPlace(retCmd);
      
      std::sort(vExploitList.begin(),vExploitList.end(),lessExploit);
      
      tempPoint = (Cmd::stWaitGenItem *)retCmd->data;

      for (DWORD i=0; i<vExploitList.size(); i++)
      {
        if (i<10)
        {
          tempPoint->dwCharID = vExploitList[i].dwUserID;
          strncpy(tempPoint->szName,vExploitList[i].szName,MAX_NAMESIZE);
          tempPoint++;
          retCmd->dwSize++;  
        }
        else{
          break;
        }
      }
    
      if (retCmd->dwSize > 0)
      {
        user->sendCmdToMe(retCmd,(retCmd->dwSize*sizeof(Cmd::stWaitGenItem)+
              sizeof(Cmd::stRtnWaitGenUserCmd)));
        
        user->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ǰû�п������Ĵ�ѡ���������Ժ�����");
      }
    }
    
    bool exec(UserSession* user)
    {
      if (user && user->dwExploit>0 && !CCountryM::getMe().isKing(user)  
          && !CCityM::getMe().isCastellan(user)
          && !CArmyM::getMe().isCaptain(user->id) && user->country == _dwCountryID)
      {
        CGraceUser temp;
        temp.dwUserID = user->id;
        temp.dwUseJob = user->dwUseJob;
        temp.dwExploit = user->dwExploit;
        strncpy(temp.szName,user->name,MAX_NAMESIZE);
        vExploitList.push_back(temp);
      }
      
      return true;
    }
  };

  struct sortAllUser allUser(pUser->country);
  execEveryUser(allUser);
  allUser.sendSort(pUser);
}

void UserSessionManager::sendCmdByCondition(const Cmd::stNullUserCmd *pstrCmd,const DWORD nCmdLen,Compare* compare)
{
  struct sendCmdToAllUser  : public execEntry<UserSession>
  {
    const Cmd::stNullUserCmd* cmd;
    DWORD cmdLen;
    Compare* compare;
    
    sendCmdToAllUser(const Cmd::stNullUserCmd *pstrCmd,const DWORD nCmdLen,Compare* pfCompare)
    {
      cmd = pstrCmd;
      cmdLen = nCmdLen;
      compare = pfCompare;
    }

    ~sendCmdToAllUser()
    {
    }

    bool exec(UserSession* user)
    {
      if (user && (*compare)(user))
        user->sendCmdToMe(cmd,cmdLen);
      return true;
    }
    
  };

  struct sendCmdToAllUser allUser(pstrCmd,nCmdLen,compare);
  execEveryUser(allUser);
}

/**
 * \brief ɾ��һ��task�ϵ������û��Ự
 * һ��task�й���������
 * \param task taskָ��
 * \return 
 */
void UserSessionManager::removeAllUserByTask(SessionTask *task)
{
  struct offlineAllUserBySessionTask: public execEntry<UserSession>
  {
    SessionTask *task;
    bool exec(UserSession *su)
    {
      if (su->getTask()==task || (su->scene!=NULL && su->scene->getTask()==task))
      {
        CUnionM::getMe().userOffline(su); // ���ڴ������Ա����
        CSchoolM::getMe().userOffline(su);
        CSeptM::getMe().userOffline(su);
        CQuizM::getMe().userOffline(su);
      }

      return true;
    }
  };
  offlineAllUserBySessionTask offline;
  struct removeAllUserBySessionTask :public removeEntry_Pred<UserSession>
  {
    SessionTask *task;
    bool isIt(UserSession *us)
    {
      if (us->getTask()==task || (us->scene!=NULL && us->scene->getTask()==task))
      {
        return true;
      }
      else
        return false;
    }
  };
  removeAllUserBySessionTask rust;
  if (task==NULL) return;

  if (task->getType()==GATEWAYSERVER || task->getType()==SCENESSERVER)
  {
    Zebra::logger->debug(" ����˷�����(%ld,%ld)�������û���Ϣ",task->getID(),task->getType());
    rust.task=task;
    offline.task=task;
  }
  else
  {
    Zebra::logger->debug(" δ֪������(%ld,%ld)",task->getID(),task->getType());
    rust.task=NULL;
    offline.task=NULL;
  }

  if (offline.task!=NULL)
  {
    execEveryUser(offline);
  }
  if (rust.task!=NULL)
  {
    removeUser_if (rust);
    for(DWORD i=0,n=rust.removed.size();i<n;i++)
    {
      SAFE_DELETE(rust.removed[i]);
    }
  }
}

///�����Ự������ʵ��
SceneSessionManager * SceneSessionManager::sm(NULL);

/**
 * \brief ���캯��
 */
SceneSessionManager::SceneSessionManager():zSceneManager()
{
  inited=false;
}

/**
 * \brief ��������
 */
SceneSessionManager::~SceneSessionManager()
{
  final();
}

/**
 * \brief �õ������Ự������ʵ��
 * \return �����Ự������ʵ��ָ��
 */
SceneSessionManager *SceneSessionManager::getInstance()
{
  if (sm==NULL)
    sm=new SceneSessionManager();
  return sm;
}

/**
 * \brief ɾ�������Ự������ʵ��
 */
void SceneSessionManager::delInstance()
{
  SAFE_DELETE(sm);
}

/**
 * \brief �õ�һ��Ψһid
 * \param tempid ������õ���id
 * \return �Ƿ�ɹ�
 */
bool SceneSessionManager::getUniqeID(DWORD &tempid)
{
  return true;
}

/**
 * \brief �ͷ�һ��Ψһ��id
 * \param tempid Ҫ�ͷŵ�id
 */
void SceneSessionManager::putUniqeID(const DWORD &tempid)
{
}

/**
 * \brief ��ʼ�������Ự������
 * \return �Ƿ�ɹ�
 */
bool SceneSessionManager::init()
{
  if (inited)
  {
    Zebra::logger->warn("Scene Session�������Ѿ�����ʼ����...");
    return true;
  }
  inited=true;
  Zebra::logger->info("��ʼ��Scene Session�������ɹ�...");
  return inited;
}

/**
 * \brief ��������������
 */
void SceneSessionManager::final()
{
  if (inited)
  {
    inited=false;
  }
}

/**
 * \brief ������������һ������
 * \param scene Ҫ��ӵĳ���
 * \return �Ƿ���ӳɹ�
 */
bool SceneSessionManager::addScene(SceneSession *scene)
{
  rwlock.wrlock();
  bool ret= addEntry(scene);
  rwlock.unlock();
  return ret;
}

/**
 * \brief �������ֵõ�����ָ��
 * \param name ��������
 * \return ����ָ�룬ʧ�ܷ���0
 */
SceneSession * SceneSessionManager::getSceneByName(const char *name)
{
  return (SceneSession *)zSceneManager::getSceneByName(name);
}

/**
 * \brief ������ʱid�õ�����ָ��
 * \param tempid ������ʱid
 * \return ����ָ�룬ʧ�ܷ���0
 */
SceneSession * SceneSessionManager::getSceneByTempID(DWORD tempid)
{
  return (SceneSession *)zSceneManager::getSceneByTempID(tempid);
}

/**
 * \brief ����id�õ�����ָ��
 * \param id ����id
 * \return ����ָ�룬ʧ�ܷ���0
 */
SceneSession * SceneSessionManager::getSceneByID(DWORD id)
{
  return (SceneSession *)zSceneManager::getSceneByID(id);
}

/**
 * \brief �ӹ������Ƴ�һ������
 * \param scene Ҫ�Ƴ��ĳ���
 */
void SceneSessionManager::removeScene(SceneSession *scene)
{
  rwlock.wrlock();
  removeEntry(scene);
  rwlock.unlock();
}
/**
 * \brief �����������رգ���Ҫע��һЩ��ͼ
 * \param task �ó������������ӵ�SessionTask
 */
void SceneSessionManager::removeAllSceneByTask(SessionTask *task)
{
  struct removeAllSceneByTask :public removeEntry_Pred<SceneSession>
  {
    SessionTask *task;
    bool isIt(SceneSession *ss)
    {
      return (ss->getTask()==task);
    }
  };
  if (task==NULL || task->getType()!=SCENESSERVER) return;
  removeAllSceneByTask rust;
  rust.task=task;
  removeScene_if (rust);
  for(DWORD i=0,n=rust.removed.size();i<n;i++)
  {
    SAFE_DELETE(rust.removed[i]);
  }
}

/**
 * \brief �����ļ����ֵõ�����ָ��
 * \param name �ļ�����
 * \return �ҵ��ĳ���ָ�룬ʧ�ܷ���0
 */
SceneSession * SceneSessionManager::getSceneByFile(const char *name)
{
  struct GetSceneByFileName: public execEntry<SceneSession>
  {
    SceneSession *ret;
    const char *_name;

    GetSceneByFileName(const char* name) : ret(NULL),_name(name)
    { }
    
    bool exec(SceneSession *scene)
    {
      if (strncmp(scene->file.c_str(),_name,MAX_NAMESIZE)==0)
      {
        ret = scene;
        return false;
      }
      else
        return true;
    }
  };

  GetSceneByFileName gsfn(name);
  execEveryScene(gsfn);
  return gsfn.ret;
}

/**
 * \brief ���ù��ҵ�˰��
 * \param byTax ˰��
 */
void SceneSessionManager::notifyCountryTax(DWORD dwCountry,BYTE byTax)
{
  struct findall : public execEntry<SceneSession> 
  {
    Cmd::Session::t_taxCountry_SceneSession send;
    findall(DWORD dwCountry,BYTE byTax)
    {
      send.dwCountryID = dwCountry;
      send.byTax = byTax;
    }

    ~findall(){}

    bool exec(SceneSession *scene)
    {
      if (scene)
      {
        send.dwTempID = scene->tempid;
        scene->sendCmd(&send,sizeof(send));
      }
      return true;
    }
  };

  findall myList(dwCountry,byTax);
  execEveryScene(myList);
}
