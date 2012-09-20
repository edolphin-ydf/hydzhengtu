/*
 �ļ��� : Socket.h
 ����ʱ�� : 2012/9/18
 ���� : hyd
 ���� : ��װ�׽ӿڵײ㺯�����ṩһ���Ƚ�ͨ�õĽӿ�
*/
#ifndef __Socket_H__
#define __Socket_H__
#include "Common.h"
#include "Node.h"
#include "Mutex.h"
#include "Timer.h"

#include <vector>
#include <assert.h>

#define USE_IOCP true // [ranqd] �Ƿ�ʹ��IOCP�շ�����

#define MSG_NOSIGNAL    0
#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#define POLLIN  1       /* Set if data to read. */
#define POLLPRI 2       /* Set if urgent data to read. */
#define POLLOUT 4       /* Set if writing data wouldn't block. */

	class CSocket;

	struct mypollfd {
		int fd;
		short events;
		short revents;
		CSocket* pSock;
	};

	extern int poll(struct mypollfd *fds,unsigned int nfds,int timeout);
	extern int WaitRecvAll( struct mypollfd *fds,unsigned int nfds,int timeout );

#ifdef __cplusplus
}
#endif //__cplusplus

const DWORD trunkSize = 64 * 1024;
#define unzip_size(zip_size) ((zip_size) * 120 / 100 + 12)
const DWORD PACKET_ZIP_BUFFER  =  unzip_size(trunkSize - 1) + sizeof(DWORD) + 8;  /**< ѹ����Ҫ�Ļ��� */

/**
* �ֽڻ��壬�����׽ӿڽ��պͷ������ݵĻ���
* \param _type ��������������
*/
template <typename _type>
class ByteBuffer
{

public:
	Mutex m_Lock; // ������

	inline void Lock()
	{
		m_Lock.lock();
	}
	inline void UnLock()
	{
		m_Lock.unlock();
	}
	/**
	* ���캯��
	*/
	ByteBuffer();

	/**
	* �򻺳���������
	* \param buf �����뻺�������
	* \param size �����뻺�����ݵĳ���
	*/
	inline void put(const BYTE *buf,const DWORD size)
	{
		if( size == 1 )
		{
			int iii = 0;
		}
		//����ȷ�ϻ����ڴ��Ƿ��㹻
		wr_reserve(size);

		if( _maxSize - _currPtr < size )
		{
			MessageBox(NULL,"���������","���ش���",MB_ICONERROR);
		}

		memcpy_s(&_buffer[_currPtr],size,(void *)buf,size);
		_currPtr += size;
		if( _currPtr - _offPtr == 1 )
		{
			int iii = 0;
		}
	}

	/**
	* �õ���ǰ��дbf��δ֪
	* ��֤�ڵ��ô˺���д������֮ǰ��Ҫ����wr_reserve(size)��Ԥ����������С
	* \return ��д�뻺�忪ʼ��ַ
	*/
	inline BYTE *wr_buf()
	{
		return &_buffer[_currPtr];
	}

	/**
	* ���ػ�������Ч���ݵĿ�ʼ��ַ
	* \return ��Ч���ݵ�ַ
	*/
	inline BYTE *rd_buf()
	{
		return &_buffer[_offPtr];
	}

	/**
	* �жϻ�����ʱ������Ч����
	* \return ���ػ������Ƿ�����Ч����
	*/
	inline bool rd_ready()
	{
		bool ret = _currPtr > _offPtr;
		return ret;
	}

	/**
	* �õ���������Ч���ݵĴ�С
	* \return ���ػ�������Ч���ݴ�С
	*/
	inline DWORD rd_size()
	{
		DWORD ret = _currPtr - _offPtr;
		return ret;
	}

	/**
	* ���������Ч���ݱ�ʹ���Ժ���Ҫ�Ի����������
	* \param size ���һ��ʹ�õ���Ч���ݳ���
	*/
	inline void rd_flip(DWORD size)
	{	
		if( size == 0 ) return;
		_offPtr += size;
		if (_currPtr > _offPtr)
		{
			DWORD tmp = _currPtr - _offPtr;
			if (_offPtr >= tmp)
			{
				memmove(&_buffer[0],&_buffer[_offPtr],tmp);
				_offPtr = 0;
				_currPtr = tmp;
			}
		}
		else
		{
			_offPtr = 0;
			_currPtr = 0;
		}
		if( _currPtr - _offPtr == 1)
		{
			int iii = 0;
		}
	}

