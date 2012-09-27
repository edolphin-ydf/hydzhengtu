/********************************************************************
创建日期和时间		2004/09/23	14:23
更新日期和时间:		2004/11/005	09:38
文件名:			fps
文件扩展名:		h
作者:			美堂蛮

  作用:		计算FPS、计算程序代码的执行时间、进行锁频（游戏用）
  
	注意点:		由于是此类函数输出是在窗口标题栏上，DOS平台上只能测试速度不能显示FPS。
*********************************************************************/
#pragma once

#include "windows.h"

//显示FPS和代码执行所花的时间
class CFps  
{
public:
	void Lock(int nFrequency);		//开始锁频
	void Update(void);		//显示FPS(参数单位为毫秒)
	void Show(void);		//显示FPS(参数单位为毫秒)
	CFps(HWND);			//传入窗口句柄
	CFps();	
	virtual ~CFps();
	
	TCHAR strFPS[30];				//存放FPS	
	
private:
	double dLockFirst;		//锁频开始
	TCHAR strOldTitle[100];	//保存窗口标题栏原来的信息
	HWND m_hwnd;
	double dFPs;
};

