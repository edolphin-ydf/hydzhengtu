#include ".\client.h"
#include <crtdbg.h>
#include "event.h"
#include "log.h"

//#define OPEN_LOG

WORD wDisProb = 0;		// �Ͽ��ĸ���
WORD wRecProb = 10000;	// �����ĸ���

CClient::CClient(void) : m_nPort(10000)
{
	// _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// ����ڴ�й©
	m_pClient = NULL;
	m_pTotalInfo = NULL;
	m_szIP[0] = '\0';
}

CClient::~CClient(void)
{
	delete m_pClient;
}

// ���Ͳ��԰�
void CClient::SendTestPack(int iSize)
{
	binStream.Clear();
	binStream.WriteWord(0xFFFF);	// header
	binStream.WriteDWord(iSize);	// size
	binStream.WriteDWord(PACK_TYPE_STRESS_TEST);	// command
	binStream.WriteByte(2);	// type
	binStream.WriteDWord(GetTickCount());	// tick
	int iCurSize = binStream.GetBufferLen();
	BYTE *pBuf = new BYTE[iSize-iCurSize];
	binStream.WriteData(pBuf, iSize-iCurSize);
	delete[] pBuf;

	FS_PACKET *packet = new FS_PACKET;
	//m_pClient->SendPacket((FS_PACKET*)binStream.GetBufferPtr());
	m_pClient->SendPacket(packet);
	delete packet;
	m_pTotalInfo->dwSendCount++;
}

// ��Ϸ��ʼ��������0�˳���Ϸ
BOOL CClient::Init(const char* strSrvIP, UINT nPort, STotalInfo* pTotalInfo)
{
	strcpy(m_szIP, strSrvIP);
	m_nPort = nPort;
	m_pClient = FSSocketFactory::CreateClient();
	if( !m_pClient->Connect(m_szIP, m_nPort) )
	{
		//printf("���ӷ�����ʧ��!\n");
		return FALSE;
	}
	m_pTotalInfo = pTotalInfo;

	return TRUE;
}

BOOL CClient::Reconnect()
{
	if( !m_pClient->Connect(m_szIP, m_nPort) )
	{
		//printf("���ӷ�����ʧ��!\n");
		return FALSE;
	}

	return TRUE;
}


DWORD CClient::HandleNetPack()
{
	if ( m_pClient->IsConnect() )
	{
		WORD nRnd = rand() % 10000;
		if (nRnd < wDisProb)
		{
			m_pClient->Disconnect();
		}
	}
	else
	{
		WORD nRnd = rand() % 10000;
		if (nRnd < wRecProb)
		{
			if ( !Reconnect() )
			{
				m_pTotalInfo->dwRecFailNum++;
			}
		}
	}


	// ����������Ϣ
	const FS_PACKET* pPacket = m_pClient->GetPacket();
	if (pPacket == NULL)
		return 0;

	switch(pPacket->nID)
	{
	case PID_SOCKET_DISCONNECT:
		{
			int i=0;
		}
		break;

	case PACK_TYPE_STRESS_TEST:
		{
			PACKET_StressTest *pEvent = (PACKET_StressTest*)pPacket;
			DWORD elapse = GetTickCount() - pEvent->dwClientSendTime;
			m_pTotalInfo->dwReceiveCnt++;
			m_pTotalInfo->dwTotalTick += elapse;
			if (elapse < m_pTotalInfo->dwMinTick)
			{
				m_pTotalInfo->dwMinTick = elapse;
			}
			if (elapse > m_pTotalInfo->dwMaxTick)
			{
				m_pTotalInfo->dwMaxTick = elapse;
			}

			m_pTotalInfo->dwCurElapse = elapse;
		}
		break;

	default:
		{
			m_pTotalInfo->dwInvalidPackCnt++;
			char ch[256] = "error";
			sprintf(ch,"Unhandled packet (not a problem): %i\n", pPacket->nID);
			OutputDebugString(ch);
		}
		break;
	}

	m_pClient->DeletePacket(&pPacket);

	return 0;
}
