/*
 文件名 : Common.h
 创建时间 : 2012/9/18
 作者 : hyd
 功能 : 
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
	UNKNOWNSERVER  =  0, /** 未知服务器类型 */
	SUPERSERVER      =  1, /** 管理服务器 */
	LOGINSERVER     =  10, /** 登陆服务器 */
	RECORDSERVER  =  11, /** 档案服务器 */
	BILLSERVER      =  12, /** 计费服务器 */
	SESSIONSERVER  =  20, /** 会话服务器 */
	SCENESSERVER  =  21, /** 场景服务器 */
	GATEWAYSERVER  =  22, /** 网关服务器 */
	TESTSERVER = 23,   //测试服务器
}ServerType;

//一些宏定义
#define SAFE_DELETE(x) { if (NULL != x) { delete (x); (x) = NULL; } }
#define SAFE_DELETE_VEC(x) { if (NULL != x) { delete [] (x); (x) = NULL; } }

// [ranqd] 包头格式定义
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
	/// 空指令
	const BYTE NULL_USERCMD      = 0;
	//  空的指令参数
	const BYTE NULL_PARA         = 0;
	/// 登陆指令
	const BYTE LOGON_USERCMD     = 1;

	
   // 空操作指令，测试信号和对时间指令
   struct Cmd_NULL
   {
     BYTE cmd;          /**< 指令代码 */
     BYTE para;         /**< 指令代码子编号 */

     //brief 构造函数
     Cmd_NULL(const BYTE cmd = NULL_USERCMD,const BYTE para = NULL_PARA) : cmd(cmd),para(para) {};
   };

   namespace Test
   {
	   const BYTE CMD_LOGIN     = 1;
	   const BYTE CMD_TestServer= 2;
	   
	   //////////////////////////////////////////////////////////////
	   /// 登陆档案服务器指令
	   //////////////////////////////////////////////////////////////
	   const BYTE PARA_LOGIN = 1;
	   const BYTE PARA_CHK   = 2;
	   struct t_LoginTest : Cmd_NULL
	   {
		   WORD wdServerID;    //当前服务器ID
		   WORD wdServerType;  //当前服务器类型
		   t_LoginTest()
			   : Cmd_NULL(CMD_LOGIN,PARA_LOGIN) {};
	   };
   }
}

/**
连接线程池的状态标记位
 */
enum{
  state_none    =  0,          /**< 空的状态 */
  state_maintain  =  1         /**< 维护中，暂时不允许建立新的连接 */
};

#endif
