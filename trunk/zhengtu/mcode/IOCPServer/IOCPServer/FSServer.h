#pragma once

#include "FSSocketPacket.h"

class FSServer
{
public:
	virtual ~FSServer(void){}

	//��ʼ��
	virtual bool Initialize(int nMaxClient, short nPort) = 0;

	//��ȡһ���Ѿ��յ��ķ��
	virtual const FS_PACKET* GetPacket(int &nClient) = 0;

	//����һ�������ָ���Ŀͻ���
	virtual bool SendPacket(const FS_PACKET* pPacket, int nClient) = 0;

	//���ͷ�����������ӿͻ�
	virtual bool BroadcastPacket(const FS_PACKET* pPacket, int nExcludeClient = -1) = 0;

	//�Ͽ��ͻ���
	virtual void DisconnectClient(int nClient) = 0;

	//�ж�ĳ���ͻ����Ƿ�����״̬
	virtual bool IsConnect(int nClient) = 0;

	// ���δ�������Ϣ��
	virtual DWORD GetUnProcessPackNum() = 0;

	// ��ȡ��ǰ�ͻ������ӵ�����
	virtual DWORD GetCurClientNum() = 0;

	// �ͷŰ�
	virtual void DeletePacket(const FS_PACKET** pPacket) = 0;
};
