// Collector2Dlg.h : 头文件
//

#pragma once
#include "GameFrameManager.h"
//#include "MolMesDistributer.h"
#include "afxwin.h"
#include "network/MolNet.h"
//#include ".\OracleHelper\OracleHelper.h"
#include "ServerSettingDlg.h"

// CCollector2Dlg 对话框
class CCollector2Dlg : public CDialog
{
// 构造
public:
	CCollector2Dlg(CWnd* pParent = NULL);	// 标准构造函数
	~CCollector2Dlg();

// 对话框数据
	enum { IDD = IDD_COLLECTOR2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	
	void PrintLog(CString &strMsg);

// 实现
protected:
	HICON							m_hIcon;
	BOOL							m_bIsRunning;

	CString							m_sDBIpAddr;				//数据库IP地址
	int								m_iDBPort;					//数据库端口
	CString							m_sDBUser;					//数据库用户名
	CString							m_sDBPswd;					//数据库密码
	CString							m_sDBName;					//数据库服务名
	CString							m_sServerIPAddr;			//服务器IP地址
	int								m_iServerPort;				//服务器端口
	int								m_iServerMaxConn;			//服务器最大连接数

	// 生成的消息映射函数
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
