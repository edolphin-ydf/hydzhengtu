/**
 * \brief ��Ϸȫ�ֻỰ������ 
 */
#pragma once
#include <zebra/srvEngine.h>
#include <set>
//��Ҫ�滻
#include <hash_set>
#include <hash_map>
#include <vector>
#include <map>

//sky �Ŷ�ϵͳ��������ӽṹ��Ԥ����
struct QueueTeamData;
struct CampData;
class CQueueManager;
class CQueuingManager;
class CArenaManager;

class UserSession;
class SessionTask;

class Recommend;

class CDare;
class CNpcDareObj;

class CQuiz;
class CVoteM;
class CTech;
class CGem;
class CArmy;

class CUnion;
class CUnionMember;

class CSchool;
class CSchoolMember;

class CSept;
class CSeptMember;

//sky ս���ȼ����������С�ȼ�
#define MAX_ARENA_USERLEVEL 60
#define MIN_ARENA_USERLEVEL 10

//sky �糡����Ա��ʱ�б�
extern std::map<DWORD, DWORD> MoveSceneMemberMap;
extern zMutex g_MoveSceneMemberMapLock;

//sky ����ս�������б�
extern std::set<SessionTask *> setBattleTask;

#pragma pack(1)

struct auctionBidInfo
{
  DWORD auctionID;
  DWORD ownerID;
  char owner[MAX_NAMESIZE+1];
  BYTE state;
  DWORD charge;
  DWORD minMoney;
  DWORD maxMoney;
  DWORD minGold;
  DWORD maxGold;
  DWORD bidderID;
  DWORD bidder2ID;
  char bidder[MAX_NAMESIZE+1];
  char bidder2[MAX_NAMESIZE+1];
  DWORD endTime;
  BYTE bidType;
  DWORD itemID;
  Cmd::Session::SessionObject item;
};

#pragma pack()

class AuctionService
{
  private:
    static AuctionService *as;
    AuctionService();
    ~AuctionService();
  public:
    static AuctionService& getMe();
    static void delMe();

    bool doAuctionCmd(const Cmd::Session::t_AuctionCmd *cmd,const DWORD cmdLen);
    void checkDB();

    bool sendAuctionItem(DWORD,DWORD,BYTE,bool);
    void delAuctionRecordByName(char *);

    static bool error(const char * msg,...);
};

#pragma pack(1)

struct mailHeadInfo
{
  DWORD id;
  BYTE state;
  char fromName[MAX_NAMESIZE+1];
  DWORD delTime;
  BYTE accessory;
  BYTE itemGot;
  BYTE type;
};

struct mailContentInfo
{
  DWORD id;
  BYTE state;
  char toName[MAX_NAMESIZE+1];
  char title[MAX_NAMESIZE+1];
  BYTE accessory;
  BYTE itemGot;
  char text[256];
  DWORD sendMoney;
  DWORD recvMoney;
  DWORD sendGold;
  DWORD recvGold;
  DWORD toID;
  Cmd::Session::SessionObject item;
};

struct mailStateInfo
{
  BYTE state;
};

struct mailNewInfo
{
  DWORD id;
  DWORD toID;
};

struct mailCheckInfo
{
  DWORD id;
  char toName[MAX_NAMESIZE+1];
  DWORD toID;
};

struct mailTurnBackInfo
{
  BYTE state;
  char fromName[MAX_NAMESIZE+1];
  char toName[MAX_NAMESIZE+1];
  char title[MAX_NAMESIZE+1];
  BYTE type;
  DWORD createTime;
  DWORD delTime;
  BYTE accessory;
  BYTE itemGot;
  char text[256];
  DWORD recvMoney;
  DWORD recvGold;
  DWORD fromID;
  DWORD toID;
};

struct mailForwardInfo
{
  BYTE state;
  char fromName[MAX_NAMESIZE+1];
  char toName[MAX_NAMESIZE+1];
  BYTE type;
  DWORD delTime;
  char text[256];
  DWORD recvMoney;
  DWORD recvGold;
  DWORD toID;
};

#pragma pack()

class MailService : public Singleton<MailService>
{
  friend class SingletonFactory<MailService>;
  private:
    hash_map<DWORD,hash_set<DWORD> > newMailMap;
    MailService();
  public:
    ~MailService();

    void loadNewMail();
    bool doMailCmd(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);
    bool sendTextMail(char *,DWORD,char *,DWORD,char *,DWORD=(DWORD)-1,BYTE=1);
    bool sendMoneyMail(char *,DWORD,char *,DWORD,DWORD,char *,DWORD=(DWORD)-1,BYTE=1,DWORD=0);
    bool sendMail(Cmd::Session::t_sendMail_SceneSession &);
    bool sendMail(DWORD,Cmd::Session::t_sendMail_SceneSession &);
    bool turnBackMail(DWORD);
    void checkDB();

    void delMailByNameAndID(char *,DWORD);
};

/**
 * \brief ������Ϣ������
 *
 * �ṩ�˶�������Ϣ�Ĺ���
 *
 */
class COfflineMessage
{
private:
  static std::string rootpath;
public:

  static bool init();

  static void writeOfflineMessage(const BYTE &type,const DWORD &id,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);

  static void getOfflineMessage(const UserSession *pUser);

  static void getOfflineMessageSetAndSend(const UserSession *pUser,std::string path);
};

#pragma pack(1)

struct cartoon_load_struct
{
  DWORD cartoonID;        
  Cmd::t_CartoonData data;
  cartoon_load_struct()
  {
    cartoonID = 0;
    bzero(&data,sizeof(data));
  }
};

struct cartoon_relation_struct
{
  DWORD relationID;        
  BYTE type;
};

#pragma pack()

class CartoonPetService
{
  private:
    /*struct key_hash : public std::unary_function<const std::string,size_t>
    {
      size_t operator()(const std::string &x) const
      {
        hash<const char *> H;
        return H(x.c_str());
      }

	  static const int bucket_size = 1000000;
	  static const int min_buckets = 1000000;

    };*/


	//const int key_hash::bucket_size = 10000000;
	//const int key_hash::min_buckets = 10000000;

    hash_map<DWORD,Cmd::t_CartoonData> cartoonPetList;
    hash_set<DWORD> modifyList;
    hash_set<DWORD> waitingList;
    hash_map<DWORD,hash_set<DWORD> > cartoonPetMap;
    hash_map<std::string,hash_set<DWORD>/*,key_hash*/> adoptedPetMap;

    static CartoonPetService *cs;
    CartoonPetService();
    ~CartoonPetService();

    void sendCmdToItsFriendAndFamily(DWORD,const char *,const void *,DWORD,const char * = "");
    void repairData();

    bool loadAllFromDB();
    DWORD group;
  public:
    DWORD writeAllToDB(bool groupflag=true);

    static CartoonPetService& getMe();
    static void delMe();

    bool doCartoonCmd(const Cmd::Session::t_CartoonCmd *cmd,const DWORD cmdLen);
    void loadFromDB(DWORD);
    void checkAdoptable(DWORD);
    bool writeDB(DWORD,Cmd::t_CartoonData&);
    void userLevelUp(DWORD id,DWORD level);

    void delPetRecordByID(DWORD masterID);

    void userOnline(UserSession * pUser);
};

const DWORD KING_CITY_ID = 139;

typedef std::set<DWORD> DareSet;

class CCity
{
  public:
    CCity()
    {
      dwCountry = 0;
      dwCityID = 0;
      dwUnionID = 0;
      isAward = 0;
      dwGold = 20000;
      bzero(catcherName,MAX_NAMESIZE);
      //dwDareUnionID = 0;
      vDareList.clear();
    }
    
    ~CCity()
    {
    }
    
    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool isMe(DWORD country,DWORD cityid,DWORD unionid);
    bool changeUnion(DWORD unionid);
    bool changeCatcher(UserSession* pUser);
    bool cancelCatcher();
    bool abandonCity();
    bool addDareList(DWORD dwUnionID);
    bool isDare(DWORD dwUnionID);
    size_t dareSize()
    {
      return vDareList.size();
    }
    
    char* getName();

    void   beginDare();
    void   endDare();  

    DWORD dwCountry;
    DWORD dwCityID;
    DWORD dwUnionID;
    int   isAward;  // �����Ǯ���Ƿ��Ѿ�ˢ��
    DWORD dwGold;
    char  name[MAX_NAMESIZE];
    char  catcherName[MAX_NAMESIZE];
    
    //DWORD dwDareUnionID;
    DareSet vDareList;
    
    zRWLock rwlock;
};

class CCityM : public Singleton<CCityM>
{
  friend class SingletonFactory<CCityM>;
  public:
    bool init();
    static void destroyMe();

    void timer();

    bool load();
    bool refreshDB();
    void refreshUnion(DWORD dwCountryID,DWORD dwCityID);
    bool addNewCity(Cmd::Session::t_UnionCity_Dare_SceneSession* pCmd);

    
    CCity* find(DWORD country,DWORD cityid,DWORD unionid);
    CCity* find(DWORD country,DWORD cityid);
    CCity* findByUnionID(DWORD unionid);
    CCity* findDareUnionByID(DWORD unionid);
    bool   isCastellan(UserSession* pUser);
    bool   isCatcher(UserSession* pUser);

    void   awardTaxGold(UserSession *pUser);

    struct cityCallback
    {
      virtual void exec(CCity *)=0;
      virtual ~cityCallback(){};
    };

    void execEveryCity(cityCallback &);//�������޹�������   
    
    void beginDare();
                void endDare();
  private:
    CCityM();
    std::vector<CCity*> citys;
    bool isBeging;
    zRWLock rwlock;
};

/// �����ʮСʱ����Ϊ��λ������
#define MAX_GROUP_TIME_GAP 60*60*60

/// �Ѻöȿ۳�������λΪ����
#define DEDUCT_POINT 5*60

struct CRelation : public zEntry
{
public:
  ///  ��ϵ���� 
  BYTE  type;

  ///  ��ϵ����
  WORD  level;

  ///  �û�Session����
//  UserSession *user;
  bool online;

  /// �û���ɫID
  DWORD charid;

  ///  ������ʱ��
  DWORD lasttime;

  ///  ְҵ
  WORD  occupation;

  CRelation();
  void sendNotifyToScene();
  bool isOnline();
private:
  /// ��д��
  //zRWLock rwlock;
};

class CRelationManager : public zEntryManager<zEntryID,zEntryName>
{
private:
  /// ��д��
  //zRWLock rwlock;

  /// �û�����
  UserSession *user;

public:
  ~CRelationManager();
  template <class YourEntry>
  bool execEveryOne(execEntry<YourEntry> &exec)
  {
    //rwlock.rdlock();
    bool ret=execEveryEntry<>(exec);
    //rwlock.unlock();
    return ret;
  }


  void init();
  void loadFromDatabase();
  void deleteDBRecord(const DWORD dwID);
  bool insertDBRecord(const CRelation *);
  void updateDBRecord(const CRelation *relation);
  void updateOtherOfflineUserDBRecord(const CRelation *relation);
  void writeOfflineNotify(const CRelation *relation);
//  void writeDatabase();
  bool processUserMessage(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
  void removeRelation(const char *);
  void addBadRelation(const char *name);
  void addEnemyRelation(const char *name);
  inline void sendStatusChange(const CRelation *,const BYTE byState);
  void addRelation(const DWORD dwID,const BYTE type);
  void changeRelationType(const char * name,const BYTE type);
  void sendRelationList();
  void online(const DWORD dwID);
  void offline(const DWORD dwID,const char* name);
  void setUser(UserSession *);
  void sendChatToMyFriend(const Cmd::stChannelChatUserCmd *,const DWORD cmdLen);
  void sendCmdToMyFriend(const void *,const DWORD,bool sendMe=true);
  void sendCmdToMyFriendExcept(const void *,const DWORD,bool sendMe=true,const char * = "");
  void sendPrivateChatToFriend(const Cmd::stChannelChatUserCmd *,const DWORD cmdLen);
  void sendBlackListToGateway(const char *,const BYTE oper);
  void sendAllBlackListToGateway();
  CRelation*  getRelationByType(int relationType);
  CRelation * getRelationByName(const char *);
  CRelation*  getRelationByID(DWORD dwRelationID);
  CRelation*  getMarryRelation();
  void setFriendDegree(Cmd::Session::t_CountFriendDegree_SceneSession *);
  void setFriendDegreeByOtherUser(const DWORD dwUserID,const WORD wdDegree,const DWORD currTime);
};

class SessionChannel : public zEntry
{               
  private:
    std::list<DWORD> userList;
    //char creater[MAX_NAMESIZE];
  public:
    bool sendToOthers(UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,DWORD cmdLen);

    //bool sendToAll(UserSession *,const char *pattern,...);
    bool sendCmdToAll(const void *cmd,int len);
    SessionChannel(UserSession *);
    bool remove(DWORD);
    bool remove(UserSession *pUser);
    bool removeAllUser();
    //bool add(const char *name);
    bool add(UserSession *pUser);
    bool has(DWORD);
    DWORD count();

    static bool sendCountry(DWORD,const void *rev,DWORD cmdLen);
    static bool sendCountryInfo(int type,DWORD countryID,const char* mess,...);
    static bool sendAllInfo(int type,const char* mess,...);
    static bool sendAllCmd(const void *cmd,const DWORD len);
    static bool sendPrivate(UserSession * pUser,const char * fromName,const char* mess,...);
};

typedef zUniqueID<DWORD> zUniqueDWORDID;
class SessionChannelManager : public zEntryManager< zEntryTempID,zEntryName >
{
  private:
    //std::map<DWORD,SessionChannel *> channelList;
    zUniqueDWORDID *channelUniqeID;
    bool getUniqeID(DWORD &tempid);
    void putUniqeID(const DWORD &tempid);
    static SessionChannelManager * scm;

    SessionChannelManager();
    ~SessionChannelManager();
  public:
    static SessionChannelManager & getMe();
    static void destroyMe();
    bool add(SessionChannel *);
    void remove(DWORD);
    SessionChannel * get(DWORD);
    //void removeUser(DWORD);
    void removeUser(UserSession *);
};

/**
 * \brief ��������������
 *
 */
class SessionTask : public zTCPTask, public zEntry,public MessageQueue
{

  public:

    /**
     * \brief ���캯��
     *
     * \param pool �������ӳ�ָ��
     * \param sock TCP/IP�׽ӿ�
     * \param addr ��ַ
     */
    SessionTask(
        zTCPTaskPool *pool,
        const SOCKET sock,
        const struct sockaddr_in *addr = NULL) : zTCPTask(pool,sock,addr)
    {
      wdServerID = 0;
      wdServerType = UNKNOWNSERVER;
      recycle_state = 0;
    }

    /**
     * \brief ����������
     *
     */
    virtual ~SessionTask();

    int verifyConn();
    int recycleConn();
    void addToContainer();
    void removeFromContainer();
    bool uniqueAdd();
    bool uniqueRemove();
    bool msgParse(const Cmd::t_NullCmd *,const DWORD);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);

    /**
     * \brief ���ط��������
     *
     * �����һ��������Ψһ�ģ������ڹ����������
     *
     * \return ���������
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief ���ط���������
     *
     * \return ����������
     */
    const WORD getType() const
    {
      return wdServerType;
    }
    bool checkRecycle();

  private:

    /**
     * \brief ���������
     *
     */
    WORD wdServerID;

    /**
     * \brief ����������
     *
     */
    WORD wdServerType;
    ///���û��ձ�־
    int recycle_state;

    bool verifyLogin(const Cmd::Session::t_LoginSession *ptCmd);

    /**
     * \brief ɾ����ɫ�Ĵ���
     *
     * �ӽ�ɫ��Ӧ������ϵ�а���ɾ���������ɫ��ĳһ����ϵ�Ľ�����������ɾ���ý�ɫ
     * �������ɾ������֪ͨGATEWAY����ӵ�����ɾ�������������ɾ��������һ��֪ͨ������ͻ��ˡ�
     *
     *
     */
    bool del_role(const Cmd::t_NullCmd* cmd,const DWORD cmdLen);
    /**
     * \brief ��������
     *
     * \param dwUserID : �����������û�ID
     *
     */
    bool SessionTask::change_country(const Cmd::Session::t_changeCountry_SceneSession* cmd);
    bool msgParse_Scene(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);
    bool msgParse_Gate(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);
    bool msgParse_Forward(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);

	//sky ����ս��������Session��Ϣ������
	bool msgParse_Arena(const Cmd::t_NullCmd *cmd, const DWORD cmdLen);

};

/**
 * \brief �Ự�����������ӹ�����
 *
 */
class SessionTaskManager : private zNoncopyable
{

  public:

    /**
     * \brief ��������
     *
     */
    ~SessionTaskManager() {};

    /**
     * \brief ��ȡ�����ӹ�����Ψһʵ��
     *
     * \return ������Ψһʵ��
     */
    static SessionTaskManager &getInstance()
    {
      if (NULL == instance)
        instance = new SessionTaskManager();

      return *instance;
    }

    /**
     * \brief �ͷ����Ψһʵ��
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    void addSessionTask(SessionTask *task);
    void removeSessionTask(SessionTask *task);
    bool uniqueAdd(SessionTask *task);
    bool uniqueRemove(SessionTask *task);
    bool broadcastGateway(const void *pstrCmd,int nCmdLen);
    bool broadcastScene(const void *pstrCmd,int nCmdLen);
    bool sendCmdToWorld(const void *pstrCmd,int nCmdLen);
    bool broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen);
    void execEvery();
    bool sendCmdToCountry(DWORD country,const void *pstrCmd,int nCmdLen);

  private:

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static SessionTaskManager *instance;

    /**
     * \brief ���캯��
     *
     */
    SessionTaskManager() {};

    /**
     * \brief ������������
     *
     */
    typedef hash_map<WORD,SessionTask *> SessionTaskHashmap;
    /**
     * \brief ������������������
     *
     */
    typedef SessionTaskHashmap::iterator SessionTaskHashmap_iterator;
    /**
     * \brief ����������������������
     *
     */
    typedef SessionTaskHashmap::const_iterator SessionTaskHashmap_const_iterator;
    /**
     * \brief ����������ֵ������
     *
     */
    typedef SessionTaskHashmap::value_type SessionTaskHashmap_pair;
    /**
     * \brief ������������
     *
     */
    typedef std::list<SessionTask *> TaskContainer;
    /**
     * \brief ������������������
     *
     */
    typedef TaskContainer::iterator TaskContainer_iterator;
    /**
     * \brief �������ʻ������
     *
     */
    zRWLock rwlock;
    //zMutex mlock;
    /**
     * \brief ����һ��������������е�������
     *
     */
    SessionTaskHashmap sessionTaskSet;
    /**
     * \brief �볡�����������ӵ�����
     *
     */
    TaskContainer sceneTaskList;
    /**
     * \brief �����ط��������ӵ�����
     *
     */
    TaskContainer gatewayTaskList;

};


/**
 * \brief �Ự��
 * �û��Ự�ͳ����Ự�Ļ���
 *
 */
class Session:private zNoncopyable
{

  private:
    ///����ʱ��
    time_t createtime;
    ///�ûỰ������
    SessionTask *task;

  protected:

    /**
     * \brief ���캯��
     * \param task �ûỰ������
     */
    Session(SessionTask *task)
    {
      createtime=time(NULL);
      this->task=task;
    }

  public:
    ///��������������
    DWORD reqAdopter;

    /**
     * \brief �õ��ûỰ������
     * \return �ûỰ������
     */
    SessionTask *  getTask() const
    {
      return task;
    }
    
    /**
     * \brief ��Է�������Ϣ
     * \param pstrCmd Ҫ���͵���Ϣ
     * \param nCmdLen ��Ϣ����
     * \return �����Ƿ�ɹ�
     */
    bool sendCmd(const void *pstrCmd,const int nCmdLen) const
    {
      if (task)
        return task->sendCmd(pstrCmd,nCmdLen);
      else
        return false;
    }

};

/**
 * \brief �����Ự
 *
 */
class SceneSession:public zScene,public Session
{

  public:

    ///��ǰ��ͼ����������ҵȼ�Ϊ0��ʾ������
    BYTE level;
    /**
     * \brief ���캯��
     */
    SceneSession::SceneSession(SessionTask *task):zScene(),Session(task)
    {
    }

    /**
     * \brief ͨ����Ϣע��һ����ͼ
     * \param reginfo ��ͼע����Ϣ
     * \return �Ƿ�ע��ɹ�
     */
    bool SceneSession::reg(Cmd::Session::t_regScene_SceneSession *reginfo)
    {
      if (reginfo)
      {
        id=reginfo->dwID;
        tempid=reginfo->dwTempID;
        strncpy(name,(char *)reginfo->byName,MAX_NAMESIZE);
        file = reginfo->fileName;
        level = reginfo->byLevel;
        return true;
      }
      else
        return false;
    }

    ///��Ӧ�ĵ�ͼ�ļ���
    std::string file;
};