	/**
	* �õ������д�����ݵĴ�С
	* \return ��д�����ݵĴ�С
	*/
	inline DWORD wr_size()
	{
		DWORD ret = _maxSize - _currPtr;
		return ret;
	}

	/**
	* ʵ���򻺳�д�������ݣ���Ҫ�Ի����������
	* \param size ʵ��д�������
	*/
	inline void wr_flip(const DWORD size)
	{
		_currPtr += size;
	}

	/**
	* ��ֵ�����е����ݣ�������õ���������
	*/
	inline void reset()
	{
		_offPtr = 0;
		_currPtr = 0;
	}

	/**
	* ���ػ�������С
	* \return ��������С
	*/
	inline DWORD maxSize() const
	{
		return _maxSize;
	}

	/**
	* �Ի�����ڴ�������������򻺳�д���ݣ���������С���㣬���µ��������С��
	* ��С����ԭ����trunkSize����������������
	* \param size �򻺳�д���˶�������
	*/
	inline void wr_reserve(const DWORD size);

private:

	DWORD _maxSize;
	DWORD _offPtr;
	DWORD _currPtr;
	_type _buffer;

};
/**
* ��̬�ڴ�Ļ����������Զ�̬��չ��������С
*/
typedef ByteBuffer<std::vector<BYTE> > t_BufferCmdQueue;
/**
* ģ��ƫ�ػ�
* �Ի�����ڴ�������������򻺳�д���ݣ���������С���㣬���µ��������С��
* ��С����ԭ����trunkSize����������������
* \param size �򻺳�д���˶�������
*/
template <>
inline void t_BufferCmdQueue::wr_reserve(const DWORD size)
{
	if (wr_size() < size)
	{
#define trunkCount(size) (((size) + trunkSize - 1) / trunkSize)
		_maxSize += (trunkSize * trunkCount(size));
		_buffer.resize(_maxSize);
	}
}
/**
* ��̬��С�Ļ���������ջ�ռ�����ķ�ʽ�������ڴ棬����һЩ��ʱ�����Ļ�ȡ
*/
typedef ByteBuffer<BYTE [PACKET_ZIP_BUFFER]> t_StackCmdQueue;

/**
* ģ��ƫ�ػ�
* �Ի�����ڴ�������������򻺳�д���ݣ���������С���㣬���µ��������С��
* ��С����ԭ����trunkSize����������������
* \param size �򻺳�д���˶�������
*/
template <>
inline void t_StackCmdQueue::wr_reserve(const DWORD size)
{
	/*
	if (wr_size() < size)
	{
	//���ܶ�̬��չ�ڴ�
	assert(false);
	}
	// */
}


// [ranqd] IO����״̬��־
typedef   enum   enum_IOOperationType   
{     
	IO_Write,     // д
	IO_Read		  // ��

}IOOperationType,   *LPIOOperationType;
// [ranqd] �Զ���IO�����ṹ��ָ����������
typedef   struct   st_OverlappedData   
{   
	OVERLAPPED Overlapped;
	IOOperationType OperationType;

	st_OverlappedData( enum_IOOperationType type )
	{
		ZeroMemory( &Overlapped, sizeof(OVERLAPPED) );
		OperationType = type;
	}

}OverlappedData,   *LPOverlappedData;

class CTCPTask;
class CSocket : private CNode
{

public:
	bool				m_bUseIocp;     // [ranqd]  �Ƿ�ʹ��IOCP�շ�����   

	DWORD               m_SendSize;     // [ranqd] ��¼ϣ�����������ܳ���
	DWORD               m_LastSend;     // [ranqd] ��¼�������������ݳ���
	DWORD               m_LastSended;   // [ranqd] �ѷ������������ݳ���

	static const int T_RD_MSEC          =  2100;          /**< ��ȡ��ʱ�ĺ����� */
	static const int T_WR_MSEC          =  2100;          /**< ���ͳ�ʱ�ĺ����� */

	static const DWORD PH_LEN       =  sizeof(DWORD);     /**< ���ݰ���ͷ��С */
	static const DWORD PACKET_ZIP_MIN  =  32;             /**< ���ݰ�ѹ����С��С */

	static const DWORD PACKET_ZIP    =  0x40000000;       /**< ���ݰ�ѹ����־ */
	static const DWORD INCOMPLETE_READ  =  0x00000001;    /**< �ϴζ��׽ӿڽ��ж�ȡ����û�ж�ȡ��ȫ�ı�־ */
	static const DWORD INCOMPLETE_WRITE  =  0x00000002;   /**< �ϴζ��׽ӿڽ���д�����ú��д����ϵı�־ */

