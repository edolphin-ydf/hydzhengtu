/*
 �ļ��� : Server.h
 ����ʱ�� : 2012/9/20
 ���� : hyd
 ���� : �������Ŀ�ܻ���
*/
#ifndef __Server_H__
#define __Server_H__

#include "Common.h"
#include "Node.h"
#include "Mutex.h"
#include "Timer.h"
#include "Socket.h"

class Server : public CNode
{
public:
	Timer  _one_sec_; // �붨ʱ��

	//����������
	virtual ~Server() { serviceInst = NULL; };

	/**
	* \brief ���¶�ȡ�����ļ���ΪHUP�źŵĴ�����
	*
	* ȱʡʲô���鶼���ɣ�ֻ�Ǽ����һ��������Ϣ�����������������ɵ�����
	*
	*/
	virtual void reloadConfig()
	{
	}

	/**
	* \brief �ж���ѭ���Ƿ����
	*
	* �������true�����������ص�
	*
	* \return ��ѭ���Ƿ����
	*/
	bool isTerminate() const
	{
		return terminate;
	}

	/**
	* \brief ������ѭ����Ҳ���ǽ������ص�����
	*
	*/
	void Terminate()
	{
		terminate = true;
	}

	void main();

	/**
	* \brief ���ط����ʵ��ָ��
	*
	* \return �����ʵ��ָ��
	*/
	static Server *serviceInstance()
	{
		return serviceInst;
	}

	//Properties env;        /**< �洢��ǰ����ϵͳ�Ļ������� */

protected:

	/**
	* \brief ���캯��
	*
	*/
	Server(const std::string &name) : name(name),_one_sec_(1)
	{
		serviceInst = this;

		terminate = false;
	}

	virtual bool init();

	/**
	* \brief ȷ�Ϸ�������ʼ���ɹ��������������ص�����
	*
	* \return ȷ���Ƿ�ɹ�
	*/
	virtual bool validate()
	{
		return true;
	}

	/**
	* \brief �����������ص���������Ҫ���ڼ�������˿ڣ��������false���������򣬷���true����ִ�з���
	*
	* \return �ص��Ƿ�ɹ�
	*/
	virtual bool serviceCallback() = 0;

	/**
	* \brief �������������򣬻�����Դ�����麯����������Ҫʵ���������
	*
	*/
	virtual void final() = 0;

private:

	static Server *serviceInst;/**< ���Ψһʵ��ָ�룬���������࣬��ʼ��Ϊ��ָ�� */

	std::string name;          /**< �������� */
	bool terminate;            /**< ���������� */
	
};

/**
* \brief Server_MTCP�࣬��װ�˷���������ģ�飬���Է���Ĵ���һ�����������󣬵ȴ��ͻ��˵�����
* ����ͬʱ��������˿�
*/
class Server_MTCP : private CNode
{
public:
	typedef std::map<int,WORD> Sock2Port;
	typedef Sock2Port::value_type Sock2Port_value_type;
	typedef Sock2Port::iterator Sock2Port_iterator;
	typedef Sock2Port::const_iterator Sock2Port_const_iterator;

	Server_MTCP(const std::string &name);
	~Server_MTCP() ;

	bool bind(const std::string &name,const WORD port);
	int accept(Sock2Port &res);//���յ������Ӹ���

private:

	static const int T_MSEC =2100;      /**< ��ѯ��ʱ������ */
	static const int MAX_WAITQUEUE = 2000;  /**< ���ȴ����� */

	std::string name;                   /**< ���������� */
	Sock2Port mapper;                   //�÷������󶨵����ж˿��б�
	Mutex mlock;
	std::vector<struct mypollfd> pfds;

}; 

/**
* \brief �����������
* ʵ���������������ܴ��룬�����Ƚ�ͨ��һ��
*/
class Server_MNet : public Server
{

public:

	/**
	* \brief ����������
	*/
	virtual ~Server_MNet() { 
		//instance = NULL; 
	};

	/**
	* \brief ���ݵõ���TCP/IP���ӻ�ȡһ����������
	* \param sock TCP/IP�׽ӿ�
	* \param srcPort ���ڷ��������˶���˿ڣ��������ָ�����������Ǹ��󶨶˿�
	* \return �½�������������
	*/
	virtual void newTCPTask(const SOCKET sock,const WORD srcPort) = 0;

	/**
	* \brief �󶨷���ĳ���˿�
	* \param name ���󶨶˿�����
	* \param port ���󶨵Ķ˿�
	* \return ���Ƿ�ɹ�
	*/
	bool bind(const std::string &name,const WORD port)
	{
		if (tcpServer)
			return tcpServer->bind(name,port);
		else
			return false;
	}

protected:

	/**
	* \brief ���캯��
	* �ܱ����Ĺ��캯����ʵ����Singleton���ģʽ����֤��һ��������ֻ��һ����ʵ��
	* \param name ����
	*/
	Server_MNet(const std::string &name) : Server(name)
	{
		//instance = this;

		serviceName = name;
		tcpServer = NULL;
	}

	bool init();
	bool serviceCallback();
	void final();

private:
	//static Server_MNet *instance;    /**< ���Ψһʵ��ָ�룬���������࣬��ʼ��Ϊ��ָ�� */
	std::string serviceName;         /**< ������������� */
	Server_MTCP *tcpServer;          /**< TCP������ʵ��ָ�� */
};

#endif
