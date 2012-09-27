
#ifndef _THREADING_STARTER_H
#define _THREADING_STARTER_H

class SERVER_DECL ThreadBase
{
	public:
		ThreadBase(std::string name) { m_name=name;}
		virtual ~ThreadBase() {}
		virtual bool run() = 0;
		virtual void OnShutdown() {}
#ifdef WIN32
		HANDLE THREAD_HANDLE;
#else
		pthread_t THREAD_HANDLE;
#endif
private:
	std::string m_name;
};

#endif

