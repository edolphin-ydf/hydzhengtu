#ifndef _MYSQL_DATA_PROVIDER_H_INCLUDE
#define _MYSQL_DATA_PROVIDER_H_INCLUDE

/** 
* MolNet��������
*
* ����:mysql�����ݿ����ʵ����
* ����:akinggw
* ����:2010.2.28
*/

#include <iosfwd>
#include <string>

#include "../network/MolCommon.h"
#include "mysql.h"

#include "connectPool.h"
#include "dataprovider.h"

/** 
 * mysql�����ݿ����ʵ����
 */
class MySqlDataProvider : public DataProvider
{
public:
	/** 
	 * �������mysql_autocommit()������my_bool����
	 */
	enum 
	{
		AUTOCOMMIT_OFF = 0,
		AUTOCOMMIT_ON
	};

	/// ���캯��
	MySqlDataProvider(void)
		throw();
	/// ��������
	~MySqlDataProvider(void)
		throw();

	/// �õ����ݿ������
	virtual DbBackends getDbBackend(void) const
		throw();
	/// ���������ݿ������
	virtual bool connect(std::string host,std::string username,std::string password,
		std::string dbName,unsigned int port);
	/// ִ��SQL���
	virtual const RecordSet execSql(const std::string& sql,const bool refresh=false);
	/// �ر������ݿ������
	virtual void disconnect(void);
	/// ��ʼһ������
	virtual void beginTransaction(void)
		throw (std::runtime_error);
	/// �ύһ������
	virtual void commitTransaction(void)
		throw (std::runtime_error);
	/// �ع�һ������
	virtual void rollbackTransaction(void)
		throw (std::runtime_error);
	/// �õ����ִ��SQL����ı���еĸ���
	virtual unsigned int getModifiedRows(void);
	/// �õ������������ݵ��к�
	virtual unsigned int getLastId(void);

private:
	ConnectPool m_ConnectPool;                             /**< ���ݵ����ӳ� */
};

#endif
