#pragma once
#include <Windows.h>
#include "FS_Engine.h"
#include "BinaryStream.h"

#include "UIlib.h"
using namespace DuiLib;
extern WORD wDisProb;	// 断开的概率
extern WORD wRecProb;	// 重练的概率

struct STotalInfo 
{
	double dwSendCount;
	double dwMinTick;
	double dwMaxTick;
	double dwTotalTick;
	double dwReceiveCnt;
	double dwInvalidPackCnt;
	double dwCurElapse;
	double dwRecFailNum;

	STotalInfo()
	{
		dwSendCount = 0;
		dwMinTick = 0xFFFFFFFF;
		dwMaxTick = 0;
		dwTotalTick = 0;
		dwReceiveCnt = 0;
		dwCurElapse = 0;
		dwRecFailNum = 0;
		dwInvalidPackCnt = 0;
	}
};


#include <list>
using namespace std;

class CClient
{
public:
	CClient(void);
	virtual ~CClient(void);

	// 游戏初始化，返回0退出游戏
	BOOL Init(const char* strSrvIP, UINT nPort, STotalInfo* pTotalInfo);

	// 重连
	BOOL Reconnect();
	
	// 处理网络数据包
	DWORD HandleNetPack();

	// 发送测试包
	void SendTestPack(int iSize);

	FSClient* GetClientNet() {return m_pClient;}
	


private:
	FSClient *m_pClient;	//网络

	char m_szIP[20];
	UINT m_nPort;
	STotalInfo* m_pTotalInfo;
	CBinaryStream binStream;
};


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

BOOL LogTestResult(const char* strMsg);
UINT WINAPI ThreadFuncForSend(void* p);
UINT WINAPI ThreadFuncForReceive(void* p);
UINT stressTestMain(CPaintManagerUI& m_pm,const char* pServerIP, int nPort, int nThreadCount, int nSendCount);
