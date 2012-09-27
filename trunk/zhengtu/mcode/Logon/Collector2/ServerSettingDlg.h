#pragma once
#include "Resource.h"
// CServerSettingDlg �Ի���

class CServerSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CServerSettingDlg)

public:
	CServerSettingDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CServerSettingDlg();

	// �Ի�������
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnCancel();
};
