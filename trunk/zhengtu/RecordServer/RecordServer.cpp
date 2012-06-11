/**
 * \brief zebra��Ŀ����������,���ڴ���������Ͷ�ȡ����
 *
 */

#include "RecordServer.h"

zDBConnPool *RecordService::dbConnPool = NULL;

RecordService *RecordService::instance = NULL;

/**
 * \brief ��ʼ���������������
 *
 * ʵ�����麯��<code>zService::init</code>
 *
 * \return �Ƿ�ɹ�
 */
bool RecordService::init()
{
  Zebra::logger->debug("RecordService::init");

  dbConnPool = zDBConnPool::newInstance(NULL);
  if (NULL == dbConnPool
      || !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
  {
   MessageBox(NULL,"�������ݿ�ʧ��","RecordServer",MB_ICONERROR);
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
  //Zebra::logger->debug("%s",pstrIP);

  if (!zSubNetService::init())
  {
    return false;
  }

  
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
void RecordService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
  Zebra::logger->debug("RecordService::newTCPTask");
  RecordTask *tcpTask = new RecordTask(taskPool,sock,addr);
  if (NULL == tcpTask)
    //�ڴ治��,ֱ�ӹر�����
    ::closesocket(sock);
  else if (!taskPool->addVerify(tcpTask))
  {
    //�õ���һ����ȷ����,��ӵ���֤������
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
bool RecordService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  Zebra::logger->error("RecordService::msgParse_SuperService(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

/**
 * \brief �������������
 *
 * ʵ���˴��麯��<code>zService::final</code>
 *
 */
void RecordService::final()
{
  Zebra::logger->debug("RecordService::final");
  while(!RecordSessionManager::getInstance().empty())
  {
    zThread::msleep(10);
  }
  if (taskPool)
  {
    taskPool->final();
    SAFE_DELETE(taskPool);
  }
  zSubNetService::final();

  RecordSessionManager::delInstance();

  zDBConnPool::delInstance(&dbConnPool);

  Zebra::logger->debug("RecordService::final");
}

/**
 * \brief ��ȡ�����ļ�
 *
 */
class RecordConfile:public zConfile
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
 * \brief ���¶�ȡ�����ļ�,ΪHUP�źŵĴ�����
 *
 */
void RecordService::reloadConfig()
{
  Zebra::logger->debug("RecordService::reloadConfig");
  RecordConfile rc;
  rc.parse("RecordServer");
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

  fprintf(stderr,"here\n");

  Zebra::logger=new zLogger("RecordServer");

  fprintf(stderr,"here2\n");

  //����ȱʡ����

  //���������ļ�����
  fprintf(stderr,"here3\n");
  RecordConfile rc;
  if (!rc.parse("RecordServer"))
  {
	fprintf(stderr,"here4\n");
    return EXIT_FAILURE;
  }

  fprintf(stderr,"here5\n");

  //ָ���⿪��
  if (Zebra::global[std::string("cmdswitch")] == "true")
  {
    fprintf(stderr,"here6\n");
    zTCPTask::analysis._switch = true;
    zTCPClient::analysis._switch=true;
  }
  else
  {
    fprintf(stderr,"here7\n");
    zTCPTask::analysis._switch = false;
    zTCPClient::analysis._switch=false;
  }

  fprintf(stderr,"here8\n");

  //������־����
  Zebra::logger->setLevel(Zebra::global["log"]);
  //����д������־�ļ�
  if ("" != Zebra::global["logfilename"]){
    Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
        Zebra::logger->removeConsoleLog();
    }

  fprintf(stderr,"here9\n");

  Zebra_Startup();

  fprintf(stderr,"here10\n");
  
  RecordService::getInstance().main();
  fprintf(stderr,"here11\n");
  RecordService::delInstance();
  fprintf(stderr,"here12\n");

  return EXIT_SUCCESS;
}

int __stdcall WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	return 1;
}

