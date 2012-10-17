//////////////////////////////////////////////////
/// @file : circularbuffer.h
/// @brief : 环形结构的缓冲区 高效的两段式循环缓冲区
/// @date:  2012/9/29
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __circularbuffer_H__
#define __circularbuffer_H__

////////////////////////////////////////////////////////////////
/// @class CircularBuffer
/// @brief 高效的两段式循环缓冲区
///
/// @note 环形结构的缓冲区
class SERVER_DECL CircularBuffer
{
		/** @brief 整个缓冲区的头指针和尾指针*/
		uint8* m_bufferHead;///< 整块缓冲区的头指针
		uint8* m_bufferEnd; ///< 整块缓冲区的尾指针

		/** @brief region A pointer, and size*/ 
		uint8* m_regionAPointer;///< A区域的开始指针
		size_t m_regionASize;   ///< A区域数据的大小

		/** @brief region size*/
		uint8* m_regionBPointer;///< B区域的开始指针
		size_t m_regionBSize;   ///< B区域数据的大小

		/** @brief pointer magic!*/
		inline size_t GetAFreeSpace() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }///< A区域后面还有的空闲空间的大小
		inline size_t GetSpaceBeforeA() { return (m_regionAPointer - m_bufferHead); }             ///< A区域后面的空间的大小，有可能包括了B区域
		inline size_t GetSpaceAfterA() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }///< A区域后面的空间，即A区域后面的空闲空间
		inline size_t GetBFreeSpace() { if(m_regionBPointer == NULL) { return 0; } return (m_regionAPointer - m_regionBPointer - m_regionBSize); }///< B区域还有的空闲空间

	public:

		/** 构造
		*/
		CircularBuffer();

		/** 析构
		*/
		~CircularBuffer();

		/** 从缓冲区中读取数据，会发生拷贝，并清空原来数据
		* @param destination pointer to destination where bytes will be written
		* @param bytes number of bytes to read
		* @return true if there was enough data, false otherwise
		*/
		bool Read(void* destination, size_t bytes);
		void AllocateB();

		/** 写数据到缓冲区中
		* @param 写入数据的指针
		* @param 被写入数据的字节数
		* @return true 是成功，否则失败
		*/
		bool Write(const void* data, size_t bytes);

		/** 得到当前缓冲区中还有多少剩余空间
		*/
		size_t GetSpace();

		/** 得到当前缓冲区中数据大小
		*/
		size_t GetSize();

		/** 返回当前缓冲区一次能取出的数据的大小
		*/
		size_t GetContiguiousBytes();

		/** 从缓冲区开始删除指定大小的数据
		* @param len the number of bytes to "cut"
		*/
		void Remove(size_t len);

		/** 返回缓冲区结尾的指针,以便于压入新数据
		*/
		void* GetBuffer();

		/** 分配缓冲区字节大小的空间（必须在初始化时调用）
		* @param size the number of bytes to allocate
		*/
		void Allocate(size_t size);

		/** 向前移动写指针到指定大小的位移
		* @param len number of bytes to step
		*/
		void IncrementWritten(size_t len);			// known as "commit"

		/** 返回缓冲区的开始指针
		*/
		void* GetBufferStart();
};

#endif		// _NETLIB_CIRCULARBUFFER_H
