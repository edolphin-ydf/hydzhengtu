
#include "Server.h"
#include "Iocp.h"
#include <signal.h>

Server *Server::serviceInst = NULL;
/**
 * \brief CTRL + C���źŵĴ�����,��������
 *
 * \param signum �źű��
 */
static void ctrlcHandler(int signum)
{
  printf("ctrlcHandler");
  fprintf(stderr,"ctrlcHandler\n");
  //���û�г�ʼ��zServiceʵ��,��ʾ����
  Server *instance = Server::serviceInstance();
  //������ѭ��
  instance->Terminate();
}

/**
 * \brief ��ʼ������������,������Ҫʵ���������
 *
 * \return �Ƿ�ɹ�
 */
bool Server::init()
{
 printf("Server::init\n");
  
  //��ʼ�������
  srand(time(NULL));
  
  return true;
}

/**
 * \brief ��������ܵ�������
 */
void Server::main()
{
  printf("Server::main\n");
  //��ʼ������,��ȷ�Ϸ����������ɹ�,���ý����źźͽ���ʱ���õĺ���
  if(signal(SIGTERM  , ctrlcHandler)==SIG_ERR)
  {
	fprintf(stderr,"�ź�����ʧ��\n");
  }
  
  //��ʼ����ȷ�Ϸ�������ʼ���ɹ��������������ص�����
  if (init()
  && validate())
  {
    //�������ص��߳�
    while(!isTerminate())
    {
      if (!serviceCallback())//�����������ص���������Ҫ���ڼ�������˿ڣ��������false���������򣬷���true����ִ�з���
      {                      //�����Ҫ����������������ض������棬�����������һֱѭ����������Ϊ����
        break;
      }
    }
  }

  //��������,�ͷ���Ӧ����Դ
  final();
}


/**
* \brief ���캯�������ڹ���һ��������Server_MTCP����
* \param name ����������
*/
Server_MTCP::Server_MTCP(const std::string &name) : name(name)
{
	printf("Server_MTCP::Server_MTCP\n");

	pfds.resize(8);
}

/**
* \brief ������������������һ��Server_MTCP����
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
* \brief �󶨼�������ĳһ���˿�
* \param name �󶨶˿�����
* \param port ����󶨵Ķ˿�
* \return ���Ƿ�ɹ�
*/
bool Server_MTCP::bind(const std::string &name,const WORD port) 
{
	printf("Server_MTCP::bind\n");
	Mutex_scope_lock scope_lock(mlock);
	struct sockaddr_in addr;
	SOCKET sock;

	//���˿��Ƿ��Ѿ��󶨷���
	for(Sock2Port_const_iterator it = mapper.begin(); it != mapper.end(); it++)
	{
		if (it->second == port)
		{
			printf("�˿� %u �Ѿ��󶨷���\n");
			return false;
		}
	}

	sock = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (INVALID_SOCKET == sock) 
	{
		printf("�����׽ӿ�ʧ��\n");
		return false;
	}

	//�����׽ӿ�Ϊ������״̬
	int reuse = 1;
	if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse))) 
	{
		printf("���������׽ӿ�Ϊ������״̬\n");
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

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	int retcode = ::bind(sock,(struct sockaddr *) &addr,sizeof(addr));
	if (-1 == retcode) 
	{
		printf("���ܰ󶨷������˿�\n");
		::closesocket(sock);
		return false;
	}

	retcode = ::listen(sock,MAX_WAITQUEUE);
	if (-1 == retcode) 
	{
		printf("�����׽ӿ�ʧ��\n");
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

	printf("������ %s:%u �˿ڳ�ʼ���󶨳ɹ�\n",name.c_str(),port);

	return true;
}

/**
* \brief ���ܿͻ��˵�����
* \param res ���ص����Ӽ���
* \return ���յ������Ӹ���
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
 * \brief ��ʼ������������
 *
 * ʵ��<code>zService::init</code>���麯��
 *
 * \return �Ƿ�ɹ�
 */
bool Server_MNet::init()
{
  printf("Server_MNet::init\n");

  if (!Server::init())
    return false;

  //��ʼ��������
  tcpServer = new Server_MTCP(serviceName);
  if (NULL == tcpServer)
    return false;

  return true;
}

/**
 * \brief ��������������ص�����
 *
 * ʵ���麯��<code>zService::serviceCallback</code>����Ҫ���ڼ�������˿ڣ��������false���������򣬷���true����ִ�з���
 *
 * \return �ص��Ƿ�ɹ�
 */
bool Server_MNet::serviceCallback()
{
  //printf("Server_MNet::serviceCallback\n");
  // [ranqd] ÿ�����һ�������������
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
        //�������ӳɹ�����������
        newTCPTask(it->first,it->second);
      }
    }
  }

  return true;
}

/**
 * \brief �����������������
 *
 * ʵ�ִ��麯��<code>zService::final</code>��������Դ
 *
 */
void Server_MNet::final()
{
  printf("Server_MNet::final\n");
  SAFE_DELETE(tcpServer);
}