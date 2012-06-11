
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
     ** \brief ���������ʻ�����
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
     ** \brief ��ÿ���û�ִ��
     ** \param exec ִ�нӿ�
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
 * \brief AI����,npc�����Ļ�������
 * �ߡ�������Ѳ�ߡ���ת��
 *
 */
enum SceneNpcAIType
{
  NPC_AI_NORMAL,///��ͨAI
  NPC_AI_SAY,///˵��
  NPC_AI_MOVETO,///�ƶ���ĳλ��
  NPC_AI_ATTACK,///��ĳ��Χ�ڹ���
  NPC_AI_FLEE,///�������
  NPC_AI_RETURN_TO_REGION,///�ص����Χ
  NPC_AI_GO_ON_PATH,///����һ��·���ƶ�
  NPC_AI_CHANGE_MAP,///�л���ͼ��ͬһ�������ڣ�
  NPC_AI_WARP,///ͬһ��ͼ��˲���ƶ�
  NPC_AI_PATROL,///Ѳ��
  NPC_AI_CLEAR,///�����npc
  NPC_AI_WAIT,///�ȴ�,ʲôҲ����
  NPC_AI_GUARD_DOOR,///����
  NPC_AI_GUARD_ARCHER,///����
  NPC_AI_GUARD_GUARD,///����
  NPC_AI_GUARD_PATROL,///Ѳ����ʿ
  NPC_AI_DROP_ITEM,///������
  NPC_AI_CALL_PET,///�г���
  NPC_AI_RANDOM_CHAT///���˵��
};

///npcAI��־����
enum NpcAIFlag
{
	AIF_ATK_PDEF            = 0x00000001, ///���ȹ��������͵ĵ���
	AIF_ATK_MDEF            = 0x00000002, ///���ȹ���ħ����͵ĵ���
	AIF_ATK_HP              = 0x00000004, ///���ȹ�������ֵ��͵ĵ���
	AIF_GIVEUP_10_SEC       = 0x00000008, ///׷��10�����Ŀ��
	AIF_GIVEUP_6_SEC        = 0x00000010, ///6��δ�ܵ��˺�����Ŀ��
	AIF_GIVEUP_3_SEC        = 0x00000020, ///3��δ�ܵ��˺�����Ŀ��
	AIF_FLEE_30_HP    = 0x00000040, ///HP30%��������4��
	AIF_FLEE_3_ENEMY_4  = 0x00000080, ///��3�����ϵ���Χ������4��
	AIF_NO_BATTLE    = 0x00000100,///��ս��npc
	AIF_NO_MOVE    = 0x00000200, ///���ƶ���������������·��ȣ�
	AIF_WARP_MOVE    = 0x00000400, ///˲�Ʒ�ʽ�ƶ�
	AIF_CALL_FELLOW_7       = 0x00000800, ///�ٻ�7*7��Χ��δ����npc(����50%)
	AIF_CALL_FELLOW_9       = 0x00001000, ///�ٻ�9*9��Χ��δ����npc������50%��
	AIF_CALL_BY_ATYPE       = 0x00002000, ///�ٻ�ͬ�ֹ������͵�ͬ�飨����������־������
	AIF_HELP_FELLOW_5  = 0x00004000,///����5*5��Χ�ڵ�ͬ�鹥�������ڱ���npc��
	AIF_ATK_MASTER    = 0x00008000,///ֱ�ӹ������������
	AIF_ATK_REDNAME    = 0x00010000,///�������������
	AIF_DOUBLE_REGION  = 0x00020000,///������Χ�ӱ�
	AIF_SPD_UP_HP20    = 0x00040000,///hp20%�����ƶ��ٶȼӱ�
	AIF_ASPD_UP_HP50  = 0x00080000,///hp50%���¹����ٶȼӱ�
	AIF_ACTIVE_MODE    = 0x00100000,///��������
	AIF_RUN_AWAY    = 0x00200000,///�������
	AIF_LOCK_TARGET    = 0x00400000,///���л�����Ŀ��ֱ����
	AIF_RCV_UNDER_30  = 0x00800000,///hp30%���³�����Ѫ1%
	AIF_RCV_REST    = 0x01000000,///����ս��30����Ѫһ��5%
	AIF_LIMIT_REGION  = 0x02000000  ///���ƻ��Χ
};

///npc˵��������
enum NpcChatType
{
  NPC_CHAT_ON_FIND_ENEMY = 1,///���ֵ���
  NPC_CHAT_ON_ATTACK,///����ʱ˵�Ļ�
  NPC_CHAT_ON_RETURN,///׷�𷵻�ʱ˵�Ļ�
  NPC_CHAT_ON_DIE,///����ʱ��˵�Ļ�
  NPC_CHAT_ON_FLEE,///����ʱ˵�Ļ�
  NPC_CHAT_ON_HIT,///����ʱ˵�Ļ�
  NPC_CHAT_ON_HELP,///����ͬ��ʱ˵�Ļ�
  NPC_CHAT_ON_BE_HELP,///ͬ��������ʱ˵�Ļ�
  NPC_CHAT_RANDOM    ///���˵��
};

/**
 * \brief һ��AI�Ķ���
 *
 */
struct t_NpcAIDefine
{
  ///����,NPC�ڸý׶ε���Ҫ����
  SceneNpcAIType type;
  ///λ�� ���ݲ�ͬ����λ�õ�����Ҳ�Բ���ͬ
  ///�ƶ�ʱ��ʾĿ�ĵ�,������ʾ���Χ����
  zPos pos;
  ///��Χ 
  ///�ƶ�ʱ��ʾ����Ŀ�ĵص��ж���Χ,������ʾ���Χ
  int regionX,regionY;
  //zRTime endTime;
  ///��AI�ĳ���ʱ��
  int lasttime;

  ///�Ƿ���������
  bool flee;
  ///���ܵķ���
  int fleeDir;
  ///���ܼ���
  int fleeCount;

  ///�л���ͼʱ,Ҫȥ�ĵ�ͼ
  ///˵��ʱ,Ҫ˵�Ļ�
  char str[MAX_CHATINFO];


  /**
   * \brief Ĭ�Ϲ��캯��
   *
   */
  t_NpcAIDefine()
    :type(NPC_AI_NORMAL),pos(zPos(0,0)),regionX(2),regionY(2),lasttime(0),flee(false),fleeDir(-1),fleeCount(0)
    {
      bzero(str,sizeof(str));
    }
  
  /**
   * \brief ���캯��
   *
   *
   * \param type AI����
   * \param pos λ��
   * \param regionX ��Χ��
   * \param regionY ��Χ��
   * \param lasttime ����ʱ��
   * \return 
   */
  t_NpcAIDefine(SceneNpcAIType type,const zPos &pos,int regionX,int regionY,int lasttime)
    :type(type),pos(pos),regionX(regionX),regionY(regionY),lasttime(lasttime)
    {
      bzero(str,sizeof(str));
    }


  /**
   * \brief �������캯��
   *
   * \param ad Ҫ���ƵĶ���
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
   * \brief ��ֵ
   *
   * \param ad Ҫ�����Ķ���
   * \return ���������ַ
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
 * \brief AI������
 * ���������Զ�ȡNPC�ű�,ʹNPC���սű�������
 * 
 * ��������������¼�,ʱ�䡢��������������
 * ���ݲ�ͬ������ΪNPC���ò�ͬ��AI
 *
 */
class NpcAIController
{
  static const int npc_call_fellow_rate;///NPC�ٻ�ͬ��ļ���
  static const int npc_one_checkpoint_time;///NPC����·���ƶ�ʱ,��һ��·����ʱ��
  static const int npc_checkpoint_region;///NPC�ƶ�,����һ��·����ж���Χ
  static const int npc_onhit_stop_time;///����NPC�ƶ��б�����ʱ,ֹͣ��ʱ��
  static const int npc_flee_distance;///NPC���빥���ߵľ���
  static const int npc_min_act_region;///NPC��С���Χ
  
  ///AI����,AI�����������е�˳������ִ��
  std::vector<t_NpcAIDefine> phaseVector;
  ///��ǰ��AI����
  DWORD curPhase;
  ///��ǰAI�Ľ���ʱ��
  zRTime phaseEndTime;

  ///�ű��ظ�����
  ///-1������ѭ��  0��ֹͣ  >0��ѭ������
  int repeat;

  ///�Ƿ������˽ű�
  bool active;
  void nextPhase(int index);
  void on_phaseEnd();
  SceneNpcAIType parseAction(char *);
  
  ///�����������Ƶ�npc
  SceneNpc * npc;

  ///��ǰ��AI�ͱ����ǰһ��AI
  t_NpcAIDefine curAI,oldAI;
  ///���Χ������
  zPos actPos;
  ///���Χ�Ŀ�͸�
  int actRegionX,actRegionY;
  bool outOfRegion() const;
  void returnToRegion();

  bool arrived(zPos pos = zPos(0,0),int x = -1,int y = -1);
  bool dstReached();
  ///�Ƿ񵽴�Ŀ�ĵ�
  bool reached;

  ///Ŀ�ĵ�ͼ
  char dstMap[32];
  ///Ŀ��λ��
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

//��Ҫ����AI������npc
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

/* һЩ����npc��ID */
const DWORD COUNTRY_MAIN_FLAG = 58001;
const DWORD COUNTRY_SEC_FLAG  =  58002;
const DWORD COUNTRY_MAIN_GEN = 58200;
const DWORD COUNTRY_SEC_GEN  = 58201;
const DWORD COUNTRY_KING_MAIN_FLAG = 58005;
const DWORD COUNTRY_KING_SEC_FLAG = 58006;
const DWORD COUNTRY_EMPEROR_MAIN_GEN = 58203;
const DWORD COUNTRY_EMPEROR_SEC_GEN = 58204;

const DWORD ALLY_GUARDNPC = 54100;//�˹��ڳ�
const DWORD EMPEROR_HORSE_ID = 3202;//�ʵ۵���ID
const DWORD KING_HORSE_ID = 3204;//��������ID

/**
 * \brief ����Npc
 *
 */
class SceneNpc : public SceneEntryPk,public zAStar<>,public zAStar<2>
{
  friend class NpcAIController;

