// Collector2Dlg.h : ͷ�ļ�
//

#pragma once
#include "GameFrameManager.h"
//#include "MolMesDistributer.h"
#include "afxwin.h"
#include "network/MolNet.h"
//#include ".\OracleHelper\OracleHelper.h"
#include "ServerSettingDlg.h"

// CCollector2Dlg �Ի���
class CCollector2Dlg : public CDialog
{
// ����
public:
	CCollector2Dlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CCollector2Dlg();

// �Ի�������
	enum { IDD = IDD_COLLECTOR2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	
	void PrintLog(CString &strMsg);

// ʵ��
protected:
	HICON							m_hIcon;
	BOOL							m_bIsRunning;

	CString							m_sDBIpAddr;				//���ݿ�IP��ַ
	int								m_iDBPort;					//���ݿ�˿�
	CString							m_sDBUser;					//���ݿ��û���
	CString							m_sDBPswd;					//���ݿ�����
	CString							m_sDBName;					//���ݿ������
	CString							m_sServerIPAddr;			//������IP��ַ
	int								m_iServerPort;				//�������˿�
	int								m_iServerMaxConn;			//���������������

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnSetting();
	afx_msg void OnBnClickedBtnExit();
	CRichEditCtrl m_edit_log;
};
