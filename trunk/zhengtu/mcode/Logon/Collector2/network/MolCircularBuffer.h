#ifndef _MOL_CIRCULAR_BUFFER_H_INCLUDE
#define _MOL_CIRCULAR_BUFFER_H_INCLUDE

/** 
 * MolNet网络引擎
 *
 * 描述:环形缓冲区
 * 作者:akinggw
 * 日期:2010.2.11
 */

#include "MolCommon.h"

class MolCircularBuffer
{
public:
	/// 构造函数
	MolCircularBuffer();
	/// 析构函数
	~MolCircularBuffer();

	/// 从缓冲区中读取数据
	bool Read(void * destination,size_t bytes);
	void AllocateB();

	/// 写数据到缓冲区中
	bool Write(const void * data,size_t bytes);
	/// 得到当前缓冲区中还有多少剩余空间
	size_t GetSpace();
	/// 得到当前缓冲区中数据大小
	size_t GetSize();
	/// 返回当前缓冲区一次能取出的数据的大小
	size_t GetContiguiousBytes();
	/// 从缓冲区开始删除指定大小的数据
	void Remove(size_t len);
	/// 返回缓冲区结尾的指针,以便于压入新数据
	void * GetBuffer();
	/// 重新分配指定大小的缓冲区
	void Allocate(size_t size);
	/// 向前移动写指针到指定大小的位移
	void IncrementWritten(size_t len);
	/// 返回缓冲区的开始指针
	void * GetBufferStart();

private:
	inline size_t GetAFreeSpace() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }
	inline size_t GetSpaceBeforeA() { return (m_regionAPointer - m_buffer); }
	inline size_t GetSpaceAfterA() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }
	inline size_t GetBFreeSpace() { if(m_regionBPointer == NULL) { return 0; } return (m_regionAPointer - m_regionBPointer - m_regionBSize); }

private:
	uint8 * m_buffer;
	uint8 * m_bufferEnd;

	uint8 * m_regionAPointer;
	size_t m_regionASize;

	uint8 * m_regionBPointer;
	size_t m_regionBSize;
};

#endif
