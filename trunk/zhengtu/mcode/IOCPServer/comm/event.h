#pragma once

// ѹ�����Գ���ṹ
#define PACK_TYPE_STRESS_TEST		0

#pragma pack(1)

struct PACKET_StressTest : public FS_PACKET
{
	BYTE Type;					// 0 - ��Ҫ���ظ��Լ�, 1 - ����Ҫ���Լ�
	DWORD dwClientSendTime;		// �ͻ��˷��͵�ʱ��
};

#pragma pack()




// ���߳���ṹ
#define PACK_TYPE_PLAYER_DRAW		100

struct PACKET_PlayerDrawEvent	: public FS_PACKET
{
	BYTE Type;					// 0 - ��Ҫ���ظ��Լ�, 1 - ����Ҫ���Լ�
	DWORD dwClientSendTime;		// �ͻ��˷��͵�ʱ��

	int iX1;		// Ŀ������
	int iY1;
	int iX2;		// Ŀ������
	int iY2;

	void Create()
	{
		nID = PACK_TYPE_PLAYER_DRAW;
		nSize = sizeof(PACKET_PlayerDrawEvent);
	}
};