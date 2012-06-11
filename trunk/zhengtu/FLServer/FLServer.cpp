/**
* \brief zebra项目登陆服务器,负责登陆,建立帐号、档案等功能
*
*/
#include "FLServer.h"

zDBConnPool * FLService::dbConnPool = NULL;
FLService *FLService::instance = NULL;

/**
* \brief 初始化网络服务器程序
*
* 实现了虚函数<code>zService::init</code>
*
* \return 是否成功
*/
bool FLService::init()
{
	Zebra::logger->debug("FLService::init");  

	dbConnPool = zDBConnPool::newInstance(NULL);

	if (NULL == dbConnPool
		|| !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
	{
		MessageBox(NULL,"连接数据库失败","FLServer",MB_ICONERROR);
		return false;
	}

	if (!zMNetService::init()) return false;
	if (!ServerACLSingleton::instance().init()) return false;
	////---if (!InfoClientManager::getInstance().init()) return false;  

	//初始化连接线程池
	int state = state_none;
	to_lower(Zebra::global["threadPoolState"]);
	if ("repair" == Zebra::global["threadPoolState"]
	|| "maintain" == Zebra::global["threadPoolState"])
		state = state_maintain;

	loginTaskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
	if (NULL == loginTaskPool
		|| !loginTaskPool->init())
		return false;

	serverTaskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
	if (NULL == serverTaskPool
		|| !serverTaskPool->init())
		return false;

	pingTaskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
	if (NULL == pingTaskPool
		|| !pingTaskPool->init())
		return false;

	login_port  = atoi(Zebra::global["login_port"].c_str());
	inside_port = atoi(Zebra::global["inside_port"].c_str());
	ping_port   = atoi(Zebra::global["ping_port"].c_str());

	if (!zMNetService::bind("登陆端口",login_port)
		|| !zMNetService::bind("内部服务端口",inside_port)
		|| !zMNetService::bind("PING端口",ping_port))
	{
		return false;
	}

	FLTimeTick::getInstance().start();

	return true;
}

/**
* \brief 新建立一个连接任务
* 实现纯虚函数<code>zMNetService::newTCPTask</code>
* \param sock TCP/IP连接
* \param srcPort 连接来源端口
* \return 新的连接任务
*/
void FLService::newTCPTask(const SOCKET sock,const WORD srcPort)
{  
	Zebra::logger->debug("FLService::newTCPTask");

	if (srcPort == login_port)
	{
		//客户端登陆验证连接
		zTCPTask *tcpTask = new LoginTask(loginTaskPool,sock);
		if (NULL == tcpTask)
			::closesocket(sock);
		else if (!loginTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else if (srcPort == inside_port)
	{
		//每个区的管理服务器连接
		zTCPTask *tcpTask = new ServerTask(serverTaskPool,sock);
		if (NULL == tcpTask)
			::closesocket(sock);
		else if (!serverTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else if (srcPort == ping_port)
	{
		// 获取PING服务器列表
		zTCPTask *tcpTask = new PingTask(serverTaskPool,sock);
		if (NULL == tcpTask)
			::closesocket(sock);
		else if (!pingTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else
		::closesocket(sock);
}

/**
* \brief 结束网络服务器
*
* 实现了纯虚函数<code>zService::final</code>
*
*/
void FLService::final()
{
	zMNetService::final();

	ServerManager::delInstance();
	LoginManager::delInstance();
	GYListManager::delInstance();

	FLTimeTick::getInstance().final();
	FLTimeTick::getInstance().join();
	FLTimeTick::delInstance();

	////---InfoClientManager::delInstance();
}

/**
* \brief 读取配置文件
*
*/
class LoginConfile:public zConfile
{
	bool parseYour(const xmlNodePtr node)
	{
		if (node)
		{
			xmlNodePtr child=parser.getChildNode(node,NULL);
			while(child)
			{
				if (strcmp((char *)child->name,"InfoServer")==0)
				{
					char buf[64];
					if (parser.getNodeContentStr(child,buf,64))
					{
						Zebra::global["InfoServer"]=buf;
						if (parser.getNodePropStr(child,"port",buf,64))
							Zebra::global["InfoPort"]=buf;
					}
				}
				else
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
void FLService::reloadConfig()
{
	LoginConfile sc;
	sc.parse("FLServer");
	if ("true" == Zebra::global["jpeg_passport"])
		FLService::getInstance().jpeg_passport = true;
	else
		FLService::getInstance().jpeg_passport = false;
	ServerACLSingleton::instance().init();
}

/**
* \brief 主程序入口
*
* \param argc 参数个数
* \param argv 参数列表
* \return 运行结果
*/
int main(int argc,char **argv)
{
	Zebra::logger=new zLogger("FLServer");

	//设置缺省参数
	Zebra::global["login_port"]    = "7000";
	Zebra::global["inside_port"]   = "7001";
	Zebra::global["ping_port"]     = "7002";
	Zebra::global["InfoServer"]    = "127.0.0.1";
	Zebra::global["InfoPort"]      = "9903";
	Zebra::global["jpeg_passport"] = "true";

	//解析配置文件参数
	LoginConfile sc;
	if (!sc.parse("FLServer")) return EXIT_FAILURE;
	if ("true" == Zebra::global["jpeg_passport"])
		FLService::getInstance().jpeg_passport = true;
	else
		FLService::getInstance().jpeg_passport = false;
	if (atoi(Zebra::global["maxGatewayUser"].c_str()))
	{
		LoginManager::maxGatewayUser = atoi(Zebra::global["maxGatewayUser"].c_str());
	}

	//设置日志级别
	Zebra::logger->setLevel(Zebra::global["log"]);
	//设置写本地日志文件
	if ("" != Zebra::global["logfilename"]){
		Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
		Zebra::logger->removeConsoleLog();
	}

	Zebra_Startup();

	FLService::getInstance().main();
	FLService::delInstance();

	return EXIT_SUCCESS;
}
