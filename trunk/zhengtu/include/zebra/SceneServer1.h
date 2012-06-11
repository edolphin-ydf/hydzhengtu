
#include "ScenesServer.h"

class SceneUserManager:protected zUserManager
{
  protected:
    inline bool getUniqeID(DWORD& tempid);
    inline void putUniqeID(const DWORD& tempid);
    SceneUserManager();
    virtual ~SceneUserManager();
    static SceneUserManager *sum;
  public:
    static SceneUserManager &getMe();
    static void destroyMe();
    SceneUser * getUserByName( const char * name);
    SceneUser * getUserByID( DWORD id);
    SceneUser * getUserByTempID( DWORD tempid);
    SceneUser * getUserByNameOut( const char * name);
    SceneUser * getUserByIDOut( DWORD id);
    SceneUser * getUserByTempIDOut( DWORD tempid);
    bool addUser(SceneUser *user);
    void removeUser(SceneUser *user);
    void removeUserInOneScene(Scene *scene);
    void removeUserToHuangcheng(Scene *scene);
    void clearEmperorDare(Scene* scene);
    void setEmperorDare(Scene* scene);
    void removeUserByTask(SceneTask *task);
    void removeAllUser();
    DWORD countUserInOneScene(Scene *scene);
    DWORD countUserByTask(SceneTask *task);
    void countryTrans(DWORD dwCountryID,DWORD dwLevel);
    void setAntiAtt(DWORD dwType,DWORD dwFromRelationID,DWORD dwToRelationID);
    void enterWar(Cmd::Session::t_enterWar_SceneSession* cmd);
};


class SceneRecycleUserManager: public zEntryManager< zEntryID > 
{
  private:
    /**
     ** \brief 管理器访问互斥锁
     **/
    zRWLock rwlock;
    static SceneRecycleUserManager *instance;
    SceneRecycleUserManager(){}
    ~SceneRecycleUserManager(){}
  public:
    static SceneRecycleUserManager &getInstance();
    static void destroyInstance();
    SceneUser* getUserByID(DWORD id);
    void removeUser(SceneUser *user);
    bool addUser(zSceneEntry *user);
    bool canReg(DWORD id);
    void refresh();
    /**
     ** \brief 对每个用户执行
     ** \param exec 执行接口
     **/
    template <class YourUserEntry>
      bool execEveryUser(execEntry<YourUserEntry> &exec)
      {
        rwlock.rdlock();
        bool ret=execEveryEntry<>(exec);
        rwlock.unlock();
        return ret;
      }

};

/**
 * \brief AI类型,npc会做的基本动作
 * 走、攻击、巡逻、跳转等
 *
 */
enum SceneNpcAIType
{
  NPC_AI_NORMAL,///普通AI
  NPC_AI_SAY,///说话
  NPC_AI_MOVETO,///移动到某位置
  NPC_AI_ATTACK,///在某范围内攻击
  NPC_AI_FLEE,///逃离玩家
  NPC_AI_RETURN_TO_REGION,///回到活动范围
  NPC_AI_GO_ON_PATH,///按照一定路线移动
  NPC_AI_CHANGE_MAP,///切换地图（同一服务器内）
  NPC_AI_WARP,///同一地图内瞬间移动
  NPC_AI_PATROL,///巡逻
  NPC_AI_CLEAR,///清除该npc
  NPC_AI_WAIT,///等待,什么也不做
  NPC_AI_GUARD_DOOR,///门卫
  NPC_AI_GUARD_ARCHER,///弓卫
  NPC_AI_GUARD_GUARD,///侍卫
  NPC_AI_GUARD_PATROL,///巡逻卫士
  NPC_AI_DROP_ITEM,///丢东西
  NPC_AI_CALL_PET,///招宠物
  NPC_AI_RANDOM_CHAT///随机说话
};

///npcAI标志定义
enum NpcAIFlag
{
	AIF_ATK_PDEF            = 0x00000001, ///优先攻击物防最低的敌人
	AIF_ATK_MDEF            = 0x00000002, ///优先攻击魔防最低的敌人
	AIF_ATK_HP              = 0x00000004, ///优先攻击生命值最低的敌人
	AIF_GIVEUP_10_SEC       = 0x00000008, ///追逐10秒放弃目标
	AIF_GIVEUP_6_SEC        = 0x00000010, ///6秒未受到伤害放弃目标
	AIF_GIVEUP_3_SEC        = 0x00000020, ///3秒未受到伤害放弃目标
	AIF_FLEE_30_HP    = 0x00000040, ///HP30%以下逃跑4格
	AIF_FLEE_3_ENEMY_4  = 0x00000080, ///被3个以上敌人围攻逃跑4格
	AIF_NO_BATTLE    = 0x00000100,///非战斗npc
	AIF_NO_MOVE    = 0x00000200, ///不移动（弓卫、买卖、路标等）
	AIF_WARP_MOVE    = 0x00000400, ///瞬移方式移动
	AIF_CALL_FELLOW_7       = 0x00000800, ///召唤7*7范围的未激活npc(几率50%)
	AIF_CALL_FELLOW_9       = 0x00001000, ///召唤9*9范围的未激活npc（几率50%）
	AIF_CALL_BY_ATYPE       = 0x00002000, ///召唤同种攻击类型的同伴（与上两个标志合作）
	AIF_HELP_FELLOW_5  = 0x00004000,///帮助5*5范围内的同伴攻击（用于被动npc）
	AIF_ATK_MASTER    = 0x00008000,///直接攻击宠物的主人
	AIF_ATK_REDNAME    = 0x00010000,///攻击红名的玩家
	AIF_DOUBLE_REGION  = 0x00020000,///搜索范围加倍
	AIF_SPD_UP_HP20    = 0x00040000,///hp20%以下移动速度加倍
	AIF_ASPD_UP_HP50  = 0x00080000,///hp50%以下攻击速度加倍
	AIF_ACTIVE_MODE    = 0x00100000,///主动攻击
	AIF_RUN_AWAY    = 0x00200000,///逃离敌人
	AIF_LOCK_TARGET    = 0x00400000,///不切换攻击目标直到死
	AIF_RCV_UNDER_30  = 0x00800000,///hp30%以下持续回血1%
	AIF_RCV_REST    = 0x01000000,///脱离战斗30秒后回血一次5%
	AIF_LIMIT_REGION  = 0x02000000  ///限制活动范围
};

///npc说话的类型
enum NpcChatType
{
  NPC_CHAT_ON_FIND_ENEMY = 1,///发现敌人
  NPC_CHAT_ON_ATTACK,///攻击时说的话
  NPC_CHAT_ON_RETURN,///追逐返回时说的话
  NPC_CHAT_ON_DIE,///死的时候说的话
  NPC_CHAT_ON_FLEE,///逃跑时说的话
  NPC_CHAT_ON_HIT,///被打时说的话
  NPC_CHAT_ON_HELP,///帮助同伴时说的话
  NPC_CHAT_ON_BE_HELP,///同伴来帮助时说的话
  NPC_CHAT_RANDOM    ///随机说话
};

/**
 * \brief 一个AI的定义
 *
 */
struct t_NpcAIDefine
{
  ///类型,NPC在该阶段的主要动作
  SceneNpcAIType type;
  ///位置 根据不同动作位置的意义也略不相同
  ///移动时表示目的地,其他表示活动范围中心
  zPos pos;
  ///范围 
  ///移动时表示到达目的地的判定范围,其他表示活动范围
  int regionX,regionY;
  //zRTime endTime;
  ///该AI的持续时间
  int lasttime;

  ///是否正在逃跑
  bool flee;
  ///逃跑的方向
  int fleeDir;
  ///逃跑计数
  int fleeCount;

  ///切换地图时,要去的地图
  ///说话时,要说的话
  char str[MAX_CHATINFO];


  /**
   * \brief 默认构造函数
   *
   */
  t_NpcAIDefine()
    :type(NPC_AI_NORMAL),pos(zPos(0,0)),regionX(2),regionY(2),lasttime(0),flee(false),fleeDir(-1),fleeCount(0)
    {
      bzero(str,sizeof(str));
    }
  
  /**
   * \brief 构造函数
   *
   *
   * \param type AI类型
   * \param pos 位置
   * \param regionX 范围宽
   * \param regionY 范围高
   * \param lasttime 持续时间
   * \return 
   */
  t_NpcAIDefine(SceneNpcAIType type,const zPos &pos,int regionX,int regionY,int lasttime)
    :type(type),pos(pos),regionX(regionX),regionY(regionY),lasttime(lasttime)
    {
      bzero(str,sizeof(str));
    }


  /**
   * \brief 拷贝构造函数
   *
   * \param ad 要复制的对象
   */
  t_NpcAIDefine(const t_NpcAIDefine &ad)
  {
    type = ad.type;
    pos = ad.pos;
    regionX = ad.regionX;
    regionY = ad.regionY;
    lasttime = ad.lasttime;
    flee = ad.flee;
    fleeDir = ad.fleeDir;
    fleeCount = ad.fleeCount;
    strncpy(str,ad.str,sizeof(str)-1);
  }

  /**
   * \brief 赋值
   *
   * \param ad 要拷贝的对象
   * \return 返回自身地址
   */
  t_NpcAIDefine & operator = (const t_NpcAIDefine &ad)
  {
    type = ad.type;
    pos = ad.pos;
    regionX = ad.regionX;
    regionY = ad.regionY;
    lasttime = ad.lasttime;
    flee = ad.flee;
    fleeDir = ad.fleeDir;
    fleeCount = ad.fleeCount;
    strncpy(str,ad.str,sizeof(str)-1);

    return *this;
  }
};


/**
 * \brief AI控制器
 * 控制器可以读取NPC脚本,使NPC按照脚本来动作
 * 
 * 控制器处理各种事件,时间、被攻击、死亡等
 * 根据不同的条件为NPC设置不同的AI
 *
 */
class NpcAIController
{
  static const int npc_call_fellow_rate;///NPC召唤同伴的几率
  static const int npc_one_checkpoint_time;///NPC按照路线移动时,走一个路点的最长时间
  static const int npc_checkpoint_region;///NPC移动,到达一个路点的判定范围
  static const int npc_onhit_stop_time;///任务NPC移动中被攻击时,停止的时间
  static const int npc_flee_distance;///NPC逃离攻击者的距离
  static const int npc_min_act_region;///NPC最小活动范围
  
  ///AI容器,AI按照在容器中的顺序依次执行
  std::vector<t_NpcAIDefine> phaseVector;
  ///当前的AI索引
  DWORD curPhase;
  ///当前AI的结束时间
  zRTime phaseEndTime;

  ///脚本重复次数
  ///-1：无限循环  0：停止  >0：循环次数
  int repeat;

  ///是否起用了脚本
  bool active;
  void nextPhase(int index);
  void on_phaseEnd();
  SceneNpcAIType parseAction(char *);
  
  ///本控制器控制的npc
  SceneNpc * npc;

  ///当前的AI和保存的前一个AI
  t_NpcAIDefine curAI,oldAI;
  ///活动范围的中心
  zPos actPos;
  ///活动范围的宽和高
  int actRegionX,actRegionY;
  bool outOfRegion() const;
  void returnToRegion();

  bool arrived(zPos pos = zPos(0,0),int x = -1,int y = -1);
  bool dstReached();
  ///是否到达目的地
  bool reached;

  ///目的地图
  char dstMap[32];
  ///目的位置
  zPos dstPos;

public:
  void setAI(const t_NpcAIDefine ai,const bool setTime = true);
  void switchAI(const bool setTime = true);
  void setNormalAI();
  void setActRegion(zPos pos = zPos(0,0),int x = -1,int y = -1);
  void getActRegion(zPos &,int &,int  &);
  NpcAIController(SceneNpc *);
  bool loadScript(DWORD id);
  void unloadScript();
  void processPhase();
  void setRepeat(int repeat);
  int getRepeat();
  bool isActive();
  void setPhaseTime(const int delay);
  void delayPhaseTime(const int delay);
  bool phaseTimeOver();

  void on_relive();
  void on_find_enemy(const SceneEntryPk *);
  void on_hit(SceneEntryPk *pAtk);
  void on_die();
};

//需要进行AI动作的npc
typedef std::set<SceneNpc *,std::less<SceneNpc *> > MonkeyNpcs;

/*
struct t_PetState
{
  DWORD id;
  Cmd::petType type;
  char name[MAX_NAMESIZE];
  DWORD hp;
  DWORD exp;
  WORD ai;
};
*/

struct t_expRec
{
  DWORD wdHP;
  zTime  attack_time;

  t_expRec()
  {
    attack_time.now();
    wdHP = 0;
  }
};

/* 一些特殊npc的ID */
const DWORD COUNTRY_MAIN_FLAG = 58001;
const DWORD COUNTRY_SEC_FLAG  =  58002;
const DWORD COUNTRY_MAIN_GEN = 58200;
const DWORD COUNTRY_SEC_GEN  = 58201;
const DWORD COUNTRY_KING_MAIN_FLAG = 58005;
const DWORD COUNTRY_KING_SEC_FLAG = 58006;
const DWORD COUNTRY_EMPEROR_MAIN_GEN = 58203;
const DWORD COUNTRY_EMPEROR_SEC_GEN = 58204;

const DWORD ALLY_GUARDNPC = 54100;//盟国镖车
const DWORD EMPEROR_HORSE_ID = 3202;//皇帝的马ID
const DWORD KING_HORSE_ID = 3204;//国王的马ID

/**
 * \brief 定义Npc
 *
 */
class SceneNpc : public SceneEntryPk,public zAStar<>,public zAStar<2>
{
  friend class NpcAIController;

  static const DWORD maxUniqueID = 100000;

  public:
  
  //unsigned short dupIndex;

  bool isMainGeneral();//是否大将军王

  zRTime reliveTime;//复活时间

  int targetDistance;//与当前目标的最短距离
  int closeCount;//追逐计数,10步以内最短距离没有减少则认为目标不可到达

  std::list<ScenePet *> semipetList;//半宠物列表

  /**
   * \brief 定义Npc跟踪状态
   *
   */
  enum SceneNpcChase
  {
    CHASE_NONE,     /// 没有跟踪状态
    CHASE_ATTACK,   /// 跟踪攻击状态
    CHASE_NOATTACK    /// 普通跟踪状态
  };

  /**
   * \brief Npc类型
   * 静态的还是动态分配的
   */
  enum SceneNpcType
  {
    STATIC,       /// 静态的
    GANG        /// 动态的
  };

  /**
   * \brief Npc基本数据
   *
   */
  zNpcB *npc;

  /**
   * \brief 增强Npc基本数据
   *
   */
  zNpcB *anpc;

  /**
   * \brief Npc定义数据
   *
   */
  const t_NpcDefine *define;

  /**
   * \brief npc当前生命值
   *
   */
  DWORD hp;

  ///上次发送时的hp
  DWORD lasthp;

  ///回血标记
  bool needRecover;
  ///下次回血的时间
  zRTime rcvTimePet;//宠物休息
  zRTime rcvTimeUnder30;//hp30以下
  zRTime rcvTimeRest;//脱离战斗
  //hp30以下回血
  bool recoverUnder30;
  //结束战斗回血
  //bool recoverLeaveBattle;

  bool checkRecoverTime(const zRTime& ct);
  void setRecoverTime(const zRTime& ct,int delay);

  virtual bool recover();
  /// npc的生命结束时间(如果没设置就是其创建时间)
  DWORD dwStandTime;

  ///npc的生命持续时间
  DWORD dwStandTimeCount;

  ///附加最小攻击力
  WORD appendMinDamage;

  ///附加最大攻击力
  WORD appendMaxDamage;

  ///是否已经锁定目标
  bool lockTarget;

  ///攻城的npc,随攻城一起删除
  bool isRushNpc;

  ///根据配置召唤其他npc
  int summonByNpcMap(std::map<DWORD,std::pair<DWORD,DWORD> > map);

  ///是否召唤过
  bool summoned;

  bool setCurTarget(SceneEntryPk *,bool=false);
  bool setCurTarget(DWORD,DWORD,bool=false);
  void leaveBattle();

  void setClearState();
  bool needClear();

  bool isSpecialNpc();
  bool isFunctionNpc();
  bool isTaskNpc();

  SceneNpc(Scene *scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype=SceneEntry_NPC,zNpcB *anpc=NULL);
  ~SceneNpc();
  bool zPosOutofRange()const;
  SceneNpcChase getChaseMode()const;
  SceneEntryPk* getChaseSceneEntry() const;
  bool chaseSceneEntry(const DWORD type,const DWORD userid);
  virtual bool isRedNamed(bool allRedMode=true);
  void unChaseUser();
  bool checkLockUserOverdue(const zRTime &ct);
  void setLockUser(const DWORD dwID);
  bool checkMoveTime(const zRTime &ct);
  void setMoveTime(const zRTime &ct);
  void setMoveTime(const zRTime &ct,const int delay);
  void delayMoveTime(const int delay);
  bool checkAttackTime(const zRTime &ct) const;
  void setAttackTime(const zRTime &ct);
  void setAttackTime(const zRTime &ct,const int delay);
  //bool checkUnchaseTime(const zRTime &ct) const;
  //void setUnchaseTime(const zRTime &ct,const int delay);
  bool canLostObject(const zRTime &ct);
  bool canRelive(const zRTime &ct);
  bool canRush();
  const SceneNpcType &getSceneNpcType() const;
  void full_t_NpcData(Cmd::t_NpcData &data);
  void full_t_MapNpcDataState(Cmd::t_MapNpcDataState &data);
  void full_t_MapNpcDataAndPosState(Cmd::t_MapNpcDataPosState &data);
  void full_t_MapNpcData(Cmd::t_MapNpcData &data);
  void full_stRTMagicPosUserCmd(Cmd::stRTMagicPosUserCmd &ret) const;
  void full_t_MapNpcDataAndPos(Cmd::t_MapNpcDataPos &data);
  virtual void sendMeToNine();
  bool isBugbear();
  bool canBeAttack();
  void death(const zRTime &ct);
  void backoff(const int direct,const int step);
  void reduceHP(SceneUser *pAtt,DWORD wdHP);
  void distributeExp();
  void distributeMoney(DWORD money);
  void clearStateToNine(WORD state);
  void setStateToNine(WORD state);
  /**
   * \brief 让宠物重生
   */
  virtual void relivePet(){};

  DWORD catchme; ///吸引怪物攻击自己
  int boostupPet; /// 增强比例
  DWORD boostupPetMDef; //增强宠物的法术防御
  DWORD boostupSummon; ///召唤兽攻击加强
  DWORD boostupHpMaxP;  ///增加生命值上限
  DWORD dwReduceDam;  /// 召唤兽伤害扣减
  DWORD giddy;   ///攻击的时候使对方眩晕的几率

  BYTE notifystep; //绿BOSS通知步骤

  //*
  static void AI(const zRTime& ctv,MonkeyNpcs &affectNpc,const DWORD group,const bool every);

