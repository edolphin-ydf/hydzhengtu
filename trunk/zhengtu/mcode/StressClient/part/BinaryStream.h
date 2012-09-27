#pragma once
#include <windows.h>
#include <assert.h>

class CBinaryStream
{
public:
	CBinaryStream(DWORD BufferSize);
	CBinaryStream(void);
	~CBinaryStream(void);

public:
	// 获取缓存大小
	int GetBufferLen();
	void Clear();				// 清除缓冲的数据
	void Release();				// 释放缓冲的数据
	const void* GetBufferPtr();	// 获得缓冲的指针(比较好用但危险的操作,需要用户来控制内存问题)

public:
	// 函数接口
	BYTE ReadByte();
	BOOL WriteByte(BYTE data);

	WORD ReadWord();
	BOOL WriteWord(WORD data);

	INT ReadInt();
	BOOL WriteInt(INT data);

	DWORD ReadDWord();
	BOOL WriteDWord(DWORD data);

	DWORD64 ReadDWord64();
	BOOL WriteDWord64(DWORD64 data);

	float ReadFloat();
	BOOL WriteFloat(float data);

	double ReadDouble();
	BOOL WriteDouble(double data);

	BOOL ReadString(char* lpString, DWORD size);
	BOOL WriteString(const char* lpString, DWORD size);

	BOOL ReadData(void* pBuffer, DWORD iDataLen);
	BOOL WriteData(void* pBuffer, DWORD iDataLen);

public:
	// 读指针移动函数
	int ReadMoveNext(int iOffset);	// 向后移动指针
	int ReadMovePrev(int iOffset);	// 向前移动指针
	int ReadMoveLast();				// 移动指针到末尾
	int ReadMoveFirst();			// 移动指针到首端

	// 写指针移动函数
	int WriteMoveNext(int iOffset);	// 向后移动指针
	int WriteMovePrev(int iOffset);	// 向前移动指针
	int WriteMoveLast();			// 移动指针到末尾
	int WriteMoveFirst();			// 移动指针到首端

public:
	//////////////////////////////////////////////////////////////////////////
	// 操作符接口

	// 数据输出
	template <class T>
	BOOL operator >> (T& data)
	{
		DWORD iDataLen = sizeof(T);
		assert(m_iReaderPos+iDataLen <= m_iBufferCurSize);
		if (m_iReaderPos+iDataLen > m_iBufferCurSize)
		{
			return FALSE;
		}

		void* pBuffer = (void*)(m_pBuffer + m_iReaderPos);
		memcpy(&data, pBuffer, iDataLen);
		m_iReaderPos += iDataLen;

		return TRUE;
	}


	// 数据输入
	template <class T>
	BOOL operator << (T& data)
	{
		DWORD iDataLen = sizeof(T);

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

		void* pBuffer = (void*)(m_pBuffer + m_iWriterPos);
		memcpy(pBuffer, &data, iDataLen);
		if (m_iWriterPos + iDataLen > m_iBufferCurSize) 
		{
			m_iBufferCurSize = m_iWriterPos + iDataLen;
		}
		m_iWriterPos = m_iWriterPos + iDataLen;

		return TRUE;
	}

private:
	// 检查并处理缓冲溢出，如果有有溢出，就扩充缓冲
	BOOL ProcessWriteOverflow(DWORD dwAddDataLen);

private:
	DWORD m_iBufferTotalSize;	// 缓冲总大小
	DWORD m_iBufferCurSize;		// 当前写入数据的大小
	BYTE* m_pBuffer;			// 缓冲指针
	DWORD m_iReaderPos;			// 当前缓冲读指针的位置
	DWORD m_iWriterPos;			// 当前缓冲写指针的位置
};
