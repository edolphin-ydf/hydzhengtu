/**
 * \brief zebra项目计费服务器
 *
 */
#include <zebra/srvEngine.h>
#include <set>

/*template <typename T>
class SingletonBase
{
  public:
    SingletonBase() {}
    virtual ~SingletonBase() {}
    static T& getInstance()
    {
      assert(instance);
      return *instance;
    }
    static void newInstance()
    {
      SAFE_DELETE(instance);
      instance = new T();
    }
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }
  protected:
    static T* instance;
  private:
    SingletonBase(const SingletonBase&);
    SingletonBase & operator= (const SingletonBase &);
};
template <typename T> T* SingletonBase<T>::instance = NULL;*/

class BillTimeTick : public zThread
{

  public:

    ~BillTimeTick() {};

    /// 当前时间
    static zRTime currentTime;
    static Timer _one_min;
    static Timer _one_sec;
    static BillTimeTick &getInstance()
    {
      if (NULL == instance)
        instance = new BillTimeTick();

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

    static BillTimeTick *instance;

    BillTimeTick() : zThread("TimeTick") {};

};

/**
 * \brief 定义计费连接任务类
 *
 */
class BillTask : public zTCPTask,public MessageQueue
{

  public:

    /**
     * \brief 构造函数
     *
     * \param pool 所属连接池指针
     * \param sock TCP/IP套接口
     * \param addr 地址
     */
    BillTask(
        zTCPTaskPool *pool,
        const SOCKET sock,
        const struct sockaddr_in *addr = NULL) : zTCPTask(pool,sock,addr)
    {
      wdServerID = 0;
      wdServerType = UNKNOWNSERVER;
      recycle_state=0;
      veriry_ok=false; 
    }

    /**
     * \brief 虚析构函数
     *
     */
    ~BillTask() {};

    int verifyConn();
    int recycleConn();
    bool uniqueAdd();
    bool uniqueRemove();
    bool msgParse(const Cmd::t_NullCmd *,const DWORD);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);
    bool checkRecycle();

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
     *
     * \return 服务器类型
     */
    const WORD getType() const
    {
      return wdServerType;
    }

    bool sendCmdToUser(DWORD id,const void *pstrCmd,const DWORD nCmdLen);
    bool sendCmdToScene(DWORD id,const void *pstrCmd,const DWORD nCmdLen);

  private:

    /**
     * \brief 容器访问互斥变量
     *
     */
    zMutex mlock;
        
    WORD wdServerID;          /**< 服务器编号,一个区唯一的 */
    WORD wdServerType;          /**< 服务器类型 */

    /**
      * \brief 查询金币
      *
      *
      */
    //void query_gold(const Cmd::Bill::t_Query_Gold_GateMoney* cmd);

    /**
      * \brief 点卡兑换金币
      *
      */
    //void change_gold(const Cmd::Bill::t_Change_Gold_GateMoney* cmd);

    /**
      * \brief 金币交易
      *
      */
    //void trade_gold(const Cmd::Bill::t_Trade_Gold_GateMoney* cmd);

    //void trade_log(const Cmd::Bill::t_Trade_Gold_GateMoney* cmd,const char* account,double money);
    bool verifyLogin(const Cmd::Bill::t_LoginBill *ptCmd);
    int recycle_state;
    bool veriry_ok;

};

/**
 * \brief 计费服务器子连接管理器
 *
 */
class BillTaskManager
{

  public:

    /**
     * \brief 析构函数
     *
     */
    ~BillTaskManager() {};

    /**
     * \brief 获取子连接管理器唯一实例
     *
     * \return 子连接唯一实例
     */
    static BillTaskManager &getInstance()
    {
      if (NULL == instance)
        instance = new BillTaskManager();

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

    bool uniqueAdd(BillTask *task);
    bool uniqueRemove(BillTask *task);
    bool broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen);
    BillTask *getTaskByID(const WORD wdServerID);
    void execEvery();

  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static BillTaskManager *instance;

    /**
     * \brief 构造函数
     *
     */
    BillTaskManager() {};

    /**
     * \brief 定义容器类型
     *
     */
    typedef hash_map<WORD,BillTask *> BillTaskHashmap;
    /**
     * \brief 定义容器迭代器类型
     *
     */
    typedef BillTaskHashmap::iterator BillTaskHashmap_iterator;
    /**
     * \brief 定义容器常量迭代器类型
     *
     */
    typedef BillTaskHashmap::const_iterator BillTaskHashmap_const_iterator;
    /**
     * \brief 定义容器键值对类型
     *
     */
    typedef BillTaskHashmap::value_type BillTaskHashmap_pair;
    /**
     * \brief 容器访问互斥变量
     *
     */
    zRWLock rwlock;
    /**
     * \brief 声明一个容器，存放所有的子连接
     *
     */
    BillTaskHashmap sessionTaskSet;

};

