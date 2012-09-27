/********************************************************************
�������ں�ʱ��		2004/09/23	14:23
�������ں�ʱ��:		2004/11/005	09:38
�ļ���:			fps
�ļ���չ��:		cpp
����:			������

  ����:		����FPS�������������ִ��ʱ�䡢������Ƶ����Ϸ�ã�
  
	ע���:		�����Ǵ��ຯ��������ڴ��ڱ������ϣ�DOSƽ̨��ֻ�ܲ����ٶȲ�����ʾFPS��
*********************************************************************/


#include <windows.h>
#include <stdio.h>
#include <mmSystem.h>
#pragma comment(lib, "Winmm.lib")
#include "fps.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFps::CFps(HWND hwnd)
{
	dLockFirst = 0.0;
	m_hwnd = NULL;
	//��ʼ��
	m_hwnd = hwnd;
	dFPs = 0.0;

	sprintf(strFPS, "0");
	
	//���洰�ڱ�����ԭ��������
	char strTmp[100];
	GetWindowText(hwnd, strTmp, 200); 
	sprintf(strOldTitle, strTmp);
}

CFps::CFps()
{
	//��ʼ��
	m_hwnd = NULL;
	dLockFirst = 0.0;
	dFPs = 0.0;
	sprintf(strFPS, "FPS:0");
}

CFps::~CFps()
{
	m_hwnd = NULL;
}

//��ʾFPS��չ��
void CFps::Update()
{
	static float fLastTime = 0.f;	// Absolute time at last frame
	static float fFramesPerSecond = 0.f;
	float        fTime;
	
	// Get the current tick count, and multiply it by 0.001 to convert it from 
	// milliseconds to seconds
    fTime = GetTickCount() * 0.001f;				
	
	// Increase the frame counter
	++ fFramesPerSecond;
	
	if( (fTime - fLastTime) > 1.f )
	{
		// Here, we set the lastTime to the currentTime.
		fLastTime = fTime;
		
		// Current FPS
		dFPs = fFramesPerSecond;
		
		//ʵ��FPS��SPDһ����ʾ�ķ���
		sprintf(strFPS, "FPS:%0.2f", dFPs);
		
		// Reset the frames per second
		fFramesPerSecond = 0.f;
	}
}

void CFps::Show()
{
	char ch[250];
	sprintf(ch, "%s  %s", strOldTitle,strFPS);
	printf("%s\n", ch);

	//��ʾ�ڴ��ڱ�������
	SetWindowText(m_hwnd, ch);
}

//��ʼ��Ƶ
void CFps::Lock(int nFrequency)
{
	//�����˷�Χ���򲻴���
	if (nFrequency <= 0 || nFrequency > 1000)
	{
		return;
	}

	double dTime=0;		//ʱ���	
	while( (dTime = (double)timeGetTime() - dLockFirst) < 1000/(double)nFrequency );

	dLockFirst = (double)timeGetTime();

	return;
}
