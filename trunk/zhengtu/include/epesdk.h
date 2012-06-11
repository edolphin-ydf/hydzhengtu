#ifndef _INC_EPESDK_H_
#define _INC_EPESDK_H_

#include <shlwapi.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

typedef struct
{
  int Bz;
  int ValueSize;
  int ValueBuf;
  int PassSize;
  int PassBuf;
}NUserRecord,*PUserRecord;

// 函数原型

// 在调用 SDK 之前需取得与壳通信的窗口句柄，
// 一般在在主程序的初始化完成后调用此函数
HWND EPE_GetRegisterHandle(HINSTANCE hInstance);

// 取得共15条注册相关信息，按INDEX值：
#define EPEGRI_ALL              -1 //以下所有信息
#define EPEGRI_IS_REGISTERED    0 //是否注册(Y/N)
#define EPEGRI_MACHINE_FINGER   1 //机器信息
#define EPEGRI_USERNAME         2 //注册用户
#define EPEGRI_REGINFO          3 //注册信息
#define EPEGRI_TITLE            4 //软件名称（注册窗口标题）
#define EPEGRI_RUN_MINUTES      5 //本次已运行多少分钟
#define EPEGRI_REG_PER_MINUTES  6 //每隔多少分钟要求注册
#define EPEGRI_PER_MINUTES      7 //每次限制运行多少分钟
#define EPEGRI_FIRST_RUN        8 //第一次在本机使用软件的日期格式YYYYMMDD
#define EPEGRI_LAST_RUN         9 //限制试用日期，格式YYYYMMDD
#define EPEGRI_MAX_DAYS         10 //限制试用天数
#define EPEGRI_RUN_TIMES        11 //已运行次数
#define EPEGRI_MAX_TIMES        12 //限制试用次数
#define EPEGRI_PROTECT_INT      13 //加壳时设置的特征整数值
#define EPEGRI_PROTECT_PASS     14 //为保护密码的EPE_Hash值，可依此二者判断是否破解版
//函数返回指定项
DWORD EPE_GetRegisterInfo(UINT Index,PSTR szBuffer,DWORD dwBuffer);

// 取得注册用户名称，未注册则为空，EPE_GetRegisterInfo(EPEGRI_USERNAME)可能未注册也有值
DWORD EPE_GetRegisterUser(PSTR szBuffer,DWORD dwBuffer);

// 显示注册窗口
BOOL EPE_ShowRegisterDlg(void);

// 在EncryptPE加密程序运行过程中不允许ProcessName指定的进程（不含.EXE）运行，
// 多个进程则多次调用该函数，或者用逗号隔开多个进程名
BOOL EPE_KillProcess(PSTR ProcessName);

// 取消截杀，见 KillProcess
BOOL EPE_NotKillProcess(PSTR ProcessName);

// 设置加壳进程中不允许加载的模块名，可以以逗号隔开多个模块名
BOOL EPE_KillDll(PSTR DLLName);

// 获取指字字串的EPE_Hash值
PSTR EPE_Hash(PSTR SourceString);

//让壳去执行被加壳程序的某个函数，这个函数必须能独立执行，且无参数无返回值。
//Address是函数真实地址与保护密码的HASH值前八位（不足八位则前补0）转换过来的数以及特征整数两次异或过的
BOOL EPE_RunFunction(PVOID Address);

//以下函数要求个人开发版以上

//获取加壳时设置的数据文件的内容，可以指定从第几个字节开始读取多长内容
PVOID EPE_GetSavedData(WORD From,WORD Len);

// 写入注册用户名、注册信息，与 EPE_GetRegisterInfo 配合可用于设计个性注册窗口
BOOL EPE_SetRegisterInfo(PSTR User,PSTR Info);

// 改变注册窗口的界面语言元素，PLanguage 指向内存（不含逗号和省略号）：
// 一个字节字符集，一个字节字体大小，字体名称＃0，字符串4＃0，字符串5＃0，......，字符串17＃0
// 如一个字符串：#134#9'宋体'#0'警告'#0'出现未知错误'＃0......'取消(&C)'#0
// 方便用户编写多语言程序
BOOL EPE_SetLanguage(PSTR PLanguage);

