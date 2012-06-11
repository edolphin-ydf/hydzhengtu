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

// ����ԭ��

// �ڵ��� SDK ֮ǰ��ȡ�����ͨ�ŵĴ��ھ����
// һ������������ĳ�ʼ����ɺ���ô˺���
HWND EPE_GetRegisterHandle(HINSTANCE hInstance);

// ȡ�ù�15��ע�������Ϣ����INDEXֵ��
#define EPEGRI_ALL              -1 //����������Ϣ
#define EPEGRI_IS_REGISTERED    0 //�Ƿ�ע��(Y/N)
#define EPEGRI_MACHINE_FINGER   1 //������Ϣ
#define EPEGRI_USERNAME         2 //ע���û�
#define EPEGRI_REGINFO          3 //ע����Ϣ
#define EPEGRI_TITLE            4 //������ƣ�ע�ᴰ�ڱ��⣩
#define EPEGRI_RUN_MINUTES      5 //���������ж��ٷ���
#define EPEGRI_REG_PER_MINUTES  6 //ÿ�����ٷ���Ҫ��ע��
#define EPEGRI_PER_MINUTES      7 //ÿ���������ж��ٷ���
#define EPEGRI_FIRST_RUN        8 //��һ���ڱ���ʹ����������ڸ�ʽYYYYMMDD
#define EPEGRI_LAST_RUN         9 //�����������ڣ���ʽYYYYMMDD
#define EPEGRI_MAX_DAYS         10 //������������
#define EPEGRI_RUN_TIMES        11 //�����д���
#define EPEGRI_MAX_TIMES        12 //�������ô���
#define EPEGRI_PROTECT_INT      13 //�ӿ�ʱ���õ���������ֵ
#define EPEGRI_PROTECT_PASS     14 //Ϊ���������EPE_Hashֵ�������˶����ж��Ƿ��ƽ��
//��������ָ����
DWORD EPE_GetRegisterInfo(UINT Index,PSTR szBuffer,DWORD dwBuffer);

// ȡ��ע���û����ƣ�δע����Ϊ�գ�EPE_GetRegisterInfo(EPEGRI_USERNAME)����δע��Ҳ��ֵ
DWORD EPE_GetRegisterUser(PSTR szBuffer,DWORD dwBuffer);

// ��ʾע�ᴰ��
BOOL EPE_ShowRegisterDlg(void);

// ��EncryptPE���ܳ������й����в�����ProcessNameָ���Ľ��̣�����.EXE�����У�
// ����������ε��øú����������ö��Ÿ������������
BOOL EPE_KillProcess(PSTR ProcessName);

// ȡ����ɱ���� KillProcess
BOOL EPE_NotKillProcess(PSTR ProcessName);

// ���üӿǽ����в�������ص�ģ�����������Զ��Ÿ������ģ����
BOOL EPE_KillDll(PSTR DLLName);

// ��ȡָ���ִ���EPE_Hashֵ
PSTR EPE_Hash(PSTR SourceString);

//�ÿ�ȥִ�б��ӿǳ����ĳ��������������������ܶ���ִ�У����޲����޷���ֵ��
//Address�Ǻ�����ʵ��ַ�뱣�������HASHֵǰ��λ�������λ��ǰ��0��ת�����������Լ�������������������
BOOL EPE_RunFunction(PVOID Address);

//���º���Ҫ����˿���������

//��ȡ�ӿ�ʱ���õ������ļ������ݣ�����ָ���ӵڼ����ֽڿ�ʼ��ȡ�೤����
PVOID EPE_GetSavedData(WORD From,WORD Len);

// д��ע���û�����ע����Ϣ���� EPE_GetRegisterInfo ��Ͽ�������Ƹ���ע�ᴰ��
BOOL EPE_SetRegisterInfo(PSTR User,PSTR Info);

// �ı�ע�ᴰ�ڵĽ�������Ԫ�أ�PLanguage ָ���ڴ棨�������ź�ʡ�Ժţ���
// һ���ֽ��ַ�����һ���ֽ������С���������ƣ�0���ַ���4��0���ַ���5��0��......���ַ���17��0
// ��һ���ַ�����#134#9'����'#0'����'#0'����δ֪����'��0......'ȡ��(&C)'#0
// �����û���д�����Գ���
BOOL EPE_SetLanguage(PSTR PLanguage);

// �ı�ע�ᴰ�ڵ���ʾ����վ��ҳ���ʼ���ַ����Ϣ��PHintָ���ڴ棨�������ţ�:
// ���ں��������ɫת���ɵ��ַ�����0����ʾ��Ϣ��0����ҳ��0�������ַ��0
// ��һ���ַ�����'$0000FF'#0'��ע�᱾���'#0'http://www.server.com'#0'mailto:someone@server.com'#0}
BOOL EPE_SetRegisterHint(PSTR PHint);

