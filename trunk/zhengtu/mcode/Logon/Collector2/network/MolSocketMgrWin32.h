#ifndef _MOL_SOCKET_MGR_WIN32_H_INCLUDE
#define _MOL_SOCKET_MGR_WIN32_H_INCLUDE

/** 
* MolNet��������
*
* ����:���ڹ���ǰϵͳ�����еĿͻ���
* ����:akinggw
* ����:2010.2.12
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

#define IDD_MESSAGE_HEART_BEAT                  10000                  // ������Ϣ

class SocketMgr : public Singleton<SocketMgr>
{
public:
	/// ���캯��
	SocketMgr();
	/// ��������
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
				TRACE("����socket���������ظ���socket.\n");
			}
			socketLock.Release();
		}
		catch (std::exception e)
		{
			socketLock.Release();
			TRACE("����socket�쳣:%s\n",e.what());
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
			TRACE("ɾ��socket�쳣:%s\n",e.what());
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
			TRACE("socket�����쳣:%s\n",e.what());
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

	/// �õ���ǰ�ж��ٸ��ͻ���
	uint32 GetClientCount(void)
	{
		return (uint32)_sockets.size();
	}

	void ShutdownThreads();
	/// �������֧�ֿͻ���,���Ϊ0�Ļ��������ƿͻ��˸���
	inline void SetMaxSockets(int c)
	{
		m_maxsockets = c;
	}
	/// �õ����֧�ֿͻ�������
	inline uint32 GetMaxSockets(void)
	{
		return m_maxsockets;
	}

	/// ������Ϣ�б�
	inline void LockSocketList(void){socketLock.Acquire();}

	/// ������Ϣ�б�
	inline void UnLockSocketList(void){socketLock.Release();}

	/// ���һ����Ϣ���б���
	inline void AddMessage(MessageStru mes)
	{
		//mesLock.Acquire();
		//_MesList.push_back(mes);
		//mesLock.Release();
	}
	/// �õ���ǰ��Ϣ����
	inline int GetMesCount(void)
	{
		return 0;/*(int)_MesList.size();*/
	}
	/// ������Ϣ�б�
	inline void LockMesList(void)
	{
		mesLock.Acquire();
	}
	/// ������Ϣ�б�
	inline void UnlockMesList(void)
	{
		mesLock.Release();
	}
	///// �õ���Ϣ�б�
	//inline std::list<MessageStru>* GetMesList(void)
	//{
	//	return &_MesList;
	//}
	/// �����Ϣ�б�
	void ClearMesList(void);

	/// ���һ������ϵͳ��
	inline void AddTask(ThreadBase *task)
	{
		if(task == NULL) return;

		_ThreadBases.push_back(task);
	}
	/// ���ϵͳ�����е�����
	void ClearTasks(void);
	///// ѹ������Ҫ���������
	//char* compress(CMolMessageOut &out,int *declength);
	///// ��ѹ���ǽ��յ�������
	//char* uncompress(unsigned char *data,int srclength,int *declength);
	///// ��������
	//void Encrypto(unsigned char *data,unsigned long length);
	///// ��������
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