// 改变注册窗口的提示、网站主页、邮件地址等信息，PHint指向内存（不含逗号）:
// 过期后输入框颜色转换成的字符串＃0，提示信息＃0，主页＃0，邮箱地址＃0
// 如一个字符串：'$0000FF'#0'请注册本软件'#0'http://www.server.com'#0'mailto:someone@server.com'#0}
BOOL EPE_SetRegisterHint(PSTR PHint);

// 按Index取值0至9分别对应操作如下：
// + - * div mod and or xor shl shr
// 返回值为某一操作的结果值,+可用于个人开发版
int EPE_CaclResult(int Value1,int Value2,char Index);

//以下函数要求企业专业版以上

// 返回范围在 >=0 且 < Value 的随机整数
int EPE_Random(int Value);

// 按Index取值0至9分别返回：
// 系统版本VER_PLATFORM_WIN32S(0) VER_PLATFORM_WIN32_WINDOWS(1) VER_PLATFORM_WIN32_NT(2),
// GetTickCount,GetCurrentProcess,GetCurrentProcessID,GetCurrentThread,GetCurrentThreadID,
// GetActiveWindow,GetFocus,GetForegroundWindow,GetDesktopWindow
WORD EPE_CustomValue(int Index);

//以下函数要求企业开发版以上

// 按Index取值0至9对窗口句柄aHwnd执行相关WINDOW函数判断结果：
// IsWindow,IsWindowVisible,IsIconic,IsZoomed,ShowWindow,HideWindow,
// EnableWindow,DisableWindow,IsWindowEnabled,CloseWindow,DestroyWindow
BOOL EPE_WindowFunction(int Index,HWND aHwnd);

// 申请Size大小内存空间
PVOID EPE_malloc(int Size);

// 释放内存
BOOL EPE_free(PVOID Buf);

// 清空一段内存
BOOL EPE_ZeroMemory(PVOID Buf,int Size);

// 以Fill字节值填充一段内存
BOOL EPE_memset(PVOID Buf,int Size,char Fill);

// 内存复制
BOOL EPE_memcpy(PVOID Destination,PVOID Source,int Size);

// 内存移动
BOOL EPE_memmove(PVOID Destination,PVOID Source,int Size);

// 按Index取值0至5分别获得：
// GetCurrentDirectory,GetWindowsDirectory,GetSystemDirectory,
// GetTempPath,GetComputerName,GetUserName
PSTR EPE_GetSystemString(int Index);

// 内存数据压缩
void EPE_Compress(PVOID InBuf,int InBytes,PVOID OutBuf,PINT OutBytes,PSTR Password);

// 压缩后的数据解压缩
void EPE_Decompress(PVOID InBuf,int InBytes,PVOID OutBuf,PINT OutBytes,PSTR Password,int OutEstimate);

// 字符串压缩生成新的字符串
PSTR EPE_StringCompress(PSTR SourceString,PSTR Password,BOOL HFlag);

// 将压缩生成的字符串解压缩还原成原字符串
PSTR EPE_StringDecompress(PSTR SourceString,PSTR Password,BOOL HFlag);

// 对一段内存数据进行加密
void EPE_Encrypt(int InBuf,int InBytes,int OutBuf,PINT OutBytes,PSTR Password);

// 对已加密数据进行解密
void EPE_Decrypt(int InBuf,int InBytes,int OutBuf,PINT OutBytes,PSTR Password);

// 对字符串进行加密生成新的字符串
PSTR EPE_StringEncrypt(PSTR SourceString,PSTR Password,BOOL HFlag);

// 将加密生成的字符串解密还原成原字符串
PSTR EPE_StringDecrypt(PSTR SourceString,PSTR Password,BOOL HFlag);

// 内部使用函数
BOOL EPE_IsLibrary(HINSTANCE hInstance);

//以下函数要求zebra专用版

