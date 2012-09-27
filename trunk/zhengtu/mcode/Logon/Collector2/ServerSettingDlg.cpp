// ServerSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ServerSettingDlg.h"


// CServerSettingDlg 对话框

IMPLEMENT_DYNAMIC(CServerSettingDlg, CDialog)
CServerSettingDlg::CServerSettingDlg(CWnd* pParent /*=NULL*/)
: CDialog(CServerSettingDlg::IDD, pParent)
{
}

CServerSettingDlg::~CServerSettingDlg()
{
}

void CServerSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CServerSettingDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OK, OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBnClickedBtnCancel)
END_MESSAGE_MAP()


// CServerSettingDlg 消息处理程序


BOOL CServerSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_IPADDR_DB)->SetWindowText(m_sDBIpAddr);
	GetDlgItem(IDC_EDIT_DB_USER)->SetWindowText(m_sDBUser);
	GetDlgItem(IDC_EDIT_DB_PSWD)->SetWindowText(m_sDBPswd);
	GetDlgItem(IDC_EDIT_DB_NAME)->SetWindowText(m_sDBName);
	CString sTemp;
	sTemp.Format(TEXT("%d"), m_iDBPort);
	GetDlgItem(IDC_EDIT_DB_PORT)->SetWindowText(sTemp);
	GetDlgItem(IDC_IPADDR_SERVER)->SetWindowText(m_sServerIPAddr);
	sTemp.Format(TEXT("%d"), m_iServerPort);
	GetDlgItem(IDC_EDIT_SERVER_PORT)->SetWindowText(sTemp);
	sTemp.Format(TEXT("%d"), m_iServerMaxConn);
	GetDlgItem(IDC_EDIT_SERVER_MAXCONN)->SetWindowText(sTemp);

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CServerSettingDlg::OnBnClickedBtnOk()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_IPADDR_DB)->GetWindowText(m_sDBIpAddr);
	GetDlgItem(IDC_EDIT_DB_USER)->GetWindowText(m_sDBUser);
	GetDlgItem(IDC_EDIT_DB_PSWD)->GetWindowText(m_sDBPswd);
	GetDlgItem(IDC_EDIT_DB_NAME)->GetWindowText(m_sDBName);
	CString sTmp;
	GetDlgItem(IDC_EDIT_DB_PORT)->GetWindowText(sTmp);
	m_iDBPort = atoi(sTmp.GetBuffer());

	GetDlgItem(IDC_IPADDR_SERVER)->GetWindowText(m_sServerIPAddr);
	GetDlgItem(IDC_EDIT_SERVER_PORT)->GetWindowText(sTmp);
	m_iServerPort = atoi(sTmp.GetBuffer());
	GetDlgItem(IDC_EDIT_SERVER_MAXCONN)->GetWindowText(sTmp);
	m_iServerMaxConn = atoi(sTmp.GetBuffer());
	__super::OnOK();
}

void CServerSettingDlg::OnBnClickedBtnCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	__super::OnCancel();
}
