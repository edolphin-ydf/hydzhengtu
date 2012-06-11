#include "scriptTickTask.h"


scriptTask::scriptTask(const char* _funcName,time_t _elapse,SceneUser *_user):funcName(_funcName)
		,elapse(_elapse),user(_user),lastTime(time(NULL))
	{
		taskID = scriptTaskManagement::getInstance().generateID();
	}

bool scriptTask::doTask(time_t t)
{
	if(t >= elapse && user != NULL)
	{
		execute_script_event(user,funcName,p1);
		return true;
	}
	return false;
}


void scriptTaskManagement::execAll()
{
	zMutex_scope_lock scope_lock(mutex);
	scriptTaskManagement::iterator it = _tasklist.begin();
	scriptTaskManagement::iterator end = _tasklist.end();
	for( ; it != end; ++it)
	{
		time_t t = (time(NULL) - it->second->lastTime);
		if(it->second->doTask(t))
		{
			it->second->lastTime = time(NULL);
		}
	}		
}

scriptMessageFilter::cmds *scriptMessageFilter::_cmds = new scriptMessageFilter::cmds(scriptMessageFilter::maxcmds);