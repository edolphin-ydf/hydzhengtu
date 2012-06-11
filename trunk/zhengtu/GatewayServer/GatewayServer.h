/**
* \brief zebra��ĿGateway������,�����û�ָ����ת�������ܽ��ܵ�
*/
#include <zebra/srvEngine.h>
#include <set>

/**
* \brief ��������������
*
*/
class GatewayTask : public zTCPTask
{
public:
	DWORD old;
	GatewayTask(zTCPTaskPool *pool,const SOCKET sock,const struct sockaddr_in *addr = NULL);
	~GatewayTask();

	int verifyConn();
	int waitSync();
	int recycleConn();
	void Terminate(const TerminateMethod method = terminate_passive);
	void addToContainer();
	void removeFromContainer();
	bool uniqueAdd();
	bool uniqueRemove();
	bool msgParse(const Cmd::t_NullCmd *ptNull,const DWORD nCmdLen);
	bool checkTime(const zRTime &ct);

	/**
	* \brief �õ�����ҵ��ʺ�id
	*
	* \return �õ���id
	*/
	DWORD getACCID() const
	{
		return accid;
	}

	/**
	* \brief ���ø��ʺ��Ƿ���֤ͨ��
	*
	*
	* \param ok �Ƿ�ͨ��
	*/
	void accountVerifyOK(const bool ok)
	{
		if (ok)
			accountVerified = ACCOUNTVERIFY_SUCCESS;
		else
			accountVerified = ACCOUNTVERIFY_FAILURE;
	}

	/**
	* \brief �����Ƿ�δvip�û�
	*
	*/
	void setVip(bool vip)
	{
		vip_user = vip;
	}

	/**
	* \brief �Ƿ���vip�û�
	*
	*/
	bool isVip()
	{
		return vip_user;
	}

private:
	///��������
	char numPassword[MAX_NUMPASSWORD];
	/// ��������DWORD��
	DWORD numPwd;
	///vip�û�
	bool vip_user;
	///ʱ��У�Զ�ʱ��
	Timer _retset_gametime;
	///������ʱ�ȵ�(����)
	DWORD recycle_wait;
	///У��ͻ���ʱ��ļ��
	static const DWORD sampleInterval = 20000;
	static const DWORD sampleInterval_sec = sampleInterval/1000;
	static const DWORD sampleInterval_error_sec = sampleInterval/1000;
	static const DWORD sampleInterval_error_msecs = sampleInterval;

	///������Ϣת���ļ��
	static const DWORD chatInterval = 1000;
	///�´������ʱ��
	zRTime nextChatTime;
	///�´ι��������ʱ��
	zRTime nextCountryChatTime;

	///�ͻ�����sampleIntervalʱ���ڷ��ͳ���maxSamplePPS�����ݰ����ж�Ϊʹ�������
	static const DWORD maxSamplePPS = 145;
	///�ϴ����v_samplePackets��ʱ��
	DWORD v_lastSampleTime;
	///ͳ�����ݰ�����
	DWORD v_samplePackets;

	///��task��ʼ����ʱ��
	zRTime initTime;
	///�ϴμ��ͻ����Ƿ��Ѿ�У����ʱ���ʱ��
	zRTime lastCheckTime;
	///�Ƿ��Ѿ�У����ʱ��
	volatile bool haveCheckTime;

	friend class GateUser;
	///ʹ�ø����ӵ����
	GateUser *pUser;

	///��Ϣ��鹤��
	//CheckerTable checker;
	/**
	* \brief �˺ű��
	*
	*/
	DWORD accid;

	/**     
	** \brief ��Ϸʱ��
	**
	**/
	QWORD qwGameTime;
	zRTime GameTimeSyn;
	QWORD dwTimestampServer;

	/**
	* \brief �û��ʺ�
	*
	*
	*/
	char account[MAX_ACCNAMESIZE+1];

	///��¼ʱ�������ʱid
	DWORD loginTempID;
	///�Ƿ���֤�˰汾
	bool versionVerified;
	///�ʺ���֤��״̬
	enum
	{
		ACCOUNTVERIFY_NONE,
		ACCOUNTVERIFY_WAITING,
		ACCOUNTVERIFY_SUCCESS,
		ACCOUNTVERIFY_FAILURE
	}accountVerified;

