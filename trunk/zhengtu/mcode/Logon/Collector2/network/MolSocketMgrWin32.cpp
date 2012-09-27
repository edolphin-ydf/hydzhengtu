#include "stdafx.h"
#include "MolSocketMgrWin32.h"

#include "MolSocket.h"
#include "MolSocketOps.h"


initialiseSingleton(SocketMgr);

/** 
 * 构造函数
 */
SocketMgr::SocketMgr()
: m_maxsockets(0),uncombuf(NULL),combuf(NULL)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);

	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,(ULONG_PTR)0,0);

	combuf = (unsigned char*)malloc(MOL_REV_BUFFER_SIZE);
	uncombuf = (unsigned char*)malloc(MOL_REV_BUFFER_SIZE);
}

/** 
 * 析构函数
 */
SocketMgr::~SocketMgr()
{
	ClearMesList();

	ClearTasks();

	if(combuf) free(combuf);
	combuf = NULL;
	if(uncombuf) free(uncombuf);
	uncombuf = NULL;
}

void SocketMgr::SpawnWorkerThreads()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	threadcount = si.dwNumberOfProcessors;

	TRACE("IOCP: 生成 %u 个工作线程。\n",threadcount);
	for(long x = 0; x < threadcount; ++x)
		ThreadPool.ExecuteTask(new SocketWorkerThread());
}

/**
 * 清除系统中所有的任务
 */
void SocketMgr::ClearTasks(void)
{
	if(_ThreadBases.empty()) return;

	std::list<ThreadBase*>::iterator iter = _ThreadBases.begin();
	for(;iter!=_ThreadBases.end();++iter)
		if((*iter)) delete (*iter);
	_ThreadBases.clear();
}

bool SocketWorkerThread::run()
{
	HANDLE cp = sSocketMgr.GetCompletionPort();
	DWORD len = 0;
	Socket * s = NULL;
	OverlappedStruct * ov=NULL;
	LPOVERLAPPED ol_ptr;

	while(true)
	{		
		try
		{
#ifndef _WIN64
			if(!GetQueuedCompletionStatus(cp, &len, (LPDWORD)&s, &ol_ptr, 10000))
#else
			if(!GetQueuedCompletionStatus(cp, &len, (PULONG_PTR)&s, &ol_ptr, 10000))
#endif
			{
				continue;
			}

			if (len == 0xFFFFFFFF)  
			{  
				return true;  
			}  

			sSocketGarbageCollector.Lock();
			ov = CONTAINING_RECORD(ol_ptr,OverlappedStruct,m_overlap);
			if(ov == NULL)
			{
				sSocketGarbageCollector.Unlock();
				continue;
			}

			if(ov->m_event == SOCKET_IO_THREAD_SHUTDOWN)
			{
				delete ov;
				ov = NULL;
				sSocketGarbageCollector.Unlock();
				return true;
			}

			if(s == NULL || s->removedFromSet)
			{
				sSocketGarbageCollector.Unlock();
				continue;
			}

			//if(sSocketMgr.FindSocket(s))
			//{
				if(ov->m_event >=0 && ov->m_event < NUM_SOCKET_IO_EVENTS)
					ophandlers[ov->m_event](s,len);
			//}
			sSocketGarbageCollector.Unlock();
		}
		catch (std::exception e)
		{
			sSocketGarbageCollector.Unlock();
			TRACE("工作线程异常:%s\n",e.what());
		}
	}

	return true;
}

void HandleReadComplete(Socket * s,uint32 len)
{
	if(s == NULL) return;

	if(!s->IsDeleted())
	{
		s->m_readEvent.Unmark();
		if(len)
		{
			s->GetReadBuffer().IncrementWritten(len);	
			s->OnRead(len);		
			s->SetupReadEvent();				
		}
		else 
		{
			//s->Delete();
			s->SetHeartCount((uint32)time(NULL)-150);
		}
	}
}

void HandleWriteComplete(Socket * s,uint32 len)
{
	if(s == NULL) return;

	if(!s->IsDeleted())
	{
		s->m_writeEvent.Unmark();
		if(s->BurstBegin())
		{
			s->GetWriteBuffer().Remove(len);
			if(s->GetWriteBuffer().GetContiguiousBytes() > 0)
				s->WriteCallback();
			else
				s->DecSendLock();
			s->BurstEnd();
		}
	}
}

void HandleShutdown(Socket * s,uint32 len)
{

}

void SocketMgr::CloseAll()
{
	socketLock.Acquire();
	for(stdext::hash_map<SOCKET,Socket*>::iterator itr = _sockets.begin(); itr != _sockets.end(); ++itr)
	{
		if((*itr).second == NULL) continue;

		SocketOps::CloseSocket( (*itr).second->GetFd() );
		sSocketGarbageCollector.QueueSocket((*itr).second);
	}
	_sockets.clear();
	socketLock.Release();

	//std::list<Socket*> tokill;

	//for(std::list<Socket*>::iterator itr = tokill.begin(); itr != tokill.end(); ++itr)
	//{
	//	if((*itr))
	//		(*itr)->Disconnect();
	//}

	//size_t size;
	//do
	//{
	//	socketLock.Acquire();
	//	size = _sockets.size();
	//	socketLock.Release();
	//}while(size);
}

void SocketMgr::ShutdownThreads()
{
	for(int i = 0; i < threadcount; ++i)
	{
		OverlappedStruct * ov = new OverlappedStruct(SOCKET_IO_THREAD_SHUTDOWN);
		PostQueuedCompletionStatus(m_completionPort, 0, (ULONG_PTR)0, &ov->m_overlap);
	}
}

