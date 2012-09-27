#pragma once
#include <Windows.h>
#include "FS_Engine.h"

#include <list>
using namespace std;

class CGameWorld
{
public:
	CGameWorld(void);
	virtual ~CGameWorld(void);

	// 游戏初始化，返回0退出游戏
	BOOL Init(int nMaxClient, short nPort);
	
	// 处理游戏逻辑，返回0退出游戏
	// fTick: 帧间隔时间
	int OnTick(float fTick);

	// 游戏退出事件，释放游戏资源
	void OnQuit(void);



// 	// 根据UNIN查找对象
// 	CMoveable* FindMoveable(unsigned int uUIN);
// 
// 	// 根据坐标查找对象
// 	CMoveable* FindMonster(int iX, int iY, int iDis);

public:
	FSServer *m_pServer;				// 网络
	//list<CMoveable*> m_listObjServer;	// 玩家list
	unsigned int m_listUin[1024];		// 存储唯一ID号
};