	bool verifyVersion(const Cmd::stNullUserCmd *pNullCmd);
	bool verifyACCID(const Cmd::stNullUserCmd *pNullCmd);
	bool forwardScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardSceneBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardBillScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardSession(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardMini(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool msgParse_Select(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool msgParse_Time(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool checkUserCmd(const Cmd::stNullUserCmd *pCmd,const zRTime &ct);

	bool checkNewName(char *);
};

/**
* \brief �����������ӹ�����
*
*/
class GatewayTaskManager
{

public:

	/**
	* \brief �ص�����
	*
	*/
	typedef zEntryCallback<GatewayTask> GatewayTaskCallback;

	/**
	* \brief ��������
	*
	*/
	~GatewayTaskManager();

	/**
	* \brief ��ȡ�����ӹ�����Ψһʵ��
	*
	* \return �����ӹ�����Ψһʵ��
	*/
	static GatewayTaskManager &getInstance()
	{
		if (NULL == instance)
			instance = new GatewayTaskManager();

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

	bool uniqueAdd(GatewayTask *task);
	bool uniqueRemove(GatewayTask *task);
	void accountVerifyOK(const DWORD accid,const bool ok);
	void execAll(GatewayTaskCallback &callback);

private:

	/**
	* \brief ���Ψһʵ��ָ��
	*
	*/
	static GatewayTaskManager *instance;

	/**
	* \brief ���캯��
	*
	*/
	GatewayTaskManager();

	/**
	* \brief ������������
	*
	*/
	typedef /*__gnu_cxx::*/hash_map<DWORD,GatewayTask *> GatewayTaskHashmap;
	/**
	* \brief ������������������
	*
	*/
	typedef GatewayTaskHashmap::iterator GatewayTaskHashmap_iterator;
	/**
	* \brief ����������������������
	*
	*/
	typedef GatewayTaskHashmap::const_iterator GatewayTaskHashmap_const_iterator;
	/**
	* \brief ����������ֵ������
	*
	*/
	typedef GatewayTaskHashmap::value_type GatewayTaskHashmap_pair;
	/**
	* \brief �������ʻ������
	*
	*/
	zRWLock rwlock;
	/**
	* \brief �����ӹ�����������
	*
	*/
	GatewayTaskHashmap gatewayTaskSet;

};

/**
* \brief ����Ự���������ӿͻ�����
*
*/
class SessionClient : public zTCPBufferClient
{

public:

	SessionClient(
		const std::string &name,
		const std::string &ip,
		const WORD port)
		: zTCPBufferClient(name,ip,port) {};

	bool connectToSessionServer();
	void run();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

};

extern SessionClient *sessionClient;

class GateUserManager;

/**
* \brief ��ɫѡ��
*
*/
class GateSelectUserSession:private zNoncopyable
{
	friend class GateUserAccountID;
protected:
	Cmd::SelectUserInfo userinfo[Cmd::MAX_CHARINFO];

	GateSelectUserSession(DWORD accid)
	{
		this->accid=accid;
		bzero(userinfo,sizeof(userinfo));
	}
public:
	DWORD accid;
	WORD face;

	void setSelectUserInfo(const Cmd::Record::t_Ret_SelectInfo_GateRecord *ptCmd);

	void putSelectUserInfo(const Cmd::SelectUserInfo &info);

	/**
	* \brief ɾ��ѡ��Ľ�ɫ
	*
	*
	* \param charid: ��ɫid
	* \return ɾ���Ƿ�ɹ�
	*/
	bool delSelectUserInfo(DWORD charid)
	{
		bool empty=true;
		for(int i = 0; i < Cmd::MAX_CHARINFO; i++)
		{
			if (userinfo[i].id== charid && userinfo[i].id!=0 && userinfo[i].id!=(DWORD)-1)
			{
				bzero(&userinfo[i],sizeof(Cmd::SelectUserInfo));
			}
			if (userinfo[i].id!=0 && userinfo[i].id!=(DWORD)-1)
				empty=false;
		}
		return empty;
	}

	/**
	* \brief ���ݽ�ɫ��ŵõ�һ����ɫ��Ϣ
	*
	*
	* \param num: ��ɫ���
	* \return ��ɫ��Ϣ
	*/
	Cmd::SelectUserInfo *getSelectUserInfo(WORD num)
	{
		if (num>=Cmd::MAX_CHARINFO)
			return NULL;
		return &userinfo[num];
	}

	/**
	* \brief �жϽ�ɫ�Ƿ�ﵽ����ɫ����
	*
	*
	* \return ��ɫ������ture,���򷵻�false
	*/
	bool charInfoFull()
	{
		bool retval = false;
		for(int i = 0; i < Cmd::MAX_CHARINFO; i++)
		{
			if (userinfo[i].id != 0 && userinfo[i].id != (DWORD)-1)
			{
				retval = true;
			}
		}
		/*
		bool retval = true;
		for(int i = 0; i < Cmd::MAX_CHARINFO; i++)
		{
		if (userinfo[i].id == 0 || userinfo[i].id == (DWORD)-1)
		{
		retval = false;
		}
		}
		// */
		return retval;
	}
};

class GatewayTask;
class SceneClient;
/**
* \brief �����û�
*
*/
class GateUser:public zUser,public GateSelectUserSession
{
	friend class GatewayTask;
private:

	/// �û������˳�״̬(socket�Ѿ��Ͽ�)
	bool logout;
	GatewayTask *gatewaytask;
	/// �ĸ�״̬�����ص�
	enum Systemstate
	{
		SYSTEM_STATE_INITING,   /// ��ʼ״̬
		SYSTEM_STATE_SELECTSERVER, // [ranqd] �ȴ�ѡ�������״̬
		SYSTEM_STATE_CREATING,   /// ������ɫ״̬
		SYSTEM_STATE_SELECT,   /// ѡ���ɫ״̬
		SYSTEM_STATE_PLAY,     /// ��Ϸ״̬
		SYSTEM_WAIT_STATE_PLAY,   /// �ȴ���Ϸ״̬
		SYSTEM_WAIT_STATE_UNREG    /// �ȴ��˳���ɫ����
	};
	volatile Systemstate systemstate;

	bool quiz;
	/// ��֤��
	char jpegPassport[5];
	/// �������б� 
	std::set<std::string> blacklist; 
	typedef std::set<std::string>::value_type blackListValueType;
	zRWLock rwlock;

public:

	//����������
	unsigned short dupIndex;

	/// ����һ�´�����ɫָ��
	Cmd::Record::t_CreateChar_GateRecord createCharCmd;

	bool backSelect;
	SceneClient *scene;
	DWORD sceneTempID;

	DWORD countryID;
	DWORD sceneID;

	DWORD CountryID;   // [ranqd] Add ������ID

	GateUser(DWORD accID,GatewayTask *histask);
	~GateUser();

	/**
	* \brief ��ʼ��״̬����
	*
	*/
	void initState()
	{
		systemstate = SYSTEM_STATE_INITING;
	}
	/**
	* \brief �Ƿ���ɳ�ʼ��
	*
	*
	* \return �����ɷ���ture,����false
	*/
	bool isInitState() const
	{
		return SYSTEM_STATE_INITING == systemstate;
	}
	/**
	* \brief ������ɫ״̬
	*
	*
	*/
	void createState()
	{
		systemstate = SYSTEM_STATE_CREATING;
	}
	/**
	* \brief �Ƿ��ڴ�����ɫ״̬
	*
	*
	* \return ����ڴ�����ɫ״̬����ture,���򷵻�false
	*/
	bool isCreateState() const
	{
		return SYSTEM_STATE_CREATING == systemstate;
	}
	/**
	* \brief ����ѡ���ɫ״̬
	*
	*
	*/
	void selectState()
	{
		systemstate=SYSTEM_STATE_SELECT;
	}
	/**
	* \brief �Ƿ��ڵȴ��˳���ɫ״̬
	*
	*
	* \return ���˳���ɫ״̬����ture,���򷵻�false
	*/
	bool isWaitUnregState() const
	{
		return SYSTEM_WAIT_STATE_UNREG == systemstate;
	}


	/**
	* \brief �����˳��ȴ�״̬
	*
	*
	*/
	void waitUnregState()
	{
		systemstate = SYSTEM_WAIT_STATE_UNREG;
	}
	/**
	* \brief �Ƿ���ѡ���ɫ״̬
	*
	*
	* \return ��ѡ��״̬����ture,���򷵻�false
	*/
	bool isSelectState() const
	{
		return SYSTEM_STATE_SELECT == systemstate;
	}
	// [ranqd] Add �����û�����ѡ�������״̬
	void selectServerState()
	{
		systemstate = SYSTEM_STATE_SELECTSERVER;
	}
	// [ranqd] Add �ж��û��Ƿ���ѡ�������״̬
	bool isSelectServerState() const
	{
		return SYSTEM_STATE_SELECTSERVER == systemstate;
	}

	void playState(SceneClient *s=NULL,DWORD scene_tempid=0);

	/**
	* \brief ���þ���״̬
	*
	*/
	void quizState()
	{
		quiz = true;
	}

	/**
	* \brief �������״̬
	*
	*/
	void clearQuizState()
	{
		quiz = false;
	}

	/**
	* \brief �Ƿ��ھ���״̬
	*
	*/
	bool isQuizState()
	{
		return quiz;
	}

	/**
	* \brief �Ƿ�����Ϸ״̬
	*
	*
	* \return ����Ϸ״̬����ture,���򷵻�false
	*/
	bool isPlayState() const
	{
		return SYSTEM_STATE_PLAY == systemstate;
	}

	/**
	* \brief �Ƿ��ڵȴ���Ϸ״̬
	* ����һ��״̬:���ظո��յ�ѡ���ɫָ����յ��˳�ָ��,���ܳ���"id����ʹ��"�����
	*
	*
	* \return ����Ϸ״̬����ture,���򷵻�false
	*/
	bool isWaitPlayState() const
	{
		return SYSTEM_WAIT_STATE_PLAY == systemstate;
	}
	/**
	* \brief ���õȴ���Ϸ״̬
	*
	*
	*/
	void waitPlayState()
	{
		systemstate = SYSTEM_WAIT_STATE_PLAY;
	}

	void Terminate();
	void refreshCharInfo();
	bool checkPassport(const char *passport);
	/**
	* \brief ֪ͨ�ͻ���û�д����Ľ�ɫ
	*
	*/
	void noCharInfo()
	{
		using namespace Cmd;
		stServerReturnLoginFailedCmd cmd;
		cmd.byReturnCode=LOGIN_RETURN_USERDATANOEXIST;
		if (sendCmd(&cmd,sizeof(cmd)))
		{
			selectState();
		}
	}

	/**
	* \brief ֪ͨ�ͻ��������ظ�
	*
	*/
	void nameRepeat()
	{
		using namespace Cmd;
		stServerReturnLoginFailedCmd cmd;
		cmd.byReturnCode=LOGIN_RETURN_CHARNAMEREPEAT;
		if (sendCmd(&cmd,sizeof(cmd)))
		{
			selectState();
		}
	}

	/**
	* \brief ��ȡ�ʺ�
	*
	*/
	const char* getAccount();

	/**
	* \brief ����Ϣת��������
	*
	*/
	bool forwardScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardSceneBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	bool forwardBillScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen);
	void setVip(bool b);

	void SendCountryInfo();
	bool beginSelect();
	void reg(int charno);
	void unreg(bool out=false);
	bool beginGame();
	bool sendCmd(const void *pstrCmd,const DWORD nCmdLen,const DWORD type=0,const char *strName=NULL,const bool hasPacked=false);
	static void cmdFilter(Cmd::stNullUserCmd *cmd,DWORD &type,char *name,DWORD &cmdLen);
	void final();
	void addBlackList(const char *);
	void removeBlackList(const char *);
	bool checkChatCmd(DWORD type,const char *strName);
public:
	/// ��ͼ������hash key
	DWORD mapScreenIndex;
	/// �Ƿ�����
	bool hide;

	/// �Ƿ��Ѿ���ӵ�����
	bool inserted;

	/// ϵͳ����
	BYTE sysSetting[20];

public:
	/**
	* \brief ˢ���û�������
	*
	*
	* \param screen �µ�������hashkey
	* \return 
	*/
	void setIndexKey(const DWORD screen)
	{
		mapScreenIndex = screen;    
	}
	const DWORD getIndexKey() const
	{
		return mapScreenIndex;
	}
	bool isHide() const
	{
		return hide;
	}
};

/**
* \brief GateUser���ʺ�IDΪkeyֵ��ָ������,��Ҫ�̳�ʹ��
*/
class GateUserAccountID:protected LimitHash<DWORD,GateUser *>
{
protected:

	/**
	* \brief ��GateUser����������
	* \param e Ҫ�����GateUser
	* \return �ɹ�����true,���򷵻�false
	*/
	bool push(GateUser * &e)
	{
		GateUser *account;
		return (!find(e->accid,account) && insert(e->accid,e));
	}

public:
	GateUserAccountID() {}
	virtual ~GateUserAccountID() {}
	/**
	* \brief ͨ���ʺ�ID�õ�GateUser
	* \param accid Ҫ�õ�GateUser���ʺ�ID
	* \return ����GateUserָ��,δ�ҵ�����NULL
	*/
	virtual GateUser * getUserByAccID(DWORD accid) =0;
	/**
	* \brief ͨ���ʺ�IDɾ��GateUser,�����ʺ��������Ƴ�
	* \param accid Ҫɾ����GateUser���ʺ�ID
	*/
	virtual void removeUserOnlyByAccID(DWORD accid) =0;
	/**
	* \brief ͨ���ʺ�ID���GateUser,����ӵ��ʺ�������
	* \param user Ҫ��ӵ�GateUser
	*/
	virtual bool addUserOnlyByAccID(GateUser *user) =0;
};

/**
* \brief �����û�������
*
*/
class GateUserManager:public zUserManager,protected GateUserAccountID
{
private:
	bool inited;
	zUniqueDWORDID *userUniqeID;
	static GateUserManager *gum;

	GateUserManager();
	~GateUserManager();
	bool getUniqeID(DWORD &tempid);
	void putUniqeID(const DWORD &tempid);
public:
	static GateUserManager *getInstance();
	static void delInstance();
	bool init();
	void final();
	GateUser * getUserByAccID(DWORD accid);
	void removeUserOnlyByAccID(DWORD accid);
	bool addUserOnlyByAccID(GateUser *user);
	void removeUserBySceneClient(SceneClient *scene);
	void removeAllUser();
	bool addCountryUser(GateUser *user);
	void removeCountryUser(GateUser *user);
	template <class YourNpcEntry>
	void execAllOfCountry(const DWORD country,execEntry<YourNpcEntry> &callback);
	void sendCmdToCountry(const DWORD country,const void *pstrCmd,const DWORD nCmdLen);
private:
	typedef std::set<GateUser *> GateUser_SET;
	typedef GateUser_SET::iterator GateUser_SET_iter;
	typedef /*__gnu_cxx::*/hash_map<DWORD,GateUser_SET> CountryUserMap;
	typedef CountryUserMap::iterator CountryUserMap_iter;
	CountryUserMap countryindex;
};
/* 
class RecycleUserManager:public zUserManager
{
private:
RecycleUserManager()
{
}
~RecycleUserManager()
{
}
public:
static RecycleUserManager *getInstance();
static void delInstance();
private:
static RecycleUserManager *instance;
};
// */
// [ranqd] Add ������״̬
enum SERVER_STATE 
{
	STATE_SERVICING	=	0, // ά��
	STATE_NOMARL	=	1, // ����
	STATE_GOOD		=	2, // ����
	STATE_BUSY		=	3, // ��æ
	STATE_FULL		=	4, // ����
};
// [ranqd] Add ����������
enum SERVER_TYPE
{
	TYPE_GENERAL		=	0, // ��ͨ
	TYPE_PEACE		=	1,     // ��ƽ
};
/**
* \brief ���������ļ���Ϣ
*
*/
class CountryInfo
{
public:
	struct Info
	{
		DWORD countryid;
		DWORD function;
		int   type;   // [ranqd Add] ���������ͣ��ο�enum SERVER_TYPE
 		DWORD Online_Now;
 		DWORD Online_Max;
		std::string countryname;
		std::string mapname;
		Info()
		{
			countryid=0;
			function=0;
			Online_Now = 0;
			Online_Max = 2000; // Ĭ��һ���������������2000��
		}
	};
private:
	typedef std::vector<Info> StrVec;
	typedef StrVec::iterator StrVec_iterator;
	StrVec country_info;
	// ����������
	zMutex mutex;
	DWORD country_order[100];

	struct CountryDic
	{
		DWORD id;
		char name[MAX_NAMESIZE];
		DWORD mapid;
		DWORD function;
		int  type;  //[ranqd Add] ��������
		CountryDic()
		{
			id=0;
			mapid=0;
			bzero(name,sizeof(name));
			function=0; 
		}
	};
	struct MapDic
	{
		DWORD id;
		char name[MAX_NAMESIZE];
		char filename[MAX_NAMESIZE];
		DWORD backto;
		MapDic()
		{
			id=0;
			bzero(name,sizeof(name));
			bzero(filename,sizeof(filename));
			backto=0;
		}
	};
	typedef std::map<DWORD,CountryDic> CountryMap;
	typedef CountryMap::iterator CountryMap_iter;
	typedef CountryMap::value_type CountryMap_value_type;
	CountryMap country_dic;
	typedef std::map<DWORD,MapDic> MapMap;
	typedef MapMap::value_type MapMap_value_type;
	typedef MapMap::iterator MapMap_iter;
	MapMap map_dic;

public:
	CountryInfo()
	{
		bzero(country_order,sizeof(country_order));
	}
	~CountryInfo(){}
	Info *getInfo(DWORD country_id);
	bool init();
	bool reload();
	int getCountrySize();
	int getCountryState( CountryInfo::Info cInfo );
	int getAll(char *buf);
	DWORD getCountryID(DWORD country_id);
	DWORD getRealMapID(DWORD map_id);
	const char *getRealMapName(const char *name);
	void setCountryOrder(Cmd::Session::CountrOrder *ptCmd);
	std::string getCountryName(DWORD country_id);
	std::string getMapName(DWORD country_id);
	bool isEnableLogin(DWORD country_id);
	bool isEnableRegister(DWORD country_id);
	void processChange(GateUser *pUser,Cmd::Scene::t_ChangeCountryStatus *rev);
	// [ranqd] ���¹�����������
	void UpdateCountryOnline( DWORD country_id, DWORD online_numbers ); 
};


/**
* \brief �������ط�����
*
* �����ʹ����Singleton���ģʽ����֤��һ��������ֻ��һ�����ʵ��
*
*/
class GatewayService : public zSubNetService
{

public:

	bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

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

	const int getPoolState() const
	{
		return taskPool->getState();
	}

	bool notifyLoginServer();

	/**
	* \brief У��ͻ��˰汾��
	*/
	DWORD verify_client_version;

	/**
	* \brief ����������
	*
	*/
	~GatewayService()
	{
		if (sessionClient)
		{
			sessionClient->final();
			sessionClient->join();
			SAFE_DELETE(sessionClient);
		}

	}

	/**
	* \brief �������Ψһʵ��
	*
	* \return ���Ψһʵ��
	*/
	static GatewayService &getInstance()
	{
		if (NULL == instance)
			instance = new GatewayService();

		return *instance;
	}

	/**
	* \brief �ͷ����Ψһʵ��
	*
	*/
	static void delInstance()
	{
		//�ر��̳߳�
		if (taskPool)
		{
			taskPool->final();
			SAFE_DELETE(taskPool);
		}

		SAFE_DELETE(instance);
	}

	void reloadConfig();
	bool isSequeueTerminate() 
	{
		return taskPool == NULL;
	}


private:

	/**
	* \brief ���Ψһʵ��ָ��
	*
	*/
	static GatewayService *instance;

	static zTCPTaskPool *taskPool;        /**< TCP���ӳص�ָ�� */


	//��������(��ͼ)��Ϣ

	/**
	* \brief ���캯��
	*
	*/
	GatewayService() : zSubNetService("���ط�����",GATEWAYSERVER)
	{
		rolereg_verify = true;
		taskPool = NULL;
		startUpFinish = false;
	}

	bool init();
	void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
	void final();

	/**
	* \brief ȷ�Ϸ�������ʼ���ɹ��������������ص�����
	*
	* �����������t_Startup_OKָ����ȷ�Ϸ����������ɹ�
	* ����֪ͨ���е�½����������̨���ط�����׼������
	*
	* \return ȷ���Ƿ�ɹ�
	*/
	virtual bool validate()
	{
		zSubNetService::validate();
		while(!startUpFinish)
		{
			if(!notifyLoginServer())
				return false;
			Sleep(10);
		}
		//return notifyLoginServer();
	}


public:
	CountryInfo country_info;
	bool rolereg_verify;
	static bool service_gold;
	static bool service_stock;
	bool startUpFinish;


};

/**
* \brief ���ض�ʱ���߳�
*
*/
class GatewayTimeTick : public zThread
{

public:

	static zRTime currentTime;

	~GatewayTimeTick() {};

	/**
	* \brief �õ�Ψһʵ��
	*
	*
	* \return Ψһʵ��
	*/
	static GatewayTimeTick &getInstance()
	{
		if (NULL == instance)
			instance = new GatewayTimeTick();

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

	static GatewayTimeTick *instance;

	GatewayTimeTick() : zThread("TimeTick"),one_second(1) {};

	Timer one_second;

};

/**
* \brief ��½�Ự������
*
*/
class LoginSessionManager
{

public:

	/**
	* \brief ��������
	*
	*/
	~LoginSessionManager() {};

	/**
	* \brief ��ȡ��½�Ự��������Ψһʵ��
	*
	* \return �Ự������Ψһʵ��
	*/
	static LoginSessionManager &getInstance()
	{
		if (NULL == instance)
			instance = new LoginSessionManager();

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

	void put(const t_NewLoginSession &session);
	bool verify(const DWORD loginTempID, const DWORD accid,char *numPassword, ZES_cblock *key=0);
	void update(const zRTime &ct);

private:

	/**
	* \brief �Ự��������Ψһʵ��ָ��
	*
	*/
	static LoginSessionManager *instance;

	/**
	* \brief ���캯��
	*
	*/
	LoginSessionManager() : lastUpdateTime() {};

	/**
	* \brief ���һ�θ���ʱ��
	*
	*/
	zRTime lastUpdateTime;

	/**
	* \brief �����¼��
	*
	*/
	bool checkUpdateTime(const zRTime &ct)
	{
		bool retval = false;
		if (ct >= lastUpdateTime)
		{
			lastUpdateTime = ct;
			lastUpdateTime.addDelay(1000);
			retval = true;
		}
		return retval;
	}

	struct LoginSession
	{
		t_NewLoginSession session;
		zTime timestamp;

		LoginSession(const t_NewLoginSession &session) : session(session), timestamp()
		{
		}
		LoginSession(const LoginSession& ls)
		{
			session = ls.session;
			timestamp = ls.timestamp;
		}
		LoginSession & operator= (const LoginSession &ls)
		{
			session = ls.session;
			timestamp = ls.timestamp;
			return *this;
		}
	};
	/**
	* \brief ������������
	*
	*/
	typedef /*__gnu_cxx::*/hash_map<DWORD, LoginSession> LoginSessionHashmap;
	/**
	* \brief �������������
	*
	*/
	typedef LoginSessionHashmap::iterator LoginSessionHashmap_iterator;
	/**
	* \brief �����ֵ������
	*
	*/
	typedef LoginSessionHashmap::value_type LoginSessionHashmap_pair;
	/**
	* \brief �Ự����
	*
	*/
	LoginSessionHashmap sessionData;
	/**
	* \brief �������ʻ������
	*
	*/
	zMutex mlock;

};

/**
* \brief �����뵵��������������
*
*/
class RecordClient : public zTCPBufferClient
{

public:

	/**
	* \brief ���캯��
	*
	* \param name ����
	* \param ip ��������ַ
	* \param port �������˿�
	*/
	RecordClient(
		const std::string &name,
		const std::string &ip,
		const WORD port)
		: zTCPBufferClient(name,ip,port) {};

	bool connectToRecordServer();
	void run();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

};

extern RecordClient *recordClient;

/**
* \brief ����С��Ϸ���������ӿͻ�����
*
*/
class MiniClient : public zTCPBufferClient
{

public:

	MiniClient(
		const std::string &name,
		const std::string &ip,
		const WORD port,
		const WORD serverid)
		: zTCPBufferClient(name,ip,port) 
	{
		wdServerID=serverid;
	};

	bool connectToMiniServer();
	void run();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
	/**
	* \brief ��ȡ�����������ı��
	*
	* \return �������������
	*/
	const WORD getServerID() const
	{
		return wdServerID;
	}
private:
	WORD wdServerID;

};

extern MiniClient *miniClient;

/**
* \brief ����Ʒѷ��������ӿͻ�����
*
*/
class BillClient : public zTCPBufferClient
{

public:

	BillClient(
		const std::string &name,
		const std::string &ip,
		const WORD port,
		const WORD serverid)
		: zTCPBufferClient(name,ip,port) 
	{
		wdServerID=serverid;
	};

	bool connectToBillServer();
	void run();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
	/**
	* \brief ��ȡ�����������ı��
	*
	* \return �������������
	*/
	const WORD getServerID() const
	{
		return wdServerID;
	}
private:
	WORD wdServerID;

};

extern BillClient *accountClient;

class CheckInfo
{
public:
	CheckInfo() : _previous(0),_last(0),_packets(0)
	{ }

	zRTime& previous()
	{
		return _previous;
	}

	void previous(const zRTime& time)
	{
		_previous = time;  
	}


	zRTime& last()
	{
		return _last;
	}

	void last(const zRTime& time)
	{
		_last = time;  
	}

	int packets() const
	{
		return _packets;
	}

	void packets(int packets_)
	{
		_packets = packets_;  
	}

private:
	zRTime _previous;
	zRTime _last;
	int _packets;
};

template <int interval,int count,int percent>
class percent_up_off
{
public:
	enum {
		MIN_INTERVAL = static_cast<int>(interval*(1.0-percent/100.0)),
		MAX_INTERVAL = static_cast<int>(interval*(1.0+percent/100.0)), 
		COUNT_TIME = interval*count,
	};

	static bool check_interval(const zRTime& current,CheckInfo& check)
	{
		//Zebra::logger->debug("previous:%ld,current:%ld,interval:%d,must_interval:%d",check.previous().msecs(),current.msecs(),check.previous().elapse(current),MIN_INTERVAL);
		if (check.previous().elapse(current) >= MIN_INTERVAL ) {
			check.previous(current);
			return true;
		}

		return false;
	}

	static bool check_count(const zRTime& current,CheckInfo& check)
	{
		//not reached 
		int packets = check.packets();
		if (packets < count) {
			check.packets(++packets);
			return true;
		}
		//time ok
		//Zebra::logger->debug("last:%ld,current:%ld,elpase:%d,must_elpase:%d",check.last().msecs(),current.msecs(),check.last().elapse(current),COUNT_TIME);
		if (check.last().elapse(current) >= COUNT_TIME ) {
			check.packets(0);
			check.last(current);
			return true;
		}

		return false;
	}
};

template<typename I = int>
class ICmdChecker
{
public:
	virtual bool check(I cmd,const zRTime& current) = 0;
	//  virtual void add(I cmd) = 0;  
	virtual ~ICmdChecker() { }
};

template <typename O,typename I = int>
class CmdChecker : public ICmdChecker<I>
{
public:  
	/*
	typedef CmdChecker<O,I> self_t;
	static self_t& instance()
	{
	if (!_instance) {
	static self_t new_instance;
	_instance = new_instance;
	}
	return *_instance;
	}
	*/  
	bool check(I cmd,const zRTime& current)
	{
		iterator it = _cmds.find(cmd);
		if (it != _cmds.end()) {
			return do_check(current);
		}

		return true;
	}

	void add(I cmd)
	{
		_cmds.insert(cmd);    
	}

	CmdChecker()
	{

	}

	~CmdChecker()
	{

	}

private:
	bool do_check(const zRTime& current)
	{
		return O::check_interval(current,_check) && O::check_count(current,_check); 
	}

	//static self_t* _instance;

	typedef std::set<I> set;
	typedef typename set::iterator iterator;
	set _cmds; 

	CheckInfo _check;

};

/*
template <typename O,typename I>
self_t* CmdChecker<O,I>::_instance = NULL;
*/

class CheckerTable
{
public:  
	//  static CheckerTable& instance();

	bool check(int cmd,const zRTime& current) const;

	CheckerTable();
	~CheckerTable();

private:

	bool init();

	class FreeMemory 
	{
	public:
		template <typename T>
		void operator () (T target)
		{
			SAFE_DELETE(target.second);
		}  
	};

	//  static CheckerTable* _instance;  

	typedef CmdChecker< percent_up_off<512,10,5> > Checker;
	typedef CmdChecker< percent_up_off<512,10,5> > MoveChecker;

	//  typedef std::vector< ICmdChecker<int>* > CHECK;

	typedef std::map< int,ICmdChecker<int>* > CHECK;
	typedef CHECK::const_iterator const_iterator;

	CHECK _checkers; 

};

/**
* \brief ����������
*
*/
class ScreenIndex :private zNoncopyable
{
private:
	//��д��
	zRWLock wrlock;

	typedef std::set<GateUser *,std::less<GateUser *>/*,__gnu_cxx::__pool_alloc<GateUser *>*/ > SceneEntry_SET;
	typedef /*__gnu_cxx::*/hash_map<DWORD,SceneEntry_SET> PosIMapIndex;
	/**
	* \brief map��������
	*/
	PosIMapIndex index;
	SceneEntry_SET all;

	///���������Ļ
	const DWORD screenx;
	///���������Ļ
	const DWORD screeny;
	const DWORD screenMax;
	//�ڼ��ص�ʱ����������ϵ������
	typedef /*__gnu_cxx::*/hash_map<DWORD,zPosIVector> NineScreen_map;
	typedef NineScreen_map::iterator NineScreen_map_iter;
	typedef NineScreen_map::value_type NineScreen_map_value_type;
	NineScreen_map ninescreen;
	NineScreen_map direct_screen[8];
	NineScreen_map reversedirect_screen[8];
	inline const zPosIVector &getNineScreen(const zPosI &posi);
	inline const zPosIVector &getDirectScreen(const zPosI &posi,const int direct);
	inline const zPosIVector &getReverseDirectScreen(const zPosI &posi,const int direct);
public:
	/**
	** \brief ���캯��
	**/
	ScreenIndex(const DWORD x,const DWORD y);
	template <class YourNpcEntry>
	void execAllOfScreen(const DWORD screen,execEntry<YourNpcEntry> &callback);
	void sendCmdToNine(const DWORD posi,const void *pstrCmd,const int nCmdLen,unsigned short dupIndex = 0);
	void sendCmdToDirect(const zPosI posi,const int direct,const void *pstrCmd,const int nCmdLen,unsigned short dupIndex = 0);
	void sendCmdToReverseDirect(const zPosI posi,const int direct,const void *pstrCmd,const int nCmdLen,unsigned short dupIndex = 0);
	void sendCmdToAll(const void *pstrCmd,const int nCmdLen);
	void sendCmdToNineExceptMe(const DWORD posi,const DWORD exceptme_id,const void *pstrCmd,const int nCmdLen);
	bool refresh(GateUser *e,const DWORD newScreen);
	void removeGateUser(GateUser *e);
};
/**
* \brief ���ص�ͼ����
*
*/
typedef std::map<DWORD,ScreenIndex*> MapIndex;
typedef MapIndex::iterator MapIndexIter;

/**
* \brief ���峡�����������ӿͻ�����
**/
class SceneClient : public zTCPClientTask
{

public:

	SceneClient(
		const std::string &ip,
		const WORD port);
	SceneClient( const std::string &name,const Cmd::Super::ServerEntry *serverEntry)
		: zTCPClientTask(serverEntry->pstrIP,serverEntry->wdPort)
	{
		wdServerID=serverEntry->wdServerID;
	};
	~SceneClient()
	{
		Zebra::logger->debug("SceneClient����");
	}

	int checkRebound();
	void addToContainer();
	void removeFromContainer();
	bool connectToSceneServer();
	bool connect();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
	void freshIndex(GateUser *pUser,const DWORD map,const DWORD screen)
	{
		MapIndexIter iter = mapIndex.find(map); 
		if (iter != mapIndex.end())
		{
			iter->second->refresh(pUser,screen);
		}
	}
	void removeIndex(GateUser *pUser,const DWORD map)
	{
		MapIndexIter iter = mapIndex.find(map); 
		if (iter != mapIndex.end())
		{
			iter->second->removeGateUser(pUser);
		}
	}
	const WORD getServerID() const
	{
		return wdServerID;
	}

private:

	/**
	* \brief ����������
	*
	*
	* \param 
	* \return 
	*/
	MapIndex mapIndex;
	/**
	* \brief ���������
	*
	*/
	WORD wdServerID;



};
#if 0
/**
* \brief ���峡�����������ӿͻ�����
*
*/
class SceneClient : public zTCPBufferClient
{

public:

	/**
	* \brief ���캯��
	*
	* \param name ����
	* \param serverEntry ������������Ϣ
	*/
	SceneClient( const std::string &name,const Cmd::Super::ServerEntry *serverEntry)
		: zTCPBufferClient(name,serverEntry->pstrIP,serverEntry->wdPort,false,8000)
	{
		wdServerID=serverEntry->wdServerID;
	};

	~SceneClient()
	{
		Zebra::logger->debug("SceneClient����");
	}
	bool connectToSceneServer();
	void run();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

	/**
	* \brief ��ȡ�����������ı��
	*
	* \return �������������
	*/
	const WORD getServerID() const
	{
		return wdServerID;
	}
	void freshIndex(GateUser *pUser,const DWORD map,const DWORD screen)
	{
		MapIndexIter iter = mapIndex.find(map); 
		if (iter != mapIndex.end())
		{
			iter->second->refresh(pUser,screen);
		}
	}
	void removeIndex(GateUser *pUser,const DWORD map)
	{
		MapIndexIter iter = mapIndex.find(map); 
		if (iter != mapIndex.end())
		{
			iter->second->removeGateUser(pUser);
		}
	}

private:

	/**
	* \brief ����������
	*
	*
	* \param 
	* \return 
	*/
	MapIndex mapIndex;
	/**
	* \brief ���������
	*
	*/
	WORD wdServerID;

};

/**
* \brief �������������ӹ�����
*
*/
class SceneClientManager
{

public:

	/**
	* \brief ��������
	*
	*/
	~SceneClientManager() {};

	/**
	* \brief ��ȡ��������Ψһʵ��
	*
	* \return ������Ψһʵ��
	*/
	static SceneClientManager &getInstance()
	{
		if (NULL == instance)
			instance = new SceneClientManager();

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

	void add(SceneClient *client);
	void remove(SceneClient *client);
	SceneClient *getByServerID(WORD serverid);
	void final();

private:

	/**
	* \brief �������������ӹ�����Ψһʵ��ָ��
	*
	*/
	static SceneClientManager *instance;

	/**
	* \brief ���캯��
	*
	*/
	SceneClientManager() {};

	/**
	* \brief �������ʻ������
	*
	*/
	zMutex mlock;
	/**
	* \brief ������������������
	*
	*/
	std::vector<SceneClient *> sceneClients;

};

#endif

/**
** \brief �����������Ϣ�ɼ����ӵĿͻ��˹�������
**/
class SceneClientManager
{

public:

	~SceneClientManager();

	/**
	** \brief ��ȡ���Ψһʵ��
	** \return ���Ψһʵ������
	**/
	static SceneClientManager &getInstance()
	{
		if (NULL == instance)
			instance = new SceneClientManager();

		return *instance;
	}

	/**
	** \brief �������Ψһʵ��
	**/
	static void delInstance()
	{
		SAFE_DELETE(instance);
	}

	bool init();
	void timeAction(const zTime &ct);
	void add(SceneClient *sceneClient);
	void remove(SceneClient *sceneClient);
	bool broadcastOne(const void *pstrCmd,int nCmdLen);
	bool sendTo(const DWORD tempid,const void *pstrCmd,int nCmdLen);
	void setUsleepTime(int time)
	{
		sceneClientPool->setUsleepTime(time);
	}

private:

	SceneClientManager();
	static SceneClientManager *instance;

	/**
	** \brief �ͻ������ӹ����
	**/
	zTCPClientTaskPool *sceneClientPool;
	/**
	** \brief ���ж�����������ʱ���¼
	**/
	zTime actionTimer;

	/**
	** \brief ��������Ѿ��ɹ���������������
	**/
	typedef std::map<const DWORD,SceneClient *> SceneClient_map;
	typedef SceneClient_map::iterator iter;
	typedef SceneClient_map::const_iterator const_iter;
	typedef SceneClient_map::value_type value_type;
	/**
	** \brief ��������Ѿ��ɹ�����������
	**/
	SceneClient_map allClients;


	/**
	** \brief �������ʶ�д��
	**/
	zRWLock rwlock;

};
