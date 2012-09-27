#ifndef _MOL_SOCKET_DEFINES_H_INCLUDE
#define _MOL_SOCKET_DEFINES_H_INCLUDE

/** 
* MolNet��������
*
* ����:IOCP�õ��Ľṹ�Ͷ���
* ����:akinggw
* ����:2010.2.11
*/

#include "MolCommon.h"

enum SocketIOEvent
{
	SOCKET_IO_EVENT_READ_COMPLETE = 0,
	SOCKET_IO_EVENT_WRITE_END     = 1,
	SOCKET_IO_THREAD_SHUTDOWN     = 2,
	NUM_SOCKET_IO_EVENTS          = 3,
};

class OverlappedStruct
{
public:
	OVERLAPPED m_overlap;
	SocketIOEvent m_event;
	volatile long m_inUse;
	OverlappedStruct(SocketIOEvent ev) : m_event(ev)
	{
		memset(&m_overlap, 0, sizeof(OVERLAPPED));
		m_inUse = 0;
	};

	OverlappedStruct()
	{
		memset(&m_overlap, 0, sizeof(OVERLAPPED));
		m_inUse = 0;
	}

	__forceinline void Reset(SocketIOEvent ev)
	{
		memset(&m_overlap, 0, sizeof(OVERLAPPED));
		m_event = ev;
	}

	void Mark()
	{
		long val = InterlockedCompareExchange(&m_inUse, 1, 0);
		if(val != 0)
			TRACE("!!!! Network: Detected double use of read/write event! Previous event was %u.\n", m_event);
	}

	void Unmark()
	{
		InterlockedExchange(&m_inUse, 0);
	}
};

#endif
