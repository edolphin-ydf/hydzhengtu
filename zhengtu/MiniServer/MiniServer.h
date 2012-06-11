/**
 * \brief zebra项目计费服务器
 *
 */
#include <zebra/srvEngine.h>
#include <set>

class zDBConnPool;
class DBMetaData;

class MiniRoom;
class MiniGame;
class MiniUser;

class Top100;

/**
 * \brief 定义计费连接任务类
 *
 */
class MiniTask : public zTCPTask,public MessageQueue
{

  public:

    /**
     * \brief 构造函数
     *
     * \param pool 所属连接池指针
     * \param sock TCP/IP套接口
     * \param addr 地址
     */
    MiniTask(
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
    ~MiniTask() {};

    int verifyConn();
    int recycleConn();
    bool uniqueAdd();
    bool uniqueRemove();
    bool msgParse(const Cmd::t_NullCmd *,const DWORD);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);
    bool parseGateMsg(const Cmd::t_NullCmd *,const DWORD);
    bool parseForwardMsg(const Cmd::t_NullCmd *,const DWORD);
    bool parseSceneMsg(const Cmd::t_NullCmd *,const DWORD);
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

    bool verifyLogin(const Cmd::Mini::t_LoginMini *ptCmd);
    int recycle_state;
    bool veriry_ok;

    bool addDBMoney(DWORD userID,DWORD num);
};

/**
 * \brief 计费服务器子连接管理器
 *
 */
class MiniTaskManager
{

  public:

    /**
     * \brief 析构函数
     *
     */
    ~MiniTaskManager() {};

    /**
     * \brief 获取子连接管理器唯一实例
     *
     * \return 子连接唯一实例
     */
    static MiniTaskManager &getInstance()
    {
      if (NULL == instance)
        instance = new MiniTaskManager();

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

    bool uniqueAdd(MiniTask *task);
    bool uniqueRemove(MiniTask *task);
    bool broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen);
    MiniTask *getTaskByID(const WORD wdServerID);
    void execEvery();

  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static MiniTaskManager *instance;

    /**
     * \brief 构造函数
     *
     */
    MiniTaskManager() {};

    /**
     * \brief 定义容器类型
     *
     */
    typedef hash_map<WORD,MiniTask *> MiniTaskHashmap;
    /**
     * \brief 定义容器迭代器类型
     *
     */
    typedef MiniTaskHashmap::iterator MiniTaskHashmap_iterator;
    /**
     * \brief 定义容器常量迭代器类型
     *
     */
    typedef MiniTaskHashmap::const_iterator MiniTaskHashmap_const_iterator;
    /**
     * \brief 定义容器键值对类型
     *
     */
    typedef MiniTaskHashmap::value_type MiniTaskHashmap_pair;
    /**
     * \brief 容器访问互斥变量
     *
     */
    zRWLock rwlock;
    /**
     * \brief 声明一个容器，存放所有的子连接
     *
     */
    MiniTaskHashmap sessionTaskSet;

};

#define MAX_MONEY 100000

struct oneGameState
{
  Cmd::MiniUserPosition pos;
  Cmd::MiniUserState state;
  //Cmd::MiniGameScore score;
  bool isNew;
  bool needSave;

  oneGameState()
    :state(Cmd::MUS_NOTPLAY),isNew(true),needSave(false){}
};

class MiniUser :public zEntry 
{
  public:
    WORD face;
    WORD country;

    MiniUser(DWORD i,char *n,WORD c,WORD f,MiniTask *gate,MiniTask *scene);
    void setScene(MiniTask *scene);

    bool sendCmdToMe(const void *pstrCmd,const int nCmdLen) const;
    bool sendCmdToScene(const void *pstrCmd,const int nCmdLen) const;
    void sendSys(int type,const char *pattern,...) const;
    void sendMiniInfo(int type,const char *pattern,...) const;

    Cmd::MiniUserState getGameState(const BYTE &type);
    void setGameState(const BYTE &type,const Cmd::MiniUserState &s);
    Cmd::MiniUserPosition getGamePos(const BYTE &type);
    void setGamePos(const BYTE &type,const Cmd::MiniUserPosition &pos);

    void full_MiniUserData(const BYTE &type,Cmd::MiniUserData &data);

    void addScore(Cmd::MiniGameScore score,bool isNew=true);
    Cmd::MiniGameScore getGameScore();
    void sendGameScore(MiniUser *u=0);

    bool save();

    friend class MiniUserManager;
    std::map<BYTE,oneGameState> gameList;
    typedef std::map<BYTE,oneGameState>::iterator game_iter;

    int getMoney();
    bool addMoney(int num);
    bool checkMoney(int num);
    bool removeMoney(int num);
  private:
    MiniTask *minitask;
    MiniTask *scene;

    Cmd::MiniGameScore score;
    bool needSave;
};

class MiniUserManager :public zEntryManager< zEntryID>
{
  private:
    zRWLock rwlock;
    BYTE saveGroup;

