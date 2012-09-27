#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include "res\resource.h"

#include "define.h"
#include "Game.h"


//ȫ�ֱ�������
HINSTANCE g_hInstance;
HWND g_hwnd;
CGame g_game;	//��Ϸ����


//��������
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


//���������////////////////////////////////////////////////////////////
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
	
	g_hwnd = CreateWindowEx( NULL, "Program Class", "fs��Ϸ����",
		DEFAULT_STYLE,//WS_OVERLAPPEDWINDOW ,
		0, 0, WND_WIDTH, WND_HEIGHT, NULL, NULL, hInstance, NULL);
	if (!g_hwnd)
	{
		return (0);
	}
	
	g_hInstance = hInstance;
	
	
	//��������----------------------------------------------------------
	RECT window_rect = {0, 0, WND_WIDTH, WND_HEIGHT};
	AdjustWindowRectEx(&window_rect, DEFAULT_STYLE, FALSE, NULL); //WS_OVERLAPPEDWINDOW
	MoveWindow(g_hwnd, 50, 50, window_rect.right - window_rect.left,
		window_rect.bottom - window_rect.top, true);
	//-------------------------------------------------------------------
	
	ShowWindow(g_hwnd, SW_SHOW);
	UpdateWindow(g_hwnd);
	
	// ��Ϸ��ʼ��
	if ( !g_game.Init(g_hwnd) )
	{
		MessageBox(g_hwnd, "��Ϸ��ʼ��ʧ��!", "����", MB_OK | MB_ICONERROR);
		// �ͷ���Ϸ��Դ
		g_game.OnQuit();
		exit(1);
	}

	//ʵʱ��Ϣѭ����ռ��CPUʱ��࣬ʹ����"��Ϸ����"�Ŀ�����/////////////////////////////////////////////////
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

		// ��Ϸ�߼�
		if ( !g_game.OnTick(fElapse) )
		{
			MessageBox(g_hwnd, "��Ϸ�쳣�˳�!", "����", MB_OK | MB_ICONERROR);
			break;
		}

		// ��Ϸ��Ⱦ
		if ( !g_game.OnRender() )
		{
			MessageBox(g_hwnd, "��Ϸ�쳣�˳�!", "����", MB_OK | MB_ICONERROR);
			break;
		}

		fLastTime = fCurrTime;
	}

	// �ͷ���Ϸ��Դ
	g_game.OnQuit();
	
	return (int)(message.wParam);
}//end WinMain


bool g_bMouseDown = false;
int iLastX = 0;
int iLastY = 0;

//��Ϣ�������///////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{	
	case WM_DESTROY: 
		PostQuitMessage(1); //��ͬ��PostMessage(NULL,WM_QUIT,NULL,NULL);
		return (0);
		break;

	case WM_CLOSE:
		PostQuitMessage(1); //��ͬ��PostMessage(NULL,WM_QUIT,NULL,NULL);
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
            PostQuitMessage(1); //��ͬ��PostMessage(NULL,WM_QUIT,NULL,NULL);
		return (0);
		
	default:break;
	}
	return ( DefWindowProc(hwnd, msg, wparam, lparam) );
}

