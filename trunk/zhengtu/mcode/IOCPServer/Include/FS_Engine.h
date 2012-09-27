#pragma once
#include <assert.h>

//动态库头文件
#include "FS_DLL.h"

//网络
#include "FSSocketFactory.h"

#pragma comment(lib, "FS_Socket.lib")


//图形
#include "fsGraphicsFactory.h"
//#ifdef _DEBUG
//	#pragma comment(lib,"fsGraphics_d.lib")
//#else
	#pragma comment(lib,"fsGraphics.lib")
//#endif