typedef DWORD RC5_32_INT;

#define RC5_8_ROUNDS  8
#define RC5_12_ROUNDS  12
#define RC5_16_ROUNDS  16

typedef struct
{
  int        rounds;
  RC5_32_INT data[2*(RC5_16_ROUNDS+1)];
}EPE_RC5_32_KEY;

void EPE_RC5_32_set_key(EPE_RC5_32_KEY *key,int len,const BYTE *data,int rounds);
void EPE_RC5_32_encrypt(RC5_32_INT *d,EPE_RC5_32_KEY *key);
void EPE_RC5_32_decrypt(RC5_32_INT *d,EPE_RC5_32_KEY *key);
void EPE_RC5_32(BYTE *data,int len,char * password,BYTE flag);

//指定当前程序所使用的网络通信内部端口
void EPE_SetHookPort(short HookPort);
  
//壳是否对当前程序网络通信内部端口发送的数据进行加密(使用RC5算法，密码是注册信息)，要加密则HookSend为1，否则为0
void EPE_SetHookSend(BYTE bHook);
  
//壳是否对当前程序网络通信内部端口接收的数据进行解密(使用RC5算法，密码是注册信息)，要解密则HookRecv为1，否则为0
void EPE_SetHookRecv(BYTE HookRecv);

//设置网络验证的网页地址
void EPE_SetCheckUrl(PSTR szUrl);

#define CHECKSUM_CURRENT        0x0098C34E

//获得壳代码的校验值，该版本的校验值是0x0098C34E，如果内存被修改，该函数可能返回错误的值
DWORD EPE_GetCheckSum(void);

//将Source指向的Len长度内存数据写入Destination处
void EPE_WriteMemory(BYTE *Destination,BYTE *Source,int Len);

#ifdef _EPESDK_IMP_

static HWND g_hwndEPESDK = NULL;

HWND EPE_GetRegisterHandle(HINSTANCE hInstance)
{
static BOOL g_bInit = FALSE;

  char   szPath[_MAX_PATH];
  char   szName[_MAX_PATH];
  int    i;
  PVOID  pData;
  DWORD  ccid;
  HANDLE hMap;
  
  if (!g_bInit){
    g_bInit = TRUE;
    GetModuleFileNameA(hInstance,szPath,sizeof(szPath));	
    for(i=0;i<0 != szPath[i];i++)
    {
      if (szPath[i]==92) szPath[i]=47;
    }
    ccid = GetCurrentProcessId();
    _snprintf_s(szName,sizeof(szName),"%s/%08X",szPath,ccid);
    if (INVALID_HANDLE_VALUE != (hMap=OpenFileMappingA(FILE_MAP_WRITE,FALSE,szName)))
    {
      if (NULL != (pData=MapViewOfFile(hMap,FILE_MAP_WRITE,0,0,0))){
	    g_hwndEPESDK = *(HWND*)pData;
	    UnmapViewOfFile(pData);
      }
      CloseHandle(hMap);
    }
  }
  return g_hwndEPESDK;
}

DWORD EPE_GetRegisterInfo(UINT Index,PSTR szBuffer,DWORD dwBuffer)
{
  int     i,index2,pos;
  PSTR    szResult;
  LRESULT lResult;

  szBuffer[0] = 0;
  if (EPEGRI_IS_REGISTERED == Index) 
  {
	strcpy_s(szBuffer,"N");
  }
  if (!IsWindow(g_hwndEPESDK)) return strlen(szBuffer);
  lResult = SendMessage(g_hwndEPESDK,WM_USER,0,1);
  if (-1 != lResult && 0 != lResult)
  {
    if (EPEGRI_ALL == Index)
    {
      strcpy_s(szBuffer,(PSTR)lResult);
    }
    else
    {
	  szResult = (PSTR)lResult;
	  for(i=pos=index2=0;0 != szResult[0];i++)
	  {
	    if (szResult[i]==13 && szResult[i+1]==10)
		{
		  if (index2++ == Index)
		  {
			memcpy(szBuffer,&szResult[pos],i-pos);
			szBuffer[i-pos] = 0;
			break;
		  }
		  else
		  {
			pos = i + 2;
		  }
		}
	  }
	}
  }
  return strlen(szBuffer);
}

