#pragma once
#include <Windows.h>
#include "DDInput.h"
#include "FS_Engine.h"
#include "FPS.h"

struct LINE
{
	int x0;
	int y0;
	int x1;
	int y1;
};

#include <list>
using namespace std;

class CGame
{
public:
	CGame(void);
	virtual ~CGame(void);

	// 游戏初始化，返回0退出游戏
	BOOL Init(HWND hWnd);
	
	// 处理游戏逻辑，返回0退出游戏
	// fTick: 帧间隔时间
	int OnTick(float fTick);

	DWORD ReceiveNetPack();

	// 渲染游戏画面，返回0退出游戏
	int OnRender(void);

	// 游戏退出事件，释放游戏资源
	void OnQuit(void);

	void AddLine(int iX1, int iY1, int iX2, int iY2);

	void SendAddLine(int iX1, int iY1, int iX2, int iY2);

	FSClient* GetClientNet() {return m_pClient;}
	


private:
	HWND m_hWnd;	// 保存窗口句柄
	CFps *m_pFps;

	FSClient *m_pClient;	//网络

	CDDInput *m_pInput;		// 输入设备
	fsGraphics* m_pGrp;		// 图形库

	list<LINE*> m_listLine;	// 点列表

	//CTexture m_objMouse;	// 鼠标
};

