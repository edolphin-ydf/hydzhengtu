#pragma once

#include "fsDllExport.h"
#include "fsGraphics.h"


/*
 *	�������
 *	��һʵ��ģʽ
 */
class FS_API fsGraphicsFactory
{
public:
	~fsGraphicsFactory(void);

	////����һ��Texture����,��Ҫ�û��Լ�ɾ��
	static fsTexture* CreateTexture();

	//////����һ���������,��Ҫ�û��Լ�ɾ��
	//static fsFont* CreateFont(FONT_TYPE fontType);

	////����һ����������,��Ҫ�û��Լ�ɾ��
	//static FSAnimation* CreateAnimation();

	//����һ����Ⱦ��ʵ��
	static fsGraphics* CreateGraphics(GRAPHICS_TYPE graphType);

	//���õ�ǰ��Ⱦ��
	static void SetCurrGraphics(fsGraphics* pGraphics);

	//ȡ�õ�ǰ��Ⱦ��
	static fsGraphics* GetCurrGraphics();
private:
	fsGraphicsFactory(void);
};


/*

//==============ȫ�ֺ���==========

//��ʼ������
extern FS_API FSGraphics*	FS_InitEngine(HWND hWnd,  int iWidth, int iHeight,  BOOL bWindow, fsGraphicsFactory::GRAPHICS_TYPE aRenderType = fsGraphicsFactory::GRAPHICS_DX3D);

//�ͷ�����
extern FS_API void			FS_ReleaseEngine();

//����ͼƬ
extern FS_API FSImage*		FS_CreateImage(char *picName, unsigned long colorKey);

//��������
extern FS_API FSAnimation*	FS_CreateAnimation(char *aniFile);

//��������
extern FS_API FSFont*		FS_CreateFont(char *fontName);
extern FS_API FSFont*		FS_CreateFont(LOGFONT &logFont);
*/

