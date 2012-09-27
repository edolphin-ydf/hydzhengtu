#include "stdafx.h"
#include "mysqldataprovider.h"

#include "../network/MolCommon.h"
#include "dalexcept.h"

/** 
 * ���캯��
 */
MySqlDataProvider::MySqlDataProvider(void)
	throw()
{

}

/** 
 * ��������
 */
MySqlDataProvider::~MySqlDataProvider(void)
	throw()
{
	if(mIsConnected)
		disconnect();
}

/** 
 * �õ����ݿ������
 *
 * @return ���ڷ��ص�ǰ���ݿ������
 */
DbBackends MySqlDataProvider::getDbBackend(void) const
	throw()
{
	return DB_BKEND_MYSQL;
}

/** 
 * ���������ݿ������
 *
 * @param host Ҫ���ӵ����ݿ��IP��ַ
 * @param username Ҫ�������ݿ���û���
 * @param password Ҫ�������ݿ���û�����
 * @param dbName Ҫ���ӵ����ݿ�����
 * @param port Ҫ���ӵ����ݿ�Ķ˿ں�
 */
bool MySqlDataProvider::connect(std::string host,std::string username,std::string password,
					 std::string dbName,unsigned int port)
{
	if(mIsConnected)
		return false;

	if(!m_ConnectPool.Init(MOL_CONN_POOL_MAX,
	                   host.c_str(),
					   username.c_str(),
					   password.c_str(),
					   dbName.c_str(),
					   port)) 
	    return false;

	mDbName = dbName;

	mIsConnected = true;

	return true;
}

/**  
 * ִ��SQL���
 *
 * @param sql Ҫִ�е�SQL���
 * @param refresh �Ƿ�Ҫˢ�����ݣ�Ҳ�������´����ݿ��л�ȡ����
 *
 * @return ����ɹ���ȡ�����ݷ���������ݼ�¼�������׳��쳣
 */
const RecordSet MySqlDataProvider::execSql(const std::string& sql,const bool refresh)
{
	if(!mIsConnected)
		throw std::runtime_error("û�������ݿ⽨�����ӡ�");

	RecordSet pRecordSet;

	if(refresh || (sql != mSql))
	{
		MYSQL *m_curWorkingConn = m_ConnectPool.GetConnet();
		if(m_curWorkingConn == NULL) return pRecordSet;

		mysql_ping(m_curWorkingConn);
		if(mysql_query(m_curWorkingConn,sql.c_str()) != 0)
			throw DbSqlQueryExecFailure(mysql_error(m_curWorkingConn));

		if(mysql_field_count(m_curWorkingConn) > 0)
		{
			MYSQL_RES* res;

			if(!(res = mysql_store_result(m_curWorkingConn)))
				throw DbSqlQueryExecFailure(mysql_error(m_curWorkingConn));

			unsigned int nFields = mysql_num_fields(res);
			//MYSQL_FIELD* fields = mysql_fetch_fields(res);
			Row fieldNames;
			for(unsigned int i=0;i<nFields;i++)
			{
				MYSQL_FIELD* fields = NULL;
				fields=mysql_fetch_field_direct(res,i);
				if(fields) fieldNames.push_back(fields->name);
			}

			pRecordSet.setColumnHeaders(fieldNames);

			MYSQL_ROW row;
			while((row = mysql_fetch_row(res)))
			{
				Row r;

				for(unsigned int i = 0;i < nFields; i++)
					r.push_back(static_cast<char*>(row[i]));

				pRecordSet.add(r);
			}

			mysql_free_result(res);
		}

		m_ConnectPool.PutConnet(m_curWorkingConn);
	}

	return pRecordSet;
}

/** 
 * �ر������ݿ������
 */
void MySqlDataProvider::disconnect(void)
{
	if(!mIsConnected)
		return;

	m_ConnectPool.Close();

	mysql_library_end();

	mIsConnected = false;
}

/** 
 * ��ʼһ������
 */
void MySqlDataProvider::beginTransaction(void)
	throw (std::runtime_error)
{
	if(!mIsConnected)
	{
		const std::string error = "��û���������ݿ������¿�ʼһ������!";
		
		throw std::runtime_error(error);
	}

	MYSQL *m_curWorkingConn = m_ConnectPool.GetConnet();
	if(m_curWorkingConn == NULL) return;

	mysql_autocommit(m_curWorkingConn,AUTOCOMMIT_OFF);
	execSql("BEGIN");

	m_ConnectPool.PutConnet(m_curWorkingConn);
}

/** 
 * �ύһ������
 */
void MySqlDataProvider::commitTransaction(void)
	throw (std::runtime_error)
{
	if(!mIsConnected)
	{
		const std::string error = "��û���������ݿ��������ύһ������!";

		throw std::runtime_error(error);
	}

	MYSQL *m_curWorkingConn = m_ConnectPool.GetConnet();
	if(m_curWorkingConn == NULL) return;

	if(mysql_commit(m_curWorkingConn) != 0)
		throw DbSqlQueryExecFailure(mysql_error(m_curWorkingConn));

	mysql_autocommit(m_curWorkingConn,AUTOCOMMIT_ON);

	m_ConnectPool.PutConnet(m_curWorkingConn);
}

/** 
 * �ع�һ������
 */
void MySqlDataProvider::rollbackTransaction(void)
	throw (std::runtime_error)
{
	if(!mIsConnected)
	{
		const std::string error = "��û���������ݿ������»ع�һ������!";

		throw std::runtime_error(error);
	}

	MYSQL *m_curWorkingConn = m_ConnectPool.GetConnet();
	if(m_curWorkingConn == NULL) return;

	if(mysql_rollback(m_curWorkingConn) != 0)
		throw DbSqlQueryExecFailure(mysql_error(m_curWorkingConn));

	mysql_autocommit(m_curWorkingConn,AUTOCOMMIT_ON);

	m_ConnectPool.PutConnet(m_curWorkingConn);
}

/** 
 * �õ����ִ��SQL����ı���еĸ���
 *
 * @return ��������ı������
 */
unsigned int MySqlDataProvider::getModifiedRows(void) 
{
	if(!mIsConnected)
	{
		const std::string error = "��û���������ݿ���������ͼִ��getModifiedRows!";

		throw std::runtime_error(error);
	}

	MYSQL *m_curWorkingConn = m_ConnectPool.GetConnet();
	if(m_curWorkingConn == NULL) return 0;

	const my_ulonglong affected = mysql_affected_rows(m_curWorkingConn);

	if(affected > INT_MAX)
		throw std::runtime_error("MySqlDataProvider: getModifiedRows ������Χ.");

	if(affected == (my_ulonglong)-1)
	{
		throw DbSqlQueryExecFailure(mysql_error(m_curWorkingConn));
	}

	m_ConnectPool.PutConnet(m_curWorkingConn);

	return (unsigned int)affected;
}

/** 
 * �õ������������ݵ��к�
 *
 * @return ���ظı����ݵ����µ��к�
 */
unsigned int MySqlDataProvider::getLastId(void) 
{
	if(!mIsConnected)
	{
		const std::string error = "��û���������ݿ���������ͼִ��getLastId!";

		throw std::runtime_error(error);
	}

	MYSQL *m_curWorkingConn = m_ConnectPool.GetConnet();
	if(m_curWorkingConn == NULL) return 0;

	const my_ulonglong lastId = mysql_insert_id(m_curWorkingConn);
	if(lastId > UINT_MAX)
		throw std::runtime_error("MySqlDataProvider: getLastId ������Χ.");

	m_ConnectPool.PutConnet(m_curWorkingConn);

	return (unsigned int)lastId;
}