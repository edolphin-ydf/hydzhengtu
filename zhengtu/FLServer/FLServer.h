/**
* \brief zebra项目登陆服务器,负责登陆,建立帐号、档案等功能
*
*/
#include <zebra/srvEngine.h>
//#include <iterator>

class FLTimeTick : public zThread
{
public:

	~FLTimeTick() {};

	static FLTimeTick &getInstance()
	{
		if (NULL == instance)
			instance = new FLTimeTick();

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

	static FLTimeTick *instance;

	FLTimeTick() : zThread("TimeTick")
	{
	}

};

/**
* \brief 定义登陆服务类
*
* 登陆服务,负责登陆,建立帐号、档案等功能<br>
* 这个类使用了Singleton设计模式,保证了一个进程中只有一个类的实例
*
*/
class FLService : public zMNetService
{

public:

	/**
	* \brief 虚析构函数
	*
	*/
	~FLService()
	{
		instance = NULL;

		if (loginTaskPool)
		{
			loginTaskPool->final();
			SAFE_DELETE(loginTaskPool);
		}

		if (serverTaskPool)
		{
			serverTaskPool->final();
			SAFE_DELETE(serverTaskPool);
		}

		if (pingTaskPool)
		{
			pingTaskPool->final();
			SAFE_DELETE(pingTaskPool);
		}
	}

	/**
	* \brief 返回唯一的类实例
	*
	* \return 唯一的类实例
	*/
	static FLService &getInstance()
	{
		if (NULL == instance)
			instance = new FLService();

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

	/**
	* \brief 获取连接池中的连接数
	* \return 连接数
	*/
	const int getPoolSize() const
	{
		return loginTaskPool->getSize();
	}

	/**
	* \brief 获取服务器类型
	* \return 服务器类型
	*/
	const WORD getType() const
	{
		return LOGINSERVER;
	}

	void reloadConfig();

	bool jpeg_passport;

	static zDBConnPool *dbConnPool;

private:

	/**
	* \brief 类的唯一实例指针
	*
	*/
	static FLService *instance;

	/**
	* \brief 构造函数
	*
	*/
	FLService() : zMNetService("登陆服务器")
	{
		jpeg_passport  = false;
		login_port     = 0;
		inside_port    = 0;
		ping_port      = 0;
		loginTaskPool  = NULL;
		serverTaskPool = NULL;
		pingTaskPool   = NULL;
	}

	bool init();
	void newTCPTask(const SOCKET sock,const WORD srcPort);
	void final();

	WORD login_port;
	WORD inside_port;
	WORD ping_port;

	zTCPTaskPool *loginTaskPool;
	zTCPTaskPool *serverTaskPool;
	zTCPTaskPool *pingTaskPool;
};

/**
* \brief 存储有效服务器的列表
* 有效服务器列表存储在xml文件中,服务器启动的时候读取这些信息到内存,
* 当一个管理服务器连接过来的时候,可以根据这些信息判断这个连接是否合法的。
*/

struct ACLZone
{
	GameZone_t gameZone;
	std::string ip;
	WORD port;
	std::string name;
	std::string desc;

	ACLZone()
	{
		port = 0;
	}
	ACLZone(const ACLZone &acl)
	{
		gameZone = acl.gameZone;
		ip = acl.ip;
		port = acl.port;
		name = acl.name;
		desc = acl.desc;
	}
};

class ServerACL : zNoncopyable
{

public:

	ServerACL() {};
	~ServerACL() {};

	bool init();
	void final();
	bool check(const char *strIP,const WORD port,GameZone_t &gameZone,std::string &name);

private:

	bool add(const ACLZone &zone);



	/*struct less_str : public std::less<GameZone_t>
	{

	bool operator()(const GameZone_t & x, const GameZone_t & y) const 
	{
	if (x.id < y.id )
	return true;

	return false;
	}
	};*/


	/**
	* \brief hash函数
	*
	*/
	/*struct GameZone_hash :public hash_compare<GameZone_t,less_str>
	{
	size_t operator()(const GameZone_t &gameZone) const
	{
	//hash<DWORD> H;
	return 1;//Hash<DWORD>(gameZone.id);
	}

	//static const unsigned int bucket_size = 100;
	//static const unsigned int min_buckets = 100;
	};*/