enum NetType
{
  NetType_near = 0, //近程路由,电信区连电信服务器,网通区连网通服务器
  NetType_far = 1      //远端路由,电信区连网通服务器,网通区连电信服务器
};


  /**
   * \brief 计费交易回调数据
   * 向回调函数中传数据处理
   */
struct BillData
{ 
  DWORD  uid;                                        //UID 
  char          tid[Cmd::UserServer::SEQ_MAX_LENGTH +1];    //交易序列号 
  int            at;                                          //交易类型 
  int            subat;                                      //子类型
  DWORD  point;                                      //扣费点数  
  int            result;                                      //交易结果,1成功,0失败
  int           balance;                                    //余额
  int           bonus;                                      //积分
  int             hadfilled;                                  //曾经充值的标志,1=曾经充值,0=没有
  time_t        timeout;                                    //超时时间戳
  char          cardid[Cmd::UserServer::CARDID_LENGTH +1];  //充值卡号
  char          ip[MAX_IP_LENGTH];                           //ip地址
  char          remark[Cmd::UserServer::REMARK_LENGTH +1];  //备注
};

/**
 * \brief 计费回调函数接口定义
 * 在写这些回调函数的时候需要保证是非阻塞模式,<br>
 * 如果这些函数组塞太久会造成对其他服务的组塞
 */
struct BillCallback
{
    /**
     * \brief 计费回调接口函数
     * 计费,点数换金币
     * 游戏区中本地换取是否成功,如写数据库等
     * 冲值
     * 查询余额、消费纪录等信息
     * \param uid 账号编号
     * \return 计费是否成功
     */
    bool (*action)(const BillData *bd);
};

class BillUser :public zEntry 
{
  public:
    enum LoginState
    {
      WAIT_LOGIN,   /**< 等待通过登陆验证的客户端登陆网关服务器 */
      CONF_LOGIN,   /**< 登陆网关服务器验证已经成功 */
      CONF_LOGOUT, /**< 等待退出 */
      WAIT_LOGIN_TIMEOUT,/**< 等待登陆超时 */
    }
    state;          /**< 会话状态 */ 
  private:
    static const int session_timeout_value = 10;
    //DWORD loginTempID;        /**< 登陆临时编号 */


    DWORD gold;    /**< 金币数量 */
    DWORD money;    /**< 银币数量 */
    DWORD all_in_gold;    /**< 总冲值金币数量 */
    DWORD all_in_money;    /**< 总冲值银币数量 */
    DWORD all_out_gold;    /**< 总提取金币数量 */
    DWORD all_out_money;    /**< 总提取银币数量 */
    DWORD all_tax_gold;    /**< 总税收 */
    DWORD all_tax_money;  /**< 总税收 */ 
    DWORD vip_time;    /**< vip到期时间 */
    zTime timestamp;    /**< 时间戳 */


    /// 交易
    char   tid[Cmd::UserServer::SEQ_MAX_LENGTH+1];                     /// 交易流水号

    /// 密码
    char password[MAX_PASSWORD+1];
    /// 是否已经登陆
    bool stock_login;
    DWORD goldlistNum;  /// 个人股票卖单数量
    DWORD moneylistNum;  /// 个人股票买单数量
    char     client_ip[MAX_IP_LENGTH];              //客户请求ip
  public:
    const char *getIp();
    char account[Cmd::UserServer::ID_MAX_LENGTH+1];
    BillTask *gatewaytask;
    BillUser(DWORD acc,DWORD logintemp,const char *count,const char *ip,BillTask *gate);
    void increaseGoldListNum();
    void increaseMoneyListNum();
    void decreaseGoldListNum();
    void decreaseMoneyListNum();


    bool sendCmd(const void *pstrCmd,const int nCmdLen)
    {
      if (gatewaytask)
      {
        return gatewaytask->sendCmd(pstrCmd,nCmdLen);
      }
      return false;
    }
    bool sendCmdToMe(const void *pstrCmd,const int nCmdLen)
    {
      if (gatewaytask)
      {
        return gatewaytask->sendCmdToUser(id,pstrCmd,nCmdLen);
      }
      return false;
    }
    bool sendCmdToScene(const void *pstrCmd,const int nCmdLen)
    {
      if (gatewaytask)
      {
        return gatewaytask->sendCmdToScene(id,pstrCmd,nCmdLen);
      }
      return false;
    }