  static const DWORD maxUniqueID = 100000;

  public:
  
  //unsigned short dupIndex;

  bool isMainGeneral();//�Ƿ�󽫾���

  zRTime reliveTime;//����ʱ��

  int targetDistance;//�뵱ǰĿ�����̾���
  int closeCount;//׷�����,10��������̾���û�м�������ΪĿ�겻�ɵ���

  std::list<ScenePet *> semipetList;//������б�

  /**
   * \brief ����Npc����״̬
   *
   */
  enum SceneNpcChase
  {
    CHASE_NONE,     /// û�и���״̬
    CHASE_ATTACK,   /// ���ٹ���״̬
    CHASE_NOATTACK    /// ��ͨ����״̬
  };

  /**
   * \brief Npc����
   * ��̬�Ļ��Ƕ�̬�����
   */
  enum SceneNpcType
  {
    STATIC,       /// ��̬��
    GANG        /// ��̬��
  };

  /**
   * \brief Npc��������
   *
   */
  zNpcB *npc;

  /**
   * \brief ��ǿNpc��������
   *
   */
  zNpcB *anpc;

  /**
   * \brief Npc��������
   *
   */
  const t_NpcDefine *define;

  /**
   * \brief npc��ǰ����ֵ
   *
   */
  DWORD hp;

  ///�ϴη���ʱ��hp
  DWORD lasthp;

  ///��Ѫ���
  bool needRecover;
  ///�´λ�Ѫ��ʱ��
  zRTime rcvTimePet;//������Ϣ
  zRTime rcvTimeUnder30;//hp30����
  zRTime rcvTimeRest;//����ս��
  //hp30���»�Ѫ
  bool recoverUnder30;
  //����ս����Ѫ
  //bool recoverLeaveBattle;

  bool checkRecoverTime(const zRTime& ct);
  void setRecoverTime(const zRTime& ct,int delay);

  virtual bool recover();
  /// npc����������ʱ��(���û���þ����䴴��ʱ��)
  DWORD dwStandTime;

  ///npc����������ʱ��
  DWORD dwStandTimeCount;

  ///������С������
  WORD appendMinDamage;

  ///������󹥻���
  WORD appendMaxDamage;

  ///�Ƿ��Ѿ�����Ŀ��
  bool lockTarget;

  ///���ǵ�npc,�湥��һ��ɾ��
  bool isRushNpc;

  ///���������ٻ�����npc
  int summonByNpcMap(std::map<DWORD,std::pair<DWORD,DWORD> > map);

  ///�Ƿ��ٻ���
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
   * \brief �ó�������
   */
  virtual void relivePet(){};

  DWORD catchme; ///�������﹥���Լ�
  int boostupPet; /// ��ǿ����
  DWORD boostupPetMDef; //��ǿ����ķ�������
  DWORD boostupSummon; ///�ٻ��޹�����ǿ
  DWORD boostupHpMaxP;  ///��������ֵ����
  DWORD dwReduceDam;  /// �ٻ����˺��ۼ�
  DWORD giddy;   ///������ʱ��ʹ�Է�ѣ�εļ���

  BYTE notifystep; //��BOSS֪ͨ����

  //*
  static void AI(const zRTime& ctv,MonkeyNpcs &affectNpc,const DWORD group,const bool every);

  /**
   * \brief ǿ�Ƹ����û�,������Ѿ��ڸ����û�,��ô��45%�ļ��ʽ�Ŀ��ת����Ŀǰ���û�
   * \param pAtt Ҫ���ٵĶ���
   */
  bool forceChaseUser(SceneEntryPk *pAtt);
  /**
   * \brief ���ͻ�����Ϣת�����Ự������
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
  bool warp(const zPos &pos,bool ignore=false);//��ת
  void jumpTo(zPos &newPos);
  bool gotoFindPath(const zPos &srcPos,const zPos &destPos);
  bool goTo(const zPos &pos);
  bool shiftMove(const int direct);
  void set_quest_status(SceneUser* user);
  void setStandingTime(DWORD standTime);
  void refreshExpmapAttackTime(SceneUser* pAtt);

  /**
   * \brief �ı��ɫ��hp
   * \param hp �����HP
   */
  void changeHP(const SDWORD &hp);

  /**
   * \brief ���ֱ���˺�
   * \param pAtt ������
   * \param dam �˺�ֵ
   * \param notify ֪ͨ�˺���ʾ
   */
  SWORD directDamage(SceneEntryPk *pAtt,const SDWORD &dam,bool notify=false);

  /**
   * \brief �ı��ɫ��sp
   * \param sp �����SP
   */
  void changeSP(const SDWORD &sp);

  /**
   * \brief �ı��ɫ��mp
   * \param mp �����MP
   */
  void changeMP(const SDWORD &mp);

  /**
   * \brief �ڱ��Լ�����֮ǰ�Ĵ���,����,װ����Ĵ���,������Ч�����жϵ�
   * \param pUser ������
   * \param rev ���ι����Ĵ���ָ��
   * \param physics �Ƿ�������
   * \param good Ϊtrue�����,Ϊfalse��Ҫ�жϻر���
   * \return trueΪ��ι�������Ч��,falseΪһ����Ч�Ĺ���
   */
  bool preAttackMe(SceneEntryPk *pUser,const Cmd::stAttackMagicUserCmd *rev,bool physics=true,const bool good = false);

  /**
   * \brief ��ɫ������
   * \param pEntry ������
   * \param rev ���ι����Ĵ�����Ϣ
   * \param physics �Ƿ�������
   * \return trueΪ��ι�������Ч��,falseΪһ����Ч�Ĺ���
   */
  bool AttackMe(SceneEntryPk *pEntry,const Cmd::stAttackMagicUserCmd *rev,bool physics=true,SWORD rangDamageBonus=0);

  /**
   * \brief ��ɫ������N��
   * \param dwAttTempID �����ߵ���ʱID
   * \param grids ���˼���
   */
  void standBack(const DWORD dwAttTempID,DWORD grids);

  /**
   * \brief ������Ŀ�껻��dwTempID��ָ��Ľ�ɫ���
   * \param dwTempID Ŀ���ɫ����ʱID
   */
  void changeAttackTarget(const DWORD &dwTempID);

  /**
   * \brief �ý�ɫ����
   */
  void toDie(const DWORD &dwTempID);

  /**
   * \brief ֪ͨ�ͻ�������ֵ�ı仯
   */
  void attackRTHpAndMp();

  /**
   * \brief ����npcѪ��
   */
  void showHP(SceneUser *pUser,DWORD npchp);

  /**
   * \brief �������Ƿ񹥻�����npc
   */
  bool isAttackMe(SceneEntryPk *);
  /**
   * \brief �жϽ�ɫ�Ƿ�����
   * \return trueΪ����
   */
  bool isDie();
  /**
   * \brief ��ȡ��ɫ�ļ���
   */
  virtual DWORD getLevel() const;

  /**
   * \brief ��Ҫ��ְҵ����,��������ʹ�õļ�������
   */
  bool needType(const DWORD &needtype);

  /**
   * \brief ��Ҫ��ְҵ����,��������ʹ�õļ�������
   */
  bool addSkillToMe(zSkill *skill);

  /**
   * \brief �Ƿ��иü�����Ҫ������
   * \return true �� false û��
   */
  bool needWeapon(DWORD skillid);

  /**
   * \brief �Ƿ�Pk����
   * \other PK�����
   * \return true �� false ��
   */
  virtual bool isPkZone(SceneEntryPk *other=NULL);

  /**
   * \brief ������Ʒ�����ͷ���
   * \param object ������Ʒ������
   * \param num ������Ʒ������
   * \return true ���ĳɹ� false ʧ��
   */
  bool reduce(const DWORD &object,const BYTE num);

  /**
   * \brief ����������Ʒ�Ƿ��㹻
   * \param object ������Ʒ������
   * \param num ������Ʒ������
   * \return true �㹻 false ����
   */
  bool checkReduce(const DWORD &object,const BYTE num);

  /**
   * \brief ʩ�ż��������µ�����MP,HP,SP
   * \param base ���ܻ������Զ���
   * \return true ���ĳɹ� false ʧ��
   */
  bool doSkillCost(const zSkillB *base);

  /**
   * \brief ���ʩ�ż��������µ�����MP,HP,SP�Ƿ��㹻
   * \param base ���ܻ������Զ���
   * \return true ���ĳɹ� false ʧ��
   */
  bool checkSkillCost(const zSkillB *base);

  /**
   * \brief ��������ʩ�ųɹ�����,������μ����Ƿ����ʩ��
   * \return true �ɹ� false ʧ��
   */
  bool checkPercent();

  /**
   * \brief �ж��Ƿ��ǵ���
   * \return true �� false ����
   */
  //bool isEnemy(SceneUser *pUser);

  /**
   * \brief ��������ض���ɫ���Ӷ��⾭��,����������װ���Ķ��⾭��
   * \param wdExp ���䵽�ľ���
   * \param pUser �������
   * \return ���¼����ľ���ֵ
   */
  DWORD addOtherExp(DWORD wdExp,SceneUser *pUser);

  /**
   * \brief ��������hp
   */
  virtual void changeAndRefreshHMS(bool lock=true,bool sendData=true);

  /**
   * \brief ���ݵȼ�������¼��㾭��
   * \param wdExp ���䵽�ľ���ֵ
   * \param char_level ��ɫ�ȼ�
   * \return ���¼����ľ���ֵ
   */
  DWORD levelExp(DWORD wdExp,DWORD char_level);

  /**
   * \brief ���·��ͱ�NPC�ĵ�ͼ����
   */
  void reSendMyMapData();

  //���ó��������
  virtual void setMaster(SceneEntryPk *){}