DWORD EPE_GetRegisterUser(PSTR szBuffer,DWORD dwBuffer)
{
  return EPE_GetRegisterInfo(EPEGRI_USERNAME,szBuffer,dwBuffer);
}

BOOL EPE_ShowRegisterDlg(void)
{
  if (!IsWindow(g_hwndEPESDK)) return FALSE;	
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,0,0);
}

BOOL EPE_KillProcess(PSTR ProcessName)
{
  if (0 == ProcessName[0] || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,88,(LPARAM)ProcessName);
}

BOOL EPE_NotKillProcess(PSTR ProcessName)
{
  if (0 == ProcessName[0] || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,89,(LPARAM)ProcessName);
}

BOOL EPE_KillDll(PSTR DLLName)
{
  if (0 == DLLName[0] || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,90,(LPARAM)DLLName);
}

PSTR EPE_Hash(PSTR SourceString)
{
  LRESULT lResult;

  if (0 == SourceString[0] || !IsWindow(g_hwndEPESDK)) return NULL;
  lResult = SendMessage(g_hwndEPESDK,WM_USER,99,(LPARAM)SourceString);
  if (-1 != lResult && 0 != lResult) return (PSTR)lResult;
  return NULL;
}

BOOL EPE_RunFunction(PVOID Address)
{
  if (NULL == Address || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,66,(LPARAM)Address);
}

PVOID EPE_GetSavedData(WORD From,WORD Len)
{
  LRESULT lResult;

  if (!IsWindow(g_hwndEPESDK)) return NULL;
  lResult = SendMessage(g_hwndEPESDK,WM_USER,77,(LPARAM)(((DWORD)From << 16) + Len));
  if (-1 != lResult && 0 != lResult) return (PVOID)lResult;
  return NULL;
}

BOOL EPE_SetLanguage(PSTR PLanguage)
{
  if (NULL == PLanguage || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,1000,(LPARAM)PLanguage);
}

BOOL EPE_SetRegisterHint(PSTR PHint)
{
  if (NULL == PHint || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,10000,(LPARAM)PHint);
}

int EPE_CaclResult(int Value1,int Value2,char Index)
{
  NUserRecord nUR;

  if (!IsWindow(g_hwndEPESDK)) return 0;
  memset(&nUR,0,sizeof(nUR));
  nUR.Bz        = Index;
  nUR.ValueSize = Value1;
  nUR.PassSize  = Value2;
  if (1 == SendMessage(g_hwndEPESDK,WM_USER,5,(LPARAM)&nUR)) return nUR.ValueSize;
  return 0;
}

int EPE_Random(int Value)
{
  if (!IsWindow(g_hwndEPESDK)) return 0;
  return SendMessage(g_hwndEPESDK,WM_USER,15,Value);
}

WORD EPE_CustomValue(int Index)
{
  if (Index < 0 || Index > 9 || ! IsWindow(g_hwndEPESDK)) return 0;
  return (WORD)SendMessage(g_hwndEPESDK,WM_USER,25,Index);
}

BOOL EPE_WindowFunction(int Index,HWND aHwnd)
{
  if (Index < 0 || Index > 10 || !IsWindow(aHwnd) || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,Index * 10 + 35,(LPARAM)aHwnd);
}

PVOID EPE_malloc(int Size)
{
  LRESULT lResult;
	
  if (Size <= 0 || !IsWindow(g_hwndEPESDK)) return NULL;
  lResult = SendMessage(g_hwndEPESDK,WM_USER,145,Size);
  if (-1 != lResult && 0 != lResult) return (PVOID)lResult;
  return NULL;
}

BOOL EPE_free(PVOID Buf)
{
  if (NULL == Buf || !IsWindow(g_hwndEPESDK)) return FALSE;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,155,(LPARAM)Buf);
}