/**
 * \brief �û��Ự��
 *
 */
class UserSession:public zUser,public Session
{

  public:
    //std::set<DWORD> cartoonList;
    //std::set<DWORD> adoptList;

    char autoReply[MAX_CHATINFO];//�Զ��ظ�

    //����
    static DWORD user_count;

    //��������
    static std::map<DWORD,DWORD> country_map;

    ///�ʺ�id
    DWORD accid;
    ///���id
    DWORD unionid;
    ///����id
    DWORD country;
    /// ��������
    BYTE countryName[MAX_NAMESIZE+1];
    ///����id
    DWORD septid;
    ///����id
    DWORD schoolid;
    ///������ʱid
    DWORD teamid;
    ///�ȼ�
    WORD  level;
    ///���ְλ
    WORD  occupation;
    /// ����ʱ��
    zRTime regTime;
    ///ͷ��
    DWORD  face;
    
    // ��ѫֵ
    DWORD dwExploit;

    // ����ְҵ
    DWORD dwUseJob;

    // ���ﵱǰ����
    QWORD qwExp;

    ///ϵͳ������Ϣ
    BYTE sysSetting[20];//ϵͳ����

    ///���ڵĳ����Ự
    SceneSession *scene;
    ///����ϵ������
    CRelationManager relationManager;

    ///�´η��̻��ʱ��
    zRTime nextBlessTime;

    //���徭��
    WORD septExp;


    UserSession(SessionTask *task);
    ~UserSession();

    /**
     * \brief ����ע��һ�����
     * \param reginfo ��������Ϣ����Ϣ
     * \return �Ƿ�ע��ɹ�
     */
    bool reg(Cmd::Session::t_regUser_GateSession *reginfo)
    {
      if (reginfo)
      {
        accid=reginfo->accid;
        id=reginfo->dwID;
        tempid=reginfo->dwTempID;
        //TODO septid ��ʼ��
        level=reginfo->wdLevel;
        occupation=reginfo->wdOccupation;
        country = reginfo->wdCountry;
        std::map<DWORD,DWORD>::iterator iter = country_map.find(country);
        if (iter == country_map.end())
        {
          country_map[country] = 0;
        }
        country_map[country]++;
        strncpy(name,(char *)reginfo->byName,MAX_NAMESIZE);
        strncpy((char*)countryName,(char*)reginfo->byCountryName,MAX_NAMESIZE);
        relationManager.setUser(this);
        return true;
      }
      else
        return false;
    }
    static void getCountryUser(std::vector<std::pair<DWORD,DWORD> > &v)
    {
      std::map<DWORD,DWORD>::iterator iter;
      for( iter = country_map.begin() ; iter != country_map.end() ; iter ++)
      {
        v.push_back( std::make_pair(iter->first,iter->second) );
      }
    }

    /**
     * \brief ��������ϵ��Ϣ
     * \param regsuccess ע��ɹ�����Ϣ
     * \return 
     */
    bool setRelationData(const Cmd::Session::t_regUserSuccess_SceneSession *regsuccess)
    {
      if (regsuccess)
      {
        unionid  = regsuccess->dwUnionID;
        country = regsuccess->dwCountryID;
        septid = regsuccess->dwSeptID;
        schoolid = regsuccess->dwSchoolID;
        return true;
      }
      else
        return false;
    }

    /**
      * \brief ������ż��Ϣ������
      *
      */
    void updateConsort();
    
    /**
      * \brief ������ż��Ϣ������
      *
      */
    void updateCountryStar(); 

    bool checkChatCmd(const Cmd::stNullUserCmd *,const DWORD nCmdLen) const;
    void sendCmdToMe(const void *pstrCmd,const DWORD nCmdLen) const;
    void sendSysChat(int type,const char *pattern,...) const;
    void sendGmChat(int type,const char* pattern,...) const;
    void setFriendDegree(Cmd::Session::t_CountFriendDegree_SceneSession *);
    void sendFriendDegree(Cmd::Session::t_RequestFriendDegree_SceneSession *);
    bool forwardScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
};

/**
 * \brief �û��Ự������
 *
 */
class UserSessionManager:public zUserManager
{
  private:
    ///������ʵ��
    static UserSessionManager *sm;

    ///�Ƿ��ʼ���ɹ�
    bool inited;

    UserSessionManager();
    ~UserSessionManager();
    inline bool getUniqeID(DWORD &tempid);
    inline void putUniqeID(const DWORD &tempid);

  public:
    static UserSessionManager *getInstance();
    static void delInstance();
    struct Compare
    {
      virtual bool operator ()(UserSession*) = 0;
      virtual ~Compare(){}
    };

    bool init();
    void final();
    UserSession *getUserSessionByName( const char * name);
    UserSession *getUserByTempID(DWORD tempid);
    UserSession *getUserByID(DWORD id);
    void sendGraceSort(UserSession* pUser);
    void sendExploitSort(UserSession* pUser);
    void sendCmdByCondition(const Cmd::stNullUserCmd *pstrCmd,const DWORD nCmdLen,Compare* compare);
	void removeAllUserByTask(SessionTask *task);
	void notifyOnlineToGate(); // [ranqd Add] ֪ͨ����GateWay��ǰ�Ĺ����������������Դ���ıȽϴ�5���ӵ���һ��
};


/**
 * \brief �����Ự������
 *
 */
class SceneSessionManager:public zSceneManager
{
  private:
    ///������ʵ��
    static SceneSessionManager *sm;

    ///�Ƿ��ʼ���ɹ�
    bool inited;

    SceneSessionManager();
    ~SceneSessionManager();
    bool getUniqeID(DWORD &tempid);
    void putUniqeID(const DWORD &tempid);

  public:
    static SceneSessionManager *getInstance();
    static void delInstance();

    bool init();
    void final();

    bool addScene(SceneSession *scene);
    SceneSession * getSceneByName(const char *name);
    SceneSession * getSceneByFile(const char *file);
    SceneSession * getSceneByID(DWORD id);
    SceneSession * getSceneByTempID(DWORD tempid);
    void removeAllSceneByTask(SessionTask *task);
    void removeScene(SceneSession *scene);
    void notifyCountryTax(DWORD dwCountry,BYTE byTax);

};

class zDBConnPool;
class DBMetaData;

/**
 * \brief ����ÿ���û��Ự��ͬһ���ҵĽ�ɫ���͵�����
 */
struct OneCountryScene: public execEntry<UserSession>
{
  DWORD country;
  DWORD cmdLen;
  Cmd::t_NullCmd* sendCmd;

  void init(Cmd::t_NullCmd * rev,DWORD len,DWORD countryID)
  {
    sendCmd = rev;
    cmdLen = len;

    country = countryID;
  }

  /**
   * \brief ����ÿ���û��Ự��ͬһ���ҵĽ�ɫ���ͳ�������
   * \param su �û��Ự
   * \return true �ɹ� false ʧ��
   */
  bool exec(UserSession *su)
  {
    if (country == su->country && su->scene)
    {
      if (sendCmd->para == Cmd::Session::PARA_ENTERWAR)
      {
        ((Cmd::Session::t_enterWar_SceneSession*)sendCmd)->dwUserID = su->id;
      }
      
      su->scene->sendCmd(sendCmd,cmdLen);
    }
    return true;
  }
};

struct worldMsg
{
  char msg[256];
  DWORD time;
  DWORD interval;
  DWORD count;
  DWORD country;
  DWORD mapID;
  char GM[32];
  worldMsg()
  {
    bzero(msg,sizeof(msg));
    time = 0;
    interval = 0;
    count = 0;
    country = 0;
    mapID = 0;
    bzero(GM,sizeof(GM));
  }
};

/**
 * \brief Session������
 *
 * ��Ϸȫ�ֵĻỰ������
 *
 */
class SessionService : public zSubNetService
{

  public:

    bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief ������������
     *
     */
    ~SessionService()
    {
      instance = NULL;

      //�ر��̳߳�
      if (taskPool)
      {
        taskPool->final();
        SAFE_DELETE(taskPool);
      }
    }

    const int getPoolSize() const
    {
      if(taskPool)
      {
        return taskPool->getSize();
      }
      else
      {
        return 0;
      }
    }

    /**
     * \brief ��ȡ���Ψһʵ��
     *
     * �����ʹ����Singleton����֤һ��������ֻ��һ�����ʵ��
     *
     * \return ���Ψһʵ��
     */
    static SessionService &getInstance()
    {
      if (NULL == instance)
        instance = new SessionService();

      return *instance;
    }

    /**
     * \brief �ͷ����Ψһʵ��
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    void reloadConfig();
    bool isSequeueTerminate() 
    {
      return taskPool == NULL;
    }
    
    /**
     * \brief ָ�����ݿ����ӳ�ʵ����ָ��
     *
     */
    static zDBConnPool *dbConnPool;
    static DBMetaData* metaData;
    bool checkShutdown();
    //bool checkGumu();

    /**
     * \brief ������ǿ��
     *
     */
    void checkCountry(struct tm &tmValue,bool donow = false);
    Cmd::Session::t_shutdown_SceneSession shutdown_time;
    
    static std::map<BYTE,worldMsg> wMsg;
    //static DWORD snCount;
    static std::map<DWORD,BYTE> userMap;
    DWORD countryLevel[13];
    bool uncheckCountryProcess;

    static bool reportGm(const char * fromName,const char *msg,...);

    static DWORD emperorForbid[10];//�ʵ۽��Թ������
    DWORD loadEmperorForbid();//�����ݿ��
    void saveEmperorForbid();//д���ݿ�
    void clearEmperorForbid();//ÿ�������¼
  private:

    DWORD gumutime;
    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static SessionService *instance;

    zTCPTaskPool *taskPool;        /**< TCP���ӳص�ָ�� */

    /**
     * \brief ���캯��
     *
     */
    SessionService() : zSubNetService("�Ự������",SESSIONSERVER)
    {
      taskPool = NULL;
      bzero(&shutdown_time,sizeof(shutdown_time));
      gumutime=0;
    }

    bool init();
    void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
    void final();
};

//enum
//{
//  SILK_TECH=1, // '�������װ'
//  SKIN_TECH=2, // 'Ƥ�����װ'
//  LORICAE_TECH=3,  // '�������װ'
//  SWORD_TECH=4,  // '������'
//  BOW_TECH=5,  // '����'
//  WAND_TECH=6,  // '����'
//  STAFF_TECH=7,  // '����'
//  FAN_TECH=8,  // '����'
//  NECKLACE_TECH=9,  // '������'
//  FINGER_TECH=10,  // '��ָ��'
//  CUFF_TECH=11,  // '������'
//  GIRDLE_TECH=12,  // '������'
//  SHOES_TECH=13,  // 'Ь����'
//  ARMET_TECH=14,  // 'ͷ����'
//};

const int TECH_MAX_NUM = 15; // �Ƽ��������Ŀ
const int COUNTRY_MATERIAL = 0; // ���Ҳֿ��е�����
const int COUNTRY_STOCK = 1;  // ���Ҳֿ��е�ԭ�� 
const int COUNTRY_MONEY = 2; // ���Ҳֿ��е�����
const int COUNTRY_ALLY_NPC_HORTATION_MONEY = 40 * 10000; // 40 ��
const int COUNTRY_ALLY_NPC_HORTATION_MATERIAL = 1000; // 1000����λ������
const DWORD NEUTRAL_COUNTRY_ID = 6; // ������ID

class CTech
{
  public:
    enum 
    {
      INIT_TECH, // ��ʼ״̬
      WAIT_TECH, // ͶƱѡ���ĿƼ�,�ȴ�����ѡ���Ա
      ACTIVE_TECH,   // ���ڽ�������
      FINISH_TECH, // �Ѿ��������,���Դ�����Ӧװ��(�����ȼ�����0������״̬�ĿƼ�)
    };
    
    CTech();
    ~CTech();

    void init(DBRecord* rec);
    void writeDatabase();
    void upLevel(UserSession* pUser);
    void setSearcher(UserSession* pUser);
    void clearSearcher(UserSession* pUser);
      
    DWORD state()
    {
      return this->dwStatus;
    }

    void state(DWORD newState)
    {
      rwlock.wrlock();
      this->dwStatus = newState;
      rwlock.unlock();
      this->writeDatabase();
    }

    DWORD type()
    {
      return this->dwType;
    }


    DWORD level()
    {
      return this->dwLevel;
    }

    DWORD dwUID;
    DWORD dwCountryID;
    DWORD dwType;
    char  szName[MAX_NAMESIZE];
    DWORD dwProgress;
    DWORD dwResearchID;
    char  szResearchName[MAX_NAMESIZE];
    DWORD dwStatus;
    DWORD dwLevel;
    DWORD dwLastUpTime;

    zRWLock rwlock;
};

class CCountry
{
  public:
    CCountry()
    {
      dwID = 0;
      dwKingUnionID = 0;
      dwLastDareTime = 0;
      dwLastDailyMoney = 0;
      dwDareTime = 0;
      dwDareCountryID = 0;
      bzero(name,sizeof(name));
      bzero(kingName,sizeof(kingName));
      dwFormalWin = 0;
      dwFormalFail  = 0;
      dwAnnoyWin = 0;
      dwAnnoyFail = 0;
      dwStar   = 0;
      dwTax = 0;  ///˰��
      qwGold = 0;  ///����
      isBeging = false;
      qwSilk  =  0;  // ˿��
      qwOre  =  0;  // ��ʯ
      qwBowlder  =  0;  // ��ʯ
      qwWood  =  0;  // ľ��
      qwCoat  =  0;  // Ƥë
      qwHerbal  =  0;  // ��ҩ
      qwMaterial  =  0; // ����
      qwStock    =  0; // ԭ��
      forbidTalk  =  0; //����
      winner_exp = 0;
      winner_time = 0;
      bzero(note,sizeof(note));
      sendPrison = 0;
      gen_level = 0;
      gen_exp = 0;
      gen_maxexp = 0;
      gen_refreshTime = 0;
      calltimes = 0;
      kingtime = 0;
    }
    
    ~CCountry()
    {
    }
    
    void init(DBRecord* rec);
    void writeDatabase();
    void loadTechFromDB();

    bool insertDatabase();
    bool isMe(DWORD country);
    bool isKing(UserSession* pUser);
    bool isOfficial(UserSession* pUser);

    bool changeKing(UserSession* pUser);
    bool changeEmperor(DWORD dwCountryID);
    void updateKing(UserSession* pUser);
    bool changeDiplomat(UserSession* pUser);// �ı��⽻��
    bool cancelDiplomat(); // �����⽻��
    
    bool changeCatcher(UserSession* pUser);// �ı䲶ͷ
    bool cancelCatcher(); // �����⽻��

    void beginDare();
    void beginAntiDare(DWORD dwAttCountry);
    void endDare();


    void addTaxMoney(QWORD qwTaxMoney);
    
    /// ��ʼ���пƼ�ͶƱ
    void beginTechVote();

    DWORD dwID;
    DWORD dwKingUnionID;
    DWORD dwDareTime;  // �ϴη�����ս��ʱ��
    DWORD dwLastDareTime;   // �ϴν�����ս��ʱ��
    DWORD dwLastDailyMoney; // �ʵ��ϴ���ȡ5D������ʱ��
    DWORD dwDareCountryID;
    bool  isBeging;

    char  name[MAX_NAMESIZE+1]; // ��������
    char  kingName[MAX_NAMESIZE+1]; // ��������
    char  diplomatName[MAX_NAMESIZE+1]; // �⽻������
    char  catcherName[MAX_NAMESIZE+1]; // ��ͷ����
    
    DWORD dwFormalWin;
    DWORD dwFormalFail;
    DWORD dwAnnoyWin;
    DWORD dwAnnoyFail;
    DWORD dwStar;
    DWORD dwTax;  ///˰��
    QWORD qwGold;  ///����
    QWORD qwSilk;  // ˿��
    QWORD qwOre;  // ��ʯ
    QWORD qwBowlder;  // ��ʯ
    QWORD qwWood;  // ľ��
    QWORD qwCoat;  // Ƥë
    QWORD qwHerbal;  // ��ҩ
    QWORD qwMaterial; // ����
    QWORD qwStock;  // ԭ��
    DWORD forbidTalk; //��������
    DWORD sendPrison; //�����ؼ���
    DWORD gen_level; //�󽫾��ĵȼ�
    DWORD gen_exp; //�󽫾��ľ���
    DWORD gen_maxexp; //�󽫾��������
    DWORD gen_refreshTime;//�󽫾������ʱ��
    DWORD calltimes;  //����ʹ�ô���
    DWORD kingtime;    //������λʱ��(Сʱ)

    char note[256];

    void  changeStar(int star); // �ı���
    DWORD getStar();

    /// �ı�ָ���������ʵİٷֱ�,0Ϊ��ͨ����,1ԭ��,2Ϊ����
    /// ����ֵΪ,����ĸı�ֵ
    int changeMaterialByPer(int type,float per);
    // �ı�ָ���������ʵ���ֵ
    void  changeMaterial(int type,int value);
    // �õ�ָ�����͵�����
    QWORD getMaterial(int type);
    void swapMaterialByPer(CCountry* pToCountry,float per);

    /// �Ƽ�����
    typedef std::map<DWORD,CTech*> CTechMap;
    CTechMap techIndex;
    CTech* getTech(DWORD dwType);
    void   addTech(DWORD dwType,CTech* pTech);
  
    zRWLock rwlock;
    void setWinnerExp();
    void checkWinnerExp();

    void addGeneralExp(DWORD num);
    void generalLevelDown();
    void refreshGeneral();
  private:
    bool winner_exp;
    DWORD winner_time;
};

class CCountryM : public Singleton<CCountryM>
{
  friend class SingletonFactory<CCountryM>;
  public:
    bool init();
    static void destroyMe();

    void timer();
    void timerPerHour();
    void save();
    void beginDare();
    void endDare();

    void resetCallTimes();
    void beginGem();
    bool load();
    bool refreshDB();
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
    bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);

    void processRequestDare(UserSession* pUser,Cmd::stRequestDareCountryCmd* rev);
    void processDareCountry(UserSession* pUser,Cmd::stDareCountryFormalCmd* rev);
    void processAntiDareCountry(UserSession* pUser,Cmd::stAntiDareCountryFormalCmd* rev);
    void processRequestTech(UserSession* pUser,Cmd::stReqTechUserCmd* rev);
    void processUpTech(UserSession* pUser,Cmd::stUpTechDegreeUserCmd* rev);
    void processSetTechSearch(UserSession* pUser,Cmd::stSetTechSearchUserCmd* rev);
    void processConfirmSearcher(UserSession* pUser,Cmd::stConfirmSearcherUserCmd* rev);
    void processReqWaitOfficial(UserSession* pUser,Cmd::stReqWaitOfficialUserCmd* rev);
    void processCancelTechSearch(UserSession* pUser,Cmd::stCancelTechSearchUserCmd* rev);
    void processReqDailyEmperorMoney(UserSession* pUser,Cmd::stReqDailyEmperorMoneyCmd* rev);
    void processSetDiplomat(UserSession* pUser,Cmd::stAppointDiplomatCmd* rev);
    void processSetCatcher(UserSession* pUser,Cmd::stAppointCatcherCmd* rev);
    void processCancelDiplomat(UserSession* pUser,Cmd::stCancelDiplomatCmd* rev);
    void processCancelCatcher(UserSession* pUser,Cmd::stCancelCatcherCmd* rev);
    
    CCountry* addNewCountry(DWORD country);

    CCountry* find(DWORD country,DWORD unionid);
    CCountry* find(DWORD country);
    CCountry* findByDare(DWORD country,bool findDare = true);

    struct countryCallback
    {
      virtual void exec(CCountry *)=0;
      virtual ~countryCallback(){};
    };
    void execEveryCountry(countryCallback &);//�������޹�������
    void userOnline(UserSession * pUser);
    void refreshTax();
    void refreshTech(SessionTask* scene,DWORD dwCounryID);

    void broadcastTech(DWORD dwCountryID);

    bool isKing(UserSession* pUser);
    bool isEmperor(UserSession* pUser);
    bool isOfficial(UserSession* pUser);

    void refreshGeneral(DWORD country);
  private:
    void clearForbid();
    void clearDiplomat();
    void clearCatcher();
    DWORD clearForbidTime;
    CCountryM();
    std::vector<CCountry*> countries;
    bool isBeging;
      
    zRWLock rwlock;
};

using namespace UnionDef;