  /**
   * \brief 强制跟踪用户,如果怪已经在跟踪用户,那么有45%的几率将目标转换成目前的用户
   * \param pAtt 要跟踪的对象
   */
  bool forceChaseUser(SceneEntryPk *pAtt);
  /**
   * \brief 将客户端消息转发到会话服务器
   */
  bool forwardSession(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
  //bool checkChaseUser(SceneUser *&sceneUser);
  bool checkChaseAttackTarget(SceneEntryPk *&entry);
  bool canChaseTarget(const SceneEntryPk *entry);
  bool canAttack(SceneEntryPk *entry);
  bool inRange(SceneEntryPk *entry);
  bool attackTarget(SceneEntryPk *entry = NULL);
  //bool chasedFindFrontUser( const int direct);
  //bool findAndChaseUser(const int radius,SceneEntryPk *&entry);
  //bool findAnyTarget(const int radius,SceneEntryPk *&entry);
  void action(const zRTime& ctv);
  virtual bool normalAction();
  bool deathAction();
  bool hideAction();
  bool moveable(const zPos &tempPos,const zPos &destPos,const int radius);
  bool move(const int direct,const int step);
  bool warp(const zPos &pos,bool ignore=false);//跳转
  void jumpTo(zPos &newPos);
  bool gotoFindPath(const zPos &srcPos,const zPos &destPos);
  bool goTo(const zPos &pos);
  bool shiftMove(const int direct);
  void set_quest_status(SceneUser* user);
  void setStandingTime(DWORD standTime);
  void refreshExpmapAttackTime(SceneUser* pAtt);

  /**
   * \brief 改变角色的hp
   * \param hp 变更的HP
   */
  void changeHP(const SDWORD &hp);

  /**
   * \brief 造成直接伤害
   * \param pAtt 攻击者
   * \param dam 伤害值
   * \param notify 通知伤害显示
   */
  SWORD directDamage(SceneEntryPk *pAtt,const SDWORD &dam,bool notify=false);

  /**
   * \brief 改变角色的sp
   * \param sp 变更的SP
   */
  void changeSP(const SDWORD &sp);

  /**
   * \brief 改变角色的mp
   * \param mp 变更的MP
   */
  void changeMP(const SDWORD &mp);

  /**
   * \brief 在被自己攻击之前的处理,包括,装备损耗处理,攻击有效几率判断等
   * \param pUser 攻击者
   * \param rev 本次攻击的触发指令
   * \param physics 是否物理攻击
   * \param good 为true则必中,为false需要判断回避率
   * \return true为这次攻击是有效的,false为一次无效的攻击
   */
  bool preAttackMe(SceneEntryPk *pUser,const Cmd::stAttackMagicUserCmd *rev,bool physics=true,const bool good = false);

  /**
   * \brief 角色被攻击
   * \param pEntry 攻击者
   * \param rev 本次攻击的触发消息
   * \param physics 是否物理攻击
   * \return true为这次攻击是有效的,false为一次无效的攻击
   */
  bool AttackMe(SceneEntryPk *pEntry,const Cmd::stAttackMagicUserCmd *rev,bool physics=true,SWORD rangDamageBonus=0);

  /**
   * \brief 角色被击退N格
   * \param dwAttTempID 攻击者的临时ID
   * \param grids 后退几格
   */
  void standBack(const DWORD dwAttTempID,DWORD grids);

  /**
   * \brief 将攻击目标换成dwTempID所指向的角色玩家
   * \param dwTempID 目标角色的临时ID
   */
  void changeAttackTarget(const DWORD &dwTempID);

  /**
   * \brief 让角色死亡
   */
  void toDie(const DWORD &dwTempID);

  /**
   * \brief 通知客户端生命值的变化
   */
  void attackRTHpAndMp();

  /**
   * \brief 发送npc血槽
   */
  void showHP(SceneUser *pUser,DWORD npchp);

  /**
   * \brief 检查玩家是否攻击过该npc
   */
  bool isAttackMe(SceneEntryPk *);
  /**
   * \brief 判断角色是否死亡
   * \return true为死亡
   */
  bool isDie();
  /**
   * \brief 获取角色的级别
   */
  virtual DWORD getLevel() const;

  /**
   * \brief 需要的职业类型,决定可以使用的技能类型
   */
  bool needType(const DWORD &needtype);

  /**
   * \brief 需要的职业类型,决定可以使用的技能类型
   */
  bool addSkillToMe(zSkill *skill);

  /**
   * \brief 是否有该技能需要的武器
   * \return true 有 false 没有
   */
  bool needWeapon(DWORD skillid);

  /**
   * \brief 是否Pk区域
   * \other PK相关人
   * \return true 是 false 否
   */
  virtual bool isPkZone(SceneEntryPk *other=NULL);

  /**
   * \brief 依赖物品消耗型法术
   * \param object 消耗物品的类型
   * \param num 消耗物品的数量
   * \return true 消耗成功 false 失败
   */
  bool reduce(const DWORD &object,const BYTE num);

  /**
   * \brief 检查可消耗物品是否足够
   * \param object 消耗物品的类型
   * \param num 消耗物品的数量
   * \return true 足够 false 不够
   */
  bool checkReduce(const DWORD &object,const BYTE num);

  /**
   * \brief 施放技能所导致的消耗MP,HP,SP
   * \param base 技能基本属性对象
   * \return true 消耗成功 false 失败
   */
  bool doSkillCost(const zSkillB *base);

  /**
   * \brief 检查施放技能所导致的消耗MP,HP,SP是否足够
   * \param base 技能基本属性对象
   * \return true 消耗成功 false 失败
   */
  bool checkSkillCost(const zSkillB *base);

  /**
   * \brief 检查自身的施放成功几率,决定这次技能是否可以施放
   * \return true 成功 false 失败
   */
  bool checkPercent();

  /**
   * \brief 判断是否是敌人
   * \return true 是 false 不是
   */
  //bool isEnemy(SceneUser *pUser);

  /**
   * \brief 死亡后给特定角色增加额外经验,包括护保和装备的额外经验
   * \param wdExp 分配到的经验
   * \param pUser 分配对象
   * \return 重新计算后的经验值
   */
  DWORD addOtherExp(DWORD wdExp,SceneUser *pUser);

  /**
   * \brief 重置最大的hp
   */
  virtual void changeAndRefreshHMS(bool lock=true,bool sendData=true);

  /**
   * \brief 根据等级差别重新计算经验
   * \param wdExp 分配到的经验值
   * \param char_level 角色等级
   * \return 重新计算后的经验值
   */
  DWORD levelExp(DWORD wdExp,DWORD char_level);

  /**
   * \brief 重新发送本NPC的地图数据
   */
  void reSendMyMapData();

  //设置宠物的主人
  virtual void setMaster(SceneEntryPk *){}

  //获取宠物的主人
  virtual SceneEntryPk *getMaster() {return 0;}
  /**
   * \brief 得到最上层的主人
   *
   * \return 主人
   */
  virtual SceneEntryPk *getTopMaster(){return this;}

  //切换场景
  bool changeMap(Scene *newScene,const zPos &pos);
  void setAspeedRate(float rate);
  void resetAspeedRate();

  /**
   * \brief 通知选中自己的用户的hp和mp发生变化
   */
  void sendtoSelectedHpAndMp();
  void sendtoSelectedState(DWORD state,WORD value,WORD time);

  bool createRush();
  virtual bool moveToMaster();

  //AI相关的方法
  void setSpeedRate(float rate);
  float getSpeedRate();
  void resetSpeedRate();

  //const t_NpcAIDefine& getAI();
  void setAI(const t_NpcAIDefine &ad);
  //void setActRegion(zPos pos,DWORD regionX,DWORD regionY);
  //void setNormalAI();
  bool doNormalAI();
  bool doSayAI();
  bool doRandomChatAI();
  bool doMovetoAI();
  bool doAttackAI();
  bool doGoOnPathAI();
  bool doPatrolAI();
  bool doFleeAI();
  bool doChangeMapAI();
  bool doWarpAI();
  bool doReturnToRegionAI();
  bool doClearAI();
  bool doDropItemAI();
  //void returnToRegion();

  virtual bool isRedNamed(bool allRedMode=true) const;
  bool useSkill(SceneEntryPk * target,DWORD id);
  virtual SceneEntryPk * chooseEnemy(SceneEntryPk_vec &);
  bool debuffEnemy(SceneEntryPk *);
  bool attackEnemy(SceneEntryPk *);
  bool moveToEnemy(SceneEntryPk *);
  virtual bool runOffEnemy(SceneEntryPk_vec &);
  virtual bool isActive();
  virtual bool canMove();
  virtual bool canFight();
  //virtual bool checkMasterTarget(SceneEntryPk *&entry){entry = 0;return false;}
  virtual void petDeath(){}
  bool healSelf();
  bool buffSelf();

  bool canReach(SceneEntryPk *);
  bool isSurrounded();
  int isEnemy(SceneEntryPk *,bool notify = false,bool good = false);
  bool getEntries(int,SceneEntryPk_vec &,int);
  bool healFellow(SceneEntryPk_vec &);
  bool buffFellow(SceneEntryPk_vec &);
  bool helpFellow(SceneEntryPk_vec &);

  virtual bool randomMove();
  void randomChat(NpcChatType type);

  //回调点
  virtual void on_reached() { }
  virtual void on_death(SceneEntryPk* att){}
  virtual void check() { }
  bool dreadProcess();
  int IsOppose(DWORD five);

  virtual void clearMaster();

  //zPos actPos;//活动位置
  //DWORD actRegionX,actRegionY;//活动范围
  //t_NpcAIDefine oldAI;
  ///npc控制器的指针
  NpcAIController * AIC;
  bool setScript(int id);
  void clearScript();
  void assaultMe(BYTE attacktype,DWORD tempid);

  BYTE getAType();

  ///npcAI标志
  DWORD aif;
  ///宠物的AI模式
  //WORD petAI;
  ScenePet * summonPet(DWORD,Cmd::petType,DWORD,DWORD,const char *,DWORD,zPos pos=zPos(0,0),BYTE dir=4);
  bool killOnePet(ScenePet *);
  void killAllPets();

  virtual void addPetExp(DWORD num,bool addPet,bool addCartoon){}
  
  virtual void full_PetDataStruct(Cmd::t_PetData &);
  virtual void setPetAI(Cmd::petAIMode){}
  virtual Cmd::petType getPetType();
  virtual void setPetType(Cmd::petType){}
  virtual void setAppendDamage(WORD mindamage,WORD maxdamage);
  virtual DWORD getMinMDamage();
  virtual  DWORD getMaxMDamage(); 
  virtual DWORD getMinPDamage();
  virtual  DWORD getMaxPDamage();
  virtual DWORD getMasterMana(){return 0;}
  virtual DWORD getMinMDefence();
  virtual DWORD getMaxMDefence(); 
  virtual DWORD getMinPDefence();
  virtual DWORD getMaxPDefence();
  virtual DWORD getMaxHP();
  virtual DWORD getBaseMaxHP();

  ///次攻击目标
  DWORD secondTargetType;
  DWORD secondTargetID;
  bool setSecondTarget(SceneEntryPk *);
  bool setSecondTarget(DWORD,DWORD);
  bool chaseSecondTarget();
  bool chaseItsMaster();
  SceneEntryPk * getSecondTarget();
  void hideMe(int showDelay);
  void goToRandomScreen();

  /**
   * \brief 物品保护
   *
   */
  DWORD dwNpcLockedUser;
protected:
  /// NPC搜索敌人的范围
  static const int npc_search_region = 5;
  /// NPC远离目标放弃追踪的范围
  static const int npc_lost_target_region = 12;
  /// NPC远离活动范围放弃追踪的距离
  static const int npc_out_of_range_region = 20;
  /// 宠物保持在主人身边的范围
  static const int npc_pet_chase_region = 2;
  /// 宠物离主人超过此距离则加速
  static const int npc_pet_run_region = 4;
  /// 宠物离主人超过此距离则跳转
  static const int npc_pet_warp_region = 6;
private:
  /**
   * \brief NPC可清除
   *
   */
  bool clearMe;

  ///npc当前的AI
  t_NpcAIDefine AIDefine;

  ///npc的主人
  //SceneUser * master;

  ///移动速度倍率
  float speedRate;
  ///攻击速度倍率
  float aspeedRate;

  ///是否已经因为hp<20%提升了移动速度
  bool speedUpUnder20;

  ///是否已经因为hp<50%提升了攻击速度
  bool aspeedUpUnder50;


  bool processDeath(SceneEntryPk *pAtt);

  ///是否正在后退中(ms)
  int backOffing;

  /**
   * \brief 第一次被攻击时间(测试用
   *
   */
  /// 第一次被攻击的时间
  zRTime first_time;

  ///半秒定时器
  Timer _half_sec;

  ///1秒定时器
  Timer _one_sec;

  ///3秒定时器
  Timer _3_sec;

  /**
   * \brief 跟踪方式
   *
   */
  ///npc的跟踪方式
  SceneNpcChase chaseMode;

  //DWORD  dwNpcChasedEntryType;
  /**
   * \brief 所跟踪目标的的编号
   *
   */
  //DWORD curTarget;
  //DWORD  dwNpcChasedEntryID;

  /**
   * \brief 物品保护时间
   *
   */
  zRTime lockedUserTime;
  /**
   * \brief 下一次移动时间
   *
   */
  zRTime nextMoveTime;
  /**
   * \brief 下一次攻击时间
   *
   */
  zRTime nextAttackTime;

  ///结束隐身状态的时间
  zRTime showTime;

  /**
   * \brief 是否可以掉落物品
   *
   */
  bool lostObject;

  //是否进行怪物攻城的判断
  bool mayRush;

  /**
   * \brief Npc类型
   * 静态的还是动态分配的
   */
  const SceneNpcType type;

  /**
   * \brief 临时编号的线性分配器
   * 主要在创建静态Npc的时候需要使用
   */
  static DWORD serialID;
  /**
   * \brief 临时编号的唯一分配器
   * 主要在创建动态Npc的时候需要使用
   */
  static zUniqueDWORDID uniqueID;

  //typedef hash_map<DWORD,t_expRec> NpcHpHashmap;
  typedef std::map<DWORD,t_expRec> NpcHpHashmap;
  typedef NpcHpHashmap::iterator NpcHpHashmap_iterator;
  typedef NpcHpHashmap::const_iterator NpcHpHashmap_const_iterator;
  typedef NpcHpHashmap::value_type NpcHpHashmap_pair;
  ///经验值列表
  ///可以分到该npc经验的玩家列表
  NpcHpHashmap expmap;


public:
  /**
   * \brief 设置角色的当前状态,并根据当前状态呈现角色的特效或者关闭特效
   * \param state 状态ID 根据enum SceneEntryState取值
   * \param isShow 是否显示效果
   * \return true为这次攻击是有效的,false为一次无效的攻击
   */
  void showCurrentEffect(const WORD &state,bool isShow,bool notify=true);
  /**
   * \brief NPC尸体的使用状态
   * true为已经被使用
   */
  bool isUse;
};

/**
 * \brief 对每个特殊npc执行的回调
 *
 */
struct specialNpcCallBack
{
  public:
    virtual bool exec(SceneNpc *npc)=0;
    virtual ~specialNpcCallBack(){};
};

/**
 * \brief 召唤一个npc
 * \param define npc定义结构
 * \param pos 召唤位置
 * \param base npc基本信息
 * \param standTime 图腾系的持续时间
 * \param abase 增强npc的基本信息
 * \return 召唤出npc的指针,失败返回0
 */
  template <typename Npc>
Npc* Scene::summonOneNpc(const t_NpcDefine &define,const zPos &pos,zNpcB *base,DWORD standTime,zNpcB* abase,BYTE vdir, SceneEntryPk * m)
{
  t_NpcDefine *pDefine = new t_NpcDefine(define);
  if (pDefine)
  {
    Npc *sceneNpc = new Npc(this,base,pDefine,SceneNpc::GANG,zSceneEntry::SceneEntry_NPC,abase);
    if (sceneNpc)
    {
      sceneNpc->setDir(vdir);
      sceneNpc->setStandingTime(standTime);
      initNpc(sceneNpc,NULL,pos);//zPos(0,0));//填NULL则在define.region范围内选择位置
      if (sceneNpc->getState() == zSceneEntry::SceneEntry_Normal)
      {
        if (base->kind != NPC_TYPE_TRAP)
        {
          Cmd::stAddMapNpcMapScreenUserCmd addNpc;
          sceneNpc->full_t_MapNpcData(addNpc.data);
          sendCmdToNine(sceneNpc->getPosI(),&addNpc,sizeof(addNpc));
          Cmd::stRTMagicPosUserCmd ret;
          sceneNpc->full_stRTMagicPosUserCmd(ret);
          sendCmdToNine(sceneNpc->getPosI(),&ret,sizeof(ret));
        }
        else
        {
          SceneEntryPk *entry = sceneNpc->getMaster();
          if (entry&& entry->getType() == zSceneEntry::SceneEntry_Player)
          {
            SceneUser *pUser = (SceneUser *)entry;
            Cmd::stAddMapNpcMapScreenUserCmd addNpc;
            sceneNpc->full_t_MapNpcData(addNpc.data);
            pUser->sendCmdToMe(&addNpc,sizeof(addNpc));
            Cmd::stRTMagicPosUserCmd ret;
            sceneNpc->full_stRTMagicPosUserCmd(ret);
            pUser->sendCmdToMe(&ret,sizeof(ret));
          }
        }
      }
#ifdef _DEBUG
      else
        Zebra::logger->debug("%s 初始状态 %u",sceneNpc->name,sceneNpc->getState());
#endif
      return sceneNpc;
    }
    else
    {
      Zebra::logger->fatal("Scene::summonOneNpc:SceneNpc分配内存失败");
      SAFE_DELETE(pDefine);
    }
  }
  else
  {
    Zebra::logger->fatal("Scene::summonOneNpc:t_NpcDefine分配内存失败");
  }
  return NULL;
}

struct petBonus
{
  DWORD type;
  WORD atkB;
  WORD defB;
  WORD hpB;

  /*
  petBonus()
  {
    type = 0;
    atkB = 0;
    defB = 0;
    hpB = 0;
  }
  petBonus(DWORD p,WORD a,WORD d,WORD h)
  {
    type = p;
    atkB = a;
    defB = d;
    hpB = h;
  }
  petBonus(petBonus & pb)
  {
    type = pb.type;
    atkB = pb.atkB;
    defB = pb.defB;
    hpB = pb.hpB;
  }
  */
};

class ScenePet : public SceneNpc
{
  private:
    /*
       static std::map<DWORD,petBonus> bonusTable;
  */

  ///主人
  //SceneEntryPk * master;
  DWORD masterID;
  DWORD masterType;

  DWORD delCount;

  ///宠物类型
  Cmd::petType type;
  ///宠物的行动模式
  //WORD petAI;

  ///等级
  //DWORD level;

  ///是否因为离主人太远而提升了速度
  bool speedUpOffMaster;
  int isUserMasterEnemy(SceneEntryPk *);
public:

  //bool needSave;
  ScenePet(Scene* scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype=SceneEntry_NPC,zNpcB *abase=NULL);

  Cmd::t_PetData petData;

  void setPetAI(Cmd::petAIMode);
  WORD getPetAI();
  int isEnemy(SceneEntryPk *,bool = false,bool good = false);
  SceneEntryPk * chooseEnemy(SceneEntryPk_vec &);
  bool isRedNamed(bool allRedMode=true);
  void returnToRegion();
  void full_PetDataStruct(Cmd::t_PetData &);
  virtual bool moveToMaster();
  bool randomMove();
  bool isActive();
  bool masterIsAlive;
  virtual bool canMove();
  bool canFight();
  bool runOffEnemy(SceneEntryPk_vec&);