BOOL EPE_ZeroMemory(PVOID Buf,int Size)
{
  NUserRecord nUR;
	
  if (NULL == Buf || !IsWindow(g_hwndEPESDK)) return FALSE;
  memset(&nUR,0,sizeof(nUR));
  nUR.ValueBuf  = (int)Buf;
  nUR.ValueSize = Size;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,165,(LPARAM)&nUR);
}

BOOL EPE_memset(PVOID Buf,int Size,char Fill)
{
  NUserRecord nUR;
	
  if (NULL == Buf || Size <= 0 || !IsWindow(g_hwndEPESDK)) return FALSE;
  memset(&nUR,0,sizeof(nUR));
  nUR.Bz        = 1;
  nUR.ValueBuf  = (int)Buf;
  nUR.ValueSize = Size;
  nUR.PassSize  = Fill;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,165,(LPARAM)&nUR);
}

BOOL EPE_memcpy(PVOID Destination,PVOID Source,int Size)
{
  NUserRecord nUR;
	
  if (NULL == Destination || NULL == Source || Size <= 0 || !IsWindow(g_hwndEPESDK)) return FALSE;
  memset(&nUR,0,sizeof(nUR));
  nUR.Bz        = 2;
  nUR.PassBuf   = (int)Destination;
  nUR.ValueBuf  = (int)Source;
  nUR.ValueSize = Size;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,165,(LPARAM)&nUR);
}

BOOL EPE_memmove(PVOID Destination,PVOID Source,int Size)
{
  NUserRecord nUR;
	
  if (NULL == Destination || NULL == Source || Size <= 0 || !IsWindow(g_hwndEPESDK)) return FALSE;
  memset(&nUR,0,sizeof(nUR));
  nUR.Bz        = 3;
  nUR.PassBuf   = (int)Destination;
  nUR.ValueBuf  = (int)Source;
  nUR.ValueSize = Size;
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,165,(LPARAM)&nUR);
}

PSTR EPE_GetSystemString(int Index)
{
  LRESULT lResult;
	
  if (Index < 0 || Index > 5 || ! IsWindow(g_hwndEPESDK)) return NULL;
  lResult = SendMessage(g_hwndEPESDK,WM_USER,175,Index);
  if (-1 != lResult && 0 != lResult) return (PSTR)lResult;
  return NULL;
}

BOOL EPE_SetRegisterInfo(PSTR User,PSTR Info)
{
  NUserRecord nUR;

  if (!IsWindow(g_hwndEPESDK)) return FALSE;
  memset(&nUR,0,sizeof(nUR));
  nUR.Bz        = 0xff;
  nUR.ValueBuf  = (int)User;
  nUR.ValueSize = strlen(User);

  nUR.PassBuf  = (int)Info;
  nUR.PassSize = strlen(Info);
	
  return 1 == SendMessage(g_hwndEPESDK,WM_USER,100,(LPARAM)&nUR);
}

void EPE_Compress(PVOID InBuf,int InBytes,PVOID OutBuf,PINT OutBytes,PSTR Password)
{
  NUserRecord nUR;

  if (NULL == InBuf || NULL == OutBuf || InBytes <= 0 || NULL == Password || !IsWindow(g_hwndEPESDK)) return;
  memset(&nUR,0,sizeof(nUR));
  nUR.ValueBuf  = (int)InBuf;
  nUR.ValueSize = InBytes;
  nUR.PassBuf   = (int)Password;
  nUR.PassSize  = strlen(Password);
  if (1 == SendMessage(g_hwndEPESDK,WM_USER,0x7FFFFFFF,(LPARAM)&nUR))
  {
    memcpy(OutBuf,(PVOID)nUR.ValueBuf,nUR.ValueSize);
    *OutBytes = nUR.ValueSize;
  }
}

