#include "BinaryStream.h"



#define BUFFER_DEFAULT_SIZE			1024 * 50;

CBinaryStream::CBinaryStream(void)
{
	m_iBufferTotalSize = BUFFER_DEFAULT_SIZE;
	m_iBufferCurSize = 0;
	m_pBuffer = new BYTE[m_iBufferTotalSize];
	ZeroMemory(m_pBuffer, m_iBufferTotalSize);
	m_iReaderPos = 0;
	m_iWriterPos = 0;
}

CBinaryStream::CBinaryStream(DWORD BufferSize)
{
	assert(BufferSize > 0);
	m_iBufferTotalSize = BufferSize;
	m_iBufferCurSize = 0;
	m_pBuffer = new BYTE[m_iBufferTotalSize];
	ZeroMemory(m_pBuffer, m_iBufferTotalSize);
	m_iReaderPos = 0;
	m_iWriterPos = 0;
}

CBinaryStream::~CBinaryStream(void)
{
	Release();
}

// 释放缓冲的数据
void CBinaryStream::Release()
{
	if (m_pBuffer != NULL)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
		m_iBufferTotalSize = 0;
		m_iBufferCurSize = 0;
		m_iReaderPos = 0;
		m_iWriterPos = 0;
	}
}

// 获取缓存大小
int CBinaryStream::GetBufferLen()
{
	return m_iBufferCurSize;
}

const void* CBinaryStream::GetBufferPtr()
{
	return (const void*)m_pBuffer;
}

// 清除缓冲的数据
void CBinaryStream::Clear()
{
	if (m_pBuffer != NULL)
	{
		ZeroMemory(m_pBuffer, m_iBufferTotalSize);
		m_iBufferCurSize = 0;
		m_iReaderPos = 0;
		m_iWriterPos = 0;
	}
}