	static const DWORD PACKET_MASK      =  trunkSize - 1; /**< ������ݰ��������� */
	static const DWORD MAX_DATABUFFERSIZE  =  PACKET_MASK;  /**< ���ݰ���󳤶ȣ�������ͷ4�ֽ� */
	static const DWORD MAX_DATASIZE      =  (MAX_DATABUFFERSIZE - PH_LEN - PACKHEADLASTSIZE);    /**< ���ݰ���󳤶� */
	static const DWORD MAX_USERDATASIZE    =  (MAX_DATASIZE - 128);                              /**< �û����ݰ���󳤶� */

	static const char *getIPByIfName(const char *ifName);

	CSocket(const SOCKET sock,const struct sockaddr_in *addr = NULL,const bool compress = false, const bool useIocp = USE_IOCP,CTCPTask* pTask = NULL );
	~CSocket();

	int recvToCmd(void *pstrCmd,const int nCmdLen,const bool wait);
	bool sendCmd(const void *pstrCmd,const int nCmdLen,const bool buffer = false);
	bool sendCmdNoPack(const void *pstrCmd,const int nCmdLen,const bool buffer = false);
	int  Send(const SOCKET sock, const void* pBuffer, const int nLen,int flags);
	bool sync();
	void force_sync();

	int checkIOForRead();
	int checkIOForWrite();
	int recvToBuf_NoPoll();
	int recvToCmd_NoPoll(void *pstrCmd,const int nCmdLen);

	/**
	* \brief ��ȡ�׽ӿڶԷ��ĵ�ַ
	* \return IP��ַ
	*/
	inline const char *getIP() const { return inet_ntoa(addr.sin_addr); }
	inline const DWORD getAddr() const { return addr.sin_addr.s_addr; }

	/**
	* \brief ��ȡ�׽ӿڶԷ��˿�
	* \return �˿�
	*/
	inline const WORD getPort() const { return ntohs(addr.sin_port); }

	/**
	* \brief ��ȡ�׽ӿڱ��صĵ�ַ
	* \return IP��ַ
	*/
	inline const char *getLocalIP() const { return inet_ntoa(local_addr.sin_addr); }

	/**
	* \brief ��ȡ�׽ӿڱ��ض˿�
	* \return �˿�
	*/
	inline const WORD getLocalPort() const { return ntohs(local_addr.sin_port); }

	/**
	* \brief ���ö�ȡ��ʱ
	* \param msec ��ʱ����λ���� 
	* \return 
	*/
	inline void setReadTimeout(const int msec) { rd_msec = msec; }

	/**
	* \brief ����д�볬ʱ
	* \param msec ��ʱ����λ���� 
	* \return 
	*/
	inline void setWriteTimeout(const int msec) { wr_msec = msec; }


	/**
	* \brief ���pollfd�ṹ
	* \param pfd �����Ľṹ
	* \param events �ȴ����¼�����
	*/
	inline void fillPollFD(struct mypollfd &pfd,short events)
	{
		pfd.fd = sock;
		pfd.events = events;
		pfd.revents = 0;
		pfd.pSock = this;
	}

	//inline void setEncMethod(CEncrypt::encMethod m) { enc.setEncMethod(m); }
	//inline void set_key_rc5(const BYTE *data,int nLen,int rounds) { enc.set_key_rc5(data,nLen,rounds); }
	//inline void set_key_des(const_ZES_cblock *key) { enc.set_key_des(key); }
	inline DWORD snd_queue_size() { return _snd_queue.rd_size() + _enc_queue.rd_size(); }

	inline DWORD getBufferSize() const {return _rcv_queue.maxSize() + _snd_queue.maxSize();}

	inline CTCPTask*& GetpTask() {return pTask;} // [ranqd] ����Taskָ��

	void ReadByte( DWORD size ); // [ranqd] �����ȡ�����ֽ�
	void RecvData( DWORD dwNum = 0 ); // [ranqd] ͨ��Iocp��ȡ����

	int SendData( DWORD dwNum ); // [ranqd] ͨ��Iocp��������

	int WaitRecv( bool bWait, int timeout = 0 ) ;  // [ranqd] �ȴ����ݽ���

	int WaitSend( bool bWait, int timeout = 0 ); // [ranqd] �ȴ����ݷ���

