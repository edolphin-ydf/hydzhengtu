#include "stdafx.h"
#include "dataprovider.h"

/** 
 * ���캯��
 */
DataProvider::DataProvider(void)
	throw()
	: mIsConnected(false)
{

}

/** 
 * ��������
 */
DataProvider::~DataProvider(void)
	throw()
{

}

/** 
 * �õ����ݿ�����״̬
 *
 * @return ������ݿ������з����棬���򷵻ؼ�
 */
bool DataProvider::isConnected(void) const
	throw()
{
	return mIsConnected;
}

/** 
 * ����SQL���
 *
 * @param sql Ҫ���򻯵�SQL���
 *
 * @return ���ع��򻯺��sql���
 */
std::string& DataProvider::escapeSQL(std::string& sql)
{
	size_t pos = 0;

	pos = sql.find("'", pos);
	while (pos != std::string::npos)
	{
		sql.replace(pos, 1, "\'\'");
		pos += 2;
		pos = sql.find("'", pos);
	}

	return sql;
}