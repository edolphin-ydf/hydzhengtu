#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "client.h"
using namespace std;

//全局变量声明


UINT WINAPI ThreadFuncForSend(void* p);
UINT WINAPI ThreadFuncForReceive(void* p);

#define CONFIG_FILE	".\\config.ini"



struct SConfigData 
{
	BOOL bRunning;
	CClient* pClient;
	DWORD dwSendTick;
	DWORD dwPackNum;
	DWORD dwPackSize;
	STotalInfo sTotalInfo;

	SConfigData()
	{
		bRunning = FALSE;
		pClient = NULL;
		dwSendTick = 0;
		dwPackNum = 0;
	}

	~SConfigData()
	{
		delete pClient;
		pClient = NULL;
		bRunning = FALSE;
	}
};


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

	// 初始化参数
	char chIP[250];
	UINT nPort = 0;
	GetPrivateProfileString("config", "ip", "127.0.0.1", chIP, 250, CONFIG_FILE); // 服务器IP
	nPort = GetPrivateProfileInt("config", "Port", 0, CONFIG_FILE);               // 端口
	dwThreadNum = GetPrivateProfileInt("config", "ThreadNum", 0, CONFIG_FILE);    // 线程数
	dwSendTick = GetPrivateProfileInt("config", "SendTick", 0, CONFIG_FILE);      // 发送间隔时间(毫秒)
	dwPackNum = GetPrivateProfileInt("config", "PackNum", 0, CONFIG_FILE);        // 测试包数
	dwPackSize = GetPrivateProfileInt("config", "PackSize", 0, CONFIG_FILE);      // 测试包大小
	wDisProb = GetPrivateProfileInt("config", "DisProb", 0, CONFIG_FILE);         // 断开概率
	wRecProb = GetPrivateProfileInt("config", "RecProb", 0, CONFIG_FILE);         // 重连概率
	
	dwThreadNum = 1;
	nPort = 3724;

	vector<SConfigData*> pCfgDataList;	// 客户端对象
	UINT *dwThreadIds = new UINT[dwThreadNum*2];

	printf("连接服务器......\n");
	for (UINT i=0; i<dwThreadNum; i++)
	{
		SConfigData* pCfgData = new SConfigData;
		pCfgData->pClient = new CClient;
		if ( !pCfgData->pClient->Init(chIP, nPort, &pCfgData->sTotalInfo) )
		{
			printf("连接服务器失败! ID=%d, IP=%s, Port=%d\n", i, chIP, nPort);
			delete pCfgData;
			continue;
		}
		pCfgData->dwSendTick = dwSendTick;
		pCfgData->dwPackNum = dwPackNum;
		pCfgData->dwPackSize = dwPackSize;
		pCfgData->bRunning = TRUE;
		pCfgDataList.push_back(pCfgData);
	}


	printf("创建测试线程......\n");
	for (UINT i=0; i<pCfgDataList.size(); i++)
	{
		// 创建线程处理网络包
		_beginthreadex(NULL, 0, ThreadFuncForReceive, pCfgDataList[i], 0, &dwThreadIds[i*2+1]);
	}
	for (UINT i=0; i<pCfgDataList.size(); i++)
	{
		// 创建线程处理网络包
		_beginthreadex(NULL, 0, ThreadFuncForSend, pCfgDataList[i], 0, &dwThreadIds[i*2]);
	}

	printf("开始测试...\n");
	
// 	// 处理控制台输入
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
			// 记录测试完成时间
			dwTotalTime = dwCurTime - dwLastTime;
		}

		double dwAvrTick = (dwReceiveCnt == 0 ? 0: dwTotalTick/dwReceiveCnt);
		double dwLastAvrTick = (pCfgDataList.size() == 0 ? 0: dwLastTotalTick/pCfgDataList.size());

		if (dwCurTime - dwLastPrintTime > 500)
		{
			dwLastPrintTime = dwCurTime;

				system("cls");
			sprintf(szBuffer, 
				"测试时间统计(毫秒): %.0f\n"
				"          发送包数: %.0f\n"
				"          接受包数: %.0f\n"
				"        无效数据包: %.0f\n"
				"      重连失败统计: %.0f\n"
				"    最短时间(毫秒): %.0f\n"
				"    最长时间(毫秒): %.0f\n"
				"    平均时间(毫秒): %.0f\n"
				"最新最短时间(毫秒): %.0f\n"
				"最新最长时间(毫秒): %.0f\n"
				"最新平均时间(毫秒): %.0f\n",
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
	SConfigData* pCfgData = (SConfigData*)p;
	CClient* pClient = pCfgData->pClient;
	DWORD dwSendTick = pCfgData->dwSendTick;
	DWORD dwPackNum = pCfgData->dwPackNum;
	DWORD dwPackSize = pCfgData->dwPackSize;

	for(UINT i=0; i<dwPackNum; i++)
	{
		Sleep(dwSendTick);

		//pClient->SendTestPack(dwPackSize);
		pClient->SendTestPack1();
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

	while (pCfgData->bRunning)
	{
		pClient->HandleNetPack();
		Sleep(1);
	}

	//delete pCfgData;

	return 0;
}