class CUnionM:public zEntryManager<zEntryID,zEntryName>,
        public Singleton<CUnionM>
{
  friend class SingletonFactory<CUnionM>;
  private:
    /// ��Ա����
    std::map<std::string,CUnionMember*> memberIndex;

    /// ���Ͷ���
    typedef std::map<std::string,CUnionMember*>::value_type memberIndexValueType;

    /// ��д��
    zRWLock rwlock;

    CUnion *  createUnionAndAddMaster(const stUnionInfo&);
    bool     initAddUnionMember(const DWORD &,const stUnionMemberInfo &,bool notify = false);
    bool    createUnionDBRecord(stUnionInfo &info);
    void    removeEntryByName(const char * name);
    CUnion*    createUnionByDBRecord(const stUnionInfo & info);
    inline std::map<std::string,CUnionMember *>::iterator  findMemberIndex(const char *pName);
    CUnionM();

  public:
    ~CUnionM();
    template <class YourEntry>
    bool execEveryOne(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
    }

    template <class YourEntry>
    void removeOne_if(removeEntry_Pred<YourEntry> &pred)
    {
      rwlock.wrlock();
      removeEntry_if<>(pred);
      rwlock.unlock();
    }

    CUnion * getUnionByName( const char * pName);
    CUnion* getUnionByID(DWORD dwUnionID);

    bool init();
    static void destroyMe();

    void fireUnionMember(UserSession*,const char *);

    /**
      * \brief ɾ������Ա
      *
      * ����ý�ɫ���ǰ��᳤�����߳���������ǣ���������������
      *
      * \param dwUserID ��ɫID
      * \param find     �Ƿ�Ϊ����
      *
      * \return ����ý�ɫ�����κΰ���У��򷵻�2
      *         ����ý�ɫ�ǰ������򷵻�0
      *         ����ý�ɫ�ǰ��ڣ���ɾ���ɹ����򷵻�1
      *
      */
    int  fireUnionMember(DWORD dwUserID,bool find);

    bool addNewMemberToUnion(const DWORD dwUnionID,const stUnionMemberInfo&);
    void addNewSeptToUnion(const DWORD dwUnionID,const DWORD dwSeptID);
    
    void userOnline(UserSession *);
    void userOffline(const UserSession *);
    void createNewUnion(Cmd::Session::t_addUnion_SceneSession *data);
    bool addMemberIndex(const char *,CUnionMember *);
    bool removeMemberIndex(const char *);
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
    void processUnionSortMessage(UserSession* pUser,const Cmd::stReqUnionSort* ptCmd);
    void processAppointCityCatcherMessage(UserSession* pUser,const Cmd::stAppointCityCatcherCmd* ptCmd);
    void processCancelCityCatcherMessage(UserSession* pUser,const Cmd::stCancelCityCatcherCmd* ptCmd);

    bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    bool processSceneUnionMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    
    bool processGateMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);
    void delUnion(const DWORD dwUnionID);
    void processMemberLeave(const DWORD dwUnionID,const DWORD dwCharID);
    void processMemberPower(UserSession* pUser,const Cmd::stUnionMemberPower* ptCmd);
    bool havePowerByName(const char* name,const int power);

    void sendUnionNotify(const DWORD unionID,const char* message,...);

    void sendUnionChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *,const DWORD cmdLen);
    void sendUnionPrivateChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *,const DWORD cmdLen);
    void sendVoteListToUser(const UserSession *pUser);
    void sendListToUser(const UserSession *pUser);
    void sendDareListToUser(const UserSession* pUser);
    
    void userVote(const UserSession *pUser,const char *pName);
    void userAboutVote(const UserSession *pUser,const char *pName);
    void addVoteMemberToUnion(const UserSession *pUser,const char *pName);
    void setUnionNote(UserSession *pUser,Cmd::stNoteUnionCmd *pCmd);


    /**
     * \brief ���ı���
     *
     * \param pUser ���������(Ŀǰֻ���ǰ���)
     *
     * \param pCmd ��������
     *
     * \return 
     */
    void change_aliasname(UserSession *pUser,Cmd::stChangeUnionMemberAliasName* pCmd);
    void delAllUnion(DWORD id);


    /**
     * \brief ʱ��ص�����
     */
    void timer();
    char * getUnionNameByUserName(char *Name);
    DWORD getCountryIDByUserName(char *Name);
};

class CUnion:public zEntryManager<zEntryID,zEntryName>,public zEntry
{
  private:
    /// ��ἶ��
    WORD                  level;

    /// ��ᾭ��
    DWORD                 exp;

    /// ����״̬����ִ��д��Ȳ���
    bool          destroy;

    /// �Ƿ񻹴���ͶƱ�ڼ䣬1Ϊ��0Ϊ��
    BYTE          byVote;

  
       /// ��д��
    zRWLock rwlock;

  public:
    /// ���᳤
    CUnionMember          *master;
    /// ����ʱ��
    DWORD          dwCreateTime;
    /// ����
    char    note[255];

    /// �����������ID
    DWORD dwCountryID;

    /// �������
    DWORD dwMana;

    // ��ǰ�ж���
    DWORD dwActionPoint;

    // ����ʽ�������
    DWORD dwMoney;
    DWORD calltimes;

    DWORD getActionPoint();//�õ��ж���
    bool  changeActionPoint(int repute);//�ı��ж���
    
    DWORD getMoney();//�õ�����ʽ�
    bool  changeMoney(int money);//�ı����ʽ�

    void sendUnionDare(UserSession* pUser,const char* fromName,DWORD dwWarID);

    template <class YourEntry>
    bool execEveryOne(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
    }

    template <class YourEntry>
    void removeOne_if(removeEntry_Pred<YourEntry> &pred)
    {
      rwlock.wrlock();
      removeEntry_if<>(pred);
      rwlock.unlock();
    }

    CUnion();
    ~CUnion();
    void      removeEntryByName(const char * name);
    DWORD      getID() {return id;}
    void      init(const stUnionInfo & info);
    CUnionMember *  addUnionMember(const stUnionMemberInfo&);
    CUnionMember *  addUnionMaster(const stUnionInfo& info);
    void      changeMaster(CUnionMember* pMember);
    void      sendUserUnionData(const char *pName);
    void      fireUnionMember(const char *,const char *);
    int      fireUnionMemberDirect(const DWORD dwCharID,const bool checksept=true);
    void       fireSeptFromUnion(const DWORD dwSeptID);

    void      sendCmdToAllMember(const Cmd::stNullUserCmd * ptCmd,const DWORD nCmdLen );
    void       sendCmdToAllMemberScene(Cmd::Session::t_enterWar_SceneSession* ptEnterWarCmd,const DWORD cmdLen);
    void      sendCallCmdToAllMemberScene(Cmd::Session::t_GoTo_Leader_Check_SceneSession* ptEnterWarCmd,const DWORD cmdLen,DWORD expect);
    void      notifyMemberFire(const char *,const char *);
    void                notifyWarResult(const char* msg,...);
    void      writeDatabase();
    void      sendUnionMemberList(UserSession *pUser);
    void      sendUnionInfoToUser(UserSession *pUser);
    bool      deleteMeFromDB();
    void       sendUnionNotify(const char* message,...);

    bool      isMember(DWORD dwUserID);

    void      disbandUnion();
    void      delUnion(const DWORD dwUnionID);
    bool      loadUnionMemberFromDB();
    DWORD      size();
    bool      isVote();
    void      letVoteOver();    
    void      setNote(Cmd::stNoteUnionCmd *pCmd);
  
    void      sendUnionInfoToAll();
    void      sendUnionManaToAll();
    // �ж��ܷ��ɢ���
    bool      isDel();

    // ͳ�Ʊ����м�������
    DWORD       septSize();

    void       changeAllSeptRepute(int repute);
  
    // ˢ�¸ð�����м��徭����ȡ��־  
    void       refreshSeptExp();


    // ��ð������
    DWORD      getMana();
    
    // �������г�Ա���ݵ�����  
    void       update_all_data();

    // ����һ����Ա��ֱ���뿪
    void    fireUnionMemberLeave(DWORD dwUserID);  
};

struct CUnionMember:public zEntry
{
public:

    /// ��Ա����
    char  aliasname[MAX_NAMESIZE+1];
    /// ��ԱȨ��
    BYTE  byPower[2];

    /// ��Ա״̬
    BYTE  byStatus;

    /// ��Աְҵ
    WORD  wdOccupation;

    /// ��Ա��������
    DWORD septid;

    /// �û��ĻỰ����
    //UserSession * user;

    /// ������������ָ��
    CUnion *myUnion;

    /// ����״̬����ִ��д��Ȳ���
    bool  destroy;
    
    /// ������
    zRWLock rwlock;

    /// ���߱�־ö��
    enum {
          Offline,
          Online
    };


    void init(const stUnionMemberInfo&);
    void getMemberBaseInfo(struct Cmd::stUnionRember& info);
    void sendUserUnionPrivateData();
    void sendUnionDare(UserSession* pDareUser,const char* fromName,DWORD dwWarID);
    void sendCmdToMe(const Cmd::stNullUserCmd *pstrCmd,const DWORD nCmdLen);
    void writeDatabase();
    void insertDatabase();
    void setUnion(CUnion * pUnion);
    void online(const DWORD status = Cmd::UNION_MEMBER_STATUS_ONLINE);
    bool isOnline();
    void offline();
    bool havePower(const int power);
    void setPower(const int power);
    void sendMessageToMe(const char *);
    void fireMe(const bool notify = true,const bool checksept = true);
    bool deleteMeFromDB();
    void sendUserUnionData();
    void change_aliasname(const char* aliasname);
    void update_data();
      
    CUnionMember();
    ~CUnionMember();
    
};

class DBRecord;
class SessionTask;
const int CREATE_ALLY_NEED_MONEY = 200000; // 20��

class CAlly
{
  public:
    CAlly();
    ~CAlly();

    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool deleteMeFromDB();

    bool changeFriendDegree(int degree);
    void refreshAlly(bool isFire = false);
    void refreshAllyToAllUser(bool isFire = false);

    DWORD friendDegree()
    {
      return this->dwFriendDegree;
    }

    DWORD dwCountryID; // ID1
    DWORD dwAllyCountryID; // ID2
    DWORD dwFriendDegree;
    DWORD dwCreateTime;
    DWORD dwLastUpTime; // ÿ���Զ��ۼ��Ѻöȵ����һ�θ���ʱ��

    BYTE  byStatus; // 2�ѽ���,1�ȴ�����

    zRWLock rwlock;
};

class CAllyM : public Singleton<CAllyM>
{
  friend class SingletonFactory<CAllyM>;
  public:
    bool init();
    static void destroyMe();

    void timer();
    void save();

    bool loadAllyFromDB();
    bool processUserMessage(UserSession* pUser,const Cmd::stNullUserCmd* pNullCmd,const DWORD cmdLen);
                bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);

    void processReqCountryAlly(UserSession* pUser,Cmd::stReqCountryAlly* rev);
    void processCancelCountryAlly(UserSession* pUser,Cmd::stCancelCountryAlly* rev);
    void processReqCountryAllyInfo(UserSession* pUser,Cmd::stReqCountryAllyInfo* rev);

    void refreshAlly(SessionTask* scene);
    void userOnline(UserSession* pUser);

    CAlly*  findAlly(DWORD dwCountryID1,DWORD dwCountryID2);
    CAlly*  findAlly(DWORD dwCountryID1);
    bool   addNewAlly(DWORD dwCountryID1,DWORD dwCountryID2);
    void   addReadyAlly(DWORD dwCountryID1,DWORD dwCountryID2);
    bool    fireAlly(DWORD dwCountryID1,DWORD dwCountryID2);
    void    removeAlly(DWORD dwCountryID1,DWORD dwCountryID2);
  private:
    std::vector<CAlly*> allies;
    zRWLock rwlock;
};

using namespace School;

/**
 * \brief ������Ա�б����
 *
 */
class CSchoolMemberListCallback
{
private:
  /**
   * \brief ��Ӱ��ĳ�Ա�б�
   */
  std::list<CSchoolMember * > memberList;
public:

  /**
   * \brief �����Ч�ڵ��ϵ
   */
  void clearInValidNodeRelation();

  /**
   * \brief ����
   * \param member ��ǰ�ڵ�
   */
  void exec(CSchoolMember *member);

  /**
   * \brief ����֪ͨ����Ӱ��ĳ�Ա
   */
  void sendNotifyToMember();
};

/**
 * \brief ���Ա����������Ա��Ϣ�Ļص�
 *
 */
class CSendSchoolCallback
{
private:
  ///��Ա�б�
  std::list<struct Cmd::stSchoolMember> memberList;
public:
  void exec(CSchoolMember *member,const BYTE tag);
  void sendListToMember(CSchoolMember *member);
};

/**
 * \brief ���ɹ�����
 *
 */
class CSchoolM : public zEntryManager<zEntryID,zEntryName>
{
private:
    /// ��Ա������
    std::map<std::string,CSchoolMember*> memberIndex;

    /// ���Ͷ���
    typedef std::map<std::string,CSchoolMember*>::value_type memberIndexValueType;

    /// Ψһʵ��
    static CSchoolM * sm;
    ///��д��
    zRWLock rwlock;
    inline std::map<std::string,CSchoolMember *>::iterator  findMemberIndex(const char *pName);

public:
    
    /**
     * \brief ��������
     */
    ~CSchoolM();
    
    /**
     * \brief ��ʼ��ʦ�����ɹ���������
     * \return true Ϊ�ɹ�,falseΪʧ��
     */
    bool init();

    template <class YourEntry>
    bool execEveryOne(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
    }

    /**
     * \brief ��ȡΨһ����ʵ��
     * \return Ψһ����ʵ��
     */
    static CSchoolM &getMe();

    /**
     * \brief �ݻ�ʦ�����ɹ���������,���ƺ���
     */
    static void destroyMe();

    /**
     * \brief ��ɫ����֪ͨ,�������ж�Ӧ�Ľڵ�������ߴ������UserSession���Ҷ������������֪ͨ
     * \param pUser ���߽�ɫ�ĻỰ����
     */
    void userOnline(UserSession *pUser);

    /**
     * \brief ��ɫ����֪ͨ,�������ж�Ӧ�Ľڵ�������ߴ���,���Ҷ������������֪ͨ
     * \param pUser ���ߵĽ�ɫ
     */
    void userOffline(UserSession * pUser);

    /**
     * \brief �������������ָ����ɫ������
     * \param name ָ����ɫ������
     * \param member ��ɫ�Ľڵ����
     * \return ʧ�ܷ���false,�ɹ�����true
     */
    bool addMemberIndex(const char *name,CSchoolMember *member);

    /**
     * \brief ����������ɾ��ָ���Ľ�ɫ������
     * \param name ָ����ɫ������
     * \return ʧ�ܷ���false,�ɹ�����true
     */
    bool removeMemberIndex(const char *name);

    /**
     * \brief ����ͻ����͹�������Ϣ��Ҳ�п���������������ת�������Ŀͻ�����Ϣ��
     * \param pUser ������Ϣ�Ľ�ɫ
     * \param pNullCmd ��Ϣ�ṹ
     * \param cmdLen ��Ϣ����
     * \return ʧ�ܷ���-1,û����Ʒ����������0,����������1,�ɹ�������д�������Ʒ����2
     */
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);

    /**
     * \brief �������������͹�������Ϣ
     * \param pNullCmd ��Ϣ�ṹ
     * \param cmdLen ��Ϣ����
     * \return ʧ�ܷ���-1,û����Ʒ����������0,����������1,�ɹ�������д�������Ʒ����2
     */
    bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);

    /**
     * \brief �����������������Ϣת��
     * \param pUser ������Ϣ������
     * \param rev ��Ϣ�ṹ��
     * \param cmdLen ��Ϣ����
     */
    void sendSchoolChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen);

    /**
     * \brief �����������������Ϣת��
     * \param pUser ������Ϣ������
     * \param rev ��Ϣ�ṹ��
     * \param cmdLen ��Ϣ����
     */
    void sendSchoolPrivateChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen);

    /**
     * \brief �ж��û���ͽ���Ƿ���,
     * \param master �����Ľ�ɫ����
     * \param schoolName ��������,������˵�ͽ��δ���Ҵ����������ɵ���ô��������ͨ�����ﷵ��
     * \return ����true��ʾmaster��ͽ��û����,����false��ʾmaster��ͽ���Ѿ���������
     */
    bool getUserPrenticeInfo(const char *master,char *schoolName);

    /**
     * \brief �ж�ĳ�����Ƿ�Ϊ�ɾ���,��˼����û��ʦ����ͽ�ܵĹ�ϵ,�������ڴ˹�������
     * \param name �����Ľ�ɫ������
     * \return trueΪ�ɾ���,false Ϊ�Ѿ���ʦ��ͽ�ܹ�ϵ
     */
    bool isClean(const char *name);

    /**
     * \brief Ϊ master ����һ��ͽ�� prentice,
     *
     * �ں����л�������˵�������о���Ĳ���,������ɹ������Ӧ��ɫ������Ϣ
     *
     * \param master ʦ��
     * \param prentice ͽ��
     */
    void addMember(UserSession *master,UserSession *prentice);

    /**
     * \brief master ����һ��ͽ�� prentice,
     *
     * �ں����л�������˵�������о���Ĳ���,������ɹ������Ӧ��ɫ������Ϣ
     *
     * \param master ʦ��
     * \param prentice ͽ��
     */
    void frieMember(UserSession *master,const char *prentice);

    /**
     * \brief ��ʦ�Ŵӿ���һ����ɫ
     *
     * \param roleName ��ɫ����
     * \param find �Ƿ�ֻ�ǽ����ж�
     *
     * \return ����ý�ɫ�����κ�ʦ����,�򷵻�2
     *         ����ý�ɫ���峤,�򷵻�0
     *         ����ý�ɫ����Ա,��ɾ���ɹ�,�򷵻�1
     *
     */
    int fireSchoolMember(const char* roleName,bool find);

    /**
     * \brief ��ʼ����ʱ��������ݿ���load�ļ�¼����һ�����ɹ�����
     * \param info ���ɵ���Ϣ�ṹ
     * \return �ɹ�����true,ʧ�ܷ���false
     */
    bool createSchoolFromDB(const stSchoolInfo &info);

    /**
     * \brief �����ݿ��м������ɵ���Ϣ�������ɹ�����
     * \return trueΪ�ɹ�,falseΪʧ��
     */
    bool loadSchoolFromDB();

    /**
     * \brief �����ݿ��м����������ɵĳ�Ա
     * \return trueΪ�ɹ�,falseΪʧ��
     */
    bool loadSchoolMemberFromDB();

    /**
     * \brief ���ָ���Ľ�ɫ�Ƿ�߱��������ɵ�����
     * \param pUser �����Ľ�ɫ
     * \return trueΪ�߱�,falseΪ���߱�
     */
    bool checkSchoolCreateCondition(const UserSession *pUser);

    /**
     * \brief ����һ���µ�����
     * \param userName ʦ�������
     * \param schoolName ���ɵ�����
     * \return true Ϊ�����ɹ�,false Ϊ����ʧ��
     */
    bool createNewSchool(const char *userName,const char *schoolName);

    /**
     * \brief ��Ӧ���湦�����ù�����߶�ȡ����
     * \param pUser ������
     * \param rev ������Ϣ
     */
    void processBulletin(const UserSession *pUser,const Cmd::stSchoolBulletinCmd *rev);

    /**
     * \brief ��ȡ������ָ���Ľڵ����
     * \param pName ����
     * \return ��ȡָ���Ľڵ����,��������ڸ����ƵĶ����򷵻�NULL
     */
    CSchoolMember *getMember(const char *pName);

    /**
     * \brief �����Ա�˳����ɻ���ʦ��
     * \param pUser �˳��ߵ�UserSession����
     */
    void processLeaveGroup(UserSession *pUser);

    /**
     * \brief �����Ա�˳����ɻ���ʦ��
     * \param roleName ��ɫ����
     */
    void processLeaveGroupDirect(const char* roleName);

    /**
     * \brief ��ɢ����,�����������pName���쵼�����ɽ�����ɢ,���е���Ա��ϵ�ظ���ԭʼ��ʦ�Ź�ϵ
     * \param pName ��ɢ�����ߵ�����
     * \return true ��ɢ�ɹ� false ��ɢʧ��
     */
    bool destroySchool(const char *pName);

    /**
     * \brief ����ID��ö�Ӧ�Ĺ�����
     * \param id ������ID
     * \return ����ɹ����ع���������,ʧ�ܷ���NULL
     */
    CSchool *getSchool(DWORD id);

    /**
     * \brief ����ָ���û��ļ���
     * \param pName �û�����
     * \param level �¼���
     */
    void setUserLevel(const char *pName,const WORD &level);

    CSchool * getSchoolByName( const char * name);
    
};

class CSchool : public zEntryManager<zEntryID,zEntryName>,public zEntry
{
private:
    /**
     * \ʦ���ID
     */
    DWORD dwMasterSerialID;

    /**
     * \����
     */
    std::string bulletin;

    /**
     * \brief ������Ч��־,һ���ж��������Ƿ񻹴������ݿ���
     */
    bool destroy;

    /**
     * \brief ��������д��
     */
    zRWLock rwlock;

public:
/*
    bool getUniqeID(DWORD &tempid){return true;};
    void putUniqeID(const DWORD &tempid){};
*/

