
#ifndef _NETLIB_CIRCULARBUFFER_H
#define _NETLIB_CIRCUALRBUFFER_H

//����(�߼���)�ṹ�Ļ�����
//��Ч������ʽѭ��������

class CircularBuffer
{
		// ������������ͷָ���βָ��
		uint8* m_bufferHead;// ���黺������ͷָ��
		uint8* m_bufferEnd; // ���黺������βָ��

		// region A pointer, and size
		uint8* m_regionAPointer;// A����Ŀ�ʼָ��
		size_t m_regionASize;   // A�������ݵĴ�С

		// region size
		uint8* m_regionBPointer;// B����Ŀ�ʼָ��
		size_t m_regionBSize;   // B�������ݵĴ�С

		// pointer magic!
		inline size_t GetAFreeSpace() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }//A������滹�еĿ��пռ�Ĵ�С
		inline size_t GetSpaceBeforeA() { return (m_regionAPointer - m_bufferHead); }             //A�������Ŀռ�Ĵ�С���п��ܰ�����B����
		inline size_t GetSpaceAfterA() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }//A�������Ŀռ䣬��A�������Ŀ��пռ�
		inline size_t GetBFreeSpace() { if(m_regionBPointer == NULL) { return 0; } return (m_regionAPointer - m_regionBPointer - m_regionBSize); }//B�����еĿ��пռ�

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
		*//// �ӻ������ж�ȡ���ݣ��ᷢ�������������ԭ������
		bool Read(void* destination, size_t bytes);
		void AllocateB();

		/** д���ݵ���������
		* @param д�����ݵ�ָ��
		* @param ��д�����ݵ��ֽ���
		* @return true �ǳɹ�������ʧ��
		*/
		bool Write(const void* data, size_t bytes);

		/** Returns the number of available bytes left.
		*///�õ���ǰ�������л��ж���ʣ��ռ�
		size_t GetSpace();

		/** Returns the number of bytes currently stored in the buffer.
		*////// �õ���ǰ�����������ݴ�С
		size_t GetSize();

		/** Returns the number of contiguous bytes (that can be pushed out in one operation)
		*/// ���ص�ǰ������һ����ȡ�������ݵĴ�С
		size_t GetContiguiousBytes();

		/** Removes len bytes from the front of the buffer
		* @param len the number of bytes to "cut"
		*/// �ӻ�������ʼɾ��ָ����С������
		void Remove(size_t len);

		/** Returns a pointer at the "end" of the buffer, where new data can be written
		*/// ���ػ�������β��ָ��,�Ա���ѹ��������
		void* GetBuffer();

		/** ���仺�����ֽڴ�С�Ŀռ䣨�����ڳ�ʼ��ʱ���ã�
		* @param size the number of bytes to allocate
		*/
		void Allocate(size_t size);

		/** Increments the "writen" pointer forward len bytes
		* @param len number of bytes to step
		*/// ��ǰ�ƶ�дָ�뵽ָ����С��λ��
		void IncrementWritten(size_t len);			// known as "commit"

		/** Returns a pointer at the "beginning" of the buffer, where data can be pulled from
		*/// ���ػ������Ŀ�ʼָ��
		void* GetBufferStart();
};

#endif		// _NETLIB_CIRCULARBUFFER_H

