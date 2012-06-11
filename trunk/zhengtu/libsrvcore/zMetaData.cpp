/**
* \brief ��ṹ��������������ʵ��
*
* 
*/
#include <zebra/srvEngine.h>
#include "wssql.h"

#define MYSQL SQLHDBC*
typedef char **MYSQL_ROW;

using namespace Zebra;

/**
* \brief ��������
*
*/
DBMetaData:: ~DBMetaData()
{
	TableMember it;
	for (it=tables.begin(); it!=tables.end(); it++)
	{
		DBFieldSet* temp = it->second;
		SAFE_DELETE(temp);
	}  
}

class MySQLMetaData : public DBMetaData
{
	/**
	* \brief ��ʼ����ṹ
	*
	* �������ݿ����ӣ���ȡ�ø����ݿ������б�ı�ṹ
	*  
	* \param url:  ���ݿ����Ӵ�
	*/
	bool init(const std::string& url)
	{
		UrlInfo urlInfo(0,url,false);  
		//	MessageBox(NULL, "��ʼ��ȡ���ݱ�.", "", 0);
		if (!this->loadMetaDataFromDB(urlInfo))
		{
			return false;
		}

		// TODO:����һЩ��Ҫ��ʼ���Ĵ���д������
		//	MessageBox(NULL, "���ݱ��ȡ���.", "", 0);
		return true;
	}  

	/**
	* \brief ͨ��ָ�������ӣ��������ݱ�ṹ
	*
	*/
	bool loadMetaDataFromDB(const UrlInfo& url)
	{
		SQLHDBC mysql_conn = NULL;

		mysql_conn = wsSQLInit();

		if (mysql_conn==NULL)
		{
			logger->error("Initiate mysql error...");
			return false;
		}

		if (!wsSQLConnet(mysql_conn,url.host,url.user,url.passwd,url.dbName,url.port))
		{
			logger->error("loadMetaDataFromDB():connect mysql://%s:%u/%s failed...",url.host,url.port,url.dbName);
			logger->error("loadMetaDataFromDB():reason: %s",wsSQLErrorMsg(mysql_conn));
			return false;
		}

		logger->info("loadMetaDataFromDB():connect mysql://%s:%u/%s successful...",url.host,url.port,url.dbName);

		if (mysql_conn)
		{
			SQLHSTMT hstmt;
			SQLRETURN retcode = SQLAllocHandle((SQLSMALLINT)SQL_HANDLE_STMT, mysql_conn, &hstmt);
			// [ranqd] ���ص�ǰ���ݿ����е��û���
			retcode = SQLTables( hstmt,NULL,0,NULL,0,NULL,0,NULL,0 );
			if( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
			{
				logger->error("loadMetaDataFromDB():SQLTables fail.");
				logger->error("loadMetaDataFromDB():reason: %s",wsSQLErrorMsg(hstmt));
				SQLFreeStmt( hstmt, SQL_DROP );
				SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
				SQLFreeHandle(SQL_HANDLE_DBC, &mysql_conn);
				return false;
			}
			SQLCHAR TableName[MAX_PATH], TableType[MAX_PATH];
			SQLINTEGER IndexInd = 0;
			while( SQLFetch(hstmt) != SQL_NO_DATA)
			{
				SQLGetData(hstmt, 3, SQL_C_CHAR, TableName, sizeof(TableName), &IndexInd);
				SQLGetData(hstmt, 4, SQL_C_CHAR, TableType, sizeof(TableType), &IndexInd);
				if( strcmp( (const char*)TableType, "SYSTEM TABLE") != 0 )
					this->addNewTable(url, (const char*)TableName);
			}
			SQLFreeStmt( hstmt, SQL_DROP );
			SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
		}

		SQLFreeHandle(SQL_HANDLE_DBC, &mysql_conn);

		return true;
	}

	/**
	* \brief ����һ���±�
	*/
	bool addNewTable(const UrlInfo& url, const char* tableName)
	{	
		SQLHDBC mysql_conn = NULL;

		mysql_conn = wsSQLInit();

		if (mysql_conn==NULL)
		{
			logger->error("Initiate mysql error...");
			return false;
		}

		if (!wsSQLConnet(mysql_conn,url.host,url.user,url.passwd,url.dbName,url.port))
		{
			logger->error("addNewTable():connect mysql://%s:%u/%s failed...",url.host,url.port,url.dbName);
			logger->error("addNewTable():reason: %s",wsSQLErrorMsg(mysql_conn));
			return false;
		}

		SQLHSTMT hstmt;
		SQLRETURN retcode = SQLAllocHandle((SQLSMALLINT)SQL_HANDLE_STMT, mysql_conn, &hstmt);
		retcode = SQLColumns(hstmt, NULL, 0, NULL, 0, (SQLCHAR*)tableName, SQL_NTS, NULL, 0);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			logger->error(wsSQLErrorMsg(hstmt));
			return false;
		}
		SQLCHAR ColumnName[MAX_PATH], typeName[MAX_PATH];
		int     ColumnType;
		SQLINTEGER IndexInd = 0;

		DBFieldSet *fields = new DBFieldSet(tableName);
		int n = 0;
		SQLBindCol( hstmt, 4, SQL_C_CHAR, ColumnName, sizeof(ColumnName), &IndexInd);
		SQLBindCol( hstmt, 5, SQL_C_LONG, &ColumnType, 0, &IndexInd);
		SQLBindCol( hstmt, 6, SQL_C_CHAR, typeName, sizeof(typeName), &IndexInd);
		while( SQLFetch(hstmt) != SQL_NO_DATA )
		{
			fields->addField( ColumnType, string((char*)ColumnName) );
			n ++;
		}
		tables.insert(valueType(tableName,fields));
		SQLFreeStmt( hstmt, SQL_DROP );
		SQLFreeHandle( SQL_HANDLE_DBC, mysql_conn );
		return true;
	}
};

