#pragma once


#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
#include <windows.h>


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the fsGraphics_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FS_API functions as being imported from a DLL, whereas this DLL sees symbols

// defined with this macro as being exported.
#ifdef FS_EXPORTS
#define FS_API __declspec(dllexport)
#else
#define FS_API __declspec(dllimport)
#endif