// 检查并处理缓冲溢出，如果有有溢出，就扩充缓冲
BOOL CBinaryStream::ProcessWriteOverflow(DWORD dwAddDataLen)
{
	if (m_iWriterPos+dwAddDataLen > m_iBufferTotalSize)
	{
		// 缓冲溢出的时候，重新分配缓存内存，大小*2
		DWORD iNewBuffSize = m_iBufferTotalSize * 2;
		if (iNewBuffSize < dwAddDataLen)
		{
			// 超过2倍，就分配当前数据大小
			iNewBuffSize = dwAddDataLen;
		}

		BYTE *pNewBuff = new BYTE[iNewBuffSize];
		if (pNewBuff == NULL)
		{
			return FALSE;
		}

		ZeroMemory(pNewBuff, iNewBuffSize);
		memcpy(pNewBuff, m_pBuffer, m_iBufferTotalSize);
		delete[] m_pBuffer;
		m_pBuffer = pNewBuff;
		m_iBufferTotalSize = iNewBuffSize;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// 函数接口
BYTE CBinaryStream::ReadByte()
{
	BYTE rtnData = 0;
	*this >>rtnData;
	return rtnData;
}

BOOL CBinaryStream::WriteByte(BYTE data)
{
	return (*this << data);
}

WORD CBinaryStream::ReadWord()
{
	WORD rtnData = 0;
	*this >>rtnData;
	return rtnData;
}

BOOL CBinaryStream::WriteWord(WORD data)
{
	return (*this << data);
}

INT CBinaryStream::ReadInt()
{
	INT rtnData = 0;
	*this >>rtnData;
	return rtnData;
}

BOOL CBinaryStream::WriteInt(INT data)
{
	return (*this << data);
}

DWORD CBinaryStream::ReadDWord()
{
	DWORD rtnData = 0;
	*this >>rtnData;
	return rtnData;
}

BOOL CBinaryStream::WriteDWord(DWORD data)
{
	return (*this << data);
}

DWORD64 CBinaryStream::ReadDWord64()
{
	DWORD64 rtnData = 0;
	*this >>rtnData;
	return rtnData;
}

BOOL CBinaryStream::WriteDWord64(DWORD64 data)
{
	return (*this << data);
}

float CBinaryStream::ReadFloat()
{
	float rtnData = 0;
	*this >>rtnData;
	return rtnData;
}

BOOL CBinaryStream::WriteFloat(float data)
{
	return (*this << data);
}

double CBinaryStream::ReadDouble()
{
	double rtnData = 0;
	*this >>rtnData;
	return rtnData;
}

BOOL CBinaryStream::WriteDouble(double data)
{
	return (*this << data);
}

BOOL CBinaryStream::ReadString(char* lpString, DWORD size)
{
	assert(m_iReaderPos+size <= m_iBufferCurSize);
	if (m_iReaderPos+size > m_iBufferCurSize)
	{
		return FALSE;
	}

	BYTE* pLocalBuffer = (BYTE*)(m_pBuffer + m_iReaderPos);
	memcpy(lpString, pLocalBuffer, size);
	m_iReaderPos += size;
	return TRUE;
}

BOOL CBinaryStream::WriteString(const char* lpString, DWORD size)
{
	BOOL bSuccess = ProcessWriteOverflow(size);
	assert( bSuccess );
	if ( !bSuccess )
	{
		return FALSE;
	}

	assert(m_iWriterPos+size <= m_iBufferTotalSize);
	if (m_iWriterPos+size > m_iBufferTotalSize)
	{
		return FALSE;
	}

	BYTE* pLocalBuffer = (BYTE*)(m_pBuffer + m_iWriterPos);
	memcpy(pLocalBuffer, lpString, size);
	if (m_iWriterPos + size > m_iBufferCurSize) 
	{
		m_iBufferCurSize = m_iWriterPos + size;
	}
	m_iWriterPos = m_iWriterPos + size;
	return TRUE;
}

BOOL CBinaryStream::ReadData(void* pBuffer, DWORD iDataLen)
{
	assert(m_iReaderPos+iDataLen <= m_iBufferCurSize);
	if (m_iReaderPos+iDataLen > m_iBufferCurSize)
	{
		return FALSE;
	}

	BYTE* pLocalBuffer = (BYTE*)(m_pBuffer + m_iReaderPos);
	memcpy(pBuffer, pLocalBuffer, iDataLen);
	m_iReaderPos += iDataLen;
	return TRUE;
}

BOOL CBinaryStream::WriteData(void* pBuffer, DWORD iDataLen)
{
	BOOL bSuccess = ProcessWriteOverflow(iDataLen);
	assert( bSuccess );
	if ( !bSuccess )
	{
		return FALSE;
	}

	assert(m_iWriterPos+iDataLen <= m_iBufferTotalSize);
	if (m_iWriterPos+iDataLen > m_iBufferTotalSize)
	{
		return FALSE;
	}

	BYTE* pLocalBuffer = (BYTE*)(m_pBuffer + m_iWriterPos);
	memcpy(pLocalBuffer, pBuffer, iDataLen);
	if (m_iWriterPos + iDataLen > m_iBufferCurSize) 
	{
		m_iBufferCurSize = m_iWriterPos + iDataLen;
	}
	m_iWriterPos = m_iWriterPos + iDataLen;
	return TRUE;
}


// 向后移动指针
int CBinaryStream::ReadMoveNext(int iOffset)
{
	int iOffset2 = iOffset;
	assert(m_iReaderPos + iOffset < m_iBufferCurSize);
	if (m_iReaderPos + iOffset >= m_iBufferCurSize)
	{
		m_iReaderPos = m_iBufferCurSize;
		iOffset2 = m_iReaderPos + iOffset - m_iBufferCurSize;
	}
	else
	{
		m_iReaderPos += iOffset;
	}

	return iOffset2;
}

// 向前移动指针
int CBinaryStream::ReadMovePrev(int iOffset)
{
	int iOffset2 = iOffset;
	assert(m_iReaderPos - iOffset >= 0);
	if (m_iReaderPos - iOffset < 0)
	{
		m_iReaderPos = 0;
		iOffset2 = 0;
	}
	else
	{
		m_iReaderPos -= iOffset;
	}

	return iOffset2;
}

// 移动指针到末尾
int CBinaryStream::ReadMoveLast()
{
	int iOffset2 = m_iBufferCurSize - m_iReaderPos;
	m_iReaderPos = m_iBufferCurSize;
	return iOffset2;
}

// 移动指针到首端
int CBinaryStream::ReadMoveFirst()
{
	int iOffset2 = m_iReaderPos - 0;
	m_iReaderPos = 0;
	return iOffset2;
}


// 向后移动指针
int CBinaryStream::WriteMoveNext(int iOffset)
{
	int iOffset2 = iOffset;
	assert(m_iWriterPos + iOffset < m_iBufferCurSize);
	if (m_iWriterPos + iOffset >= m_iBufferCurSize)
	{
		m_iWriterPos = m_iBufferCurSize;
		iOffset2 = m_iWriterPos + iOffset - m_iBufferCurSize;
	}
	else
	{
		m_iWriterPos += iOffset;
	}

	return iOffset2;
}

// 向前移动指针
int CBinaryStream::WriteMovePrev(int iOffset)
{
	int iOffset2 = iOffset;
	assert(m_iWriterPos - iOffset >= 0);
	if (m_iWriterPos - iOffset < 0)
	{
		m_iWriterPos = 0;
		iOffset2 = 0;
	}
	else
	{
		m_iWriterPos -= iOffset;
	}

	return iOffset2;
}

// 移动指针到末尾
int CBinaryStream::WriteMoveLast()
{
	int iOffset2 = m_iBufferCurSize - m_iWriterPos;
	m_iWriterPos = m_iBufferCurSize;
	return iOffset2;
}

// 移动指针到首端
int CBinaryStream::WriteMoveFirst()
{
	int iOffset2 = m_iWriterPos - 0;
	m_iWriterPos = 0;
	return iOffset2;
}