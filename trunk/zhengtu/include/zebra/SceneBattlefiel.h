//////////////////////////////////////////////////////////////////////////
/////////	sky ս������ඨ��
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <zebra/ScenesServer.h>

#define MAX_GAMEPOINT	1000000
#define MAX_CAMPREG		50000

//sky ʤ������ö��
enum eVictoryType
{
	BATTLEFIELF_WINNT_NULL			= 0,	//NULL
	BATTLEFIELF_WINNT_KILLUSERNUM	= 1,	//ɱ���Է���Ӫ��Ա�ض�����ʤ��
	BATTLEFIELF_WINNT_KILLALLUSER	= 2,	//ɱ��ȫ���Է���Ӫ��Աʤ��
	BATTLEFIELF_WINNT_WRECKBASE		= 3,	//�ݻٶԷ���Ӫ�Ļ���
	BATTLEFIELF_WINNT_WATCH_NPCATT	= 4,	//������ض�NPC����,����û���ݻپ���ʤ��
	BATTLEFIELF_WINNT_REGNAMMAX		= 5,	//��Դ���ȴ�����Ӫʤ��
	//sky �Գ���ʱ��Ϊǰ���ʤ������
	BATTLEFIELF_WINNT_TIME_GETGO	= 16,	//��ֵ�ʱ�����
	BATTLEFIELF_WINNT_TIME_KILLUSERMUCH	= 17,	//ʱ�����ʱ��ɱ���û�������Ӫʤ��
	BATTLEFIELF_WINNT_TIME_KILLNPCMUCH	= 18,	//ʱ�����ʱ��ɱ��NPC������Ӫʤ��
};

//sky ��������ö��
enum eRewardType
{
	REWARD_NULL		= 0,	//sky ��
	REWARD_MONEY	= 1,	//sky ��Ǯ
	REWARD_ITEM		= 2,	//sky ����
	REWARD_HONOR	= 3,	//sky ����
};

enum enReliveType
{
	Compulsory_Resurrection = 0,
	NpcSkill_Resurrection	= 1,
	Notallowed_Resurrection	= 2,
};

//sky �����ͷ��ṹ
struct stBattleReward
{
	stBattleReward()
	{
		RewardType = REWARD_NULL;
		RewardData1 = 0;
		RewardData2 = 0;
	}

	eRewardType	RewardType;	//����
	DWORD	RewardData1; //����1
	DWORD	RewardData2; //����2
};

//sky ��Ӫ��Ա���ݽṹ
struct  stCampMember
{
	stCampMember()
	{
		userid = 0;
		GamePoint = 0;
		DeathTime = -1;
	}
	DWORD userid;		//sky ��ԱΨһID
	int DeathTime;	//sky ����ʱ��
	DWORD GamePoint;	//sky ս������(ս��ר�û���)
};

//sky ��Ӫ���ݽṹ
struct BCampData
{
	BCampData()
	{
		CampID = 0;
		Teamnum = 0;
		CampMembe.clear();
		KillUserNum = 0;
		KillNpcNum = 0;
		GetReg = 0;
		SoulUserNum = 0;
		CampPos.x = 0;
		CampPos.y = 0;
	}

	DWORD CampID;
	//sky ��Ӫ��Ա��
	WORD Teamnum;
	//sky ��Ӫ��Ա����
	std::vector<stCampMember> CampMembe;
	//sky ɱ���Է���Ӫ��Ա����
	DWORD KillUserNum;
	//sky ɱ���Է���ӪNPC����
	DWORD KillNpcNum;
	//sky �Լ���Ӫ�д������״̬�ĳ�Ա��
	WORD SoulUserNum;
	//sky ��ȡ��Դ��
	DWORD GetReg;
	//sky ��������ʼ���͵�
	zPos CampPos;
};