  //��ȡ���������
  virtual SceneEntryPk *getMaster() {return 0;}
  /**
   * \brief �õ����ϲ������
   *
   * \return ����
   */
  virtual SceneEntryPk *getTopMaster(){return this;}

  //�л�����
  bool changeMap(Scene *newScene,const zPos &pos);
  void setAspeedRate(float rate);
  void resetAspeedRate();

  /**
   * \brief ֪ͨѡ���Լ����û���hp��mp�����仯
   */
  void sendtoSelectedHpAndMp();
  void sendtoSelectedState(DWORD state,WORD value,WORD time);

  bool createRush();
  virtual bool moveToMaster();

  //AI��صķ���
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

  //�ص���
  virtual void on_reached() { }
  virtual void on_death(SceneEntryPk* att){}
  virtual void check() { }
  bool dreadProcess();
  int IsOppose(DWORD five);

  virtual void clearMaster();

  //zPos actPos;//�λ��
  //DWORD actRegionX,actRegionY;//���Χ
  //t_NpcAIDefine oldAI;
  ///npc��������ָ��
  NpcAIController * AIC;
  bool setScript(int id);
  void clearScript();
  void assaultMe(BYTE attacktype,DWORD tempid);

  BYTE getAType();

  ///npcAI��־
  DWORD aif;
  ///�����AIģʽ
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

  ///�ι���Ŀ��
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
   * \brief ��Ʒ����
   *
   */
  DWORD dwNpcLockedUser;
protected:
  /// NPC�������˵ķ�Χ
  static const int npc_search_region = 5;
  /// NPCԶ��Ŀ�����׷�ٵķ�Χ
  static const int npc_lost_target_region = 12;
  /// NPCԶ����Χ����׷�ٵľ���
  static const int npc_out_of_range_region = 20;
  /// ���ﱣ����������ߵķ�Χ
  static const int npc_pet_chase_region = 2;
  /// ���������˳����˾��������
  static const int npc_pet_run_region = 4;
  /// ���������˳����˾�������ת
  static const int npc_pet_warp_region = 6;
private:
  /**
   * \brief NPC�����
   *
   */
  bool clearMe;

  ///npc��ǰ��AI
  t_NpcAIDefine AIDefine;

  ///npc������
  //SceneUser * master;

  ///�ƶ��ٶȱ���
  float speedRate;
  ///�����ٶȱ���
  float aspeedRate;

  ///�Ƿ��Ѿ���Ϊhp<20%�������ƶ��ٶ�
  bool speedUpUnder20;

  ///�Ƿ��Ѿ���Ϊhp<50%�����˹����ٶ�
  bool aspeedUpUnder50;


  bool processDeath(SceneEntryPk *pAtt);

  ///�Ƿ����ں�����(ms)
  int backOffing;

  /**
   * \brief ��һ�α�����ʱ��(������
   *
   */
  /// ��һ�α�������ʱ��
  zRTime first_time;

  ///���붨ʱ��
  Timer _half_sec;

  ///1�붨ʱ��
  Timer _one_sec;

  ///3�붨ʱ��
  Timer _3_sec;

  /**
   * \brief ���ٷ�ʽ
   *
   */
  ///npc�ĸ��ٷ�ʽ
  SceneNpcChase chaseMode;

  //DWORD  dwNpcChasedEntryType;
  /**
   * \brief ������Ŀ��ĵı��
   *
   */
  //DWORD curTarget;
  //DWORD  dwNpcChasedEntryID;

  /**
   * \brief ��Ʒ����ʱ��
   *
   */
  zRTime lockedUserTime;
  /**
   * \brief ��һ���ƶ�ʱ��
   *
   */
  zRTime nextMoveTime;
  /**
   * \brief ��һ�ι���ʱ��
   *
   */
  zRTime nextAttackTime;

  ///��������״̬��ʱ��
  zRTime showTime;

  /**
   * \brief �Ƿ���Ե�����Ʒ
   *
   */
  bool lostObject;

  //�Ƿ���й��﹥�ǵ��ж�
  bool mayRush;

  /**
   * \brief Npc����
   * ��̬�Ļ��Ƕ�̬�����
   */
  const SceneNpcType type;

  /**
   * \brief ��ʱ��ŵ����Է�����
   * ��Ҫ�ڴ�����̬Npc��ʱ����Ҫʹ��
   */
  static DWORD serialID;
  /**
   * \brief ��ʱ��ŵ�Ψһ������
   * ��Ҫ�ڴ�����̬Npc��ʱ����Ҫʹ��
   */
  static zUniqueDWORDID uniqueID;

  //typedef hash_map<DWORD,t_expRec> NpcHpHashmap;
  typedef std::map<DWORD,t_expRec> NpcHpHashmap;
  typedef NpcHpHashmap::iterator NpcHpHashmap_iterator;
  typedef NpcHpHashmap::const_iterator NpcHpHashmap_const_iterator;
  typedef NpcHpHashmap::value_type NpcHpHashmap_pair;
  ///����ֵ�б�
  ///���Էֵ���npc���������б�
  NpcHpHashmap expmap;


public:
  /**
   * \brief ���ý�ɫ�ĵ�ǰ״̬,�����ݵ�ǰ״̬���ֽ�ɫ����Ч���߹ر���Ч
   * \param state ״̬ID ����enum SceneEntryStateȡֵ
   * \param isShow �Ƿ���ʾЧ��
   * \return trueΪ��ι�������Ч��,falseΪһ����Ч�Ĺ���
   */
  void showCurrentEffect(const WORD &state,bool isShow,bool notify=true);
  /**
   * \brief NPCʬ���ʹ��״̬
   * trueΪ�Ѿ���ʹ��
   */
  bool isUse;
};

/**
 * \brief ��ÿ������npcִ�еĻص�
 *
 */
struct specialNpcCallBack
{
  public:
    virtual bool exec(SceneNpc *npc)=0;
    virtual ~specialNpcCallBack(){};
};

/**
 * \brief �ٻ�һ��npc
 * \param define npc����ṹ
 * \param pos �ٻ�λ��
 * \param base npc������Ϣ
 * \param standTime ͼ��ϵ�ĳ���ʱ��
 * \param abase ��ǿnpc�Ļ�����Ϣ
 * \return �ٻ���npc��ָ��,ʧ�ܷ���0
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
      initNpc(sceneNpc,NULL,pos);//zPos(0,0));//��NULL����define.region��Χ��ѡ��λ��
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
        Zebra::logger->debug("%s ��ʼ״̬ %u",sceneNpc->name,sceneNpc->getState());
#endif
      return sceneNpc;
    }
    else
    {
      Zebra::logger->fatal("Scene::summonOneNpc:SceneNpc�����ڴ�ʧ��");
      SAFE_DELETE(pDefine);
    }
  }
  else
  {
    Zebra::logger->fatal("Scene::summonOneNpc:t_NpcDefine�����ڴ�ʧ��");
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

  ///����
  //SceneEntryPk * master;
  DWORD masterID;
  DWORD masterType;

  DWORD delCount;

  ///��������
  Cmd::petType type;
  ///������ж�ģʽ
  //WORD petAI;

  ///�ȼ�
  //DWORD level;

  ///�Ƿ���Ϊ������̫Զ���������ٶ�
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

  //��GuardNpc�̳���
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
 * \brief npc������
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
     * \brief ��ÿ��npcִ�лص�����
     *
     *
     * \param exec �ص�����
     * \return �Ƿ����ִ��
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
     * \brief ɾ������������npc
     *
     * \param pred �ж�����
     */
    template <class YourNpcEntry>
    void removeNpc_if(removeEntry_Pred<YourNpcEntry> &pred)
    {
      rwlock.wrlock();
      removeEntry_if<>(pred);
      rwlock.unlock();
    }

    /**
     * \brief ɾ��һ�����ڵ�npc
     *
     *
     * \param scene ����
     */
    void removeNpcInOneScene(Scene *scene);
    void SpecialAI();
    /**
     * \brief ��ÿ������npcִ�лص�������
     *
     *
     * \param callback �ص�����
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
    ///specialNpc��ָ��ʹû������ڸ���ҲҪ�������NPC
    ///�������boss������NPC
    MonkeyNpcs specialNpc;

    SceneNpcManager();
    ~SceneNpcManager();

    ///SceneNpcManager��Ψһʵ��
    static SceneNpcManager *snm;
    ///��д��
    zRWLock rwlock;

    bool getUniqeID(DWORD &tempid) { return true; }       
    void putUniqeID(const DWORD &tempid) {}

    ///npc���˵��������
    std::map<DWORD,std::vector<std::string> > NpcCommonChatTable;
    ///npc���˵���ĸ���
    std::map<DWORD,int> NpcCommonChatRate;

    bool loadNpcCommonChatTable();
};

/**
 * \brief ��������������
 *
 */
class SceneTask : public zTCPTask,public zEntry,public MessageQueue
{

  public:

    /**
     * \brief ���캯��
     *
     * \param pool �������ӳ�ָ��
     * \param sock TCP/IP�׽ӿ�
     * \param addr ��ַ
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
     * \brief ����������
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
 * \brief ���������������ӹ�����
 *
 */
class SceneTaskManager : private zNoncopyable
{

  public:

    /**
     * \brief ��������
     *
     */
    ~SceneTaskManager() {};

