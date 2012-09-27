#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include "res\resource.h"

#include "define.h"
#include "Game.h"


//全局变量声明
HINSTANCE g_hInstance;
HWND g_hwnd;
CGame g_game;	//游戏对象


//函数声明
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


//主窗口入口////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wndclass;
	MSG message;
	
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hCursor = LoadCursor(hInstance, NULL);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WindowProc;
	wndclass.lpszClassName = "Program Class";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.hIconSm =NULL;
	
	if ( !(RegisterClassEx(&wndclass)) )
	{
		return (0);
	}
	
	g_hwnd = CreateWindowEx( NULL, "Program Class", "fs游戏窗口",
		DEFAULT_STYLE,//WS_OVERLAPPEDWINDOW ,
		0, 0, WND_WIDTH, WND_HEIGHT, NULL, NULL, hInstance, NULL);
	if (!g_hwnd)
	{
		return (0);
	}
	
	g_hInstance = hInstance;
	
	
	//调整窗口----------------------------------------------------------
	RECT window_rect = {0, 0, WND_WIDTH, WND_HEIGHT};
	AdjustWindowRectEx(&window_rect, DEFAULT_STYLE, FALSE, NULL); //WS_OVERLAPPEDWINDOW
	MoveWindow(g_hwnd, 50, 50, window_rect.right - window_rect.left,
		window_rect.bottom - window_rect.top, true);
	//-------------------------------------------------------------------
	
	ShowWindow(g_hwnd, SW_SHOW);
	UpdateWindow(g_hwnd);
	
	// 游戏初始化
	if ( !g_game.Init(g_hwnd) )
	{
		MessageBox(g_hwnd, "游戏初始化失败!", "错误", MB_OK | MB_ICONERROR);
		// 释放游戏资源
		g_game.OnQuit();
		exit(1);
	}

	//实时消息循环（占用CPU时间多，使用于"游戏程序"的开发）/////////////////////////////////////////////////
	static float fLastTime = (float)GetTickCount();
	while(TRUE)
	{
		if ( PeekMessage(&message, NULL, 0, 0, PM_REMOVE) )
		{
			if (message.message == WM_QUIT)
				break;
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		
		float fCurrTime = (float)GetTickCount();
		float fElapse = (fCurrTime - fLastTime);

		// 游戏逻辑
		if ( !g_game.OnTick(fElapse) )
		{
			MessageBox(g_hwnd, "游戏异常退出!", "错误", MB_OK | MB_ICONERROR);
			break;
		}

		// 游戏渲染
		if ( !g_game.OnRender() )
		{
			MessageBox(g_hwnd, "游戏异常退出!", "错误", MB_OK | MB_ICONERROR);
			break;
		}

		fLastTime = fCurrTime;
	}

	// 释放游戏资源
	g_game.OnQuit();
	
	return (int)(message.wParam);
}//end WinMain


bool g_bMouseDown = false;
int iLastX = 0;
int iLastY = 0;

//消息处理程序///////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{	
	case WM_DESTROY: 
		PostQuitMessage(1); //等同于PostMessage(NULL,WM_QUIT,NULL,NULL);
		return (0);
		break;

	case WM_CLOSE:
		PostQuitMessage(1); //等同于PostMessage(NULL,WM_QUIT,NULL,NULL);
		return (0);

	case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lparam);
			int y = HIWORD(lparam);
			iLastX = x;
			iLastY = y;
			g_bMouseDown = true;
			//SetCapture(hwnd);
		}
		return (0);

	case WM_MOUSEMOVE:
		{
			if (g_bMouseDown)
			{
				int x = LOWORD(lparam);
				int y = HIWORD(lparam);
				g_game.AddLine(iLastX, iLastY, x, y);
				g_game.SendAddLine(iLastX, iLastY, x, y);
				iLastX = x;
				iLastY = y;
			}
		}
		return (0);

	case WM_LBUTTONUP:
		{
			g_bMouseDown = false;
			//ReleaseCapture();
		}
		return (0);

	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
            PostQuitMessage(1); //等同于PostMessage(NULL,WM_QUIT,NULL,NULL);
		return (0);
		
	default:break;
	}
	return ( DefWindowProc(hwnd, msg, wparam, lparam) );
}

