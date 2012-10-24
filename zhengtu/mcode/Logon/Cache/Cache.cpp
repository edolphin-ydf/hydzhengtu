// Cache.cpp : 定义控制台应用程序的入口点。
//
#include "git_version.h"
#include "r_common.h"
using namespace std;
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi")
#include <signal.h>

#define BANNER "MCodeNet %s %s/%s-%s (%s) :: 缓存服务器"
#define DEF_VALUE_NOT_SET 2   /**< 2为调试等级  */
#define CONFDIR "configs"

Database* sCacheSQL;
RedisClient* sCacheRedis;
pbPsrser* sCachepb;
MCodeNet::Threading::AtomicBoolean mrunning(true);
Mutex _cacheSocketLock;
set<CacheSocket*> _cacheSockets;

/** @brief 更改窗口名称 */
void SetDlgTitle();
int w32initWinSock(void);
/*** Signal Handler ***/
void _OnSignal(int s);
bool startDB();
void CheckForDeadSockets();

int main()
{
	/** @brief 初始化配置文件，日志系统 */
	SetDlgTitle();
	UNIXTIME = time(NULL);
	g_localTime = *localtime(&UNIXTIME);
	char* config_file = (char*)CONFDIR "/cache.conf";
	Config.SetSource(config_file);
	int file_log_level = DEF_VALUE_NOT_SET;
	int screen_log_level = DEF_VALUE_NOT_SET;
	int do_check_conf = 0;

	sLog.Init(0, LOGON_LOG);
	sLog.outBasic(BANNER, BUILD_TAG, BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
	sLog.outError(BANNER, BUILD_TAG, BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
	/* set new log levels */
	sLog.SetFileLoggingLevel(file_log_level);
	printf("The key combination <Ctrl-C> will safely shut down the server at any time.");
	Log.Success("System", "Initializing Random Number Generators...");
	Log.Success("Config", "Loading Config Files...");


	if (!w32initWinSock()) {
		sLog.outError("Winsock init error %d", WSAGetLastError());
		exit(1);
	};

	Log.Success("线程管理", "开始启动...");
	/** @brief 启动线程池 */
	ThreadPool.Startup();
	ThreadPool.ShowStats();

	/** @brief 初始化数据库连接 */
	if(!startDB())
	{
		ThreadPool.Shutdown();
		return 0;
	}

	/** @brief 初始化连接redis */
	sCacheRedis = RedisClient::CreateRedisInterface("192.168.1.116");

	/** @brief 测试porotbuf */
	sCachepb = pbPsrser::GetPBInterface();

	SocketMgr* socketObject = NULL;
	Macro_NewClass(socketObject,SocketMgr);
	SocketGarbageCollector* socketGCObject = NULL;
	Macro_NewClass(socketGCObject,SocketGarbageCollector);/**< 垃圾回收初始化  */

	uint32 cport = Config.Value("Listen", "Port", 3724);
	string host = Config.Value("Listen", "Host", "0.0.0.0");
	ListenSocket<CacheSocket> * cl = new ListenSocket<CacheSocket>(host.c_str(), cport,"CacheSocket");
	/** @brief 生成套接字工作线程 */
	sSocketMgr.SpawnWorkerThreads();
	/** @brief 生成验证的网络监听者 */
	bool sockcreated = cl->IsOpen();
	if (sockcreated)
	{
#ifdef WIN32
		ThreadPool.ExecuteTask(cl);
#endif
		sLog.outString("Hooking signals...");
		signal(SIGINT, _OnSignal);
		signal(SIGTERM, _OnSignal);
		signal(SIGABRT, _OnSignal);
#ifdef _WIN32
		signal(SIGBREAK, _OnSignal);
#else
		signal(SIGHUP, _OnSignal);
#endif
		/** @brief 当前进程ID存到文本文件 */
		FILE* fPid = fopen("cacheserver.pid", "w");
		if(fPid)
		{
			uint32 pid;
#ifdef WIN32
			pid = GetCurrentProcessId();
#else
			pid = getpid();
#endif
			fprintf(fPid, "%u", (unsigned int)pid);
			fclose(fPid);
		}
		uint32 loop_counter = 0;
		ThreadPool.Gobble();
		sLog.outString("Success! 等待连接...");

		while(mrunning.GetVal())
		{
			if(!(++loop_counter % 20))	 /**<  20 seconds */ 
				CheckForDeadSockets();   /**<  检查CacheSocket死掉的连接 */

			if(!(loop_counter % 300))	/**< 5mins  */ 
				ThreadPool.IntegrityCheck();/**< 线程池线程数和压力检查  */

			if(!(loop_counter % 5))
			{
				//sInfoCore.TimeoutSockets();/**< 检查LogonCommServerSocket超时的连接  */
				sSocketGarbageCollector.Update();
				CheckForDeadSockets();			  /**<  Flood Protection */ 
				UNIXTIME = time(NULL);
				g_localTime = *localtime(&UNIXTIME);
			}

			MCodeNet::Sleep(1000);
		}

		sLog.outString("开始关闭清空...");
		signal(SIGINT, 0);
		signal(SIGTERM, 0);
		signal(SIGABRT, 0);
#ifdef _WIN32
		signal(SIGBREAK, 0);
#else
		signal(SIGHUP, 0);
#endif
	}
	else
	{
		LOG_ERROR("Error creating sockets. Shutting down...");
	}
	///////////////////////////////开始回收///////////////////////////////////////////
	cl->Close();

	sSocketMgr.CloseAll();
#ifdef WIN32
	sSocketMgr.ShutdownThreads();
#endif

	Macro_DeleteClass(SocketMgr::getSingletonPtr(),SocketMgr);
	Macro_DeleteClass(SocketGarbageCollector::getSingletonPtr(),SocketGarbageCollector);

	ThreadPool.Shutdown();
	delete sCacheRedis;
	delete sCacheSQL;
	delete sCachepb;
	delete cl;
	atexit((void(*)(void)) WSACleanup);
	LOG_BASIC("关闭清空...完成.");
	sLog.Close();

	/** @brief 查看内存池使用情况 */
	MemPools.GetAndPlayMemoryList();
	MemPools.DisPlayMap();
	MemPools.release();
	return 0;
}

/** @brief 更改窗口名称 */
void SetDlgTitle()
{
	char szName[_MAX_PATH*100];
	char szCode[8192];
	GetModuleFileName(NULL,szName,sizeof(szName));
	_snprintf(szCode,sizeof(szCode),"缓存服务器 - %s",PathFindFileName(szName));
	SetConsoleTitle(szCode);
}

int w32initWinSock(void) 
{

	WSADATA t_wsa;
	WORD wVers;
	int iError;

	wVers = MAKEWORD(2, 2);
	iError = WSAStartup(wVers, &t_wsa);

	if(iError != NO_ERROR || LOBYTE(t_wsa.wVersion) != 2 || HIBYTE(t_wsa.wVersion) != 2 ) {
		return 0; /* not done; check WSAGetLastError() for error number */
	};

	return 1;
}

//*** Signal Handler ***/
void _OnSignal(int s)
{
	switch(s)
	{
#ifndef WIN32
	case SIGHUP:
		{
			LOG_DETAIL("Received SIGHUP signal, reloading accounts.");
			AccountMgr::getSingleton().ReloadAccounts(true);
		}
		break;
#endif
	case SIGINT:
	case SIGTERM:
	case SIGABRT:
#ifdef _WIN32
	case SIGBREAK:
#endif
		mrunning.SetVal(false);
		break;
	}

	signal(s, _OnSignal);
}


bool startDB()
{
	string lhostname, lusername, lpassword, ldatabase;
	int lport = 0;

	bool result;

	lusername = Config.Value("CacheDatabase", "Username");
	lpassword = Config.Value("CacheDatabase", "Password");
	lhostname = Config.Value("CacheDatabase", "Hostname");
	ldatabase     = Config.Value("CacheDatabase", "Name");
	lport     = Config.Value("CacheDatabase", "Port");

	result = lusername.length() && lpassword.length() && lhostname.length() && ldatabase.length() && lport;
	if (!result)
		return false;


	sCacheSQL = Database::CreateDatabaseInterface();

	if(!sCacheSQL->Initialize(lhostname.c_str(), (unsigned int)lport, lusername.c_str(),
		lpassword.c_str(), ldatabase.c_str(), Config.Value("CacheDatabase", "ConnectionCount", 5),
		16384))
	{
		LOG_ERROR("sql: Logon database initialization failed. Exiting.");
		return false;
	}
	return true;
}

void CheckForDeadSockets()
{
	_cacheSocketLock.Acquire();
	time_t t = time(NULL);
	time_t diff;
	set<CacheSocket*>::iterator itr = _cacheSockets.begin();
	set<CacheSocket*>::iterator it2;
	CacheSocket* s;

	for(itr = _cacheSockets.begin(); itr != _cacheSockets.end();)
	{
		it2 = itr;
		s = (*it2);
		++itr;

		diff = t - s->GetLastRecv();
		if(diff > 300)		   // More than 5mins
		{
			_cacheSockets.erase(it2);
			s->removedFromSet = true;
			s->Disconnect();
		}
	}
	_cacheSocketLock.Release();
}