    /**
     * \brief ��ȡ���������ӹ�����Ψһʵ��
     *
     * \return ���������ӹ�����Ψһʵ��
     */
    static SceneTaskManager &getInstance()
    {
      if (NULL == instance)
        instance = new SceneTaskManager();

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
     * \brief ���Ψһʵ��ָ��
     *
     */
    static SceneTaskManager *instance;

    /**
     * \brief ���캯��
     *
     */
    SceneTaskManager() {};

    /**
     * \brief �����ӹ�����������
     *
     */
    typedef hash_map<WORD,SceneTask *> SceneTaskHashmap;
    /**
     * \brief ������������������
     *
     */
    typedef SceneTaskHashmap::iterator SceneTaskHashmap_iterator;
    /**
     * \brief ����������������������
     *
     */
    typedef SceneTaskHashmap::const_iterator SceneTaskHashmap_const_iterator;
    /**
     * \brief ����������ֵ������
     *
     */
    typedef SceneTaskHashmap::value_type SceneTaskHashmap_pair;
    /**
     * \brief �������ʻ������
     *
     */
    zRWLock rwlock;
    /**
     * \brief �����ӹ�����������
     *
     */
    SceneTaskHashmap sceneTaskSet;

};

/**
 * brief ���� �������������ӿͻ�����
 *
 * ���� �뵵����������������ȡ����
 * TODO ��ʱֻ��һ������������
 * 
 */
class RecordClient : public zTCPBufferClient,public MessageQueue
{

  public:

    /**
     * \brief ���캯��
     * ���ڵ��������Ѿ���ѹ�����ģ����ڵײ㴫���ʱ��Ͳ���Ҫѹ����
     * \param name ����
     * \param ip ��ַ
     * \param port �˿�
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
    * \brief ���캯��
    * \param  name ����
    * \param  ip   ��ַ
    * \param  port �˿�
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

/// ����
extern SessionClient *sessionClient;

/**
 * \brief NPC����
 *
 */
class NpcTrade
{

  public:

    enum
    {
      NPC_BUY_OBJECT    = 1, ///��
      NPC_SELL_OBJECT    = 2, ///��
      NPC_REPAIR_OBJECT  = 4, ///����
      NPC_MAKE_OBJECT    = 8, ///����
      NPC_UPDATE_OBJECT  = 16, ///����
      NPC_MERGE_OBJECT  = 32, ///�ϳ�
      NPC_ENCHANCE_OBJECT = 64,//��Ƕ
      NPC_MERGE_SOUL_OBJECT = 128,//���Ǻϳ�
      NPC_HOLE_OBJECT = 256,//���
      NPC_STORE_OBJECT = 512,//�ֿ�
      NPC_DECOMPOSE_OBJECT = 1024,//�ֽ�
    };

    struct NpcItem
    {
      DWORD id;          ///��Ʒ���
      WORD  kind;          ///��Ʒ����
      WORD  lowLevel;        ///��͵ȼ�
      WORD  level;        ///��ߵȼ�
      WORD  itemlevel;      ///������Ʒ�ĵȼ�
      WORD  action;        ///��������
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
     * \brief �õ�Ψһʵ��
     *
     *
     * \return npc����ϵͳ
     */
    static NpcTrade &getInstance()
    {
      if (NULL == instance)
        instance = new NpcTrade();

      return *instance;
    }

    /**
     * \brief ж��Ψһʵ��
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
     * \brief npc�Ի���
     *
     */
    struct NpcDialog
    {
      DWORD npcid;      ///Npc���
      char menu[6144];    ///�˵�����
      NpcItemMultiMap items;  ///��Ʒ����
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
 * \brief �ڳ�
 *
 * �����װ�˶�����NPC�ڳ��ĳ���
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
 * \brief ����Ƶ��
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
 * \brief Ƶ��������
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

/// ����GM��id,ֻ��1������GM
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
   * \brief ���ܲ���ָ��
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

    BYTE expRate;//�ͷž��������
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
      DICE_STATE_CREATE,//δ��ʼ������������
      DICE_STATE_ROLLING,//ɫ����ת���ȴ�ֹͣ��Ϣ
      DICE_STATE_END,//һ�ֿ�ʼ֮ǰ�ȴ�˫��׼��
      DICE_STATE_DEL//�ȴ�ɾ��
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

    DWORD round;//n��Ŀ
    DWORD money;
    DiceState gameState;

    DWORD tempid1,tempid2;
    DWORD value1,value2;
    bool continue1,continue2;
    char name1[MAX_NAMESIZE],name2[MAX_NAMESIZE];
};

/**
 * \brief ����װ���ṹ
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
   * \brief ���캯��
   *
   *
   * \param object: ����װ��
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
 * \brief ������Ϣ
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
     * \brief �õ�Ψһʵ��
     *
     *
     * \return Ψһʵ��
     */
    static CharInitInfo &getInstance()
    {
      if (NULL == instance)
        instance = new CharInitInfo();

      return *instance;
    }

    /**
     * \brief ɾ��Ψһʵ��
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
 * \brief ��Ʒ�������
 *
 * ��װ�˼������õĲ���
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
 * \brief ��
 *
 * ��װ�˶������صĴ�����
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
 * \brief ����ʯ
 *
 * ��װ�˶Ի���ʯ�Ĵ�����,������Ƕ���ϳɵ�
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
   * \brief �������Լ���
   *
   * \param first: ��һ�����ʯ����
   * \param second: �ڶ������ʯ����
   * \param result: ������
   * \param level:����ʯ�ȼ�
   * \return ��
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
 * \brief ����
 *
 * ʵ������Ʒ��������
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
 * \brief ��Ʒ�ֽ�
 *
 * ʵ����Ʒ�ֽ⹦��
 *
 */
class Decompose
{
public:  
  /**     
   * \brief ���캯��
   *
   * ��ʼ����ر���
   *
   * param ob : ���ֽ���Ʒ
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
//��ָ,����
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
 * \brief ����
 *
 * ʵ������Ʒ���칦��
 *
 */  
class EquipMaker
{
  public:
    EquipMaker(SceneUser* user);

    /**     
     * \brief ��������
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
          case ItemType_ClothBody ://101�����ʼ��������װ
            additivePercent(ob->data.maxhp,bob->maxsp);
            break;
          case ItemType_FellBody :      //102����Ƥ�׼�ħ�����װ
            additivePercent(ob->data.mdefence,bob->maxsp);  
            break;
          case ItemType_MetalBody:  //103����������׼�������װ
          case ItemType_Shield:   //112���������
            additivePercent(ob->data.pdefence,bob->maxsp);      
            break;
          case ItemType_Blade:        //104����������������
          case ItemType_Sword:        //105����������������
          case ItemType_Axe:             //106����������������
          case ItemType_Hammer:          //107����������������
          case ItemType_Crossbow:          //109���������������
            additivePercent(ob->data.pdamage,bob->maxsp);    
            additivePercent(ob->data.maxpdamage,bob->maxsp);    
            break;
          case ItemType_Staff:        //108��������������
          case ItemType_Stick:          //111�����ٻ���������
          case ItemType_Fan:             //110������Ů����
            additivePercent(ob->data.mdamage,bob->maxsp);    
            additivePercent(ob->data.maxmdamage,bob->maxsp);    
            break;
		  case ItemType_Helm:    //113�����ɫͷ����
		  case ItemType_Caestus:  //114�����ɫ������
		  case ItemType_Cuff:    //115�����ɫ����
		  case ItemType_Shoes:    //116�����ɫЬ�Ӳ�
		  /*sky �������Ƥ���ͷ���֧��*/
		  case ItemType_Helm_Paper: //ͷ��Ƥ
		  case ItemType_Helm_Plate: //ͷ����
		  case ItemType_Cuff_Paper: //����Ƥ
		  case ItemType_Cuff_Plate: //�����
		  case ItemType_Caestus_Paper: //����Ƥ
		  case ItemType_Caestus_Plate: //������
		  case ItemType_Shoes_Paper: //ѥ��Ƥ
		  case ItemType_Shoes_Plate: //ѥ�Ӱ�
		  //sky ������� ���� ������
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
          case ItemType_Necklace:  //117�����ɫ������
          case ItemType_Fing:    //118�����ɫ��ָ��
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
     * \brief ����ȡ���ֵ
     *
     * \param ret: ������
     * \param lv: ����ȡֵ��Χ
     * \return ��
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
     * \brief ��������װ��
     *
     * \param bob: ��Ӧװ��������
     * \param ob: ������Ʒ
     * \param kind: װ������
     * \return ��ǰ���Ƿ���true
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


        COMPUTE_R( pdamage )    // ��С�﹥
          COMPUTE_R( maxpdamage )    // ����﹥
          COMPUTE_R( mdamage )      // ��Сħ��
          COMPUTE_R( maxmdamage )    // ���ħ��
          COMPUTE_R( pdefence )      // ���
          COMPUTE_R( mdefence )      // ħ��

          if (props) {
            int index = randBetween(0,4);
            if (index!=5) {

              //���ڲ��������,ֱ��ȡֵ
              additivePercent(*ob->_p1[index],bob->_p1[index]);
            }else {
              fix_kind(bob,ob);
            }
          }else {
            COMPUTE_L( str )      // ����
              COMPUTE_L( inte )      // ����
              COMPUTE_L( dex )      // ����
              COMPUTE_L( spi )      // ����
              COMPUTE_L( con )        // ����
          }    

        COMPUTE_L( maxhp )    // �������ֵ
          COMPUTE_L( maxmp )    // �����ֵ
          //    COMPUTE_L( maxsp )    // �������ֵ

          COMPUTE_L( mvspeed )    // �ƶ��ٶ�
          COMPUTE_L( hpr )      // ����ֵ�ָ�
          COMPUTE_L( mpr )      // ����ֵ�ָ�
          COMPUTE_L( spr )      // ����ֵ�ָ�
          COMPUTE_L( akspeed )    // �����ٶ�

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
            //ÿ������Ҫ���
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
            //ȡ���ֵ
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
            COMPUTE_L( pdam )    // ������������
              COMPUTE_L( mdam )    // ����ħ��������
              COMPUTE_L( pdef )    // �������������
              COMPUTE_L( mdef )    // ����ħ��������
              COMPUTE_L( atrating )    // ������
              COMPUTE_L( akdodge )    // ������

