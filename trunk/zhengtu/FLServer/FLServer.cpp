/**
* \brief zebra��Ŀ��½������,�����½,�����ʺš������ȹ���
*
*/
#include "FLServer.h"

zDBConnPool * FLService::dbConnPool = NULL;
FLService *FLService::instance = NULL;

/**
* \brief ��ʼ���������������
*
* ʵ�����麯��<code>zService::init</code>
*
* \return �Ƿ�ɹ�
*/
bool FLService::init()
{
	Zebra::logger->debug("FLService::init");  

	dbConnPool = zDBConnPool::newInstance(NULL);

	if (NULL == dbConnPool
		|| !dbConnPool->putURL(0,Zebra::global["mysql"].c_str(),false))
	{
		MessageBox(NULL,"�������ݿ�ʧ��","FLServer",MB_ICONERROR);
		return false;
	}

	if (!zMNetService::init()) return false;
	if (!ServerACLSingleton::instance().init()) return false;
	////---if (!InfoClientManager::getInstance().init()) return false;  

	//��ʼ�������̳߳�
	int state = state_none;
	to_lower(Zebra::global["threadPoolState"]);
	if ("repair" == Zebra::global["threadPoolState"]
	|| "maintain" == Zebra::global["threadPoolState"])
		state = state_maintain;

	loginTaskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
	if (NULL == loginTaskPool
		|| !loginTaskPool->init())
		return false;

	serverTaskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
	if (NULL == serverTaskPool
		|| !serverTaskPool->init())
		return false;

	pingTaskPool = new zTCPTaskPool(atoi(Zebra::global["threadPoolServer"].c_str()),state);
	if (NULL == pingTaskPool
		|| !pingTaskPool->init())
		return false;

	login_port  = atoi(Zebra::global["login_port"].c_str());
	inside_port = atoi(Zebra::global["inside_port"].c_str());
	ping_port   = atoi(Zebra::global["ping_port"].c_str());

	if (!zMNetService::bind("��½�˿�",login_port)
		|| !zMNetService::bind("�ڲ�����˿�",inside_port)
		|| !zMNetService::bind("PING�˿�",ping_port))
	{
		return false;
	}

	FLTimeTick::getInstance().start();

	return true;
}

/**
* \brief �½���һ����������
* ʵ�ִ��麯��<code>zMNetService::newTCPTask</code>
* \param sock TCP/IP����
* \param srcPort ������Դ�˿�
* \return �µ���������
*/
void FLService::newTCPTask(const SOCKET sock,const WORD srcPort)
{  
	Zebra::logger->debug("FLService::newTCPTask");

	if (srcPort == login_port)
	{
		//�ͻ��˵�½��֤����
		zTCPTask *tcpTask = new LoginTask(loginTaskPool,sock);
		if (NULL == tcpTask)
			::closesocket(sock);
		else if (!loginTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else if (srcPort == inside_port)
	{
		//ÿ�����Ĺ������������
		zTCPTask *tcpTask = new ServerTask(serverTaskPool,sock);
		if (NULL == tcpTask)
			::closesocket(sock);
		else if (!serverTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else if (srcPort == ping_port)
	{
		// ��ȡPING�������б�
		zTCPTask *tcpTask = new PingTask(serverTaskPool,sock);
		if (NULL == tcpTask)
			::closesocket(sock);
		else if (!pingTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else
		::closesocket(sock);
}

/**
* \brief �������������
*
* ʵ���˴��麯��<code>zService::final</code>
*
*/
void FLService::final()
{
	zMNetService::final();

	ServerManager::delInstance();
	LoginManager::delInstance();
	GYListManager::delInstance();

	FLTimeTick::getInstance().final();
	FLTimeTick::getInstance().join();
	FLTimeTick::delInstance();

	////---InfoClientManager::delInstance();
}

/**
* \brief ��ȡ�����ļ�
*
*/
class LoginConfile:public zConfile
{
	bool parseYour(const xmlNodePtr node)
	{
		if (node)
		{
			xmlNodePtr child=parser.getChildNode(node,NULL);
			while(child)
			{
				if (strcmp((char *)child->name,"InfoServer")==0)
				{
					char buf[64];
					if (parser.getNodeContentStr(child,buf,64))
					{
						Zebra::global["InfoServer"]=buf;
						if (parser.getNodePropStr(child,"port",buf,64))
							Zebra::global["InfoPort"]=buf;
					}
				}
				else
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
void FLService::reloadConfig()
{
	LoginConfile sc;
	sc.parse("FLServer");
	if ("true" == Zebra::global["jpeg_passport"])
		FLService::getInstance().jpeg_passport = true;
	else
		FLService::getInstance().jpeg_passport = false;
	ServerACLSingleton::instance().init();
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
	Zebra::logger=new zLogger("FLServer");

	//����ȱʡ����
	Zebra::global["login_port"]    = "7000";
	Zebra::global["inside_port"]   = "7001";
	Zebra::global["ping_port"]     = "7002";
	Zebra::global["InfoServer"]    = "127.0.0.1";
	Zebra::global["InfoPort"]      = "9903";
	Zebra::global["jpeg_passport"] = "true";

	//���������ļ�����
	LoginConfile sc;
	if (!sc.parse("FLServer")) return EXIT_FAILURE;
	if ("true" == Zebra::global["jpeg_passport"])
		FLService::getInstance().jpeg_passport = true;
	else
		FLService::getInstance().jpeg_passport = false;
	if (atoi(Zebra::global["maxGatewayUser"].c_str()))
	{
		LoginManager::maxGatewayUser = atoi(Zebra::global["maxGatewayUser"].c_str());
	}

	//������־����
	Zebra::logger->setLevel(Zebra::global["log"]);
	//����д������־�ļ�
	if ("" != Zebra::global["logfilename"]){
		Zebra::logger->addLocalFileLog(Zebra::global["logfilename"]);
		Zebra::logger->removeConsoleLog();
	}

	Zebra_Startup();

	FLService::getInstance().main();
	FLService::delInstance();

	return EXIT_SUCCESS;
}