/**
* \brief sky ս������
*
*/
class GangScene:public Scene
{
private:
	//==== sky ս�������ⲿ�������� ====
	//sky ��Ӫ����
	WORD CampNum;
	//sky ��Ӫ����Ա��
	WORD CampUserNum;
	//sky ��Ӫ��ϵ��Ӫ��ϵ(0:�Կ�(����ɱ���Է���Ӫ) 1:�Կ�(������ɱ���Է���Ӫ) 2:����)
	WORD CampRel;
	//sky ʤ����������
	eVictoryType VictoryType;
	//sky ʤ�������Ĳ�����ֵ
	DWORD VictoryData;
	//sky ʤ���߽���
	std::vector<stBattleReward> Victory;
	//sky ʧ���߽���
	std::vector<stBattleReward> failure;
	//sky ʧ���ߴ���
	std::vector<stBattleReward> penalty;
	//sky ����������׼(��֤��������ʲô�����ֻ�ᷢ��һ��)
	bool bVictory;
	bool bfailure;
	bool bpenalty;
	
	//sky ��ɱ�ж���Ӫ�û�����ս������
	DWORD KillPoint;
	//sky ��������
	BYTE reliveType;
	//sku �����ʱ
	WORD reliveTime;
	//sky ɱ���Է���Ӫ��Ա��������
	DWORD killPoint;

	//**** sky ��ѡ���� ****
	//sky ս��������ʱ��(����)
	DWORD GameTime;
	//sky ʱ�䵽����жϵ�Ĭ��ʤ������
	eVictoryType defaultvictory;
	//**** sky ��ѡend ****
	//==== sky ս�������ⲿ�������� end ====
private:
	//==== sky ս�������߼��ñ��� ====
	//sky ս����ʼ��־(1:��ʼ 0:δ��ʼ 2:����ʱ�׶� 3:����)
	BYTE GameStart;
	//sky ս���ȴ�����ʱ(�ӽ����һ����ҿ�ʼ��ʱ:��)
	WORD StartTime;
	//sky ս���رյ���ʱ
	WORD OverTime;
	//sky ս���Ѿ�������ʱ��(����)
	DWORD passTime;
	//sky ��Ӫ�û�����<��ӪID ��Ӫ����>
	std::map<DWORD ,BCampData* > camp;
	//sky ʤ����ӪID
	DWORD WinnerCamp;
	//sky ���Ӷ�ʱ��
	Timer _one_min;
	//==== sky ս�������߼��ñ��� end ====
public:
	//sky ��ӪID����
	std::map<BYTE, DWORD> CampThisID;
	//sky ��ӪID���亯��
	DWORD ReCampThisID(BYTE index);
	//sky ��Ӫ��ʼ���������
	std::vector<zPos> CampPos;
	//==== sky �߼����� ====
	//sky ��ʼ����������
	void InitData();
	//sky ������Ӫ��Ա���ŵ���ʼpos��
	bool AddUserToScene(DWORD UserID, const zPos & initPos);
	//sky ��ȡ�ض���Ӫ��ɱ���ж������
	DWORD GetCampKillUserNum(DWORD index);
	//sky ��ȡ�ض���Ӫɱ���ж�NPC������
	DWORD GetCampKillNpcNum(DWORD index);
	//sky ��ȡ�ض���Ӫ��ȡ��Դ������
	DWORD GetCampRegNum(DWORD index);
	//sky ������Ӫ����Դ
	bool AddCampReg(DWORD campID, int RegNum);
	//sky ������Ӫ����Դ
	bool DesCampReg(DWORD campID, int RegNum);
	//sky ��ȡս���Ѿ�������ʱ��
	DWORD GetPassTime();
	//sky ʤ���ж�
	bool GetCampVictory();
	//sky ����ʤ����Ӫ����
	bool largessWinVictory();
	//sky ����ʧ����Ӫ����
	bool largessFailureVictory();
	//sky ʧ����Ӫ�ͷ�
	bool givepenalty();
	//sky ���ӳ�Աս����
	bool AddUserCamePoint(DWORD userid, DWORD point);
	//sky ���ٳ�Աս����
	bool DesUserCampPoint(DWORD userid,	DWORD point);
	//sky �����촦���û�����
	bool reliveRun();
	//sky ���⴦��һ���û��������¼�
	bool UserDeathRun(DWORD DeathID, DWORD MurderID);
	//sky ����ս���ǿ�ʼ״̬
	BYTE GetGameStart() { return GameStart; }