    bool restoregold();
    bool restorecard();
    static bool redeem_gold_err(const BillData* bd);
    static bool redeem_object_card_err(const BillData* bd);
    bool login(const DWORD loginTempID);
    bool logout(const DWORD loginTempID);
    bool query_point(const BillData* bd);
    bool redeem_gold(const BillData* bd);
    bool redeem_moth_card(const BillData* bd);
    bool redeem_object_card(const BillData* bd);
    bool begin_tid(const char *t);
    bool putList(DWORD num,DWORD price,BYTE type);
    bool addGold(DWORD num,const char *disc,bool transfer=false,bool tax=false);
    bool addMoney(DWORD num,const char *disc,bool transfer=false,bool tax=false);
    bool removeGold(DWORD num,const char *disc,bool transfer=false,bool tax=false);
    bool removeMoney(DWORD num,const char *disc,bool transfer=false,bool tax=false);
    static DWORD getRealMinTime();
    static void logger(const char *coin_type,DWORD acc,const char *act,DWORD cur,DWORD change,DWORD type,const char *action);
    void end_tid();
    DWORD loginTimeOut(zTime current);
    bool usermsgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool usermsgParseScene(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
  private:
    bool check_tid(const char *t);
    bool checkStockLogin();
};

class BillUserManager :public zEntryManager< zEntryID>
{
  private:
    zRWLock rwlock;

  private:
    BillUserManager();
    ~BillUserManager();
    static BillUserManager *instance;
  public:
    static BillUserManager *getInstance();
    static void delInstance();
    bool getUniqeID(DWORD &tempid);
    void putUniqeID(const DWORD &tempid);
    BillUser * getUserByID( DWORD id)
    {
      rwlock.rdlock();
      BillUser *ret =(BillUser *)getEntryByID(id);
      rwlock.unlock();
      return ret;
    }
    /*
    BillUser * getUserByTempID( DWORD tempid)
    {
      rwlock.rdlock();
      BillUser *ret =(BillUser *)getEntryByTempID(tempid);
      rwlock.unlock();
      return ret;
    }
    // */
    bool addUser(BillUser *user)
    {
      rwlock.wrlock();
      bool ret =addEntry((zEntry *)user);
      rwlock.unlock();
      return ret;
    }
    void removeUser(BillUser *user)
    {
      rwlock.wrlock();
      removeEntry((zEntry *)user);
      rwlock.unlock();
    }
    template <class YourUserEntry>
      bool execEveryUser(execEntry<YourUserEntry> &exec)
      {
        rwlock.rdlock();
        bool ret=execEveryEntry<>(exec);
        rwlock.unlock();
        return ret;
      }



  public:
    /**
     * \brief 会话超时时间
     * 单位,秒
     */
    void update();
    void removeUserByGatewayID(BillTask *task);
    //bool updateGold(DWORD acc,double gold);
    //bool updateVipTime(DWORD acc,DWORD vip);
    //DWORD getVipTime(DWORD acc);
    //double getGold(DWORD acc);
    //BillInfo get(const DWORD accid);


};

struct ConsignTrait
{
  DWORD id;
  DWORD accid;
  DWORD num;
  DWORD price;
  DWORD time;
};

class Consign
{
  public:
    Consign()
    {
      bzero(Buf,sizeof(Buf));
      firstfive=(Cmd::stFirstFiveListStockUserCmd *)Buf;
      constructInPlace(firstfive);
    }
    virtual ~Consign()
    {
    }
    virtual bool init()=0;
    bool sendFirstFiveToUser(BillUser *pUser);
    bool sendWaitDataToUser(BillUser *pUser);
    static bool cancelListAll();
    static bool addGold(DWORD accid,DWORD num);
    static bool addMoney(DWORD accid,DWORD num);
     
