#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "client.h"
using namespace std;

//ȫ�ֱ�������


UINT WINAPI ThreadFuncForSend(void* p);
UINT WINAPI ThreadFuncForReceive(void* p);

#define CONFIG_FILE	".\\config.ini"

int m_runing = 0;
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


void main()
{
	srand(GetTickCount());

	DWORD dwThreadNum = 0;
	DWORD dwSendTick = 1000;
	DWORD dwPackNum = 1000;
	DWORD dwPackSize = 100;

	// ��ʼ������
	char chIP[250];
	UINT nPort = 0;
	GetPrivateProfileString("config", "ip", "127.0.0.1", chIP, 250, CONFIG_FILE); // ������IP
	nPort = GetPrivateProfileInt("config", "Port", 0, CONFIG_FILE);               // �˿�
	dwThreadNum = GetPrivateProfileInt("config", "ThreadNum", 0, CONFIG_FILE);    // �߳���
	dwSendTick = GetPrivateProfileInt("config", "SendTick", 0, CONFIG_FILE);      // ���ͼ��ʱ��(����)
	dwPackNum = GetPrivateProfileInt("config", "PackNum", 0, CONFIG_FILE);        // ���԰���
	dwPackSize = GetPrivateProfileInt("config", "PackSize", 0, CONFIG_FILE);      // ���԰���С
	wDisProb = GetPrivateProfileInt("config", "DisProb", 0, CONFIG_FILE);         // �Ͽ�����
	wRecProb = GetPrivateProfileInt("config", "RecProb", 0, CONFIG_FILE);         // ��������
	
	//dwThreadNum = 1;
	//dwPackNum = 1;
	nPort = 3724;

	vector<SConfigData*> pCfgDataList;	// �ͻ��˶���
	UINT *dwThreadIds = new UINT[dwThreadNum*2];

	printf("���ӷ�����......\n");
	for (UINT i=0; i<dwThreadNum; i++)
	{
		SConfigData* pCfgData = new SConfigData;
		pCfgData->pClient = new CClient;
		if ( !pCfgData->pClient->Init(chIP, nPort, pCfgData) )
		{
			printf("���ӷ�����ʧ��! ID=%d, IP=%s, Port=%d\n", i, chIP, nPort);
			delete pCfgData;
			continue;
		}
		pCfgData->dwSendTick = dwSendTick;
		pCfgData->dwPackNum = dwPackNum;
		pCfgData->dwPackSize = dwPackSize;
		pCfgData->bRunning = TRUE;
		pCfgDataList.push_back(pCfgData);
	}


	printf("���������߳�......\n");
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

	printf("��ʼ����...\n");
	
// 	// �������̨����
// 	printf("input \"exit\" to exit program.\n");
// 	for (;;)
// 	{
// 		char command[255];
// 		cin >> command;
// 		if ( strcmp(command, "exit") == 0 )
// 		{
// 			break;
// 		}
// 	}

	char szBuffer[1024];
	DWORD dwLastPrintTime = 0;
	DWORD dwLastTime = GetTickCount();
	double dwTotalTime = 0;
	for (;;)
	{
		Sleep(100);

		DWORD dwCurTime = GetTickCount();

		double dwSendCount = 0;              //���Ͱ���
		double dwReceiveCnt = 0;             //���ܰ���
		double dwMinTick = 0xFFFFFFFF;       //���ʱ��(����)
		double dwMaxTick = 0;                //�ʱ��(����)
		double dwTotalTick = 0;              //ʱ��ͳ��(����)
		double dwLastTotalTick = 0;          //
		double dwLastMinTick = 0xFFFFFFFF;   //�������ʱ��(����)
		double dwLastMaxTick = 0;            //�����ʱ��(����)
		double dwRecFailTotal = 0;           //����ʧ��ͳ��
		double dwInvalidPackCnt = 0;         //��Ч���ݰ�
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

				system("cls");
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
			printf(szBuffer);

			if (dwTotalTime > 0)
			{
				LogTestResult(szBuffer);
				break;
			}
		}
	}
	for (UINT i=0; i<pCfgDataList.size(); i++)
	{
		// �����̴߳��������
		pCfgDataList[i]->bRunning=false;
	}
	while(m_runing>0)
	{
		Sleep(10);
	}

	while(!pCfgDataList.empty())
	{
		delete pCfgDataList.back();
		pCfgDataList.pop_back();
	}

	printf("press enter key to exit");
	getchar();
}




UINT WINAPI ThreadFuncForSend(void* p)
{
	m_runing++;
	SConfigData* pCfgData = (SConfigData*)p;
	CClient* pClient = pCfgData->pClient;
	DWORD dwSendTick = pCfgData->dwSendTick;
	DWORD dwPackNum = pCfgData->dwPackNum;
	DWORD dwPackSize = pCfgData->dwPackSize;

	for(UINT i=0; i<dwPackNum; i++)
	{
		Sleep(dwSendTick);

		pClient->SendTestPack(0);
		//pClient->SendTestPack1(1);
	}

//	pCfgData->bRunning = FALSE;
	m_runing--;
	return 0;
}

UINT WINAPI ThreadFuncForReceive(void* p)
{
	m_runing++;
	SConfigData* pCfgData = (SConfigData*)p;
	DWORD dwSendTick = pCfgData->dwSendTick;
	DWORD dwPackNum = pCfgData->dwPackNum;

	while (pCfgData->bRunning)
	{
		CClient* pClient = pCfgData->pClient;
		pClient->HandleNetPack();
		Sleep(1);
	}

	//delete pCfgData;
	m_runing--;
	return 0;
}

