/*
 文件名 : TCPTask.h
 创建时间 : 2012/9/19
 作者 : hyd
 功能 : 
*/
#ifndef __TCPTask_H__
#define __TCPTask_H__

/**
* \brief 定义一个任务类，是线程池的工作单元
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
* \brief 连接任务链表叠代器
*
*/
class CTCPTask;
class CTCPTaskPool;
typedef std::vector<CTCPTask *> CTCPTaskContainer;
typedef CTCPTaskContainer::iterator CTCPTask_IT;

typedef std::vector<struct pollfd> pollfdContainer;

//TCP任务队列
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
连接回收线程,回收所有无用的TCP连接,释放相应的资源
*/

class RecycleThread : public CThread,public CTCPTaskQueue
{

private:

	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< 任务列表 */

	Mutex m_Lock;

	void _add(CTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	RecycleThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("RecycleThread"))
		: CThread(name),pool(pool)
	{}


	//析构函数
	~RecycleThread() {};

	void run();

};


/**
处理TCP连接的验证,如果验证不通过,需要回收这个连接
*/
class VerifyThread : public CThread,public CTCPTaskQueue
{

private:

	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< 任务列表 */
	CTCPTaskContainer::size_type task_count;      /**< tasks计数(保证线程安全*/

	Mutex m_Lock;
	/**
	* \brief 添加一个连接任务
	* \param task 连接任务
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
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	VerifyThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("VerifyThread"))
		: CThread(name),pool(pool)
	{
		task_count = 0;
	}

	/**
	* \brief 析构函数
	*
	*/
	~VerifyThread()
	{
	}

	void run();

};

/**
等待其它线程同步验证这个连接,如果失败或者超时,都需要回收连接
*/
class SyncThread : public CThread,public CTCPTaskQueue
{

private:

	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< 任务列表 */

	Mutex m_Lock;
	void _add(CTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	SyncThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("zSyncThread"))
		: CThread(name),pool(pool)
	{}

	/**
	* \brief 析构函数
	*
	*/
	~SyncThread() {};

	void run();

};

/**
* \brief 连接线程池类，封装了一个线程处理多个连接的线程池框架
*
*/
class CTCPTaskPool : private CNode
{

public:

	/**
	* \brief 构造函数
	* \param maxConns 线程池并行处理有效连接的最大数量
	* \param state 初始化的时候连接线程池的状态
	*/
	explicit CTCPTaskPool(const int maxConns,const int state,const int us=50000) : maxConns(maxConns),state(state)/*,usleep_time(us)// */
	{
		setUsleepTime(us);
		syncThread = NULL;
		recycleThread = NULL;
		maxThreadCount = minThreadCount;
	};

	/**
	* \brief 析构函数，销毁一个线程池对象
	*
	*/
	~CTCPTaskPool()
	{
		final();
	}

	/**
	* \brief 获取连接线程池当前状态
	*
	* \return 返回连接线程池的当前状态
	*/
	const int getState() const
	{
		return state;
	}

	/**
	* \brief 设置连接线程池状态
	*
	* \param state 设置的状态标记位
	*/
	void setState(const int state)
	{
		this->state |= state;
	}

	/**
	* \brief 清楚连接线程池状态
	*
	* \param state 清楚的状态标记位
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

	const int maxConns;                        /**< 线程池并行处理连接的最大数量 */

	static const int maxVerifyThreads = 4;     /**< 最大验证线程数量 */
	CThreadGroup verifyThreads;                /**< 验证线程，可以有多个 */

	SyncThread *syncThread;                    /**< 等待同步线程 */

	static const int minThreadCount = 1;       /**< 线程池中同时存在主处理线程的最少个数 */
	int maxThreadCount;                        /**< 线程池中同时存在主处理线程的最大个数 */
	CThreadGroup okayThreads;                  /**< 处理主线程，多个 */

	RecycleThread *recycleThread;              /**< 连接回收线程 */

