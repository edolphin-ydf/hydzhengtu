#ifndef _DAL_EXCEPT_H_INCLUDE
#define _DAL_EXCEPT_H_INCLUDE

/** 
* MolNet��������
*
* ����:���ڴ������ݿ���������п��ܵ��쳣
* ����:akinggw
* ����:2010.2.27
*/

#include <string>

/** 
 * ���������ݿ��쳣
 */
class DbException : public std::exception
{
public:
	/** 
	 * ���캯��
	 *
	 * @param msg Ҫ�׳��Ĵ�����Ϣ
	 */
	DbException(const std::string& msg)
		throw()
			: mMsg(msg)
	{

	}

	/** 
	 * ��������
	 */
	~DbException(void)
		throw()
	{

	}

	/** 
	 * �õ��׳��Ĵ�����Ϣ
	 *
	 * @return �����׳��쳣�Ĵ�����Ϣ
	 */
	virtual const char* what(void) const
		throw()
	{
		return mMsg.c_str();
	}

private:
	std::string mMsg;
};

/** 
 * ���ݿ�����ʧ��
 */
class DbConnectionFailure : public DbException
{
public:
	/** 
	 * ��ʼ�Ĺ��캯��
	 */
	DbConnectionFailure(void)
		throw()
			: DbException("")
	{

	}

	/** 
	 * ���캯��
	 *
	 * @param msg �׳��쳣�Ĵ�����Ϣ
	 */
	DbConnectionFailure(const std::string& msg)
		throw()
			: DbException(msg)
	{

	}
};

/** 
 * ���ݿ�Ͽ�����ʧ��
 */
class DbDisConnectionFailure : public DbException
{
public:
	/** 
	 * ��ʼ�Ĺ��캯��
	 */
	DbDisConnectionFailure(void)
		throw()
			: DbException("")
	{

	}

	/** 
	 * �������Ĺ��캯��
	 *
	 * @param msg Ҫ�׳����쳣������Ϣ
	 */
	DbDisConnectionFailure(const std::string& msg)
		throw()
			: DbException(msg)
	{

	}
};

/** 
 * SQL ��ѯִ��ʧ��
 */
class DbSqlQueryExecFailure : public DbException
{
public:
	/** 
	 * ��ʼ�Ĺ��캯��
	 */
	DbSqlQueryExecFailure(void)
		throw()
			: DbException("")
	{

	}

	/** 
	 * �������Ĺ��캯��
	 *
	 * @param msg Ҫ�׳����쳣������Ϣ
	 */
	DbSqlQueryExecFailure(const std::string& msg)
		throw()
			: DbException(msg)
	{

	}
};

/** 
 * �Ѿ������쳣
 */
class AlreadySetException : public std::exception
{

};

/** 
 * ȱ����ͷ�쳣
 */
class RsColumnHeadersNotSet : public std::exception
{

};

#endif
