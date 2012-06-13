/**
* \brief ʵ�ֹ��������
*
* ��һ�����е����з��������й���
* 
*/

#include "SuperServer.h"

hash_map<int,std::vector<int> > serverSequence;

zDBConnPool *SuperService::dbConnPool = NULL;

SuperService *SuperService::instance = NULL;

/**
* \brief �����ݿ��л�ȡ��������Ϣ
*
* ������ݿ���û�й������������Ϣ,��Ҫ��ʼ��һ����¼
*
*/
bool SuperService::getServerInfo()
{
	static const dbCol col_define[] =
	{
		{"ID",zDBConnPool::DB_WORD,sizeof(WORD)},
		{"TYPE",zDBConnPool::DB_WORD,sizeof(WORD)},
		{"IP",zDBConnPool::DB_STR,sizeof(char[MAX_IP_LENGTH])},
		{"PORT",zDBConnPool::DB_WORD,sizeof(WORD)},
		{NULL,0,0}
	};
	struct
	{
		WORD wdServerID;
		WORD wdServerType;
		char pstrIP[MAX_IP_LENGTH];
		WORD wdPort;
	}
	data,*pData = NULL;
	char where[32];

	connHandleID handle = dbConnPool->getHandle();
	if ((connHandleID)-1 == handle)
	{
		Zebra::logger->error("���ܴ����ݿ����ӳػ�ȡ���Ӿ��");
		return false;
	}
	bzero(where,sizeof(where));
	_snprintf(where,sizeof(where) - 1,"`TYPE`=%u",SUPERSERVER);
	DWORD retcode = dbConnPool->exeSelect(handle,"`SERVERLIST`",col_define,where,NULL,(BYTE **)&pData);
	if ((DWORD)1 == retcode && pData)
	{
		//ֻ��һ�����������ļ�¼
		if (strcmp(pstrIP,pData->pstrIP) == 0)
		{
			wdServerID = pData->wdServerID;
			wdPort = pData->wdPort;
			SAFE_DELETE_VEC(pData);

			Zebra::logger->debug("ServerID=%u IP=%s Port=%u",wdServerID,pstrIP,wdPort);
		}
		else
		{
			Zebra::logger->error("���ݿ��еļ�¼�����ϣ�%s,%s",pstrIP,pData->pstrIP);
			SAFE_DELETE_VEC(pData);
			dbConnPool->putHandle(handle);
			return false;
		}
	}
	else
	{
		//��ѯ����,���߼�¼̫��
		Zebra::logger->error("�����ҵ�SUPERSERVER��¼,�������ݿ���SUPERSERVER��¼̫��.");
		SAFE_DELETE_VEC(pData);
		dbConnPool->putHandle(handle);
		return false;
	}
	dbConnPool->putHandle(handle);

	return true;
}

/**
* \brief ��ʼ���������������
*
* ʵ�ִ��麯��<code>zService::init</code>
*
* \return �Ƿ�ɹ�
*/
bool SuperService::init()
{
	initServerSequence();
	Zebra::logger->info("SuperService::init");

	dbConnPool = zDBConnPool::newInstance(NULL);
	string mysqlstr = Zebra::global["mysql"];
	if (NULL == dbConnPool
		|| !dbConnPool->putURL(0,mysqlstr.c_str(),false))
	{
		MessageBox(NULL,"�������ݿ�ʧ��","SuperServer",MB_ICONERROR);
		return false;
	}

	strncpy(pstrIP,zSocket::getIPByIfName(Zebra::global["ifname"].c_str()),MAX_IP_LENGTH - 1);
	Zebra::logger->debug("%s",pstrIP);

	if (!getServerInfo())
		return false;

	if (!FLClientManager::getInstance().init())
		return false;

	////---if (!InfoClientManager::getInstance().init())
	////---  return false;

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

	if (!zNetService::init(wdPort))
	{
		return false;
	}

	SuperTimeTick::getInstance().start();

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
void SuperService::newTCPTask(const SOCKET sock,const struct sockaddr_in *addr)
{
	ServerTask *tcpTask = new ServerTask(taskPool,sock,addr);
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
* \brief �������������
*
* ʵ�ִ��麯��<code>zService::final</code>
*
*/
void SuperService::final()
{
	SuperTimeTick::getInstance().final();
	SuperTimeTick::getInstance().join();
	SuperTimeTick::delInstance();

	if (taskPool)
	{
		taskPool->final();
		SAFE_DELETE(taskPool);
	}
	////---InfoClientManager::delInstance();

	zNetService::final();

	ServerManager::delInstance();

	FLClientManager::delInstance();

	RoleregCache::delInstance();

	zDBConnPool::delInstance(&dbConnPool);

	Zebra::logger->debug("SuperService::final");
}

/**
* \brief ��ȡ�����ļ�
*
*/
class SuperConfile:public zConfile
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
void SuperService::reloadConfig()
{
	Zebra::logger->debug("SuperService::reloadConfig");
	SuperConfile sc;
	sc.parse("SuperServer");
	//ָ���⿪��
	if (Zebra::global["cmdswitch"] == "true")
	{
		zTCPTask::analysis._switch = true;
	}
	else
	{
		zTCPTask::analysis._switch = false;
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
	Zebra::logger=new zLogger("SuperServer");

	//����ȱʡ����

	//���������ļ�����
	SuperConfile sc;
	if (!sc.parse("SuperServer"))
		return EXIT_FAILURE;

	//ָ���⿪��
	if (Zebra::global["cmdswitch"] == "true")
	{
		zTCPTask::analysis._switch = true;
	}
	else
	{
		zTCPTask::analysis._switch = false;
	}

	//������־����
	Zebra::logger->setLevel(Zebra::global["log"]);
	//����д������־�ļ�
	if ("" != Zebra::global["logfilename"]){
		Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
		Zebra::logger->removeConsoleLog();
	}

	Zebra_Startup();

	SuperService::getInstance().main();
	SuperService::delInstance();

	return EXIT_SUCCESS;
}