    /**
     * \brief ����ģ�嶨��
     */
    template <class YourEntry>
                bool execEveryOne(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
         }
                
         template <class YourEntry>
         void removeOne_if(removeEntry_Pred<YourEntry> &pred)
         {
                rwlock.wrlock();
                removeEntry_if<>(pred);
                rwlock.unlock();
         }


    /**
     * \brief ���캯��
     */
    CSchool();

    /**
     * \brief ��������
     */
    ~CSchool();



    /**
     * \brief ��ȡ��������
     * \return ��������
     */
    char *getSchoolName();

    /**
     * \brief ������������δ����
     * \param pName ��������
     */
    void setSchoolName(const char *pName);

    /**
     * \brief ����ʦ��Ľڵ�ID
     * \param id ʦ��Ľڵ�ID
     */
    void setMasterSerialID(const DWORD &id);

    /**
     * \brief ���һ��ʦ�ŵĸ��ڵ�
     * \param master ��ɫ����
     * \return �¼ӵĽڵ����,ʧ�ܷ���NULL
     */
    CSchoolMember *  addTeacher(UserSession *master);

    /**
     * \brief ��ӹ�����������ָ���Ľڵ�,���������������
     * \param member ָ���Ľڵ�
     */
    bool addMember(CSchoolMember *member);

    /**
     * \brief �ӹ�������ɾ��ָ���Ľڵ�,���������������ȥ��
     * \param member ָ���Ľڵ�
     */
    void removeMember(CSchoolMember *member);
  
    /**
     * \brief ����������ʼ����ʦ�Ź�ϵ������
     * \return ����NULL��ʾ���ͽ��ʧ��,���򷵻�ͽ�ܵĽڵ����
     */
    void initToNoneSchool();

    /**
     * \brief ��ʼ��������,������Ϣ�ṹ,�����ݿ��ʼ����ʱ����
     * \param info ��������Ϣ�ṹ
     */
    void initSchool(const stSchoolInfo &info);

    /**
     * \brief �����ݿ���ر��������ڵ����нڵ���Ϣ
     * \return ����true ��ʾ���سɹ�,����false��ʾ����ʧ��
     */
    bool loadSchoolMemberFromDB();

    /**
     * \brief �������ɵ���Ϣ�����ݿ�
     * \return ����true ��ʾ���سɹ�,����false��ʾ����ʧ��
     */
    bool updateSchoolInDB();

    /**
     * \brief ����һ���ڵ�,���ݿ��ʼ����ʱ����ñ�����,ͨ�����ñ�������ʱ��ڵ����ݶ��������,�ڵ����к���С����
     * \param info �ڵ���Ϣ
     */
    bool addNode(const stSchoolMemberInfo &info);

    /**
     * \brief �����������School��Ľ�ɫ
     * \param pNullCmd ��Ϣ�ṹ
     * \param cmdLen ��Ϣ����
     */
    void sendCmdToSchool(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);

    /**
     * \brief �������ɼ�¼�����ݿ���
     * \return true ����ɹ� false ����ʧ��
     */
    bool insertSchoolToDB();

    /**
     * \brief �����ڵ�����������ݿ���ɾ��
     * \return true ɾ���ɹ� false ɾ��ʧ��
     */
    bool deleteSchoolFromDB();

    /**
     * \brief userName������һ���µİ��pSchool�������е�����userName��ͽ��ͽ��鵽��School������pSchool��,�������ֻ���������ɹ�����
     * \param userName �µ�ʦ�������
     * \param pSchool �µĹ���������ָ��
     * \return true �ɹ� false ʧ��
     */
    bool moveMemberToNewSchool(const char *userName,CSchool *pSchool);


    /**
     * \brief �ж�һ����ɫ�ǲ��Ǹ�ʦ�ų�Ա
     * 
     * \param memberName �������
     *
     * \return true ��ʦ�ų�Ա,FALSE����ʦ�ų�Ա
     */
    bool  isMember(const char* memberName);
    
    /**
     * \brief ������ʦ�ų�Ա����ת��������������
     *
     * \param ptEnterWarCmd ��ת��������
     * \param cmdLen �����
     *
     */
    void sendCmdToAllMemberScene(Cmd::Session::t_enterWar_SceneSession* ptEnterWarCmd,const DWORD cmdLen);

    /**
     * \brief ֱ�����һ���ڵ�,�ýڵ��Ѿ�������������֮����
     * \param member ����ӵĽڵ�
     */
    void directAdddMember(CSchoolMember *member);

    /**
     * \brief ֱ��ɾ��һ���ڵ�,��������������ɾ���ýڵ�
     * \param member ����ӵĽڵ�
     */
    void directRemoveMember(CSchoolMember *member);

    /**
     * \brief ֱ��ɾ��һ���ڵ�,��������������ɾ���ýڵ�
     * \return �����������ɵĽڵ����,�쳣����NULL
     */
    CSchoolMember * getMasterNode();

    /**
     * \brief ���ù���
     * \param buf ��������
     */
    void setBulletin(const char *buf);

    /**
     * \brief ��ù���
     * \return �������ݻ���NULL
     */
    const char * getBulletin();

    /**
     * \brief ���ָ���ĳ�Ա
     * \param pName ָ��������
     * \return �ҵ��ĳ�Ա�������NULL
     */
    CSchoolMember *getMember(const char *pName);

    /**
     * \brief ����ָ����Ա�Ĺ�ϵ�Ͼ���������������ȫ�˳�,��ʦ�����ǶϾ���ʦ���Ĺ�ϵ
     * \param member Ҫ�˳���Ա�Ľڵ����
     * \param deleteTeacher�Ƿ�ɾ��ʦ���ڵ�,�˱�־���ڷ�ֹ�ں�����ɾ�����ö���,�˱�־�������в�������,��˼�ǲ��������ɳ�Ա�����е��ô˺���
     * \return true Ϊ�ɹ� falseΪʧ��
     */
    bool processLeaveSchool(CSchoolMember * member,bool deleteTeacher = true);

    /**
     * \brief �������ɽ�ɢ���г�Ա��ϵת�Ƶ�ʦ�Ź���������
     * \return true Ϊ�ɹ� falseΪʧ��
     */
    bool moveMemberToTeacherGroup();

    void notifyWarResult(const char* msg,...);

    DWORD getID(){return id;}
};

class CSchoolMember : public zEntry
{
private:

    /**
     * \brief ������Ч��־,һ���ж��������Ƿ񻹴������ݿ���
     */
    bool destroy;

    /**
     * \brief ��ǰ��ɫ��UserSession����
     */
    DWORD dwCharID;

    /**
     * \brief ��ǰ��ɫ��ְҵ
     */
    DWORD wdOccupation;

    /**
     * \brief ��ǰ��ɫ��UserSession����
     */
    UserSession    *user;

    /**
     * \brief ʦ���ڵ�
     */
    CSchoolMember  *preLevelNode;

    /**
     * \brief ����
     */
    struct ltword
    {
      bool operator()(const DWORD s1,const DWORD s2) const
      {
        return s1<s2;
      }
    };
    /**
     * \brief ͽ���б�
     */
    std::map<DWORD,CSchoolMember*,ltword> prenticeList;

    /// ���Ͷ���
    typedef std::map<DWORD,CSchoolMember*,ltword>::value_type prenticeListValueType;

    /**
     * \brief ���ڵ����ɹ������������ɵĹ��Ϊһ�����ɣ�
     */
    CSchool      *school;

    /**
     * \brief ʦ���ڵ��ID
     */
    DWORD dwMasterID;

    /**
     * \brief ǰһ���ڵ�����к�
     */
    DWORD dwPreSerialID;

    /**
     * \brief ��ɫ��ǰ����
     */
    DWORD wdLevel;

    /**
     * \brief ��ʦʱ��
     */
    DWORD dwJoinTime;

    /**
     * \brief ��ʦ������Ѻö�
     */
    WORD  wdDegree;

    /**
     * \brief ������ʱ��
     */
    DWORD dwLastTime;

    /**
     * \brief ��������ID���������Ϊ0
     */
    DWORD dwSchoolID;

    /**
     * \brief �ڵ���Ч���,1Ϊ��Ч,0Ϊ��Ч,��Ч�Ľڵ�ֻ����������֮������������֮��
     */
    BYTE  byTag;
    
  
    /**
     * \brief ���ڵ�Ķ�д��
     */
    zRWLock rwlock;

public:
    /**
     * \brief �����ڵ����Ϣ�������ݿ�,�ڵ������ʱ����
     * \return trueΪ�ɹ�,false Ϊʧ��
     */
    bool insertRecord(); // ���Լ��������ݿ�

    /**
     * \brief �������ݿ��еĽڵ���Ϣ
     * \param locked �Ƿ��Ѿ����� Ĭ��Ϊ��
     * \return trueΪ�ɹ�,false Ϊʧ��
     */
    bool updateRecord(bool locked = false); // ���Լ��������ݿ�

    /**
     * \brief ���Լ������ݿ��¼ɾ��
     * \return trueΪ�ɹ�,false Ϊʧ��
     */
    bool deleteRecord();

    /**
     * \brief ���캯��,�Խڵ���в��ֳ�ʼ��
     * \param pSchool ���ڵ����ڵĹ���������
     * \param pUser �ڵ��ӵ����
     */
    CSchoolMember(CSchool *pSchool,UserSession *pUser);

    /**
     * \brief ��������
     */
    ~CSchoolMember();

    /**
     * \brief ��ȡ�ڵ��Ӧ�Ľ�ɫID
     * \return ��ɫID
     */
    DWORD getCharID();

    /**
     * \brief ��ȡ�ڵ����к�
     * \return �ڵ����к�
     */
    DWORD getSerialID();

    /**
     * \brief ��ȡ�ڵ��������ɵ�ID
     * \return ����ID
     */
    DWORD getSchoolID();

    /**
     * \brief ��ȡ�Ѻö�
     * \return �Ѻö�
     */
    WORD getDegree();

    /**
     * \brief �����Ѻö�
     * \param degree �Ѻö�
     */
    void setDegree(const WORD &degree);

    /**
     * \brief ����������ʱ��
     * \param lasttime ������ʱ��
     */
    void setLastTime(const DWORD &lasttime);

    /**
     * \brief ��ȡ���ڵ����������
     * \return ��������
     */
    char *getSchoolName();

    /**
     * \brief ��õ�ǰ�ڵ��ͽ�ܼ�������Ч�ڵ�����
     * \return ��ǰ�ڵ��ͽ�ܼ���
     */
    BYTE getPrenticeCount();

    /**
     * \brief ��ð�ʦʱ��
     * \return ��ʦʱ��
     */
    DWORD getJoinTime();

    /**
     * \brief ���������ʱ��
     * \return ������ʱ��
     */
    DWORD getLastTime();

    /**
     * \brief ��ý�ɫ����
     * \return ��ɫ����
     */
    WORD getLevel();
    
    /**
     * \brief Ϊ���ڵ����һ��ͽ��
     * \param pUser ͽ�ܵ�UserSession����
     * \return ����NULL��ʾ���ͽ��ʧ��,���򷵻�ͽ�ܵĽڵ����
     */
    CSchoolMember *addPrentice(UserSession *pUser);

    /**
     * \brief ���һ�������ʦ���ڵ�,ֻ��������Щ��δ���չ�ͽ�ܵ��˵�һ������ͽ��ʱ�ڹ������г�ʼ���Լ�
     */
    void initRootMember();

    /**
     * \brief ��ͽ�ܽڵ���г�ʼ��
     * \param master ʦ���ڵ�Ľڵ����
     */
    void initGeneralMember(CSchoolMember * master);

    /**
     * \brief ����Ч�ڵ��ʼ������Ч�ڵ�
     * \param master ʦ���ڵ�Ľڵ����
     */
    void initInValidNode(CSchoolMember * master);
    /**
     * \brief �жϽڵ��Ƿ񻹴��ڹ������ϼ��ڵ�����¼��ڵ�
     * \return trueΪ���ڵ��Ѿ��������ڵ�Ͼ�һ�й�ϵ,false Ϊ���ڵ��������ڵ㻹���ڹ�ϵ
     */
    bool isClean();

    /**
     * \brief ��ñ��ڵ����ڵĹ���������
     * \return �ɹ����ع���������,ʧ�ܷ���NULL
     */
    CSchool * getSchool();

    /**
     * \brief ���Լ���������ɾ����,��ɾ����Ӧ�����ݿ��¼
     */
    void deleteMe();

    /**
     * \brief ���ߴ���
     */
    void online(UserSession *pUser);

    /**
     * \brief ���ߴ���
     */
    void offline();


    /**
     * \brief �ҵ���λ��member֮ǰ��ʦ��
     * \param member �����Ľڵ�
     * \param name ʦ�ֵ�����
     */
    void getMyBigBrother(CSchoolMember *member,char *name);

    /**
     * \brief ֪ͨ�ܱ��Ա�ҵļ��벢���ͳ�ʼ����Ϣ���Լ�
     */
    void notifyNewMemberAdd();

    /**
     * \brief ���͸��˵�ʦͽ������Ϣ���ͻ��˽��г�ʼ��
     * \param callback ������Ϣ�Ļص�����,�������������Ȼ��������Ϣ�ʺ϶����ɶ����˹㲥������ϢĬ��Ϊ��
     */
    void sendInfomationToMe(CSendSchoolCallback *callback = NULL);

    /**
     * \brief ͽ��������ʦ�����ֵܵ���Ϣ
     * \param count ������,�ۼƵ�ǰ�Ѿ��ж�����Ϣ������buf
     * \param point ��Ϣ���buf��ָ��
     * \param me ������Ϣ�Ľ�ɫ�Ľڵ����
     * \return trueΪ�ɾ���,false Ϊ�Ѿ���ʦ��ͽ�ܹ�ϵ
     */
    void prenticeRequestMemberInfo(DWORD &count,Cmd::stTeacherMember *point,CSchoolMember *me);

    /**
     * \brief �������ݿ��¼��ʼ���ڵ�
     * \param info ���ݿ��¼��Ϣ�ṹ
     */
    void initByDBRecord(const stSchoolMemberInfo &info);

    /**
     * \brief �жϽڵ��Ƿ����ʦ��,����info���������ж�
     * \return trueΪ��ʦ��,false Ϊû��ʦ��
     */
    bool haveTeacher();

    /**
     * \brief �жϽڵ��Ƿ����ǰһ�ڵ�,��������һ���˿���û��ʦ�����ǻ���ǰһ�ڵ�
     * \return trueΪ�нڵ�,false ��ǰ�ڵ�Ϊ���ڵ㣨ʦ��
     */
    bool havePreNode();

    /**
     * \brief ���һ����һ���ڵ�,������֪ͨ����,�������ݿ��ʼ����ʱ��
     * \param member ��һ���ڵ�
     */
    bool addNextLevelNode(CSchoolMember *member);

    /**
     * \brief ���ñ��ڵ���ϲ�ڵ�
     * \param pPreNode �ϲ�ڵ�
     */
    void setPreLevelNode(CSchoolMember *pPreNode);

    /**
     * \brief ����ָ�����ֵ�ͽ��
     * \param PrenticeName �������˵�����
     * \return true Ϊ�����ɹ�,falseΪû�д���
     */
    bool firePrentice(const char *PrenticeName);

    /**
      * \brief ��������ͽ��
      *
      */
    bool fireAllPrentice();

    /**
     * \brief �˶Ա��ڵ��ǲ������ƶ�Ӧ�Ľڵ�
     * \param pName ָ��������
     * \return true ����ڵ����Ҫ�ҵĽڵ�,false �ڵ㲻���������
     */
    bool isMe(const char *pName);

    /**
     * \brief �ڵ��Ӧ��ɫ�Ƿ�����
     * \return true ��ɫ����,false ��ɫ������
     */
    bool isOnline();

    /**
     * \brief �жϽڵ��Ƿ�����Ч��,�ж�����Ϊtag�Ƿ�Ϊ1
     * \return true ��Ч��,false ��Ч��
     */
    bool isValid();

    /**
     * \brief ��ýڵ������еĶ�
     * \param layer ��
     */
    void getMyLayer(DWORD &layer);

    /**
     * \brief ֪ͨ�ýڵ㼰������ͽ��,�����Լ���ʦ���б�
     */
    void notifyTeacherGroup();

    /**
     * \brief ��������ڵ��Ӧ�Ľ�ɫ
     * \param pNullCmd ��Ϣ�ṹ
     * \param cmdLen ��Ϣ����
     */
       void sendCmdToMe(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);

    /**
     * \brief ����������Ϣ���ڵ��Ӧ�Ķ���
     * \param type ϵͳ��Ϣ������ 
     * \param pattern ��Ϣ
     * \param ... ����
     */
    void sendSysChat(int type,const char *pattern,...);

    /**
     * \brief ���ʦͽ��ϵ,preLevelNodeָ���ÿ�,ʦ����Ϣ��0
     */
    void clearTeacherRelation();

    /**
     * \brief �����˵�ͽ��ͽ�����Ŀ�ͼ����Ƿ�ﵽ�������ɵ�����
     * \return true Ϊ�������� false Ϊ����������
     */
    bool checkSchoolCreateCondition();

    /**
     * \brief ���ڵ��Լ���ͽ�ܵ������Ƿ�ﵽʦ���������ɵ�Ҫ��
     * \return true Ϊ�������� false Ϊ����������
     */
    bool checkMeAndPrenticeNumberAndLevel();

    /**
     * \brief ���ڵ��Լ��������е�ͽ�ܶ�Ų��������pSchool֮��
     * \param pSchool �ڵ��ƶ���Ŀ�������
     * \param memberSet ��Ա����ص�����,�������Ҫ�Ա��ƶ���Ա���д����������ΪNULL,Ĭ��ΪNULL
     */
    void moveAllToSchool(CSchool *pSchool,CSchoolMemberListCallback *memberSet = NULL);

    /**
     * \brief ֪ͨ����ϵ���еĽڵ����·��ͳ�ʼ����Ϣ���ͻ���
     * \param callback ������Ϣ�ص�����,�����������ɵ���֯�ṹ��ʼ����Ϣ���ͻ���
     */
    void notifyAllReSendInitData(CSendSchoolCallback *callback);

    /**
     * \brief ������ɵ����г�Ա�б�����ģ�
     * \param callback �ص���
     */
    void getSchoolTree(CSendSchoolCallback &callback);

    /**
     * \brief ������ɵ����г�Ա�б�����ģ�
     * \param level ��ǰ��
     * \param tgLevel �˴ε��õ�Ŀ���
     * \param condition ���ױ�־,����Ѿ��������ĵײ���ô�ͷ��� false
     * \param callback �ص�����
     * \param tag ���л����־ �μ�enum SCHOOL_LAYER
     */
    void schoolTreeCallback(DWORD level,DWORD tgLevel,bool &condition,CSendSchoolCallback &callback,BYTE &tag);

    /**
     * \brief ���������ݷ��͸��ڵ��Ӧ�Ľ�ɫ
     */
    void sendBulletinToMe();

    /**
     * \brief ���ñ����ɵĹ�������,��Ȼ�ڵ������ʦ�����
     * \param buf ��������
     */
    void setBulletin(const char *buf);

    /**
     * \brief ��ڵ㼰��ͽ�ܷ�����Ϣ
     * \param pNullCmd ��Ϣ�ṹ
     * \param cmdLen ��Ϣ����
     * \param exceptMe �Ƿ�������ڵ�,Ĭ��Ϊ����
     */
    void sendCmdToTeacherGroup(const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen,bool exceptMe = false);

    /**
     * \brief ������֪ͨ�ҵ�����״̬
     * \param onlineStatus �ڵ��Ӧ��ɫ������״̬
     */
    void sendOnlineStatusMessage(BYTE onlineStatus);

    /**
     * \brief ����ϼ��ڵ����ָ��
     * \return �ϼ��ڵ�Ķ���,�п���ΪNULL
     */
    CSchoolMember * getPreLevelNode();

    /**
     * \brief ���ʦ���ڵ�,�ϼ��ڵ㲻һ����ʦ���ڵ�,����֮����ܲ������ڹ�ϵ���Ի��ʦ���ڵ㲻��ʹ��getPreLevelNode
     * \return �����ֱ��ʦ������ʦ���ڵ�Ķ���,����ΪNULL
     */
    CSchoolMember * getTeacher();

    /**
     * \brief ���ڵ����ó���Ч��.
     */
    void setInValid();

    /**
     * \brief �Ͼ���ʦ��֮��Ĺ�ϵ
     */
    void clearMaster();

    /**
     * \brief �����˳����ɵ���Ϣ���ͻ���
     */
    void sendDestroyNotifyToMe();

    /**
     * \brief ֱ���ڱ��ڵ���²�ڵ��б���ɾ��ָ���Ľڵ��ϵ
     * \param member ��ϵ��Ҫɾ���Ľڵ����
     */
    void directRemovePrentice(const CSchoolMember *member);