  bool checkMasterTarget(SceneEntryPk *&entry);
  void setMaster(SceneEntryPk *);
  void setMaster(DWORD,DWORD);
  SceneEntryPk * getMaster();
  DWORD getMasterID() const{return masterID;}
  SceneEntryPk * getTopMaster();
  void clearMaster();

  Cmd::petType getPetType();
  void setPetType(Cmd::petType);
  void petDeath();
  virtual void sendData();
  virtual void sendHpExp();
  virtual DWORD getLevel() const;
  virtual bool addExp(DWORD);
  void addPetExp(DWORD);

  bool isPkZone(SceneEntryPk *other=NULL);
  bool recover();

  //给GuardNpc继承用
  virtual void reset(){}
  virtual void check(){};
  virtual void on_death(SceneEntryPk* att){}
  virtual void setAppendDamage(WORD mindamage,WORD maxdamage);
  DWORD getMasterMana();
  void changeAndRefreshHMS(bool lock=true,bool sendData=true);

  virtual DWORD getMaxHP();
  virtual DWORD getBaseMaxHP();
  virtual DWORD getMinMDamage();
  virtual DWORD getMaxMDamage(); 
  virtual DWORD getMinPDamage();
  virtual DWORD getMaxPDamage();
  virtual DWORD getMinMDefence();
  virtual DWORD getMaxMDefence();
  virtual DWORD getMinPDefence();
  virtual DWORD getMaxPDefence();

  virtual void levelUp();
  void getAbilityByLevel(DWORD);

  virtual bool normalAction(){return SceneNpc::normalAction();}

  void full_t_MapPetData(Cmd::t_MapPetData &data);

  virtual void sendMeToNine();
  virtual void sendPetDataToNine();

  virtual void delMyself();
};

/**
 * \brief npc管理器
 *
 */
class SceneNpcManager : public zEntryManager< zEntryTempID >
{
  public: 

    bool init();
    bool addSceneNpc(SceneNpc *sceneNpc);
    bool addSpecialNpc(SceneNpc *,bool=false);
    void removeSceneNpc(SceneNpc *sceneNpc);

    SceneNpc *getNpcByTempID(DWORD tempid);
    static SceneNpcManager &getMe();
    static void destroyMe();
    /**
     * \brief 对每个npc执行回调函数
     *
     *
     * \param exec 回调函数
     * \return 是否继续执行
     */
    template <class YourNpcEntry>
    bool execEveryNpc(execEntry<YourNpcEntry> &exec)
    {
      rwlock.rdlock();
      bool ret=execEveryEntry<>(exec);
      rwlock.unlock();
      return ret;
    }

    /**
     * \brief 删除符合条件的npc
     *
     * \param pred 判断条件
     */
    template <class YourNpcEntry>
    void removeNpc_if(removeEntry_Pred<YourNpcEntry> &pred)
    {
      rwlock.wrlock();
      removeEntry_if<>(pred);
      rwlock.unlock();
    }

    /**
     * \brief 删除一场景内的npc
     *
     *
     * \param scene 场景
     */
    void removeNpcInOneScene(Scene *scene);
    void SpecialAI();
    /**
     * \brief 对每个特殊npc执行回吊调函数
     *
     *
     * \param callback 回调函数
     * \return 
     */
    void execAllSpecialNpc(specialNpcCallBack &callback)
    {
      rwlock.wrlock();
      for (MonkeyNpcs::iterator it=specialNpc.begin(); it!=specialNpc.end(); it++)
      {
        if (!callback.exec(*it))
        {
          rwlock.unlock();
          return;
        }
      }
      rwlock.unlock();
    }

    MonkeyNpcs &getSepcialNpc()
    {
      return specialNpc;
    }
    bool getNpcCommonChat(DWORD type,char * content);
    void removeSpecialNpc(SceneNpc *);
    void closeFunctionNpc();
  private:
    ///specialNpc是指即使没有玩家在附近也要做处理的NPC
    ///包括宠物、boss、任务NPC
    MonkeyNpcs specialNpc;

    SceneNpcManager();
    ~SceneNpcManager();

    ///SceneNpcManager的唯一实例
    static SceneNpcManager *snm;
    ///读写锁
    zRWLock rwlock;

    bool getUniqeID(DWORD &tempid) { return true; }       
    void putUniqeID(const DWORD &tempid) {}

    ///npc随机说话的内容
    std::map<DWORD,std::vector<std::string> > NpcCommonChatTable;
    ///npc随机说话的概率
    std::map<DWORD,int> NpcCommonChatRate;

    bool loadNpcCommonChatTable();
};

/**
 * \brief 服务器连接任务
 *
 */
class SceneTask : public zTCPTask,public zEntry,public MessageQueue
{

  public:

    /**
     * \brief 构造函数
     *
     * \param pool 所属连接池指针
     * \param sock TCP/IP套接口
     * \param addr 地址
     */
    SceneTask(
        zTCPTaskPool *pool,
        const SOCKET sock,
        const struct sockaddr_in *addr = NULL) : zTCPTask(pool,sock,addr)
    {
      wdServerID    = 0;
      wdServerType  = UNKNOWNSERVER;
      recycle_state = 0;
      veriry_ok     = false; 
    }

    /**
     * \brief 虚析构函数
     *
     */
    virtual ~SceneTask();

    int verifyConn();
    int waitSync();
    int recycleConn();
    bool uniqueAdd();
    bool uniqueRemove();
    bool msgParse(const Cmd::t_NullCmd *,const DWORD);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);

    /**
     * \brief 返回服务器编号
     *
     * 编号在一个区中是唯一的，保存在管理服务器中
     *
     * \return 服务器编号
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief 返回服务器类型
     *
     * \return 服务器类型
     */
    const WORD getType() const
    {
      return wdServerType;
    }
    bool checkRecycle();

  private:

    bool usermsgParse(SceneUser *pUser,const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    bool usermsgParseBill(SceneUser *pUser,const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen);
    bool loginmsgParse(const Cmd::t_NullCmd *pNullCmd,DWORD cmdLen);
    WORD wdServerID;
    WORD wdServerType;

    bool verifyLogin(const Cmd::Scene::t_LoginScene *ptCmd);
    int recycle_state;
    bool veriry_ok;


};

/**
 * \brief 场景服务器子连接管理器
 *
 */
class SceneTaskManager : private zNoncopyable
{

  public:

    /**
     * \brief 析构函数
     *
     */
    ~SceneTaskManager() {};

    /**
     * \brief 获取场景子连接管理器唯一实例
     *
     * \return 场景子连接管理器唯一实例
     */
    static SceneTaskManager &getInstance()
    {
      if (NULL == instance)
        instance = new SceneTaskManager();

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

    bool uniqueAdd(SceneTask *task);
    bool uniqueRemove(SceneTask *task);
    SceneTask *uniqueGet(WORD wdServerID);
    void execEvery();
    bool broadcastCmd(const void *pstrCmd,const int nCmdLen);
    template <class YourEntry>
    void execEverySceneTask(execEntry<YourEntry> &exec)
    {
      rwlock.rdlock();
      SceneTaskHashmap_iterator it;
      it = sceneTaskSet.begin();
      for (; it != sceneTaskSet.end() ; it ++)
      {
        exec.exec(it->second);
      }
      rwlock.unlock();
    }

  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static SceneTaskManager *instance;

    /**
     * \brief 构造函数
     *
     */
    SceneTaskManager() {};

    /**
     * \brief 子连接管理容器类型
     *
     */
    typedef hash_map<WORD,SceneTask *> SceneTaskHashmap;
    /**
     * \brief 定义容器迭代器类型
     *
     */
    typedef SceneTaskHashmap::iterator SceneTaskHashmap_iterator;
    /**
     * \brief 定义容器常量迭代器类型
     *
     */
    typedef SceneTaskHashmap::const_iterator SceneTaskHashmap_const_iterator;
    /**
     * \brief 定义容器键值对类型
     *
     */
    typedef SceneTaskHashmap::value_type SceneTaskHashmap_pair;
    /**
     * \brief 容器访问互斥变量
     *
     */
    zRWLock rwlock;
    /**
     * \brief 子连接管理容器类型
     *
     */
    SceneTaskHashmap sceneTaskSet;

};

/**
 * brief 定义 档案服务器连接客户端类
 *
 * 负责 与档案服务器交互，存取档案
 * TODO 暂时只有一个档案服务器
 * 
 */
class RecordClient : public zTCPBufferClient,public MessageQueue
{

  public:

    /**
     * \brief 构造函数
     * 由于档案数据已经是压缩过的，故在底层传输的时候就不需要压缩了
     * \param name 名称
     * \param ip 地址
     * \param port 端口
     */
    RecordClient(const std::string &name,const std::string &ip,const WORD port)
      : zTCPBufferClient(name,ip,port,false) {};

    bool connectToRecordServer();

    void run();
    bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);

};

extern RecordClient *recordClient;

class SessionClient : public zTCPBufferClient,public MessageQueue
{

  public:
    /**
    * \brief 构造函数
    * \param  name 名称
    * \param  ip   地址
    * \param  port 端口
    */
    SessionClient(
        const std::string &name,
        const std::string &ip,
        const WORD port)
      : zTCPBufferClient(name,ip,port) {};

    bool connectToSessionServer();
    void run();

    void requestFriendDegree(SceneUser *pUser);
    bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool doGmCmd(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
    bool doAuctionCmd(const Cmd::Session::t_AuctionCmd *,const DWORD);
    bool doCartoonCmd(const Cmd::Session::t_CartoonCmd *,const DWORD);
    bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD);
    bool cmdMsgParse_Country(const Cmd::t_NullCmd*,const DWORD);
    bool cmdMsgParse_Dare(const Cmd::t_NullCmd*,const DWORD);
    bool cmdMsgParse_Recommend(const Cmd::t_NullCmd*,const DWORD);
    bool cmdMsgParse_Other(const Cmd::t_NullCmd*,const DWORD);
    bool cmdMsgParse_Temp(const Cmd::t_NullCmd*,const DWORD);
    bool cmdMsgParse_Army(const Cmd::t_NullCmd* pNullCmd,const DWORD nCmdLen);
    bool cmdMsgParse_Sept(const Cmd::t_NullCmd*,const DWORD);
    bool cmdMsgParse_Union(const Cmd::t_NullCmd*,const DWORD);
    bool cmdMsgParse_Gem(const Cmd::t_NullCmd* pNullCmd,const DWORD nCmdLen);

};

/// 声明
extern SessionClient *sessionClient;

/**
 * \brief NPC买卖
 *
 */
class NpcTrade
{

  public:

    enum
    {
      NPC_BUY_OBJECT    = 1, ///买
      NPC_SELL_OBJECT    = 2, ///卖
      NPC_REPAIR_OBJECT  = 4, ///修理
      NPC_MAKE_OBJECT    = 8, ///打造
      NPC_UPDATE_OBJECT  = 16, ///改造
      NPC_MERGE_OBJECT  = 32, ///合成
      NPC_ENCHANCE_OBJECT = 64,//镶嵌
      NPC_MERGE_SOUL_OBJECT = 128,//魂魄合成
      NPC_HOLE_OBJECT = 256,//打孔
      NPC_STORE_OBJECT = 512,//仓库
      NPC_DECOMPOSE_OBJECT = 1024,//分解
    };

    struct NpcItem
    {
      DWORD id;          ///物品编号
      WORD  kind;          ///物品类型
      WORD  lowLevel;        ///最低等级
      WORD  level;        ///最高等级
      WORD  itemlevel;      ///购买物品的等级
      WORD  action;        ///动作类型
      NpcItem()
      {
        id = 0;
        kind = 0;
        lowLevel = 0;
        level = 0;
        itemlevel = 0;
        action = 0;
      }
    };

    ~NpcTrade()
    {
      final();
    }

    /**
     * \brief 得到唯一实例
     *
     *
     * \return npc买卖系统
     */
    static NpcTrade &getInstance()
    {
      if (NULL == instance)
        instance = new NpcTrade();

      return *instance;
    }

    /**
     * \brief 卸载唯一实例
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    bool init();
    bool getNpcMenu(const DWORD npcid,char *menuTxt);
    bool verifyNpcAction(const DWORD npcid,const NpcItem &item);

  private:

    NpcTrade() {};

    void final();

    static NpcTrade *instance;

    typedef hash_multimap<DWORD,NpcItem> NpcItemMultiMap;

    /**
     * \brief npc对话框
     *
     */
    struct NpcDialog
    {
      DWORD npcid;      ///Npc编号
      char menu[6144];    ///菜单内容
      NpcItemMultiMap items;  ///物品动作
      NpcDialog()
      {
        npcid = 0;
        bzero(menu,sizeof(menu));
      }
      NpcDialog(const NpcDialog& nd)
      {
        npcid = nd.npcid;
        bcopy(nd.menu,menu,sizeof(menu),sizeof(menu));
        for(NpcItemMultiMap::const_iterator it = nd.items.begin(); it != nd.items.end(); it++)
        {
          items.insert(*it);
        }
      }
    };

    typedef hash_multimap<DWORD,NpcDialog> NpcDialogMultiMap;

    NpcDialogMultiMap dialogs;
    zRWLock rwlock;

};

/**
 * \brief 镖车
 *
 * 该类封装了对特殊NPC镖车的抽象
 *
 */
class GuardNpc : public ScenePet
{
public:  
  GuardNpc(Scene *scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype = SceneEntry_NPC,zNpcB *abase=NULL);
  
  ~GuardNpc();
  
  void owner(SceneUser* user);  
  SceneUser* owner();
  
  void gold(int money);
  int gold() const;
  
  void exp(int experience);
  int exp() const;

  void dest(const zPos& pos);
  void map(const std::string& name);
  
  void reset();
  void check();

  void on_death(SceneEntryPk*);

  //bool masterIsAlive;
  bool canMove();
  bool moveToMaster();
  DWORD save(BYTE * dest);

  bool isSeptGuard;
private:
  void on_reached();
  
  //SceneUser* _owner;
  std::string _name;
  std::string _map;
      
  int _exp;
  DWORD _gold;
  zPos _dest;
  
  int _status;
  
  zRTime _time;  

  DWORD getRobGold();
  //char ownerName[MAX_NAMESIZE];
};

#define SHELLITEM_IN_SEPT               20
#define SHELLITEM_IN_SEPT_TOP   20
#define SHELLITEM_IN_COUNTRY    80
#define SHELLITEM_IN_FRIEND             20
#define SHELLITEM_IN_TEAM               10
#define SHELLITEM_IN_UNION              50
#define SHELLITEM_IN_PRIVATE    10
#define SHELLITEM_IN_NINE               10
#define SHELLITEM_IN_PERSON    10
#define SHELLITEM_IN_ZONE  10

/**
 * \brief 聊天频道
 *
 */
class Channel:public zEntry
{
        private:
                std::vector<zEntryC> userlist;
                zEntry creater;
        public:
                static bool sendNine(SceneUser *pUser,const Cmd::stChannelChatUserCmd *rev,DWORD cmdLen);
                static bool sendNine(SceneUser *pUser,const char *pattern,...);
                static bool sendNine(const SceneNpc *pNpc,const char *pattern,...);
                static bool sendPrivate(SceneUser *pUser,const Cmd::stChannelChatUserCmd *rev,DWORD cmdLen);
                static bool sendTeam(DWORD teamid,const Cmd::stChannelChatUserCmd *rev,DWORD cmdLen);
                static bool sendCountry(SceneUser *pUser,const Cmd::stChannelChatUserCmd *rev,DWORD cmdLen);
                bool sendToAll(SceneUser *pUser,const Cmd::stChannelChatUserCmd *rev,DWORD cmdLen);
                static bool sendMapInfo(DWORD mapID,DWORD infoType,const char *pattern,...);
                static bool sendMapInfo(Scene * s,DWORD infoType,const char *pattern,...);
                static bool sendCmdToMap(DWORD mapID,const void *cmd,int len);
                static bool sendCmdToMap(Scene * scene,const void *cmd,int len);

                static bool sendNine(SceneUser *pUser,const char *content);
                static bool sendPrivate(SceneUser *pUser,const char *toName,const char *pattern,...);
                static bool sendPrivate(const char *src_name,const char *toName,const char *pattern,...);
                static bool sendSys(SceneUser *pUser,int type,const char *pattern,...);
                static bool sendMoney(SceneUser *pUser,int type,DWORD money,const char *pattern,...);
                static bool sendGold(SceneUser *pUser,int type,DWORD gold,const char *pattern,...);
                static bool sendSys(DWORD dwUserTempID,int type,const char *pattern,...);
                static bool sendTeam(DWORD teamid,const char *pattern,...);
                static bool sendCountry(SceneUser *pUser,const char *pattern,...);
                static bool sendCountryInfo(DWORD countryID,DWORD infoType,const char *pattern,...);
                static bool sendAllInfo(DWORD infoType,const char *pattern,...);

    static bool doPreCheck(SceneUser *pUser,Cmd::stChannelChatUserCmd *cchat,DWORD cmdLen);
                bool sendToAll(SceneUser *pUser,const char *pattern,...);
                bool sendCmdToAll(const void *cmd,int len);
                Channel(SceneUser *pUser);
                bool remove(const char *name);
                bool add(const char *name);
                bool add(SceneUser *pUser);
                WORD has(const char *name);
};

/**
 * \brief 频道管理器
 *
 */
class ChannelM:public zEntryManager< zEntryTempID,zEntryName >
{
        private:
                static ChannelM * cm;
                zUniqueDWORDID *channelUniqeID;
                zMutex mlock;

                ChannelM();
                ~ChannelM();

                bool getUniqeID(DWORD &tempid);
                void putUniqeID(const DWORD &tempid);

    public:
                static ChannelM &getMe();
                static void destroyMe();
                bool add(Channel *channel);
                void remove(DWORD dwChannelID);
                Channel *get(DWORD dwChannelID);
                void removeUser(const char *name);
};

/// 超级GM的id,只有1个超级GM
#define SUPER_GM_ID 1

struct Gm
{       
        enum
    {       
      none_mode =   0x00,
      normal_mode =   0x01,
      gm_mode =    0x02,
      captain_mode =   0x04,
      super_mode =   0x08,
      debug_mode =   0x10,
      all_mode =   0x1f,/// all_mode
      //admin_mode =   0x10,
      //magic_mode =   0x20
    };
  
        const char *cmd;
        bool (*parse)( SceneUser *pUser,const char *para);
  BYTE priv;
        const char *desc;
  
        static void sendLog(SceneUser *pUser,const char *cmd,const char * content); 

