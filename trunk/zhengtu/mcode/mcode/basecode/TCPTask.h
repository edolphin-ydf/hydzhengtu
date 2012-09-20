/*
 �ļ��� : TCPTask.h
 ����ʱ�� : 2012/9/19
 ���� : hyd
 ���� : 
*/
#ifndef __TCPTask_H__
#define __TCPTask_H__

/**
* \brief ����һ�������࣬���̳߳صĹ�����Ԫ
*
*/
#include "Node.h"
#include "Processor.h"
#include "Thread.h"
#include "Mutex.h"
#include "Timer.h"
#include "Socket.h"
#include "CmdAnalysis.h"

#include <vector>

/**
* \brief �����������������
*
*/
class CTCPTask;
class CTCPTaskPool;
typedef std::vector<CTCPTask *> CTCPTaskContainer;
typedef CTCPTaskContainer::iterator CTCPTask_IT;

typedef std::vector<struct pollfd> pollfdContainer;

//TCP�������
class CTCPTaskQueue
{
public:
	CTCPTaskQueue() :_size(0) {}
	virtual ~CTCPTaskQueue() {}
	inline void add(CTCPTask *task)
	{
		mlock.lock();
		_queue.push_back(task);
		_size++;
		mlock.unlock();
	}
	inline void check_queue()
	{
		mlock.lock();
		while(!_queue.empty())
		{
			CTCPTask *task = _queue.back();
			_queue.pop_back();
			_add(task);
		}
		_size = 0;
		mlock.unlock();
	}
protected:
	virtual void _add(CTCPTask *task) = 0;
	DWORD _size;
private:
	Mutex mlock;
	std::vector<CTCPTask *> _queue;
};

/*
���ӻ����߳�,�����������õ�TCP����,�ͷ���Ӧ����Դ
*/

class RecycleThread : public CThread,public CTCPTaskQueue
{

private:

	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< �����б� */

	Mutex m_Lock;

	void _add(CTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	RecycleThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("RecycleThread"))
		: CThread(name),pool(pool)
	{}


	//��������
	~RecycleThread() {};

	void run();

};


/**
����TCP���ӵ���֤,�����֤��ͨ��,��Ҫ�����������
*/
class VerifyThread : public CThread,public CTCPTaskQueue
{

private:

	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< �����б� */
	CTCPTaskContainer::size_type task_count;      /**< tasks����(��֤�̰߳�ȫ*/

	Mutex m_Lock;
	/**
	* \brief ���һ����������
	* \param task ��������
	*/
	void _add(CTCPTask *task)
	{
		printf("zVerifyThread::_add");
		m_Lock.lock();
		tasks.push_back(task);
		task_count = tasks.size();
		m_Lock.unlock();
		printf("zVerifyThread::_add_end");
	}
public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	VerifyThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("VerifyThread"))
		: CThread(name),pool(pool)
	{
		task_count = 0;
	}

	/**
	* \brief ��������
	*
	*/
	~VerifyThread()
	{
	}

	void run();

};

/**
�ȴ������߳�ͬ����֤�������,���ʧ�ܻ��߳�ʱ,����Ҫ��������
*/
class SyncThread : public CThread,public CTCPTaskQueue
{

private:

	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< �����б� */

	Mutex m_Lock;
	void _add(CTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	SyncThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("zSyncThread"))
		: CThread(name),pool(pool)
	{}

	/**
	* \brief ��������
	*
	*/
	~SyncThread() {};

	void run();

};

/**
* \brief �����̳߳��࣬��װ��һ���̴߳��������ӵ��̳߳ؿ��
*
*/
class CTCPTaskPool : private CNode
{

public:

	/**
	* \brief ���캯��
	* \param maxConns �̳߳ز��д�����Ч���ӵ��������
	* \param state ��ʼ����ʱ�������̳߳ص�״̬
	*/
	explicit CTCPTaskPool(const int maxConns,const int state,const int us=50000) : maxConns(maxConns),state(state)/*,usleep_time(us)// */
	{
		setUsleepTime(us);
		syncThread = NULL;
		recycleThread = NULL;
		maxThreadCount = minThreadCount;
	};

	/**
	* \brief ��������������һ���̳߳ض���
	*
	*/
	~CTCPTaskPool()
	{
		final();
	}

