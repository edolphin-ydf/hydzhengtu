/********************************************************************
	��������		2004/09/23
	�������ں�ʱ��:		23:9:2004   14:23
	�ļ���:			DDInput
	�ļ���չ��:		h
	����:			������
	
	����:		����DirectInput���࣬ȡ�ü�������״̬

	ע���:		ֻ�е���һ��GetKeyState��GetMouseStateȻ����ܵõ�һ���豸״̬
	�����GetKeyState��GetMouseState������ѭ����һֱ����
*********************************************************************/
#ifndef DDINPUT_H
#define DDINPUT_H

#include <DInput.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")
#define MOUSE_LEFT_DOWN		0 //���������� 
#define MOUSE_RIGHT_DOWN	1 //����Ҽ�����
#define MOUSE_MID_DOWN		2 //����м�����

class CDDInput  
{
public:
	void ShowCursor(bool bShow);		//����������ʾ
	void GetCursorPos(int &x, int &y);	//�õ��������Ļ��λ��
	void SetCursorPos(int x, int y);	//�����������Ļ��λ��
	void SetMousePos(int x, int y);		//��������ڴ�����λ��
	void GetMousePos(int &x, int &y);	//�õ�����ڴ�����λ��
	void GetMouseCXYZ(long &lcx, long &lcy, long &lcz); //�õ�����λ������
	BOOL MidBnDown();			//�������м��Ƿ���
	BOOL RightBnDown();			//�������Ҽ��Ƿ���
	BOOL LeftBnDown();			//����������Ƿ���
	BOOL KeyDown(DWORD dwDik);	//���̰��·�

	BOOL UpdateKeyState();			//�����̵�״̬
	BOOL UpdateMouseState();		//�������״̬
	
	TCHAR* GetError();			//ȡ�ô�����Ϣ
	CDDInput(HWND hwnd);		
	virtual ~CDDInput();

private:
	UCHAR m_keyState[256];				//����״̬��
	DIMOUSESTATE m_mouseState;			//���״̬�ṹ
	LPDIRECTINPUTDEVICE8 m_lpdimouse;	//����豸����
	HINSTANCE m_hinst;					//ʵ�����
	HWND m_hwnd;						
	LPDIRECTINPUTDEVICE8 m_lpdikey;		//�����豸����
	TCHAR strError[30];					//���������Ϣ
	LPDIRECTINPUT8 m_lpdi;				//���豸����
	BOOL InitKeyBoardDev();				//��ʼ�������豸
	BOOL InitMouseDev();				//��ʼ������豸
};

#endif // !defined(AFX_DDINPUT_H__08B85D0C_90DC_4177_AFE3_B4658810D8B4__INCLUDED_)