        static void exec(SceneUser *pUser,char *cmd); 
        static bool fetch(SceneUser *pUser,const char *para);
        static bool getgive(SceneUser *pUser,const char *para);
        static bool summon(SceneUser *pUser,const char *para);
        static bool help(SceneUser *pUser,const char *para);
        static bool levelup(SceneUser *pUser,const char *para);
        static bool goTo(SceneUser *pUser,const char *para);
        static bool goTo_Gm(SceneUser *pUser,const char *para);
        static bool gomap(SceneUser *pUser,const char *para);
        static bool gomap_Gm(SceneUser *pUser,const char *para);
        static bool upgradeSkill(SceneUser *pUser,const char *para);
        static bool abandon(SceneUser* pUser,const char* para);
        static bool finduser(SceneUser* pUser,const char* para);
        static bool gotouser(SceneUser* pUser,const char* para);
        static bool catchuser(SceneUser* pUser,const char* para);
  static bool bczone(SceneUser* pUser,const char* para);
  static bool bcworld(SceneUser* pUser,const char* para);
  static bool bcwgamemsg(SceneUser *pUser,const char *para);
  static bool leechdom(SceneUser* pUser,const char* para);
  static bool value(SceneUser* pUser,const char* para);
  static bool team(SceneUser* pLeader,const char* para);
  static bool throwobject(SceneUser* pLeader,const char* para);
  static bool querypoint(SceneUser* pLeader,const char* para);
  static bool redeemgold(SceneUser* pLeader,const char* para);
  static bool querygold(SceneUser* pLeader,const char* para);
  static bool redeemmonthcard(SceneUser* pLeader,const char* para);
  static bool goldsystem(SceneUser* pLeader,const char* para);
  static bool stockconsign(SceneUser* pLeader,const char* para);
  static bool stocktransfer(SceneUser* pLeader,const char* para);
  static bool stockuser(SceneUser* pLeader,const char* para);
  static bool stocksystem(SceneUser* pLeader,const char* para);
  static bool givestock(SceneUser* pLeader,const char* para);
  static bool givemoney(SceneUser* pLeader,const char* para);
  static bool givegold(SceneUser* pLeader,const char* para);
  static bool backoff(SceneUser* pUser,const char* para);
  static bool goodness(SceneUser* pUser,const char* para);
  static bool kick(SceneUser* pUser,const char* para);
  static bool kickGateUser(SceneUser* pUser,const char* para);
  static bool donttalk(SceneUser* pUser,const char* para);
  static bool talk(SceneUser* pUser,const char* para);
  static bool setPriv(SceneUser* pUser,const char* para);
  static bool hideMe(SceneUser* pUser,const char* para);
  static bool showMe(SceneUser* pUser,const char* para);
  static bool goHome(SceneUser* pUser,const char* para);
  static bool loadMap(SceneUser* pUser,const char* para);
  static bool unloadMap(SceneUser* pUser,const char* para);
  static bool createRush(SceneUser* pUser,const char* para);
  static bool createQuiz(SceneUser* pUser,const char* para);
  static bool clearWorld(SceneUser* pUser,const char* para);
  static bool clearSeptUnion(SceneUser* pUser,const char* para);
  static bool createFamily(SceneUser* pUser,const char* para);
  static bool viewCountryDare(SceneUser* pUser,const char* para);
  static bool loadProcess(SceneUser *pUser,const char * para);
  static bool loadGift(SceneUser *pUser,const char * para);
  static bool clearArea(SceneUser *pUser,const char *para);
  static bool embar(SceneUser *pUser,const char *para);
  static bool closenpc(SceneUser *pUser,const char * para);
  static bool countrypower(SceneUser *pUser,const char * para);
  static bool refreshGeneral(SceneUser *pUser,const char * para);

  static bool createUnion(SceneUser* pUser,const char* para);
  static bool createSchool(SceneUser* pUser,const char* para);
  static bool showEntries(SceneUser* pUser,const char* para);
  static bool getsize(SceneUser *pUser,const char *para);
  static bool setPetAI(SceneUser *pUser,const char *para);
  static bool setPetAIF(SceneUser *pUser,const char *para);
  static bool addExp(SceneUser *pUser,const char *para);
  static bool countServerUser(SceneUser* pUser,const char* para);
  static bool shutdown(SceneUser* pUser,const char* para);
  static bool systime(SceneUser* pUser,const char* para);
  static bool usleep(SceneUser* pUser,const char* para);
  static bool qAccount(SceneUser *pUser,const char *para);
  static bool setQuest(SceneUser* pUser,const char* para);
  static bool checkQuest(SceneUser* pUser,const char* para);
  static bool debugVote(SceneUser* pUser,const char* para);
  static bool debugGem(SceneUser* pUser,const char* para);
  static bool addSeptExp(SceneUser* pUser,const char* para);

  static bool setRepute(SceneUser* pUser,const char* para);
  static bool setSeptLevel(SceneUser* pUser,const char* para);
  static bool setAllyFriendDegree(SceneUser* pUser,const char* para);
  static bool changeCountry(SceneUser* pUser,const char* para);
  static bool debugCityDare(SceneUser* pUser,const char* para);
  static bool debugEmperorDare(SceneUser* pUser,const char* para);
  static bool setService(SceneUser* pUser,const char* para);
  static bool setTire(SceneUser *pUser,const char * para);
  static bool enableLogin(SceneUser *pUser,const char * para);
  static bool enableRegister(SceneUser *pUser,const char * para);
  static bool checkCountryInfo(SceneUser *pUser,const char * para);
  static bool version(SceneUser *pUser,const char * para);
  
  /**
   * \brief 技能测试指令
   */
  static bool skill(SceneUser *pUser,const char *para);
  static bool lockValue(SceneUser *pUser,const char *para);
  static bool getvalue(SceneUser *pUser,const char *para);
  static bool setvalue(SceneUser* pUser,const char* para);
  static bool god(SceneUser *pUser,const char *para);
  static bool killer(SceneUser *pUser,const char *para);
  static bool newzone(SceneUser *pUser,const char *para);
  static bool normal(SceneUser *pUser,const char *para);
  static bool svote(SceneUser *pUser,const char *para);
  static bool uvote(SceneUser *pUser,const char *para);
  static bool tong(SceneUser *pUser,const char *para);
  static bool callPet(SceneUser *pUser,const char *para);
  static bool showMaps(SceneUser* pUser,const char* para);
  static bool showPets(SceneUser* pUser,const char* para);
  static bool killPets(SceneUser* pUser,const char* para);
  static bool showSpecialNpc(SceneUser* pUser,const char* para);
  static bool bank(SceneUser* pUser,const char* para);
  static bool setBlock(SceneUser* pUser,const char* para);
  static bool checkBlock(SceneUser* pUser,const char* para);
  static bool npcDare(SceneUser *pUser,const char * para);
  static bool clearPoint(SceneUser *pUser,const char * para);
  static bool clearSkillPoint(SceneUser *pUser,const char * para);
  static bool studySkill(SceneUser *pUser,const char * para);
  static bool clearColddown(SceneUser *pUser,const char * para);
  static bool queryincmap(SceneUser *pUser,const char * para);
  static bool showAddExp(SceneUser *pUser,const char * para);
  static bool changeHorse(SceneUser *pUser,const char * para);
  static bool contribute(SceneUser *pUser,const char * para);
  static bool clearSkill(SceneUser *pUser,const char * para);
};

extern bool sendCmdByID(DWORD id,const void *cmd,int len);
extern bool sendCmdByTempID(DWORD tempid,const void *cmd,int len);
extern bool sendCmdByName(char * name,const void *cmd,int len);

class CartoonPet : public ScenePet
{
  private:
    DWORD cartoonID;
    Cmd::t_CartoonData cartoonData;

    BYTE expRate;//释放经验的速率
    Timer _5_sec;
    DWORD _5_sec_count;

    void releaseExp();
  public:
    void releaseExp(DWORD);

    CartoonPet(Scene *scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype = SceneEntry_NPC,zNpcB *abase=NULL);

    ~CartoonPet(){};

    Cmd::t_CartoonData& getCartoonData();
    void setCartoonData(Cmd::t_CartoonData&);

    void setMaster(SceneEntryPk *);
    void setMaster(DWORD,DWORD);
    void setCartoonID(DWORD);
    DWORD getCartoonID(){return cartoonID;}
    void setExpRate(BYTE);
    void recoverSp(DWORD);
    void setName(char *);

    void putAway(Cmd::Session::saveType saveType);
    void drawExp();
    bool normalAction();
    void sendData();
    void sendHpExp();
    bool addExp(DWORD);
    void levelUp();

    DWORD getLevel() const;
    bool isAdopted();

    void save(Cmd::Session::saveType saveType);
    DWORD save(BYTE * dest);

    void delMyself();
};

class Dice
{
  public:
    Dice(SceneUser * u1,SceneUser * u2,DWORD money);
    ~Dice(){};

    enum DiceState
    {
      DICE_STATE_CREATE,//未初始化，可以聊天
      DICE_STATE_ROLLING,//色子在转，等待停止消息
      DICE_STATE_END,//一局开始之前等待双方准备
      DICE_STATE_DEL//等待删除
    };

    bool init();

    bool sendCmdToAll(const void *cmd,const DWORD len);
    bool sendAllInfo(const char *pattern,...);

    void endGame(SceneUser * user=0);
    bool rotate(DWORD id);
    bool setReady(SceneUser * user);

    DWORD getMoney();
    DWORD getTheOtherTempID(DWORD id);
    bool judge();

    DiceState getState();
    bool timer(DWORD time,SceneUser * u);
  private:
    DWORD startTime;

    DWORD round;//n周目
    DWORD money;
    DiceState gameState;

    DWORD tempid1,tempid2;
    DWORD value1,value2;
    bool continue1,continue2;
    char name1[MAX_NAMESIZE],name2[MAX_NAMESIZE];
};

/**
 * \brief 新手装备结构
 *
 */
struct InitObject
{
  ///
  DWORD id;
  char name[MAX_NAMESIZE+1];
  DWORD localeID;
  WORD  num;
  WORD  profession;

  InitObject()
  {
    id = 0;
    bzero(name,sizeof(name));
    localeID = 0;
    num = 0;
    profession = 0;
  }

  /**
   * \brief 构造函数
   *
   *
   * \param object: 新手装备
   */
  InitObject(const InitObject &object)
  {
    id = object.id;
    bcopy(object.name,name,sizeof(name),sizeof(name));
    localeID = object.localeID;
    num = object.num;
    profession = object.profession;
  }

};

typedef std::vector<InitObject> InitObjectVector;

/**
 * \brief 新手信息
 *
 */
class CharInitInfo
{

  public:

    ~CharInitInfo()
    {
      final();
    }

    /**
     * \brief 得到唯一实例
     *
     *
     * \return 唯一实例
     */
    static CharInitInfo &getInstance()
    {
      if (NULL == instance)
        instance = new CharInitInfo();

      return *instance;
    }

    /**
     * \brief 删除唯一实例
     *
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    bool init();
    void get(const WORD profession,InitObjectVector &objs);
    void final();

  private:

    static CharInitInfo *instance;

    CharInitInfo() {};

    typedef hash_multimap<WORD,InitObject> ObjectsContainer;
    zRWLock rwlock;
    ObjectsContainer objects;

};

namespace Cmd{
struct stPropertyUserCmd;
struct stFoundItemPropertyUserCmd;
}

class zObjectB;

/**
 * \brief 物品改造基类
 *
 * 封装了几个常用的操作
 *
 */  
class Base
{
public:  
  enum ReuildType{
    COMPOSE = 1,
    MAKE = 2,
    UPGRADE = 3,
    HOLE = 4,
    ENCHANCE = 5,
    DECOMPOSE = 6,
  };
  
  static bool check_npc(SceneUser& user,zObjectB* base,int action);
  
  static bool check_space(SceneUser& user,DWORD w,DWORD h);

  static bool remove_object(SceneUser& user,zObject* ob);
  
  static bool add_object(SceneUser& user,zObject* ob,bool add = true);

  static bool response(SceneUser& user,int status,ReuildType type);
    
  static void refresh_pack(SceneUser& user,ObjectPack* pack);
};

/**
 * \brief 孔
 *
 * 封装了对与孔相关的处理函数
 *
 */  
class Hole
{
public:
  enum {
    INVALID_INDEX = 4,
    INVALID_NUM = 5,
  };
  
  
  static bool can_hole(zObject * ob);
  
  static int get_empty_hole(zObject* ob);
  
  static int get_hole_num(zObject* ob);

  static int add_hole_num(zObject* ob,int num);
  
  static bool add_hole(zObject* ob,int index);

  static bool put_hole(zObject* ob,int index,int id);  

};

/**
 * \brief 魂魄石
 *
 * 封装了对魂魄石的处理函数,包括镶嵌及合成等
 *
 */  
class SoulStone
{
public:
  static zObject* compose(SceneUser& user,zObject* first,zObject* second,int odds);

  static bool enchance(SceneUser& user,zObject* dest,zObject* src);

  static int id(DWORD trait);
  
  static bool assign(zObject* ob,int monster);
  
private:
  static bool do_enchance(zObject* dest,zObject* src);
  
  static bool do_compose(zObject* first,zObject* second,zObject* dest);
  
  /**     
   * \brief 魂魄属性计算
   *
   * \param first: 第一块魂魄石属性
   * \param second: 第二块魂魄石属性
   * \param result: 计算结果
   * \param level:魂魄石等级
   * \return 无
   */  
  template<typename T>
  static void additive(T first,T second,T& result,int level)
  {
    if (first && second) {
      //result = std::max(first,second) + 0.1*std::min(first,second)  
      T max = first,min = second,grade = level & 0xff;
      if (max < min) {
        max = second;
        min = first;
        grade = level >> 8;
      }
      
      result =  max + static_cast<T>(min*0.1*grade);  
    }else {
      result = first + second;
    }
    
  }

  static const int _ids[];
};

/**
 * \brief 升级
 *
 * 实现了物品升级功能
 *
 */  
class Upgrade
{
public:
  static bool upgrade(SceneUser& user,zObject* ob,int extra_odds);

private:  
  static bool do_upgrade(zObject* ob,zUpgradeObjectB* base);    
  static bool do_downgrade(zObject* ob,zUpgradeObjectB* base);    

};

/**
 * \brief 物品分解
 *
 * 实现物品分解功能
 *
 */
class Decompose
{
public:  
  /**     
   * \brief 构造函数
   *
   * 初始化相关变量
   *
   * param ob : 待分解物品
   *
   */   
  Decompose(zObject* ob) : _ob(ob)
  { }
  
  bool bonus_items(SceneUser& user);
  bool bonus_exp(SceneUser& user);
  bool remove_from(SceneUser& user);  
  int gold() const;
    
private:
  
  int chance() const;
  int index() const;
  
  zObject* _ob;
  
  const static int _odds[];
  const static int _items[];
};

#define COMPUTE_R(x) additive(ob->data.x,bob->x);
#define COMPUTE_L(x) additive(ob->data.x,bob->x,property);

#define BONUS_SKILL std::vector<skillbonus>::iterator it = bob->skill.begin();  \
    bool must = false; \
    for ( ; it!=bob->skill.end(); ++it) { \
      int odds = odds_of_property(it->odds,property); \
      if (selectByTenTh(odds) || must) { \
        int i = 0; \
        while ( i <3) {\
          if (ob->data.skill[i].id == 0 || ob->data.skill[i].id==it->id) {\
            ob->data.skill[i].id = it->id; \
            ob->data.skill[i].point += it->level; \
            break;\
          }else {\
            ++i;\
          }\
          if (i>2 &&must) break;\
        }\
      } \
    }
//戒指,项链
#define BONUS_SKILL_RING {\
  if (!bob->skill.empty())\
  {\
    int index = randBetween(0,bob->skill.size()-1 );\
        int i = 0; \
        while ( i <3) {\
          if (ob->data.skill[i].id == 0 || ob->data.skill[i].id==bob->skill[index].id) {\
            ob->data.skill[i].id = bob->skill[index].id; \
            ob->data.skill[i].point += bob->skill[index].level; \
            break;\
          }else {\
            ++i;\
          }\
        }\
    }\
  }

#define BONUS_SKILLS int odds = odds_of_property(bob->skills.odds,property); \
    if (selectByTenTh(odds)) { \
      ob->data.skills.id = bob->skills.id; \
      ob->data.skills.point = bob->skills.level; \
    }

/**
 * \brief 打造
 *
 * 实现了物品打造功能
 *
 */  
class EquipMaker
{
  public:
    EquipMaker(SceneUser* user);

    /**     
     * \brief 析构函数
     *
     */     
    ~EquipMaker() { }

    bool check_skill(SceneUser& user,zObjectB* ob);

    bool add_skill(SceneUser& user,zObjectB* ob);
    bool add_exp(SceneUser& user,DWORD exp);

    bool check_material(zObjectB* ob,const std::map<DWORD,DWORD>& list,bool is_resource = false);

    //  void pre_level_of_material(zObjectB* base);
    void pre_level_of_material(int id,int level);

    bool level_of_material(DWORD id,DWORD num,DWORD level,zObjectB* base);

    bool is_odds_gem(DWORD kind,DWORD id);

    zObject* make_material(zObjectB* base);

    zObject* make(SceneUser* user,zObjectB* base,int flag = 0);

    void assign(SceneUser* user,zObject* ob,zObjectB* base,bool drop =false,int flag = 0);

    void fix(zObject* ob);

    void bonus_hole(zObject* ob);

  private:
    struct Odds
    {
      int per;
      int luck;
      double material_level;
      int skill_level;
      int odds;    
      int sleight;
      int odds_gem;
      Odds() : per(0),luck(0),material_level(0),skill_level(0),odds(0),sleight(0),odds_gem(0)
      { }
    };

    Odds _odds;
    double _current;
    double _base;

    bool _make;

    int _need;
    int _1_id;
    int _1_level;
    int _2_id;
    int _2_level;

    int odds_of_white(const zObjectB* ob);
    int odds_of_blue(const zObjectB* ob);
    int odds_of_gold(const zObjectB* ob);
    int odds_of_holy(int object);
    int odds_of_property(int object,int property);

    template <typename T>
      void fix_kind(T* bob,zObject* ob)
      {
        switch (ob->base->kind)
        {
          case ItemType_ClothBody ://101代表布质加生命类服装
            additivePercent(ob->data.maxhp,bob->maxsp);
            break;
          case ItemType_FellBody :      //102代表皮甲加魔防类服装
            additivePercent(ob->data.mdefence,bob->maxsp);  
            break;
          case ItemType_MetalBody:  //103代表金属铠甲加物防类服装
          case ItemType_Shield:   //112代表盾牌类
            additivePercent(ob->data.pdefence,bob->maxsp);      
            break;
          case ItemType_Blade:        //104代表武术刀类武器
          case ItemType_Sword:        //105代表武术剑类武器
          case ItemType_Axe:             //106代表武术斧类武器
          case ItemType_Hammer:          //107代表武术斧类武器
          case ItemType_Crossbow:          //109代表箭术弓类武器
            additivePercent(ob->data.pdamage,bob->maxsp);    
            additivePercent(ob->data.maxpdamage,bob->maxsp);    
            break;
          case ItemType_Staff:        //108代表法术杖类武器
          case ItemType_Stick:          //111代表召唤棍类武器
          case ItemType_Fan:             //110代表美女扇类
            additivePercent(ob->data.mdamage,bob->maxsp);    
            additivePercent(ob->data.maxmdamage,bob->maxsp);    
            break;
		  case ItemType_Helm:    //113代表角色头盔布
		  case ItemType_Caestus:  //114代表角色腰带布
		  case ItemType_Cuff:    //115代表角色护腕布
		  case ItemType_Shoes:    //116代表角色鞋子布
		  /*sky 新增板和皮类型防具支持*/
		  case ItemType_Helm_Paper: //头盔皮
		  case ItemType_Helm_Plate: //头盔板
		  case ItemType_Cuff_Paper: //护腕皮
		  case ItemType_Cuff_Plate: //护腕板
		  case ItemType_Caestus_Paper: //腰带皮
		  case ItemType_Caestus_Plate: //腰带板
		  case ItemType_Shoes_Paper: //靴子皮
		  case ItemType_Shoes_Plate: //靴子板
		  //sky 新增肩膀 手套 裤子类
		  case tyItemType_Shoulder:
		  case tyItemType_Gloves:
		  case tyItemType_Pants:
		  case ItemType_Shoulder_Paper:
		  case ItemType_Gloves_Paper:
		  case ItemType_Pants_Paper:
		  case ItemType_Shoulder_Plate:
		  case ItemType_Gloves_Plate:
		  case ItemType_Pants_Plate:
            if (randBetween(0,1)) {
              additivePercent(ob->data.pdefence,bob->maxsp);    
            }else {
              additivePercent(ob->data.mdefence,bob->maxsp);        
            }
            break;
          case ItemType_Necklace:  //117代表角色项链类
          case ItemType_Fing:    //118代表角色戒指类
            if (ob->data.pdamage || ob->data.maxpdamage) {
              additivePercent(ob->data.pdamage,bob->maxsp);    
              additivePercent(ob->data.maxpdamage,bob->maxsp);    
            }
            if (ob->data.mdamage || ob->data.maxmdamage) {
              additivePercent(ob->data.mdamage,bob->maxsp);    
              additivePercent(ob->data.maxmdamage,bob->maxsp);    
            }
            break;
        }

      }


