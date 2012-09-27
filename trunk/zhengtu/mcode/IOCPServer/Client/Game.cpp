#include ".\game.h"
#include "define.h"
#include <crtdbg.h>
#include "event.h"
#include "log.h"


//#define OPEN_LOG

CGame::CGame(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// 检测内存泄漏
	m_pInput = NULL;
	m_pFps = NULL;
}

CGame::~CGame(void)
{
}

void CGame::AddLine(int iX1, int iY1, int iX2, int iY2)
{
	LINE *pLine = new LINE;
	pLine->x0 = iX1;
	pLine->y0 = iY1;
	pLine->x1 = iX2;
	pLine->y1 = iY2;
	m_listLine.push_back(pLine);
}

void CGame::SendAddLine(int iX1, int iY1, int iX2, int iY2)
{
	PACKET_PlayerDrawEvent pack;
	pack.Type = 1;
	pack.iX1 = iX1;
	pack.iY1 = iY1;
	pack.iX2 = iX2;
	pack.iY2 = iY2;
	//pack.dwClientSendTime = GetTickCount();
	pack.Create();
	m_pClient->SendPacket(&pack);
}

// 游戏初始化，返回0退出游戏
BOOL CGame::Init(HWND hWnd)
{
	if (hWnd == NULL)
		return FALSE;

	m_hWnd = hWnd;

	char chIP[250];
	GetPrivateProfileString("server", "ip", "127.0.0.1", chIP, 250, ".\\serverip.ini");

	m_pClient = FSSocketFactory::CreateClient();
	if( !m_pClient->Connect(chIP, 10000) )
	{
		//MessageBox(m_hWnd, "连接服务器失败!", "错误", MB_OK);
		//return FALSE;
	}

	m_pFps = new CFps(m_hWnd);

	m_pInput = new CDDInput(m_hWnd);
	//m_pInput->ShowCursor( false );

	m_pGrp = fsGraphicsFactory::CreateGraphics(GRAPHICS_DX3D8);
	m_pGrp->Init(m_hWnd, WND_WIDTH, WND_HEIGHT);
	//m_pGrp->Resize(WND_WIDTH, WND_HEIGHT, false);

// 	m_objMouse.Load("mouse.bmp");
// 	m_objMouse.SetColorKey(RGB(0,0,0));

	return TRUE;
}


DWORD CGame::ReceiveNetPack()
{
	// 处理网络消息
	const FS_PACKET* pPacket = m_pClient->GetPacket();
	if (pPacket == NULL)
		return 0;

	switch(pPacket->nID)
	{
	case PID_SOCKET_DISCONNECT:
		{
			int i=0;
		}
		break;

	case PACK_TYPE_PLAYER_DRAW:
		{
			PACKET_PlayerDrawEvent *pEvent = (PACKET_PlayerDrawEvent*)pPacket;
			//pEvent->dwClientRevTime = GetTickCount();
			AddLine(pEvent->iX1, pEvent->iY1, pEvent->iX2, pEvent->iY2);

#ifdef OPEN_LOG
			char ch[256] = "error";

			// 记录远程客户端绘制线坐标
			sprintf(ch, "[point: %d,%d - %d,%d]\n", pEvent->iX1, pEvent->iY1, pEvent->iX2, pEvent->iY2);
			WriteLog(ch);

			// 记录远程客户端发送时间
			sprintf(ch, "ClientSendTime: %d\n", pEvent->dwClientSendTime);
			WriteLog(ch);

			// 服务器转发时间
			sprintf(ch, "ServTransTime: %d\t", pEvent->dwServerRevTransTime);
			WriteLog(ch);

			// 时差
			sprintf(ch, "C-S DelayTime: %d\n", pEvent->dwServerRevTransTime - pEvent->dwClientSendTime);
			WriteLog(ch);

			// 记录本客户端接收时间
			sprintf(ch, "ClientRevTime: %d\t", pEvent->dwClientRevTime);
			WriteLog(ch);

			// 时差
			sprintf(ch, "S-C DelayTime: %d\n\n", pEvent->dwClientRevTime - pEvent->dwServerRevTransTime);
			WriteLog(ch);
#endif
		}
		break;

	default:
		{
			char ch[256] = "error";
			sprintf(ch,"Unhandled packet (not a problem): %i\n", pPacket->nID);
			OutputDebugString(ch);
		}
		break;
	}

	m_pClient->DeletePacket(&pPacket);

	return 0;
}



// 处理游戏逻辑，返回0退出游戏
// fTick: 帧间隔时间
int CGame::OnTick(float fTick)
{
	ReceiveNetPack();

	m_pFps->Update();

//	m_pInput->UpdateKeyState();
// 	m_pInput->UpdateMouseState();
// 
// 	// 鼠标控制
// 	int iX;
// 	int iY;
// 	m_pInput->GetMousePos(iX, iY);
// 	m_objMouse.SetPos(iX, iY); 

	return 1;
}


// 渲染游戏画面，返回0退出游戏
int CGame::OnRender(void)
{
	m_pGrp->Clear(ARGB(255,0,0,0));
	m_pGrp->BeginScene();

	// 画地图,人物，对象，怪物...
	//m_pGrp->DrawText("网络大地图演示", 10, 10, RGB(255,0,0));

	list<LINE*>::iterator it = m_listLine.begin();
	list<LINE*>::iterator itEnd = m_listLine.end();
	while(it != itEnd)
	{
		LINE* pPT = *it;
		m_pGrp->RenderLine(pPT->x0,pPT->y0,pPT->x1,pPT->y1);
		it++;
	}

// 	// 画鼠标
// 	m_objMouse.Render( &m_grp );

/*	m_pGrp->SwapBuffers();*/
	m_pGrp->EndScene();

	m_pFps->Show();

	Sleep(1);

	return 1;
}


// 游戏退出事件，释放游戏资源
void CGame::OnQuit(void)
{
	while(m_listLine.size() > 0)
	{
		delete m_listLine.back();
		m_listLine.pop_back();
	}

	delete m_pClient;
	delete m_pInput;
	delete m_pFps;
}
