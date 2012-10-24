
#ifndef _MAPUPDATER_H
#define _MAPUPDATER_H

class MapMgr;
class Object;
class Player;
class WorldSession;
class Creature;
class GameObject;

#define MAPMGR_UPDATEOBJECT_LOOP_DELAY 100
#define MAPMGR_SESSION_UPDATE_DELAY	50

#define MAPMGR_UPDATE_DELAY			100

enum CThreadState
{
    THREADSTATE_TERMINATE = 0,
    THREADSTATE_PAUSED	= 1,
    THREADSTATE_SLEEPING  = 2,
    THREADSTATE_BUSY	  = 3,
    THREADSTATE_AWAITING  = 4,
};


struct NameTableEntry;

class SERVER_DECL CThread : public ThreadBase
{
	public:
		CThread(std::string);
		~CThread();

		MNET_INLINE void SetThreadState(CThreadState thread_state) { ThreadState.SetVal(thread_state); }
		MNET_INLINE CThreadState GetThreadState()
		{
			unsigned long val = ThreadState.GetVal();
			return static_cast<CThreadState>(val);
		}
		int GetThreadId() { return ThreadId; }
		time_t GetStartTime() { return start_time; }
		virtual bool run();
		virtual void OnShutdown();

	protected:
		CThread & operator=(CThread & other)
		{
			this->start_time = other.start_time;
			this->ThreadId = other.ThreadId;
			this->ThreadState.SetVal(other.ThreadState.GetVal());
			return *this;
		}

		MCodeNet::Threading::AtomicCounter ThreadState;
		time_t start_time;
		int ThreadId;
};

#endif