  protected:
    // 更新历史记录表
    bool updateHistory(DWORD id,DWORD acc,DWORD num,DWORD commitprice,DWORD price,DWORD comtime,bool type,DWORD sysmoney);
    //PriceIndex priceindex;
    char Buf[1024];
    Cmd::stFirstFiveListStockUserCmd *firstfive;
};
class ConsignGoldManager :public Consign
{
  private:
    ConsignGoldManager();
    ~ConsignGoldManager();
    static ConsignGoldManager *instance;
  public:
    static ConsignGoldManager *getInstance();
    static void delInstance();
    bool init();
    bool trade();
    bool trade(ConsignTrait &goldlist,DWORD &sysmoney);
    bool cancelList(BillUser *pUser,DWORD lsitid);
  public:
};
class ConsignMoneyManager :public Consign
{
  private:
    ConsignMoneyManager();
    ~ConsignMoneyManager();
    static ConsignMoneyManager *instance;
  public:
    static ConsignMoneyManager *getInstance();
    static void delInstance();
    bool init();
    bool trade();
    bool trade(ConsignTrait &goldlist,DWORD &sysmoney);
    bool cancelList(BillUser *pUser,DWORD lsitid);
  public:
};

class ConsignHistoryManager
{
  private:
    ConsignHistoryManager();
    ~ConsignHistoryManager();
    static ConsignHistoryManager *instance;
  public:
    static ConsignHistoryManager *getInstance();
    static void delInstance();
    bool init();
    bool update();
    bool sendDataToUser(BillUser *pUser,DWORD begintime,DWORD num=0);
    bool sendSelfDataToUser(BillUser *pUser,DWORD begintime,DWORD num);
  public:
    /// 类型定义
    typedef hash_multimap<DWORD,Cmd::ConsignHistoryType> HistoryIndex;

    /// 类型定义
    typedef pair<HistoryIndex::iterator,HistoryIndex::iterator> HistoryRange;
  private:
    HistoryIndex historyGold;
    HistoryIndex historyMoney;
    Cmd::ConsignHistoryType newHistoryGold; 
    Cmd::ConsignHistoryType newHistoryMoney; 
    Timer _one_min;
};

/**
 * \brief 定义计费服务类
 *
 * 这个类使用了Singleton设计模式,保证了一个进程中只有一个类的实例
 *
 */
class BillService : public zSubNetService
{

  public:

    bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief 虚析构函数
     *
     */
    ~BillService()
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
      {
        return taskPool->getSize();
      }
      else
      {
        return 0;
      }
    }

    /**
     * \brief 返回唯一的类实例
     *
     * \return 唯一的类实例
     */
    static BillService &getInstance()
    {
      if (NULL == instance)
        instance = new BillService();

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
     * \brief 指向数据库连接池实例的指针
     *
     */
    static zDBConnPool *dbConnPool;

    /**
     * \brief 指向数据库表管理器的指针
     *
     */
                static DBMetaData* metaData;    

    /**
     * \brief 指向交易日志的指针
     *
     */
                static zLogger* tradelog;    



  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static BillService *instance;

    zTCPTaskPool *taskPool;        /**< TCP连接池的指针 */

    /**
     * \brief 构造函数
     *
     */
    BillService() : zSubNetService("计费服务器",BILLSERVER)
    {
      taskPool = NULL;
    }

    bool init();
    void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
    void final();
};

#pragma pack(1)
/**
 * \brief 账号信息
 *
 */
struct BillInfo
{
  DWORD accid;          /**< 账号编号 */
  DWORD loginTempID;        /**< 登陆临时编号 */

  WORD wdGatewayID;    /**< 登陆的网关编号 */

  double gold;    /**< 金币数量 */
  DWORD vip_time;    /**< vip到期时间 */

  enum
  {
    WAIT_LOGIN,   /**< 等待通过登陆验证的客户端登陆网关服务器 */
    CONF_LOGIN      /**< 登陆网关服务器验证已经成功 */
  }
  state;          /**< 会话状态 */
  zTime timestamp;    /**< 时间戳 */
  char client_ip[MAX_IP_LENGTH];

  /**
   * \brief 缺省构造函数
   *
   */
  BillInfo() : timestamp()
  {
    accid = 0;
    loginTempID = 0;
    wdGatewayID = 0;
    state = WAIT_LOGIN;
    bzero(client_ip,sizeof(client_ip));
  }
  
  /**
   * \brief 拷贝构造函数
   *
   */
  BillInfo(const BillInfo &ai)
  {
    accid = ai.accid;
    loginTempID = ai.loginTempID;
    wdGatewayID = ai.wdGatewayID;
    state = ai.state;
    gold = 0;
    vip_time = 0;
    timestamp = ai.timestamp;
    strncpy(client_ip,ai.client_ip,sizeof(client_ip));
  }