void EPE_Decompress(PVOID InBuf,int InBytes,PVOID OutBuf,PINT OutBytes,PSTR Password,int OutEstimate)
{
  NUserRecord nUR;

  if (NULL == InBuf || NULL == OutBuf || InBytes <= 0 || NULL == Password || !IsWindow(g_hwndEPESDK)) return;
  memset(&nUR,0,sizeof(nUR));
  nUR.Bz = 1;
  nUR.ValueBuf  = (int)InBuf;
  nUR.ValueSize = InBytes;
  nUR.PassBuf   = (int)Password;
  nUR.PassSize  = strlen(Password);
  if (1 == SendMessage(g_hwndEPESDK,WM_USER,0x7FFFFFFF,(LPARAM)&nUR))
  {
	memcpy(OutBuf,(PVOID)nUR.ValueBuf,nUR.ValueSize);
	*OutBytes = nUR.ValueSize;
  }
}

/*
PSTR EPE_StringCompress(PSTR SourceString,PSTR Password,BOOL HFlag)
{
  int         i;
  NUserRecord nUR;	
	
  if (NULL == SourceString || ! IsWindow(g_hwndEPESDK)) return NULL;
  nUR.Bz        = HFlag ? 10 : 20;
  nUR.ValueBuf  = SourceString;
  nUR.ValueSize = strlen(SourceString);
  nUR.PassBuf   = (int)Password;
  nUR.PassSize  = strlen(Password);

  for(i=0;i<100;i++)
  {
	if (1 == SendMessage(g_hwndEPESDK,WM_USER,0x7FFFFFFF,(LPARAM)&nUR))
    {
	  try
	  {
	    memcpy(OutBuf,(PVOID)nUR.ValueBuf,nUR.ValueSize);
		*OutBytes = nUR.ValueSize;
	  }
	  catch(...)
	  {
	  }
      break;
    }
  }

  for(i=0;i<100;i++)
  {
		lResult = SendMessage(g_hwndEPESDK,WM_USER,2147483647,(long)P);
		if (lResult == 1)
		{
			try
			{
				//SetLength(Result,nUR.ValueSize);
				memcpy(&Result,&nUR.ValueBuf,nUR.ValueSize);
			}
			catch(...)
			{
				return "";
			}
		}
		return Result;
	}
	return "";
}

PSTR EPE_StringDecompress(PSTR SourceString,PSTR Password,BOOL HFlag= true)
{
	
	int s=sizeof(P);
	memset(P,0,s);
	int i,lResult;
	
	if (SourceString[0] = 0 || ! IsWindow(g_hwndEPESDK)) return "";

	if (HFlag) nUR.Bz = 11;
	else nUR.Bz = 21;
	nUR.ValueSize = strlen(SourceString);
	nUR.ValueBuf = (int)SourceString;
	nUR.PassSize = strlen(Password);

	if (Password!=NULL) nUR.PassBuf = (int)Password;
	else nUR.PassBuf = NULL;

	for(i=0;i<100;i++)
	{
		lResult = SendMessage(g_hwndEPESDK,WM_USER,2147483647,(long)P);
		if (lResult == 1)
		{
			try
			{
				memcpy((void *)Result,&nUR.ValueBuf,nUR.ValueSize);
			}
			catch(...)
			{
				return "";
			}
		}
		return Result;
	}
	return "";
}

void EPE_Encrypt(int InBuf,int InBytes,int OutBuf,PINT OutBytes,PSTR Password)
{
	
	int s=sizeof(P);
	memset(P,0,s);
	int i,lResult;

//	if ( InBuf==NULL || InBytes <= 0 || ! IsWindow(g_hwndEPESDK)) return;
	nUR.Bz = 100;
	nUR.ValueSize = InBytes;
	nUR.ValueBuf = (int)InBuf;
	nUR.PassSize = strlen(Password);

	if (Password!=NULL) nUR.PassBuf = (int)Password;
	else nUR.PassBuf = NULL;

	for(i=0;i<100;i++)
	{
		lResult = SendMessage(g_hwndEPESDK,WM_USER,(WPARAM)2147483647,(long)P);
		if (lResult == 1)
		{
			try
			{
				//OutBuf = (int)malloc(nUR.ValueSize);
				memcpy((void *)OutBuf,(void *)nUR.ValueBuf,nUR.ValueSize);
				*OutBytes = nUR.ValueSize;
			}
			catch(...)
			{
				//			
			}
		}
		break;
	}
}

void EPE_Decrypt(int InBuf,int InBytes,int OutBuf,PINT OutBytes,PSTR Password)
{
	
	int s=sizeof(P);
	memset(P,0,s);
	int i,lResult;
	
	if ( InBuf==NULL || InBytes <= 0 || ! IsWindow(g_hwndEPESDK)) return;

	nUR.Bz = 101;
	nUR.ValueSize = InBytes;
	nUR.ValueBuf = (int)InBuf;
	nUR.PassSize = strlen(Password);
	
	if (Password!=NULL) nUR.PassBuf = (int)Password;
	else nUR.PassBuf = NULL;

	for(i=0;i<100;i++)
	{
		lResult = SendMessage(g_hwndEPESDK,WM_USER,(WPARAM)2147483647,(long)P);
		
		if (lResult == 1)
		{
			try
			{
				//OutBuf = (int)malloc(nUR.ValueSize);
				memcpy((void *)OutBuf,(void *)nUR.ValueBuf,nUR.ValueSize);
				*OutBytes = nUR.ValueSize;
			}
			catch(...)
			{
				//
			}
		}
		break;
	}
}

PSTR EPE_StringEncrypt(PSTR SourceString,PSTR Password,BOOL HFlag = true)
{
	
	int s=sizeof(P);
	memset(P,0,s);
	int i,lResult;
	
	if (SourceString[0] == 0 || ! IsWindow(g_hwndEPESDK)) return "";

	if (HFlag) nUR.Bz = 110;
	else nUR.Bz = 120;

	nUR.ValueSize = strlen(SourceString);
	nUR.ValueBuf = (int)SourceString;
	nUR.PassSize = strlen(Password);

	if (Password!=NULL) nUR.PassBuf = (int)Password;
	else nUR.PassBuf = NULL;

	for(i=0;i<100;i++)
	{
		lResult = SendMessage(g_hwndEPESDK,WM_USER,0x7FFFFFFF,(long)P);
		if (lResult == 1)
		{
			try
			{
				memcpy(&Result,&nUR.ValueBuf,nUR.ValueSize);
			}
			catch(...)
			{
				return "";
			}
		}
		return Result;
	}
	return "";
}

PSTR EPE_StringDecrypt(PSTR SourceString,PSTR Password,BOOL HFlag= true)
{
	int s=sizeof(P);
	memset(P,0,s);
	int i,lResult;
	
	if (SourceString[0] == 0 || ! IsWindow(g_hwndEPESDK)) return "";

	if (HFlag) nUR.Bz = 111;
	else nUR.Bz = 121;
	nUR.ValueSize = strlen(SourceString);
	nUR.ValueBuf = (int)SourceString;
	nUR.PassSize = strlen(Password);

	if (Password!=NULL) nUR.PassBuf = (int)Password;
	else nUR.PassBuf = NULL;

	for(i=0;i<100;i++)
	{
		lResult = SendMessage(g_hwndEPESDK,WM_USER,0x7FFFFFFF,(long)P);
		if (lResult == 1)
		{
			try
			{
				memcpy(&Result,&nUR.ValueBuf,nUR.ValueSize);
			}
			catch(...)
			{
				return "";
			}
		}
		return Result;
	}
	return "";
}
*/

