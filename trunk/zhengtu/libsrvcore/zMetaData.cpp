/**
* \brief 表结构管理器及相关类的实现
*
* 
*/
#include <zebra/srvEngine.h>
#include "wssql.h"

#define MYSQL SQLHDBC*
typedef char **MYSQL_ROW;

using namespace Zebra;

/**
* \brief 解析函数
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
	* \brief 初始化表结构
	*
	* 建立数据库连接，并取得该数据库中所有表的表结构
	*  
	* \param url:  数据库连接串
	*/
	bool init(const std::string& url)
	{
		UrlInfo urlInfo(0,url,false);  
		//	MessageBox(NULL, "开始读取数据表.", "", 0);
		if (!this->loadMetaDataFromDB(urlInfo))
		{
			return false;
		}

		// TODO:其它一些需要初始化的代码写在这里
		//	MessageBox(NULL, "数据表读取完成.", "", 0);
		return true;
	}  

	/**
	* \brief 通过指定的连接，载入数据表结构
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
			// [ranqd] 返回当前数据库所有的用户表
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
	* \brief 加入一个新表
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
* \brief 取字段个数
*
* \return 返回字段个数
*/    
DWORD DBFieldSet::size()
{
	return fields.size();
}

/**
* \brief 重载operator[]运算符
*
* \param pos： 指定随机访问某个字段的位置 
*
* \return 如果找到该字段则返回该字段的指针，如果没找到，则返回NULL
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
* \brief 重载operator[]运算符
*
* \param pos： 指定随机访问某个字段的名称
*
* \return 如果找到该字段则返回该字段的指针，如果没找到，则返回NULL
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
* \brief 加入新的字段
*
*  字段类型目前支持以下类型:
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
* \param fieldType: 字段类型
* \param fieldName: 字段名称
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
* \brief 提供另一种添加字段的方法
*
*  重载addField
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
* \brief 解析函数
*
* 释放空间
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
* \brief 通过指定表名，获取该表的表结构
*
* \param tableName: 表名
*
* \return 如果找到该表，返回表结构指针,否则，返回为空
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
* \brief 重载operator[]运算符
*
* 可通过指定字段名，获取其该字段的值。
* 如果该字段类型为数值型，通过该函数也可返回其值，应用程序员需要自己调用相应函数进行转换
* 或者显式调用与该类型匹配的get函数
*
* \param name: 字段名。不区分大小写
* 
* \return 如果该字段存在，则返回其值。如果不存在，则返回为NULL
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
* \brief 重载operator[]运算符
*
* 通过指定列的位置获取其值，不推荐在对位置有依赖的代码中使用，因为列的位置不一定是固定的。
* 
* \param idx: 指定的位置
*
* \return 如果指定的列有值，则返回其值，否则，返回为NULL
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
* \brief 添加列
*
* \param fieldName: 字段名称
* \param value: 字段值
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
* \brief 获取指定字段的值的通用方法
* 
* 可获得所有字段类型的值，皆以字符串的形式返回其值。
* 如需按字段类型获得其值，请调用相应的get方法
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
				{// 所有整型在这里处理
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
				{//所有浮点型在这里处理`
					ret.val_float = atof(it->second.c_str());
					ret.val_double = atof(it->second.c_str());
					break;
				}
			default:
				{// 其它所有类型按字符串处理
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
* \brief 判断某个字段是否有效 
*
* \param fieldName: 字段名称

* \return 如果该记录包含该字段，返回TRUE,否则为FALSE
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
* \brief 解析方法
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
* \brief 重载operator[]运算符
*
* 通过指定的行数，获取相应的记录
*
* \param idx:指定的行数
*
* \return 如果指定的行数有效，则返回相应的记录指针，如果无效，则返回NULL
*/
DBRecord* DBRecordSet::operator[](DWORD idx)
{
	return this->get(idx);
}

/**
* \brief 获取记录数
*
* \return 返回记录数，如果没有记录，返回为0
*/
DWORD DBRecordSet::size()
{
	return recordSet.size();
}

/**
* \brief 添加记录
*
*/
void DBRecordSet::put(DBRecord* rec)
{
	recordSet.push_back(rec);
}

/**
* \brief 获取指定的行
*
* 功能与重载的operator[]运算符相同。
*/
DBRecord* DBRecordSet::get(DWORD idx)
{
	return recordSet[idx];
}

/**
* \brief builder方法，通过传入的类型描述，生成对应的实例
*
* \param 数据库类型，目前只支持MYSQL，传入时可以为空。也可以是"MYSQL"
* 
* \return  返回基类指针
*/
DBMetaData* DBMetaData::newInstance(const char* type)
{
	if (type == NULL || 0 == strcmp(type,"MYSQL"))
		return (new MySQLMetaData());
	else 
		return (new MySQLMetaData());
}