    /**
     * \brief ֪ͨ�������ɵĿͻ���ɾ�����ڵ��Ӧ�Ľ�ɫ
     */
    void notifySchoolMemberRemove();

    /**
     * \brief ��õ�һ����Ч���¼��ڵ����
     * \return �¼���Ч�ڵ����,���û�к��ʵĶ�����ܻ᷵��NULL
     */
    CSchoolMember* getFirstInValideNode();

    /**
     * \brief ������Ч�ڵ������UserSession����
     * \param pUser ��ɫ��Ӧ��UserSession����
     */
    void setUser(UserSession *pUser);

    /**
     * \brief ��ñ��ڵ��ֱ���¼��ڵ�������������Ч��Ч
     * \return ֱ���¼��ڵ���
     */
    BYTE getNextLevelNodeCount();

    /**
     * \brief ������ڵ���ǰһ�ڵ�Ĺ�ϵ����,����ص�ָ��������ÿջ���0
     */
    void clearPreLevelNode();

    /**
     * \brief ������ڵ����һ���ڵ�Ĺ�ϵ����,����һ���ڵ���ص�ָ��������ÿջ���0�����󼶽ڵ��б����
     */
    void clearAllNextLevelRelation();

    /**
     * \brief �����϶��ѺöȽ��б���
     * \param rev �ѺöȽ�����Ϣ��
     */
    void setFriendDegree(const Cmd::Session::t_CountFriendDegree_SceneSession *rev);

    /**
     * \brief ת��������Ϣ
     * \param rev ������Ϣ�ṹ��
     * \param cmdLen ��Ϣ����
     */
    void sendChatMessages(const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen);

    /**
     * \brief ת��������Ϣ
     * \return ��ȡ��ɫ��Ӧ��UserSession ���������ɫ�����߻᷵��NULL
     */
    UserSession *getUser();

    /**
     * \brief ���ü���
     * \param level �µĽ�ɫ�ȼ�
     */
    void setLevel(const WORD level);

    /**
     * \brief ��ȡ��ɫ��ְҵ
     * \param ְҵ
     */
    WORD getOccupation();

    /**
     * \brief ��ѯ��ǰ���Խ����Ľ��
     * \param ���
     */
    DWORD queryBounty();

    /**
     * \brief ����
     */
    bool putBounty();

    /**
     * \brief ��������ϵ����֪ͨ������
     */
    void sendNotifyToScene();

    /**
     * \brief ��Ϊʦ���ۼ��յ��Ľ���,��λ:��
     */
    DWORD  master_total;
    
    /**
     * \brief ��Ϊʦ����ǰ������ȡ�Ľ������,��λ:��
     */
    DWORD  master_balance;
    
    /**
     * \brief ��Ϊͽ���ۼƽ����Ľ��,��λ:��
     */
    DWORD  prentice_total;

    /**
     * \brief ��Ϊͽ���ϴν���ʱ�ĵȼ�
     */
    DWORD  prentice_lastlevel;

};

using namespace SeptDef;

class CSeptM:public zEntryManager<zEntryID,zEntryName>
{
  private:
    /// ��Ա����
    std::map<std::string,CSeptMember*> memberIndex;

    /// ���Ͷ���
    typedef std::map<std::string,CSeptMember*>::value_type memberIndexValueType;

    /// Ψһʵ��ָ��
    static CSeptM * um;

    /// ��д��
    zRWLock rwlock;

    CSept *  createSeptAndAddMaster(const stSeptInfo&);
    bool     initAddSeptMember(const DWORD &,const stSeptMemberInfo &,bool notify = false);
    bool    createSeptDBRecord(stSeptInfo &info);
    void    removeEntryByName(const char * name);
    CSept*    createSeptByDBRecord(const stSeptInfo & info);
    inline std::map<std::string,CSeptMember *>::iterator  findMemberIndex(const char *pName);
    CSeptM();
    ~CSeptM();

  public:
    template <class YourEntry>
    bool execEveryOne(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
    }

    template <class YourEntry>
    void removeOne_if(removeEntry_Pred<YourEntry> &pred)
    {
      rwlock.wrlock();
      removeEntry_if<>(pred);
      rwlock.unlock();
    }

    CSept * getSeptByName( const char * name);
    
    CSept* getSeptByID(DWORD dwSeptID);
    CSeptMember * getMemberByName(const char * name);

    bool init();
    static CSeptM &getMe();
    static void destroyMe();

    void fireSeptMember(UserSession*,const char *);
    void notifyNpcHoldData(DWORD septid);

    /**
      * \brief ɾ�������Ա
      *
      * ����ý�ɫ���Ǽ����峤,���߳���,�����,����,��������
      *
      * \param dwUserID ��ɫID
      * \param find     ΪTRUE��ʾ����,ΪFALSEΪɾ��
      *
      * \return ����ý�ɫ�����κμ�����,�򷵻�2
      *         ����ý�ɫ���峤,�򷵻�0
      *         ����ý�ɫ����Ա,��ɾ���ɹ�,�򷵻�1
      *
      */
    int  fireSeptMember(DWORD dwUserID,bool find);

    bool addNewMemberToSept(const DWORD dwSeptID,const stSeptMemberInfo&);
    void userOnline(UserSession *);
    void userOffline(const UserSession *);
    void createNewSept(Cmd::Session::t_addSept_SceneSession *data);
    bool addMemberIndex(const char *,CSeptMember *);
    bool removeMemberIndex(const char *);
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
    
    void processSeptSortMessage(UserSession* pUser,
        const Cmd::stReqSeptSort* ptCmd);
    
    void processRequestSeptExpMessage(UserSession* pUser,const Cmd::stRequestSeptExpCmd* ptCmd);
    void processRequestSeptNormalExpMessage(UserSession* pUser,const Cmd::stRequestSeptNormalExpCmd* ptCmd);
    
    bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    bool processSceneSeptMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    bool processGateMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);
    void delSept(const DWORD dwSeptID);
    void processMemberLeave(const DWORD dwSeptID,const DWORD dwCharID);
    void sendSeptNotify(const DWORD septID,const char* message,...);
    void sendNpcDareCmdToScene(const DWORD septID,Cmd::Session::t_NpcDare_NotifyScene_SceneSession* ptCmd,DWORD nCmdLen);
    void sendSeptChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen);
    void sendSeptPrivateChatMessages(const UserSession *pUser,const Cmd::stChannelChatUserCmd *rev,const DWORD cmdLen);
    void sendVoteListToUser(const UserSession *pUser);
    void sendListToUser(const UserSession *pUser);
    void sendDareListToUser(const UserSession *pUser);
    void userVote(const UserSession *pUser,const char *pName);
    void userAboutVote(const UserSession *pUser,const char *pName);
    void addVoteMemberToSept(const UserSession *pUser,const char *pName);
    void disributeExp(Cmd::Session::t_distributeSeptExp_SceneSession *cmd);
    void setSeptNote(UserSession *pUser,Cmd::stNoteSeptCmd *pCmd);
    DWORD findUserSept(DWORD dwUserID);
    void delSeptAllMember();

    /**
     * \brief ���ı���
     *
     * \param pUser ���������(Ŀǰֻ���ǰ���)
     *
     * \param pCmd ��������
     *
     * \return 
     */
    void change_aliasname(UserSession *pUser,Cmd::stChangeSeptMemberAliasName* pCmd);
    void changeAllRepute(DWORD countryid,int repute);
    DWORD getRepute(DWORD dwSeptID);//�õ�����ֵ
    void changeRepute(DWORD dwSeptID,int repute);//�ı�����ֵ
    void changeLevel(DWORD dwSeptID,int level);
    char * getSeptNameByUserName(char *Name);
    DWORD getSeptIDByUserName(char *Name);    
};

class CSept:public zEntryManager<zEntryID,zEntryName>,public zEntry
{
  private:
    /// ����״̬����ִ��д��Ȳ���
    bool          destroy;
    
    /// ��д��
    zRWLock rwlock;

  public:
    /// �Ƿ񻹴���ͶƱ�ڼ�,1Ϊ��0Ϊ��
    BYTE          byVote;

    /// ����᳤
    CSeptMember          *master;

    /// ����ʱ��
    DWORD          dwCreateTime;

    /// ����
    char          note[255];

    /// ������������
    DWORD dwCountryID;

    /// �����������
    DWORD dwUnionID;

    /// 
    void clearUnion()
    {
      this->rwlock.wrlock();
      this->dwUnionID = 0;
      this->rwlock.unlock();
    }
    

    /// ����ȼ�
    DWORD dwLevel;

    /// ��������
    DWORD dwRepute;

    /// �������ѽ��
    DWORD dwSpendGold;
    
    /// �����Ƿ���ȡ����
    DWORD dwIsExp;

    /// ����ʹ�ô���
    DWORD calltimes;
    /// ����ʹ��ʱ��
    DWORD calldaytime;
    /// �ϴ���ȡ��ͨ���徭���ʱ��
    DWORD normal_exp_time;

	struct stStepRight
	{
		char RightName[MAX_NAMESIZE];
		DWORD dwRight;
	} RightList[10];
	
    template <class YourEntry>
    bool execEveryOne(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
    }

    template <class YourEntry>
    void removeOne_if(removeEntry_Pred<YourEntry> &pred)
    {
      rwlock.wrlock();
      removeEntry_if<>(pred);
      rwlock.unlock();
    }

    CSept();
    ~CSept();
    void      removeEntryByName(const char * name);
    DWORD      getID() {return id;}
    void      init(const stSeptInfo & info);
    CSeptMember *addSeptMember(const stSeptMemberInfo&);
    CSeptMember *addSeptMaster(const stSeptInfo& info);
    CSeptMember *getMemberByName(const char* pName);
    void      sendUserSeptData(const char *pName);
    void      fireSeptMember(const char *,const char *);
    int      fireSeptMemberDirect(const DWORD dwCharID,const bool checkunion=true);

    void      sendCmdToAllMember(const Cmd::stNullUserCmd * ptCmd,const DWORD nCmdLen );
    void      sendCmdToAllMemberExcept(const Cmd::stNullUserCmd * ptCmd,const DWORD nCmdLen,const char * except);
    bool      isMember(DWORD dwUserID);
    void      sendSeptNotify(const char* message,...);

    void      sendCmdToAllMemberScene(Cmd::Session::t_enterWar_SceneSession* ptEnterWarCmd,const DWORD cmdLen);
    void      sendCallCmdToAllMemberScene(Cmd::Session::t_GoTo_Leader_Check_SceneSession* ptEnterWarCmd,const DWORD cmdLen,DWORD expect);
    void      sendNpcDareCmdToScene(Cmd::Session::t_NpcDare_NotifyScene_SceneSession* pCmd,DWORD cmdLen);

    void      sendDistributeSeptExpToScene(const DWORD dwUserID,const Cmd::Session::t_distributeSeptExp_SceneSession * ptCmd);
    void      notifyMemberFire(const char *);
    void      writeDatabase();
    void      sendSeptMemberList(UserSession *pUser);
    void      full_SeptMember_Data(Cmd::stUnionRember*& tempPoint,DWORD& count);
    void      sendSeptInfoToUser(UserSession *pUser);
    bool      deleteMeFromDB();
    void      disbandSept();
    void      delSept(const DWORD dwSeptID);
    bool      loadSeptMemberFromDB();
    DWORD      size();
    bool      isVote();
    void      letVoteOver();
    
    void  setExp()
    {
      rwlock.wrlock();
      this->dwIsExp = 1;
      rwlock.unlock();
      this->writeDatabase();
    }
    
    void  clearExp()
    {
      rwlock.wrlock();
      this->dwIsExp = 0;
      rwlock.unlock();
      this->writeDatabase();
    }

    DWORD getExp()
    {
      return this->dwIsExp;
    }

    void      setNote(Cmd::stNoteSeptCmd *pCmd);
    void                    notifyWarResult(const char* msg,...);
    void       delSeptAllMember();
    DWORD       getRepute();//�õ�����ֵ
    void                    changeRepute(int repute);//�ı�����ֵ
    void      changeLevel(int level);
    void      sendSeptInfoToAll(); // ����֪ͨ������Ϣ
    void      sendSeptReputeToAll(); // ֪ͨ���м����Ա�����ĸı�
    void      notifyNpcHoldData();
    void      sendGoldToMember(DWORD userID,DWORD num);
};

struct CSeptMember:public zEntry
{
public:
    /// ��Ա״̬
    BYTE  byStatus;
    
    /// ��Ա����
    char  aliasname[MAX_NAMESIZE+1];

    /// ��Աְҵ
    WORD  wdOccupation;

    /// �û��ĻỰ����
    //UserSession * user;

    /// �������������ָ��
    CSept *mySept;

    /// ����״̬����ִ��д��Ȳ���
    bool  destroy;
    
    /// ������
    zRWLock rwlock;

	WORD	mnRight; //��ԱȨ��;
    /// ����״̬ö��
    enum {
          Offline,
          Online
    };


    void init(const stSeptMemberInfo&);
    void getMemberBaseInfo(struct Cmd::stSeptRember& info);
    void sendCmdToMe(const Cmd::stNullUserCmd *pstrCmd,const DWORD nCmdLen);
    void writeDatabase();
    void insertDatabase();
    void setSept(CSept * pSept);
    void online(const DWORD status = Cmd::SEPT_MEMBER_STATUS_ONLINE);
    void offline();
    bool isOnline();
    void sendMessageToMe(const char *);
    void sendMessageToMe(int type,const char *);
    void fireMe(const bool notify = true,const bool checkunion=true);
    bool deleteMeFromDB();
    void sendUserSeptData();
    void change_aliasname(const char* aliasname);
    void update_data();
    void update_normal_data();
    void notifyNpcHoldData();
    void sendGoldToMember(DWORD userID,DWORD num);
    CSeptMember();
    ~CSeptMember();
    
};

extern DWORD dare_active_time;
extern DWORD dare_ready_time;
extern int dare_need_gold;
extern int dare_winner_gold;

extern int dare_need_gold_sept;
extern int dare_winner_gold_sept;

typedef zUniqueID<DWORD> zUniqueDWORDID;

/**
 * \brief ��ս��¼������
 *
 * �����ս��¼������,ʹ��Singletonģʽ
 */
class CDareM:public zEntryManager<zEntryTempID>,
       public Singleton<CDareM>
{
  friend class SingletonFactory<CDareM>;

  private:
    zUniqueDWORDID *channelUniqeID;
    zRWLock rwlock;

    CDareM();

  public:
    ~CDareM();

    bool getUniqeID(DWORD &tempid);
    void putUniqeID(const DWORD &tempid);

    template <class YourEntry>
      bool execEveryOne(execEntry<YourEntry> &exec)
      {
        rwlock.rdlock();
        bool ret=execEveryEntry<>(exec);
        rwlock.unlock();
        return ret;
      }

    template <class YourEntry>
      void removeOne_if(removeEntry_Pred<YourEntry> &pred)
      {
        rwlock.wrlock();
        removeEntry_if<>(pred);
        rwlock.unlock();
      }

    bool init();
    static void destroyMe();

    /**
     * \brief ��ս�������Ķ�ʱ���ص�����
     *
     * ��ʱ�������ж�ս��¼,ɾ����Ч��¼,�����ﵽʱ��Ķ�ս����ش���
     */
    void timer();


    /**
     * \brief �����û��Ķ�ս����
     *
     * \param pUser ���͸������������Ӧ��UserSession����
     * \param pNullCmd �յ�������
     * \param cmdLen �����
     * \return �Ƕ�ս����,���õ���Ӧ����,����Ϊtrue,����Ϊfalse
     *
     */
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);

    
    /**
     * \brief �����û��ļ�������ս��ս����
     *
     * \param pUser ���͸������������Ӧ��UserSession����
     * \param pNullCmd �յ�������
     * \param cmdLen �����
     * \return �Ƕ�ս����,���õ���Ӧ����,����Ϊtrue,����Ϊfalse
     *
     */
    bool processSeptDare(UserSession *pUser,Cmd::stActiveDareCmd *pCmd,const DWORD cmdLen);

    /**
     * \brief �����û��İ������ս��ս����
     *
     * \param pUser ���͸������������Ӧ��UserSession����
     * \param pNullCmd �յ�������
     * \param cmdLen �����
     * \return �Ƕ�ս����,���õ���Ӧ����,����Ϊtrue,����Ϊfalse
     *
     */
    bool processUnionDare(UserSession *pUser,Cmd::stActiveDareCmd *pCmd,const DWORD cmdLen);


    /**
     * \brief ���������͹����ķ���������Ϣ
     *
     * \param cmd �������������͹���������ָ��
     * \param cmdLen �����
     *
     * \return ���Ѷ���ķ�����������,���õ���Ӧ����,����true,����Ϊfalse.
     *
     */
    bool processSceneMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);

    /**
     * \brief ���ҷ��������Ķ�ս��¼
     *
     *  ��ID2Ϊ0ʱ,�����Ƿ���ID1��ս�ļ�¼��dwType�����������롣
     *
     * \param dwType ��ս����:Cmd:UNION_DARE,Cmd:SCHOOL_DARE,Cmd:SEPT_DARE
     * \param dwID1  ����ID
     * \param dwID2  �ط�ID
     *
     * \return �ҵ��򷵻ض�Ӧ��ս��¼��ָ��
     */
    CDare * findDareRecord(DWORD dwType,DWORD dwID1,DWORD dwID2);
    CDare*  findDareRecordByFull(DWORD dwType,DWORD dwDefID,std::vector<DWORD>& vDareList);

    /**
     * \brief ���Ҹ�����ϵָ���ĵĶ�ս��¼,���ܸ�����ϵ���ڹ��������ط�
     *
     * \param dwType ��ս����:Cmd:UNION_DARE,Cmd:SCHOOL_DARE,Cmd:SEPT_DARE
     * \param dwID  ����ϵID
     *
     * \return �ҵ��򷵻ض�Ӧ��ս��¼��ָ��
     */
    CDare*  findDareRecordByID(DWORD dwType,DWORD dwID);
    
    /**
     * \brief ���ҷ��������Ķ�ս��¼
     *
     *  r1,r2û��˳������,��������һ��r����ս��������
     *  ��r2ΪNULLʱ,�����Ƿ���r1��ս�ļ�¼��dwType�����������롣
     *
     * \param dwType ��ս����:Cmd:UNION_DARE,Cmd:SCHOOL_DARE,Cmd:SEPT_DARE
     * \param r1  ����ϵ����1
     * \param r2  ����ϵ����2
     *
     * \return �ҵ��򷵻ض�Ӧ��ս��¼��ָ��
     */

    CDare * findDareRecordByRelationName(DWORD dwType,const char* r1,const char* r2);

    /**
     * \brief ���ҷ��������Ķ�ս��¼
     *
     *  ID1,ID2û��˳������,��������һ��ID����ս����ID��
     *  ��ID2Ϊ0ʱ,�����Ƿ���ID1��ս�ļ�¼��dwType�����������롣
     *
     * \param dwType ��ս����:Cmd:UNION_DARE,Cmd:SCHOOL_DARE,Cmd:SEPT_DARE
     * \param dwUserID1  ���ID1
     * \param dwUserID2  ���ID2
     *
     * \return �ҵ��򷵻ض�Ӧ��ս��¼��ָ��
     */
    CDare*  findDareRecordByUser(DWORD dwType,DWORD dwUserID1,DWORD dwUserID2);


    /**
     * \brief �����µĶ�ս��¼
     *
     * \param pCmd ��ս��������
     * \param dwType ��ս����
     * \param dwID1 ��ս������ϵID
     * \param dwID2 Ӧս������ϵID
     * \param dwUserID ��ս��ID
     *
     * \return û���ظ���¼,������ɹ�,����true,���򷵻�false
     *
     */
    bool addNewDareRecord(Cmd::stActiveDareCmd *pCmd,DWORD dwType,DWORD dwID1,DWORD dwID2,DWORD dwUserID);

    /**
      * \brief �û����ߴ���
      *
      * �û�����ʱ,ͬʱ�жϸ��û��Ķ�ս״̬,������Ӧ����
      *
      * \param pUser �����û�
      *
      */
    void userOnline(UserSession* pUser);

    void userOnlineCountry(UserSession* pUser);

    /**
     * \brief �������ظ���t_activeDare����
     *
     * \param cmd ��ս��¼
     */
    void activeDare_sceneSession(Cmd::Session::t_activeDare_SceneSession* cmd);

    /**
     * \brief ��������������ս��ʱ��PKս������
     *
     * \param cmd ����ָ��
     */
    void darePk_sceneSession(Cmd::Session::t_darePk_SceneSession* cmd);
    
    /**
     * \brief ���������������½���ս������
     *
     * \param pCmd ����ָ��
     */

    void createDare_sceneSession(Cmd::Session::t_createDare_SceneSession* pCmd,std::vector<DWORD>& dare_list);
    

    void createUnionCityDare_sceneSession(Cmd::Session::t_UnionCity_Dare_SceneSession* pCmd);
};