              COMPUTE_L( poisondef )  // ��������
              COMPUTE_L( lulldef )    // ���������
              COMPUTE_L( reeldef )    // ��ѣ������
              COMPUTE_L( evildef )    // ����ħ����
              COMPUTE_L( bitedef )    // ����������
              COMPUTE_L( chaosdef )  // ����������
              COMPUTE_L( colddef )    // ����������
              COMPUTE_L( petrifydef )    // ��ʯ������
              COMPUTE_L( blinddef )    // ��ʧ������
              COMPUTE_L( stabledef )    // ����������
              COMPUTE_L( slowdef )    // ����������
              COMPUTE_L( luredef )    // ���ջ�����
          }

        if (!ob->data.durpoint) {
          if (additive(ob->data.durpoint,bob->resumedur,property)) {
            ob->data.durpoint -= bob->resumedur.sleightValue;
          }
          if (ob->data.durpoint) ob->data.dursecond = bob->resumedur.sleightValue;

        }

        COMPUTE_L( bang )       //�ػ�
          //ob->data.bang += bob->bang;
          //��ָ��������һ��
          if (ob->base->kind == ItemType_Fing || ob->base->kind == ItemType_Necklace)
          {
            BONUS_SKILL_RING
          }
        BONUS_SKILL
          BONUS_SKILLS

          if (props) 
            ob->data.kind |= 2;//��ɫװ��
          else 
            ob->data.kind |= kind;//��ɫװ��

        return true;
      }

    bool assign_holy(zObject* ob,int holy);

    bool assign_set(zObject* ob);

    /**     
     * \brief ���Լ���
     *
     * \param ret: ������
     * \param lv: ����ȡֵ��Χ
     * \return ��
     */  
    template <typename T>
      void additive(T& ret,const rangeValue &rv)
      {
        ret += randBetween(rv.min,rv.max);
      }

    /**     
     * \brief ��ʥ���Լ���
     *
     * \param ret: ������
     * \param lv: ����ȡֵ��Χ
     * \param property: ��Ʒ��ǰ������Ŀ
     * \return ��
     */  
    template <typename T>
      bool additive(T& ret,const luckRangeValue & lv,int& property)
      {
        int odds = lv.per;
        //    int odds = odds_of_property(lv.per,property);
        //    Zebra::logger->debug("���Բ�������%f,%f",lv.per*1.0,odds*1.0);
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
 * \brief ��Ʒ����
 *
 * ʵ�ָ�����Ʒ���칦��,�ṩһ��ͳһ���
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
   * \brief ���캯��
   *
   */     
  RebuildObject() { }
  
  /**     
   * \brief ��������
   *
   */     
  ~RebuildObject() { }

  
  static RebuildObject* _instance;
};
namespace Op {
  /**
   * \brief ������С�ж�
   *
   * �ж�һ�������Ƿ���ڸ���ֵ
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
     * \brief �жϸ��������Ƿ���ڲ���ֵ
     *
     * \param value: ���жϱ���
     * \param condition: ����ֵ
     * \return ���жϱ������ڲ���ֵ����true,���򷵻�false
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
   * \brief ������С�ж�
   *
   * �ж�һ�������Ƿ�С�ڸ���ֵ
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
     * \brief �жϸ��������Ƿ�С�ڲ���ֵ
     *
     * \param value: ���жϱ���
     * \param condition: ����ֵ
     * \return ���жϱ���С�ڲ���ֵ����true,���򷵻�false
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
   * \brief ������С�ж�
   *
   * �ж�һ�������Ƿ���ڸ���ֵ
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
     * \brief �жϸ��������Ƿ���ڲ���ֵ
     *
     * \param value: ���жϱ���
     * \param condition: ����ֵ
     * \return ���жϱ������ڲ���ֵ����true,���򷵻�false
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
   * \brief ������С�ж�
   *
   * �ж�һ��������ͬ�ڸ���ֵ
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
     * \brief �жϸ��������Ƿ񲻵��ڲ���ֵ
     *
     * \param value: ���жϱ���
     * \param condition: ����ֵ
     * \return ���жϱ��������ڲ���ֵ����true,���򷵻�false
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
 * \brief ������������
 *
 * ���ඨ���˽ű����������Ľӿڡ�
 *
 */  
class Condition
{
public:
  typedef Condition Base;

  virtual bool is_valid (const SceneUser* user,const Vars* vars) const;

  /**     
   * \brief ��������
   *
   */    
  virtual ~Condition() { }
  
protected:
  virtual bool check_args(const SceneUser* user,const Vars* vars) const;
  
  virtual bool check_valid(const SceneUser* user,const Vars* vars) const = 0;
};

/**
 * \brief �ؼ����б���
 *
 * �洢�˹ؼ��ʵ��б�,���ṩ�˶�Ӧ�Ĳ���.
 *
 */
class ScenesParser
{
public:

  /**     
   * \brief ȡ��xml�ڵ�����
   *
   * �����ؼ����б�,ȡ�����Ƕ�Ӧ��ֵ
   *      
   * \param xml: Ŀ��xml�ļ�
   * \param node: Ŀ��ڵ�
   * \return ��ǰ���Ƿ���true
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
   * \brief ����һ���ؼ���
   *
   * �ڹؼ����б�������һ���ؼ���,��Ӧ��ֵΪ"0"
   *      
   * \param key: �ؼ�������
   * \return ��
   */       
  void key(const std::string& key) 
  {
    _kvs[key] = "0";
  }

  /**     
   * \brief ȡֵ
   *
   * �ڹؼ����б�����Ѱ��Ӧ�Ĺؼ���,�����ض�Ӧ��ֵ,û�ҵ�������ֵ
   *      
   * \param key: �ؼ�������
   * \param value: ȡ�õ�ֵ
   * \return ��
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
 * \brief ��Ӵ�����������
 *
 * �����ṩ�˶�����������֧��
 *
 */  
class TeamCondition : public Condition
{
public:
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  TeamCondition(ScenesParser& p)
  { 
    p.value("team",_team);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~TeamCondition() { }
  
  virtual bool is_valid(const SceneUser* user,const Vars* vars) const;
  
protected:  
  
private:
  int _team;
};

/**
 * \brief ��������
 *
 * �����ṩ�˶�������ű����йر����������ж���֧��
 *
 */
template <typename Operation>
class VarCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  VarCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("name",_name);
    p.value("value",_condition);
    p.value("id",_id);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~VarCondition()
  { }

  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж�ĳ�������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief �û���������
 *
 * �����ṩ�˶�������ű����й��û������������ж���֧��
 *
 */
template <typename Operation>
class UserVarCondition : public TeamCondition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  UserVarCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("name",_name);
    p.value("value",_condition);
    p.value("id",_id);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~UserVarCondition()
  { }

  /**     
   * \brief  �û������ж�
   *
   * ������check_valid����,�ж�ĳ���û������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief ȫ�ֱ�������
 *
 * �����ṩ�˶�������ű����й�ȫ�ֱ����������ж���֧��
 *
 */
template <typename Operation>
class GlobalCondition : public Condition
{
public:
  typedef typename Operation::value_type value_type;

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  GlobalCondition(ScenesParser& p)
  { 
    p.value("name",_name);
    p.value("value",_condition);
    p.value("id",_id);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~GlobalCondition()
  { }

  /**     
   * \brief  �������
   *
   * ������check_args����,����������Ҫ�û���Ϣ
   *      
   * \param user: NULL
   * \param vars: ������Ϣ
   * \return ������Ч����true,���򷵻�false
   */   
  bool check_args(const SceneUser* user,const Vars* vars) const
  {
    if (!vars) return false;
    
    return true;
  }

  /**     
   * \brief  ȫ�ֱ����ж�
   *
   * ������check_valid����,�ж�ĳ��ȫ�ֱ����Ƿ�����ű�Ҫ��
   *      
   * \param user: NULL
   * \param vars: ������Ϣ
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief �����Ƿ�ѧϰ����
 *
 * �����ṩ���ж��û��Ƿ�ѧϰ����
 *
 */
template <typename Operation>
class HaveSkillCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  HaveSkillCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~HaveSkillCondition()
  { }
  
  /**     
   * \brief  ���ܸ����ж�
   *
   * ������check_valid����,�ж��û��ļ��������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief �������Ա�
 *
 * �����ṩ���ж��û����Ա�
 *
 */
template <typename Operation>
class CheckSexCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  CheckSexCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~CheckSexCondition()
  { }
  
  /**     
   * \brief  ���ܸ����ж�
   *
   * ������check_valid����,�ж��û��ļ��������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    Operation op;
    int sex = 0;
    switch(user->charbase.type)
    {
      case PROFESSION_1:    //����
      case PROFESSION_3:    //����
      case PROFESSION_5:    //��ʦ
      case PROFESSION_7:    //��ʦ
        sex = 1;
        break;
      case PROFESSION_2:    //��Ů
      case PROFESSION_4:    //����
      case PROFESSION_6:    //��Ů
      case PROFESSION_8:    //��Ů
        sex = 0;
        break;
      case PROFESSION_NONE:  //��ҵ
      default:
        break;
    }
    return op(sex,_condition);
  }

private:
  value_type _condition;    
};

/**
 * \brief �ȼ�����
 *
 * �����ṩ���ж��û��ȼ��Ƿ����,С��,���ڻ򲻵���ĳ��ֵ�Ľӿ�
 *
 */
template <typename Operation>
class LevelCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  LevelCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~LevelCondition()
  { }
  
  /**     
   * \brief  �ȼ��ж�
   *
   * ������check_valid����,�ж��û��ĵȼ��Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief ��������
 *
 * �����ṩ���ж��û�����ID�Ƿ����,С��,���ڻ򲻵���ĳ��ֵ�Ľӿ�
 *
 */
template <typename Operation>
class SeptIDCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  SeptIDCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~SeptIDCondition()
  { }
  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж��û��ļ���ID�Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief �������
 *
 * �����ṩ���ж��û�����ID�Ƿ����,С��,���ڻ򲻵���ĳ��ֵ�Ľӿ�
 *
 */
template <typename Operation>
class UnionIDCondition : public TeamCondition
{
public:  
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  UnionIDCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~UnionIDCondition()
  { }
  
  /**     
   * \brief  ����ж�
   *
   * ������check_valid����,�ж��û��ļ���ID�Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief ��Ǯ����
 *
 * �����ṩ���ж��û������н�Ǯ�����Ƿ����,С��,���ڻ򲻵���ĳ��ֵ�Ľӿ�
 *
 */
template <typename Operation>
class GoldCondition : public TeamCondition
{
public:  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  GoldCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~GoldCondition()
  { }

  /**     
   * \brief  ��Ǯ�ж�
   *
   * ������check_valid����,�ж��û������еĽ�Ǯ�Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    zObject* gold = const_cast<SceneUser *>(user)->packs.getGold();
    
    DWORD number = 0;
    if (gold)  number = gold->data.dwNum;
    Operation op;

    //shouldn't be exist,NB CEHUA
    //if (op.name() == "Great" && number < (DWORD)_condition) {
      //Channel::sendSys(const_cast<SceneUser*>(user),Cmd::INFO_TYPE_FAIL,"��Ǯ����");
    //}
    
    return op(number,_condition);
  }

private:
  int _condition;    
};

/**
 * \brief ��Ʒ����
 *
 * �����ṩ���ж��û������е��ض���Ʒ�����Ƿ����,С��,���ڻ򲻵���ĳ��ֵ�Ľӿ�
 *
 */
class ItemCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ItemCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("id",_id);
    p.value("value",_condition);
    p.value("level",_level);
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~ItemCondition()
  { }

  /**     
   * \brief  ��Ʒ�����ж�
   *
   * ������check_valid����,�ж��û������ռ��ĳ����Ʒ�����Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief ��������
 *
 * �����ṩ���ж��û��Ƿ�����һ�����ҵĽӿ�
 *
 */
class NationCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  NationCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~NationCondition()
  { }
  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж��û��Ƿ�����ĳ������
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return (int)user->charbase.country == _condition;
  }

private:
  value_type _condition;    
};

/**
 * \brief �Ƿ��ڱ�������
 *
 * �����ṩ���ж��û��Ƿ��ڱ����Ľӿ�
 *
 */
class InNationCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  InNationCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~InNationCondition()
  { }
  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж��û��Ƿ�����ĳ������
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return user->charbase.country == user->scene->getCountryID();
  }

private:
  value_type _condition;    
};

/**
 * \brief �������
 *
 * �����ṩ���ж��û��Ƿ�����ĳ�����Ľӿ�
 *
 */
class ConfraternityCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ConfraternityCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~ConfraternityCondition()
  { }
  
