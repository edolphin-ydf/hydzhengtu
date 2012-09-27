#ifndef _MOL_CIRCULAR_BUFFER_H_INCLUDE
#define _MOL_CIRCULAR_BUFFER_H_INCLUDE

/** 
 * MolNet��������
 *
 * ����:���λ�����
 * ����:akinggw
 * ����:2010.2.11
 */

#include "MolCommon.h"

class MolCircularBuffer
{
public:
	/// ���캯��
	MolCircularBuffer();
	/// ��������
	~MolCircularBuffer();

	/// �ӻ������ж�ȡ����
	bool Read(void * destination,size_t bytes);
	void AllocateB();

	/// д���ݵ���������
	bool Write(const void * data,size_t bytes);
	/// �õ���ǰ�������л��ж���ʣ��ռ�
	size_t GetSpace();
	/// �õ���ǰ�����������ݴ�С
	size_t GetSize();
	/// ���ص�ǰ������һ����ȡ�������ݵĴ�С
	size_t GetContiguiousBytes();
	/// �ӻ�������ʼɾ��ָ����С������
	void Remove(size_t len);
	/// ���ػ�������β��ָ��,�Ա���ѹ��������
	void * GetBuffer();
	/// ���·���ָ����С�Ļ�����
	void Allocate(size_t size);
	/// ��ǰ�ƶ�дָ�뵽ָ����С��λ��
	void IncrementWritten(size_t len);
	/// ���ػ������Ŀ�ʼָ��
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
