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

	// ��Ϸ��ʼ��������0�˳���Ϸ
	BOOL Init(HWND hWnd);
	
	// ������Ϸ�߼�������0�˳���Ϸ
	// fTick: ֡���ʱ��
	int OnTick(float fTick);

	DWORD ReceiveNetPack();

	// ��Ⱦ��Ϸ���棬����0�˳���Ϸ
	int OnRender(void);

	// ��Ϸ�˳��¼����ͷ���Ϸ��Դ
	void OnQuit(void);

	void AddLine(int iX1, int iY1, int iX2, int iY2);

	void SendAddLine(int iX1, int iY1, int iX2, int iY2);

	FSClient* GetClientNet() {return m_pClient;}
	


private:
	HWND m_hWnd;	// ���洰�ھ��
	CFps *m_pFps;

	FSClient *m_pClient;	//����

	CDDInput *m_pInput;		// �����豸
	fsGraphics* m_pGrp;		// ͼ�ο�

	list<LINE*> m_listLine;	// ���б�

	//CTexture m_objMouse;	// ���
};

