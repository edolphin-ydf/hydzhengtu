
#include "LogonStdAfx.h"
#include <signal.h>
#include "git_version.h"
#ifndef WIN32
#include <sys/resource.h>
#endif
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi")

#define BANNER "MCodeNet %s %s/%s-%s (%s) :: ��¼������"

#ifndef WIN32
#include <sched.h>
#endif

initialiseSingleton(LogonServer);
MCodeNet::Threading::AtomicBoolean mrunning(true);
Mutex _authSocketLock;
set<AuthSocket*> _authSockets;

/*** Signal Handler ***/
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

/** @brief ���Ĵ������� */
void SetDlgTitle()
{
	char szName[_MAX_PATH*100];
	char szCode[8192];
	GetModuleFileName(NULL,szName,sizeof(szName));
	_snprintf(szCode,sizeof(szCode),"��¼������ - %s",PathFindFileName(szName));
	SetConsoleTitle(szCode);
}

void InitSelfInfo(); 
void AddCacheServer();

int main(int argc, char** argv)
{
	SetDlgTitle();
#ifndef WIN32
	rlimit rl;
	if(getrlimit(RLIMIT_CORE, &rl) == -1)
		printf("getrlimit failed. This could be problem.\n");
	else
	{
		rl.rlim_cur = rl.rlim_max;
		if(setrlimit(RLIMIT_CORE, &rl) == -1)
			printf("setrlimit failed. Server may not save core.dump files.\n");
	}
#endif
	
	LogonServer* logonObject = NULL;
	Macro_NewClass(logonObject,LogonServer);
	// Run!
	LogonServer::getSingleton().Run(argc, argv);
	Macro_DeleteClass(LogonServer::getSingletonPtr(),LogonServer);

	/** @brief �鿴�ڴ��ʹ����� */
	MemPools.GetAndPlayMemoryList();
	MemPools.DisPlayMap();
	MemPools.release();

	getchar();
}

#define DEF_VALUE_NOT_SET 2   /**< 2Ϊ���Եȼ�  */

void LogonServer::Run(int argc, char** argv)
{
	m_stopEvent = false;
	UNIXTIME = time(NULL);
	g_localTime = *localtime(&UNIXTIME);
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
	
	/** @brief ��ʼ������ */
	char* config_file = (char*)CONFDIR "/logon.conf";
	if(!Config.SetSource(config_file))
	{
		LOG_ERROR("Config file could not be rehashed.");
		return;
	}
	/** @brief �������� */ 
	string host = Config.Value("Listen", "Host", "0.0.0.0");
	int cport = Config.Value("Listen", "Port", 8093);

	Log.Success("�̹߳���", "��ʼ����...");
	/** @brief �����̳߳� */
	ThreadPool.Startup();
	ThreadPool.ShowStats();

//	// Spawn periodic function caller thread for account reload every 10mins
//	// �߳����ں���ÿ10�����¼����û����ڴ�
//	int atime = Config.MainConfig.GetIntDefault("Rates", "AccountRefresh", 600);
//	atime *= 1000;
//	PeriodicFunctionCaller<AccountMgr> * pfc = new PeriodicFunctionCaller<AccountMgr>(AccountMgr::getSingletonPtr(), &AccountMgr::ReloadAccountsCallback, atime,"AccountMgr");
//	ThreadPool.ExecuteTask(pfc);//�̳߳�ִ���������

	min_build = LOGON_MINBUILD;
	max_build = LOGON_MAXBUILD;

	SocketMgr* socketObject = NULL;
	Macro_NewClass(socketObject,SocketMgr);
	SocketGarbageCollector* socketGCObject = NULL;
	Macro_NewClass(socketGCObject,SocketGarbageCollector);/**< ��������  */
	
	IntranetManager* intranetObject = NULL;
	Macro_NewClass(intranetObject,IntranetManager);
	InitSelfInfo();
	AddCacheServer();
	sIntranetMgr.Startup();

	ListenSocket<AuthSocket> * cl = new ListenSocket<AuthSocket>(host.c_str(), cport,"AuthSocket");
	ListenSocket<LogonCommServerSocket> * sl = new ListenSocket<LogonCommServerSocket>(host.c_str(), cport,"LogonCommServerSocket");
	/** @brief �����׽��ֹ����߳� */
	sSocketMgr.SpawnWorkerThreads();

	/** @brief ������֤����������� */
	bool authsockcreated = cl->IsOpen();
	bool intersockcreated = sl->IsOpen();
	if(authsockcreated && intersockcreated)
	{
#ifdef WIN32
		ThreadPool.ExecuteTask(cl);
		ThreadPool.ExecuteTask(sl);
#endif
		// hook signals
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
		FILE* fPid = fopen("logonserver.pid", "w");
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
		while(mrunning.GetVal() && !m_stopEvent)
		{
			if(!(++loop_counter % 20))	 /**<  20 seconds */ 
				CheckForDeadSockets();   /**<  ���AuthSocket���������� */

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
/////////////////////////////////��ʼ����///////////////////////////////////////////

	cl->Close();
	sl->Close();
	sSocketMgr.CloseAll();
#ifdef WIN32
	sSocketMgr.ShutdownThreads();
#endif

	ThreadPool.Shutdown();

	// delete pid file
	remove("logonserver.pid");

	Macro_DeleteClass(IntranetManager::getSingletonPtr(),IntranetManager);
	Macro_DeleteClass(SocketMgr::getSingletonPtr(),SocketMgr);
	Macro_DeleteClass(SocketGarbageCollector::getSingletonPtr(),SocketGarbageCollector);
	delete cl;
	delete sl;
	LOG_BASIC("�ر����...���.");
	sLog.Close();
}

void InitSelfInfo()
{
	ServerInfo *info = sIntranetMgr.GetSelfInfo();
	info->Address = Config.Value("Listen", "Host", "0.0.0.0");
	info->Port = Config.Value("Listen","Port");
	info->ID = Config.Value("Listen","Id");
	int aType = Config.Value("Listen","Type");
	info->Type = (EServerType)aType;
	info->Name = ServerName[aType];
	info->ServerID = 0;
	info->RetryTime = 0;
	info->Registered = false;
}

void AddCacheServer()
{
	ServerInfo *info;
	Macro_NewClass(info,ServerInfo);
	info->Address = Config.Value("Listen", "CacheHost");
	info->Port = Config.Value("Listen","CachePort");
	info->Type = EServerType_Cache;
	info->Name = ServerName[info->Type];
	info->ID = 0;
	info->ServerID = 0;
	info->RetryTime = 0;
	info->Registered = false;
	sIntranetMgr.AddConnector(info);
}

void OnCrash(bool Terminate)
{

}

void LogonServer::CheckForDeadSockets()
{
	_authSocketLock.Acquire();
	time_t t = time(NULL);
	time_t diff;
	set<AuthSocket*>::iterator itr = _authSockets.begin();
	set<AuthSocket*>::iterator it2;
	AuthSocket* s;

	for(itr = _authSockets.begin(); itr != _authSockets.end();)
	{
		it2 = itr;
		s = (*it2);
		++itr;

		diff = t - s->GetLastRecv();
		if(diff > 300)		   // More than 5mins
		{
			_authSockets.erase(it2);
			s->removedFromSet = true;
			s->Disconnect();
		}
	}
	_authSocketLock.Release();
}

void LogonServer::Stop()
{
	m_stopEvent = true;
}
