#include ".\client.h"
#include <crtdbg.h>
#include "event.h"
#include "define.h"
#include <vector>
#include <process.h>


//#define OPEN_LOG

WORD wDisProb = 0;		// �Ͽ��ĸ���
WORD wRecProb = 10000;	// �����ĸ���
bool m_theadrun = true;
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
	
	m_pClient->SendPacket((FS_PACKET*)binStream.GetBufferPtr());
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
	if(!m_pClient || !m_pClient->Connect(m_szIP, m_nPort) )
	{
		//printf("���ӷ�����ʧ��!\n");
		return FALSE;
	}

	return TRUE;
}


DWORD CClient::HandleNetPack()
{
	if (NULL == m_pClient)
	{
		return 0;
	}
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

BOOL LogTestResult(const char* strMsg)
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	char szFilename[MAX_PATH] = "";
	sprintf(szFilename, ".\\%04d-%02d-%02d_%02d-%02d-%02d.txt",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	FILE* pFile = fopen(szFilename, "w+t");
	if (pFile == NULL)
	{
		return FALSE;
	}

	fwrite(strMsg, strlen(strMsg), 1, pFile);
	fclose(pFile);

	return TRUE;
}

UINT WINAPI ThreadFuncForSend(void* p)
{
	SConfigData* pCfgData = (SConfigData*)p;
	CClient* pClient = pCfgData->pClient;
	DWORD dwSendTick = pCfgData->dwSendTick;
	DWORD dwPackNum = pCfgData->dwPackNum;
	DWORD dwPackSize = pCfgData->dwPackSize;

	for(UINT i=0; m_theadrun && i<dwPackNum; i++)
	{
		Sleep(dwSendTick);

		pClient->SendTestPack(dwPackSize);
	}

	//	pCfgData->bRunning = FALSE;

	return 0;
}

UINT WINAPI ThreadFuncForReceive(void* p)
{
	SConfigData* pCfgData = (SConfigData*)p;
	CClient* pClient = pCfgData->pClient;
	DWORD dwSendTick = pCfgData->dwSendTick;
	DWORD dwPackNum = pCfgData->dwPackNum;

	while (m_theadrun && pCfgData->bRunning)
	{
		if(pClient)
			pClient->HandleNetPack();
		Sleep(1);
	}

	//delete pCfgData;

	return 0;
}

UINT stressTestMain(CPaintManagerUI& m_pm,const char* pServerIP, int nPort, int nThreadCount, int nSendCount)
{
	srand(GetTickCount());
	DWORD dwThreadNum = 0;
	DWORD dwSendTick = 1000;
	DWORD dwPackNum = 1000;
	DWORD dwPackSize = 100;
	//����������,TCP����
	WORD wDisProb = 0;		// �Ͽ��ĸ���
	WORD wRecProb = 10000;	// �����ĸ���
	// ��ʼ������
	char chIP[250];
	GetPrivateProfileString("config", "ip", "127.0.0.1", chIP, 250, CONFIG_FILE); // ������IP
	nPort = GetPrivateProfileInt("config", "Port", 0, CONFIG_FILE);               // �˿�
	dwThreadNum = GetPrivateProfileInt("config", "ThreadNum", 0, CONFIG_FILE);    // �߳���
	dwSendTick = GetPrivateProfileInt("config", "SendTick", 0, CONFIG_FILE);      // ���ͼ��ʱ��(����)
	dwPackNum = GetPrivateProfileInt("config", "PackNum", 0, CONFIG_FILE);        // ���԰���
	dwPackSize = GetPrivateProfileInt("config", "PackSize", 0, CONFIG_FILE);      // ���԰���С
	wDisProb = GetPrivateProfileInt("config", "DisProb", 0, CONFIG_FILE);         // �Ͽ�����
	wRecProb = GetPrivateProfileInt("config", "RecProb", 0, CONFIG_FILE);         // ��������

	vector<SConfigData*> pCfgDataList;	// �ͻ��˶���
	UINT *dwThreadIds = new UINT[dwThreadNum*2];
	CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTClientReturn")));
	CStdString re_text = "���ӷ�����...\n";
	pControl->SetText(re_text);

	for (UINT i=0; i<dwThreadNum; i++)
	{
		SConfigData* pCfgData = new SConfigData;
		pCfgData->pClient = new CClient;
		if ( !pCfgData->pClient->Init(chIP, nPort, &pCfgData->sTotalInfo) )
		{
			char temp[128];
			sprintf(temp,"���ӷ�����ʧ��! ID=%d, IP=%s, Port=%d\n", i, chIP, nPort);
			re_text += temp;
			pControl->SetText(re_text);
			delete pCfgData;
			continue;
		}
		pCfgData->dwSendTick = dwSendTick;
		pCfgData->dwPackNum = dwPackNum;
		pCfgData->dwPackSize = dwPackSize;
		pCfgData->bRunning = TRUE;
		pCfgDataList.push_back(pCfgData);
	}
	re_text += "���������߳�......\n";
	pControl->SetText(re_text);
	for (UINT i=0; i<pCfgDataList.size(); i++)
	{
		// �����̴߳��������
		_beginthreadex(NULL, 0, ThreadFuncForReceive, pCfgDataList[i], 0, &dwThreadIds[i*2+1]);
	}
	for (UINT i=0; i<pCfgDataList.size(); i++)
	{
		// �����̴߳��������
		_beginthreadex(NULL, 0, ThreadFuncForSend, pCfgDataList[i], 0, &dwThreadIds[i*2]);
	}
	re_text += "��ʼ����......\n";
	pControl->SetText(re_text);

	char szBuffer[1024];
	DWORD dwLastPrintTime = 0;
	DWORD dwLastTime = GetTickCount();
	double dwTotalTime = 0;
	for (;;)
	{
		Sleep(100);

		DWORD dwCurTime = GetTickCount();

		double dwSendCount = 0;
		double dwReceiveCnt = 0;
		double dwMinTick = 0xFFFFFFFF;
		double dwMaxTick = 0;
		double dwTotalTick = 0;
		double dwLastTotalTick = 0;
		double dwLastMinTick = 0xFFFFFFFF;
		double dwLastMaxTick = 0;
		double dwRecFailTotal = 0;
		double dwInvalidPackCnt = 0;
		for (UINT k=0; k<pCfgDataList.size(); k++)
		{
			dwSendCount			+= pCfgDataList[k]->sTotalInfo.dwSendCount;
			dwReceiveCnt		+= pCfgDataList[k]->sTotalInfo.dwReceiveCnt;
			dwTotalTick			+= pCfgDataList[k]->sTotalInfo.dwTotalTick;
			dwInvalidPackCnt	+= pCfgDataList[k]->sTotalInfo.dwInvalidPackCnt;
			dwLastTotalTick		+= pCfgDataList[k]->sTotalInfo.dwCurElapse;

			if ( pCfgDataList[k]->sTotalInfo.dwMinTick < dwMinTick )
				dwMinTick = pCfgDataList[k]->sTotalInfo.dwMinTick;
			if ( pCfgDataList[k]->sTotalInfo.dwMaxTick > dwMaxTick )
				dwMaxTick = pCfgDataList[k]->sTotalInfo.dwMaxTick;

			if ( pCfgDataList[k]->sTotalInfo.dwCurElapse < dwLastMinTick )
				dwLastMinTick = pCfgDataList[k]->sTotalInfo.dwCurElapse;
			if ( pCfgDataList[k]->sTotalInfo.dwCurElapse > dwLastMaxTick )
				dwLastMaxTick = pCfgDataList[k]->sTotalInfo.dwCurElapse;

			dwRecFailTotal += pCfgDataList[k]->sTotalInfo.dwRecFailNum;
		}

		if (dwTotalTime == 0 && dwReceiveCnt >= dwThreadNum*dwPackNum)
		{
			// ��¼�������ʱ��
			dwTotalTime = dwCurTime - dwLastTime;
		}

		double dwAvrTick = (dwReceiveCnt == 0 ? 0: dwTotalTick/dwReceiveCnt);
		double dwLastAvrTick = (pCfgDataList.size() == 0 ? 0: dwLastTotalTick/pCfgDataList.size());

		if (dwCurTime - dwLastPrintTime > 500)
		{
			dwLastPrintTime = dwCurTime;

			//system("cls");
			sprintf(szBuffer, 
				"����ʱ��ͳ��(����): %.0f\n"
				"          ���Ͱ���: %.0f\n"
				"          ���ܰ���: %.0f\n"
				"        ��Ч���ݰ�: %.0f\n"
				"      ����ʧ��ͳ��: %.0f\n"
				"    ���ʱ��(����): %.0f\n"
				"    �ʱ��(����): %.0f\n"
				"    ƽ��ʱ��(����): %.0f\n"
				"�������ʱ��(����): %.0f\n"
				"�����ʱ��(����): %.0f\n"
				"����ƽ��ʱ��(����): %.0f\n",
				dwTotalTime,
				dwSendCount,
				dwReceiveCnt,
				dwInvalidPackCnt,
				dwRecFailTotal,
				dwMinTick,
				dwMaxTick,
				dwAvrTick,
				dwLastMinTick,
				dwLastMaxTick,
				dwLastAvrTick);
			

			if (dwTotalTime > 0)
			{
				LogTestResult(szBuffer);
				break;
			}
			pControl->SetText(szBuffer);
		}
	}
	m_theadrun = false;
	while(!pCfgDataList.empty())
	{
		delete pCfgDataList.back();
		pCfgDataList.pop_back();
	}
	return 0;
}
