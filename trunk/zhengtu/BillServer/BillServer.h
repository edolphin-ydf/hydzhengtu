/**
 * \brief zebra��Ŀ�Ʒѷ�����
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

    /// ��ǰʱ��
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
     * \brief �ͷ����Ψһʵ��
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
 * \brief ����Ʒ�����������
 *
 */
class BillTask : public zTCPTask,public MessageQueue
{

  public:

    /**
     * \brief ���캯��
     *
     * \param pool �������ӳ�ָ��
     * \param sock TCP/IP�׽ӿ�
     * \param addr ��ַ
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
     * \brief ����������
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
     *
     * \return ����������
     */
    const WORD getType() const
    {
      return wdServerType;
    }

    bool sendCmdToUser(DWORD id,const void *pstrCmd,const DWORD nCmdLen);
    bool sendCmdToScene(DWORD id,const void *pstrCmd,const DWORD nCmdLen);

  private:

    /**
     * \brief �������ʻ������
     *
     */
    zMutex mlock;
        
    WORD wdServerID;          /**< ���������,һ����Ψһ�� */
    WORD wdServerType;          /**< ���������� */

    /**
      * \brief ��ѯ���
      *
      *
      */
    //void query_gold(const Cmd::Bill::t_Query_Gold_GateMoney* cmd);

    /**
      * \brief �㿨�һ����
      *
      */
    //void change_gold(const Cmd::Bill::t_Change_Gold_GateMoney* cmd);

    /**
      * \brief ��ҽ���
      *
      */
    //void trade_gold(const Cmd::Bill::t_Trade_Gold_GateMoney* cmd);

    //void trade_log(const Cmd::Bill::t_Trade_Gold_GateMoney* cmd,const char* account,double money);
    bool verifyLogin(const Cmd::Bill::t_LoginBill *ptCmd);
    int recycle_state;
    bool veriry_ok;

};

/**
 * \brief �Ʒѷ����������ӹ�����
 *
 */
class BillTaskManager
{

  public:

    /**
     * \brief ��������
     *
     */
    ~BillTaskManager() {};

    /**
     * \brief ��ȡ�����ӹ�����Ψһʵ��
     *
     * \return ������Ψһʵ��
     */
    static BillTaskManager &getInstance()
    {
      if (NULL == instance)
        instance = new BillTaskManager();

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

    bool uniqueAdd(BillTask *task);
    bool uniqueRemove(BillTask *task);
    bool broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen);
    BillTask *getTaskByID(const WORD wdServerID);
    void execEvery();

  private:

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static BillTaskManager *instance;

    /**
     * \brief ���캯��
     *
     */
    BillTaskManager() {};

    /**
     * \brief ������������
     *
     */
    typedef hash_map<WORD,BillTask *> BillTaskHashmap;
    /**
     * \brief ������������������
     *
     */
    typedef BillTaskHashmap::iterator BillTaskHashmap_iterator;
    /**
     * \brief ����������������������
     *
     */
    typedef BillTaskHashmap::const_iterator BillTaskHashmap_const_iterator;
    /**
     * \brief ����������ֵ������
     *
     */
    typedef BillTaskHashmap::value_type BillTaskHashmap_pair;
    /**
     * \brief �������ʻ������
     *
     */
    zRWLock rwlock;
    /**
     * \brief ����һ��������������е�������
     *
     */
    BillTaskHashmap sessionTaskSet;

};

enum NetType
{
  NetType_near = 0, //����·��,�����������ŷ�����,��ͨ������ͨ������
  NetType_far = 1      //Զ��·��,����������ͨ������,��ͨ�������ŷ�����
};


  /**
   * \brief �Ʒѽ��׻ص�����
   * ��ص������д����ݴ���
   */
struct BillData
{ 
  DWORD  uid;                                        //UID 
  char          tid[Cmd::UserServer::SEQ_MAX_LENGTH +1];    //�������к� 
  int            at;                                          //�������� 
  int            subat;                                      //������
  DWORD  point;                                      //�۷ѵ���  
  int            result;                                      //���׽��,1�ɹ�,0ʧ��
  int           balance;                                    //���
  int           bonus;                                      //����
  int             hadfilled;                                  //������ֵ�ı�־,1=������ֵ,0=û��
  time_t        timeout;                                    //��ʱʱ���
  char          cardid[Cmd::UserServer::CARDID_LENGTH +1];  //��ֵ����
  char          ip[MAX_IP_LENGTH];                           //ip��ַ
  char          remark[Cmd::UserServer::REMARK_LENGTH +1];  //��ע
};

/**
 * \brief �Ʒѻص������ӿڶ���
 * ��д��Щ�ص�������ʱ����Ҫ��֤�Ƿ�����ģʽ,<br>
 * �����Щ��������̫�û���ɶ��������������
 */
struct BillCallback
{
    /**
     * \brief �Ʒѻص��ӿں���
     * �Ʒ�,���������
     * ��Ϸ���б��ػ�ȡ�Ƿ�ɹ�,��д���ݿ��
     * ��ֵ
     * ��ѯ�����Ѽ�¼����Ϣ
     * \param uid �˺ű��
     * \return �Ʒ��Ƿ�ɹ�
     */
    bool (*action)(const BillData *bd);
};

