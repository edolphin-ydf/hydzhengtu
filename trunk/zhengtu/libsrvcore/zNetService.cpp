/**
* \brief ʵ�����������
*
* 
*/
#include <zebra/srvEngine.h>

#include <iostream>
//#include <ext/numeric>

zNetService *zNetService::instance = NULL;

/**
* \brief ��ʼ������������
*
* ʵ��<code>zService::init</code>���麯��
*
* \param port �˿�
* \return �Ƿ�ɹ�
*/
bool zNetService::init(WORD port)
{
	Zebra::logger->debug("zNetService::init");
	if (!zService::init())
		return false;

	//��ʼ��������
	tcpServer = new zTCPServer(serviceName);
	if (NULL == tcpServer)
		return false;
	if (!tcpServer->bind(serviceName,port))
		return false;

	// [ranqd] ��ʼ�������߳�
	pAcceptThread = new zAcceptThread( this, serviceName );
	if( pAcceptThread == NULL )
		return false;
	if(!pAcceptThread->start())
		return false;

	Zebra::logger->debug("zNetService::init bind(%s:%u)",serviceName.c_str(),port);
	return true;
}

/**
* \brief ��������������ص�����
*
* ʵ���麯��<code>zService::serviceCallback</code>����Ҫ���ڼ�������˿ڣ��������false���������򣬷���true����ִ�з���
*
* \return �ص��Ƿ�ɹ�
*/
bool zNetService::serviceCallback()
{
	// [ranqd] ÿ�����һ�������������
	zRTime currentTime;
	currentTime.now();
	if( _one_sec_( currentTime ) )
	{
		zIocp::getInstance().UpdateNetLog();
	}
	Sleep(10);
	return true;
}
/**
* \brief �����������������
*
* ʵ�ִ��麯��<code>zService::final</code>��������Դ
*
*/
void zNetService::final()
{
	Zebra::logger->info("zNetService::final");
	SAFE_DELETE(tcpServer);
}

