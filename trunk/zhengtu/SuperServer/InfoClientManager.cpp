/**
 * \brief �����������Ϣ�ɼ����ӵĿͻ��˹�������
 */


#include "SuperServer.h"

/**
 * \brief ���Ψһʵ��ָ��
 */
InfoClientManager *InfoClientManager::instance = NULL;

/**
 * \brief ���캯��
 */
InfoClientManager::InfoClientManager()
{
  infoClientPool = NULL;
}

/**
 * \brief ��������
 */
InfoClientManager::~InfoClientManager()
{
  SAFE_DELETE(infoClientPool);
}

/**
 * \brief ��ʼ��������
 * \return ��ʼ���Ƿ�ɹ�
 */
bool InfoClientManager::init()
{
  infoClientPool = new zTCPClientTaskPool(atoi(Zebra::global["threadPoolClient"].c_str()));
  if (NULL == infoClientPool
      || !infoClientPool->init())
    return false;

  zXMLParser xml;
  if (!xml.initFile(Zebra::global["confdir"] + "loginServerList.xml"))
  {
    Zebra::logger->error("����ͳһ�û�ƽ̨InfoServer�б��ļ�ʧ��");
    return false;
  }
  xmlNodePtr root = xml.getRootNode("Zebra");
  if (root)
  {
    xmlNodePtr zebra_node = xml.getChildNode(root,"InfoServerList");
    while(zebra_node)
    {
      if (strcmp((char *)zebra_node->name,"InfoServerList") == 0)
      {
        xmlNodePtr node = xml.getChildNode(zebra_node,"server");
        while(node)
        {
          if (strcmp((char *)node->name,"server") == 0)
          {
            std::string InfoServer,InfoPort;
            
            InfoServer = "";
            InfoPort   = "";
            if (xml.getNodePropStr(node,"ip",InfoServer)
                && xml.getNodePropStr(node,"port",InfoPort))
            {
              Zebra::logger->debug("InfoServer: %s,%s",InfoServer.c_str(),InfoPort.c_str());
              infoClientPool->put(new InfoClient(InfoServer,atoi(InfoPort.c_str())));
            }
          }

          node = xml.getNextNode(node,NULL);
        }
      }

      zebra_node = xml.getNextNode(zebra_node,NULL);
    }
  }

  Zebra::logger->info("����ͳһ�û�ƽ̨InfoServer�б��ļ��ɹ�");
  return true;
}

/**
 * \brief ���ڼ���������ӵĶ�����������
 * \param ct ��ǰʱ��
 */
void InfoClientManager::timeAction(const zTime &ct)
{
  if (actionTimer.elapse(ct) > 4)
  {
    if (infoClientPool)
      infoClientPool->timeAction(ct);
    actionTimer = ct;
  }
}

/**
 * \brief ������������Ѿ��ɹ�������
 * \param infoClient ����ӵ�����
 */
void InfoClientManager::add(InfoClient *infoClient)
{
  if (infoClient)
  {
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    const_iter it = allClients.find(infoClient->getTempID());
    if (it == allClients.end())
    {
      allClients.insert(value_type(infoClient->getTempID(),infoClient));
      setter.insert(infoClient);
    }
  }
}

/**
 * \brief ���������Ƴ��Ͽ�������
 * \param infoClient ���Ƴ�������
 */
void InfoClientManager::remove(InfoClient *infoClient)
{
  if (infoClient)
  {
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    iter it = allClients.find(infoClient->getTempID());
    if (it != allClients.end())
    {
      allClients.erase(it);
      setter.erase(infoClient);
    }
  }
}

/**
 * \brief ��ɹ����������ӹ㲥ָ��
 * \param pstrCmd ���㲥��ָ��
 * \param nCmdLen ���㲥ָ��ĳ���
 */
bool InfoClientManager::broadcastOne(const void *pstrCmd,int nCmdLen)
{
  zRWLock_scope_rdlock scope_rdlock(rwlock);
  for(InfoClient_set::iterator it = setter.begin(); it != setter.end(); ++it)
  {
    if ((*it)->sendCmd(pstrCmd,nCmdLen))
      return true;
  }
  return false;
}

bool InfoClientManager::sendTo(const DWORD tempid,const void *pstrCmd,int nCmdLen)
{
  zRWLock_scope_rdlock scope_rdlock(rwlock);
  iter it = allClients.find(tempid);
  if (it == allClients.end())
    return false;
  else
    return it->second->sendCmd(pstrCmd,nCmdLen);
}

