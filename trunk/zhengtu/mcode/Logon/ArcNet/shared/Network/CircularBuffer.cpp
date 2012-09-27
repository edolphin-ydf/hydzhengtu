/*
 * Circular Buffer Class
 * Based on the Bip Buffer concept, from http://www.codeproject.com/KB/IP/bipbuffer.aspx
 * Implementation Copyright (C) 2008-2010 Burlex
 *
 */

#include "../Common.h"
#include "CircularBuffer.h"

/** ���캯��
 */
CircularBuffer::CircularBuffer()
{
	m_bufferHead = m_bufferEnd = m_regionAPointer = m_regionBPointer = NULL;
	m_regionASize = m_regionBSize = 0;
}

/** ��������
 */
CircularBuffer::~CircularBuffer()
{
	free(m_bufferHead);
}

/** �ӻ������ж�ȡ����
 * @param ���ڴ洢��ȡ������
 * @param ��ȡ�����ݴ�С
 * @return ������ݶ�ȡ�ɹ������棬���򷵻ؼ�
 */
bool CircularBuffer::Read(void* destination, size_t bytes)
{
	if(m_bufferHead == NULL)
		return false;

	// copy as much out of region a
	size_t cnt = bytes;
	size_t aRead = 0, bRead = 0;
	if((m_regionASize + m_regionBSize) < bytes)
		return false;

	// If we have both region A and region B, always "finish" off region A first, as
	// this will contain the "oldest" data
	if(m_regionASize > 0)
	{
		aRead = (cnt > m_regionASize) ? m_regionASize : cnt;//���ָ�������ݳ��ȴ���A�ĳ��ȣ��򿽱�A�ĳ���
		memcpy(destination, m_regionAPointer, aRead);
		m_regionASize -= aRead;  //���A����������
		m_regionAPointer += aRead;
		cnt -= aRead;
	}

	// Data left over? read the data from buffer B
	// �����Ҫ�����ݳ��Ȼ����������Ŵ�B��������
	if(cnt > 0 && m_regionBSize > 0)
	{
		bRead = (cnt > m_regionBSize) ? m_regionBSize : cnt;
		memcpy((char*)destination + aRead, m_regionBPointer, bRead);
		m_regionBSize -= bRead;   //���B����������
		m_regionBPointer += bRead;
		cnt -= bRead;
	}

	// is buffer A empty? move buffer B to buffer A, to increase future performance
	// ���A����û�����ˣ���B�ƶ�ΪA
	if(m_regionASize == 0)
	{
		if(m_regionBSize > 0)
		{
			// push it all to the start of the buffer.
			if(m_regionBPointer != m_bufferHead)
				memmove(m_bufferHead, m_regionBPointer, m_regionBSize);

			m_regionAPointer = m_bufferHead;
			m_regionASize = m_regionBSize;
			m_regionBPointer = NULL;
			m_regionBSize = 0;
		}
		else
		{
			// no data in region b
			m_regionBPointer = NULL;
			m_regionBSize = 0;
			m_regionAPointer = m_bufferHead;
			m_regionASize = 0;
		}
	}

	return true;
}

void CircularBuffer::AllocateB()
{
	//printf("[allocating B]\n");
	m_regionBPointer = m_bufferHead;
}

/** д���ݵ���������
 * @param Ҫд�뵽�������е�����
 * @param Ҫд�뵽�����������ݵĴ�С
 * @return �������д��ɹ������棬���򷵻ؼ�
 */
bool CircularBuffer::Write(const void* data, size_t bytes)
{
	if(m_bufferHead == NULL)
		return false;

	// If buffer B exists, write to it.
	// ���B������ڣ���д��B
	if(m_regionBPointer != NULL)
	{
		if(GetBFreeSpace() < bytes)//�жϻ�����ʣ��ռ��Ƿ񹻴��bytes��������
			return false;

		memcpy(&m_regionBPointer[m_regionBSize], data, bytes);
		m_regionBSize += bytes;
		return true;
	}

	// Otherwise, write to buffer A, or initialize buffer B depending on which has more space.
	// ���������д�뵽A����B��ȡ�����Ŀ�ռ�Ƚϴ�
	if(GetAFreeSpace() < GetSpaceBeforeA())
	{
		AllocateB();
		if(GetBFreeSpace() < bytes)
			return false;

		memcpy(&m_regionBPointer[m_regionBSize], data, bytes);
		m_regionBSize += bytes;
		return true;
	}
	else
	{
		if(GetAFreeSpace() < bytes)
			return false;

		memcpy(&m_regionAPointer[m_regionASize], data, bytes);
		m_regionASize += bytes;
		return true;
	}
}

