#pragma once

class CModuleManagerUI : public CContainerUI
{
public:
	CModuleManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("ModuleManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CConnectManagerUI : public CContainerUI
{
public:
	CConnectManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("ConnectManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CLogManagerUI : public CContainerUI
{
public:
	CLogManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("LogManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CWorkThreadManagerUI : public CContainerUI
{
public:
	CWorkThreadManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("WorkThreadManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CTestManagerUI : public CContainerUI
{
public:
	CTestManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("TestManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CClientConnectManagerUI : public CContainerUI
{
public:
	CClientConnectManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("ClientConnectManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CForbiddenManagerUI : public CContainerUI
{
public:
	CForbiddenManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("ForbiddenManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CServerConnectManagerUI : public CContainerUI
{
public:
	CServerConnectManagerUI()
	{
		CDialogBuilder builder;
		CContainerUI* pComputerExamine = static_cast<CContainerUI*>(builder.Create(_T("ServerConnectManager.xml"), (UINT)0));
		if( pComputerExamine ) {
			this->Add(pComputerExamine);
		}
		else {
			this->RemoveAll();
			return;
		}
	}
};

class CDialogBuilderCallbackEx : public IDialogBuilderCallback
{
public:
	CControlUI* CreateControl(LPCTSTR pstrClass) 
	{
		if( _tcscmp(pstrClass, _T("ModuleManager")) == 0 ) return new CModuleManagerUI;
		if( _tcscmp(pstrClass, _T("ConnectManager")) == 0 ) return new CConnectManagerUI;
		if( _tcscmp(pstrClass, _T("LogManager")) == 0 ) return new CLogManagerUI;
		if( _tcscmp(pstrClass, _T("WorkThreadManager")) == 0 ) return new CWorkThreadManagerUI;
		if( _tcscmp(pstrClass, _T("TestManager")) == 0 ) return new CTestManagerUI;
		if( _tcscmp(pstrClass, _T("ClientConnectManager")) == 0 ) return new CClientConnectManagerUI;
		if( _tcscmp(pstrClass, _T("ForbiddenIP")) == 0 ) return new CForbiddenManagerUI;
		if( _tcscmp(pstrClass, _T("ServerConnectManager")) == 0 ) return new CServerConnectManagerUI;
		return NULL;
	}
};