	int state;                                 /**< 连接池状态 */
public:
	static int usleep_time;                    /**< 循环等待时间 */

};

class CTCPTask : public Processor,private CNode
{

public:

	/**
	* \brief 连接断开方式
	*
	*/
	enum TerminateMethod
	{
		terminate_no,              /**< 没有结束任务 */
		terminate_active,            /**< 客户端主动断开连接，主要是由于服务器端检测到套接口关闭或者套接口异常 */
		terminate_passive,            /**< 服务器端主动断开连接 */
	};

	/**
	* \brief 构造函数，用于创建一个对象
	*
	*
	* \param pool 所属连接池指针
	* \param sock 套接口
	* \param addr 地址
	* \param compress 底层数据传输是否支持压缩
	* \param checkSignal 是否发送网络链路测试信号
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
			printf("new Socket时内存不足！");
		}
	}

	/**
	* \brief 析构函数，用于销毁一个对象
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
	* \brief 填充pollfd结构
	* \param pfd 待填充的结构
	* \param events 等待的事件参数
	*/
	void fillPollFD(struct mypollfd &pfd,short events)
	{
		mSocket->fillPollFD(pfd,events);
	}

	/**
	* \brief 检测是否验证超时
	*
	*
	* \param ct 当前系统时间
	* \param interval 超时时间，毫秒
	* \return 检测是否成功
	*/
	bool checkVerifyTimeout(const RTime &ct,const unsigned long long interval = 5000) const
	{
		return (lifeTime.elapse(ct) > interval);
	}

	/**
	* \brief 检查是否已经加入读事件
	*
	* \return 是否加入
	*/
	bool isFdsrAdd()
	{
		return fdsradd;
	}
	/**
	* \brief 设置加入读事件标志
	*
	* \return 是否加入
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
	* \brief 等待其它线程同步验证这个连接，有些线程池不需要这步，所以不用重载这个函数，缺省始终返回成功
	*
	* \return 等待是否成功，1表示成功，可以进入下一步操作，0，表示还要继续等待，-1表示等待失败或者等待超时，需要断开连接
	*/
	virtual int waitSync()
	{
		return 1;
	}

	/**
	* \brief 回收是否成功，回收成功以后，需要删除这个TCP连接相关资源
	*
	* \return 回收是否成功，1表示回收成功，0表示回收不成功
	*/
	virtual int recycleConn()
	{
		return 1;
	}

	/**
	* \brief 一个连接任务验证等步骤完成以后，需要添加到全局容器中
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void addToContainer() {}

	/**
	* \brief 连接任务退出的时候，需要从全局容器中删除
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void removeFromContainer() {}

	/**
	* \brief 添加到外部容器，这个容器需要保证这个连接的唯一性
	*
	* \return 添加是否成功
	*/
	virtual bool uniqueAdd()
	{
		return true;
	}

	/**
	* \brief 从外部容器删除，这个容器需要保证这个连接的唯一性
	*
	* \return 删除是否成功
	*/
	virtual bool uniqueRemove()
	{
		return true;
	}

	/**
	* \brief 设置唯一性验证通过标记
	*
	*/
	void setUnique()
	{
		uniqueVerified = true;
	}

	/**
	* \brief 判断是否已经通过了唯一性验证
	*
	* \return 是否已经通过了唯一性标记
	*/
	bool isUnique() const
	{
		return uniqueVerified;
	}

	/**
	* \brief 判断是否被其它线程设置为等待断开连接状态
	*
	* \return true or false
	*/
	bool isTerminateWait()
	{
		return terminate_wait; 
	}


	/**
	* \brief 判断是否被其它线程设置为等待断开连接状态
	*
	* \return true or false
	*/
	void TerminateWait()
	{
		terminate_wait=true; 
	}

	/**
	* \brief 判断是否需要关闭连接
	*
	* \return true or false
	*/
	bool isTerminate() const
	{
		return terminate_no != terminate;
	}

