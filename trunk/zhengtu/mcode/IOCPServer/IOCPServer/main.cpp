#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "IOCPServer.h"
#include "GameServer.h"
#include "LuaHttpServer.h"

#define PACK_TYPE_STRESS_TEST		100

#pragma pack(1)

struct PACKET_PlayerDrawEvent	: public FS_PACKET
{
	BYTE Type;					// 0 - ��Ҫ���ظ��Լ�, 1 - ����Ҫ���Լ�
	DWORD dwClientSendTime;		// �ͻ��˷��͵�ʱ��

	void Create()
	{
		nID = PACK_TYPE_STRESS_TEST;
		nSize = sizeof(PACKET_PlayerDrawEvent);
	}
};


#pragma pack()

double dwSendCount = 0;
double dwReceiveCount = 0;
double dwProcessCount = 0;
double dwInvalidPack = 0;


class CMyGameServer : public CGameServer
{
public:
	CMyGameServer()
	{

	}

	virtual ~CMyGameServer()
	{

	}

	virtual int OnAccept(WORD nClient)
	{
		int a = 0;

		return 0;
	}

	virtual int OnClose(WORD nClient)
	{
		int a = 0;

		return 0;
	}

	virtual int OnSend(WORD nClient, char* pBuffer, DWORD dwLen)
	{
		int a = 0;

		dwSendCount++;
		//iocpServer.DisconnectClient(nClient);

		return 0;
	}

	virtual int OnProcessPacket(WORD nClient, FS_PACKET* pPacket)
	{
		dwReceiveCount++;

		HandlePacket(nClient, pPacket);

		return 0;
	}

	virtual int OnError(WORD nClient, int iError)
	{

		return 0;
	}

	void HandlePacket(int nClient, const FS_PACKET* pPacket)
	{
		switch(pPacket->nID)
		{
		case PID_SOCKET_DISCONNECT:
			{
				int ser = 0;
				dwInvalidPack++;
			}
			break;

		case PACK_TYPE_STRESS_TEST:
			{
				PACKET_PlayerDrawEvent *pEvent = (PACKET_PlayerDrawEvent*)pPacket;
				if (pEvent->Type == 0)
				{
					// �㲥��ȫ���ͻ���
					this->SendDataToAll((char*)pPacket, pPacket->nSize);
				}
				else if (pEvent->Type == 1)
				{
					// �㲥��ȫ���ͻ���,���ǲ�����������
					this->SendDataToOther(nClient, (char*)pPacket, pPacket->nSize);
				}
				else
				{
					// ���ط�����
					this->SendData(nClient, (char*)pPacket, pPacket->nSize);
					dwProcessCount++;
				}
			}
			break;

		default:
			{
				dwInvalidPack++;
				printf("UnHandled packet (not a problem): %i\n", pPacket->nID);
			}
			break;
		}
	}	
};




CMyGameServer testServer;
extern double dwTotalSize;

CLuaHttpServer httpServer;

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // ����ڴ�й©
	//GetTickCount���شӲ���ϵͳ����������������������
	//������ӳ�ʼ��
	srand( GetTickCount() );

	//httpServer.Initialize(1024, 10000);

	testServer.Initialize(1024, 10000);

	while (1)
	{
		Sleep(500);

		system("cls");//�����Ļ
		char szBuffer[2048];
		sprintf(szBuffer, 
			"���ӿͻ�����: %d\n"
			"��������ͳ��: %.0f\n"
			"��������ͳ��: %.0f\n"
			"��������ͳ��: %.0f\n"
			"��Ч����ͳ��: %.0f\n"
			"���ݴ�Сͳ��: %.0f\n",
			testServer.GetCurClientNum(),
			dwReceiveCount,
			dwSendCount,
			dwProcessCount,
			dwInvalidPack,
			dwTotalSize);
		printf(szBuffer);

		//const FS_PACKET* pPacket = NULL;
		

		//iocpServer.DeletePacket(&pPacket);
	}
}