    /**     
     * \brief 属性取最大值
     *
     * \param ret: 计算结果
     * \param lv: 属性取值范围
     * \return 无
     */  
      template<class T>
      bool max(T& ret,const luckRangeValue &rv)
      {
        if (selectByTenTh(rv.per) )  {
          ret += rv.data.max;

        }
        /*
        if (selectByPercent(_odds.sleight) ) {
          ret += rv.sleightValue;
          return true;
        }
        // */
        return false;
      }
    /**     
     * \brief 生成特殊装备
     *
     * \param bob: 对应装备基本表
     * \param ob: 打造物品
     * \param kind: 装备类型
     * \return 当前总是返回true
     */  
    template <typename T>
      bool assign_color(T* bob,zObject* ob,int kind,int props = 0,zObjectB *base=NULL,bool drop=false)
      {
        char tmp[MAX_NAMESIZE];
        snprintf(tmp,MAX_NAMESIZE,"%s%s%s",bob->prefix,bob->joint,ob->data.strName);
        strncpy(ob->data.strName,tmp,MAX_NAMESIZE);

        int property = 1;

        if (props >= 17)
        {
          ++property;
          //ob->data.fivepoint += randBetween(bob->five.data.min,bob->five.data.max);
          if (/*ob->data.fivepoint &&// */ ob->data.fivetype == FIVE_NONE) 
          {
            ob->data.fivetype = randBetween(0,4);
          }
        }
        else //if (additive(ob->data.fivepoint,bob->five,property)) 
        {
          //ob->data.fivepoint -= bob->five.sleightValue;
          if (/*ob->data.fivepoint && // */ob->data.fivetype == FIVE_NONE && selectByTenTh(bob->five.per)) 
          {
            ob->data.fivetype = randBetween(0,4);
          }
        }


        COMPUTE_R( pdamage )    // 最小物攻
          COMPUTE_R( maxpdamage )    // 最大物攻
          COMPUTE_R( mdamage )      // 最小魔攻
          COMPUTE_R( maxmdamage )    // 最大魔攻
          COMPUTE_R( pdefence )      // 物防
          COMPUTE_R( mdefence )      // 魔防

          if (props) {
            int index = randBetween(0,4);
            if (index!=5) {

              //现在不用随机了,直接取值
              additivePercent(*ob->_p1[index],bob->_p1[index]);
            }else {
              fix_kind(bob,ob);
            }
          }else {
            COMPUTE_L( str )      // 力量
              COMPUTE_L( inte )      // 智力
              COMPUTE_L( dex )      // 敏捷
              COMPUTE_L( spi )      // 精神
              COMPUTE_L( con )        // 体质
          }    

        COMPUTE_L( maxhp )    // 最大生命值
          COMPUTE_L( maxmp )    // 最大法术值
          //    COMPUTE_L( maxsp )    // 最大体力值

          COMPUTE_L( mvspeed )    // 移动速度
          COMPUTE_L( hpr )      // 生命值恢复
          COMPUTE_L( mpr )      // 法术值恢复
          COMPUTE_L( spr )      // 体力值恢复
          COMPUTE_L( akspeed )    // 攻击速度

          if (props) {
            for (int i=0; i<=17; ++i){
              if (props == 17){
                if (i == 0 || i == 2)
                {
                  if (bob->_p2[i].per)
                    *ob->_p2[i] += bob->_p2[i].data.max;
                }
                else if (i == 1 || i == 3)
                {
                  if (bob->_p2[i].per)
                  {
                    int temp = randBetween(((bob->_p2[i].data.max - bob->_p2[i].data.min)/2 + bob->_p2[i].data.min + 1),bob->_p2[i].data.max);
                    *ob->_p2[i] += temp;
                  }
                }
                else
                {
                    max(*ob->_p2[i],bob->_p2[i]);
                }
              }else if (props == 18){
                if (i == 0 || i == 2)
                {
                  if (bob->_p2[i].per)
                    *ob->_p2[i] += 10;
                }
                else if (i == 1 || i == 3)
                {
                  if (bob->_p2[i].per)
                    *ob->_p2[i] += bob->_p2[i].data.max;
                }
                else
                {
                    max(*ob->_p2[i],bob->_p2[i]);
                }
              }else{
                additive(*ob->_p2[i],bob->_p2[i]);
              }
            }
            /*
               std::vector<int> list;
               for (int i=0; i<=17; ++i) list.push_back(i);
            //每个都需要随机
            //int geted = props;
            // */
            /*
               if (props == 5){
               if (list.size() >=4){
               additive(*ob->_p2[list[3]],bob->_p2[list[3]]);
               }
               if (list.size() >=8){
               additive(*ob->_p2[list[7]],bob->_p2[list[7]]);
               }
               }else{
            // */
            //while (/*geted -- > 0 && */list.size() > 0) {
            /*
               int index = randBetween(0,list.size()-1 );
               int p = list[index];
            //取最大值
            if (props == 17){
            max(*ob->_p2[p],bob->_p2[p]);
            }else{
            additive(*ob->_p2[p],bob->_p2[p]);
            }
            std::vector<int>::iterator it = list.begin();
            list.erase(it+index);
            // */
            //}
            //}

          }else {
            COMPUTE_L( pdam )    // 增加物理攻击力
              COMPUTE_L( mdam )    // 增加魔法攻击力
              COMPUTE_L( pdef )    // 增加物理防御力
              COMPUTE_L( mdef )    // 增加魔法防御力
              COMPUTE_L( atrating )    // 命中率
              COMPUTE_L( akdodge )    // 闪避率

              COMPUTE_L( poisondef )  // 抗毒增加
              COMPUTE_L( lulldef )    // 抗麻痹增加
              COMPUTE_L( reeldef )    // 抗眩晕增加
              COMPUTE_L( evildef )    // 抗噬魔增加
              COMPUTE_L( bitedef )    // 抗噬力增加
              COMPUTE_L( chaosdef )  // 抗混乱增加
              COMPUTE_L( colddef )    // 抗冰冻增加
              COMPUTE_L( petrifydef )    // 抗石化增加
              COMPUTE_L( blinddef )    // 抗失明增加
              COMPUTE_L( stabledef )    // 抗定身增加
              COMPUTE_L( slowdef )    // 抗减速增加
              COMPUTE_L( luredef )    // 抗诱惑增加
          }

        if (!ob->data.durpoint) {
          if (additive(ob->data.durpoint,bob->resumedur,property)) {
            ob->data.durpoint -= bob->resumedur.sleightValue;
          }
          if (ob->data.durpoint) ob->data.dursecond = bob->resumedur.sleightValue;

        }

        COMPUTE_L( bang )       //重击
          //ob->data.bang += bob->bang;
          //戒指项链至少一个
          if (ob->base->kind == ItemType_Fing || ob->base->kind == ItemType_Necklace)
          {
            BONUS_SKILL_RING
          }
        BONUS_SKILL
          BONUS_SKILLS

          if (props) 
            ob->data.kind |= 2;//有色装备
          else 
            ob->data.kind |= kind;//有色装备

        return true;
      }

    bool assign_holy(zObject* ob,int holy);

    bool assign_set(zObject* ob);

    /**     
     * \brief 属性计算
     *
     * \param ret: 计算结果
     * \param lv: 属性取值范围
     * \return 无
     */  
    template <typename T>
      void additive(T& ret,const rangeValue &rv)
      {
        ret += randBetween(rv.min,rv.max);
      }

    /**     
     * \brief 神圣属性计算
     *
     * \param ret: 计算结果
     * \param lv: 属性取值范围
     * \param property: 物品当前属性数目
     * \return 无
     */  
    template <typename T>
      bool additive(T& ret,const luckRangeValue & lv,int& property)
      {
        int odds = lv.per;
        //    int odds = odds_of_property(lv.per,property);
        //    Zebra::logger->debug("属性产生概率%f,%f",lv.per*1.0,odds*1.0);
        if (selectByTenTh(odds) )  {
          ++property;

          ret += randBetween(lv.data.min,lv.data.max);

          /*
          if (selectByPercent(_odds.sleight) ) {
            ret += lv.sleightValue;
            return true;
          }
          // */
        }

        return false;
      }  

    template <typename T>
      bool additive(T& ret,const luckRangeValue & lv)
      {
        if (selectByTenTh(lv.per) )  {
          ret += randBetween(lv.data.min,lv.data.max);

        }
        /*
        if (selectByPercent(_odds.sleight) ) {
          ret += lv.sleightValue;
          return true;
        }
        // */

        return false;
      }  
    template <typename T>
      bool additivePercent(T& ret,const luckRangeValue & lv)
      {
        ret += randBetween(lv.data.min,lv.data.max);
        /*
        if (selectByPercent(_odds.sleight) ) {
          ret += lv.sleightValue;
          return true;
        }
        // */

        return false;
      }  

};

/**
 * \brief 物品改造
 *
 * 实现各种物品改造功能,提供一个统一入口
 *
 */    
class RebuildObject : public Base
{
public:    
  enum {
    MAX_NUMBER = 50,
    
    HOLE_MONEY = 1000,
    ENCHANCE_MONEY = 500,
    
    COMP_SOUL_STONE_ID = 677,
    ENCHANCE_SONTE_ID = 678,
    HOLE_SONTE_ID = 679,
    LEVELUP_STONE_ID = 681,
  };
  
  static RebuildObject& instance();
      
  bool compose(SceneUser& user,const Cmd::stPropertyUserCmd* cmd);
  
  bool compose_soul_stone(SceneUser& user,const Cmd::stPropertyUserCmd* cmd);
  
  bool upgrade(SceneUser& user,const Cmd::stPropertyUserCmd* cmd);
  
  bool make(SceneUser& user,const Cmd::stPropertyUserCmd* cmd);
  
  bool hole(SceneUser& user,const Cmd::stPropertyUserCmd* cmd);
  
  bool enchance(SceneUser& user,const Cmd::stPropertyUserCmd* cmd);

  bool decompose(SceneUser& user,const Cmd::stPropertyUserCmd* cmd);
private:  
  /**     
   * \brief 构造函数
   *
   */     
  RebuildObject() { }
  
  /**     
   * \brief 析构函数
   *
   */     
  ~RebuildObject() { }

  
  static RebuildObject* _instance;
};
namespace Op {
  /**
   * \brief 变量大小判定
   *
   * 判断一个变量是否大于给定值
   *
   */
  template <typename T>
  class Great
  {
  public:
    enum {
      NEED_EXIST = 1,
    };
    
    typedef T value_type;
    
    /**     
     * \brief 判断给定变量是否大于测试值
     *
     * \param value: 待判断变量
     * \param condition: 测试值
     * \return 待判断变量大于测试值返回true,否则返回false
     */         
    bool operator() (T value,T condition) const
    {
      return (value>condition);
    }
    
    //added for debug
    std::string name() const 
    {
      return "Great";
    }
  };
  
  /**
   * \brief 变量大小判定
   *
   * 判断一个变量是否小于给定值
   *
   */
  template <typename T>
  class Less
  {
  public:
    enum {
      NEED_EXIST = 0,
    };

    typedef T value_type;
    
    /**     
     * \brief 判断给定变量是否小于测试值
     *
     * \param value: 待判断变量
     * \param condition: 测试值
     * \return 待判断变量小于测试值返回true,否则返回false
     */         
    bool operator() (T value,T condition) const
    {
      return (value<condition);
    }

    //added for debug
    std::string name() const 
    {
      return "Less";
    }
  };
  
  /**
   * \brief 变量大小判定
   *
   * 判断一个变量是否等于给定值
   *
   */
  template <typename T>
  class Equal
  {
  public:
    enum {
      NEED_EXIST = 1,
    };

    typedef T value_type;
    
    /**     
     * \brief 判断给定变量是否等于测试值
     *
     * \param value: 待判断变量
     * \param condition: 测试值
     * \return 待判断变量等于测试值返回true,否则返回false
     */         
    bool operator() (T value,T condition) const
    {
      return (value==condition);
    }

    //added for debug
    std::string name() const 
    {
      return "Equal";
    }
    
  };
  
  /**
   * \brief 变量大小判定
   *
   * 判断一个变量不同于给定值
   *
   */
  template <typename T>
  class Differ
  {
  public:
    enum {
      NEED_EXIST = 0,
    };

    typedef T value_type;
    
    /**     
     * \brief 判断给定变量是否不等于测试值
     *
     * \param value: 待判断变量
     * \param condition: 测试值
     * \return 待判断变量不等于测试值返回true,否则返回false
     */         
    bool operator() (T value,T condition) const
    {
      return (value!=condition);
    }

    //added for debug
    std::string name() const 
    {
      return "Differ";
    }
    
  };
}

/**
 * \brief 触发条件基类
 *
 * 该类定义了脚本触发条件的接口。
 *
 */  
class Condition
{
public:
  typedef Condition Base;

  virtual bool is_valid (const SceneUser* user,const Vars* vars) const;

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~Condition() { }
  
protected:
  virtual bool check_args(const SceneUser* user,const Vars* vars) const;
  
  virtual bool check_valid(const SceneUser* user,const Vars* vars) const = 0;
};

/**
 * \brief 关键词列表类
 *
 * 存储了关键词的列表,并提供了对应的操作.
 *
 */
class ScenesParser
{
public:

  /**     
   * \brief 取得xml节点内容
   *
   * 遍历关键词列表,取得他们对应的值
   *      
   * \param xml: 目标xml文件
   * \param node: 目标节点
   * \return 当前总是返回true
   */       
  bool parse (zXMLParser& xml,xmlNodePtr& node)
  {    
    for(iterator it=_kvs.begin(); it!=_kvs.end(); ++it) {
      if (!xml.getNodePropStr(node,it->first.c_str(),it->second)) {  
        _kvs[it->first] = "0";
        //return false;
      }
    }

    return true;
  }
  

  /**     
   * \brief 设置一个关键词
   *
   * 在关键词列表中增加一个关键词,对应的值为"0"
   *      
   * \param key: 关键词名称
   * \return 无
   */       
  void key(const std::string& key) 
  {
    _kvs[key] = "0";
  }

  /**     
   * \brief 取值
   *
   * 在关键词列表中搜寻对应的关键词,并返回对应的值,没找到返回零值
   *      
   * \param key: 关键词名称
   * \param value: 取得的值
   * \return 无
   */  
  template <typename T>
  void value(const std::string& key,T& value)
  {
    std::stringstream os(_kvs[key]);  
    os >> value;
  }
  
  
private:
  typedef std::map<std::string,std::string> KV;
  typedef KV::iterator iterator;
  KV _kvs;  
};

/**
 * \brief 组队触发条件基类
 *
 * 该类提供了对于组队任务的支持
 *
 */  
class TeamCondition : public Condition
{
public:
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  TeamCondition(ScenesParser& p)
  { 
    p.value("team",_team);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~TeamCondition() { }
  
  virtual bool is_valid(const SceneUser* user,const Vars* vars) const;
  
protected:  
  
private:
  int _team;
};

/**
 * \brief 变量条件
 *
 * 该类提供了对于任务脚本中有关变量的条件判定的支持
 *
 */
template <typename Operation>
class VarCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  VarCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("name",_name);
    p.value("value",_condition);
    p.value("id",_id);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~VarCondition()
  { }

  
  /**     
   * \brief  变量判定
   *
   * 重载了check_valid函数,判定某个变量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;
//    Zebra::logger->debug("id:%d\tname:%s\tcondition:%d\toperation:%s",_id,_name.c_str(),_condition,op.name().c_str());
    if (_id) {
      Vars* v = user->quest_list.vars(_id);
      if (v) {
        return v->is_valid(op,_name,_condition);
      }else {
        return !Operation::NEED_EXIST;
      }
    }
      
    return vars->is_valid(op,_name,_condition);
  };

private:
  std::string _name;
  value_type _condition;
  int _id;
};

/**
 * \brief 用户变量条件
 *
 * 该类提供了对于任务脚本中有关用户变量的条件判定的支持
 *
 */
template <typename Operation>
class UserVarCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  UserVarCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("name",_name);
    p.value("value",_condition);
    p.value("id",_id);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~UserVarCondition()
  { }

  /**     
   * \brief  用户变量判定
   *
   * 重载了check_valid函数,判定某个用户变量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    int id = _id?_id:vars->quest_id();

    QWORD key = ((QWORD)user->charbase.accid << 32) | user->charbase.id;
    Vars* v = UserVar::instance().vars(id,key);
    if (v) {
      return v->is_valid(op,_name,_condition);
    }

    return !Operation::NEED_EXIST;
    
  };

private:
  std::string _name;
  value_type _condition;
  int _id;
};

/**
 * \brief 全局变量条件
 *
 * 该类提供了对于任务脚本中有关全局变量的条件判定的支持
 *
 */
template <typename Operation>
class GlobalCondition : public Condition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  GlobalCondition(ScenesParser& p)
  { 
    p.value("name",_name);
    p.value("value",_condition);
    p.value("id",_id);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~GlobalCondition()
  { }

  /**     
   * \brief  参数检查
   *
   * 重载了check_args函数,该条件不需要用户信息
   *      
   * \param user: NULL
   * \param vars: 变量信息
   * \return 变量有效返回true,否则返回false
   */   
  bool check_args(const SceneUser* user,const Vars* vars) const
  {
    if (!vars) return false;
    
    return true;
  }

