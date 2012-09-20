/*
 �ļ��� : Iocp.h
 ����ʱ�� : 2012/9/20
 ���� : hyd
 ���� : 
*/
#ifndef __Iocp_H__
#define __Iocp_H__
#include "Common.h"
#include "Thread.h"
#include "Socket.h"
#include <vector>


class CIocpRecvThread : public CThread
{
public:
	CIocpRecvThread():CThread("IocpRecvThread"){};
	~CIocpRecvThread(){};

	void run();
};

class CIocp
{
public:
	CIocp();
	~CIocp();

	static CIocp &getInstance()
	{
		if (NULL == instance)
		{
			instance = new CIocp();
			instance->start();
		}

		return *instance;
	}
	static CIocp *instance;

	void start();

	DWORD m_dwThreadCount;

	std::vector<CIocpRecvThread*> m_RecvThreadList;
	HANDLE m_ComPort;
	void BindIocpPort(HANDLE hIo, CSocket* key);// ��IO�˿�
	void PostStatus(CSocket* key, LPOverlappedData lpOverLapped);
	void UpdateNetLog(); // �����������
};
#endif
