// Cache.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "git_version.h"
#include "r_common.h"
using namespace std;
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi")
#include <signal.h>

#define BANNER "MCodeNet %s %s/%s-%s (%s) :: ���������"
#define DEF_VALUE_NOT_SET 2   /**< 2Ϊ���Եȼ�  */
#define CONFDIR "configs"

Database* sCacheSQL;
RedisClient* sCacheRedis;
pbPsrser* sCachepb;
MCodeNet::Threading::AtomicBoolean mrunning(true);
Mutex _cacheSocketLock;
set<CacheSocket*> _cacheSockets;

/** @brief ���Ĵ������� */
void SetDlgTitle();
int w32initWinSock(void);
/*** Signal Handler ***/
void _OnSignal(int s);
bool startDB();
void CheckForDeadSockets();

int main()
{
	/** @brief ��ʼ�������ļ�����־ϵͳ */
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

	Log.Success("�̹߳���", "��ʼ����...");
	/** @brief �����̳߳� */
	ThreadPool.Startup();
	ThreadPool.ShowStats();

	/** @brief ��ʼ�����ݿ����� */
	if(!startDB())
	{
		ThreadPool.Shutdown();
		return 0;
	}

	/** @brief ��ʼ������redis */
	sCacheRedis = RedisClient::CreateRedisInterface("192.168.1.116");

	/** @brief ����porotbuf */
	sCachepb = pbPsrser::GetPBInterface();

	SocketMgr* socketObject = NULL;
	Macro_NewClass(socketObject,SocketMgr);
	SocketGarbageCollector* socketGCObject = NULL;
	Macro_NewClass(socketGCObject,SocketGarbageCollector);/**< �������ճ�ʼ��  */

	uint32 cport = Config.Value("Listen", "Port", 3724);
	string host = Config.Value("Listen", "Host", "0.0.0.0");
	ListenSocket<CacheSocket> * cl = new ListenSocket<CacheSocket>(host.c_str(), cport,"CacheSocket");
	/** @brief �����׽��ֹ����߳� */
	sSocketMgr.SpawnWorkerThreads();
	/** @brief ������֤����������� */
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
		/** @brief ��ǰ����ID�浽�ı��ļ� */
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
		sLog.outString("Success! �ȴ�����...");

		while(mrunning.GetVal())
		{
			if(!(++loop_counter % 20))	 /**<  20 seconds */ 
				CheckForDeadSockets();   /**<  ���CacheSocket���������� */

			if(!(loop_counter % 300))	/**< 5mins  */ 
				ThreadPool.IntegrityCheck();/**< �̳߳��߳�����ѹ�����  */

			if(!(loop_counter % 5))
			{
				//sInfoCore.TimeoutSockets();/**< ���LogonCommServerSocket��ʱ������  */
				sSocketGarbageCollector.Update();
				CheckForDeadSockets();			  /**<  Flood Protection */ 
				UNIXTIME = time(NULL);
				g_localTime = *localtime(&UNIXTIME);
			}

			MCodeNet::Sleep(1000);
		}

		sLog.outString("��ʼ�ر����...");
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
	///////////////////////////////��ʼ����///////////////////////////////////////////
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
	LOG_BASIC("�ر����...���.");
	sLog.Close();

	/** @brief �鿴�ڴ��ʹ����� */
	MemPools.GetAndPlayMemoryList();
	MemPools.DisPlayMap();
	MemPools.release();
	return 0;
}

/** @brief ���Ĵ������� */
void SetDlgTitle()
{
	char szName[_MAX_PATH*100];
	char szCode[8192];
	GetModuleFileName(NULL,szName,sizeof(szName));
	_snprintf(szCode,sizeof(szCode),"��������� - %s",PathFindFileName(szName));
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