  /**     
   * \brief  全局变量判定
   *
   * 重载了check_valid函数,判定某个全局变量是否满足脚本要求
   *      
   * \param user: NULL
   * \param vars: 变量信息
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    int id = _id?_id:vars->quest_id();

//    Zebra::logger->debug("id:%d\tname:%s\tcondition:%d\toperation:%s",id,_name.c_str(),_condition,op.name().c_str());
    
    Vars* v = GlobalVar::instance().vars(id);
    if (v) {
      return v->is_valid(op,_name,_condition);
    }
    
    return !Operation::NEED_EXIST;
  };

protected:
  std::string _name;
  value_type _condition;
  int _id;
};

template <typename Operation>
class TongVarCondition : public GlobalCondition<Operation>
{
public:
  TongVarCondition(ScenesParser& p) : GlobalCondition<Operation>(p)
  { 
  }

  virtual ~TongVarCondition()
  { }

  bool check_args(const SceneUser* user,const Vars* vars) const
  {
    if (!user) return false;
    
    return true;
  }

  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;

    Vars* v = TongVar::instance().vars(user->charbase.unionid);
    if (v) {
      return v->is_valid(op,GlobalCondition<Operation>::_name,GlobalCondition<Operation>::_condition);
    }
    
    return !Operation::NEED_EXIST;
  };
};

template <typename Operation>
class FamilyVarCondition : public TongVarCondition<Operation>
{
public:
  FamilyVarCondition(ScenesParser& p) : TongVarCondition<Operation>(p)
  { 
  }

  virtual ~FamilyVarCondition()
  { }

  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;

    Vars* v = FamilyVar::instance().vars(user->charbase.septid);
    if (v) {
      return v->is_valid(op,GlobalCondition<Operation>::_name,GlobalCondition<Operation>::_condition);
    }
    
    return !Operation::NEED_EXIST;
  };
};

template <typename Operation>
class UsersVarCondition : public TongVarCondition<Operation>
{
public:
  UsersVarCondition(ScenesParser& p) : TongVarCondition<Operation>(p)
  { 
  }

  virtual ~UsersVarCondition()
  { }

  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;

    Vars* v = UsersVar::instance().vars( ((QWORD)user->charbase.accid << 32) | user->charbase.id);
    if (v) {
      return v->is_valid(op,GlobalCondition<Operation>::_name,GlobalCondition<Operation>::_condition);
    }
    
    return !Operation::NEED_EXIST;
  };
};

/**
 * \brief 技能是否学习条件
 *
 * 该类提供了判断用户是否学习技能
 *
 */
template <typename Operation>
class HaveSkillCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  HaveSkillCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~HaveSkillCondition()
  { }
  
  /**     
   * \brief  技能个数判定
   *
   * 重载了check_valid函数,判定用户的技能数量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    int size=user->usm.size();
    return op(size,_condition);
  }

private:
  value_type _condition;    
};


/**
 * \brief 检查玩家性别
 *
 * 该类提供了判断用户的性别
 *
 */
template <typename Operation>
class CheckSexCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  CheckSexCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~CheckSexCondition()
  { }
  
  /**     
   * \brief  技能个数判定
   *
   * 重载了check_valid函数,判定用户的技能数量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    int sex = 0;
    switch(user->charbase.type)
    {
      case PROFESSION_1:    //侠客
      case PROFESSION_3:    //箭侠
      case PROFESSION_5:    //天师
      case PROFESSION_7:    //法师
        sex = 1;
        break;
      case PROFESSION_2:    //侠女
      case PROFESSION_4:    //箭灵
      case PROFESSION_6:    //美女
      case PROFESSION_8:    //仙女
        sex = 0;
        break;
      case PROFESSION_NONE:  //无业
      default:
        break;
    }
    return op(sex,_condition);
  }

private:
  value_type _condition;    
};

/**
 * \brief 等级条件
 *
 * 该类提供了判断用户等级是否大于,小于,等于或不等于某个值的接口
 *
 */
template <typename Operation>
class LevelCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  LevelCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~LevelCondition()
  { }
  
  /**     
   * \brief  等级判定
   *
   * 重载了check_valid函数,判定用户的等级是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    return op(user->charbase.level,_condition);
  }

private:
  value_type _condition;    
};


/**
 * \brief 家族条件
 *
 * 该类提供了判断用户家族ID是否大于,小于,等于或不等于某个值的接口
 *
 */
template <typename Operation>
class SeptIDCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  SeptIDCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~SeptIDCondition()
  { }
  
  /**     
   * \brief  家族判定
   *
   * 重载了check_valid函数,判定用户的家族ID是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    return op(user->charbase.septid,_condition);
  }

private:
  value_type _condition;    
};

/**
 * \brief 帮会条件
 *
 * 该类提供了判断用户家族ID是否大于,小于,等于或不等于某个值的接口
 *
 */
template <typename Operation>
class UnionIDCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  UnionIDCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~UnionIDCondition()
  { }
  
  /**     
   * \brief  帮会判定
   *
   * 重载了check_valid函数,判定用户的家族ID是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    return op(user->charbase.unionid,_condition);
  }

private:
  value_type _condition;    
};

/**
 * \brief 金钱条件
 *
 * 该类提供了判断用户包裹中金钱数量是否大于,小于,等于或不等于某个值的接口
 *
 */
template <typename Operation>
class GoldCondition : public TeamCondition
{
public:  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  GoldCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~GoldCondition()
  { }

  /**     
   * \brief  金钱判定
   *
   * 重载了check_valid函数,判定用户包裹中的金钱是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    zObject* gold = const_cast<SceneUser *>(user)->packs.getGold();
    
    DWORD number = 0;
    if (gold)  number = gold->data.dwNum;
    Operation op;

    //shouldn't be exist,NB CEHUA
    //if (op.name() == "Great" && number < (DWORD)_condition) {
      //Channel::sendSys(const_cast<SceneUser*>(user),Cmd::INFO_TYPE_FAIL,"金钱不足");
    //}
    
    return op(number,_condition);
  }

private:
  int _condition;    
};

/**
 * \brief 物品条件
 *
 * 该类提供了判断用户包裹中的特定物品数量是否大于,小于,等于或不等于某个值的接口
 *
 */
class ItemCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ItemCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("id",_id);
    p.value("value",_condition);
    p.value("level",_level);
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~ItemCondition()
  { }

  /**     
   * \brief  物品数量判定
   *
   * 重载了check_valid函数,判定用户包裹空间的某个物品数量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return user->packs.uom.exist(_id,_condition,_level);
  }

private:
  int _id;
  int _level;
  value_type _condition;    
};

/**
 * \brief 国家条件
 *
 * 该类提供了判断用户是否属于一个国家的接口
 *
 */
class NationCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  NationCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~NationCondition()
  { }
  
  /**     
   * \brief  国家判定
   *
   * 重载了check_valid函数,判定用户是否属于某个国家
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return (int)user->charbase.country == _condition;
  }

private:
  value_type _condition;    
};

/**
 * \brief 是否在本国条件
 *
 * 该类提供了判断用户是否在本国的接口
 *
 */
class InNationCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  InNationCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~InNationCondition()
  { }
  
  /**     
   * \brief  国家判定
   *
   * 重载了check_valid函数,判定用户是否属于某个国家
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return user->charbase.country == user->scene->getCountryID();
  }

private:
  value_type _condition;    
};

/**
 * \brief 帮会条件
 *
 * 该类提供了判断用户是否属于某个帮会的接口
 *
 */
class ConfraternityCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ConfraternityCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~ConfraternityCondition()
  { }
  
  /**     
   * \brief  帮会判定
   *
   * 重载了check_valid函数,判定用户是否属于某个帮会
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return (int)(user->charbase.unionid & 0x1) == _condition;
  }

private:
  value_type _condition;    
};

/**
 * \brief 职业条件
 *
 * 该类提供了判断用户是否属于某个职业的接口
 *
 */
class ProfessionCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ProfessionCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~ProfessionCondition()
  { }
  
  /**     
   * \brief  职业判定
   *
   * 重载了check_valid函数,判定用户职业是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return user->charbase.type & _condition;
  }

private:
  value_type _condition;    
};

/**
 * \brief 包裹空间条件
 *
 * 该类提供了判断用户包裹中的空间是否能容纳某个物品的接口
 *
 */
class SpaceCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  SpaceCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("size",_size);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~SpaceCondition()
  { }
  
  /**     
   * \brief  包裹空间判定
   *
   * 重载了check_valid函数,判定用户包裹空间是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    int free = user->packs.uom.space(user);

    if (free >= _size)   return true;

    //Channel::sendSys(const_cast<SceneUser*>(user),Cmd::INFO_TYPE_FAIL,"包裹空间不足");
    return false;
  }

private:
  value_type _size;    
};

/**
 * \brief 是否超过任务时间要求
 *
 * 该类提供了判断任务时间是否满足要求的接口
 *
 */
class TimeoutsCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  TimeoutsCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("long",_time);  
    p.value("id",_id);  
    p.value("less",_less);
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~TimeoutsCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定任务时间是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    if (_id) {
      Vars* v = user->quest_list.vars(_id);
      if (v) {
        return _less?(v->is_timeout(_time)):(!v->is_timeout(_time));
      }
      return true;
    }
      
    return _less?(vars->is_timeout(_time)):(!vars->is_timeout(_time));
  }

private:
  value_type _time;    
  int _id;
  int _less;
};

/**
 * \brief 时间条件
 *
 * 该类提供了判断系统时间是否满足要求的接口
 *
 */
class TimerCondition : public Condition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  TimerCondition(ScenesParser& p)
  { 
    std::string start;
    p.value("start",start);
    strptime(start.c_str(),"%H:%M:%S",&_start);    
    std::string end;
    p.value("end",end);
    strptime(end.c_str(),"%H:%M:%S",&_end);        
  }


  /**     
   * \brief 析构函数
   *
   */    
  virtual ~TimerCondition()
  { }

  /**     
   * \brief  参数检查
   *
   * 重载了check_args函数,该条件不需要用户及变量信息
   *      
   * \param user: NULL
   * \param vars: NULL
   * \return 当前总是返回true
   */   
  bool check_args(const SceneUser* user,const Vars* vars) const
  {
    return true;
  }

  /**     
   * \brief  时间条件判定
   *
   * 重载了check_valid函数,判定系统时间是否满足脚本要求
   *      
   * \param user: NULL
   * \param vars: NULL
   * \return true表示满足条件,false表示不满足条件
   */     
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
          struct tm now;
          time_t timValue = time(NULL);
          zRTime::getLocalTime(now,timValue);
    //Zebra::logger->debug("Time_START:%02d:%02d:%02d",_start.tm_hour,_start.tm_min,_start.tm_sec);
    //Zebra::logger->debug("Time_NOW  :%02d:%02d:%02d",now.tm_hour,now.tm_min,now.tm_sec);
    //Zebra::logger->debug("Time_END  :%02d:%02d:%02d",_end.tm_hour,_end.tm_min,_end.tm_sec);
    if (((now.tm_hour > _start.tm_hour) || ( (now.tm_hour == _start.tm_hour) && (now.tm_min > _start.tm_min) ) 
      || ((now.tm_hour == _start.tm_hour) && (now.tm_min == _start.tm_min) && (now.tm_sec >= _start.tm_sec))) //start
      && ((now.tm_hour < _end.tm_hour) || ( (now.tm_hour == _end.tm_hour) && (now.tm_min < _end.tm_min) ) 
      || ((now.tm_hour == _end.tm_hour) && (now.tm_min == _end.tm_min) && (now.tm_sec < _end.tm_sec)))    //end
      ) {
      return true;
    }  

    return false;  
  }

private:
  struct tm _start;
  struct tm _end;  
};

/**
 * \brief 组队条件
 *
 * 该类提供了判断用户队伍是否满足要求的接口
 *
 */
class TeamedCondition : public Condition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  TeamedCondition(ScenesParser& p) 
  { 
    p.value("number",_number);  
    p.value("male",_male);
    p.value("female",_female);
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~TeamedCondition()
  { }
  
  bool check_valid(const SceneUser* user,const Vars* vars) const;

private:
  value_type _number;  
  value_type _male;
  value_type _female;
};

class IsGuardCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  IsGuardCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_need);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~IsGuardCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定任务时间是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    int guard = user->guard?1:0;
    return guard == _need;
  }
private:
  value_type _need;
};

class FiveTypeCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  FiveTypeCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_need);  
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~FiveTypeCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定任务时间是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return (int)user->charbase.fivetype == _need;
  }
private:
  value_type _need;
};

template <typename Operation>
class FiveLevelCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  FiveLevelCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~FiveLevelCondition()
  { }

  
  /**     
   * \brief  变量判定
   *
   * 重载了check_valid函数,判定某个变量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;
      
    return op((int)user->charbase.fivelevel,_level);
  };

private:
  value_type _level;
};

template <typename Operation>
class FamilyLevelCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  FamilyLevelCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~FamilyLevelCondition() { }

  
  /**     
   * \brief  变量判定
   *
   * 重载了check_valid函数,判定某个变量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;
      
    return op((int)user->dwSeptLevel,_level);
  };

private:
  value_type _level;
};

template <typename Operation>
class ReputeCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ReputeCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~ReputeCondition()
  { }

  
  /**     
   * \brief  变量判定
   *
   * 重载了check_valid函数,判定某个变量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;
      
    return op((int)user->dwSeptRepute,_level);
  };

private:
  value_type _level;
};

template <typename Operation>
class ActionPointCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ActionPointCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief 析构函数
   *
   */    
  virtual ~ActionPointCondition()
  { }

  
  /**     
   * \brief  变量判定
   *
   * 重载了check_valid函数,判定某个变量是否满足脚本要求
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid (const SceneUser* user,const Vars* vars) const
  {
    Operation op;
      
    return op((int)user->dwUnionActionPoint,_level);
  };

private:
  value_type _level;
};

class HorseCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  HorseCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_id);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~HorseCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定用户是否拥有所需马匹
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return user->horse.horse() == (DWORD)_id;
  }
private:
  value_type _id;
};

class GradeCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  GradeCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_grade);  
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~GradeCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定用户是否拥有所需马匹
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    if (_grade == 1) return user->king;
    if (_grade == 2) return user->unionMaster;
    if (_grade == 4) return user->septMaster;
    
    return true;
  }
private:
  value_type _grade;
};

class MapCountryCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  MapCountryCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("name",_name);
    p.value("id",_id);
  }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~MapCountryCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定用户是否拥有所需马匹
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  std::string _name;
  int _id;
};

class HonorCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  HonorCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~HonorCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定用户是否拥有所需马匹
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  int _value;
};

class MaxHonorCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  MaxHonorCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~MaxHonorCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定用户是否拥有所需马匹
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  int _value;
};

class SelfCountryCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  SelfCountryCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~SelfCountryCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定用户是否拥有所需马匹
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  int _value;
};

class CountryPowerCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  CountryPowerCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~CountryPowerCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定国家强弱
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  int _value;
};


class WeekCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  WeekCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~WeekCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定今天是否是一星期中指定的那几天 value值中用0-6位表示一周的7天
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  int _value;
};


class CaptionCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  CaptionCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief 析构函数
   *
   */    
  virtual ~CaptionCondition()
  { }
  
  /**     
   * \brief  任务时间判定
   *
   * 重载了check_valid函数,判定角色是否国王或者城主
   *      
   * \param user: 触发条件的用户
   * \param vars: 用户所带的该任务相关变量
   * \return true表示满足条件,false表示不满足条件
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  int _value;
};

namespace Op {
  /**
   * \brief 增加变量值
   *
   * 增加一个变量的值
   *
   */
  template <typename T>
  class Add
  {
  public:
    typedef T value_type;

    /**     
     * \brief 
     *
     * 增加变量的值         
     *      
     * \param value: 待改变的变量
     * \param action: 要修改的值
     * \return 无
     */         
    void operator() (T& value,T action,SceneUser *user) const
    {
      value += action;
    }

    //added for debug
    std::string name() const 
    {
      return "Add";
    }    
  };

  /**
   * \brief 设定变量值
   *
   * 设定一个变量的值
   *
   */
  template <typename T>
  class Set
  {
  public:
    typedef T value_type;
    
    /**     
     * \brief  
     *
     * 设定一个变量的值         
     *      
     * \param value: 待改变的变量
     * \param action: 要设定的值
     * \return 无
     */     
     void operator() (T& value,T action,SceneUser *user) const
    {
      value = action;
    }

    //added for debug
    std::string name() const 
    {
      return "Set";
    }    
    
  };

  /**
   * \brief 减少变量值
   *
   * 减少一个变量的值
   *
   */  
  template <typename T>
  class Sub
  {
  public:
    typedef T value_type;
    
    /**     
     * \brief 
     *
     * 增加变量的值         
     *      
     * \param value: 待改变的变量
     * \param action: 要减少的值
     * \return 无
     */     
    void operator() (T& value,T action,SceneUser *user) const
    {
      value -= action;
    }

    //added for debug
    std::string name() const 
    {
      return "Sub";
    }    
  };

  /**
   * \brief 乘一个变量值
   *
   * 乘一个变量的值
   *
   */
  template <typename T>
  class Mul
  {
  public:
    typedef T value_type;
    
    /**     
     * \brief  
     *
     * 设定一个变量的值         
     *      
     * \param value: 待改变的变量
     * \param action: 要设定的值
     * \return 无
     */     
     void operator() (T& value,T action,SceneUser *user) const
    {
      value *= action;
    }

    //added for debug
    std::string name() const 
    {
      return "Mul";
    }    
  };

  /**
   * \brief 变量值乘方
   *
   * 变量值乘方
   *
   */
  template <typename T>
  class Pow
  {
  public:
    typedef T value_type;
    
    /**     
     * \brief  
     *
     * 设定一个变量的值         
     *      
     * \param value: 待改变的变量
     * \param action: 要设定的值
     * \return 无
     */     
     void operator() (T& value,T action,SceneUser *user) const
    {
      value *= value;
    }

    //added for debug
    std::string name() const 
    {
      return "Pow";
    }    
  };

  /**
   * \brief 变量值乘方
   *
   * 变量值乘方
   *
   */
  template <typename T>
  class Div
  {
  public:
    typedef T value_type;
    
    /**     
     * \brief  
     *
     * 设定一个变量的值         
     *      
     * \param value: 待改变的变量
     * \param action: 要设定的值
     * \return 无
     */     
     void operator() (T& value,T action,SceneUser *user) const
    {
      value /= action;
    }

    //added for debug
    std::string name() const 
    {
      return "Div";
    }    
  };

  /**
   * \brief 变量值乘方
   *
   * 变量值乘方
   *
   */
  template <typename T>
  class GetP
  {
  public:
    typedef T value_type;
    
    /**     
     * \brief  
     *
     * 设定一个变量的值         
     *      
     * \param value: 待改变的变量
     * \param action: 要设定的值
     * \return 无
     */     
     void operator() (T& value,T action,SceneUser *user) const
    {
      if (user)
      {
        switch(action)
        {
          case 1:
            value = user->charbase.level;
            break;
          case 2:
            value = time(NULL)%2000000000;
            break;
          case 3:
            break;
          default:
            break;
        }
      }
    }

    //added for debug
    std::string name() const 
    {
      return "GetP";
    }    
  };
}

class SceneUser;

/**
 * \brief 触发动作基类
 *
 * 该类定义了脚本触发动作的接口。
 *
 */  
class Action
{
public:  
  typedef Action Base;
  
  enum {
    SUCCESS = 0,
    FAILED = 1,
    DISABLE = 2,    
  };

  virtual int do_it (SceneUser* user,Vars* vars);
  

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~Action() { }

protected:
  virtual bool check_args(SceneUser* user,Vars* vars) const;
  
