/**
* \brief zebra项目Gateway服务器,负责用户指令检查转发、加密解密等
*/
#include <zebra/srvEngine.h>
#include <set>

/**
* \brief 服务器连接任务
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
	* \brief 得到该玩家的帐号id
	*
	* \return 得到的id
	*/
	DWORD getACCID() const
	{
		return accid;
	}

	/**
	* \brief 设置该帐号是否验证通过
	*
	*
	* \param ok 是否通过
	*/
	void accountVerifyOK(const bool ok)
	{
		if (ok)
			accountVerified = ACCOUNTVERIFY_SUCCESS;
		else
			accountVerified = ACCOUNTVERIFY_FAILURE;
	}

	/**
	* \brief 设置是否未vip用户
	*
	*/
	void setVip(bool vip)
	{
		vip_user = vip;
	}

	/**
	* \brief 是否是vip用户
	*
	*/
	bool isVip()
	{
		return vip_user;
	}

private:
	///数字密码
	char numPassword[MAX_NUMPASSWORD];
	/// 数字密码DWORD版
	DWORD numPwd;
	///vip用户
	bool vip_user;
	///时间校对定时器
	Timer _retset_gametime;
	///回收延时等到(毫秒)
	DWORD recycle_wait;
	///校验客户端时间的间隔
	static const DWORD sampleInterval = 20000;
	static const DWORD sampleInterval_sec = sampleInterval/1000;
	static const DWORD sampleInterval_error_sec = sampleInterval/1000;
	static const DWORD sampleInterval_error_msecs = sampleInterval;

	///聊天消息转发的间隔
	static const DWORD chatInterval = 1000;
	///下次聊天的时间
	zRTime nextChatTime;
	///下次国家聊天的时间
	zRTime nextCountryChatTime;

	///客户端在sampleInterval时间内发送超过maxSamplePPS个数据包则被判断为使用了外挂
	static const DWORD maxSamplePPS = 145;
	///上次晴空v_samplePackets的时间
	DWORD v_lastSampleTime;
	///统计数据包个数
	DWORD v_samplePackets;

	///该task初始化的时间
	zRTime initTime;
	///上次检查客户端是否已经校验了时间的时间
	zRTime lastCheckTime;
	///是否已经校验了时间
	volatile bool haveCheckTime;

	friend class GateUser;
	///使用该连接的玩家
	GateUser *pUser;

	///消息检查工具
	//CheckerTable checker;
	/**
	* \brief 账号编号
	*
	*/
	DWORD accid;

	/**     
	** \brief 游戏时间
	**
	**/
	QWORD qwGameTime;
	zRTime GameTimeSyn;
	QWORD dwTimestampServer;

	/**
	* \brief 用户帐号
	*
	*
	*/
	char account[MAX_ACCNAMESIZE+1];

	///登录时分配的临时id
	DWORD loginTempID;
	///是否验证了版本
	bool versionVerified;
	///帐号验证的状态
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
* \brief 服务器子连接管理器
*
*/
class GatewayTaskManager
{

public:

	/**
	* \brief 回调函数
	*
	*/
	typedef zEntryCallback<GatewayTask> GatewayTaskCallback;

	/**
	* \brief 析构函数
	*
	*/
	~GatewayTaskManager();

	/**
	* \brief 获取子连接管理器唯一实例
	*
	* \return 子连接管理器唯一实例
	*/
	static GatewayTaskManager &getInstance()
	{
		if (NULL == instance)
			instance = new GatewayTaskManager();

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

	bool uniqueAdd(GatewayTask *task);
	bool uniqueRemove(GatewayTask *task);
	void accountVerifyOK(const DWORD accid,const bool ok);
	void execAll(GatewayTaskCallback &callback);

private:

	/**
	* \brief 类的唯一实例指针
	*
	*/
	static GatewayTaskManager *instance;

	/**
	* \brief 构造函数
	*
	*/
	GatewayTaskManager();

	/**
	* \brief 定义容器类型
	*
	*/
	typedef /*__gnu_cxx::*/hash_map<DWORD,GatewayTask *> GatewayTaskHashmap;
	/**
	* \brief 定义容器迭代器类型
	*
	*/
	typedef GatewayTaskHashmap::iterator GatewayTaskHashmap_iterator;
	/**
	* \brief 定义容器常量迭代器类型
	*
	*/
	typedef GatewayTaskHashmap::const_iterator GatewayTaskHashmap_const_iterator;
	/**
	* \brief 定义容器键值对类型
	*
	*/
	typedef GatewayTaskHashmap::value_type GatewayTaskHashmap_pair;
	/**
	* \brief 容器访问互斥变量
	*
	*/
	zRWLock rwlock;
	/**
	* \brief 子连接管理容器类型
	*
	*/
	GatewayTaskHashmap gatewayTaskSet;

};

/**
* \brief 定义会话服务器连接客户端类
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
* \brief 角色选择
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
	* \brief 删除选择的角色
	*
	*
	* \param charid: 角色id
	* \return 删除是否成功
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
	* \brief 根据角色序号得到一个角色信息
	*
	*
	* \param num: 角色序号
	* \return 角色信息
	*/
	Cmd::SelectUserInfo *getSelectUserInfo(WORD num)
	{
		if (num>=Cmd::MAX_CHARINFO)
			return NULL;
		return &userinfo[num];
	}