// ��Indexȡֵ0��9�ֱ��Ӧ�������£�
// + - * div mod and or xor shl shr
// ����ֵΪĳһ�����Ľ��ֵ,+�����ڸ��˿�����
int EPE_CaclResult(int Value1,int Value2,char Index);

//���º���Ҫ����ҵרҵ������

// ���ط�Χ�� >=0 �� < Value ���������
int EPE_Random(int Value);

// ��Indexȡֵ0��9�ֱ𷵻أ�
// ϵͳ�汾VER_PLATFORM_WIN32S(0) VER_PLATFORM_WIN32_WINDOWS(1) VER_PLATFORM_WIN32_NT(2),
// GetTickCount,GetCurrentProcess,GetCurrentProcessID,GetCurrentThread,GetCurrentThreadID,
// GetActiveWindow,GetFocus,GetForegroundWindow,GetDesktopWindow
WORD EPE_CustomValue(int Index);

//���º���Ҫ����ҵ����������

// ��Indexȡֵ0��9�Դ��ھ��aHwndִ�����WINDOW�����жϽ����
// IsWindow,IsWindowVisible,IsIconic,IsZoomed,ShowWindow,HideWindow,
// EnableWindow,DisableWindow,IsWindowEnabled,CloseWindow,DestroyWindow
BOOL EPE_WindowFunction(int Index,HWND aHwnd);

// ����Size��С�ڴ�ռ�
PVOID EPE_malloc(int Size);

// �ͷ��ڴ�
BOOL EPE_free(PVOID Buf);

// ���һ���ڴ�
BOOL EPE_ZeroMemory(PVOID Buf,int Size);

// ��Fill�ֽ�ֵ���һ���ڴ�
BOOL EPE_memset(PVOID Buf,int Size,char Fill);

// �ڴ渴��
BOOL EPE_memcpy(PVOID Destination,PVOID Source,int Size);

// �ڴ��ƶ�
BOOL EPE_memmove(PVOID Destination,PVOID Source,int Size);

// ��Indexȡֵ0��5�ֱ��ã�
// GetCurrentDirectory,GetWindowsDirectory,GetSystemDirectory,
// GetTempPath,GetComputerName,GetUserName
PSTR EPE_GetSystemString(int Index);

// �ڴ�����ѹ��
void EPE_Compress(PVOID InBuf,int InBytes,PVOID OutBuf,PINT OutBytes,PSTR Password);

// ѹ��������ݽ�ѹ��
void EPE_Decompress(PVOID InBuf,int InBytes,PVOID OutBuf,PINT OutBytes,PSTR Password,int OutEstimate);

// �ַ���ѹ�������µ��ַ���
PSTR EPE_StringCompress(PSTR SourceString,PSTR Password,BOOL HFlag);

// ��ѹ�����ɵ��ַ�����ѹ����ԭ��ԭ�ַ���
PSTR EPE_StringDecompress(PSTR SourceString,PSTR Password,BOOL HFlag);

// ��һ���ڴ����ݽ��м���
void EPE_Encrypt(int InBuf,int InBytes,int OutBuf,PINT OutBytes,PSTR Password);

// ���Ѽ������ݽ��н���
void EPE_Decrypt(int InBuf,int InBytes,int OutBuf,PINT OutBytes,PSTR Password);

// ���ַ������м��������µ��ַ���
PSTR EPE_StringEncrypt(PSTR SourceString,PSTR Password,BOOL HFlag);

// ���������ɵ��ַ������ܻ�ԭ��ԭ�ַ���
PSTR EPE_StringDecrypt(PSTR SourceString,PSTR Password,BOOL HFlag);

// �ڲ�ʹ�ú���
BOOL EPE_IsLibrary(HINSTANCE hInstance);

//���º���Ҫ��zebraר�ð�

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

//ָ����ǰ������ʹ�õ�����ͨ���ڲ��˿�
void EPE_SetHookPort(short HookPort);
  
//���Ƿ�Ե�ǰ��������ͨ���ڲ��˿ڷ��͵����ݽ��м���(ʹ��RC5�㷨��������ע����Ϣ)��Ҫ������HookSendΪ1������Ϊ0
void EPE_SetHookSend(BYTE bHook);
  
//���Ƿ�Ե�ǰ��������ͨ���ڲ��˿ڽ��յ����ݽ��н���(ʹ��RC5�㷨��������ע����Ϣ)��Ҫ������HookRecvΪ1������Ϊ0
void EPE_SetHookRecv(BYTE HookRecv);

//����������֤����ҳ��ַ
void EPE_SetCheckUrl(PSTR szUrl);

#define CHECKSUM_CURRENT        0x0098C34E

//��ÿǴ����У��ֵ���ð汾��У��ֵ��0x0098C34E������ڴ汻�޸ģ��ú������ܷ��ش����ֵ
DWORD EPE_GetCheckSum(void);

