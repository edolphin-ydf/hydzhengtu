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

	// ��Ϸ��ʼ��������0�˳���Ϸ
	BOOL Init(int nMaxClient, short nPort);
	
	// ������Ϸ�߼�������0�˳���Ϸ
	// fTick: ֡���ʱ��
	int OnTick(float fTick);

	// ��Ϸ�˳��¼����ͷ���Ϸ��Դ
	void OnQuit(void);



// 	// ����UNIN���Ҷ���
// 	CMoveable* FindMoveable(unsigned int uUIN);
// 
// 	// ����������Ҷ���
// 	CMoveable* FindMonster(int iX, int iY, int iDis);

public:
	FSServer *m_pServer;				// ����
	//list<CMoveable*> m_listObjServer;	// ���list
	unsigned int m_listUin[1024];		// �洢ΨһID��
};