  /**     
   * \brief  执行脚本定义的动作
   *
   * 虚函数,继承类需要重载此函数提供对各种触发动作的支持
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */     
  virtual int done(SceneUser* user,Vars* vars) = 0;
};

/**
 * \brief 组队触发动作基类
 *
 * 该类提供了对于组队任务的支持
 *
 */  
class TeamAction : public Action
{
public:  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  TeamAction(ScenesParser& p)
  {
    p.value("team",_team);  
  }
    
  int do_it (SceneUser* user,Vars* vars);

protected:  
  /**     
   * \brief  执行脚本定义的动作
   *
   * 虚函数,继承类需要重载此函数提供对各种触发动作的支持
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */     
  int done(SceneUser* user,Vars* vars) = 0;

private:
  
  int _team;
};

/**
 * \brief 变量
 *
 * 该类提供了对于任务脚本中有关变量的修改的支持
 *
 */  
template <typename Operation>
class VarAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  VarAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name",_name);
    p.value("value",_action);
    p.value("id",_id);
    p.value("tmp",_tmp);    
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~VarAction()
  { }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
//    Zebra::logger->debug("id:%d\tname:%s\taction:%d\toperation:%s",_id,_name.c_str(),_action,op.name().c_str());          
    if (_id) {
      Vars* v = user->quest_list.vars(_id);
      if (v) {
        v->set_value(op,_name,_action,_tmp,user);
      }
    }else {
      vars->set_value(op,_name,_action,_tmp,user);
    }
    return Action::SUCCESS;
  }
  
private:
  std::string _name;
  value_type _action;
  int _tmp;
  int _id;
};

/**
 * \brief 变量间动作
 *
 * 该类提供了对于任务脚本中变量之间的操作的支持
 *
 */  
template <typename Operation>
class VarsAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  VarsAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name1",_name1);
    p.value("id1",_id1);
    p.value("name2",_name2);
    p.value("id2",_id2);
    p.value("tmp",_tmp);    
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~VarsAction()
  { }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
    value_type _action;
//    Zebra::logger->debug("id:%d\tname:%s\taction:%d\toperation:%s",_id,_name.c_str(),_action,op.name().c_str());          
    if (_id1)
    {
      Vars* v = user->quest_list.vars(_id1);
      if (v)
      {
        if (_id2)
        {
          Vars* tmpvars= user->quest_list.vars(_id2);
          tmpvars->get_value(_name2,_action);
        }
        else
        {
          vars->get_value(_name2,_action);
        }
        v->set_value(op,_name1,_action,_tmp,user);

      }
    }
    else
    {
      if (_id2)
      {
        Vars* tmpvars= user->quest_list.vars(_id2);
        tmpvars->get_value(_name2,_action);
      }
      else
      {
        vars->get_value(_name2,_action);
      }
      vars->set_value(op,_name1,_action,_tmp,user);
    }
    return Action::SUCCESS;
  }
  
private:
  int _tmp;
  int _id1;
  std::string _name1;
  int _id2;
  std::string _name2;
};

/**
 * \brief 用户变量
 *
 * 该类提供了对于任务脚本中需要保持在用户身上变量的支持
 *
 */  
template <typename Operation>
class UserVarAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  UserVarAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name",_name);
    p.value("value",_action);
    p.value("id",_id);
    p.value("tmp",_tmp);    
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~UserVarAction()
  { }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
    
    if (!_id) _id = vars->quest_id();
        
    Vars* v = UserVar::instance().add(_id,((QWORD)user->charbase.accid << 32) | user->charbase.id);
    v->set_value(op,_name,_action,_tmp,user);
    
    return Action::SUCCESS;
  }
  
private:
  std::string _name;
  value_type _action;
  int _tmp;
  int _id;
};

/**
 * \brief 系统信息
 *
 * 该类提供了对用户提供系统聊天信息的支持.
 *
 */  
class NotifyAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  NotifyAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~NotifyAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _info;  
};

/**
 * \brief 日志信息
 *
 * 该类提供了对日志信息的支持.
 *
 */  
class LogAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  LogAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~LogAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _info;  
};

class BulletinAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  BulletinAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
    p.value("kind",_kind);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~BulletinAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _info;  
  int _kind; 
};

/**
 * \brief 系统信息
 *
 * 该类提供了对用户提供系统聊天信息的支持.
 *
 */  
class Notify1Action : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  Notify1Action(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~Notify1Action() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _info;  
};

/**
 * \brief 菜单
 *
 * 该类提供了对客户端动态菜单的支持.
 *
 */  
class MenuAction : public Action
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param menu: 菜单内容
   */       
  MenuAction(const std::string& menu) : _menu(menu)
  { }
  
  virtual ~MenuAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _menu;  
};

/**
 * \brief 菜单
 *
 * 该类提供了对客户端动态菜单的支持.
 *
 */  
class SubMenuAction : public Action
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param menu: 菜单内容
   */       
  SubMenuAction(const std::string& menu) : _menu(menu)
  { }
  
  virtual ~SubMenuAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _menu;  
};

/**
 * \brief 菜单
 *
 * 该类提供了对客户端动态菜单的支持.
 *
 */  
class MainMenuAction : public Action
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param menu: 菜单内容
   */       
  MainMenuAction(const std::string& menu) : _menu(menu)
  { }
  
  virtual ~MainMenuAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _menu;  
};


/**
 * \brief 变量刷新
 *
 * 该类实现了对任务变量的及时刷新
 *
 */  
class RefreshAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  RefreshAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name",_name);  
    p.value("id",_id);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~RefreshAction() { }  
  
  int done (SceneUser* user,Vars* vars);
private:
  std::string _name;
  int _id;
};

/**
 * \brief 经验
 *
 * 该类提供了增加特定用户经验的接口
 *
 */
class ExpAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ExpAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_exp);  
    p.value("name",_name);
    p.value("id",_id);
    
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~ExpAction() { }  
  
  int done (SceneUser* user,Vars* vars);
protected:
  int _exp;
  std::string _name;
  int _id;
};



class Exp1Action : public ExpAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  Exp1Action(ScenesParser& p) : ExpAction(p)
  { 
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~Exp1Action() { }  
  
  int done (SceneUser* user,Vars* vars);
};


/**
 * \brief 银子
 *
 * 该类提供了修改用户身上银子数量的接口
 *
 */  
template <typename Operation>
class GoldAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  GoldAction(ScenesParser& p)  : TeamAction(p)
  { 
    p.value("value",_value);
    
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~GoldAction() { }  

  /**     
   * \brief  添加银子
   *
   * 重载了done函数,增加用户身上携带的银子
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */  
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;

    
    if (op.name() == "Add") {
      user->packs.addMoney(_value,"任务添加");
      return Action::SUCCESS;
    }

    if (op.name() == "Sub") {
      if (!user->packs.removeMoney(_value,"任务减少")) {
        Zebra::logger->fatal("任务银子计算错误: 用户(%s),任务(%d)",user->name,vars->quest_id());
      }
      
      return Action::SUCCESS;
    }
    
    return Action::FAILED;
  }
  
private:
  value_type _value;
};

class EnterSeptGuardAction : public Action
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  EnterSeptGuardAction()
  { 
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~EnterSeptGuardAction() { }  
  
  int done (SceneUser* user,Vars* vars);
private:
};

class FinishSeptGuardAction : public Action
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  FinishSeptGuardAction()
  { 
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~FinishSeptGuardAction() { }  
  
  int done (SceneUser* user,Vars* vars);
};

/**
 * \brief 评价
 *
 * 该类提供了增加特定用户评价的接口,尚未实现
 *
 */
template <typename Operation>
class ScoreAction : public TeamAction
{
public:

  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ScoreAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_value);  
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~ScoreAction() { }  
  
  int done (SceneUser* user,Vars* vars)
  {
    return Action::SUCCESS;
  }
  
private:
  value_type _value;
};

/**
 * \brief 技能
 *
 * 该类提供了增加特定用户技能等级的接口.
 *
 */
class SkillAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  SkillAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~SkillAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  value_type _id;
};

/**
 * \brief 禁止
 *
 * 该类提供了禁止用户执行某项动作的方法,当前支持的类型有使用物品,拾取物品及上下马.
 *
 */
class DisableAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  DisableAction(ScenesParser& p)  : TeamAction(p)
  { 
  
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~DisableAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:

};

/**
 * \brief 地图跳转
 *
 * 该类提供了用户在地图上跳转的接口
 *
 */
class GotoAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  GotoAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name",_name);  
    p.value("pos",_pos);
    p.value("pos1",_pos1);
    p.value("pos2",_pos2);
    p.value("pos3",_pos3);
    p.value("pos4",_pos4);
    p.value("cpos",_cpos);
    p.value("rlen",_rlen);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~GotoAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _name;
  std::string _pos;
  std::string _pos1;
  std::string _pos2;
  std::string _pos3;
  std::string _pos4;
  std::string _cpos;
  std::string _rlen;
};

/**
 * \brief 下马
 *
 * 该类提供了使用户下马的接口
 *
 */
class RideDownAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  RideDownAction(ScenesParser& p) : TeamAction(p)
  { }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~RideDownAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:

};

/**
 * \brief 添加物品
 *
 * 该类提供了给用户添加某个物品的接口
 *
 */
class AddItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  AddItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~AddItemAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _value;
  int _odds;
};

/**
 * \brief 添加绑定物品
 *
 * 该类提供了给用户添加某个物品的接口
 *
 */
class AddBindItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  AddBindItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~AddBindItemAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _value;
  int _odds;
};

/**
 * \brief 添加绿色绑定物品
 *
 * 该类提供了给用户添加某个物品的接口
 *
 */
class AddGreenBindItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  AddGreenBindItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~AddGreenBindItemAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _value;
  int _odds;
};

/**
 * \brief 删除物品
 *
 * 该类提供了删除用户身上某个物品的接口
 *
 */
class RemoveItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  RemoveItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);      
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~RemoveItemAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _value;
};

/**
 * \brief 丢弃物品
 *
 * 该类提供了丢弃用户身上某个物品的接口
 *
 */
class DropItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  DropItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("level",_level);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~DropItemAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _level;
};

/**
 * \brief 未实现
 *
 * 
 *
 */
class DropAction : public Action
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  DropAction(ScenesParser& p)
  {
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
    p.value("guard",_guard);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~DropAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _value;
  int _odds;
  int _guard;
};

/**
 * \brief 定时器
 *
 * 该类提供了对需要时间限制的任务的支持
 *
 */
class TimeoutsAction : public Action
{
public:

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  TimeoutsAction(ScenesParser& p)
  {
    //p.value("value",_timeout);
    p.value("id",_id);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~TimeoutsAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  //int _timeout;
  int _id;
};

/**
 * \brief 设置状态
 *
 * 该类提供了设置用户身上某个状态的接口
 *
 */
class SetStateAction : public TeamAction
{
public:

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  SetStateAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_state);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~SetStateAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _state;
};

/**
 * \brief 清除状态
 *
 * 该类提供了清除用户身上某个状态的接口
 *
 */
class ClearStateAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ClearStateAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_state);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~ClearStateAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _state;
};

/**
 * \brief 马匹
 *
 * 该类提供了给予用户马匹的接口
 *
 */
class HorseAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  HorseAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_id);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~HorseAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
};


/**
 * \brief 全局变量
 *
 * 该类提供了对于任务脚本中需要对所有用户可见变量的支持
 *
 */  
template <typename Operation>
class GlobalAction : public Action
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  GlobalAction(ScenesParser& p) : _id(0)
  { 
    p.value("name",_name);
    p.value("value",_action);
    p.value("id",_id);
    p.value("tmp",_tmp);    
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~GlobalAction()
  { }
  
  bool check_args(SceneUser* user,Vars* vars) const
  {
    if (!vars) return false;
    
    return true;    
  }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
    if (!_id) _id = vars->quest_id();

//    Zebra::logger->debug("id:%d\tname:%s\taction:%d\toperation:%s",_id,_name.c_str(),_action,op.name().c_str());
        
    Vars* v = GlobalVar::instance().add(_id);
    v->set_value(op,_name,_action,_tmp,user);

    return Action::SUCCESS;
  }
  
protected:
  std::string _name;
  value_type _action;
  int _tmp;
  int _id;
};

template <typename Operation>
class TongVarAction : public GlobalAction<Operation>
{
public:
  typedef typename Operation::value_type value_type;
  
  TongVarAction(ScenesParser& p) : GlobalAction<Operation>(p)
  { 
  }

  virtual ~TongVarAction()
  { }
  
  bool check_args(SceneUser* user,Vars* vars) const
  {
    if (!user) return false;
    
    return true;    
  }

  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
        
    Vars* v = TongVar::instance().add(user->charbase.unionid);
    v->set_value(op,GlobalAction<Operation>::_name,GlobalAction<Operation>::_action,GlobalAction<Operation>::_tmp,user);

    return Action::SUCCESS;
  }  
};

template <typename Operation>
class FamilyLevelAction : public TongVarAction<Operation>
{
public:
  typedef typename Operation::value_type value_type;
  
  FamilyLevelAction(ScenesParser& p) : TongVarAction<Operation>(p),_level(0)
  { 
    p.value("value",_level);
    Zebra::logger->debug("初始化_level%d",_level);
  }

  virtual ~FamilyLevelAction()
  { }

  int done (SceneUser* user,Vars* vars)
  {
    Cmd::Session::t_OpLevel_SceneSession uplevel;
    uplevel.dwSeptID=user->charbase.septid;
    uplevel.dwLevel=_level;
    Zebra::logger->debug("发送_level%d",_level);
    sessionClient->sendCmd(&uplevel,sizeof(uplevel));
    return Action::SUCCESS;
  }
  
private:
  value_type _level;
};

template <typename Operation>
class FamilyVarAction : public TongVarAction<Operation>
{
public:
  typedef typename Operation::value_type value_type;
  
  FamilyVarAction(ScenesParser& p) : TongVarAction<Operation>(p)
  { 
  }

  virtual ~FamilyVarAction()
  { }
  
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
        
    Vars* v = FamilyVar::instance().add(user->charbase.septid);
    v->set_value(op,GlobalAction<Operation>::_name,GlobalAction<Operation>::_action,GlobalAction<Operation>::_tmp,user);

    return Action::SUCCESS;
  }  
};

template <typename Operation>
class UsersVarAction : public TongVarAction<Operation>
{
public:
  typedef typename Operation::value_type value_type;
  
  UsersVarAction(ScenesParser& p) : TongVarAction<Operation>(p)
  { 
  }

  virtual ~UsersVarAction()
  { }
  
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
        
    Vars* v = UsersVar::instance().add(((QWORD)user->charbase.accid << 32) |user->charbase.id);
    v->set_value(op,GlobalAction<Operation>::_name,GlobalAction<Operation>::_action,GlobalAction<Operation>::_tmp,user);

    return Action::SUCCESS;
  }  
};

/**
 * \brief  NPC
 *
 * 该类提供了对场景上NPC访问的支持
 *
 */
class NpcAction : public Action
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  NpcAction(ScenesParser& p)
  { 
    p.value("id",_id);
    std::string map;
    p.value("map",map);
    _s = SceneManager::getInstance().getSceneByName(map.c_str());
    if (!_s) {
      Zebra::logger->warn("NpcAction时请求的场景(%s)不存在",map.c_str());
    }
    
    std::string pos;    
    p.value("ltpos",pos);  
    zRegex regex("(.*)[,](.*)");
    if (/*(regex.compile("(.*)[ ,](.*)") && */regex.match(pos.c_str()))) {
      //std::string s;
      //regex.getSub(s,1);
      _ltpos.x = regex.first;//atoi(s.c_str());
      //regex.getSub(s,2);
      _ltpos.y = regex.second;//atoi(s.c_str());
    }

    p.value("rbpos",pos);  
    if (/*(regex.compile("(.*)[ ,](.*)") && */regex.match(pos.c_str()))) {
      //std::string s;
      //regex.getSub(s,1);
      _rbpos.x = regex.first;//atoi(s.c_str());
      //regex.getSub(s,2);
      _rbpos.y = regex.second;//atoi(s.c_str());
    }
    
    p.value("num",_num);
    if (!_num) _num = 1;
    if (_num>100) _num = 100;
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~NpcAction()
  { }
  
  bool check_args(SceneUser* user,Vars* vars) const
  {
    return true;
  }
    
protected:
  Scene* _s;
  int _id;
  zPos _ltpos;
  zPos _rbpos;
  int _num;
  
};

/**
 * \brief 增加NPC
 *
 * 该类提供了在地图上增加一个NPC的接口
 *
 */
class AddNpcAction : public NpcAction
{
public:

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  AddNpcAction(ScenesParser& p) : NpcAction(p)
  { }
  
  /**     
   * \brief 析构函数
   *
   */     
  ~AddNpcAction()
  { }
  
  int done(SceneUser* user,Vars* vars);  
  
};

/**
 * \brief 增加镖车
 *
 * 该类提供了对护镖任务的支持
 *
 */
class AddGuardAction : public NpcAction
{
public:

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  AddGuardAction(ScenesParser& p) : NpcAction(p)
  { 
    p.value("id2",_id2);  
    p.value("id3",_id3);  
    p.value("odds1",_odds1);  
    p.value("odds2",_odds2);  
    p.value("odds3",_odds3);  
    p.value("gold",_gold);  
    p.value("path",_path);
    p.value("exp",_exp);
    p.value("script",_script);
    p.value("map",_map);

    if (_odds1+_odds2+_odds3==0)
      _odds1 = 100;

    std::string pos;    
    p.value("dest",pos);  
    zRegex regex("(.*)[,](.*)");
    if (/*(regex.compile("(.*)[ ,](.*)") && */regex.match(pos.c_str()))) {
      //std::string s;
      //regex.getSub(s,1);
      _dest.x = regex.first;//atoi(s.c_str());
      //regex.getSub(s,2);
      _dest.y = regex.second;//atoi(s.c_str());
      //Zebra::logger->debug("AddGuardAction::AddGuardAction(): _dest=%u,%u",_dest.x,_dest.y);
    }
    else
      Zebra::logger->info("AddGuardAction::AddGuardAction(): _dest匹配失败 %s",pos.c_str());
    
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  ~AddGuardAction()
  { }
  
  int done(SceneUser* user,Vars* vars);  

  bool check_args(SceneUser* user,Vars* vars) const
  {
    if (user) return true;
    
    return false;
  }
  
  private:
    std::string _path;
    int _gold;
    int _exp;
    int _script;
    zPos _dest;
    std::string _map;
    DWORD _id2,_id3;
    DWORD _odds1,_odds2,_odds3;
};

/**
 * \brief 删除NPC
 *
 * 该类提供了在地图上删除一个NPC的接口
 *
 */
class RemoveNpcAction : public NpcAction
{
public:

  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  RemoveNpcAction(ScenesParser& p) : NpcAction(p),_remove(_s,_id,_ltpos,_rbpos)
  { }
  
  /**     
   * \brief 析构函数
   *
   */     
  ~RemoveNpcAction()
  { }
  
  int done(SceneUser* user,Vars* vars);    
private:

  class RemoveNpc : public removeEntry_Pred<SceneNpc>
  {
  public:    
    RemoveNpc(Scene* s,int id,const zPos& ltpos,const zPos& rbpos) : _s(s),_id(id),_ltpos(ltpos),_rbpos(rbpos)
    {
    }

