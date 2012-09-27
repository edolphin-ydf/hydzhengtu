#include "stdafx.h"

#include "MolCircularBuffer.h"

/** 
 * ���캯��
 */
MolCircularBuffer::MolCircularBuffer()
{
	m_buffer = m_bufferEnd = m_regionAPointer = m_regionBPointer = NULL;
	m_regionASize = m_regionBSize = 0;
}

/// ��������
MolCircularBuffer::~MolCircularBuffer()
{
	free(m_buffer);
	m_buffer = NULL;
}

/** 
 * �ӻ������ж�ȡ����
 *
 * @param destination ���ڴ洢��ȡ������
 * @param bytes ��ȡ�����ݴ�С
 *
 * @return ������ݶ�ȡ�ɹ������棬���򷵻ؼ�
 */
bool MolCircularBuffer::Read(void * destination,size_t bytes)
{
	if(m_buffer == NULL)
		return false;

	// copy as much out of region a
	size_t cnt = bytes;
	size_t aRead = 0, bRead = 0;
	if( (m_regionASize + m_regionBSize) < bytes )
		return false;

	// If we have both region A and region B, always "finish" off region A first, as
	// this will contain the "oldest" data
	if( m_regionASize > 0 )
	{
		aRead = (cnt > m_regionASize) ? m_regionASize : cnt;
		memcpy(destination, m_regionAPointer, aRead);
		m_regionASize -= aRead;
		m_regionAPointer += aRead;
		cnt -= aRead;
	}

	// Data left over? read the data from buffer B
	if( cnt > 0 && m_regionBSize > 0 )
	{
		bRead = (cnt > m_regionBSize) ? m_regionBSize : cnt;
		memcpy((char*)destination + aRead, m_regionBPointer, bRead);
		m_regionBSize -= bRead;
		m_regionBPointer += bRead;
		cnt -= bRead;
	}

	// is buffer A empty? move buffer B to buffer A, to increase future performance
	if( m_regionASize == 0 )
	{
		if( m_regionBSize > 0 )
		{
			// push it all to the start of the buffer.
			if( m_regionBPointer != m_buffer )
				memmove(m_buffer, m_regionBPointer, m_regionBSize);

			m_regionAPointer = m_buffer;
			m_regionASize = m_regionBSize;
			m_regionBPointer = NULL;
			m_regionBSize = 0;
		}
		else
		{
			// no data in region b
			m_regionBPointer = NULL;
			m_regionBSize = 0;
			m_regionAPointer = m_buffer;
			m_regionASize = 0;
		}
	}

	return true;
}

void MolCircularBuffer::AllocateB()
{
	m_regionBPointer = m_buffer;
}

/** 
 * д���ݵ���������
 *
 * @param data Ҫд�뵽�������е�����
 * @param bytes Ҫд�뵽�����������ݵĴ�С
 *
 * @return �������д��ɹ������棬���򷵻ؼ�
 */
bool MolCircularBuffer::Write(const void * data,size_t bytes)
{
	if (m_buffer == NULL)
		return false;

	// If buffer B exists, write to it.
	if( m_regionBPointer != NULL )
	{
		if( GetBFreeSpace() < bytes )
			return false;

		memcpy(&m_regionBPointer[m_regionBSize], data, bytes);
		m_regionBSize += bytes;
		return true;
	}

	// Otherwise, write to buffer A, or initialize buffer B depending on which has more space.
	if( GetAFreeSpace() < GetSpaceBeforeA() )
	{
		AllocateB();
		if( GetBFreeSpace() < bytes )
			return false;

		memcpy(&m_regionBPointer[m_regionBSize], data, bytes);
		m_regionBSize += bytes;
		return true;
	}
	else
	{
		if( GetAFreeSpace() < bytes )
			return false;

		memcpy(&m_regionAPointer[m_regionASize], data, bytes);
		m_regionASize += bytes;
		return true;
	}
}

/** 
 * �õ���ǰ�������л��ж���ʣ��ռ�
 */
size_t MolCircularBuffer::GetSpace()
{
	if( m_regionBPointer != NULL )
		return GetBFreeSpace();
	else
	{
		// would allocating buffer B get us more data?
		if( GetAFreeSpace() < GetSpaceBeforeA() )
		{
			AllocateB();
			return GetBFreeSpace();
		}

		// or not?
		return GetAFreeSpace();
	}
}

/** 
 * �õ���ǰ�����������ݴ�С
 */
size_t MolCircularBuffer::GetSize()
{
	return m_regionASize + m_regionBSize;
}

/** 
 * ���ص�ǰ������һ����ȡ�������ݵĴ�С
 */
size_t MolCircularBuffer::GetContiguiousBytes()
{
	if( m_regionASize )			// A before B
		return m_regionASize;
	else
		return m_regionBSize;
}

/** 
 * �ӻ�������ʼɾ��ָ����С������
 *
 * @param len Ҫɾ�������ݵĴ�С
 */
void MolCircularBuffer::Remove(size_t len)
{
	// remove from A first before we remove from b
	size_t cnt = len;
	size_t aRem, bRem;

	// If we have both region A and region B, always "finish" off region A first, as
	// this will contain the "oldest" data
	if( m_regionASize > 0 )
	{
		aRem = (cnt > m_regionASize) ? m_regionASize : cnt;
		m_regionASize -= aRem;
		m_regionAPointer += aRem;
		cnt -= aRem;
	}

	// Data left over? cut the data from buffer B
	if( cnt > 0 && m_regionBSize > 0 )
	{
		bRem = (cnt > m_regionBSize) ? m_regionBSize : cnt;
		m_regionBSize -= bRem;
		m_regionBPointer += bRem;
		cnt -= bRem;
	}

	// is buffer A empty? move buffer B to buffer A, to increase future performance
	if( m_regionASize == 0 )
	{
		if( m_regionBSize > 0 )
		{
			// push it all to the start of the buffer.
			if( m_regionBPointer != m_buffer )
				memmove(m_buffer, m_regionBPointer, m_regionBSize);

			m_regionAPointer = m_buffer;
			m_regionASize = m_regionBSize;
			m_regionBPointer = NULL;
			m_regionBSize = 0;
		}
		else
		{
			// no data in region b
			m_regionBPointer = NULL;
			m_regionBSize = 0;
			m_regionAPointer = m_buffer;
			m_regionASize = 0;
		}
	}
}

/** 
 * ���ػ�������β��ָ��,�Ա���ѹ��������
 */
void * MolCircularBuffer::GetBuffer()
{
	if( m_regionBPointer != NULL )
		return m_regionBPointer + m_regionBSize;
	else
		return m_regionAPointer + m_regionASize;
}

/** 
 * ���·���ָ����С�Ļ�����
 *
 * @param size Ҫ���·��仺�����Ĵ�С
 */
void MolCircularBuffer::Allocate(size_t size)
{
	m_buffer = (uint8*)malloc(size);
	m_bufferEnd = m_buffer + size;
	m_regionAPointer = m_buffer;		// reset A to the start
}

/** 
 * ��ǰ�ƶ�дָ�뵽ָ����С��λ��
 *
 * @param len Ҫ�ƶ���λ�Ƶ�ƫ����
 */
void MolCircularBuffer::IncrementWritten(size_t len)
{
	if( m_regionBPointer != NULL )
		m_regionBSize += len;
	else
		m_regionASize += len;
}

/** 
 * ���ػ������Ŀ�ʼָ��
 */
void * MolCircularBuffer::GetBufferStart()
{
	if( m_regionASize > 0 )
		return m_regionAPointer;
	else
		return m_regionBPointer;
}