	/**
	* \brief ��ȡ�����̳߳ص�ǰ״̬
	*
	* \return ���������̳߳صĵ�ǰ״̬
	*/
	const int getState() const
	{
		return state;
	}

	/**
	* \brief ���������̳߳�״̬
	*
	* \param state ���õ�״̬���λ
	*/
	void setState(const int state)
	{
		this->state |= state;
	}

	/**
	* \brief ��������̳߳�״̬
	*
	* \param state �����״̬���λ
	*/
	void clearState(const int state)
	{
		this->state &= ~state;
	}

	const int getSize();
	inline const int getMaxConns() const { return maxConns; }
	bool addVerify(CTCPTask *task);
	void addSync(CTCPTask *task);
	bool addOkay(CTCPTask *task);
	void addRecycle(CTCPTask *task);
	static void  setUsleepTime(int time)
	{
		usleep_time=time;
	}

	bool init();
	void final();

private:

	const int maxConns;                        /**< �̳߳ز��д������ӵ�������� */

	static const int maxVerifyThreads = 4;     /**< �����֤�߳����� */
	CThreadGroup verifyThreads;                /**< ��֤�̣߳������ж�� */

	SyncThread *syncThread;                    /**< �ȴ�ͬ���߳� */

	static const int minThreadCount = 1;       /**< �̳߳���ͬʱ�����������̵߳����ٸ��� */
	int maxThreadCount;                        /**< �̳߳���ͬʱ�����������̵߳������� */
	CThreadGroup okayThreads;                  /**< �������̣߳���� */

	RecycleThread *recycleThread;              /**< ���ӻ����߳� */

	int state;                                 /**< ���ӳ�״̬ */
public:
	static int usleep_time;                    /**< ѭ���ȴ�ʱ�� */

};

class CTCPTask : public Processor,private CNode
{

public:

	/**
	* \brief ���ӶϿ���ʽ
	*
	*/
	enum TerminateMethod
	{
		terminate_no,              /**< û�н������� */
		terminate_active,            /**< �ͻ��������Ͽ����ӣ���Ҫ�����ڷ������˼�⵽�׽ӿڹرջ����׽ӿ��쳣 */
		terminate_passive,            /**< �������������Ͽ����� */
	};

	/**
	* \brief ���캯�������ڴ���һ������
	*
	*
	* \param pool �������ӳ�ָ��
	* \param sock �׽ӿ�
	* \param addr ��ַ
	* \param compress �ײ����ݴ����Ƿ�֧��ѹ��
	* \param checkSignal �Ƿ���������·�����ź�
	*/
	CTCPTask(
		CTCPTaskPool *pool,
		const SOCKET sock,
		const struct sockaddr_in *addr = NULL,
		const bool compress = false,
		const bool checkSignal = true,
		const bool useIocp = USE_IOCP ) :pool(pool),lifeTime(),_checkSignal(checkSignal),_ten_min(600),tick(false)
	{
		terminate = terminate_no;
		terminate_wait = false; 
		fdsradd = false; 
		buffered = false;
		state = notuse;
		mSocket = NULL;
		mSocket = new CSocket( sock,addr,compress, useIocp,this );
		if( mSocket == NULL )
		{
			printf("new Socketʱ�ڴ治�㣡");
		}
	}

	/**
	* \brief ������������������һ������
	*
	*/
	virtual ~CTCPTask() 
	{
		if( mSocket != NULL )
		{
			if(mSocket->SafeDelete( false ))
				delete mSocket;
			mSocket = NULL;
		}
	}

	/**
	* \brief ���pollfd�ṹ
	* \param pfd �����Ľṹ
	* \param events �ȴ����¼�����
	*/
	void fillPollFD(struct mypollfd &pfd,short events)
	{
		mSocket->fillPollFD(pfd,events);
	}

	/**
	* \brief ����Ƿ���֤��ʱ
	*
	*
	* \param ct ��ǰϵͳʱ��
	* \param interval ��ʱʱ�䣬����
	* \return ����Ƿ�ɹ�
	*/
	bool checkVerifyTimeout(const RTime &ct,const unsigned long long interval = 5000) const
	{
		return (lifeTime.elapse(ct) > interval);
	}

	/**
	* \brief ����Ƿ��Ѿ�������¼�
	*
	* \return �Ƿ����
	*/
	bool isFdsrAdd()
	{
		return fdsradd;
	}
	/**
	* \brief ���ü�����¼���־
	*
	* \return �Ƿ����
	*/
	bool fdsrAdd()
	{
		fdsradd=true;
		return fdsradd;
	}

