/*
 �ļ��� : Common.h
 ����ʱ�� : 2012/9/18
 ���� : hyd
 ���� : 
*/
#ifndef __Common_H__
#define __Common_H__

#include <winsock2.h>
//#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"ws2_32.lib")
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include <windows.h>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <queue>

#include "staticFun.h"

typedef enum
{
	UNKNOWNSERVER  =  0, /** δ֪���������� */
	SUPERSERVER      =  1, /** ��������� */
	LOGINSERVER     =  10, /** ��½������ */
	RECORDSERVER  =  11, /** ���������� */
	BILLSERVER      =  12, /** �Ʒѷ����� */
	SESSIONSERVER  =  20, /** �Ự������ */
	SCENESSERVER  =  21, /** ���������� */
	GATEWAYSERVER  =  22, /** ���ط����� */
	TESTSERVER = 23,   //���Է�����
}ServerType;

//һЩ�궨��
#define SAFE_DELETE(x) { if (NULL != x) { delete (x); (x) = NULL; } }
#define SAFE_DELETE_VEC(x) { if (NULL != x) { delete [] (x); (x) = NULL; } }

// [ranqd] ��ͷ��ʽ����
struct PACK_HEAD
{
	unsigned char Header[2];
	unsigned short Len;
	PACK_HEAD()
	{
		Header[0] = 0xAA;
		Header[1] = 0xDD;
	}
};

#define PACKHEADLASTSIZE (sizeof(PACK_HEAD))

#define PACKHEADSIZE    sizeof(PACK_HEAD)


namespace Cmd{
	/// ��ָ��
	const BYTE NULL_USERCMD      = 0;
	//  �յ�ָ�����
	const BYTE NULL_PARA         = 0;
	/// ��½ָ��
	const BYTE LOGON_USERCMD     = 1;

	
   // �ղ���ָ������źźͶ�ʱ��ָ��
   struct Cmd_NULL
   {
     BYTE cmd;          /**< ָ����� */
     BYTE para;         /**< ָ������ӱ�� */

     //brief ���캯��
     Cmd_NULL(const BYTE cmd = NULL_USERCMD,const BYTE para = NULL_PARA) : cmd(cmd),para(para) {};
   };

   namespace Test
   {
	   const BYTE CMD_LOGIN     = 1;
	   const BYTE CMD_TestServer= 2;
	   
	   //////////////////////////////////////////////////////////////
	   /// ��½����������ָ��
	   //////////////////////////////////////////////////////////////
	   const BYTE PARA_LOGIN = 1;
	   const BYTE PARA_CHK   = 2;
	   struct t_LoginTest : Cmd_NULL
	   {
		   WORD wdServerID;    //��ǰ������ID
		   WORD wdServerType;  //��ǰ����������
		   t_LoginTest()
			   : Cmd_NULL(CMD_LOGIN,PARA_LOGIN) {};
	   };
   }
}

/**
�����̳߳ص�״̬���λ
 */
enum{
  state_none    =  0,          /**< �յ�״̬ */
  state_maintain  =  1         /**< ά���У���ʱ���������µ����� */
};

#endif