  private:
    MiniUserManager();
    ~MiniUserManager();
    static MiniUserManager *instance;

  public:
    static MiniUserManager *getInstance();
    static MiniUserManager &getMe();
    static void delInstance();

    MiniUser * newUser(DWORD id,char *name,WORD country,WORD face,MiniTask *task);
    MiniUser * newUser(Cmd::Mini::t_UserLogin_Gateway *info);

    MiniUser * getUserByID( DWORD id)
    {
      rwlock.rdlock();
      MiniUser *ret =(MiniUser *)getEntryByID(id);
      rwlock.unlock();
      return ret;
    }

    template <class YourUserEntry>
      bool execEveryUser(execEntry<YourUserEntry> &exec)
      {
        rwlock.rdlock();
        bool ret=execEveryEntry<>(exec);
        rwlock.unlock();
        return ret;
      }

    void update();
    void removeUserByGatewayID(MiniTask *task);

    void removeUser(MiniUser *user)
    {
      rwlock.wrlock();
      removeEntry((zEntry *)user);
      delete user;
      rwlock.unlock();
    }

  private:
    bool addUser(MiniUser *user)
    {
      rwlock.wrlock();
      bool ret =addEntry((zEntry *)user);
      rwlock.unlock();
      return ret;
    }
};

class Top100 : public Singleton<Top100>
{
  public:
    bool init();
    void calculate(MiniUser *u);
    void send(MiniUser *u);
    void remove(DWORD id);
  private:
    std::list<Cmd::MiniUserData> top100;
    typedef std::list<Cmd::MiniUserData>::iterator top_iter;
};

class MiniHall : public Singleton<MiniHall>
{
  friend class MiniService;

  public:
    MiniHall();
    ~MiniHall();

    bool parseUserCmd(MiniUser *u,Cmd::stMiniGameUserCmd *cmd,DWORD len);

    MiniRoom * getRoom(const Cmd::MiniRoomID &id);
    MiniGame * getGame(const Cmd::MiniGameID &id);

    void userEnter(MiniUser *u);
    void userLeave(MiniUser *u);

    void sendRoomData(MiniUser *u);
    void updateRoomUserNum(Cmd::MiniRoomID roomID,DWORD num);

    void timer();
  private:
    bool init();
    bool parseCommonCmd(MiniUser *u,Cmd::stCommonMiniGameCmd *cmd,DWORD len);

  private:
    std::map<DWORD,MiniRoom *> roomList;
    typedef std::map<DWORD,MiniRoom *>::iterator room_iter;
    Cmd::stGameListCommonMiniGameCmd *roomDataCmd;
};

class MiniRoom
{
  public:
    MiniRoom();
    DWORD init(Cmd::MiniRoomID id,DWORD gameNum,DWORD userNum,DWORD money);

    BYTE gameType() const;
    MiniGame *getGame(const Cmd::MiniGameID &id);
    DWORD userCount();

    bool userEnter(MiniUser *u);
    void userLeave(MiniUser *u);

    void sendUserToRoom(MiniUser *u);
    void sendUserStateToRoom(MiniUser *u);
    void sendRoomToUser(MiniUser *u);
    void sendCmdToAll(const void *cmd,const DWORD len);
    void sendCmdToIdle(const void *cmd,const DWORD len);

    void timer();
  private:
    bool full() const;
    MiniGame *createGame(Cmd::MiniGameID id,DWORD userNum,DWORD money);

  public:
    Cmd::MiniRoomID id;
  private:
    DWORD oneGameUserNum;
    hash_map<DWORD,MiniGame *> gameList;//id-game

    std::set<MiniUser *> userList;// 玩家-状态

    typedef hash_map<DWORD,MiniGame *>::iterator game_iter;
    typedef std::set<MiniUser *,Cmd::MiniUserState>::iterator user_iter;
    typedef std::set<MiniUser *,Cmd::MiniUserState>::const_iterator const_user_iter;
};

struct Seat
{
  MiniUser *user;
  BYTE open;
  Seat():user(0),open(1){}
};

class MiniGame
{
  public:
    MiniGame(Cmd::MiniGameID id,BYTE userNum,DWORD money);
    virtual ~MiniGame(){};

    bool userEnter(MiniUser *u,Cmd::MiniUserPosition seatID);
    void userLeave(MiniUser *u);
    bool toggleReady(MiniUser *u);

    BYTE find(MiniUser *u);
    void full_MiniSeatData(Cmd::stSeatStateCommonMiniGameCmd *cmd);
    Cmd::MiniGameState getState();

    bool empty();
    bool full();
    bool canStart();
    bool start();
    bool end();
    void timer();

    bool parseGameCmd(MiniUser *u,Cmd::stMiniGameUserCmd *cmd,DWORD len);

    void sendCmdToAll(const void *cmd,const DWORD len) const;
    void sendInfoToAll(const int type,const char *info,...) const;

    void toggleSeat(MiniUser *host,Cmd::MiniSeatID seatID);
    void kickUser(MiniUser *h,MiniUser *u);

