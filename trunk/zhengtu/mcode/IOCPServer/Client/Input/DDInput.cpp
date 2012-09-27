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
		sprintf(strError, "��DirectInput���󴴽�ʧ��");
		return;
	}
	
	if (!InitKeyBoardDev())
	{
		sprintf(strError, "��ʼ�������豸ʧ��");
		return;
	}

	if (!InitMouseDev())
	{
		sprintf(strError, "��ʼ������豸ʧ��");
		return;
	}
}

CDDInput::~CDDInput()
{
	//�ͷ�������
	SAFE_RELEASE(m_lpdi);

	//�ͷż���
	if (m_lpdikey)
		m_lpdikey->Unacquire();
	SAFE_RELEASE(m_lpdikey);

	//�ͷ����
	if (m_lpdimouse)
		m_lpdimouse->Unacquire();
	SAFE_RELEASE(m_lpdimouse);
}


////////////////////////////////////////
//���̴�����
////////////////////////////////////
BOOL CDDInput::InitKeyBoardDev()
{
	//���������豸
	if ( FAILED(m_lpdi->CreateDevice(GUID_SysKeyboard, &m_lpdikey, NULL)) )
		return FALSE;

	//���ü���Э���ȼ�
	if ( FAILED(m_lpdikey->SetCooperativeLevel(m_hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)) )
		return FALSE;

	//���ü������ݸ�ʽ
	if ( FAILED(m_lpdikey->SetDataFormat(&c_dfDIKeyboard)) )
		return FALSE;

	//��ȡ����
	if ( FAILED(m_lpdikey->Acquire()) )
		return FALSE;

	return TRUE;
}

//������״̬
BOOL CDDInput::UpdateKeyState()
{
	if ( FAILED(m_lpdikey->GetDeviceState(256, m_keyState)) )
		return FALSE;

	return TRUE;
}

//��������
BOOL CDDInput::KeyDown(DWORD dwDik)
{
	if ( m_keyState[dwDik] & 0x80 )
		return TRUE;
	else
		return FALSE;
}


//////////////////////////
//�쳣����
//////////////////////////
//
//��ȡ������Ϣ
TCHAR* CDDInput::GetError()
{
	return strError;
}


///////////////////////////////////////
//��괦����
///////////////////////////////////////
BOOL CDDInput::InitMouseDev()
{
	//��������豸
	if ( FAILED(m_lpdi->CreateDevice(GUID_SysMouse, &m_lpdimouse, NULL)) )
		return FALSE;
	
	//�������Э���ȼ�
	if ( FAILED(m_lpdimouse->SetCooperativeLevel(m_hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)) )
		return FALSE;
	
	//����������ݸ�ʽ
	if ( FAILED(m_lpdimouse->SetDataFormat(&c_dfDIMouse)) )
		return FALSE;
	
	//��ȡ���
	if ( FAILED(m_lpdimouse->Acquire()) )
		return FALSE;
	
	return TRUE;
}

//������״̬
BOOL CDDInput::UpdateMouseState()
{
	if ( FAILED(m_lpdimouse->GetDeviceState(sizeof(DIMOUSESTATE), &m_mouseState)) )
		return FALSE;

	return TRUE;
}

//����������Ƿ���
BOOL CDDInput::LeftBnDown()
{
	if ( m_mouseState.rgbButtons[MOUSE_LEFT_DOWN] & 0x80 )
		return TRUE;
	return FALSE;
}

//�������Ҽ��Ƿ���
BOOL CDDInput::RightBnDown()
{
	if ( m_mouseState.rgbButtons[MOUSE_RIGHT_DOWN] & 0x80 )
		return TRUE;
	return FALSE;
}

//�������м��Ƿ���
BOOL CDDInput::MidBnDown()
{
	if ( m_mouseState.rgbButtons[MOUSE_MID_DOWN] & 0x80 )
		return TRUE;
	return FALSE;
}

//�õ�����λ������
void CDDInput::GetMouseCXYZ(long &lcx, long &lcy, long &lcz)
{
	lcx = m_mouseState.lX;
	lcy = m_mouseState.lY;
	lcz = m_mouseState.lZ;
}

//�õ�����ڴ�����λ��
void CDDInput::GetMousePos(int &x, int &y)
{
	POINT point;
	::GetCursorPos(&point);
	ScreenToClient(m_hwnd, &point);
	x = point.x;
	y = point.y;
}

//��������ڴ�����λ��
void CDDInput::SetMousePos(int x, int y)
{
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(m_hwnd, &point);
	::SetCursorPos(point.x, point.y);
}

//�õ��������Ļ��λ��
void CDDInput::GetCursorPos(int &x, int &y)
{
	POINT point;
	::GetCursorPos(&point);
	x = point.x;
	y = point.y;
}

//�����������Ļ��λ��
void CDDInput::SetCursorPos(int x, int y)
{
	::SetCursorPos(x, y);
}

//����������ʾ
void CDDInput::ShowCursor(bool bShow)
{
	::ShowCursor(bShow);
}
