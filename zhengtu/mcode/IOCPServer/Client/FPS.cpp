/********************************************************************
创建日期和时间		2004/09/23	14:23
更新日期和时间:		2004/11/005	09:38
文件名:			fps
文件扩展名:		cpp
作者:			美堂蛮

  作用:		计算FPS、计算程序代码的执行时间、进行锁频（游戏用）
  
	注意点:		由于是此类函数输出是在窗口标题栏上，DOS平台上只能测试速度不能显示FPS。
*********************************************************************/


#include <windows.h>
#include <stdio.h>
#include <mmSystem.h>
#pragma comment(lib, "Winmm.lib")
#include "fps.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFps::CFps(HWND hwnd)
{
	dLockFirst = 0.0;
	m_hwnd = NULL;
	//初始化
	m_hwnd = hwnd;
	dFPs = 0.0;

	sprintf(strFPS, "0");
	
	//保存窗口标题栏原来的文字
	char strTmp[100];
	GetWindowText(hwnd, strTmp, 200); 
	sprintf(strOldTitle, strTmp);
}

CFps::CFps()
{
	//初始化
	m_hwnd = NULL;
	dLockFirst = 0.0;
	dFPs = 0.0;
	sprintf(strFPS, "FPS:0");
}

CFps::~CFps()
{
	m_hwnd = NULL;
}

//显示FPS扩展版
void CFps::Update()
{
	static float fLastTime = 0.f;	// Absolute time at last frame
	static float fFramesPerSecond = 0.f;
	float        fTime;
	
	// Get the current tick count, and multiply it by 0.001 to convert it from 
	// milliseconds to seconds
    fTime = GetTickCount() * 0.001f;				
	
	// Increase the frame counter
	++ fFramesPerSecond;
	
	if( (fTime - fLastTime) > 1.f )
	{
		// Here, we set the lastTime to the currentTime.
		fLastTime = fTime;
		
		// Current FPS
		dFPs = fFramesPerSecond;
		
		//实现FPS和SPD一起显示的方法
		sprintf(strFPS, "FPS:%0.2f", dFPs);
		
		// Reset the frames per second
		fFramesPerSecond = 0.f;
	}
}

void CFps::Show()
{
	char ch[250];
	sprintf(ch, "%s  %s", strOldTitle,strFPS);
	printf("%s\n", ch);

	//显示在窗口标题栏上
	SetWindowText(m_hwnd, ch);
}

//开始锁频
void CFps::Lock(int nFrequency)
{
	//超过此范围程序不处理
	if (nFrequency <= 0 || nFrequency > 1000)
	{
		return;
	}

	double dTime=0;		//时间差	
	while( (dTime = (double)timeGetTime() - dLockFirst) < 1000/(double)nFrequency );

	dLockFirst = (double)timeGetTime();

	return;
}
