// Collector2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Collector2.h"
#include "Collector2Dlg.h"
#include "network/MolSocketMgrWin32.h"
#include "network/MolNet.h"
#include "DBOperator.h"
#include "GameServerManager.h"

//#include "libcrashrpt/MolCrashRpt.h"

//#pragma comment(lib, "libcrashrpt/MolCrashRpt.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCollector2Dlg::CCollector2Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCollector2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bIsRunning = FALSE;
	m_sDBIpAddr = AfxGetApp()->GetProfileString(TEXT("CollectorOption"),TEXT("DBIPAddr"));
	m_iDBPort = AfxGetApp()->GetProfileInt(TEXT("CollectorOption"),TEXT("DBPort"),0);
	m_sDBUser = AfxGetApp()->GetProfileString(TEXT("CollectorOption"),TEXT("DBUser"));
	m_sDBPswd = AfxGetApp()->GetProfileString(TEXT("CollectorOption"),TEXT("DBPswd"));
	m_sDBName = AfxGetApp()->GetProfileString(TEXT("CollectorOption"),TEXT("DBName"));
	m_sServerIPAddr = AfxGetApp()->GetProfileString(TEXT("CollectorOption"),TEXT("erverIPAddr"));
	m_iServerPort = AfxGetApp()->GetProfileInt(TEXT("CollectorOption"),TEXT("ServerPort"),0);
	m_iServerMaxConn = AfxGetApp()->GetProfileInt(TEXT("CollectorOption"),TEXT("ServerMaxConn"),0);
}

CCollector2Dlg::~CCollector2Dlg()
{
	CleanMolNet();

	//delete COracleHelper::getSingletonPtr();
	delete DBOperator::getSingletonPtr();
	delete GameFrameManager::getSingletonPtr();
	delete GameServerManager::getSingletonPtr();
}

void CCollector2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOGTWO, m_edit_log);
}

BEGIN_MESSAGE_MAP(CCollector2Dlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_START, OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_SETTING, OnBnClickedBtnSetting)
	ON_BN_CLICKED(IDC_BTN_EXIT, OnBnClickedBtnExit)
END_MESSAGE_MAP()


// CCollector2Dlg 消息处理程序

BOOL CCollector2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	/*MolCrash_Initiation();
	MolCrash_SetProjectName("sansanchess_loginserver");
	MolCrash_SetEmailSender("akinggw@sina.com");
	MolCrash_SetEmailReceiver("akinggw@126.com");
	MolCrash_DeleteSended(false);
	MolCrash_SetSmtpServer("smtp.sina.com");
	MolCrash_SetSmtpUser("akinggw");
	MolCrash_SetSmtpPassword("akinggw12");*/

	// TODO: 在此添加额外的初始化代码
	new GameFrameManager();
	new DBOperator();
	new GameServerManager();

	CString strMsg;
	strMsg = "【系统】 初始化完毕.";
	PrintLog(strMsg);
	strMsg.Empty();

	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCollector2Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CCollector2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCollector2Dlg::OnTimer(UINT nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
}

void CCollector2Dlg::OnBnClickedBtnStart()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strTmp;

	InitMolNet(m_iServerMaxConn+1);
	if(!StartMolNet((LPCSTR)CStringA(m_sServerIPAddr),m_iServerPort))
	{
		strTmp.Format(TEXT("【系统】 服务器启动失败,IP地址:%s,端口:%d"),m_sDBIpAddr,m_iDBPort);
		PrintLog(strTmp);
		strTmp.Empty();
		return;
	}

	/*if(!ServerDBOperator.Initilize(m_sDBIpAddr.GetBuffer(),
		m_sDBUser.GetBuffer(),
		m_sDBPswd.GetBuffer(),
		m_sDBName.GetBuffer(),
		m_iDBPort))
	{
		strTmp.Format("【系统】 数据库服务器启动失败,IP地址:%s,端口:%d,数据库名称:%s,用户名:%s,密码:%s",
			m_sDBIpAddr.GetBuffer(),
			m_iDBPort,
			m_sDBName.GetBuffer(),
			m_sDBUser.GetBuffer(),
			m_sDBPswd.GetBuffer());
		PrintLog(strTmp);
		return;
	}

	strTmp.Format("【系统】 数据库服务器启动成功,IP地址:%s,端口:%d,数据库名称:%s,用户名:%s,密码:%s",
		m_sDBIpAddr.GetBuffer(),
		m_iDBPort,
		m_sDBName.GetBuffer(),
		m_sDBUser.GetBuffer(),
		m_sDBPswd.GetBuffer());
	PrintLog(strTmp);*/

	strTmp.Format(TEXT("【系统】 服务器启动成功，IP地址:%s,端口:%d,开始接受连接..."),m_sServerIPAddr.GetBuffer(),m_iServerPort);
	PrintLog(strTmp);
	strTmp.Empty();

	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_SETTING)->EnableWindow(FALSE);
	m_bIsRunning = TRUE;
}

