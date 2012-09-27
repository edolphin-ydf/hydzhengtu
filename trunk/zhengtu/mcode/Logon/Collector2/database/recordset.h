#ifndef _RECORD_SET_H_INCLUDE
#define _RECORD_SET_H_INCLUDE

/** 
* MolNet��������
*
* ����:���ڴ洢ͨ��SQL��ѯ�Ľ��
* ����:akinggw
* ����:2010.2.28
*/

#include "../network/MolCommon.h"

#include <iostream>
#include <vector>

/** 
 * ���ڴ洢RecordSet�е���������
 */
typedef std::vector<std::string> Row;

/** 
 * RecordSet ���ڴ洢ͨ��SQL����ѯ�Ľ��
 */
class RecordSet
{
public:
	/// ��ʼ�Ĺ��캯��
	RecordSet(void)
		throw();
	/// ��������
	~RecordSet(void)
		throw();

	/// ������еļ�¼
	void clear(void);
	/// ��⵱ǰ��¼�Ƿ�Ϊ��
	bool isEmpty(void) const;
	/// �õ���ǰ��¼�ж�����
	unsigned int rows(void) const;
	/// �õ���ǰ��¼�ж�����
	unsigned int cols(void) const;
	/// ���õ�ǰ��¼���б�ͷ
	void setColumnHeaders(const Row& headers);
	/// ���һ���µ�����
	void add(const Row& row);
	/// �õ�ָ���к�ָ���е�����
	const std::string& operator()(const unsigned int row,
		                          const unsigned int col) const;
	/// �õ�ָ����ָ�����Ƶ�����
	const std::string& operator()(const unsigned int row,
		                          const std::string& name) const;
private:
	Row mHeaders;              /**< ���ڴ洢��ǰ��¼���������� */
	typedef std::vector<Row> Rows;
	Rows mRows;                /**< ���ڴ洢��ȡ������ */
};

#endif