    MiniRoom *getRoom();
  protected:
    virtual void v_userEnter(MiniUser *u,Cmd::MiniUserPosition seatID){}
    virtual void v_userLeave(MiniUser *u){}
    virtual bool v_start(){return true;}
    virtual bool v_end(){return true;}
    virtual void v_timer(){}
    virtual bool v_parseGameCmd(MiniUser *u,Cmd::stMiniGameUserCmd *cmd,DWORD len){return false;}

    Cmd::MiniSeatID makeSeatID(BYTE seat){return Cmd::MiniSeatID(id.type,id.room,id.game,seat);}
    void setHost(BYTE seat);

    void enableAllSeats();
    BYTE nextUserSeat(BYTE from);//得到下一个有人的座位
  public:
    Cmd::MiniGameID id;

  protected:
    Cmd::MiniGameState state;

    BYTE minUserNum;//最少人数
    BYTE maxUserNum;//最多人数
    BYTE curUserNum;//当前人数
    //BYTE hostSeat;//房主的座位ID
    std::vector<Seat> seatList;

    DWORD money;//游戏币基数
};

/**
 * \brief 定义计费服务类
 *
 * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
 *
 */
class MiniService : public zSubNetService
{

  public:

    bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief 虚析构函数
     *
     */
    ~MiniService()
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
    static MiniService &getInstance()
    {
      if (NULL == instance)
        instance = new MiniService();

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
        static zLogger* miniLogger;    
     */



  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static MiniService *instance;

    zTCPTaskPool *taskPool;        /**< TCP连接池的指针 */

    /**
     * \brief 构造函数
     *
     */
    MiniService() : zSubNetService("小游戏服务器",MINISERVER)
    {
      taskPool = NULL;
    }

    bool init();
    void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
    void final();
};

typedef std::map<Cmd::Card,DWORD> CardList;
typedef std::map<Cmd::Card,DWORD>::iterator card_iter;

class CardPattern
{
  public:
    static bool match_pattern(const DWORD &packNum,const Cmd::Card *cards,const DWORD &num,CardPattern &pat);

    CardList list;
    DWORD serialNum;//连续几个
    DWORD unitNum;//一组几个
    Cmd::Card value;//值,按序列最大的算
    BYTE bomb;
    BYTE missile;
    BYTE add;//有几张附带牌(3顺)

    CardPattern():serialNum(1),unitNum(1),bomb(0),missile(0),add(0),_valid(false){}
    CardPattern &operator=(const CardPattern &c);
    bool operator>(const CardPattern &c);

    void clear(){_valid = false;}
    bool valid()const{return _valid;}
  private:
    bool _valid;
};

class DDZCardGame : public MiniGame
{
  public:
    DDZCardGame(Cmd::MiniGameID id,BYTE userNum,DWORD money);
    bool v_start();
    void v_userLeave(MiniUser *u);
    bool v_parseGameCmd(MiniUser *u,Cmd::stMiniGameUserCmd *cmd,DWORD len);
    void v_timer();

  private:
    enum DDZ_State
    {
      DDZS_POINT,//叫分
      DDZS_PLAY//开始
    }ddz_state;

    std::vector<Cmd::Card> allCardList;
    std::vector<CardList> userCardList;

    BYTE packNum;//几副牌
    BYTE point;//分数
    BYTE pointTime;//倍率
    BYTE rCardNum;//保留几张底牌
    BYTE lordSeat;//地主的位置

    BYTE lordPutTime;//地主出过几次牌
    BYTE otherPutTime;//其他人出过几次牌

    BYTE curPutSeat;//当前出牌的人
    BYTE lastPutSeat;//最后一个出牌的人
    CardPattern lastPattern;//最后出的牌

    BYTE curPointSeat;//当前叫牌的人
    BYTE lastPointSeat;//上一局先叫牌的人

    DWORD countdown;//计时

    void clean();
    void shuffle();
    void deal();
    void initCards();
    void showReserveCards();//显示底牌
    BYTE nextPointSeat();
    void nextPutUser(BYTE seat=0);
    void judge(BYTE seat);

    void auto_put();
    void auto_point();

    void calcNormalScore(BYTE seat);//计算得分
    void calcFleeScore(BYTE seat);//玩家逃跑计分

    bool canPut(BYTE seat,const Cmd::stPutCardMiniGameCmd *cmd,DWORD len);
    //int compare(const std::list<Cmd::Card> &list1,const std::list<Cmd::Card> &list2);
    void putCards(BYTE seat,Cmd::stPutCardMiniGameCmd *cmd,DWORD len);

    BYTE bombCount();
};

class MiniTimeTick : public zThread
{

  public:

    ~MiniTimeTick() {};

    /// 当前时间
    static zRTime currentTime;
    static Timer _1_min;
    static MiniTimeTick &getInstance()
    {
      if (NULL == instance)
        instance = new MiniTimeTick();

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

    static MiniTimeTick *instance;

    MiniTimeTick() : zThread("TimeTick") {};

};
