#pragma once

#include "define.h"
#include <ShellAPI.h>
#include "ControlEx.h"
#include "resource.h"
#include <string>

#include "Client.h"

#define SKINFILE_NAME "StressClient.xml"

class CFrameWindowWnd : public CWindowWnd, public INotifyUI, public IListCallbackUI
{
public:
	CFrameWindowWnd() { };
	LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_FRAME | CS_DBLCLKS; };
	void OnFinalMessage(HWND /*hWnd*/) { delete this; };

	void Init() {
		m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
		m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("maxbtn")));
		m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));
	}

	void OnPrepare() {
	}

	void Notify(TNotifyUI& msg)
	{
		if( msg.sType == _T("windowinit") ) OnPrepare();
		else if( msg.sType == _T("click") ) {
			if( msg.pSender == m_pCloseBtn ) {
				PostQuitMessage(0);
				return; 
			}
			else if( msg.pSender == m_pMinBtn ) { 
				SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); return; }
			else if( msg.pSender == m_pMaxBtn ) { 
				SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; }
			else if( msg.pSender == m_pRestoreBtn ) { 
				SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; }

			CStdString name = msg.pSender->GetName();
			//如果点击"连接"
			if(name == _T("btnConnect"))
			{
				CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtServerIP")));
				CStdString strIP = pControl->GetText();
				pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtServerPort")));
				CStdString strPort = pControl->GetText();

				if(true)//== m_ManagerConnect.Connect(strIP.GetData(), atoi(strPort.GetData())
				{

					CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnDisConnect")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnShowModule")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnUnLoadModule")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnLoadModule")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnConnectCount")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnCommand")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnThreadState")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnClientInfo")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnUDPClientInfo")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnShowForbidden")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnAddForbidden")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnDelForbidden")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnCloseClient")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnCloseClient")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnServerConnectInfoTCP")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnServerConnectInfoUDP")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnConnecthistory")));
					pBtnControl->SetEnabled(true);
					pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnCommandAll")));
					pBtnControl->SetEnabled(true);
				}
				return;
			}

			if(name == _T("btnCommandAll"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnCommandAll")));
				pBtnControl->SetEnabled(false);

				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "ShowAllCommand -a");
				OnMessage_ShowAllCommand(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnConnecthistory"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnConnecthistory")));
				pBtnControl->SetEnabled(false);

				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "ShowConnectHistory -a");
				OnMessage_ShowConnectHistory(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnServerConnectInfoTCP"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnServerConnectInfoTCP")));
				pBtnControl->SetEnabled(false);

				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "ServerConnectTCP -a");
				OnMessage_ShowServerConnectInfoTCP(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnServerConnectInfoUDP"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnServerConnectInfoUDP")));
				pBtnControl->SetEnabled(false);

				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "ServerConnectUDP -a");
				OnMessage_ShowServerConnectInfoUDP(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnCloseClient"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnAddForbidden")));
				pBtnControl->SetEnabled(false);
				CEditUI* pTxtControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtCloseClientID")));
				CStdString strCloseClient = pTxtControl->GetText();

				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "CloseClient %d", atoi(strCloseClient.GetData()));
				OnMessage_CloseClient(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnAddForbidden"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnAddForbidden")));
				pBtnControl->SetEnabled(false);

				CEditUI* pTxtControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtForbidenIP")));
				CStdString strForbidenIP = pTxtControl->GetText();

				int nType = 0;
				COptionUI* popControl = static_cast<COptionUI*>(m_pm.FindControl(_T("opForever")));
				bool blState = popControl->IsSelected();
				if(blState == true)
				{
					nType = 0;
				}
				else
				{
					nType = 1;
				}

				pTxtControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtSeconds")));
				CStdString strSeconds = pTxtControl->GetText();

				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "ForbiddenIP -c %s -t %d -s %s ", strForbidenIP.GetData(), nType, strSeconds.GetData());
				OnMessage_AddForbiddenIP(szCommand);
				sprintf_s(szCommand, MAX_BUFF_200, "ShowForbiddenIP -a");
				OnMessage_ShowForbiddenIP(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnDelForbidden"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnShowForbidden")));
				pBtnControl->SetEnabled(false);

				CEditUI* pTxtControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtForbidenIP")));
				CStdString strForbidenIP = pTxtControl->GetText();

				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "LiftedIP %s", strForbidenIP.GetData());
				OnMessage_DelForbiddenIP(szCommand);
				sprintf_s(szCommand, MAX_BUFF_200, "ShowForbiddenIP -a");
				OnMessage_ShowForbiddenIP(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnShowForbidden"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnShowForbidden")));
				pBtnControl->SetEnabled(false);
				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "ShowForbiddenIP -a");
				OnMessage_ShowForbiddenIP(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnClientInfo"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnClientInfo")));
				pBtnControl->SetEnabled(false);
				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "ConnectClient -a");
				OnMessage_ShowWorkClientInfo(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnUDPClientInfo"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnUDPClientInfo")));
				pBtnControl->SetEnabled(false);
				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "UDPConnectClient -a");
				OnMessage_ShowWorkUDPClientInfo(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnThreadState"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnThreadState")));
				pBtnControl->SetEnabled(false);
				char szCommand[MAX_BUFF_200] = {'\0'};
				sprintf_s(szCommand, MAX_BUFF_200, "WorkThreadState -s");
				OnMessage_ShowWorkThreadInfo(szCommand);
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnCommand"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnCommand")));
				pBtnControl->SetEnabled(false);

				//显示指定信令信息
				char szCommand[MAX_BUFF_200] = {'\0'};
				CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtCommandID")));
				CStdString strCommand = pControl->GetText();
				sprintf_s(szCommand, MAX_BUFF_200, "CommandInfo %s", strCommand.GetData());
				OnMessage_ShowCommandInfo(szCommand);
				pBtnControl->SetEnabled(true);
			}

			if(name == _T("btnShowModule"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnShowModule")));
				pBtnControl->SetEnabled(false);
				//显示服务器所有模块
				OnMessage_ShowModule();
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnUnLoadModule"))
			{
				//卸载服务器的指定模块
				CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtModuleName")));
				CStdString strModuleName = pControl->GetText();
				OnMessage_UnloadModule(strModuleName.GetData());
				OnMessage_ShowModule();
				return;
			}

			if(name == _T("btnLoadModule"))
			{
				//重载服务器的指定模块
				CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtModuleName")));
				CStdString strModuleName = pControl->GetText();
				OnMessage_ReLoadModule(strModuleName.GetData());
				OnMessage_ShowModule();
				return;
			}

			if(name == _T("btnConnectCount"))
			{
				OnMessage_ShowClientCount();
				return;
			}

			if(name == _T("btnTest"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnDisConnect")));
				pBtnControl->SetEnabled(false);

				CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTServerIP")));
				CStdString strTServerIP = pControl->GetText();
				pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTServerPort")));
				CStdString strTServerPort = pControl->GetText();
				pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTClientCount")));
				CStdString strTClientCount = pControl->GetText();
				pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTClientPacketCount")));
				CStdString strTClientPacketCount = pControl->GetText();

				OnMessage_TestServer(strTServerIP.GetData(), atoi(strTServerPort.GetData()), atoi(strTClientCount.GetData()), atoi(strTClientPacketCount.GetData()));
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name == _T("btnTestUDP"))
			{
				CButtonUI* pBtnControl = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnDisConnect")));
				pBtnControl->SetEnabled(false);

				CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTServerIP")));
				CStdString strTServerIP = pControl->GetText();
				pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTServerPort")));
				CStdString strTServerPort = pControl->GetText();
				pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTClientCount")));
				CStdString strTClientCount = pControl->GetText();
				pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTClientPacketCount")));
				CStdString strTClientPacketCount = pControl->GetText();

				OnMessage_TestServerUDP(strTServerIP.GetData(), atoi(strTServerPort.GetData()), atoi(strTClientCount.GetData()), atoi(strTClientPacketCount.GetData()));
				pBtnControl->SetEnabled(true);
				return;
			}

			if(name==_T("ModuleManager"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(0);
				return;
			}
			else if(name==_T("ConnectManager"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(1);
				return;
			}
			else if(name==_T("LogManager"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(2);
				return;
			}
			else if(name==_T("WorkThreadManager"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(3);
				return;
			}
			else if(name==_T("ClientConnectManager"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(4);
				return;
			}
			else if(name==_T("ServerConnectManager"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(5);
				return;
			}
			else if(name==_T("ForbiddenIP"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(6);
				return;
			}
			else if(name==_T("TestManager"))
			{
				CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
				pControl->SelectItem(7);
				return;
			}

			if(name== _T("btnForum"))
			{
				OnMessage_ShowWeb();
			}
		}
		else if(msg.sType==_T("selectchanged"))
		{
		}
		else if(msg.sType==_T("setfocus"))
		{
		}
	}

	void OnMessage_ShowWeb()
	{
		ShellExecute(0, NULL, _T( "http://www.mcode.org"), NULL, NULL, SW_NORMAL);
	}

	void OnMessage_TestServer(const char* pServerIP, int nPort, int nThreadCount, int nSendCount)
	{
		stressTestMain(m_pm,pServerIP, nPort, nThreadCount, nSendCount);
	}

	void OnMessage_TestServerUDP(const char* pServerIP, int nPort, int nThreadCount, int nSendCount)
	{
		MessageBox(NULL, (LPCTSTR)"test2", (LPCTSTR)"提示信息", MB_OK);
		int nTimeCost        = 0;
		char szTimeCost[200] = {'\0'};

		if(true)// == m_ClientConnectUDP.Init(pServerIP, nPort, nThreadCount, nSendCount, nTimeCost)
		{
			CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTClientReturn")));
			sprintf_s(szTimeCost, 200, _T("一共花费时间(%d)毫秒。"), nTimeCost);

			pControl->SetText((LPCTSTR)szTimeCost);
		}
		else
		{
			CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtTClientReturn")));
			sprintf_s(szTimeCost, 200, _T("链接服务器出错。"));

			pControl->SetText((LPCTSTR)szTimeCost);
		}
	}

	void OnMessage_ShowClientCount()
	{
		MessageBox(NULL, (LPCTSTR)"test3", (LPCTSTR)"提示信息", MB_OK);
		char szData[100] = {'\0'};
		char szText[300] = {'\0'};
		int nCount     = 0;
		int nPoolCount = 0;
		sprintf_s(szData, 100, "ClientCount -cp");
		//m_ManagerConnect.SendClientCount(szData, nCount, nPoolCount);
		sprintf_s(szText, 300, _T("当前客户端链接数为(%d)，服务器连接池还剩余(%d)。"), nCount, nPoolCount);
		CEditUI* pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtClientInfo")));
		pControl->SetText((LPCTSTR)szText);
		//_ServerRunInfo ServerRunInfo = m_ManagerConnect.SendShowCurrProcessInfo("ShowCurrProcessInfo -a");
		sprintf_s(szText, 300, _T("当前CPU占用率(%f%%), 当前内存占用为%fMb。"), (float)0, (float)0);
		pControl = static_cast<CEditUI*>(m_pm.FindControl(_T("txtServerCPU")));
		pControl->SetText((LPCTSTR)szText);
	}

	void OnMessage_CloseClient(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test4", (LPCTSTR)"提示信息", MB_OK);
		//m_ManagerConnect.SendCloseClient(pCommand);
	}

	void OnMessage_AddForbiddenIP(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test5", (LPCTSTR)"提示信息", MB_OK);
		//m_ManagerConnect.SendAddForbiddenIP(pCommand);
	}

	void OnMessage_DelForbiddenIP(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test6", (LPCTSTR)"提示信息", MB_OK);
		//m_ManagerConnect.SendDelForbiddenIP(pCommand);
	}

	void OnMessage_ShowForbiddenIP(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test7", (LPCTSTR)"提示信息", MB_OK);
		//m_VecForbiddenIP.clear();
		//m_ManagerConnect.SendShowForbiddenIP(pCommand, m_VecForbiddenIP);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("forbiddenlist")));
		//CStdString strName = pList->GetName();
		pList->RemoveAll();
		//pList->SetTextCallback(this);
		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				pListElement->SetOwner(pList);
				char szBuf[100] = {'\0'};
				//pListElement->SetText(0, m_VecForbiddenIP[i].m_szIP);
				
				if(0 == 0)
				{
					sprintf_s(szBuf, 100, "永久封禁");
				}
				else
				{
					sprintf_s(szBuf, 100, "时段封禁");
				}
				pListElement->SetText(1, szBuf);

				if(0 == 0)
				{
					sprintf_s(szBuf, 100, "---");
					pListElement->SetText(2, szBuf);
				}
				else
				{
					//ACE_Time_Value tvBegin(m_VecForbiddenIP[i].m_nBeginTime);
					//ACE_Date_Time dt(tvBegin);
					//sprintf_s(szBuf, 100, "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
					sprintf_s(szBuf,100,"时间日期");
					pListElement->SetText(2, szBuf);
				}

				if(0 == 0)
				{
					sprintf_s(szBuf, 100, "---");
					pListElement->SetText(3, szBuf);
				}
				else
				{
					//sprintf_s(szBuf, 100, "%d", m_VecForbiddenIP[i].m_nSecond);
					pListElement->SetText(3, szBuf);
				}

				pList->Add(pListElement);
			}
		}
	}

	void OnMessage_ShowServerConnectInfoTCP(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test8", (LPCTSTR)"提示信息", MB_OK);
		//m_VecServerTCPConnectInfo.clear();
		//m_ManagerConnect.SendServerConnectTCP(pCommand, m_VecServerTCPConnectInfo);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("serverconnectlist")));
		CStdString strName = pList->GetName();
		pList->RemoveAll();

		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				//pListElement->SetOwner(pList);
				//char szBuf[100] = {'\0'};
				//sprintf_s(szBuf, 100, "%d", m_VecServerTCPConnectInfo[i].m_nConnectID);
				//pListElement->SetText(0, szBuf);
				//pListElement->SetText(1, m_VecServerTCPConnectInfo[i].m_szIP);
				//sprintf_s(szBuf, 100, "%d", m_VecServerTCPConnectInfo[i].m_nSendCount);
				//pListElement->SetText(2, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerTCPConnectInfo[i].m_nRecvCount);
				//pListElement->SetText(3, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerTCPConnectInfo[i].m_nAllSendSize);
				//pListElement->SetText(4, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerTCPConnectInfo[i].m_nAllRecvSize);
				//pListElement->SetText(5, szBuf);
				////ACE_Time_Value tvBegin(m_VecServerTCPConnectInfo[i].m_nBeginTime);
				////ACE_Date_Time dt(tvBegin);
				////sprintf_s(szBuf, 100, "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
				//sprintf_s(szBuf, 100, "时间日期");
				//pListElement->SetText(6, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerTCPConnectInfo[i].m_nAliveTime);
				//pListElement->SetText(7, szBuf);
				//pList->Add(pListElement);
			}
		}
	}

	void OnMessage_ShowServerConnectInfoUDP(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test9", (LPCTSTR)"提示信息", MB_OK);
		//m_VecServerUDPConnectInfo.clear();
		//m_ManagerConnect.SendServerConnectUDP(pCommand, m_VecServerUDPConnectInfo);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("serverconnectlist")));
		CStdString strName = pList->GetName();
		pList->RemoveAll();

		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				//pListElement->SetOwner(pList);
				//char szBuf[100] = {'\0'};
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nConnectID);
				//pListElement->SetText(0, szBuf);
				//pListElement->SetText(1, m_VecServerUDPConnectInfo[i].m_szIP);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nSendCount);
				//pListElement->SetText(2, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nRecvCount);
				//pListElement->SetText(3, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nAllSendSize);
				//pListElement->SetText(4, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nAllRecvSize);
				//pListElement->SetText(5, szBuf);
				////ACE_Time_Value tvBegin(m_VecServerUDPConnectInfo[i].m_nBeginTime);
				////ACE_Date_Time dt(tvBegin);
				////sprintf_s(szBuf, 100, "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
				//sprintf_s(szBuf, 100, "时间日期");
				//pListElement->SetText(6, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nAliveTime);
				//pListElement->SetText(7, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nRecvQueueCount);
				//pListElement->SetText(8, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nRecvQueueTimeCost);
				//pListElement->SetText(9, szBuf);
				//sprintf_s(szBuf, 100, "%d", m_VecServerUDPConnectInfo[i].m_nSendQueueTimeCost);
				//pListElement->SetText(10, szBuf);
				//pList->Add(pListElement);
			}
		}
	}

	void OnMessage_ShowWorkUDPClientInfo(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test10", (LPCTSTR)"提示信息", MB_OK);
		//m_VecClientConnectInfo.clear();
		//m_ManagerConnect.SendUDPClientInfo(pCommand, m_VecClientConnectInfo);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("clientconnectlist")));
		CStdString strName = pList->GetName();
		pList->RemoveAll();

		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				/*pListElement->SetOwner(pList);
				char szBuf[100] = {'\0'};
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nConnectID);
				pListElement->SetText(0, szBuf);
				pListElement->SetText(1, m_VecClientConnectInfo[i].m_szIP);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nSendCount);
				pListElement->SetText(2, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nRecvCount);
				pListElement->SetText(3, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nAllSendSize);
				pListElement->SetText(4, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nAllRecvSize);
				pListElement->SetText(5, szBuf);
				ACE_Time_Value tvBegin(m_VecClientConnectInfo[i].m_nBeginTime);
				ACE_Date_Time dt(tvBegin);
				sprintf_s(szBuf, 100, "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
				pListElement->SetText(6, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nAliveTime);
				pListElement->SetText(7, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nRecvQueueCount);
				pListElement->SetText(8, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nRecvQueueTimeCost);
				pListElement->SetText(9, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nSendQueueTimeCost);
				pListElement->SetText(10, szBuf);
				pList->Add(pListElement);*/
			}
		}
	}

	void OnMessage_ShowAllCommand(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test11", (LPCTSTR)"提示信息", MB_OK);
		int nCommandCount = 0;
		//m_VecCommandInfo.clear();
		//m_ManagerConnect.SendCommandAllInfo(pCommand, m_VecCommandInfo, nCommandCount);

		char szCommandCount[200] = {'\0'};
		sprintf_s(szCommandCount, 200, "当前服务器的总命令数为:%d", nCommandCount);
		CTextUI* pTxt = static_cast<CTextUI*>(m_pm.FindControl(_T("txtCommandCount")));
		pTxt->SetText(szCommandCount);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("commandlist")));
		CStdString strName = pList->GetName();
		pList->RemoveAll();
		//pList->SetTextCallback(this);
		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				/*pListElement->SetOwner(pList);
				char szBuf[100] = {'\0'};
				pListElement->SetText(0, m_VecCommandInfo[i].szModuleName);
				pListElement->SetText(1, m_VecCommandInfo[i].szCommandID);
				sprintf_s(szBuf, 100, "%d", m_VecCommandInfo[i].m_nCount);
				pListElement->SetText(2, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecCommandInfo[i].m_nTimeCost);
				pListElement->SetText(3, szBuf);
				pList->Add(pListElement);*/
			}
		}
	}

	void OnMessage_ShowConnectHistory(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test12", (LPCTSTR)"提示信息", MB_OK);
		//m_VecClientConnectInfo.clear();
		//m_ManagerConnect.SendServerConnectHistory(pCommand, m_VecIPAccount);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("connecthistory")));
		pList->RemoveAll();

		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				/*pListElement->SetOwner(pList);
				char szBuf[100] = {'\0'};
				sprintf_s(szBuf, 100, "%s", m_VecIPAccount[i].m_szIP);
				pListElement->SetText(0, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecIPAccount[i].m_nCount);
				pListElement->SetText(1, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecIPAccount[i].m_nAllCount);
				pListElement->SetText(2, szBuf);
				sprintf_s(szBuf, 100, "%s", m_VecIPAccount[i].m_szDate);
				pListElement->SetText(3, szBuf);
				pList->Add(pListElement);*/
			}
		}
	}

	void OnMessage_ShowWorkClientInfo(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test13", (LPCTSTR)"提示信息", MB_OK);
		//m_VecClientConnectInfo.clear();
		//m_ManagerConnect.SendClientInfo(pCommand, m_VecClientConnectInfo);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("clientconnectlist")));
		CStdString strName = pList->GetName();
		pList->RemoveAll();

		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				/*pListElement->SetOwner(pList);
				char szBuf[100] = {'\0'};
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nConnectID);
				pListElement->SetText(0, szBuf);
				pListElement->SetText(1, m_VecClientConnectInfo[i].m_szIP);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nSendCount);
				pListElement->SetText(2, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nRecvCount);
				pListElement->SetText(3, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nAllSendSize);
				pListElement->SetText(4, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nAllRecvSize);
				pListElement->SetText(5, szBuf);
				ACE_Time_Value tvBegin(m_VecClientConnectInfo[i].m_nBeginTime);
				ACE_Date_Time dt(tvBegin);
				sprintf_s(szBuf, 100, "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
				pListElement->SetText(6, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nAliveTime);
				pListElement->SetText(7, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nRecvQueueCount);
				pListElement->SetText(8, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nRecvQueueTimeCost);
				pListElement->SetText(9, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecClientConnectInfo[i].m_nSendQueueTimeCost);
				pListElement->SetText(10, szBuf);
				pList->Add(pListElement);*/
			}
		}
	}

	void OnMessage_ShowWorkThreadInfo(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test14", (LPCTSTR)"提示信息", MB_OK);
		//m_VecWorkThreadInfo.clear();
		//m_ManagerConnect.SendWorkThreadState(pCommand, m_VecWorkThreadInfo);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("workthreadlist")));
		CStdString strName = pList->GetName();
		pList->RemoveAll();
		//pList->SetTextCallback(this);
		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				/*pListElement->SetOwner(pList);
				char szBuf[100] = {'\0'};
				pListElement->SetText(0, m_VecWorkThreadInfo[i].m_szThreadName);
				sprintf_s(szBuf, 100, "%d", m_VecWorkThreadInfo[i].m_nThreadID);
				pListElement->SetText(1, szBuf);
				ACE_Date_Time dt( m_VecWorkThreadInfo[i].m_tvUpdateTime);
				sprintf_s(szBuf, 100, "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
				pListElement->SetText(2, szBuf);
				if(m_VecWorkThreadInfo[i].m_nState == 0)
				{
					sprintf_s(szBuf, 100, "线程初始化");
				}
				else if(m_VecWorkThreadInfo[i].m_nState == 1)
				{
					sprintf_s(szBuf, 100, "处理数据中");
				}
				else if(m_VecWorkThreadInfo[i].m_nState == 2)
				{
					sprintf_s(szBuf, 100, "处理数据完成");
				}
				else
				{
					sprintf_s(szBuf, 100, "线程阻塞");
				}
				pListElement->SetText(3, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecWorkThreadInfo[i].m_nRecvPacketCount);
				pListElement->SetText(4, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecWorkThreadInfo[i].m_nSendPacketCount);
				pListElement->SetText(5, szBuf);
				sprintf_s(szBuf, 100, "0x%04x", m_VecWorkThreadInfo[i].m_nCommandID);
				pListElement->SetText(6, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecWorkThreadInfo[i].m_nPacketTime);
				pListElement->SetText(7, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecWorkThreadInfo[i].m_nCurrPacketCount);
				pListElement->SetText(8, szBuf);
				pList->Add(pListElement);*/
			}
		}
	}

	void OnMessage_ShowCommandInfo(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test15", (LPCTSTR)"提示信息", MB_OK);
		int nCommandCount = 0;
		//m_VecCommandInfo.clear();
		//m_ManagerConnect.SendCommandInfo(pCommand, m_VecCommandInfo, nCommandCount);

		char szCommandCount[200] = {'\0'};
		sprintf_s(szCommandCount, 200, "当前服务器的总命令数为:%d", nCommandCount);
		CTextUI* pTxt = static_cast<CTextUI*>(m_pm.FindControl(_T("txtCommandCount")));
		pTxt->SetText(szCommandCount);

		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("commandlist")));
		CStdString strName = pList->GetName();
		pList->RemoveAll();
		//pList->SetTextCallback(this);
		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			/*if(NULL != pListElement)
			{
				pListElement->SetOwner(pList);
				char szBuf[100] = {'\0'};
				pListElement->SetText(0, m_VecCommandInfo[i].szModuleName);
				pListElement->SetText(1, m_VecCommandInfo[i].szCommandID);
				sprintf_s(szBuf, 100, "%d", m_VecCommandInfo[i].m_nCount);
				pListElement->SetText(2, szBuf);
				sprintf_s(szBuf, 100, "%d", m_VecCommandInfo[i].m_nTimeCost);
				pListElement->SetText(3, szBuf);
				pList->Add(pListElement);
			}*/
		}
	}

	void OnMessage_ShowModule()
	{
		MessageBox(NULL, (LPCTSTR)"test16", (LPCTSTR)"提示信息", MB_OK);
		//m_VecModuleInfo.clear();
		//m_ManagerConnect.SendShowModule(m_VecModuleInfo);
		CListUI* pList = static_cast<CListUI*>(m_pm.FindControl(_T("domainlist")));
		pList->RemoveAll();
		//pList->SetTextCallback(this);
		for(int i = 0; i < (int)5; i++)
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			if(NULL != pListElement)
			{
				/*pListElement->SetOwner(pList);
				pListElement->SetText(0, m_VecModuleInfo[i].szModuleFile);
				pListElement->SetText(1, m_VecModuleInfo[i].szModuleName);
				pListElement->SetText(2, m_VecModuleInfo[i].szModuleDesc);
				pListElement->SetText(3, m_VecModuleInfo[i].szModuleCreateDate);
				pList->Add(pListElement);*/
			}
		}
	}

	void OnMessage_UnloadModule(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test17", (LPCTSTR)"提示信息", MB_OK);
		/*bool blState = m_ManagerConnect.SendUnloadModule(pCommand);
		if(true == blState)
		{
		string strModuleName = pCommand;
		string strMessage = (string)"卸载: " + strModuleName + (string)"成功。";
		MessageBox(NULL, (LPCTSTR)strMessage.c_str(), (LPCTSTR)"提示信息", MB_OK);
		}
		else
		{
		string strModuleName = pCommand;
		string strMessage = (string)"卸载: " + strModuleName + (string)"失败。";
		MessageBox(NULL,(LPCTSTR)strMessage.c_str(), (LPCTSTR)"提示信息", MB_OK);
		}*/
		MessageBox(NULL, (LPCTSTR)"", (LPCTSTR)"提示信息", MB_OK);
	}

	void OnMessage_LoadModule(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test18", (LPCTSTR)"提示信息", MB_OK);
		/*string strModuleName = pCommand;
		bool blState = m_ManagerConnect.SendLoadModule(pCommand);
		if(true == blState)
		{
			string strModuleName = pCommand;
			string strMessage = (string)"重载: " + strModuleName + (string)"成功。";
			MessageBox(NULL,(LPCTSTR)strMessage.c_str(), (LPCTSTR)"提示信息", MB_OK);
		}
		else
		{
			string strModuleName = pCommand;
			string strMessage = (string)"重载: " + strModuleName + (string)"失败。";
			MessageBox(NULL,(LPCTSTR)strMessage.c_str(), (LPCTSTR)"提示信息", MB_OK);
		}*/
	}

	void OnMessage_ReLoadModule(const char* pCommand)
	{
		MessageBox(NULL, (LPCTSTR)"test19", (LPCTSTR)"提示信息", MB_OK);
		/*string strModuleName = pCommand;
		bool blState = m_ManagerConnect.SendReLoadModule(pCommand);
		if(true == blState)
		{
		string strModuleName = pCommand;
		string strMessage = (string)"重载: " + strModuleName + (string)"成功。";
		MessageBox(NULL,(LPCTSTR)strMessage.c_str(), (LPCTSTR)"提示信息", MB_OK);
		}
		else
		{
		string strModuleName = pCommand;
		string strMessage = (string)"重载: " + strModuleName + (string)"失败。";
		MessageBox(NULL,(LPCTSTR)strMessage.c_str(), (LPCTSTR)"提示信息", MB_OK);
		}*/
		MessageBox(NULL, (LPCTSTR)"", (LPCTSTR)"提示信息", MB_OK);
	}

	LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
	{
		TCHAR szBuf[MAX_PATH] = {0};
		if(pControl->GetName() == _T("domainlist"))
		{
			switch (iSubItem)
			{
			case 0:
				{
					_stprintf(szBuf, "1");
					break;
				}
			case 1:
				{
					_stprintf(szBuf, "2");
					break;
				}
			case 2:
				{
					_stprintf(szBuf, "3");
					break;
				}
			}
		}

		if(pControl->GetName() == _T("commandlist"))
		{
			switch (iSubItem)
			{
			case 0:
				{
					_stprintf(szBuf, "名字");
					break;
				}
			case 1:
				{
					_stprintf(szBuf, "命令ID");
					break;
				}
			case 2:
				{
					_stprintf(szBuf, "%d", "数量");
					break;
				}
			case 3:
				{
					_stprintf(szBuf, "%d","时间");
					break;
				}
			}
		}
		pControl->SetUserData(szBuf);
		return pControl->GetUserData();
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		CDialogBuilderCallbackEx cb;
		CControlUI* pRoot = builder.Create(_T(SKINFILE_NAME), (UINT)0,  &cb, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		//加载图标
		char buf[MAX_PATH]; 
		::GetModuleFileName(0, (LPCH)&buf, MAX_PATH);

		HINSTANCE instance = ::GetModuleHandle(buf);

		HICON hIcon = ::LoadIcon(instance, MAKEINTRESOURCEA(IDI_ICON1)); 

		::SendMessage((HWND)m_hWnd, WM_SETICON, 1, (LPARAM)hIcon); 
		::SendMessage((HWND)m_hWnd, WM_SETICON, 1, (LPARAM)hIcon); 

		Init();
		return 0;
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		::PostQuitMessage(0L);

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( ::IsIconic(*this) ) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		RECT rcCaption = m_pm.GetCaptionRect();
		if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
				if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
					_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
					_tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
					return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
			CRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CRect rcWork = oMonitor.rcWork;
		rcWork.Offset(-rcWork.left, -rcWork.top);

		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
		lpMMI->ptMaxPosition.x	= rcWork.left;
		lpMMI->ptMaxPosition.y	= rcWork.top;
		lpMMI->ptMaxSize.x		= rcWork.right;
		lpMMI->ptMaxSize.y		= rcWork.bottom;

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
		if( wParam == SC_CLOSE ) {
			::PostQuitMessage(0L);
			bHandled = TRUE;
			return 0;
		}
		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if( ::IsZoomed(*this) != bZoomed ) {
			if( !bZoomed ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
				if( pControl ) pControl->SetVisible(false);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
				if( pControl ) pControl->SetVisible(true);
			}
			else {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
				if( pControl ) pControl->SetVisible(true);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
				if( pControl ) pControl->SetVisible(false);
			}
		}
		return lRes;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch( uMsg ) 
		{
			case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
			case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
			case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
			case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
			case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
			case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
			case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
			case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
			case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
			case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
			default:
				bHandled = FALSE;
		}
		if( bHandled ) return lRes;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

public:
	CPaintManagerUI m_pm;

private:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));
	//CPaintManagerUI::SetResourceZip(_T("skin.bpr"));

	HRESULT Hr = ::CoInitialize(NULL);
	if( FAILED(Hr) ) return 0;

	CFrameWindowWnd* pFrame = new CFrameWindowWnd();
	if( pFrame == NULL ) return 0;
	pFrame->Create(NULL, _T("Stress Client"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 800, 600);
	pFrame->CenterWindow();
	pFrame->ShowWindow(true);
	CPaintManagerUI::MessageLoop();

	::CoUninitialize();
	return 0;
}

