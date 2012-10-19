// Cache.cpp : 定义控制台应用程序的入口点。
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "git_version.h"
#include "r_common.h"
using namespace std;
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi")

#define BANNER "MNet %s %s/%s-%s (%s) :: 缓存服务器"
#define DEF_VALUE_NOT_SET 2   /**< 2为调试等级  */
#define CONFDIR "configs"

Database* sCacheSQL;

int w32initWinSock(void) {

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

bool startDB()
{
	string lhostname, lusername, lpassword, ldatabase;
	int lport = 0;

	bool result;

	bool existsUsername = Config.MainConfig.GetString("LogonDatabase", "Username", &lusername);
	bool existsPassword = Config.MainConfig.GetString("LogonDatabase", "Password", &lpassword);
	bool existsHostname = Config.MainConfig.GetString("LogonDatabase", "Hostname", &lhostname);
	bool existsName     = Config.MainConfig.GetString("LogonDatabase", "Name",     &ldatabase);
	bool existsPort     = Config.MainConfig.GetInt("LogonDatabase", "Port",     &lport);

	result = existsUsername && existsPassword && existsHostname && existsName && existsPort;
	if (!result)
		return false;
	

	sCacheSQL = Database::CreateDatabaseInterface();

	if(!sCacheSQL->Initialize(lhostname.c_str(), (unsigned int)lport, lusername.c_str(),
		lpassword.c_str(), ldatabase.c_str(), Config.MainConfig.GetIntDefault("LogonDatabase", "ConnectionCount", 5),
		16384))
	{
		LOG_ERROR("sql: Logon database initialization failed. Exiting.");
		return false;
	}
	return true;
}

/** @brief 更改窗口名称 */
void SetDlgTitle()
{
	char szName[_MAX_PATH*100];
	char szCode[8192];
	GetModuleFileName(NULL,szName,sizeof(szName));
	_snprintf(szCode,sizeof(szCode),"登录服务器 - %s",PathFindFileName(szName));
	SetConsoleTitle(szCode);
}

int main()
{
	/** @brief 初始化配置文件，日志系统 */
	SetDlgTitle();
	UNIXTIME = time(NULL);
	g_localTime = *localtime(&UNIXTIME);
	char* config_file = (char*)CONFDIR "/cache.conf";
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

	/** @brief 初始化数据库连接 */
	startDB();

	RedisClient ge("192.168.1.116");
	int re = ge.exists_key("kalle");

	REDIS_TYPE atype = ge.type_key("kalle");

	ge.get_keys("kalle");

	ge.random_key();

	ge.rename_key("kalle","kall");

	ge.renamenx("kall","kalle");

	ge.dbsize();

	ge.expire_key("kalle",1000);

	ge.ttl_key("kalle");

	ge.select_index(1);

	ge.move("kalle",2);

	atexit((void(*)(void)) WSACleanup);

	/*REDIS redis;
	REDIS_INFO info;
	redis = credis_connect("192.168.1.116",6379,1000);
	if (redis == NULL) {
		printf("Error connecting to Redis server. Please start server to run tests.\n");
		exit(1);
	}
	int rc;
	char *val;
	rc = credis_ping(redis);
	printf("ping returned: %d\n", rc);

	if (credis_set(redis, "kalle", "qwerty") != 0)
		printf("get returned error\n");

	rc = credis_get(redis, "kalle", &val);
	printf("get kalle returned: %s\n", val);*/

	return 0;
}