class BillUser :public zEntry 
{
  public:
    enum LoginState
    {
      WAIT_LOGIN,   /**< �ȴ�ͨ����½��֤�Ŀͻ��˵�½���ط����� */
      CONF_LOGIN,   /**< ��½���ط�������֤�Ѿ��ɹ� */
      CONF_LOGOUT, /**< �ȴ��˳� */
      WAIT_LOGIN_TIMEOUT,/**< �ȴ���½��ʱ */
    }
    state;          /**< �Ự״̬ */ 
  private:
    static const int session_timeout_value = 10;
    //DWORD loginTempID;        /**< ��½��ʱ��� */


    DWORD gold;    /**< ������� */
    DWORD money;    /**< �������� */
    DWORD all_in_gold;    /**< �ܳ�ֵ������� */
    DWORD all_in_money;    /**< �ܳ�ֵ�������� */
    DWORD all_out_gold;    /**< ����ȡ������� */
    DWORD all_out_money;    /**< ����ȡ�������� */
    DWORD all_tax_gold;    /**< ��˰�� */
    DWORD all_tax_money;  /**< ��˰�� */ 
    DWORD vip_time;    /**< vip����ʱ�� */
    zTime timestamp;    /**< ʱ��� */


    /// ����
    char   tid[Cmd::UserServer::SEQ_MAX_LENGTH+1];                     /// ������ˮ��

    /// ����
    char password[MAX_PASSWORD+1];
    /// �Ƿ��Ѿ���½
    bool stock_login;
    DWORD goldlistNum;  /// ���˹�Ʊ��������
    DWORD moneylistNum;  /// ���˹�Ʊ������
    char     client_ip[MAX_IP_LENGTH];              //�ͻ�����ip
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
     * \brief �Ự��ʱʱ��
     * ��λ,��
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
    // ������ʷ��¼��
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
    /// ���Ͷ���
    typedef hash_multimap<DWORD,Cmd::ConsignHistoryType> HistoryIndex;

    /// ���Ͷ���
    typedef pair<HistoryIndex::iterator,HistoryIndex::iterator> HistoryRange;
  private:
    HistoryIndex historyGold;
    HistoryIndex historyMoney;
    Cmd::ConsignHistoryType newHistoryGold; 
    Cmd::ConsignHistoryType newHistoryMoney; 
    Timer _one_min;
};

/**
 * \brief ����Ʒѷ�����
 *
 * �����ʹ����Singleton���ģʽ,��֤��һ��������ֻ��һ�����ʵ��
 *
 */
class BillService : public zSubNetService
{

  public:

    bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief ����������
     *
     */
    ~BillService()
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
      {
        return taskPool->getSize();
      }
      else
      {
        return 0;
      }
    }

    /**
     * \brief ����Ψһ����ʵ��
     *
     * \return Ψһ����ʵ��
     */
    static BillService &getInstance()
    {
      if (NULL == instance)
        instance = new BillService();

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
     * \brief ָ�����ݿ����ӳ�ʵ����ָ��
     *
     */
    static zDBConnPool *dbConnPool;

    /**
     * \brief ָ�����ݿ���������ָ��
     *
     */
                static DBMetaData* metaData;    

    /**
     * \brief ָ������־��ָ��
     *
     */
                static zLogger* tradelog;    



  private:

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static BillService *instance;

    zTCPTaskPool *taskPool;        /**< TCP���ӳص�ָ�� */

    /**
     * \brief ���캯��
     *
     */
    BillService() : zSubNetService("�Ʒѷ�����",BILLSERVER)
    {
      taskPool = NULL;
    }

    bool init();
    void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
    void final();
};

#pragma pack(1)
/**
 * \brief �˺���Ϣ
 *
 */
struct BillInfo
{
  DWORD accid;          /**< �˺ű�� */
  DWORD loginTempID;        /**< ��½��ʱ��� */

  WORD wdGatewayID;    /**< ��½�����ر�� */

  double gold;    /**< ������� */
  DWORD vip_time;    /**< vip����ʱ�� */

  enum
  {
    WAIT_LOGIN,   /**< �ȴ�ͨ����½��֤�Ŀͻ��˵�½���ط����� */
    CONF_LOGIN      /**< ��½���ط�������֤�Ѿ��ɹ� */
  }
  state;          /**< �Ự״̬ */
  zTime timestamp;    /**< ʱ��� */
  char client_ip[MAX_IP_LENGTH];

  /**
   * \brief ȱʡ���캯��
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
   * \brief �������캯��
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
   * \brief ��ֵ��������
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
 * \brief �˺���Ϣ��������
 *
 * ������һ���������������˺��б�
 *
 */
class BillManager
{

  public:

    /**
     * \brief �Ự��ʱʱ��
     * ��λ,��
     */
    static const int session_timeout_value = 10;

    /**
     * \brief ȱʡ��������
     *
     */
    ~BillManager() {};

