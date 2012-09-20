
#include "Server.h"
#include "Iocp.h"
#include <signal.h>

Server *Server::serviceInst = NULL;
/**
 * \brief CTRL + C等信号的处理函数,结束程序
 *
 * \param signum 信号编号
 */
static void ctrlcHandler(int signum)
{
  printf("ctrlcHandler");
  fprintf(stderr,"ctrlcHandler\n");
  //如果没有初始化zService实例,表示出错
  Server *instance = Server::serviceInstance();
  //结束主循环
  instance->Terminate();
}

/**
 * \brief 初始化服务器程序,子类需要实现这个函数
 *
 * \return 是否成功
 */
bool Server::init()
{
 printf("Server::init\n");
  
  //初始化随机数
  srand(time(NULL));
  
  return true;
}

/**
 * \brief 服务程序框架的主函数
 */
void Server::main()
{
  printf("Server::main\n");
  //初始化程序,并确认服务器启动成功,设置结束信号和结束时调用的函数
  if(signal(SIGTERM  , ctrlcHandler)==SIG_ERR)
  {
	fprintf(stderr,"信号设置失败\n");
  }
  
  //初始化，确认服务器初始化成功，即将进入主回调函数
  if (init()
  && validate())
  {
    //运行主回调线程
    while(!isTerminate())
    {
      if (!serviceCallback())//服务程序的主回调函数，主要用于监听服务端口，如果返回false将结束程序，返回true继续执行服务
      {                      //这儿主要是输出网络流量，必定返回真，所以主服务会一直循环，除非人为结束
        break;
      }
    }
  }

  //结束程序,释放相应的资源
  final();
}


/**
* \brief 构造函数，用于构造一个服务器Server_MTCP对象
* \param name 服务器名称
*/
Server_MTCP::Server_MTCP(const std::string &name) : name(name)
{
	printf("Server_MTCP::Server_MTCP\n");

	pfds.resize(8);
}

/**
* \brief 析构函数，用于销毁一个Server_MTCP对象
*/
Server_MTCP::~Server_MTCP() 
{
	printf("Server_MTCP::~Server_MTCP\n");

	for(Sock2Port_const_iterator it = mapper.begin(); it != mapper.end(); it++)
	{
		if (-1 != it->first)
		{
			::shutdown(it->first,0x02);
			::closesocket(it->first);
		}
	}
	mapper.clear();
}

/**
* \brief 绑定监听服务到某一个端口
* \param name 绑定端口名称
* \param port 具体绑定的端口
* \return 绑定是否成功
*/
bool Server_MTCP::bind(const std::string &name,const WORD port) 
{
	printf("Server_MTCP::bind\n");
	Mutex_scope_lock scope_lock(mlock);
	struct sockaddr_in addr;
	SOCKET sock;

	//检查端口是否已经绑定服务
	for(Sock2Port_const_iterator it = mapper.begin(); it != mapper.end(); it++)
	{
		if (it->second == port)
		{
			printf("端口 %u 已经绑定服务\n");
			return false;
		}
	}

	sock = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (INVALID_SOCKET == sock) 
	{
		printf("创建套接口失败\n");
		return false;
	}

	//设置套接口为可重用状态
	int reuse = 1;
	if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse))) 
	{
		printf("不能设置套接口为可重用状态\n");
		::closesocket(sock);
		return false;
	}

	//设置套接口发送接收缓冲，并且服务器的必须在accept之前设置
	int window_size = 128 * 1024;
	if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&window_size,sizeof(window_size)))
	{
		::closesocket(sock);
		return false;
	}
	if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&window_size,sizeof(window_size)))
	{
		::closesocket(sock);
		return false;
	}

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	int retcode = ::bind(sock,(struct sockaddr *) &addr,sizeof(addr));
	if (-1 == retcode) 
	{
		printf("不能绑定服务器端口\n");
		::closesocket(sock);
		return false;
	}

	retcode = ::listen(sock,MAX_WAITQUEUE);
	if (-1 == retcode) 
	{
		printf("监听套接口失败\n");
		::closesocket(sock);
		return false;
	}


	pfds[mapper.size()].fd = sock;
	pfds[mapper.size()].events = POLLIN;
	pfds[mapper.size()].revents = 0;

	mapper.insert(Sock2Port_value_type(sock,port));

	if (mapper.size() > pfds.size())
	{
		pfds.resize(mapper.size() + 8);
	}

	printf("服务器 %s:%u 端口初始化绑定成功\n",name.c_str(),port);

	return true;
}

/**
* \brief 接受客户端的连接
* \param res 返回的连接集合
* \return 接收到的连接个数
*/
int Server_MTCP::accept(Sock2Port &res)
{
	printf("Server_MTCP::accept\n");
	Mutex_scope_lock scope_lock(mlock);
	int retval = 0;


	for(Sock2Port::size_type i = 0; i < mapper.size(); i++)
		pfds[i].revents = 0;
	int rc = ::poll(&pfds[0],mapper.size(),T_MSEC);
	if (rc > 0)
	{
		for(Sock2Port::size_type i = 0; i < mapper.size(); i++)
		{
			if (pfds[i].revents & POLLIN)
			{
				res.insert(Sock2Port_value_type(::WSAAccept(pfds[i].fd,NULL,NULL, NULL,NULL ),mapper[pfds[i].fd]));
				retval++;
			}
		}
	}

	return retval;
}


//Server_MNet *Server_MNet::instance = NULL;

/**
 * \brief 初始化服务器程序
 *
 * 实现<code>zService::init</code>的虚函数
 *
 * \return 是否成功
 */
bool Server_MNet::init()
{
  printf("Server_MNet::init\n");

  if (!Server::init())
    return false;

  //初始化服务器
  tcpServer = new Server_MTCP(serviceName);
  if (NULL == tcpServer)
    return false;

  return true;
}

/**
 * \brief 网络服务程序的主回调函数
 *
 * 实现虚函数<code>zService::serviceCallback</code>，主要用于监听服务端口，如果返回false将结束程序，返回true继续执行服务
 *
 * \return 回调是否成功
 */
bool Server_MNet::serviceCallback()
{
  //printf("Server_MNet::serviceCallback\n");
  // [ranqd] 每秒更新一次网络流量监测
  RTime currentTime;
  currentTime.now();
  if( _one_sec_( currentTime ) )
  {
	  CIocp::getInstance().UpdateNetLog();
  }

  Server_MTCP::Sock2Port res;
  if (tcpServer->accept(res) > 0) 
  {
    for(Server_MTCP::Sock2Port_const_iterator it = res.begin(); it != res.end(); it++)
    {
      if (it->first >= 0)
      {
        //接收连接成功，处理连接
        newTCPTask(it->first,it->second);
      }
    }
  }

  return true;
}

/**
 * \brief 结束网络服务器程序
 *
 * 实现纯虚函数<code>zService::final</code>，回收资源
 *
 */
void Server_MNet::final()
{
  printf("Server_MNet::final\n");
  SAFE_DELETE(tcpServer);
}