
#include "LogonStdAfx.h"
#include <signal.h>
#include "git_version.h"
#ifndef WIN32
#include <sys/resource.h>
#endif

#define BANNER "MNet %s %s/%s-%s (%s) :: Logon Server"

#ifndef WIN32
#include <sched.h>
#endif

initialiseSingleton(LogonServer);
MNet::Threading::AtomicBoolean mrunning(true);
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

int main(int argc, char** argv)
{
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

	new LogonServer;

	// Run!
	LogonServer::getSingleton().Run(argc, argv);
	delete LogonServer::getSingletonPtr();

	/** @brief �鿴�ڴ��ʹ����� */
	CMemoryPools::Instance().DisplayMemoryList();
	CMemoryPools::release();
	getchar();
	/** @brief ����ڴ�й© */
	_CrtDumpMemoryLeaks();
}

#define DEF_VALUE_NOT_SET 0xDEADBEEF

void LogonServer::Run(int argc, char** argv)
{
	UNIXTIME = time(NULL);
	g_localTime = *localtime(&UNIXTIME);
	char* config_file = (char*)CONFDIR "/logon.conf";
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
	
	Log.Success("ThreadMgr", "Starting...");
	sLog.SetFileLoggingLevel(Config.MainConfig.GetIntDefault("LogLevel", "File", 0));
	//�����̳߳�
	ThreadPool.Startup();
	//�������ݿ�����,��ʱû��

	Log.Success("AccountMgr", "Starting...");
	new AccountMgr;//�����û�����
	new IPBanner;  //ipͳ��

	Log.Success("InfoCore", "Starting...");
	new InformationCore;//��Ϣ����

	//new PatchMgr;//����Ҫ��������
	//Log.Notice("AccountMgr", "Precaching accounts...");
	sAccountMgr.ReloadAccounts(true);//���´����ݿ��м����û�
	Log.Success("AccountMgr", "%u accounts are loaded and ready.", sAccountMgr.GetCount());

	// Spawn periodic function caller thread for account reload every 10mins
	// �߳����ں���ÿ10�����¼����û����ڴ�
	int atime = Config.MainConfig.GetIntDefault("Rates", "AccountRefresh", 600);
	atime *= 1000;
	PeriodicFunctionCaller<AccountMgr> * pfc = new PeriodicFunctionCaller<AccountMgr>(AccountMgr::getSingletonPtr(), &AccountMgr::ReloadAccountsCallback, atime,"AccountMgr");
	ThreadPool.ExecuteTask(pfc);//�̳߳�ִ���������

	// ��������
	uint32 cport = Config.MainConfig.GetIntDefault("Listen", "RealmListPort", 3724);
	uint32 sport = Config.MainConfig.GetIntDefault("Listen", "ServerPort", 8093);
	string host = Config.MainConfig.GetStringDefault("Listen", "Host", "0.0.0.0");
	string shost = Config.MainConfig.GetStringDefault("Listen", "ISHost", host.c_str());

	min_build = LOGON_MINBUILD;
	max_build = LOGON_MAXBUILD;

	//��ȡָ���������������worldserver����֤
	string logon_pass = Config.MainConfig.GetStringDefault("LogonServer", "RemotePassword", "888888");
	Sha1Hash hash;
	hash.UpdateData(logon_pass);
	hash.Finalize();
	memcpy(sql_hash, hash.GetDigest(), 20);

	ThreadPool.ExecuteTask(new LogonConsoleThread("LogonConsoleThread"));

	new SocketMgr;
	new SocketGarbageCollector;//��������

	ListenSocket<AuthSocket> * cl = new ListenSocket<AuthSocket>(host.c_str(), cport,"AuthSocket");
	ListenSocket<LogonCommServerSocket> * sl = new ListenSocket<LogonCommServerSocket>(shost.c_str(), sport,"LogonCommServerSocket");

	sSocketMgr.SpawnWorkerThreads();

	// Spawn auth listener
	// Spawn interserver listener
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

		/* write pid file */
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
		//ThreadPool.Gobble();
		sLog.outString("Success! Ready for connections");
		while(mrunning.GetVal())
		{
			if(!(++loop_counter % 20))	 // 20 seconds
				CheckForDeadSockets();//���AuthSocket����������

			if(!(loop_counter % 300))	// 5mins
				ThreadPool.IntegrityCheck();//�̳߳��߳������

			if(!(loop_counter % 5))
			{
				sInfoCore.TimeoutSockets();//���LogonCommServerSocket��ʱ������
				sSocketGarbageCollector.Update();
				CheckForDeadSockets();			  // Flood Protection
				UNIXTIME = time(NULL);
				g_localTime = *localtime(&UNIXTIME);
			}

			MNet::Sleep(1000);
		}

		sLog.outString("Shutting down...");
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

	pfc->kill();
	cl->Close();
	sl->Close();
	sSocketMgr.CloseAll();
#ifdef WIN32
	sSocketMgr.ShutdownThreads();
#endif
	sLogonConsole.Kill();
	delete LogonConsole::getSingletonPtr();

	// kill db
	sLog.outString("Waiting for database to close..");

	ThreadPool.Shutdown();

	// delete pid file
	remove("logonserver.pid");

	delete AccountMgr::getSingletonPtr();
	delete InformationCore::getSingletonPtr();
	delete IPBanner::getSingletonPtr();
	delete SocketMgr::getSingletonPtr();
	delete SocketGarbageCollector::getSingletonPtr();
	delete pfc;
	delete cl;
	delete sl;
	LOG_BASIC("Shutdown complete.");
	sLog.Close();
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