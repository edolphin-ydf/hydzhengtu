#include "shared/CallBack.h"

#ifndef WIN32
static pthread_cond_t abortcond;
static pthread_mutex_t abortmutex;
#endif

template<class Type>
class PeriodicFunctionCaller : public ThreadBase
{
	public:
		template<class T>
		PeriodicFunctionCaller(T* callback, void (T::*method)(), uint32 Interval,const char* name)
			:ThreadBase(name)
		{
			cb = new CallbackP0<T>(callback, method);
			interval = Interval;
			running.SetVal(true);
		}

		~PeriodicFunctionCaller()
		{
			delete cb;
		}

		bool run()
		{
#ifndef WIN32
			struct timeval now;
			struct timespec tv;
			uint32 next = getMSTime() + interval;

			pthread_mutex_init(&abortmutex, NULL);
			pthread_cond_init(&abortcond, NULL);

			while(running.GetVal() && mrunning.GetVal())
			{
				if(getMSTime() > next)
				{
					cb->execute();
					next = getMSTime() + interval;
				}
				gettimeofday(&now, NULL);
				tv.tv_sec = now.tv_sec + 120;
				tv.tv_nsec = now.tv_usec * 1000;
				pthread_mutex_lock(&abortmutex);
				pthread_cond_timedwait(&abortcond, &abortmutex, &tv);
				pthread_mutex_unlock(&abortmutex);
			}
#else
			thread_active = true;
			hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			for(;;)
			{
				if(hEvent)
					WaitForSingleObject(hEvent, interval);

				if(!running.GetVal())
					break;	/* we got killed */

				/* times up */
				if(hEvent)
					ResetEvent(hEvent);
				cb->execute();
			}
			thread_active = false;
#endif
			return false;
		}

		void kill()
		{
			running.SetVal(false);
#ifdef WIN32
			/* push the event */
			SetEvent(hEvent);
			LOG_DETAIL("Waiting for PFC thread to exit...");
			/* wait for the thread to exit */
			while(thread_active)
			{
				MNet::Sleep(100);
			}
			LOG_DETAIL("PFC thread exited.");
#else
			pthread_cond_signal(&abortcond);
#endif
		}

	private:
		CallbackBase* cb;
		uint32 interval;
		MNet::Threading::AtomicBoolean running;
#ifdef WIN32
		bool thread_active;
		HANDLE hEvent;
#endif
};

