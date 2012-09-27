#include <iostream>
#include <tchar.h>
#include <windows.h>
#include <string.h>

#include ".\gameworld.h"
#include <crtdbg.h>
#include "event.h"
#include "log.h"

CGameWorld::CGameWorld(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// 检测内存泄漏
}

CGameWorld::~CGameWorld(void)
{
}




// 游戏初始化，返回0退出游戏
BOOL CGameWorld::Init(int nMaxClient, short nPort)
{
	m_pServer = FSSocketFactory::CreateServer();

	if (!m_pServer->Initialize(nMaxClient, nPort))
	{
		printf("创建服务器失败!\n");
		getchar();
		return 0;
	}
	printf("服务器监听....!\n");

	return 1;
}


char szBuffer[256];

// 处理游戏逻辑，返回0退出游戏
// fTick: 帧间隔时间
int CGameWorld::OnTick(float fTick)
{
	static DWORD sCount2 = 0;
	static DWORD sCount = 0;

	sCount2++;

	// 处理网络消息
	BOOL bProcessed = FALSE;
	const FS_PACKET* pPacket = NULL;
	int nClient = 0;
	if ( pPacket = m_pServer->GetPacket(nClient) )
	{
		switch(pPacket->nID)
		{
		case PID_SOCKET_DISCONNECT:
			{
				int ser = 0;
			}
			break;

		case PACK_TYPE_PLAYER_DRAW:
			{
				PACKET_PlayerDrawEvent *pEvent = (PACKET_PlayerDrawEvent*)pPacket;
				if (pEvent->Type == 0)
				{
					// 广播给全部客户端
					m_pServer->BroadcastPacket(pPacket);
				}
				else if (pEvent->Type == 1)
				{
					// 广播给全部客户端,但是不包括发送者
					m_pServer->BroadcastPacket(pPacket, nClient);
				}
				else
				{
					// 返回发送者
					m_pServer->SendPacket(pPacket, nClient);
				}
				//pEvent->dwServerRevTransTime = GetTickCount();

				bProcessed = TRUE;
				sCount++;
// 				char ch[256];
// 
// 				// 发送时间
// 				sprintf(ch, "ClientSendTime: %d\n", pEvent->dwClientSendTime);
// 				printf(ch);
// 
// 				// 转发时间
// 				sprintf(ch, "ServerRevTransTime: %d\t", pEvent->dwServerRevTransTime);
// 				printf(ch);
// 
// 				// 时差
// 				sprintf(ch, "DelayTime: %d\n", pEvent->dwServerRevTransTime - pEvent->dwClientSendTime);
// 				printf(ch);
			}
			break;

		default:
			printf("Unhandled packet (not a problem): %i\n", pPacket->nID);
			break;
		}
	}

	m_pServer->DeletePacket(&pPacket);

	static DWORD sLastTick = GetTickCount();
	DWORD dwCurTick = GetTickCount();
	static DWORD sPackNumInSec = 0;
	static DWORD dwAvrPackNumInSec = 0;
	static DWORD dwTotalPackNum = 0;
	static DWORD sLastProcNum = 0;
	if (dwCurTick - sLastTick > 1000)
	{
		sLastTick = dwCurTick;

		if (bProcessed)
		{
			DWORD dwTemp = sCount - sLastProcNum;
			sPackNumInSec = dwTemp > sPackNumInSec ? dwTemp : sPackNumInSec;
			dwAvrPackNumInSec += dwTemp;
			dwTotalPackNum++;
		}

		sLastProcNum = sCount;
	}

	

	if (sCount2 % 300 == 0)
	{
		system("cls");
		DWORD avrPackNum = dwTotalPackNum == 0 ? 0 : dwAvrPackNumInSec/dwTotalPackNum;
		sprintf(szBuffer,
			"  当前客户端数量: %d\n"
			"    总处理的包数: %d\n"
			"    未处理的包数: %d\n"
			"    每秒处理包数: %d\n"
			"平均每秒处理包数: %d\n",
			m_pServer->GetCurClientNum(),
			sCount,
			m_pServer->GetUnProcessPackNum(),
			sPackNumInSec,
			avrPackNum);
		printf(szBuffer);
	}

	//printf("总处理的包数: %d\n", sCount);
	//printf("未处理的包数: %d\n", m_pServer->GetUnProcessPackNum());
	// printf("此包处理时间: %d\n\n", fTick2-fTick1);

	return 1;
}



// 游戏退出事件，释放游戏资源
void CGameWorld::OnQuit(void)
{
	delete m_pServer;
}