void CCollector2Dlg::OnBnClickedBtnStop()
{
	// TODO: 在此添加控件通知处理程序代码
	CleanMolNet();

	ServerDBOperator.Shutdown();

	CString strMsg = TEXT("【系统】 已经关闭网络服务器。");
	PrintLog(strMsg);
	strMsg.Empty();

	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_SETTING)->EnableWindow(TRUE);
	m_bIsRunning = FALSE;
}

void CCollector2Dlg::OnBnClickedBtnSetting()
{
	// TODO: 在此添加控件通知处理程序代码
	CServerSettingDlg dlgSetting;
	dlgSetting.m_sDBIpAddr = m_sDBIpAddr;
	dlgSetting.m_iDBPort = m_iDBPort;
	dlgSetting.m_sDBUser = m_sDBUser;
	dlgSetting.m_sDBPswd = m_sDBPswd;
	dlgSetting.m_sDBName = m_sDBName;
	dlgSetting.m_sServerIPAddr = m_sServerIPAddr;
	dlgSetting.m_iServerPort = m_iServerPort;
	dlgSetting.m_iServerMaxConn = m_iServerMaxConn;

	if (dlgSetting.DoModal() == IDOK)
	{
		m_sDBIpAddr = dlgSetting.m_sDBIpAddr;
		m_iDBPort = dlgSetting.m_iDBPort;
		m_sDBUser = dlgSetting.m_sDBUser;
		m_sDBPswd = dlgSetting.m_sDBPswd;
		m_sDBName = dlgSetting.m_sDBName;
		m_sServerIPAddr = dlgSetting.m_sServerIPAddr;
		m_iServerPort = dlgSetting.m_iServerPort;
		m_iServerMaxConn = dlgSetting.m_iServerMaxConn;

		AfxGetApp()->WriteProfileString(TEXT("CollectorOption"),TEXT("DBIPAddr"),m_sDBIpAddr);
		AfxGetApp()->WriteProfileInt(TEXT("CollectorOption"),TEXT("DBPort"),m_iDBPort);
		AfxGetApp()->WriteProfileString(TEXT("CollectorOption"),TEXT("DBUser"),m_sDBUser);
		AfxGetApp()->WriteProfileString(TEXT("CollectorOption"),TEXT("DBPswd"),m_sDBPswd);
		AfxGetApp()->WriteProfileString(TEXT("CollectorOption"),TEXT("DBName"),m_sDBName);
		AfxGetApp()->WriteProfileString(TEXT("CollectorOption"),TEXT("erverIPAddr"),m_sServerIPAddr);
		AfxGetApp()->WriteProfileInt(TEXT("CollectorOption"),TEXT("ServerPort"),m_iServerPort);
		AfxGetApp()->WriteProfileInt(TEXT("CollectorOption"),TEXT("ServerMaxConn"),m_iServerMaxConn);
	}
}

void CCollector2Dlg::OnBnClickedBtnExit()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsRunning)
	{
		if(AfxMessageBox(TEXT("Server is Running, Exit?"), MB_YESNO) == IDYES)
		{
			OnBnClickedBtnStop();
			AfxGetMainWnd()->SendMessage(WM_CLOSE);
		}
	}
	else
	{
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
	}
}

void CCollector2Dlg::PrintLog(CString &strMsg)
{
	if(strMsg.IsEmpty() || !m_edit_log.GetSafeHwnd()) return;

	if (m_edit_log.GetLineCount() >= 200)
	{
		m_edit_log.SetWindowText(TEXT(""));
	}

	int iLength = m_edit_log.GetWindowTextLength();
	m_edit_log.SetSel(iLength, iLength);
	strMsg.Append(TEXT("\r\n"));
	m_edit_log.ReplaceSel(strMsg);
	m_edit_log.ScrollWindow(0,0);
}
