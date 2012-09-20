/*
 �ļ��� : Thread.h
 ����ʱ�� : 2012/9/18
 ���� : hyd
 ���� : 
*/
#ifndef __Thread_H__
#define __Thread_H__
#include "Common.h"
#include "Node.h"
#include "Mutex.h"
#include <vector>
/**
* \brief ��װ���̲߳���������ʹ���̵߳Ļ���
*
*/
class CThread : private CNode
{
public:

	/**
	* \brief ���캯��������һ������
	*
	* \param name �߳�����
	* \param joinable ��������߳��˳���ʱ���Ƿ񱣴�״̬�����Ϊtrue��ʾ�߳��˳�����״̬�����򽫲������˳�״̬
	*/
	CThread(const std::string &name = std::string("CThread"),const bool joinable = true) 
		: threadName(name),alive(false),complete(false),joinable(joinable) { m_hThread = NULL; };

	/**
	* \brief ������������������һ�����󣬻��ն���ռ�
	*
	*/
	virtual ~CThread()
	{
		if (NULL != m_hThread)
		{
			CloseHandle(m_hThread);
		}
	};

	/**
	* \brief ʹ��ǰ�߳�˯��ָ����ʱ�䣬��
	*
	*
	* \param sec ָ����ʱ�䣬��
	*/
	static void sleep(const long sec)
	{
		::Sleep(1000 * sec);
	}

	/**
	* \brief ʹ��ǰ�߳�˯��ָ����ʱ�䣬����
	*
	*
	* \param msec ָ����ʱ�䣬����
	*/
	static void msleep(const long msec)
	{
		::Sleep(msec);
	}

	/**
	* \brief ʹ��ǰ�߳�˯��ָ����ʱ�䣬΢��
	*
	*
	* \param usec ָ����ʱ�䣬΢��
	*/
	static void usleep(const long usec)
	{
		::Sleep(usec / 1000);
	}

	/**
	* \brief �߳��Ƿ���joinable��
	*
	*
	* \return joinable
	*/
	const bool isJoinable() const
	{
		return joinable;
	}

	/**
	* \brief ����߳��Ƿ�������״̬
	*
	* \return �߳��Ƿ�������״̬
	*/
	const bool isAlive() const
	{
		return alive;
	}

	static DWORD WINAPI threadFunc(void *arg);
	bool start();
	void join();

	/**
	* \brief ���������߳�
	*
	* ��ʵֻ�����ñ�ǣ���ô�̵߳�run���ص�ѭ���ؼ�������ǣ�����������Ѿ����ã����˳�ѭ��
	*
	*/
	void final()
	{
		complete = true;
	}

	/**
	* \brief �ж��߳��Ƿ����������ȥ
	*
	* ��Ҫ����run()����ѭ���У��ж�ѭ���Ƿ����ִ����ȥ
	*
	* \return �߳����ص��Ƿ����ִ��
	*/
	const bool isFinal() const 
	{
		return complete;
	}

	/**
	* \brief ���鹹�������߳����ص�������ÿ����Ҫʵ��������������Ҫ�����������
	*
	* ���������ѭ����Ҫ��ÿ��ѭ������߳��˳����isFinal()�������ܹ���֤�̰߳�ȫ�˳�
	* <pre>
	*   while(!isFinal())
	*   {
	*     ...
	*   }
	*   </pre>
	*
	*/
	virtual void run() = 0;

	/**
	* \brief �ж������߳��Ƿ���ͬһ���߳�
	* \param other ���Ƚϵ��߳�
	* \return �Ƿ���ͬһ���߳�
	*/
	//bool operator==(const CThread& other) const
	//{
	// return pthread_equal(thread,other.thread) != 0;
	//}

	/**
	* \brief �ж������߳��Ƿ���ͬһ���߳�
	* \param other ���Ƚϵ��߳�
	* \return �Ƿ���ͬһ���߳�
	*/
	//bool operator!=(const CThread& other) const
	//{
	//return !operator==(other);
	//}

	/**
	* \brief �����߳�����
	*
	* \return �߳�����
	*/
	const std::string &getThreadName() const
	{
		return threadName;
	}

public:

	std::string threadName;      /**< �߳����� */
	Mutex mlock;                /**< ������ */
	volatile bool alive;         /**< �߳��Ƿ������� */
	volatile bool complete;
	HANDLE m_hThread;            /**< �̱߳�� */
	bool joinable;               /**< �߳����ԣ��Ƿ�����joinable��� */

}; 

/**
* \brief ���߳̽��з���������
*
*/
class CThreadGroup : private CNode
{

public:

	struct Callback
	{
		virtual void exec(CThread *e)=0;
		virtual ~Callback(){};
	};

	typedef std::vector<CThread *> Container;  /**< �������� */

	CThreadGroup();
	~CThreadGroup();
	void add(CThread *thread);
	CThread *getByIndex(const Container::size_type index);
	CThread *operator[] (const Container::size_type index);
	void joinAll();
	void execAll(Callback &cb);

	const Container::size_type size()
	{
		RWLock_scope_rdlock scope_rdlock(rwlock);
		return vts.size();
	}

private:

	Container vts;                /**< �߳����� */
	RWLock rwlock;                /**< ��д�� */

};

#endif


