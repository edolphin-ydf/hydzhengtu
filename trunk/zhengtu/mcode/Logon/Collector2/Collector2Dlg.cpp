// Collector2Dlg.cpp : ʵ���ļ�
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


// CCollector2Dlg ��Ϣ�������

BOOL CCollector2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	/*MolCrash_Initiation();
	MolCrash_SetProjectName("sansanchess_loginserver");
	MolCrash_SetEmailSender("akinggw@sina.com");
	MolCrash_SetEmailReceiver("akinggw@126.com");
	MolCrash_DeleteSended(false);
	MolCrash_SetSmtpServer("smtp.sina.com");
	MolCrash_SetSmtpUser("akinggw");
	MolCrash_SetSmtpPassword("akinggw12");*/

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	new GameFrameManager();
	new DBOperator();
	new GameServerManager();

	CString strMsg;
	strMsg = "��ϵͳ�� ��ʼ�����.";
	PrintLog(strMsg);
	strMsg.Empty();

	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCollector2Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strTmp;

	InitMolNet(m_iServerMaxConn+1);
	if(!StartMolNet((LPCSTR)CStringA(m_sServerIPAddr),m_iServerPort))
	{
		strTmp.Format(TEXT("��ϵͳ�� ����������ʧ��,IP��ַ:%s,�˿�:%d"),m_sDBIpAddr,m_iDBPort);
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
		strTmp.Format("��ϵͳ�� ���ݿ����������ʧ��,IP��ַ:%s,�˿�:%d,���ݿ�����:%s,�û���:%s,����:%s",
			m_sDBIpAddr.GetBuffer(),
			m_iDBPort,
			m_sDBName.GetBuffer(),
			m_sDBUser.GetBuffer(),
			m_sDBPswd.GetBuffer());
		PrintLog(strTmp);
		return;
	}

	strTmp.Format("��ϵͳ�� ���ݿ�����������ɹ�,IP��ַ:%s,�˿�:%d,���ݿ�����:%s,�û���:%s,����:%s",
		m_sDBIpAddr.GetBuffer(),
		m_iDBPort,
		m_sDBName.GetBuffer(),
		m_sDBUser.GetBuffer(),
		m_sDBPswd.GetBuffer());
	PrintLog(strTmp);*/

	strTmp.Format(TEXT("��ϵͳ�� �����������ɹ���IP��ַ:%s,�˿�:%d,��ʼ��������..."),m_sServerIPAddr.GetBuffer(),m_iServerPort);
	PrintLog(strTmp);
	strTmp.Empty();

	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_SETTING)->EnableWindow(FALSE);
	m_bIsRunning = TRUE;
}

void CCollector2Dlg::OnBnClickedBtnStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CleanMolNet();

	ServerDBOperator.Shutdown();

	CString strMsg = TEXT("��ϵͳ�� �Ѿ��ر������������");
	PrintLog(strMsg);
	strMsg.Empty();

	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_SETTING)->EnableWindow(TRUE);
	m_bIsRunning = FALSE;
}

void CCollector2Dlg::OnBnClickedBtnSetting()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
