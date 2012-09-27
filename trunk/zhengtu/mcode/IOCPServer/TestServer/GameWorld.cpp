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
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// ����ڴ�й©
}

CGameWorld::~CGameWorld(void)
{
}




// ��Ϸ��ʼ��������0�˳���Ϸ
BOOL CGameWorld::Init(int nMaxClient, short nPort)
{
	m_pServer = FSSocketFactory::CreateServer();

	if (!m_pServer->Initialize(nMaxClient, nPort))
	{
		printf("����������ʧ��!\n");
		getchar();
		return 0;
	}
	printf("����������....!\n");

	return 1;
}


char szBuffer[256];

// ������Ϸ�߼�������0�˳���Ϸ
// fTick: ֡���ʱ��
int CGameWorld::OnTick(float fTick)
{
	static DWORD sCount2 = 0;
	static DWORD sCount = 0;

	sCount2++;

	// ����������Ϣ
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
					// �㲥��ȫ���ͻ���
					m_pServer->BroadcastPacket(pPacket);
				}
				else if (pEvent->Type == 1)
				{
					// �㲥��ȫ���ͻ���,���ǲ�����������
					m_pServer->BroadcastPacket(pPacket, nClient);
				}
				else
				{
					// ���ط�����
					m_pServer->SendPacket(pPacket, nClient);
				}
				//pEvent->dwServerRevTransTime = GetTickCount();

				bProcessed = TRUE;
				sCount++;
// 				char ch[256];
// 
// 				// ����ʱ��
// 				sprintf(ch, "ClientSendTime: %d\n", pEvent->dwClientSendTime);
// 				printf(ch);
// 
// 				// ת��ʱ��
// 				sprintf(ch, "ServerRevTransTime: %d\t", pEvent->dwServerRevTransTime);
// 				printf(ch);
// 
// 				// ʱ��
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
			"  ��ǰ�ͻ�������: %d\n"
			"    �ܴ���İ���: %d\n"
			"    δ����İ���: %d\n"
			"    ÿ�봦�����: %d\n"
			"ƽ��ÿ�봦�����: %d\n",
			m_pServer->GetCurClientNum(),
			sCount,
			m_pServer->GetUnProcessPackNum(),
			sPackNumInSec,
			avrPackNum);
		printf(szBuffer);
	}

	//printf("�ܴ���İ���: %d\n", sCount);
	//printf("δ����İ���: %d\n", m_pServer->GetUnProcessPackNum());
	// printf("�˰�����ʱ��: %d\n\n", fTick2-fTick1);

	return 1;
}



// ��Ϸ�˳��¼����ͷ���Ϸ��Դ
void CGameWorld::OnQuit(void)
{
	delete m_pServer;
}