    bool isIt(SceneNpc *npc)
    {
      if ((int)npc->id == _id && (npc->getPos() >= _ltpos && npc->getPos() <= _rbpos) ) {
        npc->setState(zSceneEntry::SceneEntry_Hide);
        Cmd::stAddMapNpcMapScreenUserCmd addNpc;
        npc->full_t_MapNpcData(addNpc.data);
        _s->sendCmdToNine(npc->getPosI(),&addNpc,sizeof(addNpc));
        return true;
      }

      return false;
    }
  private:
    Scene* _s;
    int _id;
    const zPos& _ltpos;
    const zPos& _rbpos;
  };
  
  RemoveNpc _remove;
};

/**
 * \brief 变量
 *
 * 该类提供了对于任务脚本中有关五行点数的修改的支持
 *
 */  
template <typename Operation>
class FiveLevelAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  FiveLevelAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_level);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~FiveLevelAction()
  { }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
    op((int &)user->charbase.fivelevel,_level,user);

    Cmd::stMainUserDataUserCmd  userinfo;
    user->full_t_MainUserData(userinfo.data);
    user->sendCmdToMe(&userinfo,sizeof(userinfo));

    return Action::SUCCESS;
  }
  
private:
  value_type _level;
};

/**
 * \brief 设置五行类型
 *
 * 该类提供了设置五行类型的接口
 *
 */
class FiveTypeAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  FiveTypeAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_type);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~FiveTypeAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _type;
};

template <typename Operation>
class HonorAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  HonorAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_honor);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~HonorAction()
  { }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
    op((int &)user->charbase.honor,_honor,user);

    Cmd::stMainUserDataUserCmd  userinfo;
    user->full_t_MainUserData(userinfo.data);
    user->sendCmdToMe(&userinfo,sizeof(userinfo));

    return Action::SUCCESS;
  }
  
private:
  value_type _honor;
};

template <typename Operation>
class MaxHonorAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  MaxHonorAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_honor);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~MaxHonorAction()
  { }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;
    op((int &)user->charbase.maxhonor,_honor,user);

    Cmd::stMainUserDataUserCmd  userinfo;
    user->full_t_MainUserData(userinfo.data);
    user->sendCmdToMe(&userinfo,sizeof(userinfo));

    return Action::SUCCESS;
  }
  
private:
  value_type _honor;
};


template <typename Operation>
class ActionPointAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  ActionPointAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_ap);
  }

  /**     
   * \brief 析构函数
   *
   */     
  virtual ~ActionPointAction()
  { }

  /**     
   * \brief  执行变量修改
   *
   * 重载了done函数,实现对相关变量的修改
   *      
   * \param user: 触发动作的用户
   * \param vars: 用户所带的该任务相关变量
   * \return SUCCESS表示成功,FAILED表示失败,DISABLE表示禁用某项功能
   */       
  int done (SceneUser* user,Vars* vars)
  {
    Cmd::Session::t_ChangeAP  cmd;;
    cmd.id = user->charbase.unionid;
    cmd.point = _ap;
    sessionClient->sendCmd(&cmd,sizeof(cmd));
    
    return Action::SUCCESS;
  }
  
private:
  value_type _ap;
};

class UseSkillAction : public TeamAction
{
public:
  
  /**     
   * \brief  构造函数
   *
   * 初始化相关变量
   *      
   * \param p: 关键词列表
   */       
  UseSkillAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("id",_id);
    p.value("level",_level);
  }
  
  /**     
   * \brief 析构函数
   *
   */     
  virtual ~UseSkillAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _level;
};

/**
 * \brief 产品工厂
 *
 * 封装了工厂模式
 *
 */
template <typename I,typename C>
class ProductFactory
{
public:
  typedef ProductFactory<I,C> self_t;
  
  /**     
   * \brief 
   *
   * 实现了单件模式
   *      
   * \return 该类的唯一实例
   */       
  static self_t& instance()
  {
    if (!_instance) {
      static self_t new_instance;
      _instance = &new_instance;
    }
    
    return *_instance;
  }

  /**     
   * \brief 取得创建类
   *
   * 寻找特定产品标志的创建类
   *      
   * \param id: 产品标识
   * \return 找的产品创建类,没找到返回NULL
   */       
  C* get_creator(const I& id) const
  {    
    const_list_iterator it = _list.find(id);
    if (it!= _list.end() ) {
      return it->second;
    }
    
    return NULL;    
  }
  
  /**     
   * \brief 注册方法
   *
   * 注册某类产品的创建类
   *      
   * \param id: 产品标识
   * \param creator: 产品创建类
   * \return 无
   */       
  void register_creator(const I& id,C* creator)  
  {
    _list[id] = creator;
  }
private:
  /**     
   * \brief 构造函数
   *
   */     
  ProductFactory()
  { }
  
  /**     
   * \brief 析构函数
   *
   */     
  ~ProductFactory()
  { }
  
  static self_t* _instance;
  
  typedef std::map<I,C*> LIST;
  typedef typename LIST::iterator list_iterator;
  typedef typename LIST::const_iterator const_list_iterator;
  LIST _list;
};

template<typename I,typename C>
ProductFactory<I,C>* ProductFactory<I,C>::_instance = NULL;

namespace MakeFunction {
  /**
   * \brief 产品创建基类
   *
   *定义了创建产品的接口
   *
   */
  template <typename A>
  class Maker
  {
  public:  
    typedef typename A::Base Base;

    /**     
     * \brief  产品创建接口
     *
     * 定义了产品创建 的接口,继承类需要实现此函数提供具体产品的创建方法.
     *      
     * \param xml: 脚本文件
     * \param node: 节点名称
     * \return 创建的产品
     */       
    virtual Base* make ( zXMLParser& xml,xmlNodePtr& node) = 0;

    /**     
     * \brief 析构函数
     *
     */     
    virtual ~Maker() { }
  };
  
  /**
   * \brief 节点内容创建类
   *
   *封装了需要访问节点内容的脚本触发条件及动作类的创建
   *
   */  
  template <typename A>
  class Content : public Maker<typename A::Base>
  {
  public:  
    typedef typename A::Base Base;
    
    /**     
     * \brief 析构函数
     *
     */     
    virtual ~Content() { }
    
    /**     
     * \brief  产品创建接口
     *
     *实现对访问节点内容的脚本触发条件及动作类的创建
     *      
     * \param xml: 脚本文件
     * \param node: 节点名称
     * \return 创建的产品
     */       
    Base* make (zXMLParser& xml,xmlNodePtr& node)
    {
      std::string content;
      if (!xml.getNodeContentStr(node,content)) {
        return NULL;
      }
      
      return new A(content);      
    }
  };
  
  /**
   * \brief 空属性创建类
   *
   *封装了不需要访问节点属性的脚本触发条件及动作类的创建
   *
   */  
  template <typename A>
  class Void : public Maker<Action>
  {
  public:  
    typedef Maker<Action>::Base Base;
    
    /**     
     * \brief  构造函数
     *
     * 初始化相关变量
     *      
     */       
    Void()
    { }
    
    /**     
     * \brief 析构函数
     *
     */     
    virtual ~Void() { }
    
    /**     
     * \brief  产品创建接口
     *
     *      
     * \param xml: 脚本文件
     * \param node: 节点名称
     * \return 创建的产品
     */       
    Base* make (zXMLParser& xml,xmlNodePtr& node)
    {
      return  new A();
    }
  };

  /**
   * \brief 节点属性创建类
   *
   *封装了需要访问节点属性的脚本触发条件及动作类的创建
   *
   */  
  template <typename A>
  class Prop : public Maker<typename A::Base>
  {
  public:  
    typedef typename A::Base Base;
    
    /**     
     * \brief  构造函数
     *
     * 初始化相关变量
     *      
     * \param p: 关键词列表
     */       
    Prop(ScenesParser& p) : _p(p)
    { }
    
    /**     
     * \brief 析构函数
     *
     */     
    virtual ~Prop() { }
    
    /**     
     * \brief  产品创建接口
     *
     *实现对访问节点属性的脚本触发条件及动作类的创建
     *      
     * \param xml: 脚本文件
     * \param node: 节点名称
     * \return 创建的产品
     */       
    Base* make (zXMLParser& xml,xmlNodePtr& node)
    {
      if (! _p.parse(xml,node) ) return NULL;

      return new A(_p);      
    }
  private:
  
    ScenesParser _p;
  };

  /**
   * \brief 触发动作创建类
   *
   *封装了需要按操作类型更新的脚本触发动作类的创建
   *
   */  
  template < template <typename> class A,typename T = int >
  class Update : public Maker<Action>
  {
  public:  
    typedef Maker<Action>::Base Base;
    
    /**     
     * \brief  构造函数
     *
     * 初始化相关变量
     *      
     * \param p: 关键词列表
     */       
    Update(ScenesParser& p) : _p(p)
    {
      
    }
    
    /**     
     * \brief 析构函数
     *
     */     
    virtual ~Update() { }
    
    /**     
     * \brief  产品创建接口
     *
     *实现对按操作类型更新的脚本触发动作类的创建
     *      
     * \param xml: 脚本文件
     * \param node: 节点名称
     * \return 创建的产品
     */       
    Base* make (zXMLParser& xml,xmlNodePtr& node)
    {
      if (! _p.parse(xml,node) ) return NULL;

      std::string type;
      _p.value("type",type);

      if (!type.compare("Set")) {
        return new A< Op::Set<T> >(_p);  
      }else if (!type.compare("Add")) {
        return new A< Op::Add<T> >(_p);                      
      }else if (!type.compare("Sub")) {
        return new A< Op::Sub<T> >(_p);                  
      }else if (!type.compare("Mul")) {
        return new A< Op::Mul<T> >(_p);                  
      }else if (!type.compare("Pow")) {
        return new A< Op::Pow<T> >(_p);                  
      }else if (!type.compare("GetP")) {
        return new A< Op::GetP<T> >(_p);                  
      }else if (!type.compare("Div")) {
        return new A< Op::Div<T> >(_p);                  
      }

      return NULL;
    }
    
    private:
      ScenesParser _p;
  };

  /**
   * \brief 触发条件创建类
   *
   *封装了需要按操作类型比较的脚本触发条件类的创建
   *
   */  
  template <template <typename> class A,typename T = int >
  class Compare : public Maker<Condition>
  {
  public:  
    typedef Maker<Condition>::Base Base;
    
    /**     
     * \brief  构造函数
     *
     * 初始化相关变量
     *      
     * \param p: 关键词列表
     */       
    Compare(ScenesParser& p) : _p(p)
    { 
      
    }
    
    /**     
     * \brief 析构函数
     *
     */     
    virtual ~Compare() { }
    
    /**     
     * \brief  产品创建接口
     *
     *实现对按操作类型比较的脚本触发条件类的创建
     *      
     * \param xml: 脚本文件
     * \param node: 节点名称
     * \return 创建的产品
     */ 
    Base* make (zXMLParser& xml,xmlNodePtr& node)
    {
      if (! _p.parse(xml,node) ) return NULL;
      
      std::string type;
      _p.value("type",type);
      
      if (!type.compare("Equal")) {
        return new A< Op::Equal<T> >(_p);
      }else if (!type.compare("Great")) {
        return new A< Op::Great<T> >(_p);                      
      }else if (!type.compare("Less")) {
        return new A< Op::Less<T> >(_p);                      
      }else if (!type.compare("Differ")) {
        return new A< Op::Differ<T> >(_p);                      
      }
      
      return NULL;
    }
    
    private:
      ScenesParser _p;
  };

}
  
typedef ProductFactory<std::string,MakeFunction::Maker<Action> > ActionFactory;
typedef ProductFactory<std::string,MakeFunction::Maker<Condition> > ConditionFactory;

/**
 * \brief 消息系统
 *
 */
class MessageSystem
{

  public:

    ~MessageSystem()
    {
      final();
    }

    /**
     * \brief 得到唯一实例
     *
     */
    static MessageSystem &getInstance()
    {
      if (NULL == instance)
        instance = new MessageSystem();

      return *instance;
    }

    /**
     * \brief 卸载唯一实例
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    bool init();
    void check(SceneUser *sceneUser,const bool init = false);

  private:

    static MessageSystem *instance;

    MessageSystem() {};
    void final();

    /**
     * \brief 消息数据结构
     *
     */
    struct t_Message
    {
      time_t starttime;
      time_t endtime;
      bool login;
      WORD interval;
      int count;
      int order;
      Cmd::stChannelChatUserCmd cmd;
    };

    std::vector<t_Message> messages;
    zRWLock rwlock;

};

/**
  A script that can be executed by a virtual machine.
*/
class LuaScript 
{
public:
  LuaScript( );

  virtual ~LuaScript();

  virtual bool isLoaded();

  virtual void setData( const std::string& rData );

  const std::string& getData() const;

private:
  std::string mFileName;
};

const DWORD LUALIB_BASE    = 0x00000001;
const DWORD LUALIB_TABLE    = 0x00000002;
const DWORD LUALIB_IO      = 0x00000004;
const DWORD LUALIB_STRING    = 0x00000008;
const DWORD LUALIB_MATH    = 0x00000010;
const DWORD LUALIB_DEBUG    = 0x00000020;

struct lua_State;

/** Scripting virtual machine.
  A virtual machine can execute scripting code.
*/
class LuaVM
{
protected:
  lua_State  * mLuaState;
public:
  LuaVM( DWORD libs = LUALIB_BASE|LUALIB_TABLE|LUALIB_STRING|LUALIB_MATH|LUALIB_IO );

  virtual ~LuaVM();

  virtual void execute( const std::string & rData );

  virtual void execute( LuaScript* pScript);

  lua_State* getLuaState() const { return mLuaState; }
};

/** Scripting system interface.
  Provides means to create virtual machines and scripts.
*/
class ScriptingSystemLua 
{
public:
  static ScriptingSystemLua& instance();

  virtual LuaVM* createVM();
  
  LuaVM* getVM(int index);

  virtual LuaScript* createScriptFromFile( const std::string & rFile );

private:
  ScriptingSystemLua();
  virtual ~ScriptingSystemLua();
  

  typedef std::vector< LuaVM* > VMList;
  VMList  mVMs;
  static ScriptingSystemLua* _instance;
  
};

/** Script binder,
  Provides means to export system interfaces to script
*/
class Binder
{
public:
  void bind( LuaVM* pVM );
};

namespace luabind
{
  namespace detail {
    template<class T>
    struct delete_s;
    template<class T>
    struct destruct_only_s;  
  }
}

/** global index,used to decide executing a script or not.
  it's for speed purpose and avoid wasting cpu time
*/
class ScriptQuest
{
public:
  enum {
    NPC_VISIT = 1,
    NPC_KILL = 2,
    OBJ_USE = 4,
    OBJ_GET = 8,
    OBJ_DROP = 16,
  };
    
  static ScriptQuest& get_instance();
  
  void add(int type,int id);
  bool has(int type,int id) const;
  
  //for speed
  void sort();

  
private:
  ScriptQuest() {}  
  ~ScriptQuest() { }

  friend class luabind::detail::delete_s<ScriptQuest>;  
  friend class luabind::detail::destruct_only_s<ScriptQuest>;  
  
  int hash(int type,int id) const;

  static ScriptQuest*   _instance;
  
  hash_set<int> _sq;
};

class SceneUser;
extern SceneUser* current_user;

inline int execute_script_event(SceneUser * user,const char* func)
{
  if (user == NULL )
  {
    Zebra::logger->error("设置任务脚本当前用户空指针");
    return 0;
  }
  else
    current_user=user;
  try {
    LuaVM* vm = ScriptingSystemLua::instance().getVM(0);
    int ret = luabind::call_function<int>(vm->getLuaState(),func);
    return ret;
  }
  catch (luabind::error& e)
  {
    Zebra::logger->debug("CATCHED Luabind EXCEPTION:%s\n%s",func,e.what());
    return 0;
  }          
  catch (const char* msg)
  {
    Zebra::logger->debug("CATCHED (...) EXCEPTION:%s\n%s",func,msg);
    return 0;
  }
  
  return 0;  
}

template<typename P>
int execute_script_event(SceneUser * user,const char* func,P& p)
{
  if (user == NULL )
  {
    Zebra::logger->error("设置任务脚本当前用户空指针");
    return 0;
  }
  else
    current_user=user;
  try {
    LuaVM* vm = ScriptingSystemLua::instance().getVM(0);
    int ret = luabind::call_function<int>(vm->getLuaState(),func,p);
    return ret;
  }
  catch (luabind::error& e)
  {
    Zebra::logger->debug("CATCHED Luabind EXCEPTION:%s\n%s",func,e.what());
    return 0;
  }          
  catch (const char* msg)
  {
    Zebra::logger->debug("CATCHED (...) EXCEPTION:%s\n%s",func,msg);
    return 0;
  }
  
  return 0;  
}

template<typename P1,typename P2,typename P3>
int execute_script_event(SceneUser * user,const char* func,P1& p1,P2& p2,P3& p3)
{
  if (user == NULL )
  {
    Zebra::logger->error("设置任务脚本当前用户空指针");
    return 0;
  }
  else
    current_user=user;
  Zebra::logger->debug("execute_script_event:%s,%s,%u,%u",func,p1->name,p2,p3);
  try {
    LuaVM* vm = ScriptingSystemLua::instance().getVM(0);
    int ret = luabind::call_function<int>(vm->getLuaState(),func,p1,p2,p3);
    return ret;
  }
  catch (luabind::error& e)
  {
    Zebra::logger->debug("CATCHED Luabind EXCEPTION:%s\n%s",func,e.what());
    return 0;
  }          
  catch (const char* msg)
  {
    Zebra::logger->debug("CATCHED (...) EXCEPTION:%s\n%s",func,msg);
    return 0;
  }
  
  return 0;  
}

SceneUser* me();
void set_me(SceneUser* user);

bool sys(SceneUser* target,int type,const char* msg);

void show_dialog(SceneNpc* npc,const char* menu);

class Vars;

void set_var(Vars* vars,const char* name,int value);
void set_varS(Vars* vars,const char* name,const char * value);

int get_var(Vars* vars,const char* name);
const char * get_varS(Vars* vars,const char* name);

void refresh_status(SceneNpc* npc);

void refresh_npc(int id);

int npc_tempid(SceneNpc* npc);

int npc_id(SceneNpc* npc);

void refresh_quest(int id);



/** brief 提供对脚本中全局变量的支持
  
*/
class GlobalVars
{
public:
  static Vars* add_g();
  static Vars* add_t();
  static Vars* add_f();
};

bool check_money(SceneUser* user,int money);
bool remove_money(SceneUser* user,int money);
void add_money(SceneUser* user,int money);

bool have_ob(SceneUser* user,int id,int num,int level,int type);
DWORD get_ob(SceneUser* user,int id,int level );
bool del_ob(SceneUser* user,DWORD id);
int  space(SceneUser* user);

Vars * get_familyvar(SceneUser* user,int dummy);
Vars * get_uservar(SceneUser* user,int dummy);
Vars * get_tongvar(SceneUser* user,int dummy);

void add_exp(SceneUser* user,DWORD num,bool addPet,DWORD dwTempID,BYTE byType,bool addCartoon);
int get_time();
double diff_time(int,int);

extern NFilterModuleArray g_nFMA;