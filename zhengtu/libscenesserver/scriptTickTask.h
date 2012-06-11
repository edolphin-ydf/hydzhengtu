#ifndef _SCRIPTTICKTASK_H
#define _SCRIPTTICKTASK_H

#include <zebra/ScenesServer.h>
#include <map>
#include <zebra/csBox.h>
//#include ""
struct scriptTask
{
	SceneUser *user;
	time_t lastTime;
	const char* funcName;
	time_t elapse;
	QWORD taskID;
	int p1;

	scriptTask(const char* _funcName,time_t _elapse,SceneUser *_user);

	bool doTask(time_t t); 

};

class scriptTaskManagement : public SingletonBase<scriptTaskManagement>
{

friend class SingletonBase<scriptTaskManagement>;
public:

	bool addTask(scriptTask *_task)
	{
		//fprintf(stderr,"添加了一个任务\n");
		zMutex_scope_lock scope_lock(mutex);
		
		iterator it = _tasklist.find((DWORD)(_task->taskID));
		if(it != _tasklist.end())
			return false;
		
		_tasklist[(DWORD)(_task->taskID)] = _task;
		return true;
	}

	bool deleteTask(QWORD taskID)
	{
		//fprintf(stderr,"删除了一个任务\n");
		zMutex_scope_lock scope_lock(mutex);
		iterator it = _tasklist.find(taskID);
		if(it == _tasklist.end())
			return false;
		
		delete it->second;
		_tasklist.erase(it);

		
	}

	QWORD generateID()
	{
		zMutex_scope_lock scope_lock(mutex);
		return id++;
	}

	void execAll();

protected:
	scriptTaskManagement():id(1){ }
	~scriptTaskManagement(){}

public:
	typedef std::map<QWORD,scriptTask*>::iterator iterator;

private:
	std::map<QWORD,scriptTask*> _tasklist;
	zMutex mutex;
	QWORD id;
};





class userScriptTaskContainer
{
public:
	
	userScriptTaskContainer()
	{
		memset(tasks,0,sizeof(QWORD)*taskType::end);
	}

	~userScriptTaskContainer()
	{
		for(int i = 0; i < (int)taskType::end; ++i)
		{
			if(tasks[i] != 0)
			{
				scriptTaskManagement::getInstance().deleteTask(tasks[i]);
			}
		}
	}

	bool add(taskType _type,scriptTask *_task)
	{
		if(_task == NULL)
			return false;
		if(tasks[_type] != 0)
			return false;
		
		if(scriptTaskManagement::getInstance().addTask(_task))
		{
			tasks[_type] = _task->taskID;
			return true;
		}

		return false;

	}

	bool del(taskType _type)
	{
		if(tasks[_type] == 0)
			return false;
		if(scriptTaskManagement::getInstance().deleteTask(tasks[_type]))
		{
			tasks[_type] = 0;
			return true;
		}
		return false;
	}

private:

	QWORD tasks[taskType::end];

};


class scriptMessageFilter
{
	public:

		static void initFilter()
		{
			//isInit = true;
			for(int i = 0; i < maxcmds; ++i)
				(*_cmds)[i] = NULL;
			execute_script_event("scriptCmd");
		}


		static void reinitFilter()
		{
			for(int i = 0; i < maxcmds; ++i)
			{
				delete (*_cmds)[i];
				(*_cmds)[i] = NULL;
			}
			execute_script_event("scriptCmd");
		}


		static bool exeScript(SceneUser *user,Cmd::t_NullCmd *pNullCmd)/*BYTE cmd,BYTE para*/
		{
			
			BYTE cmd = pNullCmd->cmd;
			BYTE para = pNullCmd->para;


			//int *a;

			//Cmd::stReMakObjectUserCmd *temCmd = (Cmd::stReMakObjectUserCmd*)pNullCmd;
			
			if(user != NULL && cmd <= maxcmds - 1 && para <= maxparas - 1)
			{
				
				//if(cmd == 52 && para == 1)
				//{
				//execute_script_event(user,"reMakeObj",temCmd);//);
				//	return true;
				//}
				if((*_cmds)[cmd] != NULL && (*(*_cmds)[cmd])[para] != NULL) 
				{
					execute_script_event(user,(*(*_cmds)[cmd])[para],pNullCmd);
					return true;
				}
			}

			return false;

		}

		static void add(BYTE cmd,BYTE para,const char *func)
		{
			if(func != NULL && cmd <= maxcmds - 1 && para <= maxparas - 1)
			{
				if((*_cmds)[cmd] == NULL)
				{
					(*_cmds)[cmd] = new paras(maxparas);
					for( int i = 0; i < maxparas; ++i)
						(*(*_cmds)[cmd])[i] = NULL;
				}
				
				(*(*_cmds)[cmd])[para] = func;

				//fprintf(stderr,"添加脚本处理函数cmd=%u,para=%u,funcname = %s",cmd,para,func);

			}
		}

		
		

private:
	//typedef std::map<std::string,std::string> funs;
	//typedef std::map<std::string,std::string>::iterator funs_iterator;

	static const unsigned int maxcmds = 100;
	static const unsigned int maxparas = 200;

	typedef std::vector<const char *> paras;
	typedef std::vector<paras*> cmds;

	static cmds *_cmds;
	//static bool isInit;



};

//bool scriptMessageFilter::isInit = false;



#endif