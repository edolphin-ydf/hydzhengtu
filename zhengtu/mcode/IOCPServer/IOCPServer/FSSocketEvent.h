#ifndef __SOCKETEVENT_H__
#define __SOCKETEVENT_H__

#include <winsock2.h>

class FSSocketEvent
{
public:
	virtual ~FSSocketEvent(void){}

public:
	virtual int OnAccept(WORD nClient) = 0;
	virtual int OnClose(WORD nClient) = 0;
	virtual int OnSend(WORD nClient, char* pData, DWORD dwDataLen) = 0;
	virtual int OnReceive(WORD nClient, char* pData, DWORD dwDataLen) = 0;
	virtual int OnError(WORD nClient, int iError) = 0;

};



#endif