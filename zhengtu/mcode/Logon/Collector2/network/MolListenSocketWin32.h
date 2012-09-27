#ifndef _MOL_LISTEN_SOCKET_WIN32_H_INCLUDE
#define _MOL_LISTEN_SOCKET_WIN32_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:用于侦听其它客户端的连接
* 作者:akinggw
* 日期:2010.2.12
*/

#include "MolCommon.h"
#include "MolThreadPool.h"
#include "MolSocket.h"
#include "MolSocketOps.h"

template<class T>
class ListenSocket : public ThreadBase
{
public:
	/// 构造函数
	ListenSocket()
		: m_socket(NULL),m_opened(false),timeout(60),len(0),aSocket(NULL),
		  socket(NULL),buffersize(0)
	{

	}
	/// 析构函数
	~ListenSocket()
	{
		Close();
	}

	/// 设置socket内部使用的缓冲区大小，为0的话不设置
	void SetBufferSize(uint32 size)
	{
		if(size > 0)
		{
			buffersize = size;

			SocketOps::SetRecvBufferSize(m_socket,buffersize);
			SocketOps::SetSendBufferSize(m_socket,buffersize);
		}
	}
	/// 设置socket的超时
	void SetTimeOut(uint32 time)
	{
		timeout = time;
	}

	/// 启动服务器,等待其它客户端连接
	bool Start(const char * ListenAddress, uint32 Port)
	{
		m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		SocketOps::ReuseAddr(m_socket);
		SocketOps::Blocking(m_socket);
		SocketOps::SetTimeout(m_socket, timeout);

		m_address.sin_family = AF_INET;
		m_address.sin_port = ntohs((u_short)Port);
		m_address.sin_addr.s_addr = htonl(INADDR_ANY);
		m_opened = false;

		if(inet_addr(ListenAddress) == INADDR_NONE)
		{
			struct hostent * hostname = gethostbyname(ListenAddress);
			if(hostname != 0)
				memcpy(&m_address.sin_addr.s_addr, hostname->h_addr_list[0], hostname->h_length);
		}

		// bind.. well attempt to.
		int ret = bind(m_socket, (const sockaddr*)&m_address, sizeof(m_address));
		if(ret != 0)
		{
			TRACE("Bind unsuccessful on port %u.", Port);
			return false;
		}

		ret = listen(m_socket, 5);
		if(ret != 0) 
		{
			TRACE("Unable to listen on port %u.", Port);
			return false;
		}

		m_opened = true;
		len = sizeof(sockaddr_in);
		m_cp = sSocketMgr.GetCompletionPort();

		return true;
	}

	bool run()
	{
		while(m_opened)
		{
			// 如果当前系统中客户端数量大于最大个数就停止接收连接
			if(sSocketMgr.GetMaxSockets() > 0 &&
				sSocketMgr.GetClientCount() > sSocketMgr.GetMaxSockets())
				continue;

			//aSocket = accept(m_socket, (sockaddr*)&m_tempAddress, (socklen_t*)&len);
			aSocket = WSAAccept(m_socket, (sockaddr*)&m_tempAddress, (socklen_t*)&len, NULL, NULL);
			if(aSocket == INVALID_SOCKET)
				continue;		// shouldn't happen, we are blocking.

			socket = new T(aSocket);
			socket->SetCompletionPort(m_cp);
			socket->Accept(&m_tempAddress);
		}

		return false;
	}

	void Close()
	{
		// prevent a race condition here.
		bool mo = m_opened;
		m_opened = false;

		if(mo)
			SocketOps::CloseSocket(m_socket);
	}

	inline bool IsOpen() { return m_opened; }

private:
	SOCKET m_socket;
	struct sockaddr_in m_address;
	struct sockaddr_in m_tempAddress;
	bool m_opened;
	uint32 timeout;
	uint32 buffersize;
	uint32 len;
	SOCKET aSocket;
	T * socket;
	HANDLE m_cp;
};

extern ListenSocket<NetClient> *m_ServerSocket;

#endif
