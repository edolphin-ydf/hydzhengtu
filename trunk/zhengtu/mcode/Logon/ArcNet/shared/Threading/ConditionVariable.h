
#ifndef CONDITIONVARIABLE_H
#define CONDITIONVARIABLE_H

namespace MNet
{

	namespace Threading
	{

		/////////////////////////////////////////////////////////////////
		//class ConditionVariable
		//  Class implementing a platform independent condition variable
		//
		//
		/////////////////////////////////////////////////////////////////
		class ConditionVariable
		{

			public:
				ConditionVariable();

				~ConditionVariable();

				//////////////////////////////////////////////////////////////////////////
				//void Signal()
				//  Signals the condition variable, allowing the blocked thread to proceed
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
				//  Blocks execution of the calling thread until signaled or
				//  until the timer runs out.
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
