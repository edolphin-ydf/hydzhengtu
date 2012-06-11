/**
 * \brief 实现管理服务器
 *
 * 对一个区中的所有服务器进行管理
 * 
 */
#include <zebra/srvEngine.h>
#include <set>

enum NetType
{
  NetType_near = 0,//近程路由，电信区连电信服务器，网通区连网通服务器
  NetType_far = 1    //远端路由，电信区连网通服务器，网通区连电信服务器
};


/**
 * \brief 保存服务器之间的依赖关系
 *
 */
extern hash_map<int,std::vector<int> > serverSequence;

/**
 * \brief 初始化服务器之间的依赖关系
  
   gcc 
 *
 */
void initServerSequence();




/**
 * \brief 管理服务器类
 *
 * 派生了基类<code>zNetService</code>
 *
 */
class SuperService : public zNetService
{

  public:

    /**
     * \brief 析构函数
     *
     * 虚函数
     *
     */
    ~SuperService()
    {
      instance = NULL;

      //关闭线程池
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
     * \brief 获取类的唯一实例
     *
     * 使用了Singleton设计模式，保证了一个进程中只有一个类的实例
     *
     * \return 类的唯一实例
     */
    static SuperService &getInstance()
    {
      if (NULL == instance)
        instance = new SuperService();

      return *instance;
    }

    /**
     * \brief 释放类的唯一实例
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    void reloadConfig();

    /**
     * \brief 获取游戏区编号
     * \return 返回游戏区编号
     */
    const GameZone_t &getZoneID() const
    {
      return gameZone;
    }

    /**
     * \brief 设置游戏区编号
     * \param gameZone 游戏区编号
     */
    void setZoneID(const GameZone_t &gameZone)
    {
      this->gameZone = gameZone;
    }

    /**
     * \brief 获取游戏区名称
     * \return 返回游戏区名称
     */
    const std::string &getZoneName() const
    {
      return zoneName;
    }

    /**
     * \brief 设置游戏区名称
     * \param zoneName 待设置的名称
     */
    void setZoneName(const char *zoneName)
    {
      this->zoneName = zoneName;
    }

    /**
     * \brief 获取服务器编号
     * \return 服务器编号
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief 获取服务器类型
     * \return 服务器类型
     */
    const WORD getType() const
    {
      return wdServerType;
    }

    /**
     * \brief 获取ip地址
     * \return 返回ip地址
     */
    const char *getIP() const
    {
      return pstrIP;
    }

    /**
     * \brief 获取端口
     * \return 返回端口
     */
    const WORD getPort() const
    {
      return wdPort;
    }

    /**
     * \brief 指向数据库连接池实例的指针
     *
     */
    static zDBConnPool *dbConnPool;

  private:

    /**
     * \brief 游戏区编号
     */
    GameZone_t gameZone;
    /**
     * \brief 游戏区名称
     */
    std::string zoneName;

    WORD wdServerID;          /**< 服务器编号，一个区唯一的 */
    WORD wdServerType;          /**< 服务器类型，创建类实例的时候已经确定 */
    char pstrIP[MAX_IP_LENGTH];      /**< 服务器内网地址 */
    WORD wdPort;            /**< 服务器内网端口 */

    /**
     * \brief 类的唯一实例指针
     *
     */
    static SuperService *instance;

    zTCPTaskPool *taskPool;        /**< TCP连接池的指针 */

    /**
     * \brief 构造函数
     *
     */
    SuperService() : zNetService("管理服务器")
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
     * \brief 释放类的唯一实例
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
 * \brief 服务器连接任务
 *
 * 一个区中的每一个服务器都需要和管理服务器建立连接
 * 
 */
class ServerTask : public zTCPTask
{

  public:

    /**
     * \brief 构造函数
     *
     * 用于创建一个服务器连接任务
     *
     * \param pool 所属连接池指针
     * \param sock TCP/IP套接口
     * \param addr 地址
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
     * \brief 虚析构函数
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
     * \brief 获取服务器编号
     *
     * \return 服务器编号
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief 获取服务器类型
     * \return 服务器类型
     */
    const WORD getType() const
    {
      return wdServerType;
    }

    /**
     * \brief 返回服务器在线人数
     * \return 服务器在线人数
     */
    const DWORD getOnlineNum() const
    {
      return OnlineNum;
    }

    /**
     * \brief 检查最后一次处理启动顺序的时间
     *
     * \return 检查是否成功
     */
    bool checkSequenceTime()
    {
      //启动顺序处理已经完成了，不需要再次处理
      if (sequenceOK)
        return false;

      //检测两次处理的间隔时间
      zTime currentTime;
      if (lastSequenceTime.elapse(currentTime) > 2)
      {
        lastSequenceTime = currentTime;
        return true;
      }

      return false;
    }

  private:

    WORD wdServerID;          /**< 服务器编号，一个区唯一的 */
    WORD wdServerType;          /**< 服务器类型，创建类实例的时候已经确定 */
    char pstrIP[MAX_IP_LENGTH];      /**< 服务器内网地址 */
    WORD wdPort;            /**< 服务器内网端口 */

    DWORD      OnlineNum;      /**< 在线人数统计 */

    zTime lastSequenceTime;        /**< 最后一次处理启动顺序的时间 */
    bool sequenceOK;          /**< 是否已经处理完成了启动顺序 */
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
    // 连接进来的服务器列表
    Container ses;

	const char* GetServerTypeName(const WORD wdServerType);
};

/**
 * \brief 服务器管理容器类
 *
 * 这个容器包括全局容器和唯一性验证容器
 *
 */
class ServerManager : zNoncopyable
{

  public:

    /**
     * \brief 缺省析构函数
     *
     */
    ~ServerManager() {};

    /**
     * \brief 获取类的唯一实例
     *
     * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
     *
     */
    static ServerManager &getInstance()
    {
      if (NULL == instance)
        instance = new ServerManager();

      return *instance;
    }

    /**
     * \brief 释放类的唯一实例
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
     * \brief 定义服务器容器类型
     *
     */
    typedef std::list<ServerTask *> Container;
    /**
     * \brief 定义服务器容器类型的迭代器
     *
     */
    typedef Container::iterator Container_iterator;
    /**
     * \brief 定义服务器容器类型的常量迭代器
     *
     */
    typedef Container::const_iterator Containter_const_iterator;
    /**
     * \brief 定义了服务器的唯一性验证容器类型
     * 
     **/
    typedef hash_map<WORD,ServerTask *> ServerTaskHashmap;
    /**
     * \brief 定义容器的迭代器类型
     *
     */
    typedef ServerTaskHashmap::iterator ServerTaskHashmap_iterator;
    /**
     * \brief 定义了容器的常量迭代器类型
     *
     */
    typedef ServerTaskHashmap::const_iterator ServerTaskHashmap_const_iterator;
    /**
     * \brief 定义了容器的键值对类型
     *
     */
    typedef ServerTaskHashmap::value_type ServerTaskHashmap_pair;
    /**
     * \brief 容器访问的互斥变量
     *
     */
    zMutex mutex;
    /**
     * \brief 服务器全局容器的实例
     *
     */
    Container container;
    /**
     * \brief 唯一性容器实例
     *
     */
    ServerTaskHashmap taskUniqueContainer;

    /**
     * \brief 类的唯一实例指针
     *
     */
    static ServerManager *instance;

    /**
     * \brief 构造函数
     *
     */
    ServerManager() {};

};

/**
 * \brief 统一用户平台登陆服务器的客户端连接类
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
     * \brief 获取临时编号
     * \return 临时编号
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
     * \brief 临时编号
     *
     */
    const WORD tempid;
    static WORD tempidAllocator;
    NetType netType;

    bool msgParse_gyList(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool msgParse_session(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

};

/**
 * \brief 统一用户平台登陆服务器的客户端连接类管理器
 */
class FLClientManager
{

  public:

    ~FLClientManager();

    /**
     * \brief 获取类的唯一实例
     * \return 类的唯一实例引用
     */
    static FLClientManager &getInstance()
    {
      if (NULL == instance)
        instance = new FLClientManager();

      return *instance;
    }

    /**
     * \brief 销毁类的唯一实例
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
     * \brief 客户端连接管理池
     */
    zTCPClientTaskPool *flClientPool;
    /**
     * \brief 进行断线重连检测的时间记录
     */
    zTime actionTimer;

    /**
     * \brief 存放连接已经成功的连接容器类型
     */
    typedef hash_map<WORD,FLClient *> FLClientContainer;
    typedef FLClientContainer::iterator iter;
    typedef FLClientContainer::const_iterator const_iter;
    typedef FLClientContainer::value_type value_type;
    /**
     * \brief 存放连接已经成功的连接容器
     */
    FLClientContainer allClients;
    /**
     * \brief 容器访问读写锁
     */
    zRWLock rwlock;

};

class RoleregCache
{

  public:

    struct Data
    {
      WORD wdServerID;      /**< 服务器编号 */
      DWORD accid;        /**< 账号编号 */
      char name[MAX_NAMESIZE];  /**< 角色名称 */
      WORD state;          /**< 各种状态的位组合 */

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
 * \brief 信息收集客户端连接类
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
 * \brief 定义服务器信息采集连接的客户端管理容器
 */
class InfoClientManager
{

  public:

    ~InfoClientManager();

    /**
     * \brief 获取类的唯一实例
     * \return 类的唯一实例引用
     */
    static InfoClientManager &getInstance()
    {
      if (NULL == instance)
        instance = new InfoClientManager();

      return *instance;
    }

    /**
     * \brief 销毁类的唯一实例
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
     * \brief 客户端连接管理池
     */
    zTCPClientTaskPool *infoClientPool;
    /**
     * \brief 进行断线重连检测的时间记录
     */
    zTime actionTimer;

    /**
     * \brief 存放连接已经成功的连接容器类型
     */
    typedef std::map<const DWORD,InfoClient *> InfoClient_map;
    typedef InfoClient_map::iterator iter;
    typedef InfoClient_map::const_iterator const_iter;
    typedef InfoClient_map::value_type value_type;
    /**
     * \brief 存放连接已经成功的连接容器
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
     * \brief 容器访问读写锁
     */
    zRWLock rwlock;

};