class CDare:public zEntry
{
  protected:
    zRWLock rwlock;                             // ��д��

    /**
     * \brief ���㲢���ͽ���
     *
     *  �ú���Ŀǰֻ��setWaitBountyState��������
     *
     * \return ���ͽ���ɹ�,����TRUE,���򷵻�ʧ�ܡ�
     */
    virtual bool computeResult();

    virtual char* getFirstName(){return NULL;}      // ��ս������ϵ������
    virtual DWORD getSecondUserID(){return 0;}    // Ӧս����ҵ�ID

    /// ��ս����ʱ��
    DWORD active_time;

    /// ��սǰ�ĵȴ�ʱ��
    DWORD ready_time;

    // ��ս��ʼʱ��
    time_t  start_time;

  public:
    enum{
      DARE_READY,   // ����ȴ�״̬
      DARE_READY_QUESTION,   // �ȴ�����ս�߻�Ӧ״̬      
      DARE_READY_ACTIVE,     // �ȴ��۳�Ӧս�߽��,���ʧ��,�����DARE_RETURN_GOLD
      DARE_RETURN_GOLD,      // �����Ҳ���,���Ԥ�ȿ۳�����ս���Ľ�Ǯ����������������DARE_OVER״̬��
      DARE_ACTIVE,   // ��ս״̬
      DARE_DATAPROCESS, // ���ݴ���״̬,��ʱδ��
      DARE_READY_OVER,       // ������ս,֪ͨ����ȡ�������˵Ķ�ս״̬,��������Ӯ,
      DARE_WAIT_BOUNTY,      // ���Ӯ������������,�������״̬,��������ߺ�,�ѽ������Ÿ���
      DARE_OVER    // �������״̬
    };

    CDare(DWORD active_time,DWORD ready_time); 
        
    virtual ~CDare();

    /**
     * \brief ʱ���¼�����ص�����,��TimeTick.cpp�к����ص�
     *
     *  ����ս����DARE_READY״̬�����,��ȡ����ս,������DARE_RETURN_GOLD״̬
     *  ����ս����DARE_READY_QUESTION  ����ս����DARE_READY
     *
     *
     */
    virtual void timer();  //ʱ���¼�����ص�


    // ����Ϊ���ö�ս״̬�Ĵ���������ս״̬����鿴��սϵͳ״̬ת��ͼ
    // ״̬ת������Ĭ�����,�Բ���Ҫ���ء�ֻ������ָ���ļ���������
    // �������ʵ��һ����Ĭ����Ϊ��ͬ��״̬ת�����̡�������Ҫ��������״̬ת������
    
    virtual void setReadyState();
    virtual void setReadyQuestionState();
    virtual void setReadyActiveState(UserSession* pUser);
    virtual void setReturnGoldState();
    virtual void setActiveState();
    virtual void setReadyOverState();
    virtual void setWaitBountyState();
    virtual void setOverState();
    virtual void sendAntiAtt(){};

    virtual void setSecondID(DWORD dwID)=0;
    virtual void addFirstID(DWORD dwID)=0;
    virtual bool isInvalid() {return false; }
    
    // ������ϵ�Ƿ����ڸö�ս��¼�Ĺ���
    virtual bool isAtt(DWORD dwID);
    
    virtual void notifyWarResult(int winner_type) = 0;

    /**
     * \brief ���Ͷ�ս����״̬������
     *
     *  ��������Ӧ�����Ӷ�ս��¼,��sendNineToMeʱ,���ж�ս״̬�ļ���,��
     *  ����ֻ���û����ߴ���ʱ�����á�
     *
     */
    virtual void sendActiveStateToScene(UserSession* pUser);


    /**
      * \brief ������������в���ö�ս�����
      *
      * ���Ͷ�ս�������������setActiveState��setReadyOverState��������
      *
      */
    virtual void 
    sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,
        DWORD cmdLen,DWORD relationID) = 0;

    /**
      * \brief �ж��Ƿ����ڶ�սʱ��
      *
      * \return ������ڶ�սʱ��,����TRUE,���򷵻�FALSE
      */
    virtual bool isActivePeriod()
    {
      time_t cur_time = time(NULL);
      return (cur_time-start_time)<this->active_time ? true : false;
    }

    /**
      * \brief �ж��Ƿ�����׼����
      *
      * \return �������׼����ʱ��,����TRUE,���򷵻�FALSE
      */
    virtual bool isReadyPeriod()
    {
      return this->count<this->ready_time ? true : false;
    }

    /**
      * \brief ���¼�����
      *
      */
    virtual void updateTime()
    {
      rwlock.wrlock();
      this->count += 5;
      rwlock.unlock();
    }

    /**
     * \brief ս���ӷ�
     *
     *  ����һ��,����Ӧ������ŶӼ���PK����һ�ˡ��÷ֹ�ʽ���£�
     *  grade = grade + user.level/10;
     *
     * \param pAtt ������
     * \param pDef ���ط�
     */
    virtual void addGrade(UserSession* pAtt,UserSession* pDef) = 0;

    void  printState();


    DWORD type;    // ս������:Cmd::UNION_DARE,Cmd::SCHOOL_DARE,Cmd::SEPT_DARE
    
    //DWORD firstID;    // ��ս������ϵID1
    std::vector<DWORD> attList;  // ��ս���б�
    DWORD secondID;    // ���ط�����ϵID2

    DWORD userid1;       // ��ս��ID
    DWORD userid2;       // Ӧս��ID
    DWORD grade1;           // ��ս����1�����÷���
    DWORD grade2;    // ��ս����2�����÷���
    DWORD pk1;              // ����ϵ1PK����
    DWORD pk2;              // ����ϵ2PK����
    DWORD count;    // ������λΪ��
    BYTE  state;         // ����״̬
    DWORD dwWinnerID;  // ���յ�Ӯ��ID,���ڶ������ʱʹ��
    bool  isAntiAtt;  // ���÷�����־
};


class CDareUnion : public CDare
{
  public:
    CDareUnion();
    CDareUnion(DWORD active_time,DWORD ready_time);
    virtual ~CDareUnion();

  public:    
    // ����Ϊ���ö�ս״̬�Ĵ���������ս״̬����鿴��սϵͳ״̬ת��ͼ
    virtual void setSecondID(DWORD dwID);
    virtual void addFirstID(DWORD dwID);

    virtual void sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,DWORD cmdLen,DWORD relationID);
    virtual void sendActiveStateToScene(UserSession* pUser);
    virtual void notifyWarResult(int winner_type);

    /**
     * \brief ս���ӷ�
     *
     *  ����һ��,����Ӧ������ŶӼ���PK����һ�ˡ��÷ֹ�ʽ���£�
     *  grade = grade + user.level/10;
     *
     * \param pAtt ������
     * \param pDef ���ط�
     */
    virtual void addGrade(UserSession* pAtt,UserSession* pDef);

    virtual void timer();
    virtual bool isInvalid();

  protected:    
    virtual char* getFirstName();
    virtual DWORD getSecondUserID();
};

class CDareSchool : public CDare
{
  public:
    CDareSchool();
    virtual ~CDareSchool();

  public:    
    // ����Ϊ���ö�ս״̬�Ĵ���������ս״̬����鿴��սϵͳ״̬ת��ͼ
    virtual void setSecondID(DWORD dwID);
    virtual void addFirstID(DWORD dwID);
    
    virtual void sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,DWORD cmdLen,DWORD relationID);
    virtual void sendActiveStateToScene(UserSession* pUser);
    virtual void notifyWarResult(int winner_type);

    /**
     * \brief ս���ӷ�
     *
     *  ����һ��,����Ӧ������ŶӼ���PK����һ�ˡ��÷ֹ�ʽ���£�
     *  grade = grade + user.level/10;
     *
     * \param pAtt ������
     * \param pDef ���ط�
     */
    virtual void addGrade(UserSession* pAtt,UserSession* pDef);

  protected:    
    virtual char* getFirstName();
    virtual DWORD getSecondUserID();

//    CSchool* pFirst;
//    CSchool* pSecond;
};

class CDareUnionCity : public CDareUnion
{
  public:
    CDareUnionCity(DWORD active_time,DWORD ready_time);
    virtual ~CDareUnionCity();

  public:    
    virtual void setSecondID(DWORD dwID){CDareUnion::setSecondID(dwID);}
    virtual void addFirstID(DWORD dwID){CDareUnion::addFirstID(dwID);}
    
    virtual void sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,
        DWORD cmdLen,DWORD relationID)
    {
      CDareUnion::sendCmdToAllDarePlayer(cmd,cmdLen,relationID);
    }
    
    virtual void sendActiveStateToScene(UserSession* pUser)
    {
      CDareUnion::sendActiveStateToScene(pUser);
    }
    
    virtual void setReadyState();
    virtual void setReadyOverState();

    /**
        * \brief ��ս���֪ͨ
      *
      * \param winner_type �������:0 ��ս��ʤ,1 Ӧս��ʤ,2 սƽ
      *      
      */
     virtual void notifyWarResult(int winner_type){};

    virtual void timer();  //ʱ���¼�����ص�

    /**
     * \brief ս���ӷ�
     *
     *  ����һ��,����Ӧ������ŶӼ���PK����һ�ˡ��÷ֹ�ʽ���£�
     *  grade = grade + user.level/10;
     *
     * \param pAtt ������
     * \param pDef ���ط�
     */
    virtual void addGrade(UserSession* pAtt,UserSession* pDef){};
    int last_fulltime;
};

class CDareCountry : public CDare
{
  public:
    CDareCountry();
    CDareCountry(DWORD active_time,DWORD ready_time);
    virtual ~CDareCountry();

  public:    
    // ����Ϊ���ö�ս״̬�Ĵ���������ս״̬����鿴��սϵͳ״̬ת��ͼ
    virtual void setSecondID(DWORD dwID);
    virtual void addFirstID(DWORD dwID);
    virtual void sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,DWORD cmdLen,DWORD relationID);

    virtual bool isInvalid();

    virtual void setActiveState();
    virtual void setReadyOverState();
    virtual void notifyWarResult(int winner_type) {};
    virtual void addGrade(UserSession* pAtt,UserSession* pDef){};

    virtual void sendAntiAtt();
    virtual void sendActiveStateToScene(UserSession* pUser);
    
    virtual void timer();
};

class CDareAntiCountry : public CDareCountry
{
  public:
    CDareAntiCountry();
    CDareAntiCountry(DWORD active_time,DWORD ready_time);
    virtual ~CDareAntiCountry();

  public:    
    // ֻ���ؽ�������.
    virtual void setReadyOverState();
};


class CDareSept : public CDare
{
  public:
    CDareSept();
    CDareSept(DWORD active_time,DWORD ready_time);
    virtual ~CDareSept();

  public:    
    // ����Ϊ���ö�ս״̬�Ĵ���������ս״̬����鿴��սϵͳ״̬ת��ͼ
    virtual void setSecondID(DWORD dwID);
    virtual void addFirstID(DWORD dwID);
    virtual bool isInvalid();

    virtual void setReadyQuestionState();
    virtual void setReadyActiveState(UserSession* pUser);
    virtual void setReturnGoldState();
    virtual void setReadyOverState();
    
    virtual void sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,DWORD cmdLen,DWORD relationID);
    virtual void sendActiveStateToScene(UserSession* pUser);
     virtual void notifyWarResult(int winner_type);

    /**
     * \brief ս���ӷ�
     *
     *  ����һ��,����Ӧ������ŶӼ���PK����һ�ˡ��÷ֹ�ʽ���£�
     *  grade = grade + user.level/10;
     *
     * \param pAtt ������
     * \param pDef ���ط�
     */
    virtual void addGrade(UserSession* pAtt,UserSession* pDef);
    virtual void timer();

    int dwDareRepute;
  protected:    
    virtual char* getFirstName();
    virtual DWORD getSecondUserID();

};


/**
 * \brief ����NPC����ս 
 *
 */
class CDareSeptNpc : public CDareSept
{
  public:
    CDareSeptNpc(DWORD active_time,DWORD ready_time);
    virtual ~CDareSeptNpc();

  public:    
    virtual void setSecondID(DWORD dwID){CDareSept::setSecondID(dwID);}
    virtual void addFirstID(DWORD dwID){CDareSept::addFirstID(dwID);}
    
    virtual void sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,DWORD cmdLen,DWORD relationID)
    {
      CDareSept::sendCmdToAllDarePlayer(cmd,cmdLen,relationID);
    }
    
    virtual void sendActiveStateToScene(UserSession* pUser)
    {
      CDareSept::sendActiveStateToScene(pUser);
    }

    /**
        * \brief ��ս���֪ͨ
      *
      * \param winner_type �������:0 ��ս��ʤ�� 1 Ӧս��ʤ�� 2 սƽ
      *      
      */
     virtual void notifyWarResult(int winner_type){};

    /**
     * \brief ս���ӷ�
     *
     *  ����һ�Σ�����Ӧ������ŶӼ���PK����һ�ˡ��÷ֹ�ʽ���£�
     *  grade = grade + user.level/10;
     *
     * \param pAtt ������
     * \param pDef ���ط�
     */
    virtual void addGrade(UserSession* pAtt,UserSession* pDef){};
    
    virtual void setReadyOverState();
};

class CDareEmperor : public CDare
{
  public:
    CDareEmperor();
    CDareEmperor(DWORD active_time,DWORD ready_time);
    virtual ~CDareEmperor();

  public:    
    // ����Ϊ���ö�ս״̬�Ĵ���������ս״̬����鿴��սϵͳ״̬ת��ͼ
    virtual void setSecondID(DWORD dwID);
    virtual void addFirstID(DWORD dwID);
    virtual bool isInvalid();

    virtual void setActiveState();
    virtual void setReadyOverState();
    
    virtual void sendCmdToAllDarePlayer(Cmd::Session::t_enterWar_SceneSession* cmd,DWORD cmdLen,DWORD relationID)
    {}

    virtual void addGrade(UserSession* pAtt,UserSession* pDef){};
    virtual void notifyWarResult(int winner_type) {};

    virtual void timer();
};

class CDareRecord
{
  public:
    CDareRecord()
    {
      dwTime = 0;
      dwAttCountryID = 0;
      dwDefCountryID = 0;
      dwResult = 0;
      bzero(attKingName,sizeof(attKingName));
      bzero(defKingName,sizeof(defKingName));
    }
    
    ~CDareRecord()
    {
    }
    
    void init(DBRecord* rec);
    //void writeDatabase();
    bool insertDatabase();
    //bool isMe(DWORD country,DWORD cityid,DWORD unionid);
    //bool changeUnion(DWORD unionid);

    DWORD  dwTime;
    DWORD  dwAttCountryID;
    DWORD  dwDefCountryID;
    DWORD  dwResult;
    char    attKingName[MAX_NAMESIZE+1];
    char    defKingName[MAX_NAMESIZE+1];

    zRWLock rwlock;

};

class CDareRecordM : public Singleton<CDareRecordM>
{
  friend class SingletonFactory<CDareRecordM>;
  
  public:
    bool init();
    void timer();
    void destoryMe()
    {
      delMe();
    };

    bool load();
    bool addNewDareRecord(DWORD dwAttCountry,DWORD dwDefCountry,DWORD dwResult);
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
    CDareRecord* findLastRecord(DWORD dwAttCountry,DWORD dwDefCountry);

  private:
    CDareRecordM(){}
    std::vector<CDareRecord*> vDareRecord;
    zRWLock rwlock;
};

class CNpcDareM
{
  public:
    CNpcDareM();
    bool init();
    static CNpcDareM &getMe();
    static void destroyMe();

    bool load();
    bool refreshDB();
    void timer();
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
    bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    void processRequest(Cmd::Session::t_NpcDare_Dare_SceneSession * rev);
    CNpcDareObj* findObject(DWORD country,DWORD mapid,DWORD npcid);
    bool searchSept(DWORD septid);
    CNpcDareObj* searchRecord(DWORD dwCountryID,DWORD dwMapID,DWORD dwNpcID);
    void doDare();
    void doResult();
    CNpcDareObj* getNpcDareObjBySept(DWORD septid);
    void processGetGold(Cmd::Session::t_NpcDare_GetGold_SceneSession *rev);
    void notifyDareReady();
    void forceProcessResult();
    void sendUserData(UserSession *pUser);
    CNpcDareObj* searchSeptHold(DWORD septid);
    template <class YourEntry>
    bool execEveryOne(execEntry<YourEntry> &exec)
    {
      std::vector<CNpcDareObj*>::iterator iter;
      for(iter = _objList.begin() ; iter != _objList.end() ; iter ++)
      {
        exec.exec((YourEntry*)*iter);
      }
      return true;
    }
  private:
    static CNpcDareM * um;
    std::vector<CNpcDareObj*> _objList;
    bool _notDare;
    bool _notifyDareMessage;
};

using namespace NpcDareDef;

class CNpcDareObj
{
  public:
    void create(NpcDareDef::NpcDareRecord &);
    void writeDatabase();
    bool isMe(DWORD country,DWORD mapid,DWORD npcid);
    void dareRequest(DWORD userId);
    bool doDare();
    void processResult(DWORD septid);
    void processGetGold(UserSession *pUser,DWORD septid,DWORD dwNpcID,DWORD dwMapID,DWORD dwCountryID);
    static void itemBack(UserSession *pUser);
    void doResult();
    bool notifyDareReady();
    void forceProcessResult();
    DWORD get_country();
    DWORD get_mapid();
    DWORD get_npcid();
    DWORD get_holdseptid();
    DWORD get_dareseptid();
    DWORD get_gold();
    DWORD get_posx();
    DWORD get_posy();

    void abandon_npc();

  private:
    DWORD _dwCountry;
    DWORD _dwMapID;
    DWORD _dwNpcID;
    DWORD _dwHoldSeptID;
    DWORD _dwDareSeptID;
    DWORD _dwGold;
    DWORD _dwPosX;
    DWORD _dwPosY;
    DWORD _dwResultHold;
    DWORD _dwResultDare;
    BYTE  _dareStep;
    zRTime resultTime;
};

extern DWORD quiz_active_time;
extern DWORD quiz_ready_time;
typedef zUniqueID<DWORD> zUniqueDWORDID;

/**
 * \brief �����¼������
 *
 * ��������¼������,ʹ��Singletonģʽ
 */
class CQuizM : public zEntryManager<zEntryTempID>,
         public Singleton<CQuizM>
{
  friend class SingletonFactory<CQuizM>;

  private:
    zUniqueDWORDID *channelUniqeID;
    zRWLock rwlock;
    CQuizM();

  public:
    bool getUniqeID(DWORD &tempid);
    void putUniqeID(const DWORD &tempid);

    template <class YourEntry>
      bool execEveryOne(execEntry<YourEntry> &exec)
      {
        rwlock.rdlock();
        bool ret=execEveryEntry<>(exec);
        rwlock.unlock();
        return ret;
      }

    template <class YourEntry>
      void removeOne_if(removeEntry_Pred<YourEntry> &pred)
      {
        rwlock.wrlock();
        removeEntry_if<>(pred);
        rwlock.unlock();
      }

    ~CQuizM();
    bool init();
    static void destroyMe();

    /**
     * \brief ��ս�������Ķ�ʱ���ص�����
     *
     * ��ʱ�������ж�ս��¼,ɾ����Ч��¼,�����ﵽʱ��Ķ�ս����ش���
     */
    void timer();
    void printSize();


          


    /**
     * \brief �����û��Ķ�ս����
     *
     * \param pUser ���͸������������Ӧ��UserSession����
     * \param pNullCmd �յ�������
     * \param cmdLen �����
     * \return �Ƕ�ս����,���õ���Ӧ����,����Ϊtrue,����Ϊfalse
     *
     */
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);

    /**
     * \brief ���������͹����ķ���������Ϣ
     *
     * \param cmd �������������͹���������ָ��
     * \param cmdLen �����
     *
     * \return ���Ѷ���ķ�����������,���õ���Ӧ����,����true,����Ϊfalse.
     *
     */
    bool processSceneMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);

    /**
     * \brief �����µľ�����¼
     *
     * \param pCmd ����ָ��
     * \return û���ظ���¼,������ɹ�,����true,���򷵻�false
     *
     */
    bool addNewQuiz_sceneSession(Cmd::Session::t_createQuiz_SceneSession* pCmd);



    /**
     * \brief ����ȫ��������¼
     *
     * \return ����ҵ�,���ض�Ӧָ��,���û��,����NULL
     */
    CQuiz* findWorldQuiz();

    /**
     * \brief ���Ҹ����ʴ��¼
     *
     * \return ����ҵ�,���ض�Ӧָ��,���û��,����NULL
     */
    CQuiz* findPersonalQuiz(DWORD dwUserID);
    
    /**
      * \brief �û����ߴ���
      *
      * �û�����ʱ,ͬʱ�жϸ��û��ľ���״̬,������Ӧ����
      *
      * \param pUser �����û�
      *
      */
    void userOnline(UserSession* pUser);
    
    /**
      * \brief �û����ߴ���
      *
      * �û�����ʱ,ͬʱ�жϸ��û��ľ���״̬,������Ӧ����
      *
      * \param pUser �����û�
      *
      */
    void userOffline(UserSession* pUser);
};


