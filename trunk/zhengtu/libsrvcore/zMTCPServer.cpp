/**
* \brief 实现类zMTCPServer
*
* 
*/
#include <zebra/srvEngine.h>

/**
* \brief 构造函数，用于构造一个服务器zMTCPServer对象
* \param name 服务器名称
*/
zMTCPServer::zMTCPServer(const std::string &name) : name(name)
{
	Zebra::logger->debug("zMTCPServer::zMTCPServer");

	pfds.resize(8);
}

/**
* \brief 析构函数，用于销毁一个zMTCPServer对象
*/
zMTCPServer::~zMTCPServer() 
{
	Zebra::logger->debug("zMTCPServer::~zMTCPServer");

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
bool zMTCPServer::bind(const std::string &name,const WORD port) 
{
	Zebra::logger->debug("zMTCPServer::bind");
	zMutex_scope_lock scope_lock(mlock);
	struct sockaddr_in addr;
	SOCKET sock;

	for(Sock2Port_const_iterator it = mapper.begin(); it != mapper.end(); it++)
	{
		if (it->second == port)
		{
			Zebra::logger->warn("端口 %u 已经绑定服务");
			return false;
		}
	}

	sock = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (INVALID_SOCKET == sock) 
	{
		Zebra::logger->error("创建套接口失败");
		return false;
	}

	//设置套接口为可重用状态
	int reuse = 1;
	if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse))) 
	{
		Zebra::logger->error("不能设置套接口为可重用状态");
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

	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	int retcode = ::bind(sock,(struct sockaddr *) &addr,sizeof(addr));
	if (-1 == retcode) 
	{
		Zebra::logger->error("不能绑定服务器端口");
		::closesocket(sock);
		return false;
	}

	retcode = ::listen(sock,MAX_WAITQUEUE);
	if (-1 == retcode) 
	{
		Zebra::logger->error("监听套接口失败");
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

	Zebra::logger->info("服务器 %s:%u 端口初始化绑定成功",name.c_str(),port);

	return true;
}

/**
* \brief 接受客户端的连接
* \param res 返回的连接集合
* \return 接收到的连接个数
*/
int zMTCPServer::accept(Sock2Port &res)
{
	Zebra::logger->debug("zMTCPServer::accept");
	zMutex_scope_lock scope_lock(mlock);
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