	virtual int verifyConn()
	{
		return 1;
	}

	/**
	* \brief �ȴ������߳�ͬ����֤������ӣ���Щ�̳߳ز���Ҫ�ⲽ�����Բ����������������ȱʡʼ�շ��سɹ�
	*
	* \return �ȴ��Ƿ�ɹ���1��ʾ�ɹ������Խ�����һ��������0����ʾ��Ҫ�����ȴ���-1��ʾ�ȴ�ʧ�ܻ��ߵȴ���ʱ����Ҫ�Ͽ�����
	*/
	virtual int waitSync()
	{
		return 1;
	}

	/**
	* \brief �����Ƿ�ɹ������ճɹ��Ժ���Ҫɾ�����TCP���������Դ
	*
	* \return �����Ƿ�ɹ���1��ʾ���ճɹ���0��ʾ���ղ��ɹ�
	*/
	virtual int recycleConn()
	{
		return 1;
	}

	/**
	* \brief һ������������֤�Ȳ�������Ժ���Ҫ��ӵ�ȫ��������
	*
	* ���ȫ���������ⲿ����
	*
	*/
	virtual void addToContainer() {}

	/**
	* \brief ���������˳���ʱ����Ҫ��ȫ��������ɾ��
	*
	* ���ȫ���������ⲿ����
	*
	*/
	virtual void removeFromContainer() {}

	/**
	* \brief ��ӵ��ⲿ���������������Ҫ��֤������ӵ�Ψһ��
	*
	* \return ����Ƿ�ɹ�
	*/
	virtual bool uniqueAdd()
	{
		return true;
	}

	/**
	* \brief ���ⲿ����ɾ�������������Ҫ��֤������ӵ�Ψһ��
	*
	* \return ɾ���Ƿ�ɹ�
	*/
	virtual bool uniqueRemove()
	{
		return true;
	}

	/**
	* \brief ����Ψһ����֤ͨ�����
	*
	*/
	void setUnique()
	{
		uniqueVerified = true;
	}

	/**
	* \brief �ж��Ƿ��Ѿ�ͨ����Ψһ����֤
	*
	* \return �Ƿ��Ѿ�ͨ����Ψһ�Ա��
	*/
	bool isUnique() const
	{
		return uniqueVerified;
	}

	/**
	* \brief �ж��Ƿ������߳�����Ϊ�ȴ��Ͽ�����״̬
	*
	* \return true or false
	*/
	bool isTerminateWait()
	{
		return terminate_wait; 
	}


	/**
	* \brief �ж��Ƿ������߳�����Ϊ�ȴ��Ͽ�����״̬
	*
	* \return true or false
	*/
	void TerminateWait()
	{
		terminate_wait=true; 
	}

	/**
	* \brief �ж��Ƿ���Ҫ�ر�����
	*
	* \return true or false
	*/
	bool isTerminate() const
	{
		return terminate_no != terminate;
	}

	/**
	* \brief ��Ҫ�����Ͽ��ͻ��˵�����
	*
	* \param method ���ӶϿ���ʽ
	*/
	virtual void Terminate(const TerminateMethod method = terminate_passive)
	{
		terminate = method;
	}

	virtual bool sendCmd(const void *,int);
	bool sendCmdNoPack(const void *,int);
	virtual bool ListeningRecv(bool);
	virtual bool ListeningSend();

	/**
	* \brief ��������״̬
	*
	*/
	enum CTCPTask_State
	{
		notuse    =  0,            /**< ���ӹر�״̬ */
		verify    =  1,            /**< ������֤״̬ */
		sync    =  2,            /**< �ȴ�������������������֤��Ϣͬ�� */
		okay    =  3,            /**< ���Ӵ���׶Σ���֤ͨ���ˣ�������ѭ�� */
		recycle    =  4              /**< �����˳�״̬������ */
	};

	/**
	* \brief ��ȡ��������ǰ״̬
	* \return ״̬
	*/
	const CTCPTask_State getState() const
	{
		return state;
	}

	/**
	* \brief ������������״̬
	* \param state ��Ҫ���õ�״̬
	*/
	void setState(const CTCPTask_State state)
	{
		this->state = state;
	}

	void getNextState();
	void resetState();

