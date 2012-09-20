/*
 �ļ��� : TcpClient.h
 ����ʱ�� : 2012/9/18
 ���� : hyd
 ���� : 
*/
#ifndef __TcpClient_H__
#define __TcpClient_H__

#include "Thread.h"
#include "Processor.h"
#include "CmdAnalysis.h"
#include "Socket.h"
/**
* \brief TCP�ͻ���
*
* ��װ��һЩTCP�ͻ��˵��߼������罨�����ӵȵȣ���ʵ��Ӧ���У���Ҫ��������࣬�����ؽ���ָ��ĺ���msgParse
*
*/
class CTCPClient : public CThread,public Processor
{

public:

	/**
	* \brief ���캯��������ʵ�����󣬳�ʼ�������Ա
	*
	*
	* \param name ����
	* \param ip ��ַ
	* \param port �˿�
	* \param compress �ײ����ݴ����Ƿ�֧��ѹ��
	*/
	CTCPClient(
		const std::string &name,
		const std::string &ip = "127.0.0.1",
		const WORD port = 80,
		const bool compress = false) 
		: CThread(name),ip(ip),port(port),pSocket(NULL),compress(compress) {};

	/**
	* \brief �������������ٶ���
	*
	*/
	~CTCPClient() 
	{
		close();
	}

	bool connect();

	/**
	* \brief ����һ������������TCP���ӣ�ָ����������IP��ַ�Ͷ˿�
	*
	*
	* \param ip ��������IP��ַ
	* \param port �������Ķ˿�
	* \return �����Ƿ�ɹ�
	*/
	bool connect(const char *ip,const WORD port)
	{
		this->ip = ip;
		this->port = port;
		return connect();
	}

	/**
	* \brief �رտͻ�������
	*
	*/
	virtual void close()
	{
		//SAFE_DELETE( pSocket );
		if( pSocket != NULL )
		{
			if(pSocket->SafeDelete( false ))
				delete pSocket;
			pSocket = NULL;
		}
	}

	virtual bool sendCmd(const void *pstrCmd,const int nCmdLen);

	/**
	* \brief ���÷�����IP��ַ
	*
	*
	* \param ip ���õķ�����IP��ַ
	*/
	void setIP(const char *ip)
	{
		this->ip = ip;
	}

	/**
	* \brief ��ȡ������IP��ַ
	*
	*
	* \return ���ص�ַ
	*/
	const char *getIP() const
	{
		return ip.c_str();
	}

	/**
	* \brief ���÷������˿�
	*
	*
	* \param port ���õķ������˿�
	*/
	void setPort(const WORD port)
	{
		this->port = port;
	}

	/**
	* \brief ��ȡ�������˿�
	*
	*
	* \return ���ض˿�
	*/
	const WORD getPort() const
	{
		return port;
	}

	virtual void run();
	//ָ�����
	static CmdAnalysis analysis;

protected:

	std::string ip;                  /**< ��������ַ */
	WORD port;                       /**< �������˿� */
	CSocket *pSocket;                /**< �ײ��׽ӿ� */

	const bool compress;             /**< �Ƿ�֧��ѹ�� */

}; 

class CTCPBufferClient : public CTCPClient
{

public:

	CTCPBufferClient(
		const std::string &name,
		const std::string &ip = "127.0.0.1",
		const WORD port = 80,
		const bool compress = false,
		const int usleep_time = 50000) 
		: CTCPClient(name,ip,port,compress),usleep_time(usleep_time),_buffered(false) { }

	void close()
	{
		sync();
		CTCPClient::close();
	}

	void run();
	bool sendCmd(const void *pstrCmd,const int nCmdLen);
	void setUsleepTime(const int utime)
	{
		usleep_time = utime;
	}

private :

	bool ListeningRecv();
	bool ListeningSend();
	void sync();

	int usleep_time;
	volatile bool _buffered;

};


#endif