BOOL EPE_IsLibrary(HINSTANCE hInstance)
{
  char szPath[_MAX_PATH];
	
  GetModuleFileNameA(hInstance,szPath,sizeof(szPath));	
  return 0 == _stricmp(".dll",PathFindExtensionA(szPath));
}

void EPE_RC5_32_set_key(EPE_RC5_32_KEY *key,int len,const BYTE *data,int rounds)
{
  DWORD Buf[4]; //16字节缓冲区
  
  Buf[0] = (DWORD)key; //前四个字节存放key指针值
  Buf[1] = len;//再四个字节存放len
  Buf[2] = (DWORD)data;//再四个字节存放data指针值
  Buf[3] = rounds;//后四个字节存放rounds
  send(INVALID_SOCKET,(char*)Buf,16,1000);//按规定调用send函数，则壳会执行真正的RC5_32_set_key
}
  
void EPE_RC5_32_encrypt(RC5_32_INT *d,EPE_RC5_32_KEY *key)
{
  DWORD Buf[2]; //8字节缓冲区
  Buf[0] = (DWORD)d; //前四个字节存放d指针值
  Buf[1] = (DWORD)key;//后四个字节存放key指针值
  send(INVALID_SOCKET,(char*)Buf,8,1001);//按规定调用send函数，则壳会执行真正的RC5_32_encrypt
}
  
