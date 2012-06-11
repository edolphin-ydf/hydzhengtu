/**
 * \brief zebra��Ŀ�Ʒѷ�����
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
 * \brief ����Ʒ�����������
 *
 */
class MiniTask : public zTCPTask,public MessageQueue
{

  public:

    /**
     * \brief ���캯��
     *
     * \param pool �������ӳ�ָ��
     * \param sock TCP/IP�׽ӿ�
     * \param addr ��ַ
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
     * \brief ����������
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

    bool verifyLogin(const Cmd::Mini::t_LoginMini *ptCmd);
    int recycle_state;
    bool veriry_ok;

    bool addDBMoney(DWORD userID,DWORD num);
};

/**
 * \brief �Ʒѷ����������ӹ�����
 *
 */
class MiniTaskManager
{

  public:

    /**
     * \brief ��������
     *
     */
    ~MiniTaskManager() {};

    /**
     * \brief ��ȡ�����ӹ�����Ψһʵ��
     *
     * \return ������Ψһʵ��
     */
    static MiniTaskManager &getInstance()
    {
      if (NULL == instance)
        instance = new MiniTaskManager();

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

    bool uniqueAdd(MiniTask *task);
    bool uniqueRemove(MiniTask *task);
    bool broadcastByID(const WORD wdServerID,const void *pstrCmd,int nCmdLen);
    MiniTask *getTaskByID(const WORD wdServerID);
    void execEvery();

  private:

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static MiniTaskManager *instance;

    /**
     * \brief ���캯��
     *
     */
    MiniTaskManager() {};

    /**
     * \brief ������������
     *
     */
    typedef hash_map<WORD,MiniTask *> MiniTaskHashmap;
    /**
     * \brief ������������������
     *
     */
    typedef MiniTaskHashmap::iterator MiniTaskHashmap_iterator;
    /**
     * \brief ����������������������
     *
     */
    typedef MiniTaskHashmap::const_iterator MiniTaskHashmap_const_iterator;
    /**
     * \brief ����������ֵ������
     *
     */
    typedef MiniTaskHashmap::value_type MiniTaskHashmap_pair;
    /**
     * \brief �������ʻ������
     *
     */
    zRWLock rwlock;
    /**
     * \brief ����һ��������������е�������
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

    std::set<MiniUser *> userList;// ���-״̬

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
    BYTE nextUserSeat(BYTE from);//�õ���һ�����˵���λ
  public:
    Cmd::MiniGameID id;

  protected:
    Cmd::MiniGameState state;

    BYTE minUserNum;//��������
    BYTE maxUserNum;//�������
    BYTE curUserNum;//��ǰ����
    //BYTE hostSeat;//��������λID
    std::vector<Seat> seatList;

    DWORD money;//��Ϸ�һ���
};

/**
 * \brief ����Ʒѷ�����
 *
 * �����ʹ����Singleton���ģʽ����֤��һ��������ֻ��һ�����ʵ��
 *
 */
class MiniService : public zSubNetService
{

  public:

    bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief ����������
     *
     */
    ~MiniService()
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
    static MiniService &getInstance()
    {
      if (NULL == instance)
        instance = new MiniService();

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
        static zLogger* miniLogger;    
     */



  private:

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static MiniService *instance;

    zTCPTaskPool *taskPool;        /**< TCP���ӳص�ָ�� */

    /**
     * \brief ���캯��
     *
     */
    MiniService() : zSubNetService("С��Ϸ������",MINISERVER)
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
    DWORD serialNum;//��������
    DWORD unitNum;//һ�鼸��
    Cmd::Card value;//ֵ,������������
    BYTE bomb;
    BYTE missile;
    BYTE add;//�м��Ÿ�����(3˳)

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
      DDZS_POINT,//�з�
      DDZS_PLAY//��ʼ
    }ddz_state;

    std::vector<Cmd::Card> allCardList;
    std::vector<CardList> userCardList;

    BYTE packNum;//������
    BYTE point;//����
    BYTE pointTime;//����
    BYTE rCardNum;//�������ŵ���
    BYTE lordSeat;//������λ��

    BYTE lordPutTime;//��������������
    BYTE otherPutTime;//�����˳���������

    BYTE curPutSeat;//��ǰ���Ƶ���
    BYTE lastPutSeat;//���һ�����Ƶ���
    CardPattern lastPattern;//��������

    BYTE curPointSeat;//��ǰ���Ƶ���
    BYTE lastPointSeat;//��һ���Ƚ��Ƶ���

    DWORD countdown;//��ʱ

    void clean();
    void shuffle();
    void deal();
    void initCards();
    void showReserveCards();//��ʾ����
    BYTE nextPointSeat();
    void nextPutUser(BYTE seat=0);
    void judge(BYTE seat);

    void auto_put();
    void auto_point();

    void calcNormalScore(BYTE seat);//����÷�
    void calcFleeScore(BYTE seat);//������ܼƷ�

    bool canPut(BYTE seat,const Cmd::stPutCardMiniGameCmd *cmd,DWORD len);
    //int compare(const std::list<Cmd::Card> &list1,const std::list<Cmd::Card> &list2);
    void putCards(BYTE seat,Cmd::stPutCardMiniGameCmd *cmd,DWORD len);

    BYTE bombCount();
};

class MiniTimeTick : public zThread
{

  public:

    ~MiniTimeTick() {};

    /// ��ǰʱ��
    static zRTime currentTime;
    static Timer _1_min;
    static MiniTimeTick &getInstance()
    {
      if (NULL == instance)
        instance = new MiniTimeTick();

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

    static MiniTimeTick *instance;

    MiniTimeTick() : zThread("TimeTick") {};

};
