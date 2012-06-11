/**
* \brief 实现网络服务器的框架代码
*
* 
*/
#include <zebra/srvEngine.h>

#include <iostream>
#include <string>
#include <deque>
//#include <ext/numeric>

zSubNetService *zSubNetService::subNetServiceInst = NULL;

/**
* \brief 管理服务器的连接客户端类
*
*/
class SuperClient : public zTCPBufferClient
{

public:

	friend class zSubNetService;

	/**
	* \brief 构造函数
	*
	*/
	SuperClient() : zTCPBufferClient("管理服务器客户端"),verified(false)
	{
		Zebra::logger->error("SuperClient::SuperClient");
	}

	/**
	* \brief 析构函数
	*
	*/
	~SuperClient() {};

	void run();
	bool msgParse_Startup(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

private:

	bool verified;      /**< 是否已经通过了管理服务器的验证 */

};

/**
* \brief 重载zThread中的纯虚函数,是线程的主回调函数,用于处理接收到的指令
*
*/
void SuperClient::run()
{
	zTCPBufferClient::run();
	//与管理服务器之间的连接断开,需要关闭服务器
	zSubNetService::subNetServiceInstance()->Terminate();
}

/**
* \brief 解析来自管理服务器的关于启动的指令
*
* \param pNullCmd 待处理的指令
* \param nCmdLen 指令长度
* \return 解析是否成功
*/
bool SuperClient::msgParse_Startup(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
	using namespace Cmd::Super;

	//Zebra::logger->error("cmd from super");

	switch(pNullCmd->para)
	{
	case PARA_GAMETIME:
		{
			t_GameTime *ptCmd = (t_GameTime *)pNullCmd;

			//Zebra::logger->error("PARA_GAMETIME %lu",ptCmd->qwGameTime);
			Zebra::qwGameTime = ptCmd->qwGameTime;
			return true;
		}
		break;
	case PARA_STARTUP_RESPONSE:
		{
			t_Startup_Response *ptCmd = (t_Startup_Response *)pNullCmd;

			//Zebra::logger->error("PARA_STARTUP_RESPONSE %d,%d",ptCmd->wdServerID,ptCmd->wdPort);
			zSubNetService::subNetServiceInstance()->setServerInfo(ptCmd);
			return true;
		}

		break;
	case PARA_STARTUP_SERVERENTRY_NOTIFYME:
		{
			t_Startup_ServerEntry_NotifyMe *ptCmd = (t_Startup_ServerEntry_NotifyMe *)pNullCmd;

			//Zebra::logger->error("PARA_STARTUP_SERVERENTRY_NOTIFYME size = %d ",ptCmd->size );
			for(WORD i = 0; i < ptCmd->size; i++)
			{
				//需要一个容器来管理这些服务器列表
				zSubNetService::subNetServiceInstance()->addServerEntry(ptCmd->entry[i]);
			}
			verified = true;
			return true;
		}

		break;
	case PARA_STARTUP_SERVERENTRY_NOTIFYOTHER:
		{
			t_Startup_ServerEntry_NotifyOther *ptCmd = (t_Startup_ServerEntry_NotifyOther *)pNullCmd;

			//Zebra::logger->error("PARA_STARTUP_SERVERENTRY_NOTIFYOTHER");
			//需要一个容器来管理这些服务器列表
			zSubNetService::subNetServiceInstance()->addServerEntry(ptCmd->entry);
			// 发送命令到SuperServer
			return sendCmd(ptCmd,nCmdLen);
		}
		break;
	}

	//Zebra::logger->error("SuperClient::msgParse_Startup(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}

/**
* \brief 解析来自管理服务器的指令
*
* \param pNullCmd 待处理的指令
* \param nCmdLen 指令长度
* \return 解析是否成功
*/
bool SuperClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
	Zebra::logger->error("?? SuperClient::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif

	switch(pNullCmd->cmd)
	{
	case Cmd::Super::CMD_STARTUP:
		if (msgParse_Startup(pNullCmd,nCmdLen)) return true;
		break;
	default:
		if (zSubNetService::subNetServiceInstance()->msgParse_SuperService(pNullCmd,nCmdLen)) return true;
		break;
	}

	Zebra::logger->error("SuperClient::msgParse(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
	return false;
}

/**
* \brief 构造函数
* 
* \param name 名称
* \param wdType 服务器类型
*/
zSubNetService::zSubNetService(const std::string &name,const WORD wdType) : zNetService(name),superClient(NULL)
{
	Zebra::logger->debug("zSubNetService::zSubNetService");

	subNetServiceInst = this;

	bzero(pstrIP,sizeof(pstrIP));
	superClient  = new SuperClient();
	wdServerID   = 0;
	wdServerType = wdType;
	wdPort        = 0;
}

/**
* \brief 虚析构函数
*
*/
zSubNetService::~zSubNetService()
{
	Zebra::logger->debug("zSubNetService::~zSubNetService");
	serverList.clear();

	SAFE_DELETE(superClient);

	subNetServiceInst = NULL;
}

/**
* \brief 初始化网络服务器程序
*
* 实现纯虚函数<code>zService::init</code>
* 建立到管理服务器的连接,并得到服务器信息
*
* \return 是否成功
*/
bool zSubNetService::init()
{
	Zebra::logger->debug("zSubNetService::init");
try_agin:
	//建立到管理服务器的连接
	while (!superClient->connect(Zebra::global["server"].c_str(),atoi(Zebra::global["port"].c_str())))
	{
		printf("连接管理服务器失败(%s:%s),2秒后重试.....\n",Zebra::global["server"].c_str(),Zebra::global["port"].c_str());
		Sleep(2000);
	}
	printf("连接管理服务器成功！\n");

	//发送登陆管理服务器的指令
	Cmd::Super::t_Startup_Request tCmd;
	tCmd.wdServerType = wdServerType;
	strncpy(tCmd.pstrIP,pstrIP,sizeof(tCmd.pstrIP));
	if (!superClient->sendCmd(&tCmd,sizeof(tCmd)))
	{
		printf("向管理服务器发送登陆指令失败，2秒后重试.....\n");
		Sleep(2000);
		goto try_agin;
	}


	//等待管理服务器返回信息
	while(!superClient->verified)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = superClient->pSocket->recvToCmd(pstrCmd,sizeof(pstrCmd),true);    
		if (-1 == nCmdLen)
		{
			printf("等待管理服务器返回信息失败，2秒后重试.....\n");
			Sleep(2000);
			goto try_agin;
		}
		else if (nCmdLen > 0)
		{
			if (!superClient->msgParse((Cmd::t_NullCmd *)pstrCmd,nCmdLen))
			{
				printf("从管理服务器收到错误的指令(%d:%d)，启动失败！\n",((Cmd::t_NullCmd *)pstrCmd)->cmd, ((Cmd::t_NullCmd *)pstrCmd)->para);
				return false;
			}
		}
	}

	Zebra::logger->info("zSubNetService::init %d,%d,%s:%d",wdServerType,wdServerID,pstrIP,wdPort);

	//建立线程与管理服务器交互
	superClient->start();

	//调用真实的初始化函数
	return zNetService::init(wdPort);
}

/**
* \brief 确认服务器初始化成功,即将进入主回调函数
*
* 向服务器发送t_Startup_OK指令来确认服务器启动成功
*
* \return 确认是否成功
*/
bool zSubNetService::validate()
{
	Cmd::Super::t_Startup_OK tCmd;

	Zebra::logger->debug("zSubNetService::validate");  
	tCmd.wdServerID = wdServerID;
	return superClient->sendCmd(&tCmd,sizeof(tCmd));
}

/**
* \brief 结束网络服务器
*
* 实现纯虚函数<code>zService::final</code>
*
*/
void zSubNetService::final()
{
	Zebra::logger->debug("zSubNetService::final");
	zNetService::final();

	//关闭到管理服务器的连接
	superClient->final();
	superClient->join();
	superClient->close();
}

/**
* \brief 向管理服务器发送指令
*
* \param pstrCmd 待发送的指令
* \param nCmdLen 待发送指令的大小
* \return 发送是否成功
*/
bool zSubNetService::sendCmdToSuperServer(const void *pstrCmd,const int nCmdLen)
{
	Zebra::logger->debug("zSubNetService::sendCmdToSuperServer");
	return superClient->sendCmd(pstrCmd,nCmdLen);
}

/**
* \brief 根据管理服务器返回信息,设置服务器的信息
*
* \param ptCmd 管理服务器返回信息
*/
void zSubNetService::setServerInfo(const Cmd::Super::t_Startup_Response *ptCmd)
{  
	Zebra::logger->info("zSubNetService::setServerInfo(%d,%s:%d) %s",wdServerID,ptCmd->pstrIP,wdPort,pstrIP);
	wdServerID = ptCmd->wdServerID;
	wdPort     = ptCmd->wdPort;
	strncpy(pstrIP,ptCmd->pstrIP,sizeof(pstrIP));  
}

/**
* \brief 添加关联服务器信息到一个容器中
*
*/
void zSubNetService::addServerEntry(const Cmd::Super::ServerEntry &entry)
{
	Zebra::logger->error("-----zSubNetService::addServerEntry(%d,%s:%d)------",entry.wdServerID,entry.pstrIP,entry.wdPort,entry.state);

	mlock.lock();
	//首先查找有没有重复的
	std::deque<Cmd::Super::ServerEntry>::iterator it;
	bool found = false;
	for(it = serverList.begin(); it != serverList.end(); it++)
	{
		if (entry.wdServerID == it->wdServerID)
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		//已经存在只是更新
		(*it) = entry;
	}
	else
	{
		//还不存在,需要新建立一个节点
		serverList.push_back(entry);
	}
	mlock.unlock();
}

/**
* \brief 查找相关服务器信息
*
* \param wdServerID 服务器编号
* \return 服务器信息
*/
const Cmd::Super::ServerEntry *zSubNetService::getServerEntryById(const WORD wdServerID)
{
	Zebra::logger->debug("zSubNetService::getServerEntryById(%d)",wdServerID);
	Cmd::Super::ServerEntry *ret = NULL;
	std::deque<Cmd::Super::ServerEntry>::iterator it;
	mlock.lock();
	for(it = serverList.begin(); it != serverList.end(); it++)
	{
		if (wdServerID == it->wdServerID)
		{
			ret = &(*it);
			break;
		}
	}
	mlock.unlock();
	return ret;
}

/**
* \brief 查找相关服务器信息
*
* \param wdServerType 服务器类型
* \return 服务器信息
*/
const Cmd::Super::ServerEntry *zSubNetService::getServerEntryByType(const WORD wdServerType)
{
	Zebra::logger->debug("zSubNetService::getServerEntryByType(type=%d)",wdServerType);
	Cmd::Super::ServerEntry *ret = NULL;
	std::deque<Cmd::Super::ServerEntry>::iterator it;
	mlock.lock();
	for(it = serverList.begin(); it != serverList.end(); it++)
	{
		Zebra::logger->debug("服务器信息：%d,%d",wdServerType,it->wdServerType);
		if (wdServerType == it->wdServerType)
		{
			ret = &(*it);
			break;
		}
	}
	mlock.unlock();
	return ret;
}

/**
* \brief 查找相关服务器信息
*
* \param wdServerType 服务器类型
* \param prev 上一个服务器信息
* \return 服务器信息
*/
const Cmd::Super::ServerEntry *zSubNetService::getNextServerEntryByType(const WORD wdServerType,const Cmd::Super::ServerEntry **prev)
{
	Zebra::logger->debug("zSubNetService::getNextServerEntryByType");
	Cmd::Super::ServerEntry *ret = NULL;
	bool found = false;
	std::deque<Cmd::Super::ServerEntry>::iterator it;
	mlock.lock();
	for(it = serverList.begin(); it != serverList.end(); it++)
	{
		Zebra::logger->debug("服务器信息：%d,%d",wdServerType,it->wdServerType);
		if (wdServerType == it->wdServerType)
		{
			if (NULL == prev
				|| found)
			{
				ret = &(*it);
				break;
			}
			else if (!found
				&& (*prev)->wdServerID == it->wdServerID)
			{
				found = true;
			}
		}
	}
	mlock.unlock();
	return ret;
}

