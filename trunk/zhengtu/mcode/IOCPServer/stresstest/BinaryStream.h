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
	// ��ȡ�����С
	int GetBufferLen();
	void Clear();				// ������������
	void Release();				// �ͷŻ��������
	const void* GetBufferPtr();	// ��û����ָ��(�ȽϺ��õ�Σ�յĲ���,��Ҫ�û��������ڴ�����)

public:
	// �����ӿ�
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
	// ��ָ���ƶ�����
	int ReadMoveNext(int iOffset);	// ����ƶ�ָ��
	int ReadMovePrev(int iOffset);	// ��ǰ�ƶ�ָ��
	int ReadMoveLast();				// �ƶ�ָ�뵽ĩβ
	int ReadMoveFirst();			// �ƶ�ָ�뵽�׶�

	// дָ���ƶ�����
	int WriteMoveNext(int iOffset);	// ����ƶ�ָ��
	int WriteMovePrev(int iOffset);	// ��ǰ�ƶ�ָ��
	int WriteMoveLast();			// �ƶ�ָ�뵽ĩβ
	int WriteMoveFirst();			// �ƶ�ָ�뵽�׶�

public:
	//////////////////////////////////////////////////////////////////////////
	// �������ӿ�

	// �������
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


	// ��������
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
	// ��鲢����������������������������仺��
	BOOL ProcessWriteOverflow(DWORD dwAddDataLen);

private:
	DWORD m_iBufferTotalSize;	// �����ܴ�С
	DWORD m_iBufferCurSize;		// ��ǰд�����ݵĴ�С
	BYTE* m_pBuffer;			// ����ָ��
	DWORD m_iReaderPos;			// ��ǰ�����ָ���λ��
	DWORD m_iWriterPos;			// ��ǰ����дָ���λ��
};
