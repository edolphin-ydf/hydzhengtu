#ifndef _MOL_SOCKET_OPS_H_INCLUDE
#define _MOL_SOCKET_OPS_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:用于设置IOCP的一些参数
* 作者:akinggw
* 日期:2010.2.11
*/

#include "MolCommon.h"

namespace SocketOps
{
/// 建立socket的文件描述符
SOCKET CreateTCPFileDescriptor();

/// 设置不使用阻塞模式
bool Nonblocking(SOCKET fd);

/// 设置使用阻塞模式
bool Blocking(SOCKET fd);

/// 不使用nagle缓冲算法
bool DisableBuffering(SOCKET fd);

/// 使用nagle缓冲算法
bool EnableBuffering(SOCKET fd);

/// 设置socket的内部接收缓冲区大小
bool SetRecvBufferSize(SOCKET fd,uint32 size);

/// 设置socket的内部发送缓冲区大小
bool SetSendBufferSize(SOCKET fd,uint32 size);

/// 设置socket的超时
bool SetTimeout(SOCKET fd,uint32 timeout);

/// 完全关闭socket
void CloseSocket(SOCKET fd);

/// 设置 SO_REUSEADDR
void ReuseAddr(SOCKET fd);
}

#endif
