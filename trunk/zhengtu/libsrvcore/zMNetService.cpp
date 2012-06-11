/**
 * \brief ʵ�����������
 *
 * 
 */
#include <zebra/srvEngine.h>

#include <iostream>
#include <string>
//#include <ext/numeric>

zMNetService *zMNetService::instance = NULL;

/**
 * \brief ��ʼ������������
 *
 * ʵ��<code>zService::init</code>���麯��
 *
 * \return �Ƿ�ɹ�
 */
bool zMNetService::init()
{
  Zebra::logger->debug("zMNetService::init");

  if (!zService::init())
    return false;

  //��ʼ��������
  tcpServer = new zMTCPServer(serviceName);
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
bool zMNetService::serviceCallback()
{
  Zebra::logger->debug("zMNetService::serviceCallback");
  // [ranqd] ÿ�����һ�������������
  zRTime currentTime;
  currentTime.now();
  if( _one_sec_( currentTime ) )
  {
	  zIocp::getInstance().UpdateNetLog();
  }

  zMTCPServer::Sock2Port res;
  if (tcpServer->accept(res) > 0) 
  {
    for(zMTCPServer::Sock2Port_const_iterator it = res.begin(); it != res.end(); it++)
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
void zMNetService::final()
{
  Zebra::logger->debug("zMNetService::final");
  SAFE_DELETE(tcpServer);
}

