#pragma once

// 压力测试程序结构
#define PACK_TYPE_STRESS_TEST		0

#pragma pack(1)

struct PACKET_StressTest : public FS_PACKET
{
	BYTE Type;					// 0 - 需要返回给自己, 1 - 不需要给自己
	DWORD dwClientSendTime;		// 客户端发送的时间
};

#pragma pack()




// 画线程序结构
#define PACK_TYPE_PLAYER_DRAW		100

struct PACKET_PlayerDrawEvent	: public FS_PACKET
{
	BYTE Type;					// 0 - 需要返回给自己, 1 - 不需要给自己
	DWORD dwClientSendTime;		// 客户端发送的时间

	int iX1;		// 目标坐标
	int iY1;
	int iX2;		// 目标坐标
	int iY2;

	void Create()
	{
		nID = PACK_TYPE_PLAYER_DRAW;
		nSize = sizeof(PACKET_PlayerDrawEvent);
	}
};