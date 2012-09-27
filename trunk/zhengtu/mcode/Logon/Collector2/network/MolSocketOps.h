#ifndef _MOL_SOCKET_OPS_H_INCLUDE
#define _MOL_SOCKET_OPS_H_INCLUDE

/** 
* MolNet��������
*
* ����:��������IOCP��һЩ����
* ����:akinggw
* ����:2010.2.11
*/

#include "MolCommon.h"

namespace SocketOps
{
/// ����socket���ļ�������
SOCKET CreateTCPFileDescriptor();

/// ���ò�ʹ������ģʽ
bool Nonblocking(SOCKET fd);

/// ����ʹ������ģʽ
bool Blocking(SOCKET fd);

/// ��ʹ��nagle�����㷨
bool DisableBuffering(SOCKET fd);

/// ʹ��nagle�����㷨
bool EnableBuffering(SOCKET fd);

/// ����socket���ڲ����ջ�������С
bool SetRecvBufferSize(SOCKET fd,uint32 size);

/// ����socket���ڲ����ͻ�������С
bool SetSendBufferSize(SOCKET fd,uint32 size);

/// ����socket�ĳ�ʱ
bool SetTimeout(SOCKET fd,uint32 timeout);

/// ��ȫ�ر�socket
void CloseSocket(SOCKET fd);

/// ���� SO_REUSEADDR
void ReuseAddr(SOCKET fd);
}

#endif
