#ifndef _GAMESERVER_H_
#define _GAMESERVER_H_
#include "IOCPServer.h"
#include <assert.h>
#include "FSSocketPacket.h"



#define SPLIT_BUFFER_SIZE		MAX_RECV_BUFFER * 2

struct SBuffer
{
public:
	DWORD dwSize;
	char* pBuffCur;
	char* pBuffStart;
	char* pBuffEnd;
	char szBuffer[SPLIT_BUFFER_SIZE];

	SBuffer()
	{
		ZeroMemory(szBuffer, SPLIT_BUFFER_SIZE*sizeof(char));
		pBuffStart = szBuffer;
		pBuffCur = pBuffStart;
		pBuffEnd = szBuffer + SPLIT_BUFFER_SIZE;
		dwSize = 0;
	}

	void Clear()
	{	
		dwSize = 0;
		pBuffCur = pBuffStart;
	}

	void Reset()
	{
		if (dwSize>0)
		{
			memmove(pBuffStart, pBuffCur, dwSize);
		}
		pBuffCur = pBuffStart;
	}

	char* GetBuffer()
	{
		return pBuffCur;
	}

	BOOL AddData(char* pBuffer, DWORD dwDataLen)
	{
		if (pBuffCur + dwDataLen > pBuffEnd)
		{
			assert("内存溢出");
			return FALSE;
		}

		memcpy(pBuffCur+dwSize, pBuffer, dwDataLen);
		dwSize += dwDataLen;
		return TRUE;
	}

	void DeleteData(DWORD dwDataLen)
	{
		assert(dwSize >= dwDataLen);
		ZeroMemory(pBuffCur, dwDataLen);	// 可以不需要
		pBuffCur = pBuffCur + dwDataLen;
		dwSize -= dwDataLen;
	}

	DWORD GetDataLen()
	{
		return dwSize;
	}
};

class CGameServer :
	public CIOCPServer
{
public:
	CGameServer(void);
public:
	virtual ~CGameServer(void);

	virtual int OnReceive(WORD nClient, char* pData, DWORD dwDataLen);
	virtual int OnProcessPacket(WORD nClient, FS_PACKET* pPacket) = 0;

private:
	SBuffer m_SplitBuff[MAX_USER_NUM];
	CCriticalSection m_csData;
};

#endif