	typedef hash_map<const GameZone_t,ACLZone> Container;
	Container datas;
	zRWLock rwlock;

};

typedef singleton_default<ServerACL> ServerACLSingleton;

using namespace Cmd;//add by Victor

/**
* \brief 服务器连接任务
*
*/
class LoginTask : public zTCPTask
{

public:
	DWORD old;
	LoginTask( zTCPTaskPool *pool,const SOCKET sock);
	/**
	* \brief 虚析构函数
	*
	*/
	~LoginTask() {
	};

	int verifyConn();
	int recycleConn();
	void addToContainer();
	bool uniqueAdd();
	bool uniqueRemove();
	bool msgParse(const Cmd::t_NullCmd *,const DWORD);

	void genTempID()
	{
		tempid = ++uniqueID;
	}

	const DWORD getTempID() const
	{
		return tempid;
	}

	/**
	* \brief 登陆错误,返回错误代码到客户端显示
	*
	* \param retcode 错误代码
	* \param tm 是否断开连接
	*/
	void LoginReturn(const BYTE retcode,const bool tm = true)
	{
		using namespace Cmd;
		stServerReturnLoginFailedCmd tCmd;

		tCmd.byReturnCode = retcode;
		sendCmd(&tCmd,sizeof(tCmd));

		//由于登陆错误,需要断开连接
		//whj 可能导致coredown,屏蔽测试
		if (tm) Terminate();
	}

	/**
	* \brief 判断登陆连接是否过长
	* 如果登陆连接太长,登陆服务器应该主动断开连接
	* \param ct 当前时间
	* \return 登陆时间是否过长
	*/
	bool timeout(const zTime &ct)
	{
		if (lifeTime.elapse(ct) >= 600)
			return true;
		else
			return false;
	}

private:

	/**
	* \brief 校验客户端版本号
	*/
	DWORD verify_client_version;

	/**
	* \brief 生命期时间
	*/
	zTime lifeTime;
	/**
	* \brief 临时唯一编号
	*
	*/
	DWORD tempid;
	/**
	* \brief 临时唯一编号分配器
	*
	*/
	static DWORD uniqueID;
	/**
	* \brief 验证码
	*
	*/
	char jpegPassport[5];

	bool requestLogin(const Cmd::stUserRequestLoginCmd *ptCmd);

};

/**
* \brief 登陆连接管理容器
*
* 管理所有的登陆连接的容器,方便查找连接
*
*/
class LoginManager
{

public:

	/**
	** \brief 网关最大容纳用户数目
	**
	**/
	static DWORD maxGatewayUser;

	/**
	* \brief 定义回调函数类
	*
	*/
	typedef zEntryCallback<LoginTask,void> LoginTaskCallback;

	/**
	* \brief 析构函数
	*
	*/
	~LoginManager() {};

	/**
	* \brief 获取管理容器的唯一实例
	*
	* 容器实现了Singleton设计模式,保证了一个进程中只有一个类的实例
	*/
	static LoginManager &getInstance()
	{
		if (NULL == instance)
			instance = new LoginManager();

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

	bool add(LoginTask *task);
	void remove(LoginTask *task);
	bool broadcast(const DWORD loginTempID,const void *pstrCmd,int nCmdLen);
	void verifyReturn(const DWORD loginTempID,const BYTE retcode,const t_NewLoginSession &session);
	void loginReturn(const DWORD loginTempID,const BYTE retcode,const bool tm = true);
	void execAll(LoginTaskCallback &cb);

private:

	/**
	* \brief 构造函数
	*
	*/
	LoginManager(){};

	/**
	* \brief 类的唯一实例指针
	*
	*/
	static LoginManager *instance;
	/**
	* \brief 定义容器类型
	*
	*/
	typedef hash_map<DWORD,LoginTask *> LoginTaskHashmap;
	/**
	* \brief 定义容器迭代器类型
	*
	*/
	typedef LoginTaskHashmap::iterator LoginTaskHashmap_iterator;
	/**
	* \brief 定义容器常量迭代器类型
	*
	*/
	typedef LoginTaskHashmap::const_iterator LoginTaskHashmap_const_iterator;
	/**
	* \brief 定义容器键值对类型
	*
	*/
	typedef LoginTaskHashmap::value_type LoginTaskHashmap_pair;
	/**
	* \brief 互斥变量,保证原子访问容器
	*
	*/
	zMutex mlock;
	/**
	* \brief 子连接管理容器类型
	*
	*/
	LoginTaskHashmap loginTaskSet;

};

/**
* \brief 服务器连接任务
*/
class ServerTask : public zTCPTask
{

public:
	DWORD old;
	/**
	* \brief 构造函数
	* 用于创建一个服务器连接任务
	* \param pool 所属的连接池
	* \param sock TCP/IP套接口
	*/
	ServerTask(
		zTCPTaskPool *pool,
		const SOCKET sock) : zTCPTask(pool,sock,NULL)
	{
	}

