/**
 * \brief ʵ�ֹ��������
 *
 * ��һ�����е����з��������й���
 * 
 */
#include <zebra/srvEngine.h>
#include <set>

enum NetType
{
  NetType_near = 0,//����·�ɣ������������ŷ���������ͨ������ͨ������
  NetType_far = 1    //Զ��·�ɣ�����������ͨ����������ͨ�������ŷ�����
};


/**
 * \brief ���������֮���������ϵ
 *
 */
extern hash_map<int,std::vector<int> > serverSequence;

/**
 * \brief ��ʼ��������֮���������ϵ
  
   gcc 
 *
 */
void initServerSequence();




/**
 * \brief �����������
 *
 * �����˻���<code>zNetService</code>
 *
 */
class SuperService : public zNetService
{

  public:

    /**
     * \brief ��������
     *
     * �麯��
     *
     */
    ~SuperService()
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
      if (taskPool)
        return taskPool->getSize();
      else
        return 0;
    }

    /**
     * \brief ��ȡ���Ψһʵ��
     *
     * ʹ����Singleton���ģʽ����֤��һ��������ֻ��һ�����ʵ��
     *
     * \return ���Ψһʵ��
     */
    static SuperService &getInstance()
    {
      if (NULL == instance)
        instance = new SuperService();

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

    /**
     * \brief ��ȡ��Ϸ�����
     * \return ������Ϸ�����
     */
    const GameZone_t &getZoneID() const
    {
      return gameZone;
    }

    /**
     * \brief ������Ϸ�����
     * \param gameZone ��Ϸ�����
     */
    void setZoneID(const GameZone_t &gameZone)
    {
      this->gameZone = gameZone;
    }

    /**
     * \brief ��ȡ��Ϸ������
     * \return ������Ϸ������
     */
    const std::string &getZoneName() const
    {
      return zoneName;
    }

    /**
     * \brief ������Ϸ������
     * \param zoneName �����õ�����
     */
    void setZoneName(const char *zoneName)
    {
      this->zoneName = zoneName;
    }

    /**
     * \brief ��ȡ���������
     * \return ���������
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief ��ȡ����������
     * \return ����������
     */
    const WORD getType() const
    {
      return wdServerType;
    }

    /**
     * \brief ��ȡip��ַ
     * \return ����ip��ַ
     */
    const char *getIP() const
    {
      return pstrIP;
    }

    /**
     * \brief ��ȡ�˿�
     * \return ���ض˿�
     */
    const WORD getPort() const
    {
      return wdPort;
    }

    /**
     * \brief ָ�����ݿ����ӳ�ʵ����ָ��
     *
     */
    static zDBConnPool *dbConnPool;

  private:

    /**
     * \brief ��Ϸ�����
     */
    GameZone_t gameZone;
    /**
     * \brief ��Ϸ������
     */
    std::string zoneName;

    WORD wdServerID;          /**< ��������ţ�һ����Ψһ�� */
    WORD wdServerType;          /**< ���������ͣ�������ʵ����ʱ���Ѿ�ȷ�� */
    char pstrIP[MAX_IP_LENGTH];      /**< ������������ַ */
    WORD wdPort;            /**< �����������˿� */

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static SuperService *instance;

    zTCPTaskPool *taskPool;        /**< TCP���ӳص�ָ�� */

    /**
     * \brief ���캯��
     *
     */
    SuperService() : zNetService("���������")
    {
      wdServerID = 1;
      wdServerType = SUPERSERVER;
      bzero(pstrIP,sizeof(pstrIP));
      wdPort = 0;
      taskPool = NULL;
    }

    bool init();
    void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
    void final();

    bool getServerInfo();

};

class SuperTimeTick : public zThread
{

  public:

    ~SuperTimeTick() {};

    static SuperTimeTick &getInstance()
    {
      if (NULL == instance)
        instance = new SuperTimeTick();

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

    static zRTime currentTime;
    static SuperTimeTick *instance;

    zRTime startTime;
    QWORD qwStartGameTime;

    SuperTimeTick() : zThread("TimeTick"),startTime()
    {
      qwStartGameTime = 0;
    }

    bool readTime();
    bool saveTime();

};

/**
 * \brief ��������������
 *
 * һ�����е�ÿһ������������Ҫ�͹����������������
 * 
 */
class ServerTask : public zTCPTask
{

