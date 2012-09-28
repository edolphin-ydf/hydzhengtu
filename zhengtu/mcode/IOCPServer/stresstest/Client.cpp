#include ".\client.h"
#include <crtdbg.h>
#include "event.h"
#include "log.h"

//#define OPEN_LOG

WORD wDisProb = 0;		// 断开的概率
WORD wRecProb = 10000;	// 重练的概率

CClient::CClient(void) : m_nPort(10000)
{
	// _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// 检测内存泄漏
	m_pFSClient = NULL;
	m_pTotalInfo = NULL;
	m_szIP[0] = '\0';
	sData = NULL;
}

CClient::~CClient(void)
{
	if(m_pFSClient)
	{
		delete m_pFSClient;
		m_pFSClient = NULL;
	}
}

// 游戏初始化，返回0退出游戏
BOOL CClient::Init(const char* strSrvIP, UINT nPort, SConfigData* pCfgData)
{
	strcpy(m_szIP, strSrvIP);
	m_nPort = nPort;
	m_pFSClient = FSSocketFactory::CreateClient();
	if( !m_pFSClient->Connect(m_szIP, m_nPort) )
	{
		//printf("连接服务器失败!\n");
		return FALSE;
	}
	m_pTotalInfo = &(pCfgData->sTotalInfo);

	return TRUE;
}

BOOL CClient::Reconnect()
{
	if(m_pFSClient && !m_pFSClient->Connect(m_szIP, m_nPort) )
	{
		//printf("连接服务器失败!\n");
		return FALSE;
	}

	return TRUE;
}


DWORD CClient::HandleNetPack()
{
	if ( m_pFSClient && m_pFSClient->IsConnect() )
	{
		WORD nRnd = rand() % 10000;
		if (nRnd < wDisProb)
		{
			m_pFSClient->Disconnect();
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


	// 处理网络消息
	int revlen = 0;
	const FS_PACKET* pPacket = (const FS_PACKET*)m_pFSClient->GetPacket(revlen);
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
			//PACKET_StressTest *pEvent = (PACKET_StressTest*)pPacket;
			DWORD elapse = GetTickCount() - pPacket->dwClientSendTime;
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
	case 1:
		SendTestPack(2);
		break;
	case 2:
		SendTestPack(3);
		break;
	case 3:
		m_pFSClient->Disconnect();
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

	m_pFSClient->DeletePacket((const char **)&pPacket);

	return 0;
}

void CClient::SendTestPack(int aid)
{
	binStream.Clear();
	FS_PACKET *packet = new FS_PACKET;
	packet->nID = aid;
	packet->dwClientSendTime = GetTickCount();
	int len = sizeof(FS_PACKET);
	binStream.WriteInt(len);
	binStream.WriteData(packet,sizeof(FS_PACKET));
	m_pFSClient->SendPacket((const char *)binStream.GetBufferPtr(),binStream.GetBufferLen());
	delete packet;
}
