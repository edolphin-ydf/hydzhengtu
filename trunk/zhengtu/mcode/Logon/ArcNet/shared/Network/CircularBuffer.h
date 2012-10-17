//////////////////////////////////////////////////
/// @file : circularbuffer.h
/// @brief : ���νṹ�Ļ����� ��Ч������ʽѭ��������
/// @date:  2012/9/29
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __circularbuffer_H__
#define __circularbuffer_H__

////////////////////////////////////////////////////////////////
/// @class CircularBuffer
/// @brief ��Ч������ʽѭ��������
///
/// @note ���νṹ�Ļ�����
class SERVER_DECL CircularBuffer
{
		/** @brief ������������ͷָ���βָ��*/
		uint8* m_bufferHead;///< ���黺������ͷָ��
		uint8* m_bufferEnd; ///< ���黺������βָ��

		/** @brief region A pointer, and size*/ 
		uint8* m_regionAPointer;///< A����Ŀ�ʼָ��
		size_t m_regionASize;   ///< A�������ݵĴ�С

		/** @brief region size*/
		uint8* m_regionBPointer;///< B����Ŀ�ʼָ��
		size_t m_regionBSize;   ///< B�������ݵĴ�С

		/** @brief pointer magic!*/
		inline size_t GetAFreeSpace() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }///< A������滹�еĿ��пռ�Ĵ�С
		inline size_t GetSpaceBeforeA() { return (m_regionAPointer - m_bufferHead); }             ///< A�������Ŀռ�Ĵ�С���п��ܰ�����B����
		inline size_t GetSpaceAfterA() { return (m_bufferEnd - m_regionAPointer - m_regionASize); }///< A�������Ŀռ䣬��A�������Ŀ��пռ�
		inline size_t GetBFreeSpace() { if(m_regionBPointer == NULL) { return 0; } return (m_regionAPointer - m_regionBPointer - m_regionBSize); }///< B�����еĿ��пռ�

	public:

		/** ����
		*/
		CircularBuffer();

		/** ����
		*/
		~CircularBuffer();

		/** �ӻ������ж�ȡ���ݣ��ᷢ�������������ԭ������
		* @param destination pointer to destination where bytes will be written
		* @param bytes number of bytes to read
		* @return true if there was enough data, false otherwise
		*/
		bool Read(void* destination, size_t bytes);
		void AllocateB();

		/** д���ݵ���������
		* @param д�����ݵ�ָ��
		* @param ��д�����ݵ��ֽ���
		* @return true �ǳɹ�������ʧ��
		*/
		bool Write(const void* data, size_t bytes);

		/** �õ���ǰ�������л��ж���ʣ��ռ�
		*/
		size_t GetSpace();

		/** �õ���ǰ�����������ݴ�С
		*/
		size_t GetSize();

		/** ���ص�ǰ������һ����ȡ�������ݵĴ�С
		*/
		size_t GetContiguiousBytes();

		/** �ӻ�������ʼɾ��ָ����С������
		* @param len the number of bytes to "cut"
		*/
		void Remove(size_t len);

		/** ���ػ�������β��ָ��,�Ա���ѹ��������
		*/
		void* GetBuffer();

		/** ���仺�����ֽڴ�С�Ŀռ䣨�����ڳ�ʼ��ʱ���ã�
		* @param size the number of bytes to allocate
		*/
		void Allocate(size_t size);

		/** ��ǰ�ƶ�дָ�뵽ָ����С��λ��
		* @param len number of bytes to step
		*/
		void IncrementWritten(size_t len);			// known as "commit"

		/** ���ػ������Ŀ�ʼָ��
		*/
		void* GetBufferStart();
};

#endif		// _NETLIB_CIRCULARBUFFER_H
