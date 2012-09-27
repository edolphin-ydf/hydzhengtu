#ifndef _BZ_CRASH_RPT_H_INCLUDE
#define _BZ_CRASH_RPT_H_INCLUDE

/**
 * BzCrashRpt ��������
 *
 * ������Ŀ�����ڳ����г����쳣�����ʱ�����ʹ��󱨸浽ָ�������䣬�Ա���
 * ������Ա����
 *
 * ����: akinggw
 * ����: 2010.3.12
 */

#include "BzCommon.h"

/// ��ʼ�������Լ����쳣�������
EXT_CLASS void          MolCrash_Initiation(void); 
/// ����windowsϵͳ�е��쳣�������(��Ҫ���VS2005),���������Զ�����쳣������ƺ�ʹ��
EXT_CLASS void          MolCrash_DisableSetUnhandledExceptionFilter();
/// ����������Ŀ����
EXT_CLASS void          MolCrash_SetProjectName(const char* name);
/// �õ��쳣��Ϣ
EXT_CLASS const char*   MolCrash_GetExceptionString(void);
/// �õ�������Ŀ����
EXT_CLASS const char*   MolCrash_GetProjectName(void);
/// ����ѹ������(��ʱ����)
EXT_CLASS void          MolCrash_SetZipPassword(const char* pass);
/// �õ�ѹ������
EXT_CLASS const char*   MolCrash_GetZipPassword(void);
/// ���÷������ʼ���ַ
EXT_CLASS void          MolCrash_SetEmailSender(const char* address);
/// ���ý������ʼ���ַ
EXT_CLASS void          MolCrash_SetEmailReceiver(const char* address);
/// �����Ƿ�����ɺ�ɾ���ļ�����ʼ��ɾ����
EXT_CLASS void          MolCrash_DeleteSended(bool isDel);
/// �����ʼ����ͷ�����
EXT_CLASS void          MolCrash_SetSmtpServer(const char *server);
/// �õ��ʼ����ͷ�����
EXT_CLASS const char*   MolCrash_GetSmtpServer(void);
/// �����ʼ�������
EXT_CLASS void          MolCrash_SetSmtpUser(const char* user);
/// �õ��ʼ�������
EXT_CLASS const char*   MolCrash_GetSmtpUser(void);
/// �����ʼ�����������
EXT_CLASS void          MolCrash_SetSmtpPassword(const char* pass);
/// �õ��ʼ�����������
EXT_CLASS const char*   MolCrash_GetSmtpPassword(void);

#endif
