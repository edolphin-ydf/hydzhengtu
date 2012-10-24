/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * Socket implementable class.
 *
 */

#ifndef SOCKET_H
#define SOCKET_H

#include "SocketDefines.h"
////////////////////////////////////////////////////////////////
/// @class Socket
/// @brief �׽��ֻ���
///
/// @note ע��˶�����Ҫ�Լ��ͷţ�ֻ��Ҫ����Delete����Disconnect
class SERVER_DECL Socket
{
	public:
		//���캯�������fd=0,fd�ᱻָ������
		Socket(SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize);

		// ��������
		virtual ~Socket();

		// ����ָ���Ļ���.������ip,�˿�
		bool Connect(const char* Address, uint32 Port);

		// �Ͽ�����
		void Disconnect();

		// �����Ѿ����õ��ļ�������
		void Accept(sockaddr_in* address);

		/* ��ʵ�ֵ��鷽�� */

		// �����ݴﵽʱ����
		virtual void OnRead() {}

		// ��һ�����ӳɹ�����ʱ����
		virtual void OnConnect() {}

		// ��һ�����ӶϿ�ʱ����
		virtual void OnDisconnect() {}

		/* Sending Operations */

		// �������ͻ�����,�������ݣ�����������
		bool Send(const uint8* Bytes, uint32 Size);

		// ����ϵͳ - �������ͻ�����
		MNET_INLINE void BurstBegin() { m_writeMutex.Acquire(); }

		// ����ϵͳ - ������ݵ����ͻ�����
		bool BurstSend(const uint8* Bytes, uint32 Size);

		// ����ϵͳ - �ж��Ƿ���Է��ͣ��������ͣ�֮��Ӧ�õ���BurstEnd
		void BurstPush();

		// ����ϵͳ - �������ͻ�����
		MNET_INLINE void BurstEnd() { m_writeMutex.Release(); }

		/* Client Operations */

		// Get the client's ip in numerical form.
		string GetRemoteIP();
		MNET_INLINE uint32 GetRemotePort() { return ntohs(m_client.sin_port); }
		MNET_INLINE SOCKET GetFd() { return m_fd; }

		/* Platform-specific methods */
		
		void SetupReadEvent();
		void ReadCallback(uint32 len);
		void WriteCallback();

		MNET_INLINE bool IsDeleted()
		{
			return m_deleted.GetVal();
		}
		MNET_INLINE bool IsConnected()
		{
			return m_connected.GetVal();
		}
		MNET_INLINE sockaddr_in & GetRemoteStruct() { return m_client; }

		void Delete();//ɾ�����socket

		MNET_INLINE in_addr GetRemoteAddress() { return m_client.sin_addr; }


		CircularBuffer readBuffer;
		CircularBuffer writeBuffer;

		virtual void SendPing() = 0;         /**< ����PING����  */
		virtual void HandPong() = 0;         /**< ����PING����  */
		virtual void SendExit() = 0;
		virtual void HandExit() = 0;
	protected:

		// Called when connection is opened.
		void _OnConnect();

		SOCKET m_fd;

		Mutex m_writeMutex;
		Mutex m_readMutex;

		// we are connected? stop from posting events.
		// �����Ƿ��Ѿ�����,���ӵĻ�ֹͣ�����¼�
		MCodeNet::Threading::AtomicBoolean m_connected;

		// We are deleted? Stop us from posting events.
		// �����Ƿ�ɾ��,ɾ���Ļ�ֹͣ�����¼�
		MCodeNet::Threading::AtomicBoolean m_deleted;

		sockaddr_in m_client;

		unsigned long m_BytesSent;
		unsigned long m_BytesRecieved;

	public:
		// Atomic wrapper functions for increasing read/write locks
		MNET_INLINE void IncSendLock() { ++m_writeLock; }
		MNET_INLINE void DecSendLock() { --m_writeLock; }
		MNET_INLINE bool AcquireSendLock()
		{
			if(m_writeLock.SetVal(1) != 0)
				return false;
			else
				return true;
		}

	private:
		// Write lock, stops multiple write events from being posted.
		MCodeNet::Threading::AtomicCounter m_writeLock;

		/* Win32 - IOCP Specific Calls */
#ifdef CONFIG_USE_IOCP

	public:

		// Set completion port that this socket will be assigned to.
		MNET_INLINE void SetCompletionPort(HANDLE cp) { m_completionPort = cp; }

		OverlappedStruct m_readEvent;
		OverlappedStruct m_writeEvent;

	private:
		// �������ɶ˿�
		HANDLE m_completionPort;

		// ����socket��������ɶ˿�
		void AssignToCompletionPort();

#endif

		/* Linux - EPOLL Specific Calls */
#ifdef CONFIG_USE_EPOLL
	public:
		// Posts a epoll event with the specifed arguments.
		void PostEvent(uint32 events);

		MNET_INLINE bool HasSendLock()
		{
			bool res;
			res = (m_writeLock.GetVal() != 0);
			return res;
		}
#endif

		/* FreeBSD - kqueue specific calls */
#ifdef CONFIG_USE_KQUEUE
	public:
		// Posts a epoll event with the specifed arguments.
		void PostEvent(int events, bool oneshot);
		MNET_INLINE bool HasSendLock()
		{
			bool res;
			res = (m_writeLock.GetVal() != 0);
			return res;
		}
#endif

	public:
		// Polls and resets the traffic data
		void PollTraffic(unsigned long* sent, unsigned long* recieved)
		{

			m_writeMutex.Acquire();
			*sent = m_BytesSent;
			*recieved = m_BytesRecieved;
			m_BytesSent = 0;
			m_BytesRecieved = 0;

			m_writeMutex.Release();
		}
};

/** Connect to a server.
* @param hostname Hostname or IP address to connect to
* @param port Port to connect to
* @return templated type if successful, otherwise null
*/
template<class T>
T* ConnectTCPSocket(const char* hostname, u_short port)
{
	sockaddr_in conn;
	hostent* host;

	/* resolve the peer */
	host = gethostbyname(hostname);

	if(!host)
		return NULL;

	/* copy into our address struct */
	memcpy(&conn.sin_addr, host->h_addr_list[0], sizeof(in_addr));
	conn.sin_family = AF_INET;
	conn.sin_port = ntohs(port);

	T* s = new T(0);
	if(!s->Connect(hostname, port))
	{
		s->Delete();
		return 0;
	}
	return s;
}


#define SOCKET_GC_TIMEOUT 15
////////////////////////////////////////////////////////////////
/// @class SocketGarbageCollector
/// @brief �׽��������ռ���
///
/// @note
class SocketGarbageCollector : public Singleton<SocketGarbageCollector>
{
		map<Socket*, time_t> deletionQueue;
		Mutex lock;
	public:
		~SocketGarbageCollector()
		{
			map<Socket*, time_t>::iterator i;
			for(i = deletionQueue.begin(); i != deletionQueue.end(); ++i)
				delete i->first;
		}

		void Update()
		{
			map<Socket*, time_t>::iterator i, i2;
			time_t t = UNIXTIME;
			lock.Acquire();
			for(i = deletionQueue.begin(); i != deletionQueue.end();)
			{
				i2 = i++;
				if(i2->second <= t)
				{
					delete i2->first;
					deletionQueue.erase(i2);
				}
			}
			lock.Release();
		}

		void QueueSocket(Socket* s)
		{
			lock.Acquire();
			deletionQueue.insert(map<Socket*, time_t>::value_type(s, UNIXTIME + SOCKET_GC_TIMEOUT));
			lock.Release();
		}
};

#define sSocketGarbageCollector SocketGarbageCollector::getSingleton()

#endif