    /**
     * \brief �������Ψһʵ��
     *
     * \return ���Ψһʵ��
     */
    static BillManager &getInstance()
    {
      if (NULL == instance)
        instance = new BillManager();

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
     * \brief ���Ψһʵ��ָ��
     *
     */
    static BillManager *instance;

    /**
     * \brief ȱʡ���캯��
     *
     */
    BillManager() {};

    /**
     * \brief �˺Ź�����������
     *
     */
    typedef hash_map<DWORD,BillInfo> BillInfoMap;
    /**
     * \brief ���������ĵ���������
     *
     */
    typedef BillInfoMap::iterator BillInfoMap_iterator;
    /**
     * \brief ���������ļ�ֵ������
     *
     */
    typedef BillInfoMap::value_type BillInfoMap_pair;

    /**
     * \brief �������ʻ������
     *
     */
    zMutex mlock;
    /**
     * \brief �˺Ź�������
     *
     */
    BillInfoMap infoMap;

};

/**
 * \brief �Ʒѿͻ�������
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
 * \brief ͳһ�û�ƽ̨��½�������ļƷѿͻ��������������
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
     * \brief ���׼�¼��log
     */
    static zLogger *tlogger;

    /**
     * \brief ��ֵ������Դ,�������ļ���ȡ��
     */
    static int  source;

  private:

    friend class SingletonBase<BillClientManager>;
    BillClientManager();
    ~BillClientManager();

    BillCallback bc;

    /**
     * \brief �ͻ������ӹ����
     */
    zTCPClientTaskPool *billClientPool;
    /**
     * \brief ���ж�����������ʱ���¼
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
     * \brief ��������Ѿ��ɹ���������������
     */
    typedef std::multiset<BillClient *,lt_client> BillClientContainer;
    typedef BillClientContainer::iterator iter;
    typedef BillClientContainer::const_iterator const_iter;
    typedef BillClientContainer::size_type size_type;
    /**
     * \brief ��������Ѿ��ɹ�����������
     */
    BillClientContainer allClients;
    /**
     * \brief �������ʻ������
     */
    zRWLock rwlock;

};

/**
 * \brief ��ʼ���Ʒѿͻ���ģ��
 * \param confile �Ʒѷ������б��ļ���
 * \param tradelog ������־�ļ���
 * \param bc ��ʼ�Ļص�����
 * \return ��ʼ���Ƿ�ɹ�
 */
bool Bill_init(const std::string &confile,const std::string &tradelog,struct BillCallback *bc);

/**
 * \brief �����ͷżƷѿͻ���ģ�����Դ
 * �ر�����,�ͷ��̵߳Ȳ���
 */
void Bill_final();

/**
 * \brief ʱ��ص�����
 * ��Ҫ����ͻ������Ӷ�������
 */
void Bill_timeAction();

/**
 * \brief �Ʒ�
 * ���������,��ֵ,��ѯ��
 * \param uid �˺ű��
 * \return �Ʒ��Ƿ�ɹ�,�������true,��bd->tid����ν��׵�Ψһ���к�
 */
bool Bill_action(BillData *bd);


/**
 * \brief ȱʡ�Ʒѳ�ʱʱ��
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
 * \brief ��ɫ������ȡд��ĻỰ��¼
 *
 */
struct BillSession
{
  DWORD  accid;      /// �ʺű��
  DWORD  charid;      /// ��ɫ���
  char   tid[Cmd::UserServer::SEQ_MAX_LENGTH+1];                     /// ������ˮ��
  char   account[Cmd::UserServer::ID_MAX_LENGTH+1];     /// �ʺ�(��Ҫ���͸��Ƿѷ�����)
  char   name[MAX_NAMESIZE+1];     /// ��ɫ����
  DWORD point;                    /// �۷ѵ���


  /**
   * \brief ȱʡ���캯��
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
     * \brief Ĭ����������
     *
     */
    ~BillSessionManager()
    {
      sessionMap.clear();
    }

    /**
     * \brief �������Ψһʵ��
     *
     * ʵ����Singleton���ģʽ,��֤��һ��������ֻ��һ�����ʵ��
     *
     */
    static BillSessionManager &getInstance()
    {
      if (NULL == instance)
        instance = new BillSessionManager;

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

    bool add(BillSession &bs);
//    bool verify(const DWORD accid,const DWORD id,const WORD wdServerID);
    bool remove(const std::string& tid);
//  BillSession find(const std::string& tid);
    BillSession get(const std::string& tid);
//    void removeAllByServerID(const WORD wdServerID);

  private:

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static BillSessionManager *instance;

    /**
     * \brief Ĭ�Ϲ��캯��
     *
     */
    BillSessionManager() {};

    /**
     * \brief ������������
     *
     */
    typedef std::map<std::string,BillSession> BillSessionHashmap;
    /**
     * \brief ������������������
     *
     */
    typedef BillSessionHashmap::iterator BillSessionHashmap_iterator;
    /**
     * \brief ����������ֵ������
     *
     */
    typedef BillSessionHashmap::value_type BillSessionHashmap_pair;
    
    /**
     * \brief �洢�����ʺ��б���Ϣ������
     *
     */
    BillSessionHashmap sessionMap;
    
    /**
     * \brief �������
     *
     */
    zMutex mlock;
};
