#include "stdafx.h"
#include "MolSocketOps.h"

namespace SocketOps
{

/** 
 * 建立socket的文件描述符
 *
 * @return 返回建立的socket描述符
 */
SOCKET CreateTCPFileDescriptor()
{
	return ::WSASocket(AF_INET,SOCK_STREAM,0,0,0,WSA_FLAG_OVERLAPPED);
}

/** 
 * 设置不使用阻塞模式
 *
 * @param fd 要设置的socket
 *
 * @return 如果设置成功返回真,否则返回假
 */
bool Nonblocking(SOCKET fd)
{
	u_long arg = 1;
	return (::ioctlsocket(fd,FIONBIO,&arg) == 0);
}

/** 
 * 设置使用阻塞模式
 *
 * @param fd 要设置的socket
 *
 * @return 如果设置成功返回真,否则返回假
 */
bool Blocking(SOCKET fd)
{
	u_long arg = 0;
	return (::ioctlsocket(fd,FIONBIO,&arg) == 0);
}

/** 
 * 不使用nagle缓冲算法
 *
 * @param fd 要设置的socket
 *
 * @return 如果设置成功返回真,否则返回假
 */
bool DisableBuffering(SOCKET fd)
{
	uint32 arg = 1;
	return (setsockopt(fd, 0x6, TCP_NODELAY, (const char*)&arg, sizeof(arg)) == 0);
}

/** 
 * 使用nagle缓冲算法
 *
 * @param fd 要设置的socket
 *
 * @return 如果设置成功返回真,否则返回假
 */
bool EnableBuffering(SOCKET fd)
{
	uint32 arg = 0;
	return (setsockopt(fd, 0x6, TCP_NODELAY, (const char*)&arg, sizeof(arg)) == 0);
}

/** 
 * 设置socket的内部接收缓冲区大小
 *
 * @param fd 要设置的socket
 * @param size 要设置的缓冲区大小
 *
 * @return 如果设置成功返回真,否则返回假
 */
bool SetRecvBufferSize(SOCKET fd,uint32 size)
{
	return (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) == 0);
}

/** 
 * 设置socket的内部发送缓冲区大小
 *
 * @param fd 要设置的socket
 * @param size 要设置的缓冲区大小
 *
 * @return 如果设置成功返回真,否则返回假
 */
bool SetSendBufferSize(SOCKET fd,uint32 size)
{
	return (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) == 0);
}

/** 
 * 设置socket的超时
 *
 * @param fd 要设置的socket
 * @param timeout 要设置的socket的超时时间
 *
 * @return 如果设置成功返回真,否则返回假
 */
bool SetTimeout(SOCKET fd,uint32 timeout)
{
	struct timeval to;
	to.tv_sec = timeout;
	to.tv_usec = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&to, (socklen_t)sizeof(to)) != 0) return false;
	return (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&to, (socklen_t)sizeof(to)) == 0);
}

/** 
 * 完全关闭socket
 *
 * @param fd 要关闭的socket
 */
void CloseSocket(SOCKET fd)
{
	shutdown(fd, SD_BOTH);
	closesocket(fd);
}

/** 
 * 设置 SO_REUSEADDR
 *
 * @param fd 要设置的socket
 */
void ReuseAddr(SOCKET fd)
{
	uint32 option = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, 4);
}

}