  /**     
   * \brief  ����ж�
   *
   * ������check_valid����,�ж��û��Ƿ�����ĳ�����
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return (int)(user->charbase.unionid & 0x1) == _condition;
  }

private:
  value_type _condition;    
};

/**
 * \brief ְҵ����
 *
 * �����ṩ���ж��û��Ƿ�����ĳ��ְҵ�Ľӿ�
 *
 */
class ProfessionCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ProfessionCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_condition);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~ProfessionCondition()
  { }
  
  /**     
   * \brief  ְҵ�ж�
   *
   * ������check_valid����,�ж��û�ְҵ�Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    return user->charbase.type & _condition;
  }

private:
  value_type _condition;    
};

/**
 * \brief �����ռ�����
 *
 * �����ṩ���ж��û������еĿռ��Ƿ�������ĳ����Ʒ�Ľӿ�
 *
 */
class SpaceCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  SpaceCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("size",_size);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~SpaceCondition()
  { }
  
  /**     
   * \brief  �����ռ��ж�
   *
   * ������check_valid����,�ж��û������ռ��Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const
  {
    int free = user->packs.uom.space(user);

    if (free >= _size)   return true;

    //Channel::sendSys(const_cast<SceneUser*>(user),Cmd::INFO_TYPE_FAIL,"�����ռ䲻��");
    return false;
  }

private:
  value_type _size;    
};

/**
 * \brief �Ƿ񳬹�����ʱ��Ҫ��
 *
 * �����ṩ���ж�����ʱ���Ƿ�����Ҫ��Ľӿ�
 *
 */
class TimeoutsCondition : public TeamCondition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  TimeoutsCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("long",_time);  
    p.value("id",_id);  
    p.value("less",_less);
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~TimeoutsCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж�����ʱ���Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief ʱ������
 *
 * �����ṩ���ж�ϵͳʱ���Ƿ�����Ҫ��Ľӿ�
 *
 */
class TimerCondition : public Condition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
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
   * \brief ��������
   *
   */    
  virtual ~TimerCondition()
  { }

  /**     
   * \brief  �������
   *
   * ������check_args����,����������Ҫ�û���������Ϣ
   *      
   * \param user: NULL
   * \param vars: NULL
   * \return ��ǰ���Ƿ���true
   */   
  bool check_args(const SceneUser* user,const Vars* vars) const
  {
    return true;
  }

  /**     
   * \brief  ʱ�������ж�
   *
   * ������check_valid����,�ж�ϵͳʱ���Ƿ�����ű�Ҫ��
   *      
   * \param user: NULL
   * \param vars: NULL
   * \return true��ʾ��������,false��ʾ����������
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
 * \brief �������
 *
 * �����ṩ���ж��û������Ƿ�����Ҫ��Ľӿ�
 *
 */
class TeamedCondition : public Condition
{
public:  
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  TeamedCondition(ScenesParser& p) 
  { 
    p.value("number",_number);  
    p.value("male",_male);
    p.value("female",_female);
  }

  /**     
   * \brief ��������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  IsGuardCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_need);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~IsGuardCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж�����ʱ���Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  FiveTypeCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_need);  
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~FiveTypeCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж�����ʱ���Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  FiveLevelCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~FiveLevelCondition()
  { }

  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж�ĳ�������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  FamilyLevelCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~FamilyLevelCondition() { }

  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж�ĳ�������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ReputeCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~ReputeCondition()
  { }

  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж�ĳ�������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ActionPointCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_level);
  }
  
  /**     
   * \brief ��������
   *
   */    
  virtual ~ActionPointCondition()
  { }

  
  /**     
   * \brief  �����ж�
   *
   * ������check_valid����,�ж�ĳ�������Ƿ�����ű�Ҫ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  HorseCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_id);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~HorseCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж��û��Ƿ�ӵ��������ƥ
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  GradeCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_grade);  
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~GradeCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж��û��Ƿ�ӵ��������ƥ
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  MapCountryCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("name",_name);
    p.value("id",_id);
  }

  /**     
   * \brief ��������
   *
   */    
  virtual ~MapCountryCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж��û��Ƿ�ӵ��������ƥ
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  HonorCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief ��������
   *
   */    
  virtual ~HonorCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж��û��Ƿ�ӵ��������ƥ
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  MaxHonorCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief ��������
   *
   */    
  virtual ~MaxHonorCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж��û��Ƿ�ӵ��������ƥ
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  SelfCountryCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief ��������
   *
   */    
  virtual ~SelfCountryCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж��û��Ƿ�ӵ��������ƥ
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  CountryPowerCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief ��������
   *
   */    
  virtual ~CountryPowerCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж�����ǿ��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  WeekCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief ��������
   *
   */    
  virtual ~WeekCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж������Ƿ���һ������ָ�����Ǽ��� valueֵ����0-6λ��ʾһ�ܵ�7��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  CaptionCondition(ScenesParser& p) : TeamCondition(p)
  { 
    p.value("value",_value);
   }

  /**     
   * \brief ��������
   *
   */    
  virtual ~CaptionCondition()
  { }
  
  /**     
   * \brief  ����ʱ���ж�
   *
   * ������check_valid����,�ж���ɫ�Ƿ�������߳���
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return true��ʾ��������,false��ʾ����������
   */    
  bool check_valid(const SceneUser* user,const Vars* vars) const;
private:
  int _value;
};

namespace Op {
  /**
   * \brief ���ӱ���ֵ
   *
   * ����һ��������ֵ
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
     * ���ӱ�����ֵ         
     *      
     * \param value: ���ı�ı���
     * \param action: Ҫ�޸ĵ�ֵ
     * \return ��
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
   * \brief �趨����ֵ
   *
   * �趨һ��������ֵ
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
     * �趨һ��������ֵ         
     *      
     * \param value: ���ı�ı���
     * \param action: Ҫ�趨��ֵ
     * \return ��
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
   * \brief ���ٱ���ֵ
   *
   * ����һ��������ֵ
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
     * ���ӱ�����ֵ         
     *      
     * \param value: ���ı�ı���
     * \param action: Ҫ���ٵ�ֵ
     * \return ��
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
   * \brief ��һ������ֵ
   *
   * ��һ��������ֵ
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
     * �趨һ��������ֵ         
     *      
     * \param value: ���ı�ı���
     * \param action: Ҫ�趨��ֵ
     * \return ��
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
   * \brief ����ֵ�˷�
   *
   * ����ֵ�˷�
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
     * �趨һ��������ֵ         
     *      
     * \param value: ���ı�ı���
     * \param action: Ҫ�趨��ֵ
     * \return ��
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
   * \brief ����ֵ�˷�
   *
   * ����ֵ�˷�
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
     * �趨һ��������ֵ         
     *      
     * \param value: ���ı�ı���
     * \param action: Ҫ�趨��ֵ
     * \return ��
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
   * \brief ����ֵ�˷�
   *
   * ����ֵ�˷�
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
     * �趨һ��������ֵ         
     *      
     * \param value: ���ı�ı���
     * \param action: Ҫ�趨��ֵ
     * \return ��
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
 * \brief ������������
 *
 * ���ඨ���˽ű����������Ľӿڡ�
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
   * \brief ��������
   *
   */     
  virtual ~Action() { }

protected:
  virtual bool check_args(SceneUser* user,Vars* vars) const;
  