/**
* \brief ȡ�ֶθ���
*
* \return �����ֶθ���
*/    
DWORD DBFieldSet::size()
{
	return fields.size();
}

/**
* \brief ����operator[]�����
*
* \param pos�� ָ���������ĳ���ֶε�λ�� 
*
* \return ����ҵ����ֶ��򷵻ظ��ֶε�ָ�룬���û�ҵ����򷵻�NULL
*/
DBField* DBFieldSet::operator[] (DWORD pos)
{
	if (pos<0 || pos>=fields.size())
	{
		return NULL;
	}

	return fields[pos];
}

/**
* \brief ����operator[]�����
*
* \param pos�� ָ���������ĳ���ֶε�����
*
* \return ����ҵ����ֶ��򷵻ظ��ֶε�ָ�룬���û�ҵ����򷵻�NULL
*/
DBField* DBFieldSet::operator[](const std::string& name)
{
	for (DWORD i=0; i<fields.size(); i++)
	{
		DBField* ret = fields.at(i);

		if (ret)
		{
			if (ret->name == name)
			{
				return ret;
			}
		}
	}

	return NULL;
}


/**
* \brief �����µ��ֶ�
*
*  �ֶ�����Ŀǰ֧����������:
*
*  FIELD_TYPE_TINY TINYINT field
*  FIELD_TYPE_SHORT SMALLINT field
*  FIELD_TYPE_LONG INTEGER field
*  FIELD_TYPE_INT24 MEDIUMINT field
*  FIELD_TYPE_LONGLONG BIGINT field
*  FIELD_TYPE_DECIMAL DECIMAL or NUMERIC field
*  FIELD_TYPE_FLOAT FLOAT field
*  FIELD_TYPE_DOUBLE DOUBLE or REAL field
*  FIELD_TYPE_TIMESTAMP TIMESTAMP field
*  FIELD_TYPE_DATE DATE field
*  FIELD_TYPE_TIME TIME field
*  FIELD_TYPE_DATETIME DATETIME field
*  FIELD_TYPE_YEAR YEAR field
*  FIELD_TYPE_STRING CHAR field
*  FIELD_TYPE_VAR_STRING VARCHAR field
*  FIELD_TYPE_BLOB BLOB or TEXT field 
*  FIELD_TYPE_SET SET field
*  FIELD_TYPE_ENUM ENUM field
*  FIELD_TYPE_NULL NULL-type field
*  FIELD_TYPE_CHAR Deprecated; use FIELD_TYPE_TINY instead
*
* \param fieldType: �ֶ�����
* \param fieldName: �ֶ�����
*
*
*/
bool DBFieldSet::addField(int fieldType,const std::string& fieldName)
{
	std::string tempname = fieldName;

	transform(tempname.begin(),tempname.end(),
		tempname.begin(),
		toupper);

	DBField* field = new DBField(fieldType,tempname);

	if (field)
	{
		fields.push_back(field);
		return true;
	}

	return false;
}

