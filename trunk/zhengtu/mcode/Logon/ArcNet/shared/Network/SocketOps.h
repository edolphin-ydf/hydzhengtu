/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketOps - wrapper for any specific socket operations that may be platform-dependant.
 *
 */


#ifndef SOCKET_OPS_H
#define SOCKET_OPS_H

namespace SocketOps
{
	// ��ʵ���Ǵ���һ���׽���
	SERVER_DECL SOCKET CreateTCPFileDescriptor();

	// �����������send/recv����
	SERVER_DECL bool Nonblocking(SOCKET fd);

	// ��ֹ��������send/recv����
	SERVER_DECL bool Blocking(SOCKET fd);

	// ��ֹ���ͺϲ���Nagle�㷨
	SERVER_DECL bool DisableBuffering(SOCKET fd);

	// �����ͺϲ���Nagle�㷨
	SERVER_DECL bool EnableBuffering(SOCKET fd);

	// ָ�����ͻ�������С
	SERVER_DECL bool SetRecvBufferSize(SOCKET fd, uint32 size);

	// ָ�����ջ�������С
	SERVER_DECL bool SetSendBufferSize(SOCKET fd, uint32 size);

	// �趨��ʱʱ��
	SERVER_DECL bool SetTimeout(SOCKET fd, uint32 timeout);

	// �ر�һ���׽���
	SERVER_DECL void CloseSocket(SOCKET fd);

	// CloseSocket�ö˿��ͷź��������ö˿ھͿ��Ա��ٴ�ʹ��
	SERVER_DECL void ReuseAddr(SOCKET fd);
};

#endif

