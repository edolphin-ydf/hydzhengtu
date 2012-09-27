// DDInput.cpp: implementation of the CDDInput class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "DDInput.h"

#define SAFE_RELEASE(x){if (x){x->Release();x=NULL;}}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDDInput::CDDInput(HWND hwnd)
{
	m_hwnd = hwnd;
	m_hinst = GetModuleHandle(NULL);
	//m_hinst = hinst;
	
	if ( FAILED(DirectInput8Create(m_hinst, DIRECTINPUT_VERSION,
		IID_IDirectInput8, (void**)&m_lpdi,NULL)) )
	{
		sprintf(strError, "主DirectInput对象创建失败");
		return;
	}
	
	if (!InitKeyBoardDev())
	{
		sprintf(strError, "初始化键盘设备失败");
		return;
	}

	if (!InitMouseDev())
	{
		sprintf(strError, "初始化鼠标设备失败");
		return;
	}
}

CDDInput::~CDDInput()
{
	//释放主对象
	SAFE_RELEASE(m_lpdi);

	//释放键盘
	if (m_lpdikey)
		m_lpdikey->Unacquire();
	SAFE_RELEASE(m_lpdikey);

	//释放鼠标
	if (m_lpdimouse)
		m_lpdimouse->Unacquire();
	SAFE_RELEASE(m_lpdimouse);
}


////////////////////////////////////////
//键盘处理函数
////////////////////////////////////
BOOL CDDInput::InitKeyBoardDev()
{
	//创建键盘设备
	if ( FAILED(m_lpdi->CreateDevice(GUID_SysKeyboard, &m_lpdikey, NULL)) )
		return FALSE;

	//设置键盘协作等级
	if ( FAILED(m_lpdikey->SetCooperativeLevel(m_hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)) )
		return FALSE;

	//设置键盘数据格式
	if ( FAILED(m_lpdikey->SetDataFormat(&c_dfDIKeyboard)) )
		return FALSE;

	//获取键盘
	if ( FAILED(m_lpdikey->Acquire()) )
		return FALSE;

	return TRUE;
}

//检测键盘状态
BOOL CDDInput::UpdateKeyState()
{
	if ( FAILED(m_lpdikey->GetDeviceState(256, m_keyState)) )
		return FALSE;

	return TRUE;
}

//按键测试
BOOL CDDInput::KeyDown(DWORD dwDik)
{
	if ( m_keyState[dwDik] & 0x80 )
		return TRUE;
	else
		return FALSE;
}


//////////////////////////
//异常处理
//////////////////////////
//
//获取错误信息
TCHAR* CDDInput::GetError()
{
	return strError;
}


///////////////////////////////////////
//鼠标处理函数
///////////////////////////////////////
BOOL CDDInput::InitMouseDev()
{
	//创建鼠标设备
	if ( FAILED(m_lpdi->CreateDevice(GUID_SysMouse, &m_lpdimouse, NULL)) )
		return FALSE;
	
	//设置鼠标协作等级
	if ( FAILED(m_lpdimouse->SetCooperativeLevel(m_hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)) )
		return FALSE;
	
	//设置鼠标数据格式
	if ( FAILED(m_lpdimouse->SetDataFormat(&c_dfDIMouse)) )
		return FALSE;
	
	//获取鼠标
	if ( FAILED(m_lpdimouse->Acquire()) )
		return FALSE;
	
	return TRUE;
}

//检测鼠标状态
BOOL CDDInput::UpdateMouseState()
{
	if ( FAILED(m_lpdimouse->GetDeviceState(sizeof(DIMOUSESTATE), &m_mouseState)) )
		return FALSE;

	return TRUE;
}

//检测鼠标左键是否按下
BOOL CDDInput::LeftBnDown()
{
	if ( m_mouseState.rgbButtons[MOUSE_LEFT_DOWN] & 0x80 )
		return TRUE;
	return FALSE;
}

//检测鼠标右键是否按下
BOOL CDDInput::RightBnDown()
{
	if ( m_mouseState.rgbButtons[MOUSE_RIGHT_DOWN] & 0x80 )
		return TRUE;
	return FALSE;
}

//检测鼠标中键是否按下
BOOL CDDInput::MidBnDown()
{
	if ( m_mouseState.rgbButtons[MOUSE_MID_DOWN] & 0x80 )
		return TRUE;
	return FALSE;
}

//得到鼠标的位移坐标
void CDDInput::GetMouseCXYZ(long &lcx, long &lcy, long &lcz)
{
	lcx = m_mouseState.lX;
	lcy = m_mouseState.lY;
	lcz = m_mouseState.lZ;
}

//得到鼠标在窗口中位置
void CDDInput::GetMousePos(int &x, int &y)
{
	POINT point;
	::GetCursorPos(&point);
	ScreenToClient(m_hwnd, &point);
	x = point.x;
	y = point.y;
}

//设置鼠标在窗口中位置
void CDDInput::SetMousePos(int x, int y)
{
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(m_hwnd, &point);
	::SetCursorPos(point.x, point.y);
}

//得到鼠标在屏幕中位置
void CDDInput::GetCursorPos(int &x, int &y)
{
	POINT point;
	::GetCursorPos(&point);
	x = point.x;
	y = point.y;
}

//设置鼠标在屏幕中位置
void CDDInput::SetCursorPos(int x, int y)
{
	::SetCursorPos(x, y);
}

//设置鼠标的显示
void CDDInput::ShowCursor(bool bShow)
{
	::ShowCursor(bShow);
}
