/**
* \brief zebra项目场景服务器,游戏绝大部分内容都在本实现
*/

//#include <zebra/ScenesServer.h>
#include "duplicateManager.h"
#include "scriptTickTask.h"
#include "meterialsManager.h"
#include <zebra/csBox.h>
#include "giftBox.h"
#include "boxCircle.h"

ScenesService *ScenesService::instance = NULL;
bool ScenesService::reload=false;
zLogger * ScenesService::gmlogger = NULL;
zLogger * ScenesService::objlogger = NULL;
zLogger * ScenesService::wglogger = NULL;
std::set<DWORD> dupMapList;

WORD max_level = 0;
QWORD max_exp = 0;
WORD trun_point_rate = 0;
WORD trun_skill_rate = 0;

//sky 装备掉落极品几率
WORD g_blue_changce = 0;
WORD g_green_changce = 0;
WORD g_purple_changce = 0;
WORD g_orange_changce = 0;

//sky 有益技能列表
std::vector<DWORD> UseableMagicList;

//sky 冷却时间配置表
std::vector<stXmlItemCoolTime> vXmlItemCoolTime;

//[Shx Add 套装配置表]
std::vector<stxml_SuitAttribute> vXmlSuitAttribute;

std::vector<BYTE> CoolTimeSendData;

//sky 打坐恢复系数(默认是4%)
WORD recoverRate = 4;

Cmd::stChannelChatUserCmd * ScenesService::pStampData = 0;

/// 判国所需经费
DWORD cancel_country_need_money = 50000; //默认五锭
DWORD is_cancel_country = 0; // 是否允许叛国

NFilterModuleArray g_nFMA;


