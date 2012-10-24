
#ifndef CONDITIONVARIABLE_H
#define CONDITIONVARIABLE_H
////////////////////////////////////////////////////////////////
/// @namespace MCodeNet
/// @brief M网络库
namespace MCodeNet
{

	namespace Threading
	{

		/////////////////////////////////////////////////////////////////
		//class ConditionVariable
		//  类实现一个平台独立的条件变量
		//
		//
		/////////////////////////////////////////////////////////////////
		class SERVER_DECL ConditionVariable
		{

			public:
				ConditionVariable();

				~ConditionVariable();

				//////////////////////////////////////////////////////////////////////////
				//void Signal()
				//  信号条件变量,允许阻塞的线程继续下去。
				//
				//Parameter(s)
				//  None
				//
				//Return Value
				//  None
				//
				//
				//////////////////////////////////////////////////////////////////////////
				void Signal();


				/////////////////////////////////////////////////////////////////////////
				//void Wait( unsigned long timems )
				//  阻塞执行调用线程,直到有信号或超时。
				//  
				//
				//Parameter(s)
				//  unsigned long timems  -  Maximum time to block in milliseconds
				//
				//Return Value(s)
				//  None
				//
				//
				/////////////////////////////////////////////////////////////////////////
				void Wait(unsigned long timems);

			private:

#ifdef WIN32
				HANDLE hEvent;
#else
				pthread_cond_t cond;
				pthread_mutex_t mutex;
#endif
		};
	}
}

#endif