  public:

    /**
     * \brief ���캯��
     *
     * ���ڴ���һ����������������
     *
     * \param pool �������ӳ�ָ��
     * \param sock TCP/IP�׽ӿ�
     * \param addr ��ַ
     */
	  	DWORD old;
    ServerTask(
        zTCPTaskPool *pool,
        const SOCKET sock,
        const struct sockaddr_in *addr = NULL) : zTCPTask(pool,sock,addr)
    {
      wdServerID = 0;
      wdServerType = UNKNOWNSERVER;
      bzero(pstrIP,sizeof(pstrIP));
      wdPort = 0;

      OnlineNum = 0;

      sequenceOK = false;
      hasNotifyMe = false;
      hasprocessSequence = false;
    }

    /**
     * \brief ����������
     *
     */
    virtual ~ServerTask();

    int verifyConn();
    int waitSync();
    int recycleConn();
    void addToContainer();
    void removeFromContainer();
    bool uniqueAdd();
    bool uniqueRemove();
    bool msgParse(const Cmd::t_NullCmd *,const DWORD);
    void responseOther(const WORD wdServerID);

    /**
     * \brief ��ȡ���������
     *
     * \return ���������
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief ��ȡ����������
     * \return ����������
     */
    const WORD getType() const
    {
      return wdServerType;
    }

    /**
     * \brief ���ط�������������
     * \return ��������������
     */
    const DWORD getOnlineNum() const
    {
      return OnlineNum;
    }

    /**
     * \brief ������һ�δ�������˳���ʱ��
     *
     * \return ����Ƿ�ɹ�
     */
    bool checkSequenceTime()
    {
      //����˳�����Ѿ�����ˣ�����Ҫ�ٴδ���
      if (sequenceOK)
        return false;

      //������δ���ļ��ʱ��
      zTime currentTime;
      if (lastSequenceTime.elapse(currentTime) > 2)
      {
        lastSequenceTime = currentTime;
        return true;
      }

      return false;
    }

  private:

    WORD wdServerID;          /**< ��������ţ�һ����Ψһ�� */
    WORD wdServerType;          /**< ���������ͣ�������ʵ����ʱ���Ѿ�ȷ�� */
    char pstrIP[MAX_IP_LENGTH];      /**< ������������ַ */
    WORD wdPort;            /**< �����������˿� */

    DWORD      OnlineNum;      /**< ��������ͳ�� */

    zTime lastSequenceTime;        /**< ���һ�δ�������˳���ʱ�� */
    bool sequenceOK;          /**< �Ƿ��Ѿ��������������˳�� */
    bool hasNotifyMe;
    bool hasprocessSequence;

    bool verify(WORD wdType,const char *pstrIP);
    bool verifyTypeOK(const WORD wdType,std::vector<ServerTask *> &sv);
    bool processSequence();
    bool notifyOther();
    bool notifyOther(WORD dstID);
    bool notifyMe();