  /**     
   * \brief  ִ�нű�����Ķ���
   *
   * �麯��,�̳�����Ҫ���ش˺����ṩ�Ը��ִ���������֧��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
   */     
  virtual int done(SceneUser* user,Vars* vars) = 0;
};

/**
 * \brief ��Ӵ�����������
 *
 * �����ṩ�˶�����������֧��
 *
 */  
class TeamAction : public Action
{
public:  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  TeamAction(ScenesParser& p)
  {
    p.value("team",_team);  
  }
    
  int do_it (SceneUser* user,Vars* vars);

protected:  
  /**     
   * \brief  ִ�нű�����Ķ���
   *
   * �麯��,�̳�����Ҫ���ش˺����ṩ�Ը��ִ���������֧��
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
   */     
  int done(SceneUser* user,Vars* vars) = 0;

private:
  
  int _team;
};

/**
 * \brief ����
 *
 * �����ṩ�˶�������ű����йر������޸ĵ�֧��
 *
 */  
template <typename Operation>
class VarAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  VarAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name",_name);
    p.value("value",_action);
    p.value("id",_id);
    p.value("tmp",_tmp);    
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~VarAction()
  { }

  /**     
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
 * \brief �����䶯��
 *
 * �����ṩ�˶�������ű��б���֮��Ĳ�����֧��
 *
 */  
template <typename Operation>
class VarsAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
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
   * \brief ��������
   *
   */     
  virtual ~VarsAction()
  { }

  /**     
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
 * \brief �û�����
 *
 * �����ṩ�˶�������ű�����Ҫ�������û����ϱ�����֧��
 *
 */  
template <typename Operation>
class UserVarAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  UserVarAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name",_name);
    p.value("value",_action);
    p.value("id",_id);
    p.value("tmp",_tmp);    
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~UserVarAction()
  { }

  /**     
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
 * \brief ϵͳ��Ϣ
 *
 * �����ṩ�˶��û��ṩϵͳ������Ϣ��֧��.
 *
 */  
class NotifyAction : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  NotifyAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~NotifyAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _info;  
};

/**
 * \brief ��־��Ϣ
 *
 * �����ṩ�˶���־��Ϣ��֧��.
 *
 */  
class LogAction : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  LogAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
  }

  /**     
   * \brief ��������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  BulletinAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
    p.value("kind",_kind);
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~BulletinAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _info;  
  int _kind; 
};

/**
 * \brief ϵͳ��Ϣ
 *
 * �����ṩ�˶��û��ṩϵͳ������Ϣ��֧��.
 *
 */  
class Notify1Action : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  Notify1Action(ScenesParser& p) : TeamAction(p)
  { 
    p.value("content",_info);  
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~Notify1Action() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _info;  
};

/**
 * \brief �˵�
 *
 * �����ṩ�˶Կͻ��˶�̬�˵���֧��.
 *
 */  
class MenuAction : public Action
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param menu: �˵�����
   */       
  MenuAction(const std::string& menu) : _menu(menu)
  { }
  
  virtual ~MenuAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _menu;  
};

/**
 * \brief �˵�
 *
 * �����ṩ�˶Կͻ��˶�̬�˵���֧��.
 *
 */  
class SubMenuAction : public Action
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param menu: �˵�����
   */       
  SubMenuAction(const std::string& menu) : _menu(menu)
  { }
  
  virtual ~SubMenuAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _menu;  
};

/**
 * \brief �˵�
 *
 * �����ṩ�˶Կͻ��˶�̬�˵���֧��.
 *
 */  
class MainMenuAction : public Action
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param menu: �˵�����
   */       
  MainMenuAction(const std::string& menu) : _menu(menu)
  { }
  
  virtual ~MainMenuAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  std::string _menu;  
};


/**
 * \brief ����ˢ��
 *
 * ����ʵ���˶���������ļ�ʱˢ��
 *
 */  
class RefreshAction : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  RefreshAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("name",_name);  
    p.value("id",_id);
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~RefreshAction() { }  
  
  int done (SceneUser* user,Vars* vars);
private:
  std::string _name;
  int _id;
};

/**
 * \brief ����
 *
 * �����ṩ�������ض��û�����Ľӿ�
 *
 */
class ExpAction : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ExpAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_exp);  
    p.value("name",_name);
    p.value("id",_id);
    
  }
  
  /**     
   * \brief ��������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  Exp1Action(ScenesParser& p) : ExpAction(p)
  { 
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~Exp1Action() { }  
  
  int done (SceneUser* user,Vars* vars);
};


/**
 * \brief ����
 *
 * �����ṩ���޸��û��������������Ľӿ�
 *
 */  
template <typename Operation>
class GoldAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  GoldAction(ScenesParser& p)  : TeamAction(p)
  { 
    p.value("value",_value);
    
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~GoldAction() { }  

  /**     
   * \brief  �������
   *
   * ������done����,�����û�����Я��������
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
   */  
  int done (SceneUser* user,Vars* vars)
  {
    Operation op;

    
    if (op.name() == "Add") {
      user->packs.addMoney(_value,"�������");
      return Action::SUCCESS;
    }

    if (op.name() == "Sub") {
      if (!user->packs.removeMoney(_value,"�������")) {
        Zebra::logger->fatal("�������Ӽ������: �û�(%s),����(%d)",user->name,vars->quest_id());
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  EnterSeptGuardAction()
  { 
  }
  
  /**     
   * \brief ��������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  FinishSeptGuardAction()
  { 
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~FinishSeptGuardAction() { }  
  
  int done (SceneUser* user,Vars* vars);
};

/**
 * \brief ����
 *
 * �����ṩ�������ض��û����۵Ľӿ�,��δʵ��
 *
 */
template <typename Operation>
class ScoreAction : public TeamAction
{
public:

  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ScoreAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_value);  
  }
  
  /**     
   * \brief ��������
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
 * \brief ����
 *
 * �����ṩ�������ض��û����ܵȼ��Ľӿ�.
 *
 */
class SkillAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  SkillAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~SkillAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  value_type _id;
};

/**
 * \brief ��ֹ
 *
 * �����ṩ�˽�ֹ�û�ִ��ĳ����ķ���,��ǰ֧�ֵ�������ʹ����Ʒ,ʰȡ��Ʒ��������.
 *
 */
class DisableAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  DisableAction(ScenesParser& p)  : TeamAction(p)
  { 
  
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~DisableAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:

};

/**
 * \brief ��ͼ��ת
 *
 * �����ṩ���û��ڵ�ͼ����ת�Ľӿ�
 *
 */
class GotoAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
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
   * \brief ��������
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
 * \brief ����
 *
 * �����ṩ��ʹ�û�����Ľӿ�
 *
 */
class RideDownAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  RideDownAction(ScenesParser& p) : TeamAction(p)
  { }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~RideDownAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:

};

/**
 * \brief �����Ʒ
 *
 * �����ṩ�˸��û����ĳ����Ʒ�Ľӿ�
 *
 */
class AddItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  AddItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
  }

  /**     
   * \brief ��������
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
 * \brief ��Ӱ���Ʒ
 *
 * �����ṩ�˸��û����ĳ����Ʒ�Ľӿ�
 *
 */
class AddBindItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  AddBindItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
  }

  /**     
   * \brief ��������
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
 * \brief �����ɫ����Ʒ
 *
 * �����ṩ�˸��û����ĳ����Ʒ�Ľӿ�
 *
 */
class AddGreenBindItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  AddGreenBindItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
  }

  /**     
   * \brief ��������
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
 * \brief ɾ����Ʒ
 *
 * �����ṩ��ɾ���û�����ĳ����Ʒ�Ľӿ�
 *
 */
class RemoveItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  RemoveItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("value",_value);      
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~RemoveItemAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _value;
};

/**
 * \brief ������Ʒ
 *
 * �����ṩ�˶����û�����ĳ����Ʒ�Ľӿ�
 *
 */
class DropItemAction : public TeamAction
{
public:
  typedef int value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  DropItemAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("id",_id);
    p.value("level",_level);
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~DropItemAction() { }
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _level;
};

/**
 * \brief δʵ��
 *
 * 
 *
 */
class DropAction : public Action
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  DropAction(ScenesParser& p)
  {
    p.value("id",_id);
    p.value("value",_value);  
    p.value("odds",_odds);
    p.value("guard",_guard);
  }
  
  /**     
   * \brief ��������
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
 * \brief ��ʱ��
 *
 * �����ṩ�˶���Ҫʱ�����Ƶ������֧��
 *
 */
class TimeoutsAction : public Action
{
public:

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  TimeoutsAction(ScenesParser& p)
  {
    //p.value("value",_timeout);
    p.value("id",_id);
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~TimeoutsAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  //int _timeout;
  int _id;
};

/**
 * \brief ����״̬
 *
 * �����ṩ�������û�����ĳ��״̬�Ľӿ�
 *
 */
class SetStateAction : public TeamAction
{
public:

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  SetStateAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_state);
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~SetStateAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _state;
};

/**
 * \brief ���״̬
 *
 * �����ṩ������û�����ĳ��״̬�Ľӿ�
 *
 */
class ClearStateAction : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ClearStateAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_state);
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~ClearStateAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _state;
};

/**
 * \brief ��ƥ
 *
 * �����ṩ�˸����û���ƥ�Ľӿ�
 *
 */
class HorseAction : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  HorseAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_id);
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~HorseAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
};


/**
 * \brief ȫ�ֱ���
 *
 * �����ṩ�˶�������ű�����Ҫ�������û��ɼ�������֧��
 *
 */  
template <typename Operation>
class GlobalAction : public Action
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  GlobalAction(ScenesParser& p) : _id(0)
  { 
    p.value("name",_name);
    p.value("value",_action);
    p.value("id",_id);
    p.value("tmp",_tmp);    
  }

  /**     
   * \brief ��������
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
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
    Zebra::logger->debug("��ʼ��_level%d",_level);
  }

  virtual ~FamilyLevelAction()
  { }

  int done (SceneUser* user,Vars* vars)
  {
    Cmd::Session::t_OpLevel_SceneSession uplevel;
    uplevel.dwSeptID=user->charbase.septid;
    uplevel.dwLevel=_level;
    Zebra::logger->debug("����_level%d",_level);
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
 * �����ṩ�˶Գ�����NPC���ʵ�֧��
 *
 */
