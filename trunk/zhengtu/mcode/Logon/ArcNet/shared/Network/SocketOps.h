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
	// 其实就是创建一个套接字
	SOCKET CreateTCPFileDescriptor();

	// 允许非阻塞的send/recv调用
	bool Nonblocking(SOCKET fd);

	// 禁止非阻塞的send/recv调用
	bool Blocking(SOCKET fd);

	// 禁止发送合并的Nagle算法
	bool DisableBuffering(SOCKET fd);

	// 允许发送合并的Nagle算法
	bool EnableBuffering(SOCKET fd);

	// 指定发送缓冲区大小
	bool SetRecvBufferSize(SOCKET fd, uint32 size);

	// 指定接收缓冲区大小
	bool SetSendBufferSize(SOCKET fd, uint32 size);

	// 设定超时时间
	bool SetTimeout(SOCKET fd, uint32 timeout);

	// 关闭一个套接字
	void CloseSocket(SOCKET fd);

	// CloseSocket让端口释放后立即调用端口就可以被再次使用
	void ReuseAddr(SOCKET fd);
};

#endif