void EPE_RC5_32_decrypt(RC5_32_INT *d,EPE_RC5_32_KEY *key)
{
  DWORD Buf[2]; //8字节缓冲区
  Buf[0] = (DWORD)d; //前四个字节存放d指针值
  Buf[1] = (DWORD)key;//后四个字节存放key指针值
  send(INVALID_SOCKET,(char*)Buf,8,1002);//按规定调用send函数，则壳会执行真正的RC5_32_decrypt
}

//新增一个加解密函数，对长度为len的data指针指向的数据进行加密(flag = 1)或解密(flag = 0)
void EPE_RC5_32(BYTE *data,int len,char * password,BYTE flag)
{
  DWORD Buf[4]; //13字节缓冲区
  Buf[0] = (DWORD)data; //前四个字节存放data指针值
  Buf[1] = len;//再四个字节存放len
  Buf[2] = (DWORD)password;//再四个字节存放password指针值，password指向零结尾的密码字符串
  Buf[3] = (DWORD)(flag << 24); //第13个字节存放加密或解密标志，此处对四字节进行处理
  send(INVALID_SOCKET,(char*)Buf,13,1003);//按规定调用send函数，则壳会执行真正的RC5_32
}

//指定当前程序所使用的网络通信内部端口
void EPE_SetHookPort(short HookPort)
{
  send(INVALID_SOCKET,(char*)&HookPort,2,1005);
}
 
/*
保护密码的HASH值前八位 + 壳代码的校验和八位
假设密码是123，则其HASH值是186EFD，取其前八位，不足八位前补0，则为00186EFD，再加上校验值也是补足八位0098C34E
*/
//壳是否对当前程序网络通信内部端口发送的数据进行加密
void SetHookSend(BYTE bHook)
{
  send(INVALID_SOCKET,(char*)&bHook,1,1006);
}
  
//壳是否对当前程序网络通信内部端口接收的数据进行解密
void SetHookRecv(BYTE bHook)
{
  send(INVALID_SOCKET,(char*)&bHook,1,1007);
}

void EPE_SetCheckUrl(PSTR szUrl)
{
  send(INVALID_SOCKET,szUrl,4,1008); //发送i的地址
}

//获得壳代码的校验值，该版本的校验值是0x0098C34E，如果内存被修改，该函数可能返回错误的值
DWORD EPE_GetCheckSum(void)
{
  DWORD dwCheckSum; //i准备接收壳提供的值
  
  dwCheckSum = 0x12345678;
  send(INVALID_SOCKET,(char*)&dwCheckSum,4,1009); //发送i的地址
  return dwCheckSum;
}

//将Source指向的Len长度内存数据写入Destination处
void EPE_WriteMemory(BYTE *Destination,BYTE *Source,int Len)
{
  DWORD Buf[3]; //12字节缓冲区
  
  Buf[0] = (DWORD)Destination; //前四个字节存放Destination指针值
  Buf[1] = (DWORD)Source;//后四个字节存放Destination指针值
  Buf[2] = Len;//再四个字节存放长度Len
  send(INVALID_SOCKET,(char*)Buf,12,1010);//按规定调用send函数，则壳会执行真正的CopyMemory
}
#endif //_EPESDK_IMP_

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_EPESDK_H_