/** �õ���ǰ�������л��ж���ʣ��ռ�
 */
size_t CircularBuffer::GetSpace()
{
	if(m_regionBPointer != NULL)
		return GetBFreeSpace();
	else
	{
		// would allocating buffer B get us more data?
		if(GetAFreeSpace() < GetSpaceBeforeA())
		{
			AllocateB();
			return GetBFreeSpace();
		}

		// or not?
		return GetAFreeSpace();
	}
}

/**�õ���ǰ�����������ݴ�С
 */
size_t CircularBuffer::GetSize()
{
	return m_regionASize + m_regionBSize;
}

/** ���ص�ǰ������һ����ȡ�������ݵĴ�С
 */
size_t CircularBuffer::GetContiguiousBytes()
{
	if(m_regionASize)			// A before B
		return m_regionASize;
	else
		return m_regionBSize;
}

/** �ӻ�������ʼɾ��ָ����С������
 * @param Ҫɾ�������ݵĴ�С
 */
void CircularBuffer::Remove(size_t len)
{
	// remove from A first before we remove from b
	size_t cnt = len;
	size_t aRem, bRem;

	// If we have both region A and region B, always "finish" off region A first, as
	// this will contain the "oldest" data
	if(m_regionASize > 0)
	{
		aRem = (cnt > m_regionASize) ? m_regionASize : cnt;
		m_regionASize -= aRem;
		m_regionAPointer += aRem;
		cnt -= aRem;
	}

	// Data left over? cut the data from buffer B
	if(cnt > 0 && m_regionBSize > 0)
	{
		bRem = (cnt > m_regionBSize) ? m_regionBSize : cnt;
		m_regionBSize -= bRem;
		m_regionBPointer += bRem;
		cnt -= bRem;
	}

	// is buffer A empty? move buffer B to buffer A, to increase future performance
	if(m_regionASize == 0)
	{
		if(m_regionBSize > 0)
		{
			// push it all to the start of the buffer.
			if(m_regionBPointer != m_bufferHead)
				memmove(m_bufferHead, m_regionBPointer, m_regionBSize);

			m_regionAPointer = m_bufferHead;
			m_regionASize = m_regionBSize;
			m_regionBPointer = NULL;
			m_regionBSize = 0;
		}
		else
		{
			// no data in region b
			m_regionBPointer = NULL;
			m_regionBSize = 0;
			m_regionAPointer = m_bufferHead;
			m_regionASize = 0;
		}
	}
}

/** ���ػ�������β��ָ��,�Ա���ѹ��������
 */
void* CircularBuffer::GetBuffer()
{
	if(m_regionBPointer != NULL)
		return m_regionBPointer + m_regionBSize;
	else
		return m_regionAPointer + m_regionASize;
}

/** ���·���ָ����С�Ļ�����
 * @param size Ҫ���·��仺�����Ĵ�С
 */
void CircularBuffer::Allocate(size_t size)
{
	m_bufferHead = (uint8*)malloc(size);
	m_bufferEnd = m_bufferHead + size;
	m_regionAPointer = m_bufferHead;		// reset A to the start
}

/** ��ǰ�ƶ�дָ�뵽ָ����С��λ��
 * @param Ҫ�ƶ���λ�Ƶ�ƫ����
 */
void CircularBuffer::IncrementWritten(size_t len)			// known as "commit"
{
	if(m_regionBPointer != NULL)
		m_regionBSize += len;
	else
		m_regionASize += len;
}

/** ���ػ������Ŀ�ʼָ��
 */
void* CircularBuffer::GetBufferStart()
{
	if(m_regionASize > 0)
		return m_regionAPointer;
	else
		return m_regionBPointer;

}