//[shx Add 给套装物品一个标志,使其可以被快速定位]
void InitObjBase_SuitData()
{
	for(int i = 0; i < vXmlSuitAttribute.size(); i ++)
	{
		LPDWORD pParts = vXmlSuitAttribute[i].MemberList;
		for(int j = 0; j < vXmlSuitAttribute[i].count; j ++)
		{
			zObjectB* pBase = objectbm.get(pParts[j]);
			if(pBase ) 
			{
				if( pBase->nSuitData >= 0)
					printf("\n错误: 物品 %u 在套装配置文件中被重复使用\n", pParts[j]);

				pBase->nSuitData = i;
			}			
		}
	}
}
/**
* \brief 初始化网络服务器程序
*
* 实现了虚函数<code>zService::init</code>
*
* \return 是否成功
*/
bool ScenesService::init()
{
	using namespace Cmd;


	NFilterModuleArray::const_iterator pIterator;

	Zebra::logger->info("ScenesService::init()");

	for(int i=0; i<13; i++) countryPower[i]=1;

	//初始化连接线程池
	int state = state_none;
	to_lower(Zebra::global["threadPoolState"]);
	if ("repair" == Zebra::global["threadPoolState"]
	|| "maintain" == Zebra::global["threadPoolState"])
		state = state_maintain;
	taskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state,5000);

	max_level = atoi(Zebra::global["maxlevel"].c_str());
	max_exp = atoi(Zebra::global["maxExp"].c_str());

	char *duplist = const_cast<char*>(Zebra::global["duplicate"].c_str());
	char *token = strtok( duplist,";" );
	while( token != NULL )
	{
		dupMapList.insert(atoi(token));
		token = strtok( NULL,";");
	}


	trun_point_rate = atoi(Zebra::global["trun_point_rate"].c_str());
	trun_skill_rate = atoi(Zebra::global["trun_skill_rate"].c_str());

	//sky 从配置文件中读取极品掉落基本配置
	g_blue_changce = atoi(Zebra::global["g_blue_changce"].c_str());		//蓝色
	g_green_changce = atoi(Zebra::global["g_green_changce"].c_str());		//绿色
	g_purple_changce = atoi(Zebra::global["g_purple_changce"].c_str());	//紫色
	g_orange_changce = atoi(Zebra::global["g_orange_changce"].c_str());	//橙色

	//sky 从配置文件中读取打坐恢复系数
	recoverRate = atoi(Zebra::global["recoverRate"].c_str());

	fprintf(stderr,"maxlevel %ld\n",atoi(Zebra::global["maxlevel"].c_str()));
	fprintf(stderr,"maxlexp %ld\n",atoi(Zebra::global["maxExp"].c_str()));

	fprintf(stderr, "基本蓝色装备掉落率: %ld\n",g_blue_changce);
	fprintf(stderr, "基本绿色装备掉落率: %ld\n",g_green_changce);
	fprintf(stderr, "基本紫色装备掉落率: %ld\n",g_purple_changce);
	fprintf(stderr, "基本橙色装备掉落率: %ld\n",g_orange_changce);

	fprintf(stderr, "打坐恢复系数: %u%%\n", recoverRate);

	if (NULL == taskPool
		|| !taskPool->init())
		return false;

	strncpy(pstrIP,zSocket::getIPByIfName(Zebra::global["ifname"].c_str()),MAX_IP_LENGTH - 1);
	//Zebra::logger->debug("%s",pstrIP);

	if (!zSubNetService::init())
	{
		return false;
	}

	const Cmd::Super::ServerEntry *serverEntry = NULL;

	//连接档案服务器
	serverEntry = getServerEntryByType(RECORDSERVER);
	if (NULL == serverEntry)
	{
		Zebra::logger->error("不能找到档案服务器相关信息,不能连接档案服务器");
		return false;
	}

	recordClient = new RecordClient("档案服务器客户端",serverEntry->pstrIP,serverEntry->wdPort);
	if (NULL == recordClient)
	{
		Zebra::logger->error("没有足够内存,不能建立档案服务器客户端实例");
		return false;
	}
	if (!recordClient->connectToRecordServer())
	{
		Zebra::logger->error("ScenesService::init 连接档案服务器失败");
		return false;
	}
	if (recordClient->start())
	{
	}
	//Zebra::logger->info("初始化档案服务器模块(%s:%d)成功",serverEntry->pstrIP,serverEntry->wdPort);

	//连接会话服务器
	serverEntry = getServerEntryByType(SESSIONSERVER);
	if (NULL == serverEntry)
	{
		Zebra::logger->error("不能找到会话服务器相关信息,不能连接会话服务器");
		return false;
	}
	sessionClient = new SessionClient("会话服务器客户端",serverEntry->pstrIP,serverEntry->wdPort);
	if (NULL == sessionClient)
	{
		Zebra::logger->error("没有足够内存,不能建立会话服务器客户端实例");
		return false;
	}
	if (!sessionClient->connectToSessionServer())
	{
		Zebra::logger->error("ScenesService::init 连接会话服务器失败");
		return false;
	}
	if (sessionClient->start())
		Zebra::logger->info("初始化会话服务器模块(%s:%d)成功",serverEntry->pstrIP,serverEntry->wdPort);

	//连接小游戏服务器
	serverEntry = getServerEntryByType(MINISERVER);
	if (NULL == serverEntry)
	{
		Zebra::logger->error("不能找到小游戏服务器相关信息,不能连接小游戏服务器");
		return false;
	}
	miniClient = new MiniClient("小游戏服务器客户端",serverEntry->pstrIP,serverEntry->wdPort,serverEntry->wdServerID);
	if (NULL == miniClient)
	{
		Zebra::logger->error("没有足够内存,不能建立小游戏服务器客户端实例");
		return false;
	}
	if (!miniClient->connectToMiniServer())
	{
		Zebra::logger->error("ScenesService::init 连接小游戏服务器失败");
		return false;
	}
	if (!miniClient->start())
	{
		Zebra::logger->warn("初始化Mini服务器模块失败");
	}
	else{
		Zebra::logger->info("初始化Mini服务器模块(%s:%d)成功",serverEntry->pstrIP,serverEntry->wdPort);
	}

	if (!SceneNpcManager::getMe().init())
	{
		Zebra::logger->warn("初始化NPC管理器失败");
	}
	else{
		Zebra::logger->info("初始化NPC管理器成功");
	}

	if (!SceneTimeTick::getInstance().start())
	{
		Zebra::logger->warn("初始化TimeTick模块失败");
	}
	else{
		Zebra::logger->info("初始化TimeTick模块成功");
	}

	//加载基本数据
	if (!loadAllBM())
	{
		Zebra::logger->error("初始化基本数据模块失败");
		return false;
	}
	Zebra::logger->info("初始化基本数据模块成功");

	char srv[256];
	bzero(srv,sizeof(srv));
	sprintf(srv,"WS[%d]",getServerID());
	objlogger = new zLogger(srv);
	objlogger->setLevel(Zebra::global["log"]);
	//设置写本地日志文件
	if ("" != Zebra::global["objlogfilename"])
	{
		bzero(srv,sizeof(srv));
		char sub[256];
		bzero(sub,sizeof(sub));
		_snprintf(srv,sizeof(srv),"%s",Zebra::global["objlogfilename"].c_str());
		char *tok = strstr(srv,".");
		if (tok != NULL)
		{
			strncpy(sub,tok,sizeof(sub));
			bzero(tok,strlen(tok));
			sprintf(srv + strlen(srv),"%d",getServerID());
			strncat(srv,sub,sizeof(srv) - strlen(srv));
		}
		else
		{
			_snprintf(srv + strlen(srv),sizeof(srv) - strlen(srv),"%d",getServerID());
		}
		objlogger->addLocalFileLog(srv);
		objlogger->removeConsoleLog();
	}

	gmlogger = new zLogger("gmlog");
	gmlogger->setLevel(Zebra::global["log"]);
	if ("" != Zebra::global["gmlogfilename"]){
		gmlogger->addLocalFileLog(Zebra::global["gmlogfilename"]);
		gmlogger->removeConsoleLog();
	}

	wglogger = new zLogger("wglog");
	wglogger->setLevel(Zebra::global["log"]);
	if ("" != Zebra::global["wglogfilename"]){
		wglogger->addLocalFileLog(Zebra::global["wglogfilename"]);
		wglogger->removeConsoleLog();
	}

	Zebra::logger->info("加载特征码文件,大小%u",updateStampData());

	if (!SceneManager::getInstance().init())
	{
		Zebra::logger->error("初始化场景管理器失败");
		return false;
	}
	Zebra::logger->info("初始化场景管理器成功");

	if (!NpcTrade::getInstance().init())
	{
		Zebra::logger->error("初始化NPC交易配置模块失败");
		return false;
	}
	Zebra::logger->info("初始化NPC交易配置模块成功");

	ALLVARS1(server_id,getServerID());
	ALLVARS(load);

	if (!QuestTable::instance().init())
	{
		Zebra::logger->error("初始化任务模块失败");
		return false;
	}
	Zebra::logger->info("初始化任务模块成功");

	LuaVM* vm = ScriptingSystemLua::instance().createVM();
	LuaScript* script = ScriptingSystemLua::instance().createScriptFromFile("newquest/quest.lua");
	Binder bind;
	bind.bind(vm);
	vm->execute(script);
	SAFE_DELETE(script);
	//sppeed up
	ScriptQuest::get_instance().sort();

	if (!MagicRangeInit::getInstance().init())
	{
		Zebra::logger->error("初始化攻击范围定义模块失败");
		return false;
	}
	Zebra::logger->info("初始化攻击范围定义模块成功");

	CountryDareM::getMe().init();
	CountryTechM::getMe().init();
	CountryAllyM::getMe().init();

	loadFilter(g_nFMA,"ScenesServer_*.dll");
	//init
	for(pIterator=g_nFMA.begin(); pIterator != g_nFMA.end();pIterator++)
	{
		if (NULL != pIterator->filter_init)
		{
			pIterator->filter_init();
		}
	}

	Zebra::logger->info("ScenesService::init() OK");

	globalBox::newInstance();
	globalBox::getInstance().init();
	boxCircle::newInstance();
	scriptMessageFilter::initFilter();

	//Shx 读取套装配置列表
	LoadSuitInfo();
	InitObjBase_SuitData();


	//sky 读取物品冷却列表
	LoadItmeCoolTime();
	//sky 把读到的冷却数据风装成发给客户端的数据包
	StlToSendData();
	//duplicateManager::newInstance();

	return true;
}