	/**
	* \brief ���״̬���ַ�������
	*
	*
	* \param state ״̬
	* \return ����״̬���ַ�������
	*/
	const char *getStateString(const CTCPTask_State state) const
	{
		const char *retval = NULL;

		switch(state)
		{
		case notuse:
			retval = "notuse";
			break;
		case verify:
			retval = "verify";
			break;
		case sync:
			retval = "sync";
			break;
		case okay:
			retval = "okay";
			break;
		case recycle:
			retval = "recycle";
			break;
		default:
			retval = "none";
			break;
		}

		return retval;
	}

	/**
	* \brief �������ӵ�IP��ַ
	* \return ���ӵ�IP��ַ
	*/
	const char *getIP() const
	{
		return mSocket->getIP();
	}
	const DWORD getAddr() const
	{
		return mSocket->getAddr();
	}

	const WORD getPort()
	{
		return mSocket->getPort();
	}

	int WaitRecv( bool bWait = false, int timeout = 0 )
	{
		return mSocket->WaitRecv( bWait, timeout );
	}

	int WaitSend( bool bWait = false, int timeout = 0 )
	{
		return mSocket->WaitSend( bWait, timeout );
	}

	bool UseIocp()
	{
		return mSocket->m_bUseIocp;
	}
	/**
	* \brief �Ƿ�������������·�����ź�
	* \return true or false
	*/
	const bool ifCheckSignal() const
	{
		return _checkSignal;
	}

	/**
	* \brief �������źŷ��ͼ��
	*
	* \return ����Ƿ�ɹ�
	*/
	bool checkInterval(const RTime &ct)
	{
		return _ten_min(ct);
	}

	/**
	* \brief �������źţ���������ź��ڹ涨ʱ���ڷ��أ���ô���·��Ͳ����źţ�û�з��صĻ�����TCP�����Ѿ�������
	*
	* \return true����ʾ���ɹ���false����ʾ���ʧ�� 
	*/
	bool checkTick() const
	{
		return tick;
	}

	/**
	* \brief �����ź��Ѿ�������
	*
	*/
	void clearTick()
	{
		tick = false;
	}

	/**
	* \brief ���Ͳ����źųɹ�
	*
	*/
	void setTick()
	{
		tick = true;
	}
	CTCPTaskPool *getPool()
	{
		return pool; 
	}

	void checkSignal(const RTime &ct);

	static CmdAnalysis analysis;
protected:

	bool buffered;                     /**< ����ָ���Ƿ񻺳� */
	//	zSocket mSocket;               /**< �ײ��׽ӿ� */
	CSocket* mSocket;                  // [ranqd] �޸�Ϊָ��

	CTCPTask_State state;              /**< ����״̬ */

private:

	CTCPTaskPool *pool;               /**< ���������ĳ� */
	TerminateMethod terminate;        /**< �Ƿ�������� */
	bool terminate_wait;              /**< �����߳����õȴ��Ͽ�����״̬,��pool�߳����öϿ�����״̬ */
	bool fdsradd;                     /**< ���¼���ӱ�־ */
	RTime lifeTime;                   /**< ���Ӵ���ʱ���¼ */

	bool uniqueVerified;              /**< �Ƿ�ͨ����Ψһ����֤ */
	const bool _checkSignal;          /**< �Ƿ�����·����ź� */
	Timer _ten_min;
	bool tick;

};

/**
* \brief TCP���ӵ��������߳�,һ��һ���̴߳�����TCP����,���������������Ч��
*
*/
class COkayThread : public CThread,public CTCPTaskQueue
{

private:

	Timer  _one_sec_; // �붨ʱ��
	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< �����б� */
	CTCPTaskContainer::size_type task_count;      /**< tasks����(��֤�̰߳�ȫ*/
	Mutex m_Lock;

	void _add(CTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		task_count = tasks.size();
		task->ListeningRecv(false);
		m_Lock.unlock();
	}

public:

	static const CTCPTaskContainer::size_type connPerThread = 512;  /**< ÿ���̴߳����������� */

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	COkayThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("COkayThread"))
		: CThread(name),pool(pool),_one_sec_(1)
	{
		task_count = 0;
	}

	/**
	��������
	*/
	~COkayThread()
	{
	}

	void run();

	/**
	* \brief ������������ĸ���
	* \return ����̴߳��������������
	*/
	const CTCPTaskContainer::size_type size() const
	{
		return task_count + _size;
	}

};


#endif
