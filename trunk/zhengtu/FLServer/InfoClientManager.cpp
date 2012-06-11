/**
* \brief �����������Ϣ�ɼ����ӵĿͻ��˹�������
*/


#include "FLServer.h"

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
	Zebra::logger->debug("InfoClientManager::init");
	infoClientPool = new zTCPClientTaskPool(atoi(Zebra::global["threadPoolClient"].c_str()));
	if (NULL == infoClientPool
		|| !infoClientPool->init())
		return false;

	Zebra::logger->debug("InfoServer: %s,%s",Zebra::global["InfoServer"].c_str(),Zebra::global["InfoPort"].c_str());
	infoClientPool->put(new InfoClient(Zebra::global["InfoServer"],atoi(Zebra::global["InfoPort"].c_str())));

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
		Zebra::logger->debug("InfoClientManager::timeAction");
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
	Zebra::logger->debug("InfoClientManager::add");
	if (infoClient)
	{
		mlock.lock();
		allClients.push_back(infoClient);
		mlock.unlock();
	}
}

/**
* \brief ���������Ƴ��Ͽ�������
* \param infoClient ���Ƴ�������
*/
void InfoClientManager::remove(InfoClient *infoClient)
{
	Zebra::logger->debug("InfoClientManager::remove");
	if (infoClient)
	{
		mlock.lock();
		InfoClientContainer::iterator it = find(allClients.begin(),allClients.end(),infoClient);
		if (it != allClients.end())
		{
			allClients.erase(it);
		}
		mlock.unlock();
	}
}

/**
* \brief ��ɹ����������ӹ㲥ָ��
* \param pstrCmd ���㲥��ָ��
* \param nCmdLen ���㲥ָ��ĳ���
*/
bool InfoClientManager::broadcast(const void *pstrCmd,int nCmdLen)
{
	Zebra::logger->debug("InfoClientManager::broadcast");
	bool retval = false;
	mlock.lock();
	for(InfoClientContainer::iterator it = allClients.begin(); it != allClients.end(); ++it)
	{
		retval = retval | (*it)->sendCmd(pstrCmd,nCmdLen);
	}
	mlock.unlock();
	return retval;
}