/**
* \brief 新建立一个连接任务
*
* 实现纯虚函数<code>zNetService::newTCPTask</code>
*
* \param sock TCP/IP连接
* \param addr 地址
*/
void ScenesService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
#ifdef ZEBRA_RELEASE
	char szLimit[256];

	//limit
	EPE_GetRegisterInfo(EPEGRI_REGINFO,szLimit,sizeof(szLimit));
	if (taskPool->getSize() >= atoi(szLimit))
	{
		::closesocket(sock);
		return;
	}
#endif //ZEBRA_RELEASE

	SceneTask *tcpTask = new SceneTask(taskPool,sock,addr);

	zTCPTask* pTask = (zTCPTask*)tcpTask;
	if (NULL == tcpTask)
	{
		//内存不足,直接关闭连接
		::closesocket(sock);
	}
	else if (!taskPool->addVerify(tcpTask))
	{
		//得到了一个正确连接,添加到验证队列中
		SAFE_DELETE(tcpTask);
	}
}

/**
* \brief 解析来自管理服务器的指令
*
* 这些指令是网关和管理服务器交互的指令<br>
* 实现了虚函数<code>zSubNetService::msgParse_SuperService</code>
*
* \param pNullCmd 待解析的指令
* \param nCmdLen 待解析的指令长度
* \return 解析是否成功
*/
bool ScenesService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
	switch(pNullCmd->cmd)
	{
	case Cmd::GmTool::CMD_GMTOOL:
		{
			using namespace Cmd::GmTool;

			switch(pNullCmd->para)
			{
			case PARA_PUNISH_GMTOOL:
				{
					t_Punish_GmTool * rev = (t_Punish_GmTool *)pNullCmd;
					SceneUser *pUser = SceneUserManager::getMe().getUserByName(rev->userName);
					if (!pUser) break;
					switch (rev->operation)
					{
					case 1://禁言
						{
							pUser->delayForbidTalk(rev->delay);
							if (rev->delay>0)
							{
								Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"你被GM禁言 %d 秒",rev->delay);                                               
								ScenesService::gmlogger->info("玩家 %s 被禁言 %d 秒",pUser->name,rev->delay);
							}
							else
							{
								Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"你被GM解除禁言,现在可以说话了");
								ScenesService::gmlogger->info("玩家 %s 被解除禁言",pUser->name);
							}
						}
						break;
					case 2://关禁闭
						break;
					case 3://踢下线
						{
							OnQuit event(1);
							EventTable::instance().execute(*pUser,event);
							execute_script_event(pUser,"quit");

							pUser->save(Cmd::Record::LOGOUT_WRITEBACK);
							Cmd::Session::t_unregUser_SceneSession ret;
							ret.dwUserID=pUser->id;
							ret.dwSceneTempID=pUser->scene->tempid;
							ret.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
							sessionClient->sendCmd(&ret,sizeof(ret));
							Cmd::Scene::t_Unreg_LoginScene retgate;
							retgate.dwUserID = pUser->id;
							retgate.dwSceneTempID = pUser->scene->tempid;
							retgate.retcode = Cmd::Scene::UNREGUSER_RET_ERROR;
							pUser->gatetask->sendCmd(&retgate,sizeof(retgate));

							pUser->unreg();
						}
						break;
					case 4://警告
						{
							Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,rev->reason);
						}
						break;
					default:
						return true;
					}

					rev->level = pUser->charbase.level;
					rev->accid = pUser->charbase.accid;
					zRTime ct;
					rev->startTime = ct.sec();
					strncpy(rev->country,SceneManager::getInstance().getCountryNameByCountryID(pUser->charbase.country),MAX_NAMESIZE);
					ScenesService::getInstance().sendCmdToSuperServer(rev,sizeof(t_Punish_GmTool));
				}
				break;
			}
		}
		break;
	}

	Zebra::logger->error("ScenesService::msgParse_SuperService(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}

/**
* \brief 结束网络服务器
*
* 实现了纯虚函数<code>zService::final</code>
*
*/
void ScenesService::final()
{
	NFilterModuleArray::const_iterator pIterator;

	//term
	for(pIterator=g_nFMA.begin(); pIterator != g_nFMA.end();pIterator++)
	{
		if (NULL != pIterator->filter_term)
		{
			pIterator->filter_term();
		}
	}

	SceneTimeTick::getInstance().final();
	SceneTimeTick::getInstance().join();
	SceneTimeTick::delInstance();
	SceneUserManager::getMe().removeAllUser();

	if (taskPool)
	{
		taskPool->final();
		SAFE_DELETE(taskPool);
	}
	if (sessionClient)
	{
		sessionClient->final();
		sessionClient->join();
		SAFE_DELETE(sessionClient);
	}

	if (recordClient)
	{
		recordClient->final();
		recordClient->join();
		SAFE_DELETE(recordClient);
	}

	SceneTaskManager::delInstance();

	SceneManager::delInstance();
	GlobalObjectIndex::delInstance();
	NpcTrade::delInstance();

	unloadAllBM();
	zSubNetService::final();

	Zebra::logger->debug("ScenesService::final");
	SAFE_DELETE(gmlogger);
}

/**
* \brief 读取配置文件
*
*/
class SceneConfile:public zConfile
{
	bool parseYour(const xmlNodePtr node)
	{
		if (node)
		{
			xmlNodePtr child=parser.getChildNode(node,NULL);
			while(child)
			{
				parseNormal(child);
				child=parser.getNextNode(child,NULL);
			}
			return true;
		}
		else
			return false;
	}
};

/**
* \brief 重新读取配置文件,为HUP信号的处理函数
*
*/
void ScenesService::reloadConfig()
{
	reload=true;
	Zebra::logger->debug("ScenesService::reloadConfig");
}

void ScenesService::checkAndReloadConfig()
{
	if (reload)
	{
		reload=false;
		Zebra::logger->debug("ScenesService::checkAndReloadConfig");
		SceneConfile sc;
		sc.parse("ScenesServer");
		loadAllBM();
		NpcTrade::getInstance().init();
		//MessageSystem::getInstance().init();
		//定时存档配置
		if (atoi(Zebra::global["writebacktimer"].c_str()))
		{
			ScenesService::getInstance().writeBackTimer = atoi(Zebra::global["writebacktimer"].c_str());
		}
		else
		{
			ScenesService::getInstance().writeBackTimer = 600;
		}

		ScenesService::getInstance().ExpRate       = atoi(Zebra::global["ExpRate"].c_str());
		ScenesService::getInstance().DropRate      = atoi(Zebra::global["DropRate"].c_str());
		ScenesService::getInstance().DropRateLevel = atoi(Zebra::global["DropRateLevel"].c_str());

		//指令检测开关
		if (Zebra::global["cmdswitch"] == "true")
		{
			zTCPTask::analysis._switch = true;
			zTCPClient::analysis._switch=true;
		}
		else
		{
			zTCPTask::analysis._switch = false;
			zTCPClient::analysis._switch=false;
		}
	}
}

/**
* \brief 重新读取特征码文件
*
*/
DWORD ScenesService::updateStampData()
{
	std::string process_file;

	if (pStampData)
	{
		free(pStampData);
		pStampData = 0;
	}

	int f=0;

	process_file = Zebra::global["confdir"] + "process.dat";
	// f = open(process_file.c_str(),O_RDONLY);
	/* if (f != -1)
	{
	pStampData = (Cmd::stChannelChatUserCmd *)malloc(zSocket::MAX_DATASIZE);
	bzero(pStampData,zSocket::MAX_DATASIZE);
	constructInPlace(pStampData);

	pStampData->dwType = Cmd::CHAT_TYPE_SYSTEM;
	pStampData->dwSysInfoType = Cmd::INFO_TYPE_GAME;
	strncpy(pStampData->pstrChat,"欢迎来到英雄无双",MAX_CHATINFO-1);
	pStampData->dwFromID = read(f,(void *)(pStampData->tobject_array),zSocket::MAX_DATASIZE-sizeof(Cmd::stChannelChatUserCmd));
	close(f);

	pStampData->dwChannelID = atoi(Zebra::global["service_flag"].c_str()) & Cmd::Session::SERVICE_PROCESS;
	return pStampData->dwFromID;
	}*/

	return 1;
	//return 0;
}

/**
* \brief 主程序入口
*
* \param argc 参数个数
* \param argv 参数列表
* \return 运行结果
*/
int service_main(int argc,char *argv[])
{
	Zebra::logger=new zLogger("ScenesServer");


	//设置缺省参数
	Zebra::global["datadir"]         = "data/";
	Zebra::global["confdir"]         = "conf/";
	Zebra::global["questdir"]        = "quest/";
	Zebra::global["cmdswitch"]       = "true";
	Zebra::global["writebacktimer"]  = "600";
	Zebra::global["ExpRate"]         = "1";
	Zebra::global["DropRate"]        = "1";
	Zebra::global["DropRateLevel"]   = "0";
	Zebra::global["mail_service"]    = "on";

	//解析配置文件参数
	SceneConfile sc;
	if (!sc.parse("ScenesServer"))
		return -1;

	//设置日志级别
	Zebra::logger->setLevel(Zebra::global["log"]);
	//设置写本地日志文件
	if ("" != Zebra::global["logfilename"]){
		Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
		Zebra::logger->removeConsoleLog();
	}

	if (atoi(Zebra::global["writebacktimer"].c_str()))
	{
		ScenesService::getInstance().writeBackTimer = atoi(Zebra::global["writebacktimer"].c_str());
	}
	else
	{
		ScenesService::getInstance().writeBackTimer = 600;
	}

	ScenesService::getInstance().ExpRate       = atoi(Zebra::global["ExpRate"].c_str());
	ScenesService::getInstance().DropRate      = atoi(Zebra::global["DropRate"].c_str());
	ScenesService::getInstance().DropRateLevel = atoi(Zebra::global["DropRateLevel"].c_str());

	//指令检测开关
	if (Zebra::global["cmdswitch"] == "true")
	{
		zTCPTask::analysis._switch = true;
		zTCPClient::analysis._switch=true;
	}
	else
	{
		zTCPTask::analysis._switch = false;
		zTCPClient::analysis._switch=false;
	}

	scriptTaskManagement::newInstance();
	meterialsManager::newInstance();
	duplicateManager::newInstance();

	Zebra_Startup();

	ScenesService::getInstance().main();
	ScenesService::delInstance();
	SceneUserManager::destroyMe();



	return 0;
}


//[Shx Add 读取套装配置信息]
bool ScenesService::LoadSuitInfo()
{
	char sFileName[MAX_PATH];
	strcpy( sFileName, "data/SuitInfo.xml" );

	zXMLParser xml;
	if (!xml.initFile(sFileName))
	{
		Zebra::logger->error("加载套装配置文件 %s 失败",sFileName);
		return false;
	}
	xmlNodePtr _root;
	xmlNodePtr _suits;

	_root = xml.getRootNode("HREO");
	_suits = xml.getChildNode(_root, "SUITS");	
	if(_root && _suits)
	{


		xmlNodePtr _node = xml.getChildNode(_suits,"suit");  
		while (_node) 
		{	
			stxml_SuitAttribute stData;		

			xml.getNodePropNum(_node, "id", &stData.id, 2);

			std::string str="";
			xml.getNodePropStr(_node, "name", str);			
			strncpy(stData.Name, str.c_str(), 30);

			xmlNodePtr _part = xml.getChildNode(_node, "part");
			int n_p = 0;
			while (_part && n_p < MAX_SUIT_NUM)
			{
				xml.getNodePropNum(_part,"itemid", &stData.MemberList[n_p], 4);				
				n_p ++;					
				_part = xml.getNextNode(_part,"part");
			}
			stData.count = n_p;	
			
			int n_e = 0;
			xmlNodePtr _Effect = xml.getChildNode(_node, "effect");					
			while (_Effect && n_e < MAX_SUIT_NUM)
			{
				st_SuitEffect stEffect;			
				xml.getNodePropNum(_Effect, "itemcount",&stEffect.eRequire,	1);			
				xml.getNodePropNum(_Effect, "ekey",		&stEffect.eKey,		1);
				xml.getNodePropNum(_Effect, "evalue",	&stEffect.eValue,	2);	

				stData.EffectList.push_back(stEffect);				
				n_e ++;
				_Effect = xml.getNextNode(_Effect,"effect");				
			}
			stData.eCount = n_e;

			vXmlSuitAttribute.push_back(stData);

			_node = xml.getNextNode(_node, "suit");	

			printf("加载套装 %s,部件: %d, 属性: %d\n", stData.Name, n_p, n_e);
		}
		printf("读取套装配置记录: %d\n", vXmlSuitAttribute.size());
		return true;
	}
	Zebra::logger->error("读取套装配置文件 %s 失败",sFileName);
	return false;
}
//End Shx

bool ScenesService::LoadItmeCoolTime()
{
	char CoolFileName[MAX_PATH];
	strcpy( CoolFileName, "data/ItemCoolTime.xml" );

	zXMLParser xml;
	if (!xml.initFile(CoolFileName))
	{
		Zebra::logger->error("加载物品冷却文件 %s 失败",CoolFileName);
		return false;
	}

	xmlNodePtr root;

	root = xml.getRootNode("CoolTime");

	if (root)
	{
		xmlNodePtr node = xml.getChildNode(root,"cooldown");  
		while (node) 
		{
			stXmlItemCoolTime CoolTiem;
			stItemTypeCoolTiem TypeCool;
			stItemIdCoolTime IdCool;

			if(!xml.getNodePropNum(node, "CoolID", &(CoolTiem.CoolTimeType),sizeof(WORD))) 
			{
				return false;
			}

			if(!xml.getNodePropNum(node, "times", &(CoolTiem.nCoolTime), sizeof(DWORD)))
			{
				return false;
			}

			xmlNodePtr phaseNode = xml.getChildNode(node,"objclass");

			while(phaseNode)
			{
				if(!xml.getNodePropNum(phaseNode, "type", &(TypeCool.ItemType), sizeof(WORD)))
				{
					return false;
				}

				if(!xml.getNodePropNum(phaseNode, "time", &(TypeCool.CoolTime), sizeof(DWORD)))
				{
					TypeCool.CoolTime = 0;
				}

				CoolTiem.TypeCoolTime.push_back(TypeCool);

				phaseNode = xml.getNextNode(phaseNode, "objclass");
			}

			xmlNodePtr IdNode = xml.getChildNode(node, "objid");

			while(IdNode)
			{
				if(!xml.getNodePropNum(IdNode, "id", &(IdCool.ItemID), sizeof(DWORD)))
				{
					return false;
				}

				if(!xml.getNodePropNum(IdNode, "time", &(IdCool.CoolTime), sizeof(DWORD)))
				{
					IdCool.CoolTime = 0;
				}

				CoolTiem.IdCoolTime.push_back(IdCool);

				IdNode = xml.getNextNode(IdNode, "objid");
			}

			vXmlItemCoolTime.push_back(CoolTiem);

			node = xml.getNextNode(node,"cooldown");
		}

		int count = vXmlItemCoolTime.size();
		printf("读取物品冷却类型数量:%d", count);

		return true;
	}

	return false;
}

bool ScenesService::StlToSendData()
{
	int nLen = 0;
	int index = 0;

	std::vector<stXmlItemCoolTime>::iterator iter;
	std::vector<stItemTypeCoolTiem>::iterator iter1;
	std::vector<stItemIdCoolTime>::iterator iter2;

	for(iter=vXmlItemCoolTime.begin(); iter!=vXmlItemCoolTime.end(); iter++)
	{
		nLen += iter->TypeCoolTime.size() * sizeof(stItemTypeCoolTiem); // TypeCoolTime
		nLen += iter->IdCoolTime.size() * sizeof(stItemIdCoolTime); // IdCoolTime
	}

	nLen += vXmlItemCoolTime.size() * sizeof(WORD); // TypeCoolTime
	nLen += vXmlItemCoolTime.size() * sizeof(DWORD);// nCoolTime

	CoolTimeSendData.resize(nLen + sizeof(Cmd::stItemCoolTimesUserCmd) + sizeof(WORD) * vXmlItemCoolTime.size() * 2);
	Cmd::stItemCoolTimesUserCmd cmd(vXmlItemCoolTime.size());
	memcpy(&CoolTimeSendData[index], &cmd, sizeof(Cmd::stItemCoolTimesUserCmd), sizeof(Cmd::stItemCoolTimesUserCmd));
	index += sizeof(Cmd::stItemCoolTimesUserCmd);
	for(iter=vXmlItemCoolTime.begin(); iter!=vXmlItemCoolTime.end(); iter++)
	{
		memcpy(&CoolTimeSendData[index],&(*iter).CoolTimeType, sizeof(WORD), sizeof(WORD));
		index += sizeof(WORD);
		memcpy(&CoolTimeSendData[index],&(*iter).nCoolTime, sizeof(DWORD), sizeof(DWORD));
		index += sizeof(DWORD);

		WORD Count = iter->TypeCoolTime.size();

		memcpy(&CoolTimeSendData[index],&Count, sizeof(WORD), sizeof(WORD));
		index += sizeof(WORD);

		for(iter1=iter->TypeCoolTime.begin(); iter1!=iter->TypeCoolTime.end(); iter1++)
		{
			memcpy(&CoolTimeSendData[index], &(*iter1), sizeof(stItemTypeCoolTiem), sizeof(stItemTypeCoolTiem));
			index += sizeof(stItemTypeCoolTiem);
		}

		Count = iter->IdCoolTime.size();
		memcpy(&CoolTimeSendData[index],&Count, sizeof(WORD), sizeof(WORD));
		index += sizeof(WORD);

		for(iter2=iter->IdCoolTime.begin(); iter2!=iter->IdCoolTime.end(); iter2++)
		{
			memcpy(&CoolTimeSendData[index], &(*iter2), sizeof(stItemIdCoolTime), sizeof(stItemIdCoolTime));
			index += sizeof(stItemIdCoolTime);
		}
	}

	return true;
}