/**
* \brief �ṩ��һ������ֶεķ���
*
*  ����addField
*/
bool DBFieldSet::addField(DBField* field)
{
	if (!field)
	{
		fields.push_back(field);
		return true;
	}

	return false;
}

/**
* \brief ��������
*
* �ͷſռ�
*/
DBFieldSet::~DBFieldSet()
{
	DWORD num_field = fields.size(); 

	for (DWORD i=0; i<num_field; i++)
	{
		SAFE_DELETE(fields[i]);
	}

}
/**
* \brief ͨ��ָ����������ȡ�ñ�ı�ṹ
*
* \param tableName: ����
*
* \return ����ҵ��ñ����ر�ṹָ��,���򣬷���Ϊ��
*/
DBFieldSet* DBMetaData::getFields(const std::string& tableName)
{
	std::string tempname = tableName;

	//free table name under windows is lower
	transform(tempname.begin(),tempname.end(),
		tempname.begin(),
		tolower);

	TableMember tm = tables.find(tempname);  
	if (tm!=tables.end())
	{
		return (DBFieldSet*)(tm->second);
	}

	return NULL;
}

/**
* \brief ����operator[]�����
*
* ��ͨ��ָ���ֶ�������ȡ����ֶε�ֵ��
* ������ֶ�����Ϊ��ֵ�ͣ�ͨ���ú���Ҳ�ɷ�����ֵ��Ӧ�ó���Ա��Ҫ�Լ�������Ӧ��������ת��
* ������ʽ�����������ƥ���get����
*
* \param name: �ֶ����������ִ�Сд
* 
* \return ������ֶδ��ڣ��򷵻���ֵ����������ڣ��򷵻�ΪNULL
*/
DBVarType DBRecord::operator[](const std::string& name)
{
	/*  std::string tempname = name;

	transform(tempname.begin(),tempname.end(),
	tempname.begin(),
	toupper);

	field_it it = field.find(tempname);


	if (it != field.end())
	{
	return it->second.c_str();
	}

	return NULL;*/
	return this->get(name);
}

/**
* \brief ����operator[]�����
*
* ͨ��ָ���е�λ�û�ȡ��ֵ�����Ƽ��ڶ�λ���������Ĵ�����ʹ�ã���Ϊ�е�λ�ò�һ���ǹ̶��ġ�
* 
* \param idx: ָ����λ��
*
* \return ���ָ��������ֵ���򷵻���ֵ�����򣬷���ΪNULL
*/
const char* DBRecord::operator[](DWORD idx)
{
	field_it it;
	DWORD i=0;

	for (it = field.begin(); it!=field.end(); i++,it++)
	{
		if (idx == i)
		{
			return it->second.c_str();  
		}
	}

	return NULL;
}

/**
* \brief �����
*
* \param fieldName: �ֶ�����
* \param value: �ֶ�ֵ
* 
*/
void DBRecord::put(const char* fieldName)
{
	if (fieldName == NULL)
	{
		return;
	}

	std::string tempname = fieldName;

	transform(tempname.begin(),tempname.end(),
		tempname.begin(),
		toupper);


	field.insert(valType(tempname,""));

}

