#ifndef _MOL_SOCKET_H_INCLUDE
#define _MOL_SOCKET_H_INCLUDE

/** 
* MolNet��������
*
* ����:���ǽ��յ�����ͻ���
* ����:akinggw
* ����:2010.2.11
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
	/// ���캯��
	Socket(SOCKET fd,uint32 sendbuffersize,uint32 recvbuffersize);
	/// ��������
	virtual ~Socket();

	/// ����ָ���Ļ���
	bool Connect(const char * Address,uint32 Port);
	/// �Ͽ�����
	void Disconnect(bool isDel=true);
	/// �����Ѿ����õ��ļ�������
	void Accept(sockaddr_in * address);
	
	/// �����ݴﵽʱ����
	virtual void OnRead(uint32 size) {}
	/// ��һ�����ӳɹ�����ʱ����
	virtual void OnConnect() {}
	/// ��һ�����ӶϿ�ʱ����
	virtual void OnDisconnect() {}

	/// �������ͻ�����,�������ݣ�����������
	bool Send(const uint8 * Bytes,uint32 Size);
	/// ��������
	bool Send(CMolMessageOut &out);
	/// ����ϵͳ - �������ͻ�����
	inline bool BurstBegin() { m_writeMutex.Acquire(); return true; }
	/// ����ϵͳ - ������ݵ����ͻ�����
	bool BurstSend(const uint8 * Bytes,uint32 Size);
	/// ����ϵͳ - ѹ���¼��������� �������д�¼�
	void BurstPush();
	/// ����ϵͳ - �������ͻ�����
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
	//������������
	inline void SetHeartCount(uint32 count) { m_heartJitter = count; }

	//�����������
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
	/// ��������
	void Encrypto(unsigned char *data,unsigned long length);
	/// ��������
	void Decrypto(unsigned char *data,unsigned long length);

protected:
	/// �����Ӵ�ʱ����
	void _OnConnect();

	SOCKET m_fd;

	Mutex m_writeMutex;
	Mutex m_readMutex;

	// �����Ƿ��Ѿ�����,���ӵĻ�ֹͣ�����¼�
	AtomicBoolean m_connected;

	// �����Ƿ�ɾ��,ɾ���Ļ�ֹͣ�����¼�
	AtomicBoolean m_deleted;

	sockaddr_in m_client; 

	MolCircularBuffer readBuffer;
	MolCircularBuffer writeBuffer;

	unsigned long m_BytesSent;
	unsigned long m_BytesRecieved;

	uint32					m_heartJitter;

private:
	// ����ϵͳ������IOCP�˿�
	HANDLE m_completionPort;

	// д������,��ֹ���д����ͬʱ����
	AtomicCounter m_writeLock;

	// ����socket��������ɶ˿�
	void AssignToCompletionPort();
};

class SocketGarbageCollector : public Singleton<SocketGarbageCollector>
{
public:
	/// ��������
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
	/// ���캯��
	NetClient(SOCKET fd);
	/// ��������
	~NetClient();

	/// �����ݴﵽʱ����
	virtual void OnRead(uint32 size);
	/// ��һ�����ӳɹ�����ʱ����
	virtual void OnConnect();
	/// ��һ�����ӶϿ�ʱ����
	virtual void OnDisconnect();

private:
	uint32 remaining;
	uint16 opcode;
};

class MolNetworkUpdate : public ThreadBase
{
public:
	/// ���캯��
	MolNetworkUpdate();
	/// ��������
	~MolNetworkUpdate();

	bool run();

private:
	DWORD m_curTime,m_TimeSpeed;
	DWORD m_threadTimer,m_threadTimeSpeed;
	DWORD m_UpdateTime,m_UpdateTimeSpeed;
};

extern MolNetworkUpdate *m_NetworkUpdate;

#endif