/*
 文件名 : MessageQueue.h
 创建时间 : 2012/9/19
 作者 : hyd
 功能 : 
*/
#ifndef __MessageQueue_H__
#define __MessageQueue_H__

#include "Common.h"
#include <queue>
template <class T>
class __mt_alloc
{
	T memPool[2046];
public:
	char * allocate(size_t  len){return (char*)malloc(len);}

	void deallocate(unsigned char* ptr,size_t len)
	{
		free(ptr);
	}


};

typedef std::pair<DWORD,BYTE *> CmdPair;

template <int QueueSize=102400>
class MsgQueue
{
public:
	MsgQueue()
	{
		queueRead=0;
		queueWrite=0;
	}
	~MsgQueue()
	{
		clear();
	}
	typedef std::pair<volatile bool,CmdPair > CmdQueue;
	CmdPair *get()
	{
		CmdPair *ret=NULL;
		if (cmdQueue[queueRead].first)
		{
			ret=&cmdQueue[queueRead].second;
		}
		return ret;
	}
	void erase()
	{
		//SAFE_DELETE_VEC(cmdQueue[queueRead].second.second);
		__mt_alloc.deallocate(cmdQueue[queueRead].second.second,cmdQueue[queueRead].second.first);
		cmdQueue[queueRead].first=false;
		queueRead = (++queueRead)%QueueSize;
	}
	bool put(const void *pNullCmd,const DWORD cmdLen)
	{
		//BYTE *buf = new BYTE[cmdLen];
		BYTE *buf = (BYTE*)__mt_alloc.allocate(cmdLen);
		if (buf)
		{
			memcpy_s(buf,cmdLen,(void *)pNullCmd,cmdLen);
			if (!putQueueToArray() && !cmdQueue[queueWrite].first)
			{
				cmdQueue[queueWrite].second.first = cmdLen;
				cmdQueue[queueWrite].second.second = buf;
				cmdQueue[queueWrite].first=true;
				queueWrite = (++queueWrite)%QueueSize;
				return true;
			}
			else
			{
				queueCmd.push(std::make_pair(cmdLen,buf));
			}
			return true;
		}
		return false;

	}
private:
	void clear()
	{
		while(putQueueToArray())
		{
			while(get())
			{
				erase();
			}
		}
		while(get())
		{
			erase();
		}
	}
	bool putQueueToArray()
	{
		bool isLeft=false;
		while(!queueCmd.empty())
		{
			if (!cmdQueue[queueWrite].first)
			{
				cmdQueue[queueWrite].second = queueCmd.front();;
				cmdQueue[queueWrite].first=true;
				queueWrite = (++queueWrite)%QueueSize;
				queueCmd.pop();
			}
			else
			{
				isLeft = true; 
				break;
			}
		}
		return isLeft;
	}
	__mt_alloc<BYTE> __mt_alloc;
	CmdQueue cmdQueue[QueueSize];
	std::queue<CmdPair> queueCmd;
	DWORD queueWrite;
	DWORD queueRead;
};

class MessageQueue
{
protected:
	virtual ~MessageQueue(){};
public:
	bool msgParse(const Cmd::Cmd_NULL *pNullCmd,const DWORD cmdLen)
	{
		return cmdQueue.put((void*)pNullCmd,cmdLen);
	}
	virtual bool cmdMsgParse(const Cmd::Cmd_NULL *,const DWORD)=0;
	bool doCmd()
	{
		CmdPair *cmd = cmdQueue.get();
		while(cmd)
		{
			cmdMsgParse((const Cmd::Cmd_NULL *)cmd->second,cmd->first);
			cmdQueue.erase();
			cmd = cmdQueue.get();
		}
		if (cmd)
		{
			cmdQueue.erase();
		}
		return true;
	}

private:
	MsgQueue<> cmdQueue;
};
#endif