/**
 * \brief ������
 *
 */
class CPothunter
{
  public:
    DWORD dwUserID;   // ������ID
    int dwScore;    // �÷�
    DWORD dwGrace;    // �����Ĳɵ÷�
    int dwLuck;  // ������ʹ�ô���

    int dwAnswerStatus; // ���δ���״̬
    
    CPothunter()
    {
      dwUserID = 0;
      dwScore = 0;
      dwGrace = 0;
      dwLuck = 0;

      dwAnswerStatus = -1;
    }
    /*bool operator ==(CPothunter& ref)
    {
      return ref.dwUserID==this->dwUserID;
    }*/
    
    ~CPothunter(){}
};



class CSubject
{
  public:
    /*std::string title;
    std::string answer_a;
    std::string answer_b;
    std::string answer_c;
    std::string answer_d;
    */
    char title[256];
    char answer_a[56];
    char answer_b[56];
    char answer_c[56];
    char answer_d[56];
    char answer_e[56];
    char answer_f[56];

    DWORD answer;
    int  quiz_type; // 0Ϊȫ��,1Ϊ����,2Ϊ������

    CSubject(const CSubject& ref)
    {
      strcpy_s(this->title,ref.title);
      strcpy_s(this->answer_a,ref.answer_a);
      strcpy_s(this->answer_b,ref.answer_b);
      strcpy_s(this->answer_c,ref.answer_c);
      strcpy_s(this->answer_d,ref.answer_d);
      strcpy_s(this->answer_e,ref.answer_e);
      strcpy_s(this->answer_f,ref.answer_f);

      this->answer = ref.answer;
      this->quiz_type = ref.quiz_type;
    }

    CSubject()
    {
      answer=0;
      bzero(title,sizeof(title));
      bzero(answer_a,sizeof(answer_a));
      bzero(answer_b,sizeof(answer_b));
      bzero(answer_c,sizeof(answer_c));
      bzero(answer_d,sizeof(answer_d));
      bzero(answer_e,sizeof(answer_e));
      bzero(answer_f,sizeof(answer_f));
      quiz_type = 0;
    }

    const CSubject & operator= (const CSubject &ref)
    {
      strcpy_s(this->title,ref.title);
      strcpy_s(this->answer_a,ref.answer_a);
      strcpy_s(this->answer_b,ref.answer_b);
      strcpy_s(this->answer_c,ref.answer_c);
      strcpy_s(this->answer_d,ref.answer_d);
      strcpy_s(this->answer_e,ref.answer_e);
      strcpy_s(this->answer_f,ref.answer_f);
      
      this->answer = ref.answer;
      this->quiz_type = ref.quiz_type;
      return *this;
    }

    ~CSubject()
    {
#ifdef _DEBUG      
      Zebra::logger->debug("~CSubject:%s",this->title);
#endif        
    }
    
};

class CSubjectM : public Singleton<CSubjectM>
{
  public:
    ~CSubjectM();
    bool init();
    static void destroyMe();
    
    std::vector<CSubject> gsubjects;
    std::vector<CSubject> levels;
    std::vector<CSubject> personals;
    CSubjectM();
};

class CQuiz:public zEntry
{
  protected:
    zRWLock rwlock;                             // ��д��

    /// ����ǰ�ĵȴ�ʱ��
    DWORD ready_time;

    /// ready_question����ʱ��,Ĭ��10��
    DWORD ready_question_time;
    

    /// ������ǰ״̬
    BYTE state;

    /// ��������
    BYTE type;
    
    virtual void sendExitToAll(){};
    std::set<int> answered;


  public:
    /// ������ʱ,��ready_question����ʱ���������,�ٽ��м�ʱ,Ĭ�Ͻ���30����
    DWORD count;

    /// �������
    BYTE subject_type;

    DWORD active_time;

    /// ��ǰ�Ѵ���Ŀ��
    DWORD cur_subject;

    /// ���δ����������
    DWORD total_subject;

    /// �����ʱ
    DWORD question_count;

    enum{
      QUIZ_READY,  // ����ȴ�״̬
      QUIZ_READY_QUESTION,  // �ȴ�����Ӧ�߻�Ӧ״̬,����10��
      QUIZ_SEND_QUESTION,// ����״̬
      QUIZ_ACTIVE_QUESTION,// ����״̬
      QUIZ_END_QUESTION,    // ���������ʴ�,������Ӧ����(ȫ������ʱ,��������������������÷�,
            // �����ʴ�����е÷ֺ��Ĳɵķ���)
      QUIZ_READY_OVER,      // ��������,֪ͨ����ȡ�������˵Ķ�ս״̬,��������Ӯ,���ò鿴����ʱ��
      QUIZ_READ_SORT,  // ��״ֻ̬����ȫ��,�ȴ�����ʱ���,�ڴ�״̬�ٷ���һ�ν���
      QUIZ_RETURN_GOLD,// �����ʴ����,����δ���н���
      QUIZ_OVER    // �������״̬
    };

    CQuiz();
    CQuiz(DWORD active_time,DWORD ready_time); 
        
    virtual ~CQuiz();

    /**
     * \brief ʱ���¼�����ص�����,��TimeTick.cpp�к����ص�
     *
     *
     */
    void timer();  //ʱ���¼�����ص�


    // ����Ϊ���ö�ս״̬�Ĵ���������ս״̬����鿴��սϵͳ״̬ת��ͼ
    // ״̬ת������Ĭ�����,�Բ���Ҫ���ء�ֻ������ָ���ļ���������
    // �������ʵ��һ����Ĭ����Ϊ��ͬ��״̬ת�����̡�������Ҫ��������״̬ת������
    
    virtual void setReadyState();
    virtual void setReadyQuestionState();
    virtual void setSendQuestionState();
    virtual void setActiveQuestionState();
    virtual void setEndQuestionState();
    virtual void setReadyOverState();
    virtual void setReturnGoldState();
    virtual void setReadSortState();
    virtual void setOverState();

    virtual bool addPothunters(UserSession* pUser) = 0;

    /**
     * \brief ���������ǰ״̬
     *
     */
    void printState();

    /**
      * \brief �û����ߴ���
      *
      * �û�����ʱ,ͬʱ�жϸ��û��ľ���״̬,������Ӧ����,�����ʴ����߲��账��
      * ���羺��ʱ,��Ҫ�ж����Ƿ��Ѿ��μ��˾���,���û�вμ�,�򷢳�ѯ��,����Ѿ��μ�,���ٴ���
      *
      * \param pUser �����û�
      *
      */
    virtual void userOnline(UserSession* pUser) = 0;
    
    /**
      * \brief �û����ߴ���
      *
      * �û�����ʱ,ͬʱ�жϸ��û��ľ���״̬,������Ӧ����,�����ʴ�����ʱ,����setReadyOver״̬,ȫ������ʱ��������
      *
      * \param pUser �����û�
      *
      */
    virtual void userOffline(UserSession* pUser) = 0;

    /**
      * \brief �ж��Ƿ����ھ����ڼ�
      *
      * \return ������ڶ�սʱ��,����TRUE,���򷵻�FALSE
      */
    virtual bool isActivePeriod()=0;

    /**
      * \brief �ж��Ƿ�����׼����
      *
      * \return �������׼����ʱ��,����TRUE,���򷵻�FALSE
      */
    virtual bool isReadyPeriod()
    {
      return this->count<this->ready_time ? true : false;
    }
    
    /**
      * \brief �ж��Ƿ�����׼����
      *
      * \return �������׼����ʱ��,����TRUE,���򷵻�FALSE
      */
    virtual bool isReadyQuestionPeriod()
    {
      return this->count<ready_question_time ? true : false;
    }


    /**
     * \brief  �ж��Ƿ����ڲ鿴������
     *
     */
    virtual bool isReadSortPeriod()
    {
      return this->count<5?true:false;
    }
    
    virtual bool isActiveQuestionPeriod()
    {
      return this->question_count<=20 ? true : false;
    }

    virtual bool isEndQuestionPeriod()
    {
      return this->question_count<=25 ? true : false;
    }
    
    /**
      * \brief �ж��Ƿ�����׼����
      *
      * \return �������׼����ʱ��,����TRUE,���򷵻�FALSE
      */
    virtual void updateTime()
    {
      rwlock.wrlock();
      this->count += 1;
      this->question_count += 1;
      rwlock.unlock();
    }

    virtual BYTE getType()
    {
      return this->type;
    }

    virtual BYTE getState()
    {
      return this->state;
    }

    virtual int answer(Cmd::stAnswerQuiz* pCmd,DWORD dwUserID) = 0;
    virtual void exitQuiz(DWORD dwUserID) = 0;

    /// ���ξ�������Ŀ
    std::vector<CSubject>   subjects;
//    CSubject subjects[1];
//    int subjects[100];
};


class CQuizPersonal : public CQuiz
{
  public:
    CQuizPersonal();
    CQuizPersonal(DWORD active_time,DWORD ready_time);
    virtual ~CQuizPersonal();
    
    bool isActivePeriod()
    {
      return this->cur_subject<=total_subject;
    }
    
    void setSendQuestionState();
    void setEndQuestionState();
    void setReadyOverState();
    void setReturnGoldState();

    int answer(Cmd::stAnswerQuiz* pCmd,DWORD dwUserID);
    bool addPothunters(UserSession* pUser);
    void exitQuiz(DWORD dwUserID);
    
    /**
      * \brief �û����ߴ���
      *
      * �û�����ʱ,ͬʱ�жϸ��û��ľ���״̬,������Ӧ����
      *
      * \param pUser �����û�
      *
      */
    void userOnline(UserSession* pUser);
    void userOffline(UserSession* pUser);
    CPothunter pothunter;
  protected:
    bool award();
    UserSession* user;
};

class CQuizWorld : public CQuiz
{
  public:
    CQuizWorld();
    CQuizWorld(DWORD active_time,DWORD ready_time,BYTE subject_type=0);
    virtual ~CQuizWorld();

    void setSendQuestionState();
    void setEndQuestionState();
    void setReadyOverState();
    
    bool addPothunters(UserSession* pUser);
    int answer(Cmd::stAnswerQuiz* pCmd,DWORD dwUserID);
    
    void exitQuiz(DWORD dwUserID);

    bool isActivePeriod()
    {
      return this->cur_subject<=total_subject;
    }

    /**
      * \brief �û����ߴ���
      *
      * �û�����ʱ,ͬʱ�жϸ��û��ľ���״̬,������Ӧ����
      *
      * \param pUser �����û�
      *
      */
    void userOnline(UserSession* pUser);
    void userOffline(UserSession* pUser);
  public:    
    /// ������Ա�б�
    std::vector<CPothunter> pothunters;
//    int valid_pothunter; // ������Ч����
  private:
    virtual void sendExitToAll();

};

class RecordClient : public zTCPClient,public MessageQueue
{

  public:

    /**
     * \brief ���캯��
     * ���ڵ��������Ѿ���ѹ�����ģ����ڵײ㴫���ʱ��Ͳ���Ҫѹ����
     * \param name ����
     * \param ip ��ַ
     * \param port �˿�
     */
    RecordClient(
        const std::string &name,
        const std::string &ip,
        const WORD port)
      : zTCPClient(name,ip,port,false) {};

    bool connectToRecordServer();

    void run();
    bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);

};

extern RecordClient *recordClient;

class CSortM
{
  private:

    CSortM();

    static CSortM *csm;
    //WORD leveltable[MAX_LEVEL+10];

    /**
     * \brief ����
     */
    struct ltqword
    {
      bool operator()(const QWORD s1,const QWORD s2) const
      {
        return s1>s2;
      }
    };
    std::multimap<QWORD,DWORD,ltqword> _sortKey;
    std::map<DWORD,QWORD> _sortMap;

    typedef std::multimap<QWORD,DWORD,ltqword>::value_type keyValueType;
    typedef std::map<DWORD,QWORD>::value_type mapValueType;

  public:
    ~CSortM();
    static CSortM &getMe();
    bool init();
    static void destroyMe();
    void onlineCount(UserSession *pUser);
    void onlineCount(DWORD dwCharID,WORD wdLevel,QWORD qwExp);
    void offlineCount(UserSession *pUser);
    WORD getLevelDegree(UserSession *pUser);
    void upLevel(UserSession *pUser);
    bool createDBRecord();
    bool clearDBTable();

};

class CVoteItem
{
  public:
    DWORD dwOption;
    char szOptionDesc[MAX_NAMESIZE];
    DWORD dwBallot;
    DWORD dwVoteID;
    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool deleteMeFromDB();
    CVoteItem();
    ~CVoteItem();
};

class CVote
{
  friend class CVoteM;
  public:
    enum
    {
      VOTE_READY,           // ׼��״̬
      VOTE_ACTIVE,          // ����ͶƱ״̬
      VOTE_READY_OVER,      // ����ͶƱ���޸Ķ�Ӧ���ҿƼ�״̬
      VOTE_OVER               // �������״̬
    };
    
    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool deleteMeFromDB();
    bool loadItemFromDB();
    bool loadItemFromVec(std::vector<CTech*>& itemset);
    
    virtual void setReadyState();
    virtual void setActiveState();  
    virtual void setReadyOverState();
    virtual void setOverState();    
    virtual bool isActiveState();
    
    DWORD   getState()
    {
      return this->dwStatus;
    }

    DWORD   getType()
    {
      return this->dwType;
    }
    
    virtual void vote(UserSession* pUser,DWORD dwOption);
    bool addVoted(DWORD dwCharID);
    bool clearVoted();
    
    CVote();  
    virtual ~CVote();

  protected:
    DWORD dwID;
    DWORD dwCountryID;
    DWORD dwType;
    DWORD dwStatus;

    zRWLock rwlock;
    std::vector<CVoteItem*> items;
};

class CVoteM :   public Singleton<CVoteM>
{
  friend class SingletonFactory<CVoteM>;
  public:
    bool init();
    static void destroyMe();
    void timer();
    bool load();

    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
                bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);

    CVote* find(DWORD dwCountryID,DWORD dwType);
    CVote* findByID(DWORD dwVoteID);
    
    ~CVoteM();
    
    bool createNewVote(DWORD dwCountry,DWORD dwType,std::vector<CTech*>& items);
    void removeVote(DWORD dwCountry,DWORD dwType);
    void removeVoteByID(DWORD dwID);
    void force_close_vote(DWORD dwCountryID,DWORD dwType);
    
  protected:
    CVoteM();
    std::vector<CVote*> votes;
    void addNewVote();
    zRWLock rwlock;
};


typedef zUniqueID<DWORD> zUniqueDWORDID;

struct ArhatMapPoint
{
  DWORD dwMapID;
  DWORD x;
  DWORD y;
};
const int mappoint_num = 10;

/**
 * \brief ���������¼������
 *
 * �����ս��¼������,ʹ��Singletonģʽ
 */
class CGemM:public zEntryManager<zEntryTempID>,
      public Singleton<CGemM>
{
  friend class SingletonFactory<CGemM>;

  private:
  zUniqueDWORDID *channelUniqeID;
  zRWLock rwlock;

  CGemM();

  public:
  ~CGemM();

  bool getUniqeID(DWORD &tempid);
  void putUniqeID(const DWORD &tempid);

  template <class YourEntry>
    bool execEveryOne(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
    }

  template <class YourEntry>
    void removeOne_if(removeEntry_Pred<YourEntry> &pred)
    {
      rwlock.wrlock();
      removeEntry_if<>(pred);
      rwlock.unlock();
    }

  ArhatMapPoint wait_point[mappoint_num];
  bool init();
  static void destroyMe();

  /**
   * \brief ��������������Ķ�ʱ���ص�����
   *
   * ��ʱ�������ж�ս��¼,ɾ����Ч��¼,�����ﵽʱ��Ķ�ս����ش���
   */
  void timer();


  /**
   * \brief �����û��Ķ�ս����
   *
   * \param pUser ���͸������������Ӧ��UserSession����
   * \param pNullCmd �յ�������
   * \param cmdLen �����
   * \return �ǻ��������������,���õ���Ӧ����,����Ϊtrue,����Ϊfalse
   *
   */
  bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);

  /**
   * \brief ���������͹����ķ���������Ϣ
   *
   * \param cmd �������������͹���������ָ��
   * \param cmdLen �����
   *
   * \return ���Ѷ���ķ�����������,���õ���Ӧ����,����true,����Ϊfalse.
   *
   */
  bool processSceneMessage(const Cmd::t_NullCmd *cmd,const DWORD cmdLen);

  /**
   * \brief ���ҷ��������Ļ�������
   *
   *  ��ID2Ϊ0ʱ,�����Ƿ���ID1��ս�ļ�¼��dwType�����������롣
   *
   * \param dwType ��ս����:Cmd:UNION_DARE,Cmd:SCHOOL_DARE,Cmd:SEPT_DARE
   * \param dwID1  ����ID
   * \param dwID2  �ط�ID
   *
   * \return �ҵ��򷵻ض�Ӧ��ս��¼��ָ��
   */
  CGem * findGem(DWORD dwCountryID);

  /**
   * \brief ����ID���һ�������
   *
   * \param dwID  GEM ID
   *
   * \return �ҵ��򷵻ض�Ӧ���������¼��ָ��
   */
  CGem*  findGemByID(DWORD dwID);

  /**
   * \brief �����µĶ�ս��¼
   *
   * \param dwCountryID ������������
   *
   * \return û���ظ���¼,������ɹ�,����true,���򷵻�false
   *
   */
  bool addNewGem(DWORD dwCountryID);
        

  void forceEnd();

  /**
   * \brief �û����ߴ���
   *
   * �û�����ʱ,ͬʱ�жϸ��û��Ķ�ս״̬,������Ӧ����
   *
   * \param pUser �����û�
   *
   */
  void userOnline(UserSession* pUser);
  
  /**
   * \brief �û����ߴ���
   *
   * �û�����ʱ,ͬʱ״̬�������Ļ���״̬,������Ӧ����
   *
   * \param pUser �����û�
   *
   */
  void userOffline(UserSession* pUser);
};

/// �޺�
class CArhat
{
  public:
  DWORD x;
  DWORD y;
  DWORD dwCountryID;
  DWORD dwMapRealID;
  BYTE  byState; // ״̬,0Ϊ����δ���˻�ȡ,1Ϊ�����ѱ��˻�ȡ
  DWORD dwHoldUserID; // ����Ŀǰ�������ID
  DWORD dwID;  // �޺���ID

  CArhat()
  {
    x = 0;
    y = 0;
    dwCountryID = 0;
    dwMapRealID = 0;
    byState = 0;
    dwHoldUserID = 0;
    dwID = 0;
  }

  bool refreshNPC();
  bool clearNPC();
};

class CGem : public zEntry
{
  protected:
    zRWLock rwlock;                             // ��д��
    time_t tmStart;            // ��ʼʱ��

  public:
    enum{
      GEM_READY,  // ��������׼��״̬
      GEM_ACTIVE,  // �����������״̬
      GEM_READY_OVER,      // ��������׼������״̬
      GEM_OVER    // �������״̬
    };

    CGem();

    virtual ~CGem();

    /**
     * \brief ʱ���¼�����ص�����,��TimeTick.cpp�к����ص�
     *
     *  ����ս����DARE_READY״̬�����,��ȡ����ս,������DARE_RETURN_GOLD״̬
     *  ����ս����DARE_READY_QUESTION  ����ս����DARE_READY
     *
     *
     */
    virtual void timer();  //ʱ���¼�����ص�


    // ״̬������,��timer��״̬��Ǩ�Ĵ���
    virtual void setReadyState();
    virtual void setActiveState();
    virtual void setReadyOverState();
    virtual void setOverState();

    /**
     * \brief ���Ͷ�ս����״̬������
     *
     *  ��������Ӧ�����Ӷ�ս��¼,��sendNineToMeʱ,���ж�ս״̬�ļ���,��
     *  ����ֻ���û����ߴ���ʱ�����á�
     *
     */
    virtual void sendActiveStateToScene(UserSession* pUser);

    /**
     * \brief �ж��Ƿ����ڻ�������ʱ��
     *
     * \return ������ڻ�������ʱ��,����TRUE,���򷵻�FALSE
     */
    virtual bool isActivePeriod();

    void  printState();

    void    holdDragon(UserSession* pUser);
    void    holdTiger(UserSession* pUser);

    void    resetDragon();
    void    resetTiger();

    CArhat dragon; // ����
    CArhat tiger;   // ����
    BYTE   state;         // ����״̬
    DWORD  dwCountryID;  // ����������������
};