	/**
	* \brief 判断角色是否达到最大角色数量
	*
	*
	* \return 角色满返回ture,否则返回false
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
* \brief 网关用户
*
*/
class GateUser:public zUser,public GateSelectUserSession
{
	friend class GatewayTask;
private:

	/// 用户处于退出状态(socket已经断开)
	bool logout;
	GatewayTask *gatewaytask;
	/// 四个状态不能重叠
	enum Systemstate
	{
		SYSTEM_STATE_INITING,   /// 初始状态
		SYSTEM_STATE_SELECTSERVER, // [ranqd] 等待选择服务器状态
		SYSTEM_STATE_CREATING,   /// 创建角色状态
		SYSTEM_STATE_SELECT,   /// 选择角色状态
		SYSTEM_STATE_PLAY,     /// 游戏状态
		SYSTEM_WAIT_STATE_PLAY,   /// 等待游戏状态
		SYSTEM_WAIT_STATE_UNREG    /// 等待退出角色流程
	};
	volatile Systemstate systemstate;

	bool quiz;
	/// 验证码
	char jpegPassport[5];
	/// 黑名单列表 
	std::set<std::string> blacklist; 
	typedef std::set<std::string>::value_type blackListValueType;
	zRWLock rwlock;

public:

	//副本号索引
	unsigned short dupIndex;

	/// 缓存一下创建角色指令
	Cmd::Record::t_CreateChar_GateRecord createCharCmd;

	bool backSelect;
	SceneClient *scene;
	DWORD sceneTempID;

	DWORD countryID;
	DWORD sceneID;

	DWORD CountryID;   // [ranqd] Add 服务器ID

	GateUser(DWORD accID,GatewayTask *histask);
	~GateUser();

	/**
	* \brief 初始化状态设置
	*
	*/
	void initState()
	{
		systemstate = SYSTEM_STATE_INITING;
	}
	/**
	* \brief 是否完成初始化
	*
	*
	* \return 如果完成返回ture,否则false
	*/
	bool isInitState() const
	{
		return SYSTEM_STATE_INITING == systemstate;
	}
	/**
	* \brief 创建角色状态
	*
	*
	*/
	void createState()
	{
		systemstate = SYSTEM_STATE_CREATING;
	}
	/**
	* \brief 是否在创建角色状态
	*
	*
	* \return 如果在创建角色状态返回ture,否则返回false
	*/
	bool isCreateState() const
	{
		return SYSTEM_STATE_CREATING == systemstate;
	}
	/**
	* \brief 设置选择角色状态
	*
	*
	*/
	void selectState()
	{
		systemstate=SYSTEM_STATE_SELECT;
	}
	/**
	* \brief 是否在等待退出角色状态
	*
	*
	* \return 在退出角色状态返回ture,否则返回false
	*/
	bool isWaitUnregState() const
	{
		return SYSTEM_WAIT_STATE_UNREG == systemstate;
	}


	/**
	* \brief 设置退出等待状态
	*
	*
	*/
	void waitUnregState()
	{
		systemstate = SYSTEM_WAIT_STATE_UNREG;
	}
	/**
	* \brief 是否在选择角色状态
	*
	*
	* \return 在选择状态返回ture,否则返回false
	*/
	bool isSelectState() const
	{
		return SYSTEM_STATE_SELECT == systemstate;
	}
	// [ranqd] Add 设置用户处于选择服务器状态
	void selectServerState()
	{
		systemstate = SYSTEM_STATE_SELECTSERVER;
	}
	// [ranqd] Add 判断用户是否处于选择服务器状态
	bool isSelectServerState() const
	{
		return SYSTEM_STATE_SELECTSERVER == systemstate;
	}

