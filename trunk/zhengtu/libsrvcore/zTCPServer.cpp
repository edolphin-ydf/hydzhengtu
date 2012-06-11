/**
 * \brief ʵ����zTCPServer
 *
 * 
 */
#include <zebra/srvEngine.h>

/**
 * \brief ���캯��,���ڹ���һ��������zTCPServer����
 * \param name ����������
 */
zTCPServer::zTCPServer(const std::string &name)
: name(name),
  sock(INVALID_SOCKET)
{
  Zebra::logger->info("zTCPServer::zTCPServer");
}

/**
 * \brief ��������,��������һ��zTCPServer����
 *
 *
 */
zTCPServer::~zTCPServer() 
{
  Zebra::logger->info("zTCPServer::~zTCPServer");

  if (INVALID_SOCKET != sock) 
  {
    ::shutdown(sock,0x02);
    ::closesocket(sock);
    sock = INVALID_SOCKET;
  }
}

/**
 * \brief �󶨼�������ĳһ���˿�
 * \param name �󶨶˿�����
 * \param port ����󶨵Ķ˿�
 * \return ���Ƿ�ɹ�
 */
bool zTCPServer::bind(const std::string &name,const WORD port)
{
  Zebra::logger->info("zTCPServer::bind");
  struct sockaddr_in addr;

  if (INVALID_SOCKET != sock) 
  {
    Zebra::logger->error("�����������Ѿ���ʼ��");;
    return false;
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
    sock = INVALID_SOCKET;
    return false;
  }

  //�����׽ӿڷ��ͽ��ջ���,���ҷ������ı�����accept֮ǰ����
  int window_size = 128 * 1024;
  if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&window_size,sizeof(window_size)))
  {
    ::closesocket(sock);
    return false;
  }
  if (-1 == ::setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&window_size,sizeof(window_size)))
  {
    ::closesocket(sock);
        sock = INVALID_SOCKET;
    return false;
  }

  bzero(&addr,sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  int retcode = ::bind(sock,(struct sockaddr *) &addr,sizeof(addr));
  if (-1 == retcode) 
  {
    char tmpc[MAX_PATH];
	sprintf(tmpc, "�����ʼ��ʧ�ܣ��˿� %u �Ѿ���ռ�ã�",port);
	MessageBox( NULL, tmpc, "����", MB_ICONERROR );
    ::closesocket(sock);
        sock = INVALID_SOCKET;
    return false;
  }

  retcode = ::listen(sock,MAX_WAITQUEUE);
  if (-1 == retcode) 
  {
	  char tmpc[MAX_PATH];
	  sprintf(tmpc, "�����׽ӿ�ʧ�ܣ��˿� %u �Ѿ���ռ�ã�",port);
	  MessageBox( NULL, tmpc, "����", MB_ICONERROR );
    ::closesocket(sock);
    sock = INVALID_SOCKET;
    return false;
  }

  Zebra::logger->info("��ʼ�� %s:%u �ɹ�",name.c_str(),port);

  return true;
}

/**
 * \brief ���ܿͻ��˵�����
 *
 *
 * \param addr ���صĵ�ַ
 * \return ���صĿͻ����׽ӿ�
 */
int zTCPServer::accept(struct sockaddr_in *addr)
{
//  Zebra::logger->info("zTCPServer::accept");
  int len = sizeof(struct sockaddr_in);
  bzero(addr,sizeof(struct sockaddr_in));
//// [ranqd] �˴�����poll����׽ӿ�״̬�������������̹߳�����޷������˳�
//  struct pollfd pfd;
//  pfd.fd = sock;
//  pfd.events = POLLIN;
//  pfd.revents = 0;
//  int rc = ::poll(&pfd,1,T_MSEC);
//  if (1 == rc && (pfd.revents & POLLIN))
//    //׼���ý���
    return ::WSAAccept(sock,(struct sockaddr *)addr,&len,NULL, NULL );

  //return -1;
}