  /**
   * \brief 赋值操作符号
   *
   */
  BillInfo & operator= (const BillInfo &ai)
  {
    accid = ai.accid;
    loginTempID = ai.loginTempID;
    wdGatewayID = ai.wdGatewayID;
    state = ai.state;
    timestamp = ai.timestamp;
    strncpy(client_ip,ai.client_ip,sizeof(client_ip));

    return *this;
  }

};
#pragma pack()

/**
 * \brief 账号信息管理容器
 *
 * 保留了一个区中所有在线账号列表
 *
 */
class BillManager
{

  public:

    /**
     * \brief 会话超时时间
     * 单位,秒
     */
    static const int session_timeout_value = 10;

    /**
     * \brief 缺省析构函数
     *
     */
    ~BillManager() {};

    /**
     * \brief 返回类的唯一实例
     *
     * \return 类的唯一实例
     */
    static BillManager &getInstance()
    {
      if (NULL == instance)
        instance = new BillManager();

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

    bool verify(const t_NewLoginSession &session);
    void update();
    void updateByGatewayID(const WORD wdGatewayID);
    bool login(const DWORD accid,const DWORD loginTempID);
    bool logout(const DWORD accid,const DWORD loginTempID);
    bool updateGold(DWORD acc,double gold);
    bool updateVipTime(DWORD acc,DWORD vip);
    DWORD getVipTime(DWORD acc);
    double getGold(DWORD acc);
    BillInfo get(const DWORD accid);

  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static BillManager *instance;

    /**
     * \brief 缺省构造函数
     *
     */
    BillManager() {};

    /**
     * \brief 账号管理容器类型
     *
     */
    typedef hash_map<DWORD,BillInfo> BillInfoMap;
    /**
     * \brief 定义容器的迭代器类型
     *
     */
    typedef BillInfoMap::iterator BillInfoMap_iterator;
    /**
     * \brief 定义容器的键值对类型
     *
     */
    typedef BillInfoMap::value_type BillInfoMap_pair;

    /**
     * \brief 容器访问互斥变量
     *
     */
    zMutex mlock;
    /**
     * \brief 账号管理容器
     *
     */
    BillInfoMap infoMap;

};

/**
 * \brief 计费客户端连接
 */
class BillClient : public zTCPClientTask,public MessageQueue
{
  public:

    BillClient(const std::string &ip,const WORD port,BillCallback &bc,const DWORD my_id);
    ~BillClient();

    int checkRebound();
    void addToContainer();
    void removeFromContainer();
    bool connect();
    bool action(BillData *bd);
    bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);

    const NetType getNetType() const
    {
      return netType;
    }

    const DWORD getID() const
    {
      return my_id;
    }

  private:

    BillCallback &bc;
    NetType netType;
    DWORD my_id;
    GameZone_t gameZone;
    char gameZone_str[6];

};

/**
 * \brief 统一用户平台登陆服务器的计费客户端连接类管理器
 */
class BillClientManager : public SingletonBase<BillClientManager>
{

  public:

    bool init(const std::string &confile,const std::string &tradelog,BillCallback &bc);
    void timeAction(const zTime &ct);
    void add(BillClient *billClient);
    void remove(BillClient *billClient);

    bool action(BillData *bd);
    void execEvery();

    /**
     * \brief 交易记录的log
     */
    static zLogger *tlogger;

    /**
     * \brief 充值消费来源,从配置文件里取得
     */
    static int  source;

  private:

    friend class SingletonBase<BillClientManager>;
    BillClientManager();
    ~BillClientManager();

    BillCallback bc;

    /**
     * \brief 客户端连接管理池
     */
    zTCPClientTaskPool *billClientPool;
    /**
     * \brief 进行断线重连检测的时间记录
     */
    zTime actionTimer;
    DWORD maxID;

