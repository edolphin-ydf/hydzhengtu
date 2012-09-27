#pragma once

#include "FSServer.h"
#include "FSClient.h"


/*
*	�������
*	��һʵ��ģʽ
*/
class FS_API FSSocketFactory
{
public:
	~FSSocketFactory(void);

	//����һ��ͼ�ζ���,��Ҫ�û��Լ�ɾ��
	static FSServer* CreateServer();

	//����һ���������,��Ҫ�û��Լ�ɾ��
	static FSClient* CreateClient();

private:
	FSSocketFactory(void);
};
