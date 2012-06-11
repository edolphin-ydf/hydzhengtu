/**
* \brief zebra��Ŀ��½������,�����½,�����ʺš������ȹ���
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
	* \brief �ͷ����Ψһʵ��
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
* \brief �����½������
*
* ��½����,�����½,�����ʺš������ȹ���<br>
* �����ʹ����Singleton���ģʽ,��֤��һ��������ֻ��һ�����ʵ��
*
*/
class FLService : public zMNetService
{

public:

	/**
	* \brief ����������
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
	* \brief ����Ψһ����ʵ��
	*
	* \return Ψһ����ʵ��
	*/
	static FLService &getInstance()
	{
		if (NULL == instance)
			instance = new FLService();

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

	/**
	* \brief ��ȡ���ӳ��е�������
	* \return ������
	*/
	const int getPoolSize() const
	{
		return loginTaskPool->getSize();
	}

	/**
	* \brief ��ȡ����������
	* \return ����������
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
	* \brief ���Ψһʵ��ָ��
	*
	*/
	static FLService *instance;

	/**
	* \brief ���캯��
	*
	*/
	FLService() : zMNetService("��½������")
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
* \brief �洢��Ч���������б�
* ��Ч�������б�洢��xml�ļ���,������������ʱ���ȡ��Щ��Ϣ���ڴ�,
* ��һ��������������ӹ�����ʱ��,���Ը�����Щ��Ϣ�ж���������Ƿ�Ϸ��ġ�
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
	* \brief hash����
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
* \brief ��������������
*
*/
class LoginTask : public zTCPTask
{

public:
	DWORD old;
	LoginTask( zTCPTaskPool *pool,const SOCKET sock);
	/**
	* \brief ����������
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
	* \brief ��½����,���ش�����뵽�ͻ�����ʾ
	*
	* \param retcode �������
	* \param tm �Ƿ�Ͽ�����
	*/
	void LoginReturn(const BYTE retcode,const bool tm = true)
	{
		using namespace Cmd;
		stServerReturnLoginFailedCmd tCmd;

		tCmd.byReturnCode = retcode;
		sendCmd(&tCmd,sizeof(tCmd));

		//���ڵ�½����,��Ҫ�Ͽ�����
		//whj ���ܵ���coredown,���β���
		if (tm) Terminate();
	}

	/**
	* \brief �жϵ�½�����Ƿ����
	* �����½����̫��,��½������Ӧ�������Ͽ�����
	* \param ct ��ǰʱ��
	* \return ��½ʱ���Ƿ����
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
	* \brief У��ͻ��˰汾��
	*/
	DWORD verify_client_version;

	/**
	* \brief ������ʱ��
	*/
	zTime lifeTime;
	/**
	* \brief ��ʱΨһ���
	*
	*/
	DWORD tempid;
	/**
	* \brief ��ʱΨһ��ŷ�����
	*
	*/
	static DWORD uniqueID;
	/**
	* \brief ��֤��
	*
	*/
	char jpegPassport[5];

	bool requestLogin(const Cmd::stUserRequestLoginCmd *ptCmd);

};

/**
* \brief ��½���ӹ�������
*
* �������еĵ�½���ӵ�����,�����������
*
*/
class LoginManager
{

public:

	/**
	** \brief ������������û���Ŀ
	**
	**/
	static DWORD maxGatewayUser;

	/**
	* \brief ����ص�������
	*
	*/
	typedef zEntryCallback<LoginTask,void> LoginTaskCallback;

	/**
	* \brief ��������
	*
	*/
	~LoginManager() {};

	/**
	* \brief ��ȡ����������Ψһʵ��
	*
	* ����ʵ����Singleton���ģʽ,��֤��һ��������ֻ��һ�����ʵ��
	*/
	static LoginManager &getInstance()
	{
		if (NULL == instance)
			instance = new LoginManager();

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

	bool add(LoginTask *task);
	void remove(LoginTask *task);
	bool broadcast(const DWORD loginTempID,const void *pstrCmd,int nCmdLen);
	void verifyReturn(const DWORD loginTempID,const BYTE retcode,const t_NewLoginSession &session);
	void loginReturn(const DWORD loginTempID,const BYTE retcode,const bool tm = true);
	void execAll(LoginTaskCallback &cb);

private:

	/**
	* \brief ���캯��
	*
	*/
	LoginManager(){};

	/**
	* \brief ���Ψһʵ��ָ��
	*
	*/
	static LoginManager *instance;
	/**
	* \brief ������������
	*
	*/
	typedef hash_map<DWORD,LoginTask *> LoginTaskHashmap;
	/**
	* \brief ������������������
	*
	*/
	typedef LoginTaskHashmap::iterator LoginTaskHashmap_iterator;
	/**
	* \brief ����������������������
	*
	*/
	typedef LoginTaskHashmap::const_iterator LoginTaskHashmap_const_iterator;
	/**
	* \brief ����������ֵ������
	*
	*/
	typedef LoginTaskHashmap::value_type LoginTaskHashmap_pair;
	/**
	* \brief �������,��֤ԭ�ӷ�������
	*
	*/
	zMutex mlock;
	/**
	* \brief �����ӹ�����������
	*
	*/
	LoginTaskHashmap loginTaskSet;

};

/**
* \brief ��������������
*/
class ServerTask : public zTCPTask
{

public:
	DWORD old;
	/**
	* \brief ���캯��
	* ���ڴ���һ����������������
	* \param pool ���������ӳ�
	* \param sock TCP/IP�׽ӿ�
	*/
	ServerTask(
		zTCPTaskPool *pool,
		const SOCKET sock) : zTCPTask(pool,sock,NULL)
	{
	}

	/**
	* \brief ����������
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
* \brief ����������������
*
* �����������ȫ��������Ψһ����֤����
*
*/
class ServerManager : zNoncopyable
{

public:

	/**
	* \brief ȱʡ��������
	*
	*/
	~ServerManager() {};

	/**
	* \brief ��ȡ���Ψһʵ��
	*
	* �����ʹ����Singleton���ģʽ,��֤��һ��������ֻ��һ�����ʵ��
	*
	*/
	static ServerManager &getInstance()
	{
		if (NULL == instance)
			instance = new ServerManager();

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

	bool uniqueAdd(ServerTask *task);
	bool uniqueRemove(ServerTask *task);
	bool broadcast(const GameZone_t &gameZone,const void *pstrCmd,int nCmdLen);

private:

	/**
	* \brief ���캯��
	*
	*/
	ServerManager() {};

	/**
	* \brief ���Ψһʵ��ָ��
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
	* \brief hash����
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
	* \brief �����˷�������Ψһ����֤��������
	* 
	**/
	typedef hash_map<const GameZone_t,ServerTask *> ServerTaskContainer;
	/**
	* \brief ���������ĵ���������
	*
	*/
	typedef ServerTaskContainer::iterator ServerTaskContainer_iterator;
	/**
	* \brief �����������ĳ�������������
	*
	*/
	typedef ServerTaskContainer::const_iterator ServerTaskContainer_const_iterator;
	/**
	* \brief �����������ļ�ֵ������
	*
	*/
	typedef ServerTaskContainer::value_type ServerTaskContainer_value_type;
	/**
	* \brief �������ʵĻ������
	*
	*/
	zMutex mlock;
	/**
	* \brief Ψһ������ʵ��
	*
	*/
	ServerTaskContainer taskUniqueContainer;

};

/**
* \brief ������Ϣ�ڵ�
*
*/
struct GYList
{
	WORD wdServerID;      /**< ��������� */
	char pstrIP[MAX_IP_LENGTH];  /**< ��������ַ */
	WORD wdPort;        /**< �������˿� */
	WORD wdNumOnline;      /**< ������������ */
	int  state;          /**< ������״̬ */
	DWORD zoneGameVersion;

	/**
	* \brief ȱʡ���캯��
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
	* \brief �������캯��
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
	* \brief ��ֵ����
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
* \brief ������Ϣ�б������
*
*/
class GYListManager
{

public:

	/**
	* \brief Ĭ����������
	*
	*/
	~GYListManager()
	{
		gyData.clear();
	}

	/**
	* \brief �������Ψһʵ��
	*
	* ʵ����Singleton���ģʽ����֤��һ��������ֻ��һ�����ʵ��
	*
	*/
	static GYListManager &getInstance()
	{
		if (NULL == instance)
			instance = new GYListManager;

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

	bool put(const GameZone_t &gameZone,const GYList &gy);
	void disableAll(const GameZone_t &gameZone);
	GYList *getAvl(const GameZone_t &gameZone);
	void full_ping_list(Cmd::stPingList* cmd,const GameZone_t& gameZone);
	bool verifyVer(const GameZone_t &gameZone,DWORD verify_client_version,BYTE &retcode);

	DWORD getOnline(void);

private:

	/**
	* \brief ���Ψһʵ��ָ��
	*
	*/
	static GYListManager *instance;

	/**
	* \brief Ĭ�Ϲ��캯��
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
	* \brief hash����
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
	* \brief ������������
	*
	*/
	typedef hash_multimap<const GameZone_t,GYList> GYListContainer;
	/**
	* \brief ������������������
	*
	*/
	typedef GYListContainer::iterator GYListContainer_iterator;
	/**
	* \brief ����������ֵ������
	*
	*/
	typedef GYListContainer::value_type GYListContainer_value_type;
	/**
	* \brief �洢�����б���Ϣ������
	*
	*/
	GYListContainer gyData;
	/**
	* \brief �������
	*
	*/
	zMutex mlock;

};

/**
* \brief ��������������
*
*/
class PingTask : public zTCPTask
{

public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param sock TCP/IP�׽ӿ�
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
	* \brief ����������
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
* \brief ��Ϣ�ռ��ͻ���������
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
* \brief �����������Ϣ�ɼ����ӵĿͻ��˹�������
*/
class InfoClientManager
{

public:

	~InfoClientManager();

	/**
	* \brief ��ȡ���Ψһʵ��
	* \return ���Ψһʵ������
	*/
	static InfoClientManager &getInstance()
	{
		if (NULL == instance)
			instance = new InfoClientManager();

		return *instance;
	}

	/**
	* \brief �������Ψһʵ��
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
	* \brief �ͻ������ӹ����
	*/
	zTCPClientTaskPool *infoClientPool;
	/**
	* \brief ���ж�����������ʱ���¼
	*/
	zTime actionTimer;

	/**
	* \brief ��������Ѿ��ɹ���������������
	*/
	typedef std::vector<InfoClient *> InfoClientContainer;
	/**
	* \brief ��������Ѿ��ɹ�����������
	*/
	InfoClientContainer allClients;
	/**
	* \brief �������ʻ������
	*/
	zMutex mlock;

};
