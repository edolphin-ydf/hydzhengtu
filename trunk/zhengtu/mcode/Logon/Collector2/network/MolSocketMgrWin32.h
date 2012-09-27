#ifndef _MOL_SOCKET_MGR_WIN32_H_INCLUDE
#define _MOL_SOCKET_MGR_WIN32_H_INCLUDE

/** 
* MolNet网络引擎
*
* 描述:用于管理当前系统中所有的客户端
* 作者:akinggw
* 日期:2010.2.12
*/

#include "MolCommon.h"
#include "AtomicCounter.h"
#include "MolSingleton.h"
#include "MolThreadStarter.h"
#include "MolThreadPool.h"
#include "MolSocketDefines.h"
#include "MolSocket.h"
#include "MolSocketOps.h"

#include "MolNetMessage.h"
#include <hash_map>

#define IDD_MESSAGE_HEART_BEAT                  10000                  // 心跳消息

class SocketMgr : public Singleton<SocketMgr>
{
public:
	/// 构造函数
	SocketMgr();
	/// 析构函数
	~SocketMgr();

	inline HANDLE GetCompletionPort() { return m_completionPort; }
	void SpawnWorkerThreads();
	void CloseAll();
	void ShowStatus();
	void AddSocket(Socket * s)
	{
		if(!s) return;

		try
		{
			socketLock.Acquire();
			stdext::hash_map<SOCKET,Socket*>::iterator iter = _sockets.find(s->GetFd());
			if(iter == _sockets.end())
			{
				s->removedFromSet=false;
				s->OnConnect();
				_sockets.insert(std::pair<SOCKET,Socket*>(s->GetFd(),s));
				++socket_count;
			}
			else
			{
				TRACE("加入socket出错，出现重复的socket.\n");
			}
			socketLock.Release();
		}
		catch (std::exception e)
		{
			socketLock.Release();
			TRACE("加入socket异常:%s\n",e.what());
		}
	}

	void RemoveSocket(Socket * s)
	{
		if(!s || _sockets.empty()) return;

		try
		{
			socketLock.Acquire();
			stdext::hash_map<SOCKET,Socket*>::iterator iter = _sockets.find(s->GetFd());
			if(iter != _sockets.end())
			{
				s->OnDisconnect();			
				_sockets.erase(iter);
				SocketOps::CloseSocket( s->GetFd() );
				--socket_count;
				s->removedFromSet=true;
			}
			socketLock.Release();
		}
		catch (std::exception e)
		{
			socketLock.Release();
			TRACE("删除socket异常:%s\n",e.what());
		}
	}

	void Update(void)
	{
		time_t t = time(NULL);
		time_t diff;
	
		try
		{
			socketLock.Acquire();
			stdext::hash_map<SOCKET,Socket*>::iterator itr = _sockets.begin();
			Socket * s=NULL;

			for(itr = _sockets.begin();itr != _sockets.end();)
			{
				s = (*itr).second;

				if(s != NULL)
				{
					diff = t - s->GetHeartCount();
					if(diff > 50)		   // More than 5mins
					{		
						s->Disconnect(false);
						s->OnDisconnect();						
						itr = _sockets.erase(itr);
						SocketOps::CloseSocket( s->GetFd() );
						s->removedFromSet=true;
					}
					else
					{
						if(diff > 5)
						{
							CMolMessageOut out(IDD_MESSAGE_HEART_BEAT);
							s->Send(out);
						}

						++itr;
					}
				}
				else
				{
					++itr;
				}
			}
			socketLock.Release();
		}
		catch (std::exception e)
		{
			socketLock.Release();
			TRACE("socket更新异常:%s\n",e.what());
		}	
	}

	inline bool FindSocket(Socket * s)
	{
		if(_sockets.empty() || s == NULL) return false;

		stdext::hash_map<SOCKET,Socket*>::iterator iter = _sockets.find(s->GetFd());
		if(iter != _sockets.end())
			return true;

		return false;
	}

	/// 得到当前有多少个客户端
	uint32 GetClientCount(void)
	{
		return (uint32)_sockets.size();
	}

	void ShutdownThreads();
	/// 设置最大支持客户端,如果为0的话，不限制客户端个数
	inline void SetMaxSockets(int c)
	{
		m_maxsockets = c;
	}
	/// 得到最大支持客户端数量
	inline uint32 GetMaxSockets(void)
	{
		return m_maxsockets;
	}

	/// 锁定消息列表
	inline void LockSocketList(void){socketLock.Acquire();}

	/// 锁定消息列表
	inline void UnLockSocketList(void){socketLock.Release();}

	/// 添加一个消息到列表中
	inline void AddMessage(MessageStru mes)
	{
		//mesLock.Acquire();
		//_MesList.push_back(mes);
		//mesLock.Release();
	}
	/// 得到当前消息个数
	inline int GetMesCount(void)
	{
		return 0;/*(int)_MesList.size();*/
	}
	/// 锁定消息列表
	inline void LockMesList(void)
	{
		mesLock.Acquire();
	}
	/// 解锁消息列表
	inline void UnlockMesList(void)
	{
		mesLock.Release();
	}
	///// 得到消息列表
	//inline std::list<MessageStru>* GetMesList(void)
	//{
	//	return &_MesList;
	//}
	/// 清空消息列表
	void ClearMesList(void);

	/// 添加一个任务到系统中
	inline void AddTask(ThreadBase *task)
	{
		if(task == NULL) return;

		_ThreadBases.push_back(task);
	}
	/// 清除系统中所有的任务
	void ClearTasks(void);
	///// 压缩我们要传输的数据
	//char* compress(CMolMessageOut &out,int *declength);
	///// 解压我们接收到的数据
	//char* uncompress(unsigned char *data,int srclength,int *declength);
	///// 加密数据
	//void Encrypto(unsigned char *data,unsigned long length);
	///// 解密数据
	//void Decrypto(unsigned char *data,unsigned long length);

	long threadcount;

private:
	HANDLE m_completionPort;
	stdext::hash_map<SOCKET,Socket*> _sockets;
	//std::list<MessageStru> _MesList;
	Mutex mesLock;
	Mutex socketLock;
	uint32 m_maxsockets;
	unsigned char *combuf,*uncombuf;
	std::list<ThreadBase*> _ThreadBases;
	AtomicCounter socket_count;
};

#define sSocketMgr SocketMgr::getSingleton()

typedef void(*OperationHandler)(Socket * s,uint32 len);

class SocketWorkerThread : public ThreadBase
{
public:
	bool run();
};

void HandleReadComplete(Socket * s,uint32 len);
void HandleWriteComplete(Socket * s,uint32 len);
void HandleShutdown(Socket * s,uint32 len);

static OperationHandler ophandlers[NUM_SOCKET_IO_EVENTS] = {
	&HandleReadComplete,
	&HandleWriteComplete,
	&HandleShutdown };

#endif
