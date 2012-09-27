#pragma once
#include "Resource.h"
// CServerSettingDlg 对话框

class CServerSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CServerSettingDlg)

public:
	CServerSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CServerSettingDlg();

	// 对话框数据
	enum { IDD = IDD_DLG_SETTING };

	CString					m_sDBIpAddr;
	int						m_iDBPort;
	CString					m_sDBUser;
	CString					m_sDBPswd;
	CString					m_sDBName;
	CString					m_sServerIPAddr;
	int						m_iServerPort;
	int						m_iServerMaxConn;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnCancel();
};