	/**
	* \brief 需要主动断开客户端的连接
	*
	* \param method 连接断开方式
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
	* \brief 连接任务状态
	*
	*/
	enum CTCPTask_State
	{
		notuse    =  0,            /**< 连接关闭状态 */
		verify    =  1,            /**< 连接验证状态 */
		sync    =  2,            /**< 等待来自其它服务器的验证信息同步 */
		okay    =  3,            /**< 连接处理阶段，验证通过了，进入主循环 */
		recycle    =  4              /**< 连接退出状态，回收 */
	};

	/**
	* \brief 获取连接任务当前状态
	* \return 状态
	*/
	const CTCPTask_State getState() const
	{
		return state;
	}

	/**
	* \brief 设置连接任务状态
	* \param state 需要设置的状态
	*/
	void setState(const CTCPTask_State state)
	{
		this->state = state;
	}

	void getNextState();
	void resetState();

	/**
	* \brief 获得状态的字符串描述
	*
	*
	* \param state 状态
	* \return 返回状态的字符串描述
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
	* \brief 返回连接的IP地址
	* \return 连接的IP地址
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
	* \brief 是否发送网络连接链路测试信号
	* \return true or false
	*/
	const bool ifCheckSignal() const
	{
		return _checkSignal;
	}

	/**
	* \brief 检测测试信号发送间隔
	*
	* \return 检测是否成功
	*/
	bool checkInterval(const RTime &ct)
	{
		return _ten_min(ct);
	}

	/**
	* \brief 检查测试信号，如果测试信号在规定时间内返回，那么重新发送测试信号，没有返回的话可能TCP连接已经出错了
	*
	* \return true，表示检测成功；false，表示检测失败 
	*/
	bool checkTick() const
	{
		return tick;
	}

	/**
	* \brief 测试信号已经返回了
	*
	*/
	void clearTick()
	{
		tick = false;
	}

	/**
	* \brief 发送测试信号成功
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

	bool buffered;                     /**< 发送指令是否缓冲 */
	//	zSocket mSocket;               /**< 底层套接口 */
	CSocket* mSocket;                  // [ranqd] 修改为指针

	CTCPTask_State state;              /**< 连接状态 */

private:

	CTCPTaskPool *pool;               /**< 任务所属的池 */
	TerminateMethod terminate;        /**< 是否结束任务 */
	bool terminate_wait;              /**< 其它线程设置等待断开连接状态,由pool线程设置断开连接状态 */
	bool fdsradd;                     /**< 读事件添加标志 */
	RTime lifeTime;                   /**< 连接创建时间记录 */

	bool uniqueVerified;              /**< 是否通过了唯一性验证 */
	const bool _checkSignal;          /**< 是否发送链路检测信号 */
	Timer _ten_min;
	bool tick;

};

/**
* \brief TCP连接的主处理线程,一般一个线程带几个TCP连接,这样可以显著提高效率
*
*/
class COkayThread : public CThread,public CTCPTaskQueue
{

private:

	Timer  _one_sec_; // 秒定时器
	CTCPTaskPool *pool;
	CTCPTaskContainer tasks;  /**< 任务列表 */
	CTCPTaskContainer::size_type task_count;      /**< tasks计数(保证线程安全*/
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

	static const CTCPTaskContainer::size_type connPerThread = 512;  /**< 每个线程带的连接数量 */

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	COkayThread(
		CTCPTaskPool *pool,
		const std::string &name = std::string("COkayThread"))
		: CThread(name),pool(pool),_one_sec_(1)
	{
		task_count = 0;
	}

	/**
	析构函数
	*/
	~COkayThread()
	{
	}

	void run();

	/**
	* \brief 返回连接任务的个数
	* \return 这个线程处理的连接任务数
	*/
	const CTCPTaskContainer::size_type size() const
	{
		return task_count + _size;
	}

};


#endif
