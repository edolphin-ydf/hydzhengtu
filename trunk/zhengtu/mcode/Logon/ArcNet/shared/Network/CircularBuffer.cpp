/*
 * Circular Buffer Class
 * Based on the Bip Buffer concept, from http://www.codeproject.com/KB/IP/bipbuffer.aspx
 * Implementation Copyright (C) 2008-2010 Burlex
 *
 */

#include "../Common.h"
#include "CircularBuffer.h"

/** 构造函数
 */
CircularBuffer::CircularBuffer()
{
	m_bufferHead = m_bufferEnd = m_regionAPointer = m_regionBPointer = NULL;
	m_regionASize = m_regionBSize = 0;
}

/** 析构函数
 */
CircularBuffer::~CircularBuffer()
{
	free(m_bufferHead);
}

/** 从缓冲区中读取数据
 * @param 用于存储读取的数据
 * @param 读取的数据大小
 * @return 如果数据读取成功返回真，否则返回假
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
		aRead = (cnt > m_regionASize) ? m_regionASize : cnt;//如果指定的数据长度大于A的长度，则拷贝A的长度
		memcpy(destination, m_regionAPointer, aRead);
		m_regionASize -= aRead;  //清空A被读的数据
		m_regionAPointer += aRead;
		cnt -= aRead;
	}

	// Data left over? read the data from buffer B
	// 如果需要的数据长度还不够，接着从B拷贝数据
	if(cnt > 0 && m_regionBSize > 0)
	{
		bRead = (cnt > m_regionBSize) ? m_regionBSize : cnt;
		memcpy((char*)destination + aRead, m_regionBPointer, bRead);
		m_regionBSize -= bRead;   //清空B被读的数据
		m_regionBPointer += bRead;
		cnt -= bRead;
	}

	// is buffer A empty? move buffer B to buffer A, to increase future performance
	// 如果A区域没数据了，把B移动为A
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

/** 写数据到缓冲区中
 * @param 要写入到缓冲区中的数据
 * @param 要写入到缓冲区的数据的大小
 * @return 如果数据写入成功返回真，否则返回假
 */
bool CircularBuffer::Write(const void* data, size_t bytes)
{
	if(m_bufferHead == NULL)
		return false;

	// If buffer B exists, write to it.
	// 如果B区域存在，则写入B
	if(m_regionBPointer != NULL)
	{
		if(GetBFreeSpace() < bytes)//判断缓冲区剩余空间是否够存放bytes长的数据
			return false;

		memcpy(&m_regionBPointer[m_regionBSize], data, bytes);
		m_regionBSize += bytes;
		return true;
	}

	// Otherwise, write to buffer A, or initialize buffer B depending on which has more space.
	// 这种情况，写入到A还是B，取决于哪块空间比较大
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

/** 得到当前缓冲区中还有多少剩余空间
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

/**得到当前缓冲区中数据大小
 */
size_t CircularBuffer::GetSize()
{
	return m_regionASize + m_regionBSize;
}

/** 返回当前缓冲区一次能取出的数据的大小
 */
size_t CircularBuffer::GetContiguiousBytes()
{
	if(m_regionASize)			// A before B
		return m_regionASize;
	else
		return m_regionBSize;
}

/** 从缓冲区开始删除指定大小的数据
 * @param 要删除的数据的大小
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

/** 返回缓冲区结尾的指针,以便于压入新数据
 */
void* CircularBuffer::GetBuffer()
{
	if(m_regionBPointer != NULL)
		return m_regionBPointer + m_regionBSize;
	else
		return m_regionAPointer + m_regionASize;
}

/** 重新分配指定大小的缓冲区
 * @param size 要重新分配缓冲区的大小
 */
void CircularBuffer::Allocate(size_t size)
{
	m_bufferHead = (uint8*)malloc(size);
	m_bufferEnd = m_bufferHead + size;
	m_regionAPointer = m_bufferHead;		// reset A to the start
}

/** 向前移动写指针到指定大小的位移
 * @param 要移动的位移的偏移量
 */
void CircularBuffer::IncrementWritten(size_t len)			// known as "commit"
{
	if(m_regionBPointer != NULL)
		m_regionBSize += len;
	else
		m_regionASize += len;
}

/** 返回缓冲区的开始指针
 */
void* CircularBuffer::GetBufferStart()
{
	if(m_regionASize > 0)
		return m_regionAPointer;
	else
		return m_regionBPointer;

}