/**
* \brief ��ȡָ���ֶε�ֵ��ͨ�÷���
* 
* �ɻ�������ֶ����͵�ֵ�������ַ�������ʽ������ֵ��
* ���谴�ֶ����ͻ����ֵ���������Ӧ��get����
*/
DBVarType DBRecord::get(const std::string& fieldName)
{
	std::string tempname = fieldName;
	DBVarType ret;

	transform(tempname.begin(),tempname.end(),
		tempname.begin(),
		toupper);

	field_it it = field.find(tempname);

	if (it == field.end())
	{
		return ret;
	}

	if (fields)
	{
		DBField* fl = (*fields)[tempname];

		if (fl)
		{
			ret.setValid(true);

			switch (fl->type)
			{
			case SQL_NUMERIC:
			case SQL_DECIMAL:
			case SQL_INTEGER:
			case SQL_SMALLINT:
				{// �������������ﴦ��
					ret.val_us = atoi(it->second.c_str());
					ret.val_short = atoi(it->second.c_str());
					ret.val_int = atoi(it->second.c_str());
					ret.val_dword = strtoul(it->second.c_str(),NULL,10);
					ret.val_qword = strtoul(it->second.c_str(),NULL,10);
					ret.val_sqword = strtoul(it->second.c_str(),NULL,10);
					ret.val_long = atol(it->second.c_str());
					ret.val_byte = atoi(it->second.c_str());
					break;
				}
			case SQL_FLOAT:
			case SQL_REAL:
			case SQL_DOUBLE:
				{//���и����������ﴦ��`
					ret.val_float = atof(it->second.c_str());
					ret.val_double = atof(it->second.c_str());
					break;
				}
			default:
				{// �����������Ͱ��ַ�������
					ret.val_pstr = it->second.c_str();
				}
			}
		}
	}
	else
	{
		ret.setValid(true);
		ret.val_pstr = it->second.c_str();
	}

	return ret;
}

/**
* \brief �ж�ĳ���ֶ��Ƿ���Ч 
*
* \param fieldName: �ֶ�����

* \return ����ü�¼�������ֶΣ�����TRUE,����ΪFALSE
*/
bool DBRecord::find(const std::string& fieldName)
{
	std::string tempname = fieldName;

	transform(tempname.begin(),tempname.end(),
		tempname.begin(),
		toupper);

	if (field.find(tempname) == field.end())
	{
		return false;
	}

	return true;

}

/**
* \brief ��������
*/
DBRecordSet::~DBRecordSet()
{
	//  DWORD num_record = recordSet.size();
	//  std::cout << "~DBRecordSet" << std::endl;
	for (std::vector<DBRecord*>::iterator pos = recordSet.begin(); pos!=recordSet.end(); pos++)
	{
		SAFE_DELETE(*pos);
	}
}

/**
* \brief ����operator[]�����
*
* ͨ��ָ������������ȡ��Ӧ�ļ�¼
*
* \param idx:ָ��������
*
* \return ���ָ����������Ч���򷵻���Ӧ�ļ�¼ָ�룬�����Ч���򷵻�NULL
*/
DBRecord* DBRecordSet::operator[](DWORD idx)
{
	return this->get(idx);
}

/**
* \brief ��ȡ��¼��
*
* \return ���ؼ�¼�������û�м�¼������Ϊ0
*/
DWORD DBRecordSet::size()
{
	return recordSet.size();
}

/**
* \brief ��Ӽ�¼
*
*/
void DBRecordSet::put(DBRecord* rec)
{
	recordSet.push_back(rec);
}

/**
* \brief ��ȡָ������
*
* ���������ص�operator[]�������ͬ��
*/
DBRecord* DBRecordSet::get(DWORD idx)
{
	return recordSet[idx];
}

/**
* \brief builder������ͨ��������������������ɶ�Ӧ��ʵ��
*
* \param ���ݿ����ͣ�Ŀǰֻ֧��MYSQL������ʱ����Ϊ�ա�Ҳ������"MYSQL"
* 
* \return  ���ػ���ָ��
*/
DBMetaData* DBMetaData::newInstance(const char* type)
{
	if (type == NULL || 0 == strcmp(type,"MYSQL"))
		return (new MySQLMetaData());
	else 
		return (new MySQLMetaData());
}