    bool msgParse_Startup(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool msgParse_Bill(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool msgParse_Gateway(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool msgParse_GmTool(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool msgParse_CountryOnline(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

   /* struct key_hash
    {
      size_t operator()(const Cmd::Super::ServerEntry &x) const
      {
        __gnu_cxx::hash<WORD> H;
        return H(x.wdServerID);
      }
    };
    struct key_equal : public std::binary_function<Cmd::Super::ServerEntry,Cmd::Super::ServerEntry,bool>
    {
      bool operator()(const Cmd::Super::ServerEntry &s1,const Cmd::Super::ServerEntry &s2) const
      {
        return s1.wdServerID == s2.wdServerID;
      }
    };*/


    typedef hash_map<Cmd::Super::ServerEntry,bool/*,key_hash,key_equal*/> Container;
    // ���ӽ����ķ������б�
    Container ses;

	const char* GetServerTypeName(const WORD wdServerType);
};

/**
 * \brief ����������������
 *
 * �����������ȫ��������Ψһ����֤����
 *
 */
class ServerManager : zNoncopyable
{

  public:

    /**
     * \brief ȱʡ��������
     *
     */
    ~ServerManager() {};

    /**
     * \brief ��ȡ���Ψһʵ��
     *
     * �����ʹ����Singleton���ģʽ����֤��һ��������ֻ��һ�����ʵ��
     *
     */
    static ServerManager &getInstance()
    {
      if (NULL == instance)
        instance = new ServerManager();

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

    void addServer(ServerTask *task);
    void removeServer(ServerTask *task);
    ServerTask *getServer(WORD wdServerID);
    bool uniqueAdd(ServerTask *task);
    bool uniqueVerify(const WORD wdServerID);
    bool uniqueRemove(ServerTask *task);
    bool broadcast(const void *pstrCmd,int nCmdLen);
    bool broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen);
    bool broadcastByType(const WORD wdType,const void *pstrCmd,int nCmdLen);
    const DWORD caculateOnlineNum();
    void responseOther(const WORD srcID,const WORD wdServerID);

  private:

    /**
     * \brief �����������������
     *
     */
    typedef std::list<ServerTask *> Container;
    /**
     * \brief ����������������͵ĵ�����
     *
     */
    typedef Container::iterator Container_iterator;
    /**
     * \brief ����������������͵ĳ���������
     *
     */
    typedef Container::const_iterator Containter_const_iterator;
    /**
     * \brief �����˷�������Ψһ����֤��������
     * 
     **/
    typedef hash_map<WORD,ServerTask *> ServerTaskHashmap;
    /**
     * \brief ���������ĵ���������
     *
     */
    typedef ServerTaskHashmap::iterator ServerTaskHashmap_iterator;
    /**
     * \brief �����������ĳ�������������
     *
     */
    typedef ServerTaskHashmap::const_iterator ServerTaskHashmap_const_iterator;
    /**
     * \brief �����������ļ�ֵ������
     *
     */
    typedef ServerTaskHashmap::value_type ServerTaskHashmap_pair;
    /**
     * \brief �������ʵĻ������
     *
     */
    zMutex mutex;
    /**
     * \brief ������ȫ��������ʵ��
     *
     */
    Container container;
    /**
     * \brief Ψһ������ʵ��
     *
     */
    ServerTaskHashmap taskUniqueContainer;

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static ServerManager *instance;

    /**
     * \brief ���캯��
     *
     */
    ServerManager() {};

};

/**
 * \brief ͳһ�û�ƽ̨��½�������Ŀͻ���������
 */
class FLClient : public zTCPClientTask
{

  public:

    FLClient(
        const std::string &ip,
        const WORD port);
    ~FLClient();

    int checkRebound();
    void addToContainer();
    void removeFromContainer();
    bool connect();
    bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief ��ȡ��ʱ���
     * \return ��ʱ���
     */
    const WORD getTempID() const
    {
      return tempid;
    }

    const NetType getNetType() const
    {
      return netType;
    }


  private:

    /**
     * \brief ��ʱ���
     *
     */
    const WORD tempid;
    static WORD tempidAllocator;
    NetType netType;

    bool msgParse_gyList(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool msgParse_session(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

};

/**
 * \brief ͳһ�û�ƽ̨��½�������Ŀͻ��������������
 */
class FLClientManager
{

  public:

    ~FLClientManager();

    /**
     * \brief ��ȡ���Ψһʵ��
     * \return ���Ψһʵ������
     */
    static FLClientManager &getInstance()
    {
      if (NULL == instance)
        instance = new FLClientManager();

      return *instance;
    }

    /**
     * \brief �������Ψһʵ��
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    bool init();
    void timeAction(const zTime &ct);
    void add(FLClient *flClient);
    void remove(FLClient *flClient);
    void broadcast(const void *pstrCmd,int nCmdLen);
    void sendTo(const WORD tempid,const void *pstrCmd,int nCmdLen);

  private:

    FLClientManager();
    static FLClientManager *instance;

    /**
     * \brief �ͻ������ӹ����
     */
    zTCPClientTaskPool *flClientPool;
    /**
     * \brief ���ж�����������ʱ���¼
     */
    zTime actionTimer;

    /**
     * \brief ��������Ѿ��ɹ���������������
     */
    typedef hash_map<WORD,FLClient *> FLClientContainer;
    typedef FLClientContainer::iterator iter;
    typedef FLClientContainer::const_iterator const_iter;
    typedef FLClientContainer::value_type value_type;
    /**
     * \brief ��������Ѿ��ɹ�����������
     */
    FLClientContainer allClients;
    /**
     * \brief �������ʶ�д��
     */
    zRWLock rwlock;

};

class RoleregCache
{

  public:

    struct Data
    {
      WORD wdServerID;      /**< ��������� */
      DWORD accid;        /**< �˺ű�� */
      char name[MAX_NAMESIZE];  /**< ��ɫ���� */
      WORD state;          /**< ����״̬��λ��� */

      Data(const Cmd::Super::t_Charname_Gateway &cmd)
      {
        wdServerID = cmd.wdServerID;
        accid = cmd.accid;
        strncpy(name,cmd.name,sizeof(name));
        state = cmd.state;
      }

      Data(const Data &data)
      {
        wdServerID = data.wdServerID;
        accid = data.accid;
        strncpy(name,data.name,sizeof(name));
        state = data.state;
      }

      Data &operator=(const Data &data)
      {
        wdServerID = data.wdServerID;
        accid = data.accid;
        strncpy(name,data.name,sizeof(name));
        state = data.state;
        return *this;
      }
    };

    ~RoleregCache() {};

    static RoleregCache &getInstance()
    {
      if (NULL == instance)
        instance = new RoleregCache();

      return *instance;
    }

    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    void add(const Cmd::Super::t_Charname_Gateway &cmd);
    void timeAction(const zTime &ct);

    bool msgParse_loginServer(WORD wdServerID,DWORD accid,char name[MAX_NAMESIZE],WORD state);

  private:

    RoleregCache() {};

    static RoleregCache *instance;

    zTime actionTimer;

    typedef std::list<Data> DataCache;
    zMutex mlock;
    DataCache datas;

};

/**
 * \brief ��Ϣ�ռ��ͻ���������
 */
class InfoClient : public zTCPClientTask
{

  public:

    InfoClient(
        const std::string &ip,
        const WORD port);
    ~InfoClient();

    int checkRebound();
    void addToContainer();
    void removeFromContainer();
    bool connect();
    bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    const DWORD getTempID() const
    {
      return tempid;
    }

    const NetType getNetType() const
    {
      return netType;
    }

  private:
    const DWORD tempid;
    static DWORD tempidAllocator;
    NetType netType;

};

/**
 * \brief �����������Ϣ�ɼ����ӵĿͻ��˹�������
 */
class InfoClientManager
{

  public:

    ~InfoClientManager();

    /**
     * \brief ��ȡ���Ψһʵ��
     * \return ���Ψһʵ������
     */
    static InfoClientManager &getInstance()
    {
      if (NULL == instance)
        instance = new InfoClientManager();

      return *instance;
    }

    /**
     * \brief �������Ψһʵ��
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    bool init();
    void timeAction(const zTime &ct);
    void add(InfoClient *infoClient);
    void remove(InfoClient *infoClient);
    bool broadcastOne(const void *pstrCmd,int nCmdLen);
    bool sendTo(const DWORD tempid,const void *pstrCmd,int nCmdLen);

  private:

    InfoClientManager();
    static InfoClientManager *instance;

    /**
     * \brief �ͻ������ӹ����
     */
    zTCPClientTaskPool *infoClientPool;
    /**
     * \brief ���ж�����������ʱ���¼
     */
    zTime actionTimer;

    /**
     * \brief ��������Ѿ��ɹ���������������
     */
    typedef std::map<const DWORD,InfoClient *> InfoClient_map;
    typedef InfoClient_map::iterator iter;
    typedef InfoClient_map::const_iterator const_iter;
    typedef InfoClient_map::value_type value_type;
    /**
     * \brief ��������Ѿ��ɹ�����������
     */
    InfoClient_map allClients;

    struct lt_client
    {
      bool operator()(InfoClient *s1,InfoClient *s2) const
      {
        return s1->getNetType() < s2->getNetType();
      }
    };
    typedef std::multiset<InfoClient *,lt_client> InfoClient_set;
    InfoClient_set setter;

    /**
     * \brief �������ʶ�д��
     */
    zRWLock rwlock;

};