    struct lt_client
    {
      bool operator()(BillClient *s1,BillClient *s2) const
      {
        if (s1->getID() == s2->getID())
          return s1->getNetType() < s2->getNetType();
        else
          return s1->getID() < s2->getID();
      }
    };
    /**
     * \brief 存放连接已经成功的连接容器类型
     */
    typedef std::multiset<BillClient *,lt_client> BillClientContainer;
    typedef BillClientContainer::iterator iter;
    typedef BillClientContainer::const_iterator const_iter;
    typedef BillClientContainer::size_type size_type;
    /**
     * \brief 存放连接已经成功的连接容器
     */
    BillClientContainer allClients;
    /**
     * \brief 容器访问互斥变量
     */
    zRWLock rwlock;

};

/**
 * \brief 初始化计费客户端模块
 * \param confile 计费服务器列表文件名
 * \param tradelog 交易日志文件名
 * \param bc 初始的回调函数
 * \return 初始化是否成功
 */
bool Bill_init(const std::string &confile,const std::string &tradelog,struct BillCallback *bc);

/**
 * \brief 回收释放计费客户端模块的资源
 * 关闭连接,释放线程等操作
 */
void Bill_final();

/**
 * \brief 时间回调函数
 * 主要处理客户端连接断线重连
 */
void Bill_timeAction();

/**
 * \brief 计费
 * 点数换金币,冲值,查询等
 * \param uid 账号编号
 * \return 计费是否成功,如果返回true,则bd->tid是这次交易的唯一序列号
 */
bool Bill_action(BillData *bd);


/**
 * \brief 缺省计费超时时间
 */
#define DEFAULT_BILL_TIMEOUT 60

class BillCache : public SingletonBase<BillCache>
{

  public:

    BillData *add(BillData *bd,const char *gameZone_str);
    BillData *get(const char *tid);
    void remove(const char *tid);
    void update(const zTime &ct);

  private:

    friend class SingletonBase<BillCache>;
    BillCache();
    ~BillCache();

    DWORD SerialNumber;

    struct eqstr
    {
      bool operator()(const char* s1,const char* s2) const
      {
        return strcmp(s1,s2) == 0;
      }
    };

    typedef hash_map<const char *,BillData *> CacheContainer;
    typedef CacheContainer::iterator iter;
    typedef CacheContainer::const_iterator const_iter;
    typedef CacheContainer::value_type value_type;
    CacheContainer cache;
    zMutex mlock;

};

/**
 * \brief 角色档案读取写入的会话记录
 *
 */
struct BillSession
{
  DWORD  accid;      /// 帐号编号
  DWORD  charid;      /// 角色编号
  char   tid[Cmd::UserServer::SEQ_MAX_LENGTH+1];                     /// 交易流水号
  char   account[Cmd::UserServer::ID_MAX_LENGTH+1];     /// 帐号(需要发送给记费服务器)
  char   name[MAX_NAMESIZE+1];     /// 角色名称
  DWORD point;                    /// 扣费点数


  /**
   * \brief 缺省构造函数
   *
   */
  /*
  BillSession(const char* tid,const Cmd::Bill::t_Change_Gold_GateMoney* cmd,BillTask* task = NULL)
  {
    strncpy(this->tid,tid,Cmd::UserServer::SEQ_MAX_LENGTH);
    strncpy(this->account,cmd->account,Cmd::UserServer::ID_MAX_LENGTH);
    this->accid = cmd->accid;
    this->charid = cmd->charid;
    this->point = cmd->point;
    this->task = task;
//    strncpy(this->name,cmd->name,MAX_NAMESIZE);
  }
  // */
  
  BillSession()
  {
    accid = 0;
    charid = 0;
    tid[0] = '\0';
    account[0]  = '\0';
    name[0] = '\0';
    point = 0;
  }

};

class BillSessionManager
{

  public:

    /**
     * \brief 默认析构函数
     *
     */
    ~BillSessionManager()
    {
      sessionMap.clear();
    }

    /**
     * \brief 返回类的唯一实例
     *
     * 实现了Singleton设计模式,保证了一个进程中只有一个类的实例
     *
     */
    static BillSessionManager &getInstance()
    {
      if (NULL == instance)
        instance = new BillSessionManager;

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

    bool add(BillSession &bs);
//    bool verify(const DWORD accid,const DWORD id,const WORD wdServerID);
    bool remove(const std::string& tid);
//  BillSession find(const std::string& tid);
    BillSession get(const std::string& tid);
//    void removeAllByServerID(const WORD wdServerID);

  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static BillSessionManager *instance;

    /**
     * \brief 默认构造函数
     *
     */
    BillSessionManager() {};

    /**
     * \brief 定义容器类型
     *
     */
    typedef std::map<std::string,BillSession> BillSessionHashmap;
    /**
     * \brief 定义容器迭代器类型
     *
     */
    typedef BillSessionHashmap::iterator BillSessionHashmap_iterator;
    /**
     * \brief 定义容器键值对类型
     *
     */
    typedef BillSessionHashmap::value_type BillSessionHashmap_pair;
    
    /**
     * \brief 存储在线帐号列表信息的容器
     *
     */
    BillSessionHashmap sessionMap;
    
    /**
     * \brief 互斥变量
     *
     */
    zMutex mlock;
};
