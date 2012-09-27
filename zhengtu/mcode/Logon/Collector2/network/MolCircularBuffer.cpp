#include "stdafx.h"

#include "MolCircularBuffer.h"

/** 
 * 构造函数
 */
MolCircularBuffer::MolCircularBuffer()
{
	m_buffer = m_bufferEnd = m_regionAPointer = m_regionBPointer = NULL;
	m_regionASize = m_regionBSize = 0;
}

/// 析构函数
MolCircularBuffer::~MolCircularBuffer()
{
	free(m_buffer);
	m_buffer = NULL;
}

/** 
 * 从缓冲区中读取数据
 *
 * @param destination 用于存储读取的数据
 * @param bytes 读取的数据大小
 *
 * @return 如果数据读取成功返回真，否则返回假
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
 * 写数据到缓冲区中
 *
 * @param data 要写入到缓冲区中的数据
 * @param bytes 要写入到缓冲区的数据的大小
 *
 * @return 如果数据写入成功返回真，否则返回假
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
 * 得到当前缓冲区中还有多少剩余空间
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
 * 得到当前缓冲区中数据大小
 */
size_t MolCircularBuffer::GetSize()
{
	return m_regionASize + m_regionBSize;
}

/** 
 * 返回当前缓冲区一次能取出的数据的大小
 */
size_t MolCircularBuffer::GetContiguiousBytes()
{
	if( m_regionASize )			// A before B
		return m_regionASize;
	else
		return m_regionBSize;
}

/** 
 * 从缓冲区开始删除指定大小的数据
 *
 * @param len 要删除的数据的大小
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
 * 返回缓冲区结尾的指针,以便于压入新数据
 */
void * MolCircularBuffer::GetBuffer()
{
	if( m_regionBPointer != NULL )
		return m_regionBPointer + m_regionBSize;
	else
		return m_regionAPointer + m_regionASize;
}

/** 
 * 重新分配指定大小的缓冲区
 *
 * @param size 要重新分配缓冲区的大小
 */
void MolCircularBuffer::Allocate(size_t size)
{
	m_buffer = (uint8*)malloc(size);
	m_bufferEnd = m_buffer + size;
	m_regionAPointer = m_buffer;		// reset A to the start
}

/** 
 * 向前移动写指针到指定大小的位移
 *
 * @param len 要移动的位移的偏移量
 */
void MolCircularBuffer::IncrementWritten(size_t len)
{
	if( m_regionBPointer != NULL )
		m_regionBSize += len;
	else
		m_regionASize += len;
}

/** 
 * 返回缓冲区的开始指针
 */
void * MolCircularBuffer::GetBufferStart()
{
	if( m_regionASize > 0 )
		return m_regionAPointer;
	else
		return m_regionBPointer;
}