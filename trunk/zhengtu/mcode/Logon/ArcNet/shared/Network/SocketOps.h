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
	SOCKET CreateTCPFileDescriptor();

	// �����������send/recv����
	bool Nonblocking(SOCKET fd);

	// ��ֹ��������send/recv����
	bool Blocking(SOCKET fd);

	// ��ֹ���ͺϲ���Nagle�㷨
	bool DisableBuffering(SOCKET fd);

	// �����ͺϲ���Nagle�㷨
	bool EnableBuffering(SOCKET fd);

	// ָ�����ͻ�������С
	bool SetRecvBufferSize(SOCKET fd, uint32 size);

	// ָ�����ջ�������С
	bool SetSendBufferSize(SOCKET fd, uint32 size);

	// �趨��ʱʱ��
	bool SetTimeout(SOCKET fd, uint32 timeout);

	// �ر�һ���׽���
	void CloseSocket(SOCKET fd);

	// CloseSocket�ö˿��ͷź��������ö˿ھͿ��Ա��ٴ�ʹ��
	void ReuseAddr(SOCKET fd);
};

#endif

