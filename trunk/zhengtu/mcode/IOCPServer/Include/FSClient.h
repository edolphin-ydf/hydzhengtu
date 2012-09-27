#pragma once

#include "FSSocketPacket.h"

class FS_API FSClient
{
public:
	FSClient(int len = 0){  m_PacketSize=len; }
	virtual ~FSClient(void);

	// ���ӷ�������֮ǰ�������Initialize
	virtual bool Connect(const char* lpszServerIP, unsigned short nPort) = 0;

	// �Ͽ�����
	virtual void Disconnect() = 0;

	// ����Ϣ������ȡ��һ����Ϣ��ʹ������delete����Ϣ�����������û����Ϣ�򷵻�NULL
	virtual const char* GetPacket(int &len) = 0;

	// ����һ����Ϣ��bToSelf == TRUE ��ͨ�������ֱ�ӷ����Լ���Ĭ����ͨ�����紫��
	virtual bool SendPacket(const char* data,int len) = 0;

	//�ж��Ƿ�ͷ�������������״̬
	virtual bool IsConnect() = 0;

	// �ͷŰ�
	virtual void DeletePacket(const char** pPacket) = 0;

protected:
	int m_PacketSize;
};