class CCaptain
{//�ӳ�
  public:
    DWORD dwArmyID; // ����ID
    DWORD dwCharID; // �ӳ�ID
    char  szCapName[MAX_NAMESIZE]; // �ӳ�����
    DWORD dwNpcNum; // �ӳ������NPC������ʱδ��
    CArmy* myArmy;
    
    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool deleteMeFromDB();
    void update_scene();
    void fireMe();

    CCaptain();
    ~CCaptain();
};

class CArmy
{
  friend class CArmyM;
  friend class CCaptain;
  public:
    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool deleteMeFromDB();
    bool loadCaptainFromDB();

    bool fireCaptain(DWORD dwUserID);
    bool hireCaptain(DWORD dwUserID);
    void changeName(const char* newname);
    void update_all_captain();
    bool canAddCaptain();
    
    CArmy();
    ~CArmy();
    enum
    {
      WAIT_CREATE = 1,
      FINISH_CREATE = 2,
    };

    void status(BYTE value)
    {
      rwlock.wrlock();
      byStatus = value;
      rwlock.unlock();
    }

  protected:
    DWORD dwID; // ����ID
    DWORD dwCountryID; // ��������ID
    DWORD dwCityID;  // ��������ID
    
    char  name[MAX_NAMESIZE]; // ��������
    DWORD  dwGenID; // ���ӽ�����ɫID
    char  genName[MAX_NAMESIZE]; // ���ӽ�������

    BYTE  byStatus;
    DWORD dwCreateTime;

    zRWLock rwlock;
    std::vector<CCaptain*> captains;
};

class CArmyM : public Singleton<CArmyM>
{
  friend class SingletonFactory<CArmyM>;
  public:
    bool init();
    static void destroyMe();
    void timer();
    bool load();
    
    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
    bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);

    void processReqWaitGen(UserSession* pUser,Cmd::stReqWaitGenUserCmd* rev);
    void processCreateArmy(UserSession* pUser,Cmd::stCreateArmyUserCmd* rev);
    void processChangeArmyName(UserSession* pUser,Cmd::stChangeArmyNameUserCmd* rev);
    
    void processReqArmyList(Cmd::Session::t_ReqArmyList_SceneSession* rev);
    void processReqArmySpec(UserSession* pUser,Cmd::stReqArmySpecUserCmd* rev);
    void processExitArmy(UserSession* pUser,Cmd::stExitArmyUserCmd* rev);
    void processRemoveArmy(UserSession* pUser,Cmd::stRemoveArmyUserCmd* rev);
    void processAddCaptain(UserSession* pUser,Cmd::stAddArmyCaptainUserCmd* rev);
    void processFireCaptain(UserSession* pUser,Cmd::stFireArmyCaptainUserCmd* rev);
    
    void userOnline(UserSession *pUser);
    
    //CArmy* find(DWORD dwCountryID,DWORD dwCityID);
    int    countByCity(DWORD dwCountryID,DWORD dwCityID);
    CArmy* findByID(DWORD dwArmyID);
    CArmy* findByGenID(DWORD dwGenID);
    CArmy* findByName(const char* value);

    bool  addCaptain(DWORD dwUserID,CCaptain* pCaptain);
    bool  removeCaptain(DWORD dwUserID);
    bool    isCaptain(DWORD dwUserID);

    ~CArmyM();
    void removeArmyByID(DWORD dwArmyID);
    void removeArmyByGenID(DWORD dwGenID);

    std::vector<CArmy*> armys;
    /// ���Ͷ���
    typedef std::map<DWORD,CCaptain*>::value_type captainIndexValueType;
        
    typedef std::map<DWORD,CCaptain *>::iterator capIter;
    std::map<DWORD,CCaptain*> captainIndex;
    

  protected:
    CArmyM();
    zRWLock rwlock;
};


class RecommendSub
{
  public:
    char name[MAX_NAMESIZE]; // ���Ƽ�������
    DWORD id; // ���Ƽ����û�ID
    DWORD lastLevel; // ���һ����ȡ�����ĵȼ�
    DWORD recommendid; // �Ƽ���ID
    DWORD dwTotal; // �ۼ�����ȡ�Ľ��

    Recommend*  myRecommend; // �ҵ��Ƽ���
    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool deleteMeFromDB();
    bool fireMe();
    void pickupBounty(UserSession* pUser);
    DWORD queryBounty();
    DWORD queryTotal()
    {
      return dwTotal;
    }

    RecommendSub();
    ~RecommendSub();
};

class Recommend
{
  friend class RecommendM;
  friend class RecommendSub;

  public:
    void init(DBRecord* rec);
    void writeDatabase();
    bool insertDatabase();
    bool deleteMeFromDB();
    bool loadRecommendSubFromDB();

    bool addRecommended(DWORD dwUserID);
    void rmRecommended(DWORD dwUserID);
    void processQuery(UserSession* pUser);

    void pickupBounty(UserSession* pUser);
    DWORD queryBounty()
    {
      return this->dwBalance;
    }

    DWORD queryTotal()
    {
      return this->dwTotal;
    }
    

    Recommend();
    ~Recommend();
      
  protected:
    DWORD id; // �Ƽ���ID
    char  name[MAX_NAMESIZE]; // �Ƽ�������
    DWORD dwBalance;  // �Ƽ��˿�����ȡ�Ľ���
    DWORD dwTotal;    // �ۻ���ȡ�Ľ���
    std::vector<RecommendSub*> subs; // ���Ƽ����б�
    zRWLock rwlock;
};

class RecommendM : public Singleton<RecommendM>
{
  friend class SingletonFactory<RecommendM>;
  public:
    bool init();
    static void destroyMe();
    void timer();
    bool load();

    bool processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen);
    bool processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    void processAddRecommended(const Cmd::Record::t_chkUserExist_SessionRecord* cmd);

    Recommend* findByID(DWORD dwUserID);
    RecommendSub* findSubByID(DWORD dwUserID);

    bool addRecommend(DWORD dwUserID,Recommend* r);
    bool addRecommendSub(DWORD dwUserID,RecommendSub* rs);
    void removeRecommend(DWORD dwUserID);
    bool removeRecommendSub(DWORD dwUserID);

    void fireRecommendSub(DWORD dwUserID);

    ~RecommendM();
    
    typedef std::map<DWORD,Recommend*>::value_type recommendValueType;
    typedef std::map<DWORD,Recommend*>::iterator recommendIter;
    std::map<DWORD,Recommend*> recommends;
    /// ���Ͷ���
    typedef std::map<DWORD,RecommendSub*>::value_type recommendsubIndexValueType;
        
    typedef std::map<DWORD,RecommendSub *>::iterator resubIter;
    std::map<DWORD,RecommendSub*> recommendsubIndex;
  protected:
    RecommendM();
    zRWLock rwlock;
};

#pragma pack(1)
struct actInfo
{
  DWORD id;
  char name[MAX_NAMESIZE];
  BYTE state;
  char text[MAX_CHATINFO];
};
#pragma pack()

class Gift : public Singleton<Gift>
{
  friend class SingletonFactory<Gift>;
  private:
    Gift();

    static std::vector<actInfo> actList;
    static std::multimap<DWORD,Cmd::Session::giftInfo> giftList;
    static std::multimap<DWORD,std::string> winnerList;
    static DWORD loadActList();
    static DWORD loadGiftList();
  public:
    ~Gift();
    bool init();

    bool doGiftCmd(UserSession * pUser,const Cmd::stNullUserCmd *cmd,const DWORD cmdLen);
};

class EmperorForbid : public Singleton<EmperorForbid>
{
  friend class SingletonFactory<EmperorForbid>;
  private:
    std::vector<DWORD> list;

    void loadDB();
    void writeDB();

    EmperorForbid();

    void clear();
    bool find(DWORD);
  public:
    ~EmperorForbid();

    void timer();
    DWORD count();
    bool add(DWORD);
};

struct forbidInfo
{
  char name[MAX_NAMESIZE+1];
  QWORD startTime;
  int delay;
  WORD operation;
  char reason[256];
  char gm[MAX_NAMESIZE+1];
  WORD isValid;
};

class ForbidTalkManager
{
  private:
    static ForbidTalkManager *ftm;
    ForbidTalkManager();
    ~ForbidTalkManager();
  public:
    static ForbidTalkManager& getMe();
    static void delMe();

    static void checkDB();
};

using namespace std;


/**
* \sky ���鳡����������
*/
struct TeamSceneExec
{
	virtual ~TeamSceneExec(){};
	virtual bool exec(SceneSession * scene) = 0;
};

/**
* \sky �����������֪ͨ�����ӳ���Ϣ
*/
struct TeamChangeLeaberSceneExec : public TeamSceneExec
{
	Cmd::Session::t_Team_ChangeLeader rev;

	TeamChangeLeaberSceneExec(DWORD TeamThisID, char * NewName)
	{
		rev.dwTeam_tempid = TeamThisID;
		strncpy(rev.NewLeaderName, NewName, MAX_NAMESIZE);
	}

	bool exec(SceneSession * scene)
	{
		scene->sendCmd(&rev, sizeof(Cmd::Session::t_Team_ChangeLeader));
		return true;
	}


};

/**
* \sky �����������֪ͨ���Ӷ�Ա��Ϣ
*/
struct TeamAddMemberSceneExec : public TeamSceneExec
{
	Cmd::Session::t_Team_AddMember rev;

	TeamAddMemberSceneExec(DWORD leaberID, DWORD TeamThisID, char * AddMemberName, DWORD dwID, DWORD face)
	{
		rev.dwLeaderID = leaberID;
		rev.dwTeam_tempid = TeamThisID;
		strncpy(rev.AddMember.name, AddMemberName, MAX_NAMESIZE);
		rev.AddMember.dwID = dwID;
		rev.AddMember.face = face;
	}

	bool exec(SceneSession * scene)
	{
		scene->sendCmd(&rev, sizeof(Cmd::Session::t_Team_AddMember));
		return true;
	}
};

/**
* \sky �����������֪ͨɾ����Ա��Ϣ
*/
struct TeamDelMemberSceneExec : public TeamSceneExec
{
	Cmd::Session::t_Team_DelMember rev;

	TeamDelMemberSceneExec(DWORD TeamID, char * DelName)
	{
		rev.dwTeam_tempid = TeamID;
		strncpy(rev.MemberNeam, DelName,MAX_NAMESIZE);
	}

	bool exec(SceneSession * scene)
	{
		scene->sendCmd(&rev, sizeof(Cmd::Session::t_Team_DelMember));
		return true;
	}
};

/**
/* \sky �����������֪ͨɾ��������Ϣ                                                                      
*/
struct TeamDelTeamSceneExec : public TeamSceneExec
{
	Cmd::Session::t_Team_DelTeam rev;

	TeamDelTeamSceneExec(DWORD TeamThisID)
	{
		rev.TeamThisID = TeamThisID;
	}

	bool exec(SceneSession * scene)
	{
		scene->sendCmd(&rev, sizeof(Cmd::Session::t_Team_DelTeam));
		return true;
	}
};

/**
 * \brief Session�Ķ�����Ϣ
 *
 */
struct Team
{
  Team():leaderid(0){};
  typedef set<DWORD> MemberSet;
  typedef set<SceneSession *> MapIDSet;
  typedef MemberSet::iterator MemberSet_iter;
  typedef MemberSet::const_iterator MemberSet_const_iter;

  //Sky �����Խ�ĳ�����ID����
  MapIDSet	MapID;

  private:
  MemberSet member;
  DWORD leaderid; //sky �ӳ�ID
  DWORD dwTeam_tempid; //sky ����ΨһID
  

  
  public:
  bool addMember(const DWORD userid);
  bool delMember(const DWORD userid);
  bool setLeader(const DWORD leader);
  DWORD GetLeader();
  bool delTeam();
  //sky���ö����ΨһID
  void SetTeamThisID(DWORD TeamThisID);

  //sky ÿ��һ�������Ա�糡����ʱ��,���������ƶ����ĵ�ͼ�ŵ�������������
  //����֪ͨ�ó����������Ķ��������
  //���з��͸��³�����������
  bool MemberMoveScen(SceneSession * scene);

  //sky�¼Ӹ����ӳ�����
  bool ChangeLeader(const char * leaberName=NULL);

  //sky�����������ڵ�ȫ������
  bool execEvery(TeamSceneExec &callback);

  //sky ����MapID����
  void UpDataMapID(DWORD useID);

  //sky ��ȡ�ض���ԱID
  DWORD GetMemberID(int i);

  //sky ��ȡ��Ա��Ŀ
  DWORD GetMemberNum();
};

/**
 * \brief Session���������
 *
 */
class GlobalTeamIndex
{
private:
	GlobalTeamIndex(){};
	~GlobalTeamIndex(){};
	static GlobalTeamIndex *instance;

	typedef map<DWORD,Team> TeamMap;
	typedef TeamMap::value_type TeamMap_value_type;
	typedef TeamMap::iterator TeamMap_iterator;
	typedef TeamMap::const_iterator TeamMap_const_iterator;
	TeamMap team;
	zMutex mlock;

public:
	static GlobalTeamIndex *getInstance();
	static void delInstance();
	bool addMember(const DWORD tempid, const DWORD leaderid,const DWORD userid);
	bool delMember(const DWORD tempid,const char * userName);
	bool ChangeLeader( DWORD tempid, const char * leaberName=NULL);

	//sky ÿ��һ�������Ա�糡����ʱ��,���������ƶ����ĵ�ͼ�ŵ�������������
	//����֪ͨ�ó����������Ķ��������
	bool MemberMoveScen(const DWORD tempid, SceneSession * scene);

	//sky ���ݶ���ΨһID����һ���������ָ��
	Team * GetpTeam(const DWORD tempid);

	//sky ɾ������
	bool DelTeam(DWORD TeamThisID);

	//sky ����MapID����
	void UpDataMapID(DWORD useID, DWORD TeamThisID);
};

//sky �����ŶӵĽṹ
struct QueueTeamData
{
	QueueTeamData()
	{
		TeamID = 0;
		AddTime = 0;
	}

	DWORD	TeamID;		//ID
	QWORD	AddTime;	//�Ŷӵ�ʱ��
};

//sky �Ŷӹ�����
class CQueueManager
{
private:
	std::vector<QueueTeamData>	WaitTeamID;		//sky �ȴ��Ķ������
	std::vector<DWORD>			WaitUserID;		//sky �ȴ����û�����

public:
	CQueueManager();
	~CQueueManager();

	//sky �û��Ŷ���
	zMutex QueueUserLock;
	//sky �����Ŷ���
	zMutex QueueTeamLock;
	void Queue_AddUser(DWORD UserID);					//sky ��Ҫ�Ŷӵ��û���ӵ��Ŷ�������
	void Queue_AddTeam(DWORD UserID);		//sky ��Ҫ�ŶӵĶ�����ӵ��Ŷ�������
	bool Queue_RemoveUser(DWORD * UserID, int num);		//sky ����num�����Բ������û�ID�����Ŷ�������ɾ��
	bool Queue_RemoveTeam(DWORD * UserID, int num);		//sky ����num�����Բ������û�ID���Ӷ����Ŷ�������ɾ��
	bool IfUserInWait(BYTE type, DWORD UserID);			//sky �û��Ƿ��Ѿ����ڵ�ǰ������
};

struct CampData
{
	WORD Teamnum;			//sky ��Ӫ��Ա��
	std::vector<DWORD> UserID;	//sky ����õĳ�Ա
};

struct SceneMapData
{
	DWORD SceneID;
	Cmd::Session::CampPos pos[20];
};

typedef std::vector<CampData> tyCamp;

class CQueuingManager
{
private:
	//sky ���ȼ����ֵ��Ŷ�����
	std::map<DWORD, CQueueManager*> Queuing;

	//sky �Ѿ������õ�ս��ID
	std::vector<SceneMapData> sceneMapID;

	//sky ��Ӫ��������,����������ɹ���������MaxCampNum���볡��
	std::map<DWORD, tyCamp> Camp;
public:
	//sky �����±�ƫ��ֵ
	int MapIDMove;
	//sky ��ͼ������(������Ϊ�������ӳٴ����²�ͣ����)
	bool NewMapLock;
	//sky ��Ӧ��_ArenaMap��key(����new��ͼ��Ϣ��Ҫ�õ�)
	DWORD ManagerKey;
	//sky ������ͼID
	DWORD MapBaseID;
	//sky ��Ӫ�������
	WORD MaxTeamnum;
	//sky ս��������Ӫ��
	WORD MaxCampNum;
	//sky ��־(false:�ӵ�����Ҷ�����ȡ true:�Ӷ��������ȡ)
	bool bAccess;
public:
	CQueuingManager();
	~CQueuingManager();

	//sky ��Ҫ�Ŷӵ��û���ӵ��Ŷ�������
	void Queuing_AddUser(DWORD UserID, BYTE UserType);
	//sky ִ�в�ѯ��������(����������)
	void Queuing_Main();
	//sky �����û����Ѿ����ɺõĵ�ͼ��
	bool DistributionUser(int index);
	//sky ֪ͨս�����߾���������һ����ͼ
	bool SendSceneNewMap(int index);
	//sky ��һ�������õ�ս����ͼ�ķŵ�ս��������
	bool Queuing_AddScene(Cmd::Session::t_Sports_ReturnMapID * cmd);
	//sky �û��Ƿ��Ѿ����ڵ�ǰս��������
	bool IfUserInWaits(BYTE type, DWORD UserID);
};

//sky ս�����������й�����
class CArenaManager
{
private:
	std::map<DWORD,CQueuingManager*> _ArenaMap;		//sky 3cս�����й�����
public:
	CArenaManager();
	~CArenaManager();

	//sky ���������ӷŵ��б�������
	void InsertBattleTask(SessionTask * pTask);
	//sky ��Ҫ�Ŷӵ��û���ӵ���Ӧ��ս���Ŷ�������
	void Arena_AddUser(Cmd::Session::t_Sports_AddMeToQueuing * cmd);
	void Arena_timer();
	//sky ��ȡ�����ļ�����ս������
	bool LoadXmlToArena();
	//sky �Ѿ���ս���ŵ�index�����Ķ��й�������
	bool AddMapToQueuing(Cmd::Session::t_Sports_ReturnMapID * cmd);
	//sky �����ض����й�����ĳ���������
	void NewMap_Lock(DWORD index, bool block);
	//sky �ƶ��ض����й�����ĳ���ƫ��
	void MoveSceneM(DWORD index);
	//sky �û��Ƿ��Ѿ����ڶ��й�������
	bool IfUserInQueuing(BYTE type, DWORD UserID);

	//sky ��ȡ���ߴ������Ψһʵ��
	static CArenaManager &getInstance()
	{
		if (NULL == instance)
			instance = new CArenaManager();

		return *instance;
	}

	//sky �ͷ����Ψһʵ��
	static void delInstance()
	{
		SAFE_DELETE(instance);
	}

private:
	//sky ���Ψһʵ��ָ��
	static CArenaManager *instance;
};

struct zMutex;
class zRTime;
/**
 * \brief ��ʱ�������ݽṹ
 *
 */
struct TempArchive 
{
  TempArchive() 
  {
    id = 0;
    createtime = 0;
    dwSize = 0;
  }
  DWORD id;
  zRTime createtime;
  DWORD dwSize;
  char data[0];
};

/**
 * \brief ��ʱ����������
 *
 */
class GlobalTempArchiveIndex//:private zEntryManager< zEntryID >
{
  private:
    static GlobalTempArchiveIndex *_instance;
    typedef std::map<DWORD,TempArchive *> TempArchive_map;
    typedef TempArchive_map::iterator TempArchive_iterator;
    typedef TempArchive_map::value_type TempArchive_value_type;
    TempArchive_map tempArchive;
    zMutex mlock;

    GlobalTempArchiveIndex();
    ~GlobalTempArchiveIndex();
    void remove(TempArchive_iterator del_iter);
  public:
    static GlobalTempArchiveIndex *getInstance();
    static void delInstance();
    bool readTempArchive(DWORD id,char *out,DWORD &outSize);
    void checkOverdue();
    bool writeTempArchive(DWORD id,char *data,DWORD  dwSize);
};

class SessionTimeTick : public zThread
{

  public:

    static zRTime currentTime;

    ~SessionTimeTick() {};

    static SessionTimeTick &getInstance()
    {
      if (NULL == instance)
        instance = new SessionTimeTick();

      return *instance;
    }

    /**
     * \brief �ͷ����Ψһʵ��
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    void run();

  private:
    Timer _five_sec;
    Timer _one_sec;
    Timer _one_min;
    Timer _ten_min;
    Timer _one_hour;
    Timer _five_min; // [ranqd Add] ����Ӷ�ʱ��
    static SessionTimeTick *instance;

    SessionTimeTick() : zThread("TimeTick"),_five_sec(5),_one_sec(1),_one_min(60),_five_min(300),_ten_min(60*10),_one_hour(3480) {};

};

extern NFilterModuleArray g_nFMA;
