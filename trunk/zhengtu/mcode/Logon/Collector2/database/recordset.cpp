#include "stdafx.h"
#include "recordset.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "dalexcept.h"

/** 
 * ��ʼ�Ĺ��캯��
 */
RecordSet::RecordSet(void)
	throw()
{

}

/** 
 * ��������
 */
RecordSet::~RecordSet(void)
	throw()
{

}

/** 
 * ������еļ�¼
 */
void RecordSet::clear(void)
{
	mHeaders.clear();
	mRows.clear();
}

/** 
 * ��⵱ǰ��¼�Ƿ�Ϊ��
 *
 * @return �����ǰ��¼Ϊ�յĻ������棬���򷵻ؼ�
 */
bool RecordSet::isEmpty(void) const
{
	return (mRows.size() == 0);
}

/** 
 * �õ���ǰ��¼�ж�����
 *
 * @return ���ص�ǰ��¼�ж���������
 */
unsigned int RecordSet::rows(void) const
{
	return (unsigned int)mRows.size();
}

/** 
 * �õ���ǰ��¼�ж�����
 *
 * @return ���ص�ǰ��¼�ж���������
 */
unsigned int RecordSet::cols(void) const
{
	return (unsigned int)mHeaders.size();
}

/** 
 * ���õ�ǰ��¼���б�ͷ
 *
 * @param headers Ҫ���õ��б�ͷ
 */
void RecordSet::setColumnHeaders(const Row& headers)
{
	if(mHeaders.size() > 0)
	{
		throw AlreadySetException();
	}

	mHeaders = headers;
}

/** 
 * ���һ���µ�����
 *
 * @param row Ҫ��ӵ�һ������
 */
void RecordSet::add(const Row& row)
{
	const unsigned int nCols = (unsigned int)mHeaders.size();

	if(nCols == 0)
	{
		throw RsColumnHeadersNotSet();
	}

	if(row.size() != nCols)
	{
		std::ostringstream msg;
		msg << "��ǰ����" << (unsigned int)row.size() << "��;"
			<< "ʵ����" << nCols << "��" << std::ends;

		throw std::invalid_argument(msg.str());
	}

	mRows.push_back(row);
}

/** 
* �õ�ָ���к�ָ���е�����
*
* @param row,col Ҫȡ�õ����ݵ��кź��к�
*
* @return ���������ݴ��ڷ���������ݵ��ַ�����ʾ�������׳��쳣
*/
const std::string& RecordSet::operator()(const unsigned int row,
										 const unsigned int col) const
{
	if((row >= mRows.size() || (col >= mHeaders.size())))
	{
		std::ostringstream os;
		os << "(" << row << "," << col << ") �ǳ����˷�Χ;"
			<< "�����:" << (unsigned int)mRows.size()
			<< ",�����:" << (unsigned int)mHeaders.size() << std::ends;

		throw std::out_of_range(os.str());
	}

	return mRows[row][col];
}

/** 
 * �õ�ָ����ָ�����Ƶ�����
 *
 * @param row Ҫȡ�õ����ݵ��к�
 * @param name Ҫȡ�õ����ݵ�����
 *
 * @return ���������ݴ��ڷ���������ݵ��ַ�����ʾ�������׳��쳣
 */
const std::string& RecordSet::operator()(const unsigned int row,
							  const std::string& name) const
{
	if(row >= mRows.size())
	{
		std::ostringstream os;
		os << "�к�" << row << "������Χ;"
			<< "��ǰ�����:" << (unsigned int)mRows.size() << std::ends;

		throw std::out_of_range(os.str());
	}

	Row::const_iterator it = std::find(mHeaders.begin(),
		                               mHeaders.end(),
									   name);

	if(it == mHeaders.end())
	{
		std::ostringstream os;
		os << "����Ϊ:" << name << "�����ݲ�����." << std::ends;

		throw std::invalid_argument(os.str());
	}

	// �ҵ�������ݵ�������
	const unsigned int nCols = (unsigned int)mHeaders.size();
	unsigned int i;
	for(i=0;i<nCols;++i)
	{
		if(mHeaders[i] == name)
			break;
	}

	return mRows[row][i];
}