	/**
	* \brief 虚析构函数
	*/
	~ServerTask() {
	};

	int verifyConn();
	int waitSync();
	bool uniqueAdd();
	bool uniqueRemove();
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

	/*void setZoneID(const GameZone_t &gameZone)
	{
	this->gameZone = gameZone;
	}*/

	const GameZone_t &getZoneID() const
	{
		return gameZone;
	}

private:

	GameZone_t gameZone;
	std::string name;

	bool msgParse_gyList(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
	bool msgParse_session(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

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
	* 这个类使用了Singleton设计模式,保证了一个进程中只有一个类的实例
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

	bool uniqueAdd(ServerTask *task);
	bool uniqueRemove(ServerTask *task);
	bool broadcast(const GameZone_t &gameZone,const void *pstrCmd,int nCmdLen);

private:

	/**
	* \brief 构造函数
	*
	*/
	ServerManager() {};

	/**
	* \brief 类的唯一实例指针
	*
	*/
	static ServerManager *instance;


	/*struct less_str 
	{

	bool operator()(const GameZone_t & x, const GameZone_t & y) const 
	{
	if (x.id < y.id )
	return true;

	return false;
	}
	};*/

	/**
	* \brief hash函数
	*
	*/
	/*struct GameZone_hash : public hash_compare<GameZone_t,less_str>
	{
	size_t operator()(const GameZone_t &gameZone) const
	{
	//__gnu_cxx::hash<DWORD> H;
	//return H(gameZone.id);
	//return Hash<DWORD>(gameZone.id);
	return 1;
	}

	};*/
	/**
	* \brief 定义了服务器的唯一性验证容器类型
	* 
	**/
	typedef hash_map<const GameZone_t,ServerTask *> ServerTaskContainer;
	/**
	* \brief 定义容器的迭代器类型
	*
	*/
	typedef ServerTaskContainer::iterator ServerTaskContainer_iterator;
	/**
	* \brief 定义了容器的常量迭代器类型
	*
	*/
	typedef ServerTaskContainer::const_iterator ServerTaskContainer_const_iterator;
	/**
	* \brief 定义了容器的键值对类型
	*
	*/
	typedef ServerTaskContainer::value_type ServerTaskContainer_value_type;
	/**
	* \brief 容器访问的互斥变量
	*
	*/
	zMutex mlock;
	/**
	* \brief 唯一性容器实例
	*
	*/
	ServerTaskContainer taskUniqueContainer;

};

/**
* \brief 网关信息节点
*
*/
struct GYList
{
	WORD wdServerID;      /**< 服务器编号 */
	char pstrIP[MAX_IP_LENGTH];  /**< 服务器地址 */
	WORD wdPort;        /**< 服务器端口 */
	WORD wdNumOnline;      /**< 网关在线人数 */
	int  state;          /**< 服务器状态 */
	DWORD zoneGameVersion;

	/**
	* \brief 缺省构造函数
	*
	*/
	GYList()
	{
		wdServerID = 0;
		bzero(pstrIP,sizeof(pstrIP));
		wdPort = 0;
		wdNumOnline = 0;
		state = state_none;
		zoneGameVersion = 0;
	}

	/**
	* \brief 拷贝构造函数
	*
	*/
	GYList(const GYList& gy)
	{
		wdServerID = gy.wdServerID;
		bcopy(gy.pstrIP,pstrIP,sizeof(pstrIP),sizeof(pstrIP));
		wdPort = gy.wdPort;
		wdNumOnline = gy.wdNumOnline;
		state = gy.state;
		zoneGameVersion = gy.zoneGameVersion;
	}

	/**
	* \brief 赋值函数
	*
	*/
	GYList & operator= (const GYList &gy)
	{
		wdServerID = gy.wdServerID;
		bcopy(gy.pstrIP,pstrIP,sizeof(pstrIP),sizeof(pstrIP));
		wdPort = gy.wdPort;
		wdNumOnline = gy.wdNumOnline;
		state = gy.state;
		zoneGameVersion = gy.zoneGameVersion;
		return *this;
	}

};

/**
* \brief 网关信息列表管理器
*
*/
class GYListManager
{

public:

	/**
	* \brief 默认析构函数
	*
	*/
	~GYListManager()
	{
		gyData.clear();
	}

	/**
	* \brief 返回类的唯一实例
	*
	* 实现了Singleton设计模式，保证了一个进程中只有一个类的实例
	*
	*/
	static GYListManager &getInstance()
	{
		if (NULL == instance)
			instance = new GYListManager;

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

	bool put(const GameZone_t &gameZone,const GYList &gy);
	void disableAll(const GameZone_t &gameZone);
	GYList *getAvl(const GameZone_t &gameZone);
	void full_ping_list(Cmd::stPingList* cmd,const GameZone_t& gameZone);
	bool verifyVer(const GameZone_t &gameZone,DWORD verify_client_version,BYTE &retcode);

	DWORD getOnline(void);

private:

	/**
	* \brief 类的唯一实例指针
	*
	*/
	static GYListManager *instance;

	/**
	* \brief 默认构造函数
	*
	*/
	GYListManager() {};

	/*struct less_str 
	{

	bool operator()(const GameZone_t & x, const GameZone_t & y) const 
	{
	if (x.id < y.id )
	return true;

	return false;
	}
	};*/

	/**
	* \brief hash函数
	*
	*/
	/*struct GameZone_hash : public hash_compare<GameZone_t,less_str>
	{
	size_t operator()(const GameZone_t &gameZone) const
	{
	//Hash<DWORD> H;
	//return Hash<DWORD>(gameZone.id);
	return 1;
	}

	//static const unsigned int bucket_size = 100;
	//static const unsigned int min_buckets = 100;
	};*/
	/**
	* \brief 定义容器类型
	*
	*/
	typedef hash_multimap<const GameZone_t,GYList> GYListContainer;
	/**
	* \brief 定义容器迭代器类型
	*
	*/
	typedef GYListContainer::iterator GYListContainer_iterator;
	/**
	* \brief 定义容器键值对类型
	*
	*/
	typedef GYListContainer::value_type GYListContainer_value_type;
	/**
	* \brief 存储网关列表信息的容器
	*
	*/
	GYListContainer gyData;
	/**
	* \brief 互斥变量
	*
	*/
	zMutex mlock;

};

/**
* \brief 服务器连接任务
*
*/
class PingTask : public zTCPTask
{

public:

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param sock TCP/IP套接口
	*/
	PingTask(
		zTCPTaskPool *pool,
		const SOCKET sock) : zTCPTask(pool,sock,NULL,true,false)
	{
		/*
		pSocket->enc.setEncMethod(CEncrypt::ENCDEC_RC5);
		//pSocket->enc.set_key_rc5((const BYTE *)Zebra::global["rc5_key"].c_str(),16,12);
		BYTE key[16] = {28,196,25,36,193,125,86,197,35,92,194,41,31,240,37,223};
		pSocket->enc.set_key_rc5((const BYTE *)key,16,12);
		*/
	}

	/**
	* \brief 虚析构函数
	*
	*/
	~PingTask() {};

	int verifyConn();
	int recycleConn();
	//  void addToContainer();
	//  bool uniqueAdd();
	//  bool uniqueRemove();
	bool msgParse(const Cmd::t_NullCmd* ptNull,const DWORD);
private:
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
	bool broadcast(const void *pstrCmd, int nCmdLen);

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
	typedef std::vector<InfoClient *> InfoClientContainer;
	/**
	* \brief 存放连接已经成功的连接容器
	*/
	InfoClientContainer allClients;
	/**
	* \brief 容器访问互斥变量
	*/
	zMutex mlock;

};
