#include "stdafx.h"
#include "MolSocketOps.h"

namespace SocketOps
{

/** 
 * ����socket���ļ�������
 *
 * @return ���ؽ�����socket������
 */
SOCKET CreateTCPFileDescriptor()
{
	return ::WSASocket(AF_INET,SOCK_STREAM,0,0,0,WSA_FLAG_OVERLAPPED);
}

/** 
 * ���ò�ʹ������ģʽ
 *
 * @param fd Ҫ���õ�socket
 *
 * @return ������óɹ�������,���򷵻ؼ�
 */
bool Nonblocking(SOCKET fd)
{
	u_long arg = 1;
	return (::ioctlsocket(fd,FIONBIO,&arg) == 0);
}

/** 
 * ����ʹ������ģʽ
 *
 * @param fd Ҫ���õ�socket
 *
 * @return ������óɹ�������,���򷵻ؼ�
 */
bool Blocking(SOCKET fd)
{
	u_long arg = 0;
	return (::ioctlsocket(fd,FIONBIO,&arg) == 0);
}

/** 
 * ��ʹ��nagle�����㷨
 *
 * @param fd Ҫ���õ�socket
 *
 * @return ������óɹ�������,���򷵻ؼ�
 */
bool DisableBuffering(SOCKET fd)
{
	uint32 arg = 1;
	return (setsockopt(fd, 0x6, TCP_NODELAY, (const char*)&arg, sizeof(arg)) == 0);
}

/** 
 * ʹ��nagle�����㷨
 *
 * @param fd Ҫ���õ�socket
 *
 * @return ������óɹ�������,���򷵻ؼ�
 */
bool EnableBuffering(SOCKET fd)
{
	uint32 arg = 0;
	return (setsockopt(fd, 0x6, TCP_NODELAY, (const char*)&arg, sizeof(arg)) == 0);
}

/** 
 * ����socket���ڲ����ջ�������С
 *
 * @param fd Ҫ���õ�socket
 * @param size Ҫ���õĻ�������С
 *
 * @return ������óɹ�������,���򷵻ؼ�
 */
bool SetRecvBufferSize(SOCKET fd,uint32 size)
{
	return (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) == 0);
}

/** 
 * ����socket���ڲ����ͻ�������С
 *
 * @param fd Ҫ���õ�socket
 * @param size Ҫ���õĻ�������С
 *
 * @return ������óɹ�������,���򷵻ؼ�
 */
bool SetSendBufferSize(SOCKET fd,uint32 size)
{
	return (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) == 0);
}

/** 
 * ����socket�ĳ�ʱ
 *
 * @param fd Ҫ���õ�socket
 * @param timeout Ҫ���õ�socket�ĳ�ʱʱ��
 *
 * @return ������óɹ�������,���򷵻ؼ�
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
 * ��ȫ�ر�socket
 *
 * @param fd Ҫ�رյ�socket
 */
void CloseSocket(SOCKET fd)
{
	shutdown(fd, SD_BOTH);
	closesocket(fd);
}

/** 
 * ���� SO_REUSEADDR
 *
 * @param fd Ҫ���õ�socket
 */
void ReuseAddr(SOCKET fd)
{
	uint32 option = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, 4);
}

}