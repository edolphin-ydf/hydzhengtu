#pragma once

#include "FSServer.h"
#include "FSClient.h"


/*
*	组件工厂
*	单一实例模式
*/
class FS_API FSSocketFactory
{
public:
	~FSSocketFactory(void);

	//创建一个图形对象,需要用户自己删除
	static FSServer* CreateServer();

	//创建一个字体对象,需要用户自己删除
	static FSClient* CreateClient();

private:
	FSSocketFactory(void);
};