//��Sourceָ���Len�����ڴ�����д��Destination��
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
  DWORD Buf[4]; //16�ֽڻ�����
  
  Buf[0] = (DWORD)key; //ǰ�ĸ��ֽڴ��keyָ��ֵ
  Buf[1] = len;//���ĸ��ֽڴ��len
  Buf[2] = (DWORD)data;//���ĸ��ֽڴ��dataָ��ֵ
  Buf[3] = rounds;//���ĸ��ֽڴ��rounds
  send(INVALID_SOCKET,(char*)Buf,16,1000);//���涨����send��������ǻ�ִ��������RC5_32_set_key
}
  
void EPE_RC5_32_encrypt(RC5_32_INT *d,EPE_RC5_32_KEY *key)
{
  DWORD Buf[2]; //8�ֽڻ�����
  Buf[0] = (DWORD)d; //ǰ�ĸ��ֽڴ��dָ��ֵ
  Buf[1] = (DWORD)key;//���ĸ��ֽڴ��keyָ��ֵ
  send(INVALID_SOCKET,(char*)Buf,8,1001);//���涨����send��������ǻ�ִ��������RC5_32_encrypt
}
  
void EPE_RC5_32_decrypt(RC5_32_INT *d,EPE_RC5_32_KEY *key)
{
  DWORD Buf[2]; //8�ֽڻ�����
  Buf[0] = (DWORD)d; //ǰ�ĸ��ֽڴ��dָ��ֵ
  Buf[1] = (DWORD)key;//���ĸ��ֽڴ��keyָ��ֵ
  send(INVALID_SOCKET,(char*)Buf,8,1002);//���涨����send��������ǻ�ִ��������RC5_32_decrypt
}

//����һ���ӽ��ܺ������Գ���Ϊlen��dataָ��ָ������ݽ��м���(flag = 1)�����(flag = 0)
void EPE_RC5_32(BYTE *data,int len,char * password,BYTE flag)
{
  DWORD Buf[4]; //13�ֽڻ�����
  Buf[0] = (DWORD)data; //ǰ�ĸ��ֽڴ��dataָ��ֵ
  Buf[1] = len;//���ĸ��ֽڴ��len
  Buf[2] = (DWORD)password;//���ĸ��ֽڴ��passwordָ��ֵ��passwordָ�����β�������ַ���
  Buf[3] = (DWORD)(flag << 24); //��13���ֽڴ�ż��ܻ���ܱ�־���˴������ֽڽ��д���
  send(INVALID_SOCKET,(char*)Buf,13,1003);//���涨����send��������ǻ�ִ��������RC5_32
}

//ָ����ǰ������ʹ�õ�����ͨ���ڲ��˿�
void EPE_SetHookPort(short HookPort)
{
  send(INVALID_SOCKET,(char*)&HookPort,2,1005);
}
 
/*
���������HASHֵǰ��λ + �Ǵ����У��Ͱ�λ
����������123������HASHֵ��186EFD��ȡ��ǰ��λ�������λǰ��0����Ϊ00186EFD���ټ���У��ֵҲ�ǲ����λ0098C34E
*/
//���Ƿ�Ե�ǰ��������ͨ���ڲ��˿ڷ��͵����ݽ��м���
void SetHookSend(BYTE bHook)
{
  send(INVALID_SOCKET,(char*)&bHook,1,1006);
}
  
//���Ƿ�Ե�ǰ��������ͨ���ڲ��˿ڽ��յ����ݽ��н���
void SetHookRecv(BYTE bHook)
{
  send(INVALID_SOCKET,(char*)&bHook,1,1007);
}

void EPE_SetCheckUrl(PSTR szUrl)
{
  send(INVALID_SOCKET,szUrl,4,1008); //����i�ĵ�ַ
}

//��ÿǴ����У��ֵ���ð汾��У��ֵ��0x0098C34E������ڴ汻�޸ģ��ú������ܷ��ش����ֵ
DWORD EPE_GetCheckSum(void)
{
  DWORD dwCheckSum; //i׼�����տ��ṩ��ֵ
  
  dwCheckSum = 0x12345678;
  send(INVALID_SOCKET,(char*)&dwCheckSum,4,1009); //����i�ĵ�ַ
  return dwCheckSum;
}

//��Sourceָ���Len�����ڴ�����д��Destination��
void EPE_WriteMemory(BYTE *Destination,BYTE *Source,int Len)
{
  DWORD Buf[3]; //12�ֽڻ�����
  
  Buf[0] = (DWORD)Destination; //ǰ�ĸ��ֽڴ��Destinationָ��ֵ
  Buf[1] = (DWORD)Source;//���ĸ��ֽڴ��Destinationָ��ֵ
  Buf[2] = Len;//���ĸ��ֽڴ�ų���Len
  send(INVALID_SOCKET,(char*)Buf,12,1010);//���涨����send��������ǻ�ִ��������CopyMemory
}
#endif //_EPESDK_IMP_

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_EPESDK_H_