	void playState(SceneClient *s=NULL,DWORD scene_tempid=0);

	/**
	* \brief 设置竞赛状态
	*
	*/
	void quizState()
	{
		quiz = true;
	}

	/**
	* \brief 清除竞赛状态
	*
	*/
	void clearQuizState()
	{
		quiz = false;
	}

	/**
	* \brief 是否在竞赛状态
	*
	*/
	bool isQuizState()
	{
		return quiz;
	}

	/**
	* \brief 是否在游戏状态
	*
	*
	* \return 在游戏状态返回ture,否则返回false
	*/
	bool isPlayState() const
	{
		return SYSTEM_STATE_PLAY == systemstate;
	}

	/**
	* \brief 是否在等待游戏状态
	* 存在一种状态:网关刚刚收到选择角色指令后收到退出指令,可能出现"id正在使用"的情况
	*
	*
	* \return 在游戏状态返回ture,否则返回false
	*/
	bool isWaitPlayState() const
	{
		return SYSTEM_WAIT_STATE_PLAY == systemstate;
	}
	/**
	* \brief 设置等待游戏状态
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
	* \brief 通知客户端没有创建的角色
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
	* \brief 通知客户端名字重复
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
	* \brief 获取帐号
	*
	*/
	const char* getAccount();

	/**
	* \brief 将消息转发到场景
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
	/// 地图屏索引hash key
	DWORD mapScreenIndex;
	/// 是否隐身
	bool hide;

	/// 是否已经添加到索引
	bool inserted;

	/// 系统设置
	BYTE sysSetting[20];

public:
	/**
	* \brief 刷新用户屏索引
	*
	*
	* \param screen 新的屏索引hashkey
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
* \brief GateUser以帐号ID为key值的指针容器,需要继承使用
*/
class GateUserAccountID:protected LimitHash<DWORD,GateUser *>
{
protected:

	/**
	* \brief 将GateUser加入容器中
	* \param e 要加入的GateUser
	* \return 成功返回true,否则返回false
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
	* \brief 通过帐号ID得到GateUser
	* \param accid 要得到GateUser的帐号ID
	* \return 返回GateUser指针,未找到返回NULL
	*/
	virtual GateUser * getUserByAccID(DWORD accid) =0;
	/**
	* \brief 通过帐号ID删除GateUser,仅从帐号容器中移除
	* \param accid 要删除的GateUser的帐号ID
	*/
	virtual void removeUserOnlyByAccID(DWORD accid) =0;
	/**
	* \brief 通过帐号ID添加GateUser,仅添加到帐号容器中
	* \param user 要添加的GateUser
	*/
	virtual bool addUserOnlyByAccID(GateUser *user) =0;
};

/**
* \brief 网关用户管理器
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
// [ranqd] Add 服务器状态
enum SERVER_STATE 
{
	STATE_SERVICING	=	0, // 维护
	STATE_NOMARL	=	1, // 正常
	STATE_GOOD		=	2, // 良好
	STATE_BUSY		=	3, // 繁忙
	STATE_FULL		=	4, // 爆满
};
// [ranqd] Add 服务器类型
enum SERVER_TYPE
{
	TYPE_GENERAL		=	0, // 普通
	TYPE_PEACE		=	1,     // 和平
};
/**
* \brief 国家配置文件信息
*
*/
class CountryInfo
{
public:
	struct Info
	{
		DWORD countryid;
		DWORD function;
		int   type;   // [ranqd Add] 服务器类型，参考enum SERVER_TYPE
 		DWORD Online_Now;
 		DWORD Online_Max;
		std::string countryname;
		std::string mapname;
		Info()
		{
			countryid=0;
			function=0;
			Online_Now = 0;
			Online_Max = 2000; // 默认一个服务器最大在线2000人
		}
	};
private:
	typedef std::vector<Info> StrVec;
	typedef StrVec::iterator StrVec_iterator;
	StrVec country_info;
	// 国家排序锁
	zMutex mutex;
	DWORD country_order[100];

