/*
 文件名 : Server.h
 创建时间 : 2012/9/20
 作者 : hyd
 功能 : 服务器的框架基类
*/
#ifndef __Server_H__
#define __Server_H__

#include "Common.h"
#include "Node.h"
#include "Mutex.h"
#include "Timer.h"
#include "Socket.h"

class Server : public CNode
{
public:
	Timer  _one_sec_; // 秒定时器

	//虚析构函数
	virtual ~Server() { serviceInst = NULL; };

	/**
	* \brief 重新读取配置文件，为HUP信号的处理函数
	*
	* 缺省什么事情都不干，只是简单输出一个调试信息，重载这个函数干想干的事情
	*
	*/
	virtual void reloadConfig()
	{
	}

	/**
	* \brief 判断主循环是否结束
	*
	* 如果返回true，将结束主回调
	*
	* \return 主循环是否结束
	*/
	bool isTerminate() const
	{
		return terminate;
	}

	/**
	* \brief 结束主循环，也就是结束主回调函数
	*
	*/
	void Terminate()
	{
		terminate = true;
	}

	void main();

	/**
	* \brief 返回服务的实例指针
	*
	* \return 服务的实例指针
	*/
	static Server *serviceInstance()
	{
		return serviceInst;
	}

	//Properties env;        /**< 存储当前运行系统的环境变量 */

protected:

	/**
	* \brief 构造函数
	*
	*/
	Server(const std::string &name) : name(name),_one_sec_(1)
	{
		serviceInst = this;

		terminate = false;
	}

	virtual bool init();

	/**
	* \brief 确认服务器初始化成功，即将进入主回调函数
	*
	* \return 确认是否成功
	*/
	virtual bool validate()
	{
		return true;
	}

	/**
	* \brief 服务程序的主回调函数，主要用于监听服务端口，如果返回false将结束程序，返回true继续执行服务
	*
	* \return 回调是否成功
	*/
	virtual bool serviceCallback() = 0;

	/**
	* \brief 结束服务器程序，回收资源，纯虚函数，子类需要实现这个函数
	*
	*/
	virtual void final() = 0;

private:

	static Server *serviceInst;/**< 类的唯一实例指针，包括派生类，初始化为空指针 */

	std::string name;          /**< 服务名称 */
	bool terminate;            /**< 服务结束标记 */
	
};

/**
* \brief Server_MTCP类，封装了服务器监听模块，可以方便的创建一个服务器对象，等待客户端的连接
* 可以同时监听多个端口
*/
class Server_MTCP : private CNode
{
public:
	typedef std::map<int,WORD> Sock2Port;
	typedef Sock2Port::value_type Sock2Port_value_type;
	typedef Sock2Port::iterator Sock2Port_iterator;
	typedef Sock2Port::const_iterator Sock2Port_const_iterator;

	Server_MTCP(const std::string &name);
	~Server_MTCP() ;

	bool bind(const std::string &name,const WORD port);
	int accept(Sock2Port &res);//接收到的连接个数

private:

	static const int T_MSEC =2100;      /**< 轮询超时，毫秒 */
	static const int MAX_WAITQUEUE = 2000;  /**< 最大等待队列 */

	std::string name;                   /**< 服务器名称 */
	Sock2Port mapper;                   //该服务器绑定的所有端口列表
	Mutex mlock;
	std::vector<struct mypollfd> pfds;

}; 

/**
* \brief 网络服务器类
* 实现了网络服务器框架代码，这个类比较通用一点
*/
class Server_MNet : public Server
{

public:

	/**
	* \brief 虚析构函数
	*/
	virtual ~Server_MNet() { 
		//instance = NULL; 
	};

	/**
	* \brief 根据得到的TCP/IP连接获取一个连接任务
	* \param sock TCP/IP套接口
	* \param srcPort 由于服务器绑定了多个端口，这个参数指定连接来自那个绑定端口
	* \return 新建立的连接任务
	*/
	virtual void newTCPTask(const SOCKET sock,const WORD srcPort) = 0;

	/**
	* \brief 绑定服务到某个端口
	* \param name 待绑定端口名称
	* \param port 待绑定的端口
	* \return 绑定是否成功
	*/
	bool bind(const std::string &name,const WORD port)
	{
		if (tcpServer)
			return tcpServer->bind(name,port);
		else
			return false;
	}

protected:

	/**
	* \brief 构造函数
	* 受保护的构造函数，实现了Singleton设计模式，保证了一个进程中只有一个类实例
	* \param name 名称
	*/
	Server_MNet(const std::string &name) : Server(name)
	{
		//instance = this;

		serviceName = name;
		tcpServer = NULL;
	}

	bool init();
	bool serviceCallback();
	void final();

private:
	//static Server_MNet *instance;    /**< 类的唯一实例指针，包括派生类，初始化为空指针 */
	std::string serviceName;         /**< 网络服务器名称 */
	Server_MTCP *tcpServer;          /**< TCP服务器实例指针 */
};

#endif
