
#ifndef _NETLIB_CIRCULARBUFFER_H
#define _NETLIB_CIRCUALRBUFFER_H

//环形(逻辑上)结构的缓冲区
//高效的两段式循环缓冲区

class CircularBuffer
{
		// 整个缓冲区的头指针和尾指针
		uint8* m_bufferHead;// 整块缓冲区的头指针
		uint8* m_bufferEnd; // 整块缓冲区的尾指针

		// region A pointer, and size
		uint8* m_regionAPointer;// A区域的开始指针
		size_t m_regionASize;   // A区域数据的大小

		// region size
		uint8* m_regionBPointer;// B区域的开始指针
		size_t m_regionBSize;   // B区域数据的大小

		// pointer magic!
		inline size_t GetAFreeSpace() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }//A区域后面还有的空闲空间的大小
		inline size_t GetSpaceBeforeA() { return (m_regionAPointer - m_bufferHead); }             //A区域后面的空间的大小，有可能包括了B区域
		inline size_t GetSpaceAfterA() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }//A区域后面的空间，即A区域后面的空闲空间
		inline size_t GetBFreeSpace() { if(m_regionBPointer == NULL) { return 0; } return (m_regionAPointer - m_regionBPointer - m_regionBSize); }//B区域还有的空闲空间

	public:

		/** Constructor
		*/
		CircularBuffer();

		/** Destructor
		*/
		~CircularBuffer();

		/** Read bytes from the buffer
		* @param destination pointer to destination where bytes will be written
		* @param bytes number of bytes to read
		* @return true if there was enough data, false otherwise
		*//// 从缓冲区中读取数据，会发生拷贝，并清空原来数据
		bool Read(void* destination, size_t bytes);
		void AllocateB();

		/** 写数据到缓冲区中
		* @param 写入数据的指针
		* @param 被写入数据的字节数
		* @return true 是成功，否则失败
		*/
		bool Write(const void* data, size_t bytes);

		/** Returns the number of available bytes left.
		*///得到当前缓冲区中还有多少剩余空间
		size_t GetSpace();

		/** Returns the number of bytes currently stored in the buffer.
		*////// 得到当前缓冲区中数据大小
		size_t GetSize();

		/** Returns the number of contiguous bytes (that can be pushed out in one operation)
		*/// 返回当前缓冲区一次能取出的数据的大小
		size_t GetContiguiousBytes();

		/** Removes len bytes from the front of the buffer
		* @param len the number of bytes to "cut"
		*/// 从缓冲区开始删除指定大小的数据
		void Remove(size_t len);

		/** Returns a pointer at the "end" of the buffer, where new data can be written
		*/// 返回缓冲区结尾的指针,以便于压入新数据
		void* GetBuffer();

		/** 分配缓冲区字节大小的空间（必须在初始化时调用）
		* @param size the number of bytes to allocate
		*/
		void Allocate(size_t size);

		/** Increments the "writen" pointer forward len bytes
		* @param len number of bytes to step
		*/// 向前移动写指针到指定大小的位移
		void IncrementWritten(size_t len);			// known as "commit"

		/** Returns a pointer at the "beginning" of the buffer, where data can be pulled from
		*/// 返回缓冲区的开始指针
		void* GetBufferStart();
};

#endif		// _NETLIB_CIRCULARBUFFER_H