	struct CountryDic
	{
		DWORD id;
		char name[MAX_NAMESIZE];
		DWORD mapid;
		DWORD function;
		int  type;  //[ranqd Add] 国家类型
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
	// [ranqd] 更新国家在线人数
	void UpdateCountryOnline( DWORD country_id, DWORD online_numbers ); 
};


/**
* \brief 定义网关服务类
*
* 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
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
	* \brief 校验客户端版本号
	*/
	DWORD verify_client_version;

	/**
	* \brief 虚析构函数
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
	* \brief 返回类的唯一实例
	*
	* \return 类的唯一实例
	*/
	static GatewayService &getInstance()
	{
		if (NULL == instance)
			instance = new GatewayService();

		return *instance;
	}

	/**
	* \brief 释放类的唯一实例
	*
	*/
	static void delInstance()
	{
		//关闭线程池
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
	* \brief 类的唯一实例指针
	*
	*/
	static GatewayService *instance;

	static zTCPTaskPool *taskPool;        /**< TCP连接池的指针 */


	//国家名称(地图)信息

	/**
	* \brief 构造函数
	*
	*/
	GatewayService() : zSubNetService("网关服务器",GATEWAYSERVER)
	{
		rolereg_verify = true;
		taskPool = NULL;
		startUpFinish = false;
	}

	bool init();
	void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
	void final();

	/**
	* \brief 确认服务器初始化成功，即将进入主回调函数
	*
	* 向服务器发送t_Startup_OK指令来确认服务器启动成功
	* 并且通知所有登陆服务器，这台网关服务器准备好了
	*
	* \return 确认是否成功
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
* \brief 网关定时器线程
*
*/
class GatewayTimeTick : public zThread
{

public:

	static zRTime currentTime;

	~GatewayTimeTick() {};

	/**
	* \brief 得到唯一实例
	*
	*
	* \return 唯一实例
	*/
	static GatewayTimeTick &getInstance()
	{
		if (NULL == instance)
			instance = new GatewayTimeTick();

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

	static GatewayTimeTick *instance;

	GatewayTimeTick() : zThread("TimeTick"),one_second(1) {};

	Timer one_second;

};

/**
* \brief 登陆会话管理器
*
*/
class LoginSessionManager
{

public:

	/**
	* \brief 析构函数
	*
	*/
	~LoginSessionManager() {};

	/**
	* \brief 获取登陆会话管理器的唯一实例
	*
	* \return 会话管理器唯一实例
	*/
	static LoginSessionManager &getInstance()
	{
		if (NULL == instance)
			instance = new LoginSessionManager();

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

	void put(const t_NewLoginSession &session);
	bool verify(const DWORD loginTempID, const DWORD accid,char *numPassword, ZES_cblock *key=0);
	void update(const zRTime &ct);

private:

	/**
	* \brief 会话管理容器唯一实例指针
	*
	*/
	static LoginSessionManager *instance;

	/**
	* \brief 构造函数
	*
	*/
	LoginSessionManager() : lastUpdateTime() {};

	/**
	* \brief 最后一次更新时间
	*
	*/
	zRTime lastUpdateTime;

	/**
	* \brief 检查更新间隔
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
	* \brief 定义容器类型
	*
	*/
	typedef /*__gnu_cxx::*/hash_map<DWORD, LoginSession> LoginSessionHashmap;
	/**
	* \brief 定义迭代器类型
	*
	*/
	typedef LoginSessionHashmap::iterator LoginSessionHashmap_iterator;
	/**
	* \brief 定义键值对类型
	*
	*/
	typedef LoginSessionHashmap::value_type LoginSessionHashmap_pair;
	/**
	* \brief 会话容器
	*
	*/
	LoginSessionHashmap sessionData;
	/**
	* \brief 容器访问互斥变量
	*
	*/
	zMutex mlock;

};

/**
* \brief 网关与档案服务器的连接
*
*/
class RecordClient : public zTCPBufferClient
{

public:

	/**
	* \brief 构造函数
	*
	* \param name 名称
	* \param ip 服务器地址
	* \param port 服务器端口
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
* \brief 定义小游戏服务器连接客户端类
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
	* \brief 获取场景服务器的编号
	*
	* \return 场景服务器编号
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
* \brief 定义计费服务器连接客户端类
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
	* \brief 获取场景服务器的编号
	*
	* \return 场景服务器编号
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
* \brief 网关屏索引
*
*/
class ScreenIndex :private zNoncopyable
{
private:
	//读写锁
	zRWLock wrlock;

	typedef std::set<GateUser *,std::less<GateUser *>/*,__gnu_cxx::__pool_alloc<GateUser *>*/ > SceneEntry_SET;
	typedef /*__gnu_cxx::*/hash_map<DWORD,SceneEntry_SET> PosIMapIndex;
	/**
	* \brief map索引容器
	*/
	PosIMapIndex index;
	SceneEntry_SET all;

	///横向多少屏幕
	const DWORD screenx;
	///纵向多少屏幕
	const DWORD screeny;
	const DWORD screenMax;
	//在加载的时候计算九屏关系并保存
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
	** \brief 构造函数
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
* \brief 网关地图索引
*
*/
typedef std::map<DWORD,ScreenIndex*> MapIndex;
typedef MapIndex::iterator MapIndexIter;

/**
* \brief 定义场景服务器连接客户端类
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
		Zebra::logger->debug("SceneClient析构");
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
	* \brief 网关屏索引
	*
	*
	* \param 
	* \return 
	*/
	MapIndex mapIndex;
	/**
	* \brief 服务器编号
	*
	*/
	WORD wdServerID;



};
#if 0
/**
* \brief 定义场景服务器连接客户端类
*
*/
class SceneClient : public zTCPBufferClient
{

public:

	/**
	* \brief 构造函数
	*
	* \param name 名称
	* \param serverEntry 场景服务器信息
	*/
	SceneClient( const std::string &name,const Cmd::Super::ServerEntry *serverEntry)
		: zTCPBufferClient(name,serverEntry->pstrIP,serverEntry->wdPort,false,8000)
	{
		wdServerID=serverEntry->wdServerID;
	};

	~SceneClient()
	{
		Zebra::logger->debug("SceneClient析构");
	}
	bool connectToSceneServer();
	void run();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

	/**
	* \brief 获取场景服务器的编号
	*
	* \return 场景服务器编号
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
	* \brief 网关屏索引
	*
	*
	* \param 
	* \return 
	*/
	MapIndex mapIndex;
	/**
	* \brief 服务器编号
	*
	*/
	WORD wdServerID;

};

/**
* \brief 场景服务器连接管理器
*
*/
class SceneClientManager
{

public:

	/**
	* \brief 析构函数
	*
	*/
	~SceneClientManager() {};

	/**
	* \brief 获取管理器的唯一实例
	*
	* \return 管理器唯一实例
	*/
	static SceneClientManager &getInstance()
	{
		if (NULL == instance)
			instance = new SceneClientManager();

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

	void add(SceneClient *client);
	void remove(SceneClient *client);
	SceneClient *getByServerID(WORD serverid);
	void final();

private:

	/**
	* \brief 场景服务器连接管理器唯一实例指针
	*
	*/
	static SceneClientManager *instance;

	/**
	* \brief 构造函数
	*
	*/
	SceneClientManager() {};

	/**
	* \brief 容器访问互斥变量
	*
	*/
	zMutex mlock;
	/**
	* \brief 场景服务器连接容器
	*
	*/
	std::vector<SceneClient *> sceneClients;

};

#endif

/**
** \brief 定义服务器信息采集连接的客户端管理容器
**/
class SceneClientManager
{

public:

	~SceneClientManager();

	/**
	** \brief 获取类的唯一实例
	** \return 类的唯一实例引用
	**/
	static SceneClientManager &getInstance()
	{
		if (NULL == instance)
			instance = new SceneClientManager();

		return *instance;
	}

	/**
	** \brief 销毁类的唯一实例
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
	** \brief 客户端连接管理池
	**/
	zTCPClientTaskPool *sceneClientPool;
	/**
	** \brief 进行断线重连检测的时间记录
	**/
	zTime actionTimer;

	/**
	** \brief 存放连接已经成功的连接容器类型
	**/
	typedef std::map<const DWORD,SceneClient *> SceneClient_map;
	typedef SceneClient_map::iterator iter;
	typedef SceneClient_map::const_iterator const_iter;
	typedef SceneClient_map::value_type value_type;
	/**
	** \brief 存放连接已经成功的连接容器
	**/
	SceneClient_map allClients;


	/**
	** \brief 容器访问读写锁
	**/
	zRWLock rwlock;

};
