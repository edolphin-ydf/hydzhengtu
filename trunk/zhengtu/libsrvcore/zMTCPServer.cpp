/**
* \brief ʵ����zMTCPServer
*
* 
*/
#include <zebra/srvEngine.h>

/**
* \brief ���캯�������ڹ���һ��������zMTCPServer����
* \param name ����������
*/
zMTCPServer::zMTCPServer(const std::string &name) : name(name)
{
	Zebra::logger->debug("zMTCPServer::zMTCPServer");

	pfds.resize(8);
}

/**
* \brief ������������������һ��zMTCPServer����
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
* \brief �󶨼�������ĳһ���˿�
* \param name �󶨶˿�����
* \param port ����󶨵Ķ˿�
* \return ���Ƿ�ɹ�
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
			Zebra::logger->warn("�˿� %u �Ѿ��󶨷���");
			return false;
		}
	}

	sock = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (INVALID_SOCKET == sock) 
	{
		Zebra::logger->error("�����׽ӿ�ʧ��");
		return false;
	}

	//�����׽ӿ�Ϊ������״̬
	int reuse = 1;
	if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse))) 
	{
		Zebra::logger->error("���������׽ӿ�Ϊ������״̬");
		::closesocket(sock);
		return false;
	}

	//�����׽ӿڷ��ͽ��ջ��壬���ҷ������ı�����accept֮ǰ����
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
		Zebra::logger->error("���ܰ󶨷������˿�");
		::closesocket(sock);
		return false;
	}

	retcode = ::listen(sock,MAX_WAITQUEUE);
	if (-1 == retcode) 
	{
		Zebra::logger->error("�����׽ӿ�ʧ��");
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

	Zebra::logger->info("������ %s:%u �˿ڳ�ʼ���󶨳ɹ�",name.c_str(),port);

	return true;
}

/**
* \brief ���ܿͻ��˵�����
* \param res ���ص����Ӽ���
* \return ���յ������Ӹ���
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