	//**** sky ʤ�������� ****
	//sky �Ƿ�ɱ��������Ӫ��Ա�ض�����ʤ�����(����������ʱ��ͼ�����ֵ���Ӫ��ɱ����)
	DWORD IfKillUserNum(DWORD CampID);
	//sky ɱ��ȫ���Է���Ӫ��Աʤ�����(����:���Ը����ս����ʤ���������޷�����)
	DWORD IfKillAllUser();
	//sky �ݻٶԷ���Ӫ�Ļ��ؼ��
	bool IfWreckBase(DWORD npcid);
	//sky ������ض�NPC����,����û���ݻپ���ʤ�����
	DWORD IfWatchNpcAtt();
	//sky ��Դ���ȴ�����Ӫʤ�����
	DWORD IfRegNumMax(DWORD CampID);	
	//sky �Գ���ʱ��Ϊǰ���ʤ������
	//sky ��ֵ�ʱ��������(��ֵ�����Ӫȫ��ʤ��)
	bool IfTimeGetGo();
	//sky ʱ�����ʱ��ɱ���û�������Ӫʤ��
	DWORD IfTimeKillUserMuch();
	//sky ʱ�����ʱ��ɱ��NPC������Ӫʤ��
	DWORD IfTimeKillNpcMuch();
	//sky ս��ʤ��������
	bool CampVictoryRun();
	//sky ����ս������
	bool OverBattGame();
	//sky ս��������ʱ������
	virtual void GangSceneTime(const zRTime& ctv);
	//**** sky ʤ�������� end ****
	//==== sky �߼����� end ====
public:
	//sky �������ٵ�ͼ��ʱ���ͷ�ռ�õ�ΨһID
	DWORD GangmapID;
public:
	GangScene();
	~GangScene();
	bool save();
public:
	//sky ��̬������ʼ��
	bool GangSceneInit(DWORD countryid, DWORD baseid, DWORD mapid);
	virtual bool IsGangScene() { return true; }
}; 


/**
* \brief ����������NPC���ʵ�� [sky]
*/
class SceneArchitecture : public SceneNpc
{
public:
	std::vector<SummonNpcData> SummonNpc;	//sky �ٻ���NPC��Ϣ(ID, ����)
	DWORD SummonTime;					//sky �ٻ����(����)
	WORD LevelUpTime;					//sky �ٻ��ı�����ʱ��(����)
	BYTE SummonLevel;							//sky �ٻ���ʿ����ǰ�ȼ�
	bool bOutcome;						//sky �����Ƿ�Ӱ��ʤ��
	DWORD SummonAI;						//sky ˢ������ʿ��ʹ�õ��ƶ�AI_ID
public:
	SceneArchitecture(Scene* scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype=SceneEntry_NPC,zNpcB *abase=NULL);
	~SceneArchitecture() {}

	virtual bool IsArchitecture() {return true;}

	//sky �������AI��ȡ��ʽ���Լ�����ʽ
	virtual bool laod_ai();
	virtual bool GetNpcAi(); //sky ��ȡAI

	//sky ������NPC��ʱ������
	virtual void ArchitecTimer(const zRTime& ctv);

	//sky ��ʼ������
	void Init();

	//[sky] �ٻ�ʿ��(ÿ��ʿ������һ�������ĸ��岻ͬ���ٻ��޻��߳���)
	int summonSoldiers(DWORD id, Cmd::petType type, WORD num, DWORD sid=0, const char * name="", DWORD anpcid = 0,zPos pos=zPos(0,0),BYTE dir=4);
private:
	//sky һ���Ӷ�ʱ��
	Timer _one_min;

	//sky ˢ������ʱ
	DWORD SummonCountdown;
	//sky ��������ʱ
	DWORD LevelUpCountdown;
};