#ifndef _MOL_SOCKET_H_INCLUDE
#define _MOL_SOCKET_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:我们接收的网络客户端
* 作者:akinggw
* 日期:2010.2.11
*/

#include "MolCommon.h"
#include "MolMutex.h"
#include "AtomicBoolean.h"
#include "AtomicCounter.h"
#include "MolSingleton.h"
#include "MolCircularBuffer.h"
#include "MolSocketDefines.h"
#include "MolThreadStarter.h"

#include "MolMessageOut.h"

//#include "VMemPool.h"

#define SOCKET_GC_TIMEOUT 15

class Socket/* : public CVMemPool<Socket>*/
{
public:
	/// 构造函数
	Socket(SOCKET fd,uint32 sendbuffersize,uint32 recvbuffersize);
	/// 析构函数
	virtual ~Socket();

	/// 连接指定的机器
	bool Connect(const char * Address,uint32 Port);
	/// 断开连接
	void Disconnect(bool isDel=true);
	/// 接收已经设置的文件描述符
	void Accept(sockaddr_in * address);
	
	/// 当数据达到时调用
	virtual void OnRead(uint32 size) {}
	/// 当一个连接成功建立时调用
	virtual void OnConnect() {}
	/// 当一个连接断开时调用
	virtual void OnDisconnect() {}

	/// 锁定发送互斥锁,发送数据，解锁互斥锁
	bool Send(const uint8 * Bytes,uint32 Size);
	/// 发送数据
	bool Send(CMolMessageOut &out);
	/// 爆裂系统 - 锁定发送缓冲区
	inline bool BurstBegin() { m_writeMutex.Acquire(); return true; }
	/// 爆裂系统 - 添加数据到发送缓冲区
	bool BurstSend(const uint8 * Bytes,uint32 Size);
	/// 爆裂系统 - 压入事件到队列中 在最后做写事件
	void BurstPush();
	/// 爆裂系统 - 解锁发送互斥锁
	inline void BurstEnd() { m_writeMutex.Release(); }

	std::string GetRemoteIP();
	inline uint32 GetRemotePort() { return ntohs(m_client.sin_port); }
	inline SOCKET GetFd() { return m_fd; }

	void SetupReadEvent();
	void ReadCallback(uint32 len);
	void WriteCallback();

	inline void Clear(void)
	{
		m_connected.SetVal(false);
		m_deleted.SetVal(false);
		m_writeLock.SetVal(false);
		removedFromSet = false;
	}
	inline bool IsDeleted()
	{
		return m_deleted.GetVal();
	}
	inline bool IsConnected()
	{
		return m_connected.GetVal();
	}
	//设置心跳计数
	inline void SetHeartCount(uint32 count) { m_heartJitter = count; }

	//获得心跳计数
	inline uint32 GetHeartCount(void) { return m_heartJitter; }

	inline sockaddr_in & GetRemoteStruct() { return m_client; }
	inline MolCircularBuffer & GetReadBuffer() { return readBuffer; }
	inline MolCircularBuffer & GetWriteBuffer() { return writeBuffer; }

	void Delete();

	inline in_addr GetRemoteAddress() { return m_client.sin_addr; }

	inline void SetCompletionPort(HANDLE cp) { m_completionPort = cp; }

	inline void IncSendLock() { ++m_writeLock; }
	inline void DecSendLock() { --m_writeLock; }
	inline bool AcquireSendLock()
	{
		if(m_writeLock.SetVal(true) != 0)
			return false;
		else
			return true;
	}

	// Polls and resets the traffic data
	void PollTraffic( unsigned long *sent, unsigned long *recieved ){

		m_writeMutex.Acquire();
		*sent = m_BytesSent;
		*recieved = m_BytesRecieved;
		m_BytesSent = 0;
		m_BytesRecieved = 0;

		m_writeMutex.Release();
	}

	OverlappedStruct m_readEvent;
	OverlappedStruct m_writeEvent;
	bool removedFromSet;

public:
	/// 加密数据
	void Encrypto(unsigned char *data,unsigned long length);
	/// 解密数据
	void Decrypto(unsigned char *data,unsigned long length);

protected:
	/// 当连接打开时调用
	void _OnConnect();

	SOCKET m_fd;

	Mutex m_writeMutex;
	Mutex m_readMutex;

	// 我们是否已经连接,连接的话停止传送事件
	AtomicBoolean m_connected;

	// 我们是否被删除,删除的话停止传送事件
	AtomicBoolean m_deleted;

	sockaddr_in m_client; 

	MolCircularBuffer readBuffer;
	MolCircularBuffer writeBuffer;

	unsigned long m_BytesSent;
	unsigned long m_BytesRecieved;

	uint32					m_heartJitter;

private:
	// 我们系统建立的IOCP端口
	HANDLE m_completionPort;

	// 写互斥锁,防止多个写操作同时发送
	AtomicCounter m_writeLock;

	// 关联socket到它的完成端口
	void AssignToCompletionPort();
};

class SocketGarbageCollector : public Singleton<SocketGarbageCollector>
{
public:
	/// 析构函数
	~SocketGarbageCollector()
	{
		std::map<Socket*,time_t>::iterator i;
		for(i=deletonQueue.begin();i!=deletonQueue.end();++i)
		{
			Socket *pSocket = i->first;
			delete pSocket;
			pSocket = NULL;
		}
	}

	void Update();

	inline void QueueSocket(Socket * s)
	{
		if(s == NULL) return;

		lock.Acquire();
		deletonQueue.insert(std::map<Socket*,time_t>::value_type(s,time(NULL) + SOCKET_GC_TIMEOUT));
		lock.Release();
	}

	inline void DeleteSocket(void)
	{
		lock.Acquire();
		std::map<Socket*,time_t>::iterator i;
		for(i=deletonQueue.begin();i!=deletonQueue.end();++i)
		{
			Socket *pSocket = i->first;
			if(pSocket == NULL) continue;

			delete pSocket;
			pSocket = NULL;
		}
		deletonQueue.clear();
		lock.Release();
	}

	inline void Lock(void) { lock.Acquire(); }
	inline void Unlock(void) { lock.Release(); }

private:
	std::map<Socket*,time_t> deletonQueue;
	Mutex lock;
};

#define sSocketGarbageCollector SocketGarbageCollector::getSingleton()

class NetClient : public Socket
{
public:
	/// 构造函数
	NetClient(SOCKET fd);
	/// 析构函数
	~NetClient();

	/// 当数据达到时调用
	virtual void OnRead(uint32 size);
	/// 当一个连接成功建立时调用
	virtual void OnConnect();
	/// 当一个连接断开时调用
	virtual void OnDisconnect();

private:
	uint32 remaining;
	uint16 opcode;
};

class MolNetworkUpdate : public ThreadBase
{
public:
	/// 构造函数
	MolNetworkUpdate();
	/// 析构函数
	~MolNetworkUpdate();

	bool run();

private:
	DWORD m_curTime,m_TimeSpeed;
	DWORD m_threadTimer,m_threadTimeSpeed;
	DWORD m_UpdateTime,m_UpdateTimeSpeed;
};

extern MolNetworkUpdate *m_NetworkUpdate;

#endif