	void DisConnet()
	{
		::shutdown(sock,0x02);
		::closesocket(sock);
		sock = INVALID_SOCKET;
	}

	Mutex m_Lock;

	bool m_bIocpDeleted;   // iocp�Ƿ��ͷ�
	bool m_bTaskDeleted;   // task�Ƿ��ͷ�

	bool m_bDeleted;       // �ͷű�־
	bool SafeDelete( bool bFromIocp ) // [ranqd] ��ȫ�ͷű���
	{
		m_Lock.lock();
		if( bFromIocp )
		{
			m_bIocpDeleted = true;
		}
		else
		{
			m_bTaskDeleted = true;
		}
		if( !m_bIocpDeleted || !m_bTaskDeleted )
		{			
			DisConnet();
			m_Lock.unlock();
			return false;
		}
		if( m_bDeleted )
		{
			m_Lock.unlock();
			return false;
		}
		m_bDeleted = true;
		m_Lock.unlock();
		printf( "�ͷ����� %0.8X", pTask );
		return true;
	}
private:
	SOCKET sock;                        /**< �׽ӿ� */
	struct sockaddr_in addr;            /**< �׽ӿڵ�ַ */
	struct sockaddr_in local_addr;      /**< �׽ӿڵ�ַ */
	int rd_msec;                        /**< ��ȡ��ʱ������ */
	int wr_msec;                        /**< д�볬ʱ������ */

	Mutex m_RecvLock;                   // [ranqd]  ���ջ����߳���
	t_BufferCmdQueue    m_RecvBuffer;   // [ranqd]  Iocp�������ݻ���
	OverlappedData		m_ovIn;         // [ranqd]  Io��״̬��¼
	OverlappedData      m_ovOut;        // [ranqd]  Ioд״̬��¼
	//	HANDLE              m_hRecvEvent;   // [ranqd]  ���ݽ����¼�

	DWORD               m_dwMySendCount;// [ranqd]  ͨ����װ���͵������ܳ���
	DWORD               m_dwSendCount;  // [ranqd]  ʵ�ʷ��������ݳ���
	DWORD               m_dwRecvCount;  // [ranqd]  ���������ݳ���

	CTCPTask*           pTask;          // [ranqd]  ��Sock��Ӧ��Taskָ��

	t_BufferCmdQueue tQueue;             // [ranqd]  ����������

	t_BufferCmdQueue _rcv_queue;        /**< ���ջ���ָ����� */
	DWORD _rcv_raw_size;                /**< ���ջ���������ݴ�С */
	t_BufferCmdQueue _snd_queue;        /**< ���ܻ���ָ����� */
	t_BufferCmdQueue _enc_queue;        /**< ���ܻ���ָ����� */
	DWORD _current_cmd;
	Mutex mutex;                        /**< �� */

	RTm last_check_time;                /**< ���һ�μ��ʱ�� */

	DWORD bitmask;                      /**< ��־���� */
	//CEncrypt enc;                       /**< ���ܷ�ʽ */

	inline void set_flag(DWORD _f) { bitmask |= _f; }
	inline bool isset_flag(DWORD _f) const { return bitmask & _f; }
	inline void clear_flag(DWORD _f) { bitmask &= ~_f; }
	//inline bool need_enc() const { return CEncrypt::ENCDEC_NONE!=enc.getEncMethod(); }
	/**
	* \brief �������ݰ���ͷ��С����
	* \return ��С����
	*/
	inline DWORD packetMinSize() const { return PH_LEN; }

	/**
	* \brief �����������ݰ��ĳ���
	* \param in ���ݰ�
	* \return �����������ݰ��ĳ���
	*/
	inline DWORD packetSize(const BYTE *in) const { return PH_LEN + ((*((DWORD *)in)) & PACKET_MASK); }

	inline int sendRawData(const void *pBuffer,const int nSize);
	inline bool sendRawDataIM(const void *pBuffer,const int nSize);
	inline int sendRawData_NoPoll(const void *pBuffer,const int nSize);
	inline bool setNonblock();
	inline int waitForRead();
	inline int waitForWrite();
	inline int recvToBuf();

	inline DWORD packetUnpack(BYTE *in,const DWORD nPacketLen,BYTE *out);
	template<typename buffer_type>
	inline DWORD packetAppend(const void *pData,const DWORD nLen,buffer_type &cmd_queue);
public:
	template<typename buffer_type>
	static inline DWORD packetPackZip(const void *pData,const DWORD nLen,buffer_type &cmd_queue,const bool _compress = true);

};
#endif
