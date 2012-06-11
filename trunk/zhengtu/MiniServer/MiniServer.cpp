/**
 * \brief zebra��Ŀ�Ʒѷ�����
 *
 */

#include "MiniServer.h"

zDBConnPool *MiniService::dbConnPool = NULL;
MiniService *MiniService::instance = NULL;
DBMetaData* MiniService::metaData = NULL;

/**
 * \brief ��ʼ���������������
 *
 * ʵ�����麯��<code>zService::init</code>
 *
 * \return �Ƿ�ɹ�
 */
bool MiniService::init()
{
  dbConnPool = zDBConnPool::newInstance(NULL);
  if (NULL == dbConnPool
      || !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
  {
    MessageBox(NULL,"�������ݿ�ʧ��","MiniServer",MB_ICONERROR);
    return false;
  }

  metaData = DBMetaData::newInstance("");

  if (NULL == metaData
      || !metaData->init(Zebra::global["mysql"]))
  {
    MessageBox(NULL,"�������ݿ�ʧ��","MiniServer",MB_ICONERROR);
    return false;
  }

  //��ʼ�������̳߳�
  int state = state_none;
  to_lower(Zebra::global["threadPoolState"]);
  if ("repair" == Zebra::global["threadPoolState"]
      || "maintain" == Zebra::global["threadPoolState"])
    state = state_maintain;
  taskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
  if (NULL == taskPool
      || !taskPool->init())
    return false;

  strncpy(pstrIP,zSocket::getIPByIfName(Zebra::global["ifname"].c_str()),MAX_IP_LENGTH - 1);

  MiniTimeTick::getInstance().start();
  if (!MiniHall::getMe().init()) return false;

  if (!zSubNetService::init())
    return false;

  return true;
}

/**
 * \brief �½���һ����������
 *
 * ʵ�ִ��麯��<code>zNetService::newTCPTask</code>
 *
 * \param sock TCP/IP����
 * \param addr ��ַ
 */
void MiniService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
  MiniTask *tcpTask = new MiniTask(taskPool,sock,addr);
  if (NULL == tcpTask)
    //�ڴ治�㣬ֱ�ӹر�����
    ::closesocket(sock);
  else if (!taskPool->addVerify(tcpTask))
  {
    //�õ���һ����ȷ���ӣ���ӵ���֤������
    SAFE_DELETE(tcpTask);
  }
}

/**
 * \brief �������Թ����������ָ��
 *
 * ��Щָ�������غ͹��������������ָ��<br>
 * ʵ�����麯��<code>zSubNetService::msgParse_SuperService</code>
 *
 * \param pNullCmd ��������ָ��
 * \param nCmdLen ��������ָ���
 * \return �����Ƿ�ɹ�
 */
bool MiniService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  using namespace Cmd::Super;

  return true;
}

/**
 * \brief �������������
 *
 * ʵ���˴��麯��<code>zService::final</code>
 *
 */
void MiniService::final()
{
  MiniTimeTick::getInstance().final();
  MiniTimeTick::getInstance().join();
  MiniTimeTick::delInstance();

  if (taskPool)
  {
    SAFE_DELETE(taskPool);
  }

  MiniTaskManager::delInstance();
  MiniUserManager::delInstance();

  zSubNetService::final();

  zDBConnPool::delInstance(&dbConnPool);
  Zebra::logger->debug("MiniService::final");
}

/**
 * \brief ��ȡ�����ļ�
 *
 */
class MiniConfile:public zConfile
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
 * \brief ���¶�ȡ�����ļ���ΪHUP�źŵĴ�����
 *
 */
void MiniService::reloadConfig()
{
  Zebra::logger->debug("MiniService::reloadConfig");
  MiniConfile rc;
  rc.parse("MiniServer");
  //ָ���⿪��
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

/**
 * \brief ���������
 *
 * \param argc ��������
 * \param argv �����б�
 * \return ���н��
 */
int main(int argc,char **argv)
{
  Zebra::logger=new zLogger("MiniServer");

  //����ȱʡ����

  //���������ļ�����
  MiniConfile rc;
  if (!rc.parse("MiniServer"))
    return EXIT_FAILURE;

  //ָ���⿪��
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

  //������־����
  Zebra::logger->setLevel(Zebra::global["log"]);
  //����д������־�ļ�
  if ("" != Zebra::global["logfilename"]){
    Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
        Zebra::logger->removeConsoleLog();
  }

  Zebra_Startup();
  
  MiniService::getInstance().main();
  MiniService::delInstance();

  return EXIT_SUCCESS;
}
