#ifndef _DATA_PROVIDER_H_INCLUDE
#define _DATA_PROVIDER_H_INCLUDE

/** 
* MolNet��������
*
* ����:���ݿ�����Ļ��࣬���е�ʵ����������������
* ����:akinggw
* ����:2010.2.28
*/

#include <string>
#include <stdexcept>

#include "recordset.h"

/** 
 * ����Ҫ���������ݿ�����
 */
typedef enum 
{
	DB_BKEND_MYSQL = 0,
	DB_BKEND_END
} DbBackends;

/** 
 * ���ݿ��������
 */
class DataProvider
{
public:
	/// ���캯��
	DataProvider(void)
		throw();
	/// ��������
	virtual ~DataProvider(void)
		throw();

	/// �õ����ݿ�����״̬
	bool isConnected(void) const
		throw();
	/// �õ����ݿ������
	virtual DbBackends getDbBackend(void) const
		throw() = 0;
	/// ���������ݿ������
	virtual bool connect(std::string host,std::string username,std::string password,
		std::string dbName,unsigned int port) = 0;
	/// ִ��SQL���
	virtual const RecordSet execSql(const std::string& sql,const bool refresh=false) = 0;
	/// �ر������ݿ������
	virtual void disconnect(void) = 0;
	/// �õ����ݿ������
	std::string getDbName(void);
	/// ��ʼһ������
	virtual void beginTransaction(void)
		throw (std::runtime_error) = 0;
	/// �ύһ������
	virtual void commitTransaction(void)
		throw (std::runtime_error) = 0;
	/// �ع�һ������
	virtual void rollbackTransaction(void)
		throw (std::runtime_error) = 0;
	/// �õ����ִ��SQL����ı���еĸ���
	virtual unsigned int getModifiedRows(void) = 0;
	/// �õ������������ݵ��к�
	virtual unsigned int getLastId(void) = 0;
	/// ����SQL���
	std::string& escapeSQL(std::string& sql);

protected:
	std::string mDbName;                /**< ���ݿ������ */
	bool mIsConnected;                  /**< ���ݿ������״̬ */
	std::string mSql;                   /**< ���ڴ洢�����SQL��� */
};

#endif
