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
			// ����һ����򵥵İ���ʣ����ֽڴ滺��
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
				// ���С������ͷ
				DWORD dwNeedSize = MinPackLen - m_SplitBuff[nClient].dwSize;
				if ( !m_SplitBuff[nClient].AddData(pDataCur, dwNeedSize) )
				{
					return -1;
				}
				pDataCur = pDataCur + dwNeedSize;
			}

			// ����ϴ���ʣ������
			pPacket = (FS_PACKET*)m_SplitBuff[nClient].GetBuffer();
			if (pPacket->nSize > ((DWORD)(pDataEnd - pDataCur)) + m_SplitBuff[nClient].dwSize)
			{
				assert("�쳣����");
				// ������ݲ���,���뻺�棬�ȴ��´�����
				DWORD dwLessSize = (DWORD)(pDataEnd - pDataCur);
				if ( !m_SplitBuff[nClient].AddData(pDataCur, dwLessSize) )
				{
					return -1;
				}
				pDataCur += dwLessSize;
			}
			else
			{
				// �����������Ҫ�����ݳ���
				DWORD dwNeedSize = pPacket->nSize - m_SplitBuff[nClient].dwSize;
				DWORD dwLessLen = (DWORD)(pDataEnd - pDataCur);
				assert(dwLessLen >= dwNeedSize);
				if ( !m_SplitBuff[nClient].AddData(pDataCur, dwNeedSize) )
				{
					return -1;
				}
				pDataCur += dwNeedSize;
				// �������е��Ǹ���
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
			// �Ƿ������ݰ�
			return -1;
		}

		if (pDataCur + pPacket->nSize > pDataEnd)
		{
			// ���ݲ���һ����,���뻺�棬�ȴ��´�����
			DWORD dwLessSize = (DWORD)(pDataEnd - pDataCur);
			if ( !m_SplitBuff[nClient].AddData(pDataCur, dwLessSize) )
			{
				return -1;
			}
			break;
		}

		OnProcessPacket(nClient, pPacket);

		// �Ƶ���һ�����ݰ�
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
			// �Ƿ������ݰ�
			return -1;
		}

		if (pPacket->nSize > m_SplitBuff[nClient].GetDataLen())
		{
			// ���ݲ������ȴ��´�����
			break;
		}

		OnProcessPacket(nClient, pPacket);
		m_SplitBuff[nClient].DeleteData(pPacket->nSize);
	}

	// ��ɽ���
	m_SplitBuff[nClient].Reset();

	return 0;
}

#endif