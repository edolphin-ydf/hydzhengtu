#pragma once

#include "fsDllExport.h"
#include "fsGraphics.h"


/*
 *	组件工厂
 *	单一实例模式
 */
class FS_API fsGraphicsFactory
{
public:
	~fsGraphicsFactory(void);

	////创建一个Texture对象,需要用户自己删除
	static fsTexture* CreateTexture();

	//////创建一个字体对象,需要用户自己删除
	//static fsFont* CreateFont(FONT_TYPE fontType);

	////创建一个动画对象,需要用户自己删除
	//static FSAnimation* CreateAnimation();

	//创建一个渲染器实例
	static fsGraphics* CreateGraphics(GRAPHICS_TYPE graphType);

	//设置当前渲染器
	static void SetCurrGraphics(fsGraphics* pGraphics);

	//取得当前渲染器
	static fsGraphics* GetCurrGraphics();
private:
	fsGraphicsFactory(void);
};


/*

//==============全局函数==========

//初始化引擎
extern FS_API FSGraphics*	FS_InitEngine(HWND hWnd,  int iWidth, int iHeight,  BOOL bWindow, fsGraphicsFactory::GRAPHICS_TYPE aRenderType = fsGraphicsFactory::GRAPHICS_DX3D);

//释放引擎
extern FS_API void			FS_ReleaseEngine();

//创建图片
extern FS_API FSImage*		FS_CreateImage(char *picName, unsigned long colorKey);

//创建动画
extern FS_API FSAnimation*	FS_CreateAnimation(char *aniFile);

//创建字体
extern FS_API FSFont*		FS_CreateFont(char *fontName);
extern FS_API FSFont*		FS_CreateFont(LOGFONT &logFont);
*/

