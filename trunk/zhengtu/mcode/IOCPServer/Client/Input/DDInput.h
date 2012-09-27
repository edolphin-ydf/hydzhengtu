/********************************************************************
	创建日期		2004/09/23
	创建日期和时间:		23:9:2004   14:23
	文件名:			DDInput
	文件扩展名:		h
	作者:			美堂蛮
	
	作用:		基于DirectInput的类，取得键盘鼠标的状态

	注意点:		只有调用一次GetKeyState或GetMouseState然后才能得到一次设备状态
	建议把GetKeyState或GetMouseState放在主循环下一直调用
*********************************************************************/
#ifndef DDINPUT_H
#define DDINPUT_H

#include <DInput.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")
#define MOUSE_LEFT_DOWN		0 //鼠标左键按下 
#define MOUSE_RIGHT_DOWN	1 //鼠标右键按下
#define MOUSE_MID_DOWN		2 //鼠标中键按下

class CDDInput  
{
public:
	void ShowCursor(bool bShow);		//设置鼠标的显示
	void GetCursorPos(int &x, int &y);	//得到鼠标在屏幕中位置
	void SetCursorPos(int x, int y);	//设置鼠标在屏幕中位置
	void SetMousePos(int x, int y);		//设置鼠标在窗口中位置
	void GetMousePos(int &x, int &y);	//得到鼠标在窗口中位置
	void GetMouseCXYZ(long &lcx, long &lcy, long &lcz); //得到鼠标的位移坐标
	BOOL MidBnDown();			//检测鼠标中键是否按下
	BOOL RightBnDown();			//检测鼠标右键是否按下
	BOOL LeftBnDown();			//检测鼠标左键是否按下
	BOOL KeyDown(DWORD dwDik);	//键盘按下否

	BOOL UpdateKeyState();			//检测键盘的状态
	BOOL UpdateMouseState();		//检测鼠标的状态
	
	TCHAR* GetError();			//取得错误信息
	CDDInput(HWND hwnd);		
	virtual ~CDDInput();

private:
	UCHAR m_keyState[256];				//键盘状态表
	DIMOUSESTATE m_mouseState;			//鼠标状态结构
	LPDIRECTINPUTDEVICE8 m_lpdimouse;	//鼠标设备对象
	HINSTANCE m_hinst;					//实例句柄
	HWND m_hwnd;						
	LPDIRECTINPUTDEVICE8 m_lpdikey;		//键盘设备对象
	TCHAR strError[30];					//保存错误信息
	LPDIRECTINPUT8 m_lpdi;				//主设备对象
	BOOL InitKeyBoardDev();				//初始化键盘设备
	BOOL InitMouseDev();				//初始化鼠标设备
};

#endif // !defined(AFX_DDINPUT_H__08B85D0C_90DC_4177_AFE3_B4658810D8B4__INCLUDED_)
