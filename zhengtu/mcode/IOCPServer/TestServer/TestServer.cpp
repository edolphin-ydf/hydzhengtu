// Server.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <tchar.h>
#include <windows.h>
#include <string.h>
#include ".\gameworld.h"
using namespace std;


CGameWorld g_gameWorld;

#define CONFIG_FILE	".\\config.ini"


int _tmain(int argc, _TCHAR* argv[])
{
	UINT nPort = 10000;
	UINT nMaxClient = 10;

	nPort = GetPrivateProfileInt("config", "Port", 0, CONFIG_FILE);
	nMaxClient = GetPrivateProfileInt("config", "MaxClient", 0, CONFIG_FILE);

	// init game world
	g_gameWorld.Init(nMaxClient, nPort);

	// update game logic
	float fLastTime = (float)GetTickCount();
	float fCurrTime = 0;
	float fElapse = 0;

	float fTick1 = 0;
	float fTick2 = 0;
	while(TRUE)
	{
		fCurrTime = (float)GetTickCount();
		fElapse = (fCurrTime - fLastTime);
		fTick1 = fCurrTime;
		
		// 游戏逻辑
		if ( !g_gameWorld.OnTick(fElapse) )
		{
			printf("游戏异常退出!");
			break;
		}

		fLastTime = fCurrTime;
		fTick2 = (float)GetTickCount();

		Sleep(1);
	}

	g_gameWorld.OnQuit();
	getchar();
	return 0;
}