class NpcAction : public Action
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  NpcAction(ScenesParser& p)
  { 
    p.value("id",_id);
    std::string map;
    p.value("map",map);
    _s = SceneManager::getInstance().getSceneByName(map.c_str());
    if (!_s) {
      Zebra::logger->warn("NpcActionʱ����ĳ���(%s)������",map.c_str());
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
   * \brief ��������
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
 * \brief ����NPC
 *
 * �����ṩ���ڵ�ͼ������һ��NPC�Ľӿ�
 *
 */
class AddNpcAction : public NpcAction
{
public:

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  AddNpcAction(ScenesParser& p) : NpcAction(p)
  { }
  
  /**     
   * \brief ��������
   *
   */     
  ~AddNpcAction()
  { }
  
  int done(SceneUser* user,Vars* vars);  
  
};

/**
 * \brief �����ڳ�
 *
 * �����ṩ�˶Ի��������֧��
 *
 */
class AddGuardAction : public NpcAction
{
public:

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
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
      Zebra::logger->info("AddGuardAction::AddGuardAction(): _destƥ��ʧ�� %s",pos.c_str());
    
  }
  
  /**     
   * \brief ��������
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
 * \brief ɾ��NPC
 *
 * �����ṩ���ڵ�ͼ��ɾ��һ��NPC�Ľӿ�
 *
 */
class RemoveNpcAction : public NpcAction
{
public:

  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  RemoveNpcAction(ScenesParser& p) : NpcAction(p),_remove(_s,_id,_ltpos,_rbpos)
  { }
  
  /**     
   * \brief ��������
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
 * \brief ����
 *
 * �����ṩ�˶�������ű����й����е������޸ĵ�֧��
 *
 */  
template <typename Operation>
class FiveLevelAction : public TeamAction
{
public:
  typedef typename Operation::value_type value_type;
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  FiveLevelAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_level);
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~FiveLevelAction()
  { }

  /**     
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
 * \brief ������������
 *
 * �����ṩ�������������͵Ľӿ�
 *
 */
class FiveTypeAction : public TeamAction
{
public:
  
  /**     
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  FiveTypeAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("value",_type);
  }
  
  /**     
   * \brief ��������
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  HonorAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_honor);
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~HonorAction()
  { }

  /**     
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  MaxHonorAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_honor);
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~MaxHonorAction()
  { }

  /**     
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  ActionPointAction(ScenesParser& p) : TeamAction(p)
  { 
    p.value("value",_ap);
  }

  /**     
   * \brief ��������
   *
   */     
  virtual ~ActionPointAction()
  { }

  /**     
   * \brief  ִ�б����޸�
   *
   * ������done����,ʵ�ֶ���ر������޸�
   *      
   * \param user: �����������û�
   * \param vars: �û������ĸ�������ر���
   * \return SUCCESS��ʾ�ɹ�,FAILED��ʾʧ��,DISABLE��ʾ����ĳ���
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
   * \brief  ���캯��
   *
   * ��ʼ����ر���
   *      
   * \param p: �ؼ����б�
   */       
  UseSkillAction(ScenesParser& p) : TeamAction(p)
  {
    p.value("id",_id);
    p.value("level",_level);
  }
  
  /**     
   * \brief ��������
   *
   */     
  virtual ~UseSkillAction() { }  
  
  int done (SceneUser* user,Vars* vars);
  
private:
  int _id;
  int _level;
};

/**
 * \brief ��Ʒ����
 *
 * ��װ�˹���ģʽ
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
   * ʵ���˵���ģʽ
   *      
   * \return �����Ψһʵ��
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
   * \brief ȡ�ô�����
   *
   * Ѱ���ض���Ʒ��־�Ĵ�����
   *      
   * \param id: ��Ʒ��ʶ
   * \return �ҵĲ�Ʒ������,û�ҵ�����NULL
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
   * \brief ע�᷽��
   *
   * ע��ĳ���Ʒ�Ĵ�����
   *      
   * \param id: ��Ʒ��ʶ
   * \param creator: ��Ʒ������
   * \return ��
   */       
  void register_creator(const I& id,C* creator)  
  {
    _list[id] = creator;
  }
private:
  /**     
   * \brief ���캯��
   *
   */     
  ProductFactory()
  { }
  
  /**     
   * \brief ��������
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
   * \brief ��Ʒ��������
   *
   *�����˴�����Ʒ�Ľӿ�
   *
   */
  template <typename A>
  class Maker
  {
  public:  
    typedef typename A::Base Base;

    /**     
     * \brief  ��Ʒ�����ӿ�
     *
     * �����˲�Ʒ���� �Ľӿ�,�̳�����Ҫʵ�ִ˺����ṩ�����Ʒ�Ĵ�������.
     *      
     * \param xml: �ű��ļ�
     * \param node: �ڵ�����
     * \return �����Ĳ�Ʒ
     */       
    virtual Base* make ( zXMLParser& xml,xmlNodePtr& node) = 0;

    /**     
     * \brief ��������
     *
     */     
    virtual ~Maker() { }
  };
  
  /**
   * \brief �ڵ����ݴ�����
   *
   *��װ����Ҫ���ʽڵ����ݵĽű�����������������Ĵ���
   *
   */  
  template <typename A>
  class Content : public Maker<typename A::Base>
  {
  public:  
    typedef typename A::Base Base;
    
    /**     
     * \brief ��������
     *
     */     
    virtual ~Content() { }
    
    /**     
     * \brief  ��Ʒ�����ӿ�
     *
     *ʵ�ֶԷ��ʽڵ����ݵĽű�����������������Ĵ���
     *      
     * \param xml: �ű��ļ�
     * \param node: �ڵ�����
     * \return �����Ĳ�Ʒ
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
   * \brief �����Դ�����
   *
   *��װ�˲���Ҫ���ʽڵ����ԵĽű�����������������Ĵ���
   *
   */  
  template <typename A>
  class Void : public Maker<Action>
  {
  public:  
    typedef Maker<Action>::Base Base;
    
    /**     
     * \brief  ���캯��
     *
     * ��ʼ����ر���
     *      
     */       
    Void()
    { }
    
    /**     
     * \brief ��������
     *
     */     
    virtual ~Void() { }
    
    /**     
     * \brief  ��Ʒ�����ӿ�
     *
     *      
     * \param xml: �ű��ļ�
     * \param node: �ڵ�����
     * \return �����Ĳ�Ʒ
     */       
    Base* make (zXMLParser& xml,xmlNodePtr& node)
    {
      return  new A();
    }
  };

  /**
   * \brief �ڵ����Դ�����
   *
   *��װ����Ҫ���ʽڵ����ԵĽű�����������������Ĵ���
   *
   */  
  template <typename A>
  class Prop : public Maker<typename A::Base>
  {
  public:  
    typedef typename A::Base Base;
    
    /**     
     * \brief  ���캯��
     *
     * ��ʼ����ر���
     *      
     * \param p: �ؼ����б�
     */       
    Prop(ScenesParser& p) : _p(p)
    { }
    
    /**     
     * \brief ��������
     *
     */     
    virtual ~Prop() { }
    
    /**     
     * \brief  ��Ʒ�����ӿ�
     *
     *ʵ�ֶԷ��ʽڵ����ԵĽű�����������������Ĵ���
     *      
     * \param xml: �ű��ļ�
     * \param node: �ڵ�����
     * \return �����Ĳ�Ʒ
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
   * \brief ��������������
   *
   *��װ����Ҫ���������͸��µĽű�����������Ĵ���
   *
   */  
  template < template <typename> class A,typename T = int >
  class Update : public Maker<Action>
  {
  public:  
    typedef Maker<Action>::Base Base;
    
    /**     
     * \brief  ���캯��
     *
     * ��ʼ����ر���
     *      
     * \param p: �ؼ����б�
     */       
    Update(ScenesParser& p) : _p(p)
    {
      
    }
    
    /**     
     * \brief ��������
     *
     */     
    virtual ~Update() { }
    
    /**     
     * \brief  ��Ʒ�����ӿ�
     *
     *ʵ�ֶ԰��������͸��µĽű�����������Ĵ���
     *      
     * \param xml: �ű��ļ�
     * \param node: �ڵ�����
     * \return �����Ĳ�Ʒ
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
   * \brief ��������������
   *
   *��װ����Ҫ���������ͱȽϵĽű�����������Ĵ���
   *
   */  
  template <template <typename> class A,typename T = int >
  class Compare : public Maker<Condition>
  {
  public:  
    typedef Maker<Condition>::Base Base;
    
    /**     
     * \brief  ���캯��
     *
     * ��ʼ����ر���
     *      
     * \param p: �ؼ����б�
     */       
    Compare(ScenesParser& p) : _p(p)
    { 
      
    }
    
    /**     
     * \brief ��������
     *
     */     
    virtual ~Compare() { }
    
    /**     
     * \brief  ��Ʒ�����ӿ�
     *
     *ʵ�ֶ԰��������ͱȽϵĽű�����������Ĵ���
     *      
     * \param xml: �ű��ļ�
     * \param node: �ڵ�����
     * \return �����Ĳ�Ʒ
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
 * \brief ��Ϣϵͳ
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
     * \brief �õ�Ψһʵ��
     *
     */
    static MessageSystem &getInstance()
    {
      if (NULL == instance)
        instance = new MessageSystem();

      return *instance;
    }

    /**
     * \brief ж��Ψһʵ��
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
     * \brief ��Ϣ���ݽṹ
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
    Zebra::logger->error("��������ű���ǰ�û���ָ��");
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
    Zebra::logger->error("��������ű���ǰ�û���ָ��");
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
    Zebra::logger->error("��������ű���ǰ�û���ָ��");
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



/** brief �ṩ�Խű���ȫ�ֱ�����֧��
  
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