void SocketMgr::ShowStatus()
{
	socketLock.Acquire();

	TRACE("_sockets.size(): %d", _sockets.size());

	socketLock.Release();
}

/**
 * 清空消息列表
 */
void SocketMgr::ClearMesList(void)
{
	//if(_MesList.empty()) return;

	//mesLock.Acquire();
	//std::list<MessageStru>::iterator iter = _MesList.begin();
	//for(;iter!=_MesList.end();++iter)
	//{
	//	if((*iter).mes)
	//		delete (*iter).mes;
	//}
	//_MesList.clear();
	//mesLock.Release();
}

///** 
//* 压缩我们要传输的数据
//*
//* @param out 我们要压缩的数据
//* @param declength 压缩后的数据长度
//*
//* @return 返回压缩后的数据
//*/
//char* SocketMgr::compress(CMolMessageOut &out,int *declength)
//{
//	if(out.getData() == NULL || out.getLength() <= 0) return NULL;
//
//	int length = out.getLength();
//	int result=0;
//
//	memset(combuf,0,MOL_REV_BUFFER_SIZE);
//
//	memcpy(combuf,out.getData(),length);
//
//	// 加密数据
//	Encrypto((unsigned char*)combuf,length);
//
//	*declength = length;
//
//	return (char*)combuf;
//}
//
///**
//* 解压我们接收到的数据
//*
//* @param data 要解压的数据
//* @param srclength 解压的数据的长度
//* @param declength 解压后的数据的长度
//*
//* @return 返回解压后的数据
//*/
//char* SocketMgr::uncompress(unsigned char *data,int srclength,int *declength)
//{
//	if(data == NULL || srclength <= 0) return NULL;
//
//	int src_length = srclength;
//	int result=0;
//
//	//src_length = src_length * 200;
//
//	memset(uncombuf,0,MOL_REV_BUFFER_SIZE);
//
//	memcpy(uncombuf,data,srclength);
//
//	// 加密数据
//	Decrypto((unsigned char*)uncombuf,srclength);
//
//	*declength = srclength;
//	return (char*)uncombuf;
//}
//
///**
//* 加密数据
//*
//* @param data 要加密的数据
//* @param length 要加密的数据的长度
//*/
//void SocketMgr::Encrypto(unsigned char *data,unsigned long length)
//{
//	if(data == NULL || length <= 0) return;
//
//	unsigned char pKeyList[] = {76,225,112,120,103,92,84,105,8,12,238,122,206,165,222,21,117,217,106,214,239,66,32,3,85,67,224,180,
//		240,233,236,171,89,13,52,109,123,99,132,213,15,137,226,69,231,228,60,28,190,193,74,144,81,53,17,101,230,207,79,93,88,36,30,
//		141,115,110,20,169,173,243,219,80,72,184,125,175,174,139,95,24,148,48,113,182,50,223,61,118,140,14,78,181,16,4,121,73,187,//		147,168,9,116,23,63,216,215,244,232,59,195,154,200,55,62,220,75,161,196,68,159,6,167,40,45,0,22,155,64,127,27,237,192,212,58,//		26,98,201,41,209,179,130,211,208,82,152,172,7,35,205,107,46,33,146,185,87,199,25,2,77,39,156,164,102,194,163,241,96,166,10,11,//		235,198,157,229,126,94,56,189,134,5,153,133,242,1,31,119,37,145,47,178,18,177,176,86,129,197,65,210,111,54,43,70,188,128,90,//		227,162,104,186,108,114,158,142,57,218,151,202,170,234,150,100,183,71,135,160,42,203,49,97,138,91,124,29,149,83,44,51,19,143,
//		131,38,34,136,221,191,204,245,246,247,248,249,250,251,252,253,254,255};
//
//	for(int i=0;i<(int)length;i++)
//	{
//		data[i] = pKeyList[data[i]];
//	}
//}
//
///**
//* 解密数据
//*
//* @param data 要解密的数据
//* @param length 要解密的数据的长度
//*/
//void SocketMgr::Decrypto(unsigned char *data,unsigned long length)
//{
//	if(data == NULL || length <= 0) return;
//
//	unsigned char pKeyList[] = {123,182,156,23,93,178,119,145,8,99,167,168,9,33,89,40,92,54,189,236,66,15,124,101,79,155,133,128,47,231,//		62,183,22,150,240,146,61,185,239,158,121,136,224,199,234,122,149,187,81,226,84,235,34,53,198,111,175,212,132,107,46,86,112,102,//		126,195,21,25,117,43,200,221,72,95,50,114,0,157,90,58,71,52,142,233,6,24,192,153,60,32,203,229,5,59,174,78,165,227,134,37,219,55,//		161,4,206,7,18,148,208,35,65,197,2,82,209,64,100,16,87,184,3,94,11,36,230,74,173,127,202,193,139,238,38,180,177,222,241,41,228,77,//		88,63,211,237,51,186,151,97,80,232,218,214,143,179,109,125,159,171,210,118,223,115,205,163,160,13,166,120,98,67,216,31,144,68,76,75,//		191,190,188,138,27,91,83,220,73,152,207,96,201,176,48,243,130,49,162,108,116,194,170,154,110,135,215,225,244,147,12,57,141,137,196,//		140,131,39,19,104,103,17,213,70,113,242,14,85,26,1,42,204,45,172,56,44,106,29,217,169,30,129,10,20,28,164,181,69,105,245,246,247,248,
//		249,250,251,252,253,254,255};
//
//	for(int i=0;i<(int)length;i++)
//	{
//		data[i] = pKeyList[data[i]];
//	}
//}