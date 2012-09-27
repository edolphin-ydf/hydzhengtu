#include "GameServer.h"
#include <assert.h>



double dwTotalSize = 0;


CGameServer::CGameServer(void)
{
}

CGameServer::~CGameServer(void)
{
}


#if 0

int CGameServer::OnReceive(WORD nClient, char* pData, DWORD dwDataLen)
{
	FS_PACKET* pPacket = NULL;

	CAutoLock autoLock(m_csData);

	char* pDataCur = pData;
	char* pDataEnd = pData + dwDataLen;
	while (pDataCur < pDataEnd)
	{
		WORD OverHeadLen = sizeof(DWORD)+sizeof(DWORD);
		WORD MinPackLen = sizeof(FS_PACKET);

		DWORD dwTmp = (DWORD)(pDataEnd - pDataCur);
		if (dwTmp < MinPackLen)
		{
			// 不够一个最简单的包，剩余的字节存缓存
			if ( !m_SplitBuff[nClient].AddData(pDataCur, dwTmp) )
			{
				return -1;
			}
			break;
		}

		if (m_SplitBuff[nClient].dwSize > 0)
		{
			if (m_SplitBuff[nClient].dwSize < MinPackLen)
			{
				// 如果小于数据头
				DWORD dwNeedSize = MinPackLen - m_SplitBuff[nClient].dwSize;
				if ( !m_SplitBuff[nClient].AddData(pDataCur, dwNeedSize) )
				{
					return -1;
				}
				pDataCur = pDataCur + dwNeedSize;
			}

			// 如果上次有剩余数据
			pPacket = (FS_PACKET*)m_SplitBuff[nClient].GetBuffer();
			if (pPacket->nSize > ((DWORD)(pDataEnd - pDataCur)) + m_SplitBuff[nClient].dwSize)
			{
				assert("异常数据");
				// 如果数据不够,存入缓存，等待下次数据
				DWORD dwLessSize = (DWORD)(pDataEnd - pDataCur);
				if ( !m_SplitBuff[nClient].AddData(pDataCur, dwLessSize) )
				{
					return -1;
				}
				pDataCur += dwLessSize;
			}
			else
			{
				// 计算这个包需要的数据长度
				DWORD dwNeedSize = pPacket->nSize - m_SplitBuff[nClient].dwSize;
				DWORD dwLessLen = (DWORD)(pDataEnd - pDataCur);
				assert(dwLessLen >= dwNeedSize);
				if ( !m_SplitBuff[nClient].AddData(pDataCur, dwNeedSize) )
				{
					return -1;
				}
				pDataCur += dwNeedSize;
				// 处理缓存中的那个包
				pPacket = (FS_PACKET*)m_SplitBuff[nClient].GetBuffer();
				OnProcessPacket(nClient, pPacket);
				assert(m_SplitBuff[nClient].dwSize == pPacket->nSize);
				m_SplitBuff[nClient].Clear();
				continue;
			}
		}

		pPacket = (FS_PACKET*)pDataCur;
		if (pPacket->wHeader != 0xFFFF)
		{
			// 非法的数据包
			return -1;
		}

		if (pDataCur + pPacket->nSize > pDataEnd)
		{
			// 数据不足一个包,存入缓存，等待下次数据
			DWORD dwLessSize = (DWORD)(pDataEnd - pDataCur);
			if ( !m_SplitBuff[nClient].AddData(pDataCur, dwLessSize) )
			{
				return -1;
			}
			break;
		}

		OnProcessPacket(nClient, pPacket);

		// 移到下一个数据包
		pDataCur = pDataCur + pPacket->nSize;
	}


	return 0;
}

#else

int CGameServer::OnReceive(WORD nClient, char* pData, DWORD dwDataLen)
{
	dwTotalSize += dwDataLen;

	FS_PACKET* pPacket = NULL;

	CAutoLock autoLock(m_csData);

	WORD MinPackLen = sizeof(FS_PACKET);
	if (!m_SplitBuff[nClient].AddData(pData, dwDataLen))
	{
		return -1;
	}

	while ( m_SplitBuff[nClient].GetDataLen() >=  MinPackLen)
	{
		pPacket = (FS_PACKET*)m_SplitBuff[nClient].GetBuffer();
		if (pPacket->wHeader != 0xFFFF)
		{
			// 非法的数据包
			return -1;
		}

		if (pPacket->nSize > m_SplitBuff[nClient].GetDataLen())
		{
			// 数据不够，等待下次数据
			break;
		}

		OnProcessPacket(nClient, pPacket);
		m_SplitBuff[nClient].DeleteData(pPacket->nSize);
	}

	// 完成解析
	m_SplitBuff[nClient].Reset();

	return 0;
}

#endif