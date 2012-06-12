#pragma once
#ifndef _INC_SRVENGINE_H_
#define _INC_SRVENGINE_H_

#include <zebra/csCommon.h>
//#include <sys/types.h>
#include "baseLib/regex.h"
#include <zlib.h>
#include <libxml/parser.h>
#include "baseLib/timeLib.h"
//#include "baseLib/gcchash.h"
//#include "baseLib/gccmt_allocator.h"
//#include <string.h>
#include <assert.h>
//#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <queue>
//#include <xhash>
#include <hash_map>
//#include <hash_multimap>
#include <functional>

extern long g_RecvSize;
extern long g_SendSize;
extern long g_WantSendSize;
extern DWORD g_SocketSize; 


inline void gotoxy(int x,int y) 
{ 
	COORD c; 

	c.X=x;c.Y=y; 
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition (hOut,c); 

} 
// [ranqd Add] 返回地址效验宏 
#define CALL_CHECK_BEGIN(n)  \
	unsigned long ret_add = 0;\
	int   oft = n * 4;\
	{\
		_asm	mov edx,oft \
		_asm	add edx,esp \
		_asm	mov eax,[edx] \
		_asm	mov ret_add,eax\
	}
	
#define __CALL_CHECK_END(file, line, func ) {\
	unsigned long ret_add2 = 0;\
	{\
		_asm	mov edx,oft \
		_asm	add edx,esp \
		_asm	mov eax,[edx] \
		_asm	mov ret_add2,eax \
	}\
	if( ret_add != ret_add2 )\
		{\
		char tmpc[1024];\
		sprintf(tmpc, "%s(%d)函数:%s 返回时地址已改变，可能栈已损坏！", file,line,func);\
		MessageBox(NULL,tmpc,"地址效验失败",MB_ICONERROR);\
		}\
	}
#define __CALL_CHECK_END_(file, line, func ) {\
	unsigned long ret_add2 = 0;\
	{\
	_asm	mov edx,oft \
	_asm	add edx,esp \
	_asm	mov eax,[edx] \
	_asm	mov ret_add2,eax \
	}\
	if( ret_add != ret_add2 )\
	{\
		char tmpc[1024];\
		sprintf(tmpc, "%s(%d)函数:%s 返回时地址已改变，可能栈已损坏！", file,line,func);\
		MessageBox(NULL,tmpc,"地址效验失败",MB_ICONERROR);\
	}

#define CALL_CHECK_END __CALL_CHECK_END( __FILE__,__LINE__,__FUNCTION__ )
#define CALL_CHECK_END_ __CALL_CHECK_END_( __FILE__,__LINE__,__FUNCTION__ )

#define CRETURN(x) CALL_CHECK_END_; return x;}
#define CRETURN    CALL_CHECK_END_; return;}

#define USE_IOCP true // [ranqd] 是否使用IOCP收发数据

// [ranqd] 包头格式定义
struct PACK_HEAD
{
	unsigned char Header[2];
	unsigned short Len;
	PACK_HEAD()
	{
		Header[0] = 0xAA;
		Header[1] = 0xDD;
	}
};
// [ranqd] 包尾格式定义
struct PACK_LAST
{
	unsigned char Last;
	PACK_LAST()
	{
		Last = 0xAA;
	}
};

#define PACKHEADLASTSIZE (sizeof(PACK_HEAD))

#define PACKHEADSIZE    sizeof(PACK_HEAD)

#define PACKLASTSIZE    0

template <class T>
class __mt_alloc
{
	T memPool[2046];
public:
	char * allocate(size_t  len){return (char*)malloc(len);}

	void deallocate(unsigned char* ptr,size_t len)
	{
		free(ptr);
	}


};

template <typename T>
class SingletonBase
{
public:
	SingletonBase() {}
	virtual ~SingletonBase() {}
	static T& getInstance()
	{
		assert(instance);
		return *instance;
	}
	static void newInstance()
	{
		SAFE_DELETE(instance);
		instance = new T();
	}
	static void delInstance()
	{
		SAFE_DELETE(instance);
	}
protected:
	static T* instance;
private:
	SingletonBase(const SingletonBase&);
	SingletonBase & operator= (const SingletonBase &);
};
template <typename T> T* SingletonBase<T>::instance = NULL;

using namespace std;
using namespace stdext;

//#include <ext/hash_map>
//#include <ext/pool_allocator.h>
////#include <ext/mt_allocator.h>

class zLogger;
class zProperties;
class zCond;
class zMutex;
class zMutex_scope_lock;
class zRWLock;
class zRWLock_scope_rdlock;
class zRWLock_scope_wrlock;
class zThread;
class zThreadGroup;
class zTCPTask;
class zTCPTaskPool;
class zSyncThread;
class zRecycleThread;
class zCheckconnectThread;
class zCheckwaitThread;
class zTCPClientTaskThread;

/**
* \brief 字段信息类
*
* 维护字段名称及类型，目前系统只能处理MYSQL的所有标准类型，
*
* 还不支持Zebra中定义的ZIP,ZIP2两种类型
* 如需支持更多的字段类型，请修改zMysqlDBConnPool::exeSelect,exeUpdate,exeInsert,exeDelete四个方法
*
*/

class DBField
{
public:
	/**
	* \brief 构造方法
	*
	*
	*/
	DBField(int fieldType,const std::string& fieldName) 
		: type(fieldType),name(fieldName)
	{
	}

	/**
	* \ brief 解析方法
	*
	*/
	~DBField(){}

	/// 字段类型
	int type;

	/// 字段名称
	std::string name;
};


/**
* \brief 字段容器
*
* 维护一个表的所有字段
*
*/
class DBFieldSet
{
public:
	/**
	* \brief 默认构造方法
	*
	*/
	DBFieldSet(){}


	/**
	* \brief 初始化表名的构造方法
	*
	* 不支持显式类型转换
	*/
	explicit DBFieldSet(const std::string& tbn) : tableName(tbn)
	{
	}


	/**
	* \brief 解析函数
	*
	*/
	virtual ~DBFieldSet();

	/**
	* \brief 取字段个数
	*
	* \return 返回字段个数
	*/    
	DWORD size();


	/**
	* \brief 重载operator[]运算符
	*
	* \param pos： 指定随机访问某个字段的位置 
	*
	* \return 如果找到该字段则返回该字段的指针，如果没找到，则返回NULL
	*/
	DBField* operator[](DWORD pos);


	/**
	* \brief 重载operator[]运算符
	*
	* \param name： 指定随机访问某个字段的名称
	*
	* \return 如果找到该字段则返回该字段的指针，如果没找到，则返回NULL
	*/
	DBField* operator[](const std::string& name);



	/**
	* \brief 取指定位置的字段的方法
	*
	* \param pos: 指定随机访问某个字段的位置
	*
	* \return 如果找到该字段则返回该字段的指针，如果没找到，则返回NULL
	*/
	DBField* getField(DWORD pos)
	{
		CALL_CHECK_BEGIN(1)
		return fields[pos];
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
	bool addField(int fieldType,const std::string& fieldName);

	/**
	* \brief 提供另一种添加字段的方法
	*
	*  重载addField
	*/
	bool addField(DBField* field);

	/**
	* \brief 设置该字段集与之关联的表名
	*
	* 如通过默认构造函数生成的对象，则必须显式调用该函数进行设置
	*
	* \param name：表名
	*/
	void setTableName(const std::string& name);

	/**
	* \brief 得到表名
	*
	* \return 返回表名
	*/
	const char* getTableName() const
	{
		return tableName.c_str();  
	}

protected:
	/// 字段容器
	std::vector<DBField*> fields;

	/// 表名
	std::string tableName;  
};


/**
* \brief 表结构管理器
*
* 该类采用Builder模式，以支持不同数据库的表结构管理
*
*/
class DBMetaData
{
public:
	/**
	* \brief builder方法，通过传入的类型描述，生成对应的实例
	*
	* \param type 数据库类型，目前只支持MYSQL，传入时可以为空。也可以是"MYSQL"
	* 
	* \return  返回基类指针
	*/
	static DBMetaData* newInstance(const char* type);

	/**
	* \brief 默认构造函数
	*
	*/
	DBMetaData()
	{
	}

	/**
	* \brief 解析函数
	*
	*/
	virtual ~DBMetaData();

	/**
	* \brief 初始化表结构
	*
	* 建立数据库连接，并取得该数据库中所有表的表结构
	*  
	* \param url:  数据库连接串
	*/
	virtual bool init(const std::string& url) = 0;

	/**
	* \brief 通过指定表名，获取该表的表结构
	*
	* \param tableName: 表名
	*
	* \return 如果找到该表，返回表结构指针,否则，返回为空
	*/
	DBFieldSet* getFields(const std::string& tableName);

protected:

	typedef std::map<std::string,DBFieldSet*> TableManager;
	typedef TableManager::iterator TableMember;
	typedef TableManager::value_type valueType;

	/// 内部表结构管理器
	TableManager tables;
};


class DBVarType
{
public:
	DBVarType()
	{
		val_us = 0;
		val_short = 0;
		val_int = 0;
		val_dword = 0;
		val_qword = 0;
		val_sqword = 0;
		val_long = 0;

		val_float = 0.0;
		val_double = 0.0;
		val_byte = 0;

		val_pstr = NULL;
		valid = false;
	}

	operator WORD() const
	{
		return val_us;
	}

	operator short() const
	{
		return val_short;
	}

	operator int() const
	{
		return val_int;
	}

	operator DWORD() const
	{
		return val_dword;
	}

	operator QWORD() const
	{
		return val_qword;
	}

	operator SQWORD() const
	{
		return val_sqword;
	}

	operator long() const
	{
		return val_long;
	}

	operator float() const
	{
		return val_float;
	}

	operator double() const
	{
		return val_double;
	}


	operator const char*() const
	{
		return val_pstr;
	}

	operator BYTE() const
	{
		return val_byte;
	}

	void setValid(bool value)
	{
		valid = value;
	}

	bool isValid()
	{
		return valid;
	}

	WORD val_us;
	short val_short;
	int val_int;
	DWORD val_dword;
	QWORD val_qword;
	SQWORD val_sqword;
	long val_long;
	float val_float;
	double val_double;
	BYTE val_byte;

	const char* val_pstr;
	//const BYTE* val_ustr;

	bool valid;
};
/**
* \brief 记录类
*
* 维护一条数据库记录
*/
class DBRecord
{
public:
	/**
	* \brief 默认构造方法
	*/
	DBRecord()
	{
		//      std::cout << "DBRecord" << std::endl;
		fields = NULL;
	}

	/**
	* \brief 解析方法
	*/
	virtual ~DBRecord()
	{
		//      std::cout << "~DBRecord" << std::endl;
		field.clear();
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
	DBVarType operator[](const std::string& name);



	/**
	* \brief 重载operator[]运算符
	*
	* 通过指定列的位置获取其值，不推荐在对位置有依赖的代码中使用，因为列的位置不一定是固定的。
	* 
	* \param idx: 指定的位置
	*
	* \return 如果指定的列有值，则返回其值，否则，返回为NULL
	*/
	const char* operator[](DWORD idx);

	/**
	* \brief 添加列
	*
	* 注意：第二个参数，绝不允许为NULL值，否则会导致程序崩溃
	*
	* \param fieldName: 字段名称
	* \param value: 字段值
	* 
	*/
	template <typename X>
	void put(const char* fieldName,const X& value,bool binary = false)
	{
		if (fieldName == NULL)
		{
			return;
		}

		std::ostringstream oss;
		std::string tempname = fieldName;

		std::transform(tempname.begin(),tempname.end(),
			tempname.begin(),
			::toupper);

		if (binary)
		{

		}
		else
		{
			oss << value;
		}

		field_it member = field.find(tempname);

		if (member==field.end())
		{
			field.insert(valType(tempname,oss.str()));
		}
		else
		{
			field.erase(member);
			field.insert(valType(tempname,oss.str()));
		}

	}

	/**
	* \brief 添加列
	*
	* 注意：第二个参数，绝不允许为NULL值，否则会导致程序崩溃
	*
	* \param fieldName: 字段名称
	* \param value: 字段值
	* 
	*/
	/*template <> void put<const char*>(const char* fieldName,const char* value)
	{
	if (fieldName == NULL)
	{
	return;
	}

	std::ostringstream oss;
	std::string tempname = fieldName;

	std::transform(tempname.begin(),tempname.end(),
	tempname.begin(),
	::toupper);

	if (value)
	{
	oss << value;
	}

	field_it member = field.find(tempname);

	if (member==field.end())
	{
	field.insert(valType(tempname,oss.str()));
	}
	else
	{
	field.erase(member);
	field.insert(valType(tempname,oss.str()));
	}

	}*/

	/**
	* \brief 添加列
	*
	*  主要用于当这个DBRecord用做,SELECT时的column,groupby子句。
	*  添加一个列，但这个列的值为空
	*
	* \param fieldName: 字段名称
	* 
	*/
	void put(const char* fieldName);

	/**
	* \brief 清空所有列
	*
	*/
	void clear()
	{
		field.clear();
	}

	/**
	* \brief 获取指定字段的值的通用方法
	* 
	* 可获得所有字段类型的值，皆以字符串的形式返回其值。
	* 如需按字段类型获得其值，请调用相应的get方法
	*/
	DBVarType get(const std::string& fieldName);


	//bool getBool(const std::string& fieldName);
	//double getDouble(const std::string& fieldName);
	//int    getInt(const std::string& fieldName);

	/**
	* \brief 判断某个字段是否有效 
	*
	* \param fieldName: 字段名称

	* \return 如果该记录包含该字段，返回TRUE,否则为FALSE
	*/
	bool find(const std::string& fieldName);

	/**
	* \brief 获取该记录的列数
	*
	* \return 返回该记录的列数，为0表示没有列。
	*/
	DWORD size()
	{
		return field.size();
	}


	DBFieldSet* fields;

private:
	typedef std::map<std::string,std::string> FIELD;
	typedef FIELD::value_type valType;
	typedef FIELD::iterator field_it;

	/// 字段-值对
	FIELD field;
};


class DBRecordSet
{
public:
	/**
	* \brief 默认构造函数
	*
	*/
	DBRecordSet()
	{
		//      std::cout << "DBRecordSet" << std::endl;
	}

	/**
	* \brief 解析函数
	*
	*/
	virtual ~DBRecordSet();

	/**
	* \brief 重载operator[]运算符
	*
	* 通过指定的行数，获取相应的记录
	*
	* \param idx:指定的行数
	*
	* \return 如果指定的行数有效，则返回相应的记录指针，如果无效，则返回NULL
	*/
	DBRecord* operator[](DWORD idx);

	/**
	* \brief 获取记录数
	*
	* \return 返回记录数，如果没有记录，返回为0
	*/
	DWORD size();

	/**
	* \brief 获取记录数
	*
	* \return 返回记录数，如果没有记录，返回为0
	*/
	bool empty(){return recordSet.empty();}

	/**
	* \brief 添加记录
	*
	*/
	void put(DBRecord* rec);


	/**
	* \brief 清空所有记录
	*
	*/
	void clear()
	{
		recordSet.clear();
	}

	/**
	* \brief 获取指定的行
	*
	* 功能与重载的operator[]运算符相同。
	*/
	DBRecord* get(DWORD idx);

private:
	/// 记录集
	std::vector<DBRecord*>    recordSet;
};

#define MAX_HOSTSIZE  32
#define MAX_USERSIZE  32
#define MAX_DBSIZE    32

/**
*
* \brief 数据库字段描述结构类型定义
* 
* 本结构描述了要操作的数据的数据库类型字段,描述的数组必须以{NULL,0，0}作为描述结束标记。
*
* 注意：如果type是DB_BIN2或者DB_ZIP2，那么大小必须是缓冲的最大大小。
* 
* 例子：
*
*  dbCol mycountCol_define[]=
*
*  {
*
*    {"COUNT(*)",DB_DWORD,sizeof(DWORD)},
*
*    {NULL,0,0}
*
*  };
*/
typedef struct
{
	const char *name;  /**< 字段名字 */
	int type;      /**< ZEBRA数据类型 */
	DWORD size;  /**< 数据大小 */
} dbCol;

class DBRecord;
class DBFieldSet;
class DBRecordSet;
/**
* \brief 哈希代码函数类型定义
* 
* 用户可以根据自己的需要写自己的哈希函数，以便对相对应用户定义的数据库进行操作。
*/
typedef DWORD(* hashCodeFunc)(const void *data);

/**
* \brief 连接句柄,用户调用使用,只能从链接池中得到
*/
typedef DWORD connHandleID;

/**
* \brief 数据链接池接口定义
*
* 本类提供了对数据库的简单的基本访问,比如UPDATE,INSERT,SELECT,DELETE,执行SQL语句等.
*
*  用户只需要定义要操作的数据库数据，即可访问。
*
* 如果选用不同数据库,必须实现这个接口,目前提供了Mysql的实现.
*/
class zDBConnPool
{
public:
	/**
	* \brief 数据库支持的数据类型
	*/
	enum
	{
		DB_BYTE,    /**< BYTE类型 1字节长度 */
		DB_CHAR,    /**< CHAR类型 1字节长度 */
		DB_WORD,    /**< WORD类型 2字节长度 */
		DB_DWORD,    /**< DWORD类型 4字节长度 */
		DB_QWORD,    /**< QWORD类型 8字节长度 */
		DB_STR,      /**< 字符串类型 */
		DB_BIN,      /**< 二进制数据类型 */
		DB_ZIP,      /**< zip压缩数据类型 */
		DB_BIN2,    /**< 扩展二进制数据类型 */
		DB_ZIP2      /**< 扩展zip压缩数据类型 */
	};

	/**
	* \brief 新建立一个链接池的实例
	*
	* 本接口没有默认的构造函数，为了使用者无需关心底层数据库的差异。提供此接口。
	* \param hashfunc 哈希函数指针，如果为NULL，接口实现者应该提供默认函数。
	* \return 返回这个接口的一个实例，错误返回NULL。
	*/
	static zDBConnPool *newInstance(hashCodeFunc hashfunc);

	/**
	* \brief 回收一个链接池的实例
	*
	* 本接口没有默认的析构函数，为了使用者无需关心底层数据库的差异。提供此接口。
	* \param delThisClass 要回收的链接池实例。
	*/
	static void delInstance(zDBConnPool **delThisClass);

	/**
	* \brief 根据数据字段描述计算数据字段的大小
	*
	* \param column 数据字段描述指针。
	* \return 数据字段大小。
	*/
	static DWORD getColSize(dbCol* column);

	/**
	* \brief 得到数据字段的类型字符串 
	* \param type 类型
	* \return 类型字符串
	*/
	static const char *getTypeString(int type);

	/**
	* \brief 打印出数据类型定义描述  
	* \param column 数据定义指针
	*/
	static void dumpCol(const dbCol *column);

	/**
	* \brief 接口析构虚函数
	*/
	//virtual ~zDBConnPool(){};
	virtual ~zDBConnPool()
	{
	};

	/**
	* \brief 向连接池中添加数据库连接URL，并设置此连接是否支持事务
	*
	* \param hashcode 此连接所对应的哈希代码，使用者需要指定。
	* \param url 数据库联接的url
	* \param supportTransactions 此连接是否只是事务
	* \return 成功返回true，否则返回false
	*/
	virtual bool putURL(DWORD hashcode,const char *url,bool supportTransactions) =0;

	/**
	* \brief 根据hashData得到连接Handle
	* 
	* \param hashData 链接池用此参数作为调用hashCodeFunc的参数，计算hashcode，用来得到相应的数据库联接。
	* \return 数据库联接句柄,-1表示无效句柄
	*/
	virtual connHandleID getHandle(const void *hashData=NULL) =0;

	/**
	* \brief 根据当前Handle得到下一个Handle用来遍历所有不同URL的db连接
	*
	* \param handleID 当前的链接句柄
	* \return 下一个链接句柄，-1表示没有不同连接句柄了
	*/
	virtual connHandleID getNextHandle(connHandleID handleID) =0;

	/**
	* \brief 将Handle放回连接池
	*
	* 用户在使用完数据库联接句柄后，应该将其放回链接池，已备下次使用。
	* \param handleID 放回链接池的链接句柄
	*/
	virtual void putHandle(const connHandleID handleID) =0;

	/**
	* \brief 执行Sql语句,返回db_real_query的返回结果
	*
	* 为了提供更灵活的数据库操作，提供了本函数
	* \param handleID 操作的链接句柄
	* \param sql 要执行的SQL语句
	* \param sqllen SQL语句的长度
	* \return 返回执行数据语句后的代码，根具体的数据库的返回值有关
	*/
	virtual int execSql(connHandleID handleID,const char *sql,DWORD sqllen) =0;

	/**
	* \brief 执行SELECT SQL
	*
	* \param handleID 操作的链接句柄
	* \param tableName 要操作的表名
	* \param column 要操作的数据字段描述，以{NULL,0,0}为结尾标记
	* \param where SQL的where表达式,没有时用NULL
	* \param order SQL的order表达式,没有时用NULL
	* \param data SELECT后的结果数据存储的位置，如果返回值大于0，调用者应该释放*data内存空间
	* \return 返回值为结果的个数，如果错误返回-1
	*/
	virtual DWORD exeSelect(connHandleID handleID,const char* tableName,
		const dbCol *column,const char *where,const char *order,BYTE **data) =0;

	/**
	* \brief 执行SELECT SQL
	*
	*  使用方法可参见test/NewMySQLTest中的使用代码例程 
	*
	* \param handleID 操作的链接句柄
	* \param table 要操作的表结构对象，通过DBMetaData.getFields取得
	* \param column 要操作的数据字段描述，不指定时为返回所有字段"*" 
	* \param where SQL的where描述,没有时用NULL
	* \param order SQL的order描述,没有时用NULL，可不填写
	* \param limit 返回结果的最大限制，为0时，为不限制，或可不填写
	* \param groupby SQL中的GROUPBY子句描述，未有时，可不填写。也可填为NULL
	* \param having SQL中的HAVING子句描述，未有时，可不填写，也可填为NULL
	*
	* \return 返回结果集
	*
	*/

	virtual DBRecordSet* exeSelect(connHandleID handleID,DBFieldSet* table,DBRecord* column,
		DBRecord* where,DBRecord* order = NULL,
		DWORD limit=0,
		DBRecord* groupby = NULL,DBRecord* having = NULL) = 0;

	/*virtual DWORD  exeSelect(connHandleID handleID,DBRecordSet* result,DBFieldSet* table,DBRecord* column,
	DBRecord* where,DBRecord* order = NULL,
	DWORD limit=0,
	DBRecord* groupby = NULL,DBRecord* having = NULL) = 0;*/



	/**
	* \brief 执行SELECT SQL,并限制返回结果的个数
	* 
	* \param handleID 操作的链接句柄
	* \param tableName 要操作的表名
	* \param column 要操作的数据字段描述，以{NULL,0,0}为结尾标记
	* \param where SQL的where表达式,没有时用NULL
	* \param order SQL的order表达式,没有时用NULL
	* \param limit 返回结果的最大限制
	* \param data SELECT后的结果数据存储的位置,data应该有足够的空间存储返回的结果
	* \return 返回值为结果的个数，如果错误返回-1
	*/
	virtual DWORD exeSelectLimit(connHandleID handleID,const char* tableName,
		const dbCol *column,const char *where,const char *order,DWORD limit,BYTE *data,DWORD limit_from = 0) =0;

	/**
	* \brief 执行SELECT SQL,并限制返回结果的个数
	* 
	* \param handleID 操作的链接句柄
	* \param sql 标准查询sql语句
	* \param sqlen sql长度
	* \param cloumn 需尧返回的列结构
	* \param limit 返回结果的最大限制
	* \param data SELECT后的结果数据存储的位置,data应该有足够的空间存储返回的结果
	* \return 返回值为结果的个数，如果错误返回-1
	*/
	virtual DWORD execSelectSql(connHandleID handleID,const char *sql,
		DWORD sqllen,const dbCol *column,DWORD limit,BYTE *data)=0;
	/**
	* \brief 将data添加进数据库
	*
	* 本函数保证是原子操作
	*
	* \param handleID 操作的链接句柄
	* \param tableName 要操作的表名
	* \param column 要操作的数据字段描述，以{NULL,0,0}为结尾标记
	* \param data 要操作的数据字段
	* \return 插入数据的描述返回值为插入语句执行后，为AUTO_INCREMENT column所产生的ID,如果为-1表示函数错误
	* 显然如果本次插入没有AUTO_INCREMENT column,返回值大于等于0是没有意义的
	*/
	virtual DWORD exeInsert(connHandleID handleID,const char *tableName,const dbCol *column,const BYTE *data) =0;

	/**
	* \brief 插入一条记录
	*
	* 本函数保证是原子操作
	*
	* \param handleID 操作的链接句柄
	* \param table 要操作的表结构，通过DBMetaData::getFields获得
	* \param rec 要操作的数据字段
	*
	* \return 插入数据的描述返回值为插入语句执行后，为AUTO_INCREMENT column所产生的ID,如果为-1表示函数错误
	* 显然如果本次插入没有AUTO_INCREMENT column,返回值大于等于0是没有意义的
	*/
	virtual DWORD exeInsert(connHandleID handleID,DBFieldSet* table,DBRecord* rec) = 0;

	/**
	* \brief 执行删除操作
	*
	* \param handleID 操作的链接句柄
	* \param tableName 要操作的表名
	* \param where 删除的条件
	* \return 返回受影响的记录数,返回-1表示有错误发生
	*/
	virtual DWORD exeDelete(connHandleID handleID,const char *tableName,const char *where) =0;

	/**
	* \brief 执行删除操作
	*
	* \param handleID 操作的链接句柄
	* \param table 要操作的表结构，通过DBMetaData::getFields获得
	* \param where 删除的条件
	*
	* \return 返回受影响的记录数,返回-1表示有错误发生
	*/
	virtual DWORD exeDelete(connHandleID handleID,DBFieldSet* table,DBRecord* where) = 0;

	/**
	* \brief 更新数据
	* \param handleID 操作的链接句柄
	* \param tableName 要操作的表名
	* \param column 要操作的数据字段描述，以{NULL,0,0}为结尾标记
	* \param data 要操作的数据字段
	* \param where 更新条件
	* \return 返回受影响的记录数,返回-1表示有错误发生
	*/
	virtual DWORD exeUpdate(connHandleID handleID,const char *tableName,const dbCol *column,const BYTE *data,const char *where) =0;

	/**
	* \brief 更新数据
	* \param handleID 操作的链接句柄
	* \param table 要操作的表结构，通过DBMetaData::getFields获得
	* \param data  要更新的字段及值
	* \param where 更新条件描述
	*
	* \return 返回受影响的记录数,返回-1表示有错误发生
	*/
	virtual DWORD exeUpdate(connHandleID handleID,DBFieldSet* table,DBRecord* data,DBRecord* where) = 0;

	/**
	* \brief 转化字符串为有效的db字符串
	* \param handleID 操作的链接句柄
	* \param src 操作源数据
	* \param dest 转换后字符串所存放的空间,为了程序的安全你应该为dest分配(size==0?strlen(src):size)*2+1的空间
	* \param size 如果size>0,表示转化指定长度的字符串，用于二进制数据的转化，如果为0表示一般字符串的转户
	* \return 失败返回NULL,成功返回dest
	*/
	virtual char * escapeString(connHandleID handleID,const char *src,char *dest,DWORD size) =0;

	/**
	* \brief 转化字符串为有效的db字符串
	* \param handleID 操作的链接句柄
	* \param src 操作源数据
	* \param dest 转换后字符串所存放的空间,为了程序的安全你应该为dest分配(size==0?strlen(src):size)*2+1的空间
	* \param size 如果size>0,表示转化指定长度的字符串，用于二进制数据的转化，如果为0表示一般字符串的转户
	* \return 失败返回NULL,成功返回dest
	*/
	virtual std::string& escapeString(connHandleID handleID,const std::string &src,std::string &dest) =0;

	/**
	* \brief 获取表中记录个数
	* \param handleID 操作的链接句柄
	* \param tableName 要操作的表名
	* \param where 计数条件
	* \return 返回计数结果
	*/
	virtual DWORD getCount(connHandleID handleID,const char* tableName,const char *where) =0;

	/**
	* \brief 把表中某个时间字段更新到最新时间
	* \param handleID 操作的链接句柄
	* \param tableName 要操作的表名
	* \param colName 要操作的字段名
	*/
	virtual void updateDatatimeCol(connHandleID handleID,const char* tableName,const char *colName) =0;

	/**
	* \brief 事务提交
	* \param handleID 操作的链接句柄
	* \return 成功返回true，失败返回false
	*/
	virtual bool commit(connHandleID handleID) =0;


	/**
	* \brief 事务回滚
	* \param handleID 操作的链接句柄
	* \return 成功返回true，失败返回false
	*/
	virtual bool rollback(const connHandleID handleID) =0;

	/**
	* \brief 设置此链接是否支持事务
	* \param handleID 操作的链接句柄
	* \param supportTransactions  是否支持事务
	* \return 成功返回true，失败返回false
	*/
	virtual bool setTransactions(const connHandleID handleID,bool supportTransactions) =0;

	/**
	* \brief 检查此链接是否支持事务
	* \param handleID 操作的链接句柄
	* \return 支持返回true，否则返回false
	*/
	virtual bool supportTransactions(connHandleID handleID) =0;
};


namespace Zebra
{
	/**
	* \brief 游戏时间
	*
	*/
	extern volatile QWORD qwGameTime;

	/**
	* \brief 日志指针
	*
	*/
	extern zLogger *logger;

	/**
	* \brief 存取全局变量的容器
	*
	*/
	extern zProperties global;
};

/**
* \brief 把字符串根据token转化为多个字符串
*
* 下面是使用例子程序：
*    <pre>
*    std::list<string> ls;
*    stringtok (ls," this  \t is\t\n  a test  ");
*    for(std::list<string>const_iterator i = ls.begin(); i != ls.end(); ++i)
*        std::cerr << ':' << (*i) << ":\n";
*     </pre>
*
* \param container 容器，用于存放字符串
* \param in 输入字符串
* \param delimiters 分隔符号
* \param deep 深度，分割的深度，缺省没有限制
*/
template <typename Container>
inline void
stringtok(Container &container,std::string const &in,
		  const char * const delimiters = " \t\n",
		  const int deep = 0)
{
	const std::string::size_type len = in.length();
	std::string::size_type i = 0;
	int count = 0;

	while(i < len)
	{
		i = in.find_first_not_of (delimiters,i);
		if (i == std::string::npos)
			return;   // nothing left

		// find the end of the token
		std::string::size_type j = in.find_first_of (delimiters,i);

		count++;
		// push token
		if (j == std::string::npos
			|| (deep > 0 && count > deep)) {
				container.push_back (in.substr(i));
				return;
		}
		else
			container.push_back (in.substr(i,j-i));

		// set up for next loop
		i = j + 1;
	}
}

/**
* \brief 把字符转化为小写的函数对象
*
* 例如：
* <pre>
* std::string  s ("Some Kind Of Initial Input Goes Here");
* std::transform (s.begin(),s.end(),s.begin(),ToLower());
* </pre>
*/
struct ToLower
{
	char operator() (char c) const
	{
		//return std::tolower(c);
		return tolower(c);
	}
};

/**
* \brief 把字符串转化为小写
* 
* 把输入的字符串转化为小写
*
* \param s 需要转化的字符串
*/
inline void to_lower(std::string &s)
{
	std::transform(s.begin(),s.end(),s.begin(),ToLower());
}

/**
* \brief 把字符转化为大写的函数对象
*
* 例如：
* <pre>
* std::string  s ("Some Kind Of Initial Input Goes Here");
* std::transform (s.begin(),s.end(),s.begin(),ToUpper());
* </pre>
*/
struct ToUpper
{
	char operator() (char c) const
	{
		return toupper(c);
	}
};

/**
* \brief 把字符串转化为大写
* 
* 把输入的字符串转化为大写
*
* \param s 需要转化的字符串
*/
inline void to_upper(std::string &s)
{
	std::transform(s.begin(),s.end(),s.begin(),ToUpper());
}

/**
* \brief png格式的验证码生成器
*/
void *jpeg_Passport(char *buffer,const int buffer_len,int *size);

/**
* \brief base64编码解码函数
*/
void base64_encrypt(const std::string &input,std::string &output);
void base64_decrypt(const std::string &input,std::string &output);

template <typename V>
class Parse
{
public:
	V* operator () (const std::string& down,const std::string& separator_down)
	{
		std::string::size_type pos = 0;
		if  ( (pos = down.find(separator_down)) != std::string::npos ) {

			std::string first_element = down.substr(0,pos);
			std::string second_element = down.substr(pos+separator_down.length());
			return new V(first_element,second_element);
		}

		return NULL;
	}
};

template <typename V>
class Parse3
{
public:
	V* operator () (const std::string& down,const std::string& separator_down)
	{
		std::string::size_type pos = 0;
		if  ( (pos = down.find(separator_down)) != std::string::npos ) {

			std::string first_element = down.substr(0,pos);
			std::string::size_type npos = 0;
			if ((npos = down.find(separator_down,pos+separator_down.length())) != std::string::npos) {
				std::string second_element = down.substr(pos+separator_down.length(),npos-pos);
				std::string third_element = down.substr(npos+separator_down.length());
				return new V(first_element,second_element,third_element);
			}
		}

		return NULL;
	}
};

/**
* \brief  分隔由二级分隔符分隔的字符串
* \param list 待分隔的字符串
* \param dest 存储分隔结果，必须满足特定的语义要求
* \param separator_up 一级分隔符
* \param separator_down 二级分隔符     
*/
template <template <typename> class P = Parse>
class Split
{
public:

	template <typename T>
	void operator() (const std::string& list,T& dest,const std::string& separator_up = ";",const std::string& separator_down = ",")
	{  
		typedef typename T::value_type value_type;
		typedef typename T::pointer pointer;

		std::string::size_type lpos = 0;
		std::string::size_type pos = 0;
		P<value_type> p;


		while ( ( lpos = list.find(separator_up,pos)) != std::string::npos) {
			/*
			std::string down = list.substr(pos,lpos - pos);
			std::string::size_type dpos = 0;
			if  ( (dpos = down.find(separator_down)) != std::string::npos ) {

			std::string first_element = down.substr(0,dpos);
			std::string second_element = down.substr(dpos+separator_down.length());
			dest.push_back(typename T::value_type(first_element,second_element));
			}
			pos = lpos+1;
			*/
			std::string down = list.substr(pos,lpos - pos);
			pointer v = p(down,separator_down);
			if (v) {
				dest.push_back(*v);
				SAFE_DELETE(v);
			}
			pos = lpos+1;
		}

		std::string down = list.substr(pos,lpos - pos);
		pointer v = p(down,separator_down);
		if (v) {
			dest.push_back(*v);
			SAFE_DELETE(v);
		}
	}
};

/**
* \brief 属性关联类容器，所有属性关键字和值都使用字符串代表，关键字不区分大小写
*
*/
class zProperties
{

public:

	/**
	* \brief 获取一个属性值
	*
	* \param key 关键字
	* \return 返回与关键字对应的属性值
	*/
	const std::string &getProperty(const std::string &key)
	{
		return properties[key];
	}

	/**
	* \brief 设置一个属性
	*
	* \param key 关键字
	* \param value 关键字对应的属性
	*/
	void setProperty(const std::string &key,const std::string &value)
	{
		properties[key] = value;
	}

	/**
	* \brief 重载操作符，返回与关键字对应的属性值
	*
	* \param key 关键字
	* \return 属性值
	*/
	std::string & operator[] (const std::string &key)
	{
		//fprintf(stderr,"properties operator[%s]\n",key.c_str());
		return properties[key];
	}

	/**
	* \brief 输出存储的所有属性值
	*
	*/
	void dump(std::ostream &out)
	{
		property_hashtype::const_iterator it;
		for(it = properties.begin(); it != properties.end(); it++)
			out << it->first << " = " << it->second << std::endl;
	}

	DWORD parseCmdLine(const std::string &cmdLine);
	DWORD parseCmdLine(const char *cmdLine);

protected:

	/**
	* \brief hash函数
	*
	*/
	/*struct key_hash : public std::unary_function<const std::string,size_t>
	{
	size_t operator()(const std::string &x) const
	{
	std::string s = x;
	hash<string> H;
	// _Hash<string> H;
	//转化字符串为小写
	to_lower(s);
	//tolower(s);
	//return H(s);
	return 0;
	}
	};*/
	/**
	* \brief 判断两个字符串是否相等
	*
	*/
	/*struct key_equal : public std::binary_function<const std::string,const std::string,bool>
	{
	bool operator()(const std::string &s1,const std::string &s2) const
	{
	// return strcasecmp(s1.c_str(),s2.c_str()) == 0;
	return (s1==s2);
	}
	};*/

	/**
	* \brief 字符串的hash_map
	*
	*/
	//typedef hash_map<std::string,std::string,key_hash,key_equal> property_hashtype;

	//typedef hash_map<std::string,std::string,key_hash,key_equal> property_hashtype;
	typedef hash_map<std::string,std::string> property_hashtype;

	property_hashtype properties;      /**< 保存属性的键值对 */

};

struct UrlInfo
{
	const DWORD hashcode;
	const std::string url;
	const bool supportTransactions;

	char host[MAX_HOSTSIZE];
	char user[MAX_USERSIZE];
	char passwd[MAX_PASSWORD];
	DWORD port;
	char dbName[MAX_DBSIZE];

	UrlInfo()
		: hashcode(0),url(),supportTransactions(false) {};
	UrlInfo(const DWORD hashcode,const std::string &url,const bool supportTransactions)
		: hashcode(hashcode),url(url),supportTransactions(supportTransactions)
	{
		parseMySQLURLString();
	}
	UrlInfo(const UrlInfo &ui)
		: hashcode(ui.hashcode),url(ui.url),supportTransactions(ui.supportTransactions)
	{
		parseMySQLURLString();
	}

private:
	void parseMySQLURLString()
	{
		bzero(host,sizeof(host));
		bzero(user,sizeof(user));
		bzero(passwd,sizeof(passwd));
		port=3306;
		bzero(dbName,sizeof(dbName));

		char strPort[16] = "";
		int  j,k;
		size_t i;
		const char *connString = url.c_str();
		if (0 == strncmp(connString,"mysql://",strlen("mysql://")))
		{
			i = 0; j = 0; k = 0;
			for(i = strlen("mysql://"); i < strlen(connString) + 1; i++)
			{
				switch(j)
				{
				case 0:
					if (connString[i] == ':')
					{
						user[k] = '\0'; j++; k = 0;
					}
					else
						user[k++] = connString[i];
					break;
				case 1:
					if (connString[i] == '@')
					{
						passwd[k] = '\0'; j++; k = 0;
					}
					else
						passwd[k++] = connString[i];
					break;
				case 2:
					if (connString[i] == ':')
					{
						host[k] = '\0'; j++; k = 0;
					}
					else
						host[k++] = connString[i];
					break;
				case 3:
					if (connString[i] == '/')
					{
						strPort[k] = '\0'; j++; k = 0;
					}
					else
						strPort[k++] = connString[i];
					break;
				case 4:
					if (connString[i] == '\0')
					{
						dbName[k] = '\0'; j++; k = 0;
					}
					else
						dbName[k++] = connString[i];
					break;
				default:
					break;
				}
			}
		}
		port=atoi(strPort);
	}
};
class zLock
{
public:

	/**
	* \brief 构造函数，构造一个临界区对象
	*
	*/
	zLock() 
	{
		InitializeCriticalSection(&m_critical);
	}

	/**
	* \brief 析构函数，销毁一个临界区对象
	*
	*/
	~zLock()
	{
		DeleteCriticalSection(&m_critical);
	}

	/**
	* \brief 加锁一个临界区
	*
	*/
	inline void lock(  )
	{
		EnterCriticalSection(&m_critical);
	}

	/**
	* \brief 解锁一个临界区
	*
	*/
	inline void unlock(  )
	{
		LeaveCriticalSection(&m_critical);
	}

private:

	CRITICAL_SECTION    m_critical; // 系统临界区
};
/**
* \brief Zebra项目的日志类。
*
* 目前实现了两种写日志方式，即控制台、本地文件。
*
* 默认日志级别是#DEBUG
*
* 此类为线程安全类。
*/
class zLogger
{
public:
	/**
	* \brief zLevel声明了几个日志等级
	*
	* 除了用log4cxx提供的标准日志等级作为日志等级外，还自定义了游戏日志等级.
	*
	* 程序日志等级关系为 #OFF> #FATAL> #ERROR> #WARN> #INFO> #DEBUG> #ALL
	*
	*/

	/**
	* \brief Zebra项目所支持日志等级数字定义
	*/
	typedef enum
	{
		/**
		* \brief 当zLogger等级设置为OFF，否则不会输出任何日志
		*/
		LEVEL_OFF   = INT_MAX,

		/**
		* \brief 当zLogger等级设置为FATAL，只输出FATAL等级的日志
		*
		* 程序致命错误，已经无法提供正常的服务功能。
		*/
		LEVEL_FATAL = 50000,

		/**
		* \brief 当zLogger等级设置为ERROR，只输出大于等于此等级的日志
		*
		* 错误，可能不能提供某种服务，但可以保证程序正确运行。
		*/
		LEVEL_ERROR = 40000,

		/**
		* \brief 当zLogger等级设置为WARN，只输出大于等于此等级的日志
		*
		* 警告，某些地方需要引起注意，比如没有配置文件，但程序用默认选项可以使用。
		*/
		LEVEL_WARN  = 30000,

		/**
		* \brief 当zLogger等级设置为INFO，只输出大于等于此等级的日志
		*
		* 信息，提供一般信息记录，多用于一些程序状态的记录。
		*/
		LEVEL_INFO  = 20000,

		/**
		* \brief 当zLogger等级设置为DEBUG，输出所有等级的日志
		*/
		LEVEL_DEBUG = 10000,

		/**
		* \brief 当zLogger等级设置为ALL，输出所有等级的日志
		*/
		LEVEL_ALL   = INT_MIN
	}zLevel;

	zLogger(char *name = "Zebra");
	~zLogger();

	void removeConsoleLog();
	void addLocalFileLog(const std::string &file);

	void setLevel(const zLevel level);
	void setLevel(const std::string &level);

	void logtext(const zLevel level,const char * text);
	void logva(const zLevel level,const char * pattern,va_list vp);
	void log(const zLevel level,const char * pattern,...);

	void debug(const char * pattern,...);
	void debug16(const char* info, const BYTE* pData, int Datasize);
	void error(const char * pattern,...);
	void info(const char * pattern,...);
	void fatal(const char * pattern,...);
	void warn(const char * pattern,...);

private:
	zLock msgMut;

	zLevel      m_level;
	FILE        *fp_console;
	FILE        *fp_file;
	int         m_day;
	std::string m_name;
	std::string m_file;
};

/**
* \brief 定义类zThread
*
* 
*/
/**
* \brief 临界区，封装了系统临界区，避免了使用系统临界区时候需要手工初始化和销毁临界区对象的操作
*
*/
class zMutex : private zNoncopyable
{

	friend class zCond;

public:

//	/**
//	* \brief 构造函数，构造一个临界区对象
//	*
//	*/
//	zMutex() 
//	{
//		InitializeCriticalSection(&m_critical);
//	}
//
//	/**
//	* \brief 析构函数，销毁一个临界区对象
//	*
//	*/
//	~zMutex()
//	{
//		DeleteCriticalSection(&m_critical);
//	}
//
//	/**
//	* \brief 加锁一个临界区
//	*
//	*/
//	inline void lock()
//	{
////		Zebra::logger->debug("Locking - %0.8X - %s(%u)", (DWORD)this, file,line );
//		EnterCriticalSection(&m_critical);
////		Zebra::logger->debug("Locked - %0.8X - %s(%u)", (DWORD)this, file,line );
//	}
//
//	/**
//	* \brief 解锁一个临界区
//	*
//	*/
//	inline void unlock()
//	{
////		Zebra::logger->debug("UnLock - %0.8X - %s(%u)", (DWORD)this, file,line );
//		LeaveCriticalSection(&m_critical);
//	}
//
//private:
//
//	CRITICAL_SECTION    m_critical; // 系统临界区
	/**
	* \brief 构造函数，构造一个互斥体对象
	*
	*/
	zMutex() 
	{
		m_hMutex = CreateMutex(NULL,FALSE,NULL);
	}

	/**
	* \brief 析构函数，销毁一个互斥体对象
	*
	*/
	~zMutex()
	{
		CloseHandle(m_hMutex);
	}

	/**
	* \brief 加锁一个互斥体
	*
	*/
	inline void lock()
	{
		if( WaitForSingleObject(m_hMutex,10000) == WAIT_TIMEOUT )
		{
			char szName[MAX_PATH];
			GetModuleFileName(NULL,szName,sizeof(szName));
			::MessageBox(NULL,"发生死锁！", szName, MB_ICONERROR);
		}
	}

	/**
	* \brief 解锁一个互斥体
	*
	*/
	inline void unlock()
	{
		ReleaseMutex(m_hMutex);
	}

private:

	HANDLE m_hMutex;    /**< 系统互斥体 */

};

/**
* \brief Wrapper
* 方便在复杂函数中锁的使用
*/
class zMutex_scope_lock : private zNoncopyable
{

public:

	/**
	* \brief 构造函数
	* 对锁进行lock操作
	* \param m 锁的引用
	*/
	zMutex_scope_lock(zMutex &m) : mlock(m)
	{
		mlock.lock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~zMutex_scope_lock()
	{
		mlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	zMutex &mlock;

};

/**
* \brief 封装了系统条件变量，使用上要简单，省去了手工初始化和销毁系统条件变量的工作，这些工作都可以由构造函数和析构函数来自动完成
*
*/
class zCond : private zNoncopyable
{

public:

	/**
	* \brief 构造函数，用于创建一个条件变量
	*
	*/
	zCond()
	{
		m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	}

	/**
	* \brief 析构函数，用于销毁一个条件变量
	*
	*/
	~zCond()
	{
		CloseHandle(m_hEvent);
	}

	/**
	* \brief 对所有等待这个条件变量的线程广播发送信号，使这些线程能够继续往下执行
	*
	*/
	void broadcast()
	{
		SetEvent(m_hEvent);
	}

	/**
	* \brief 对所有等待这个条件变量的线程发送信号，使这些线程能够继续往下执行
	*
	*/
	void signal()
	{
		SetEvent(m_hEvent);
	}

	/**
	* \brief 等待特定的条件变量满足
	*
	*
	* \param m_hMutex 需要等待的互斥体
	*/
	void wait(zMutex &mutex)
	{
		WaitForSingleObject(m_hEvent,INFINITE);
	}

private:

	HANDLE m_hEvent;    /**< 系统条件变量 */

};

/**
* \brief 封装了系统读写锁，使用上要简单，省去了手工初始化和销毁系统读写锁的工作，这些工作都可以由构造函数和析构函数来自动完成
*
*/
class zRWLock : private zNoncopyable
{

public:
	/**
	* \brief 构造函数，用于创建一个读写锁
	*
	*/
	zRWLock()
	{
		m_hMutex = CreateMutex(NULL,FALSE,NULL);
	}

	/**
	* \brief 析构函数，用于销毁一个读写锁
	*
	*/
	~zRWLock()
	{
		CloseHandle(m_hMutex);
	}

	/**
	* \brief 对读写锁进行读加锁操作
	*
	*/
	inline void rdlock()
	{
		WaitForSingleObject(m_hMutex,INFINITE);
	};

	/**
	* \brief 对读写锁进行写加锁操作
	*
	*/
	inline void wrlock()
	{
		WaitForSingleObject(m_hMutex,INFINITE);
	}

	/**
	* \brief 对读写锁进行解锁操作
	*
	*/
	inline void unlock()
	{
		ReleaseMutex(m_hMutex);
	}

private:

	HANDLE m_hMutex;    /**< 系统读写锁 */

};

/**
* \brief rdlock Wrapper
* 方便在复杂函数中读写锁的使用
*/
class zRWLock_scope_rdlock : private zNoncopyable
{

public:

	/**
	* \brief 构造函数
	* 对锁进行rdlock操作
	* \param m 锁的引用
	*/
	zRWLock_scope_rdlock(zRWLock &m) : rwlock(m)
	{
		rwlock.rdlock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~zRWLock_scope_rdlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	zRWLock &rwlock;

};

/**
* \brief wrlock Wrapper
* 方便在复杂函数中读写锁的使用
*/
class zRWLock_scope_wrlock : private zNoncopyable
{

public:

	/**
	* \brief 构造函数
	* 对锁进行wrlock操作
	* \param m 锁的引用
	*/
	zRWLock_scope_wrlock(zRWLock &m) : rwlock(m)
	{
		rwlock.wrlock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~zRWLock_scope_wrlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	zRWLock &rwlock;

};


/**
* \brief 定义zSocket类，用于对套接口底层进行封装
*/
#define SHUT_RDWR       SD_BOTH
#define SHUT_RD         SD_RECEIVE
#define SHUT_WR         SD_SEND

#define MSG_NOSIGNAL    0
#define EWOULDBLOCK     WSAEWOULDBLOCK

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#define POLLIN  1       /* Set if data to read. */
#define POLLPRI 2       /* Set if urgent data to read. */
#define POLLOUT 4       /* Set if writing data wouldn't block. */

	class zSocket;

	struct mypollfd {
		int fd;
		short events;
		short revents;
		zSocket* pSock;
	};

	extern int poll(struct mypollfd *fds,unsigned int nfds,int timeout);
	extern int WaitRecvAll( struct mypollfd *fds,unsigned int nfds,int timeout );

#ifdef __cplusplus
}
#endif //__cplusplus

const DWORD trunkSize = 64 * 1024;
#define unzip_size(zip_size) ((zip_size) * 120 / 100 + 12)
const DWORD PACKET_ZIP_BUFFER  =  unzip_size(trunkSize - 1) + sizeof(DWORD) + 8;  /**< 压缩需要的缓冲 */

/**
* 字节缓冲，用于套接口接收和发送数据的缓冲
* \param _type 缓冲区数据类型
*/
template <typename _type>
class ByteBuffer
{

public:
	zMutex m_Lock; // 互斥锁

	inline void Lock()
	{
		m_Lock.lock();
	}
	inline void UnLock()
	{
		m_Lock.unlock();
	}
	/**
	* 构造函数
	*/
	ByteBuffer();

	/**
	* 向缓冲填入数据
	* \param buf 待填入缓冲的数据
	* \param size 待填入缓冲数据的长度
	*/
	inline void put(const BYTE *buf,const DWORD size)
	{
		if( size == 1 )
		{
			int iii = 0;
		}
		//首先确认缓冲内存是否足够
		wr_reserve(size);

		if( _maxSize - _currPtr < size )
		{
			MessageBox(NULL,"缓冲区溢出","严重错误",MB_ICONERROR);
		}

		bcopy((void *)buf,&_buffer[_currPtr],size,size);
		_currPtr += size;
		if( _currPtr - _offPtr == 1 )
		{
			int iii = 0;
		}
	}

	/**
	* 得到当前可写bf的未知
	* 保证在调用此函数写入数据之前需要调用wr_reserve(size)来预留缓冲区大小
	* \return 可写入缓冲开始地址
	*/
	inline BYTE *wr_buf()
	{
		return &_buffer[_currPtr];
	}

	/**
	* 返回缓冲中有效数据的开始地址
	* \return 有效数据地址
	*/
	inline BYTE *rd_buf()
	{
		return &_buffer[_offPtr];
	}

	/**
	* 判断缓冲中时候有有效数据
	* \return 返回缓冲中是否有有效数据
	*/
	inline bool rd_ready()
	{
		bool ret = _currPtr > _offPtr;
		return ret;
	}

	/**
	* 得到缓冲中有效数据的大小
	* \return 返回缓冲中有效数据大小
	*/
	inline DWORD rd_size()
	{
		DWORD ret = _currPtr - _offPtr;
		return ret;
	}

	/**
	* 当缓冲的有效数据被使用以后，需要对缓冲进行整理
	* \param size 最后一次使用的有效数据长度
	*/
	inline void rd_flip(DWORD size)
	{	
		if( size == 0 ) return;
		_offPtr += size;
		if (_currPtr > _offPtr)
		{
			DWORD tmp = _currPtr - _offPtr;
			if (_offPtr >= tmp)
			{
				memmove(&_buffer[0],&_buffer[_offPtr],tmp);
				_offPtr = 0;
				_currPtr = tmp;
			}
		}
		else
		{
			_offPtr = 0;
			_currPtr = 0;
		}
		if( _currPtr - _offPtr == 1)
		{
			int iii = 0;
		}
	}

	/**
	* 得到缓冲可写入数据的大小
	* \return 可写入数据的大小
	*/
	inline DWORD wr_size()
	{
		DWORD ret = _maxSize - _currPtr;
		return ret;
	}

	/**
	* 实际向缓冲写入了数据，需要对缓冲进行整理
	* \param size 实际写入的数据
	*/
	inline void wr_flip(const DWORD size)
	{
		_currPtr += size;
	}

	/**
	* 重值缓冲中的数据，清空无用的垃圾数据
	*/
	inline void reset()
	{
		_offPtr = 0;
		_currPtr = 0;
	}

	/**
	* 返回缓冲最大大小
	* \return 缓冲最大大小
	*/
	inline DWORD maxSize() const
	{
		return _maxSize;
	}

	/**
	* 对缓冲的内存进行重新整理，向缓冲写数据，如果缓冲大小不足，重新调整缓冲大小，
	* 大小调整原则按照trunkSize的整数倍进行增加
	* \param size 向缓冲写入了多少数据
	*/
	inline void wr_reserve(const DWORD size);

private:

	DWORD _maxSize;
	DWORD _offPtr;
	DWORD _currPtr;
	_type _buffer;

};

/**
* 动态内存的缓冲区，可以动态扩展缓冲区大小
*/
typedef ByteBuffer<std::vector<BYTE> > t_BufferCmdQueue;

/**
* 模板偏特化
* 对缓冲的内存进行重新整理，向缓冲写数据，如果缓冲大小不足，重新调整缓冲大小，
* 大小调整原则按照trunkSize的整数倍进行增加
* \param size 向缓冲写入了多少数据
*/
template <>
inline void t_BufferCmdQueue::wr_reserve(const DWORD size)
{
	if (wr_size() < size)
	{
#define trunkCount(size) (((size) + trunkSize - 1) / trunkSize)
		_maxSize += (trunkSize * trunkCount(size));
		_buffer.resize(_maxSize);
	}
}


/**
* 静态大小的缓冲区，以栈空间数组的方式来分配内存，用于一些临时变量的获取
*/
typedef ByteBuffer<BYTE [PACKET_ZIP_BUFFER]> t_StackCmdQueue;

/**
* 模板偏特化
* 对缓冲的内存进行重新整理，向缓冲写数据，如果缓冲大小不足，重新调整缓冲大小，
* 大小调整原则按照trunkSize的整数倍进行增加
* \param size 向缓冲写入了多少数据
*/
template <>
inline void t_StackCmdQueue::wr_reserve(const DWORD size)
{
	/*
	if (wr_size() < size)
	{
	//不能动态扩展内存
	assert(false);
	}
	// */
}

/**
* \brief 变长指令的封装，固定大小的缓冲空间
* 在栈空间分配缓冲内存
* \param cmd_type 指令类型
* \param size 缓冲大小
*/
template <typename cmd_type,DWORD size = 64 * 1024>
class CmdBuffer_wrapper
{

public:

	typedef cmd_type type;
	DWORD cmd_size;
	DWORD max_size;
	type *cnt;

	CmdBuffer_wrapper() : cmd_size(sizeof(type)),max_size(size)// : cnt(NULL)
	{
		cnt = (type *)buffer;
		constructInPlace(cnt);
	}

private:

	BYTE buffer[size];

};

/**
* \brief 时间定义
*
* 
*/
/**
* \brief 真实时间类,对timeval结构简单封装,提供一些常用时间函数
* 时间精度精确到毫秒，
* 关于timeval请man gettimeofday
*/
class zRTime
{

private:

	/**
	* \brief 真实时间换算为毫秒
	*
	*/
	QWORD _msecs;

	/**
	* \brief 得到当前真实时间
	*
	* \return 真实时间，单位毫秒
	*/
	QWORD _now()
	{
		QWORD retval = 0LL;
		struct timeval tv;
		gettimeofday(&tv,NULL);
		retval = tv.tv_sec;
		retval *= 1000;
		retval += tv.tv_usec / 1000;
		return retval;
	}

	/**
	* \brief 得到当前真实时间延迟后的时间
	* \param delay 延迟，可以为负数，单位毫秒
	*/
	void nowByDelay(int delay)
	{
		_msecs = _now();
		addDelay(delay);
	}

public:

	/**
	* \brief 构造函数
	*
	* \param delay 相对于现在时间的延时，单位毫秒
	*/
	zRTime(const int delay = 0)
	{
		nowByDelay(delay);
	}

	/**
	* \brief 拷贝构造函数
	*
	* \param rt 拷贝的引用
	*/
	zRTime(const zRTime &rt)
	{
		_msecs = rt._msecs;
	}

	/**
	* \brief 获取当前时间
	*
	*/
	void now()
	{
		_msecs = _now();
	}

	/**
	* \brief 返回秒数
	*
	* \return 秒数
	*/
	DWORD sec() const
	{
		return _msecs / 1000;
	}

	/**
	* \brief 返回毫秒数
	*
	* \return 毫秒数
	*/
	DWORD msec() const
	{
		return _msecs % 1000;
	}

	/**
	* \brief 返回总共的毫秒数
	*
	* \return 总共的毫秒数
	*/
	QWORD msecs() const
	{
		return _msecs;
	}

	/**
	* \brief 返回总共的毫秒数
	*
	* \return 总共的毫秒数
	*/
	void setmsecs(QWORD data)
	{
		_msecs = data;
	}

	/**
	* \brief 加延迟偏移量
	*
	* \param delay 延迟，可以为负数，单位毫秒
	*/
	void addDelay(int delay)
	{
		_msecs += delay;
	}

	/**
	* \brief 重载=运算符号
	*
	* \param rt 拷贝的引用
	* \return 自身引用
	*/
	zRTime & operator= (const zRTime &rt)
	{
		_msecs = rt._msecs;
		return *this;
	}

	/**
	* \brief 重构+操作符
	*
	*/
	const zRTime & operator+ (const zRTime &rt)
	{
		_msecs += rt._msecs;
		return *this;
	}

	/**
	* \brief 重构-操作符
	*
	*/
	const zRTime & operator- (const zRTime &rt)
	{
		_msecs -= rt._msecs;
		return *this;
	}

	/**
	* \brief 重构>操作符，比较zRTime结构大小
	*
	*/
	bool operator > (const zRTime &rt) const
	{
		return _msecs > rt._msecs;
	}

	/**
	* \brief 重构>=操作符，比较zRTime结构大小
	*
	*/
	bool operator >= (const zRTime &rt) const
	{
		return _msecs >= rt._msecs;
	}

	/**
	* \brief 重构<操作符，比较zRTime结构大小
	*
	*/
	bool operator < (const zRTime &rt) const
	{
		return _msecs < rt._msecs;
	}

	/**
	* \brief 重构<=操作符，比较zRTime结构大小
	*
	*/
	bool operator <= (const zRTime &rt) const
	{
		return _msecs <= rt._msecs;
	}

	/**
	* \brief 重构==操作符，比较zRTime结构是否相等
	*
	*/
	bool operator == (const zRTime &rt) const
	{
		return _msecs == rt._msecs;
	}

	/**
	* \brief 计时器消逝的时间，单位毫秒
	* \param rt 当前时间
	* \return 计时器消逝的时间，单位毫秒
	*/
	QWORD elapse(const zRTime &rt) const
	{
		if (rt._msecs > _msecs)
			return (rt._msecs - _msecs);
		else
			return 0LL;
	}

	static std::string & getLocalTZ(std::string & s);
	static void getLocalTime(struct tm & tv1,time_t timValue)
	{
		timValue +=8*60*60;
		tv1 = *gmtime(&timValue);
	}

};

/**
* \brief 时间类,对struct tm结构简单封装
*/

class zTime
{

public:

	/**
	* \brief 构造函数
	*/
	zTime()
	{
		time(&secs);
		zRTime::getLocalTime(tv,secs);
	}

	/**
	* \brief 拷贝构造函数
	*/
	zTime(const zTime &ct)
	{
		secs = ct.secs;
		zRTime::getLocalTime(tv,secs);
	}

	/**
	* \brief 获取当前时间
	*/
	void now()
	{
		time(&secs);
		zRTime::getLocalTime(tv,secs);
	}

	/**
	* \brief 返回存储的时间
	* \return 时间，秒
	*/
	time_t sec() const
	{
		return secs;
	}

	/**
	* \brief 重载=运算符号
	* \param rt 拷贝的引用
	* \return 自身引用
	*/
	zTime & operator= (const zTime &rt)
	{
		secs = rt.secs;
		return *this;
	}

	/**
	* \brief 重构+操作符
	*/
	const zTime & operator+ (const zTime &rt)
	{
		secs += rt.secs;
		return *this;
	}

	/**
	* \brief 重构-操作符
	*/
	const zTime & operator- (const zTime &rt)
	{
		secs -= rt.secs;
		return *this;
	}

	/**
	* \brief 重构-操作符
	*/
	const zTime & operator-= (const time_t s)
	{
		secs -= s;
		return *this;
	}

	/**
	* \brief 重构>操作符，比较zTime结构大小
	*/
	bool operator > (const zTime &rt) const
	{
		return secs > rt.secs;
	}

	/**
	* \brief 重构>=操作符，比较zTime结构大小
	*/
	bool operator >= (const zTime &rt) const
	{
		return secs >= rt.secs;
	}

	/**
	* \brief 重构<操作符，比较zTime结构大小
	*/
	bool operator < (const zTime &rt) const
	{
		return secs < rt.secs;
	}

	/**
	* \brief 重构<=操作符，比较zTime结构大小
	*/
	bool operator <= (const zTime &rt) const
	{
		return secs <= rt.secs;
	}

	/**
	* \brief 重构==操作符，比较zTime结构是否相等
	*/
	bool operator == (const zTime &rt) const
	{
		return secs == rt.secs;
	}

	/**
	* \brief 计时器消逝的时间，单位秒
	* \param rt 当前时间
	* \return 计时器消逝的时间，单位秒
	*/
	time_t elapse(const zTime &rt) const
	{
		if (rt.secs > secs)
			return (rt.secs - secs);
		else
			return 0;
	}

	/**
	* \brief 计时器消逝的时间，单位秒
	* \return 计时器消逝的时间，单位秒
	*/
	time_t elapse() const
	{
		zTime rt;
		return (rt.secs - secs);
	}

	/**
	* \brief 得到当前分钟，范围0-59点
	*
	* \return 
	*/
	int getSec()
	{
		return tv.tm_sec;
	}

	/**
	* \brief 得到当前分钟，范围0-59点
	*
	* \return 
	*/
	int getMin()
	{
		return tv.tm_min;
	}

	/**
	* \brief 得到当前小时，范围0-23点
	*
	* \return 
	*/
	int getHour()
	{
		return tv.tm_hour;
	}

	/**
	* \brief 得到天数，范围1-31
	*
	* \return 
	*/
	int getMDay()
	{
		return tv.tm_mday;
	}

	/**
	* \brief 得到当前星期几，范围1-7
	*
	* \return 
	*/
	int getWDay()
	{
		return tv.tm_wday;
	}

	/**
	* \brief 得到当前月份，范围1-12
	*
	* \return 
	*/
	int getMonth()
	{
		return tv.tm_mon+1;
	}

	/**
	* \brief 得到当前年份
	*
	* \return 
	*/
	int getYear()
	{
		return tv.tm_year+1900;
	}  

private:

	/**
	* \brief 存储时间，单位秒
	*/
	time_t secs;

	/**
	* \brief tm结构，方便访问
	*/
	struct tm tv;


};

class Timer
{
public:
	Timer(const float how_long,const int delay=0) : _long((int)(how_long*1000)),_timer(delay*1000)
	{

	}
	Timer(const float how_long,const zRTime cur) : _long((int)(how_long*1000)),_timer(cur)
	{
		_timer.addDelay(_long);
	}
	void next(const zRTime &cur)
	{
		_timer=cur;
		_timer.addDelay(_long);
	} 
	bool operator() (const zRTime& current)
	{
		if (_timer <= current) {
			_timer = current;
			_timer.addDelay(_long);
			return true;
		}

		return false;
	}
private:
	int _long;
	zRTime _timer;
};

struct odds_t
{
	DWORD upNum;
	DWORD downNum;
};

//从字符串中查找第pos(从零开始)个数字，如果未找到返回defValue
template <typename T>
WORD getAllNum(const char *s,std::vector<T> & data)
{
	size_t i;
	int count = 0;
	if (s == NULL) return count;
	bool preIsD = false;
	for (i = 0; i < strlen(s); i++)
	{
		if (isdigit(*(s + i)))
		{
			if (!preIsD)
			{
				count++;
				data.push_back(atoi(s+i));
			}
			preIsD = true;
		}
		else
			preIsD = false;
	}
	return count;
}

//随机产生min~max之间的数字，包括min和max
int randBetween(int min,int max);

//获取几分之的几率
bool selectByOdds(const DWORD upNum,const DWORD downNum);

//获取几分之几的几率
bool selectByt_Odds(const odds_t &odds);

//获取百分之的几率
bool selectByPercent(const DWORD percent);

//获取万分之的几率
bool selectByTenTh(const DWORD tenth);


//获取十万分之的几率
bool selectByLakh(const DWORD lakh);

//获取亿分之之的几率
bool selectByOneHM(const DWORD lakh);

//获取当前时间字符串，需要给定格式
void getCurrentTimeString(char *buffer,const int bufferlen,const char *format);

char *getTimeString(time_t t,char *buffer,const int bufferlen,const char *format);

char *getTimeString(time_t t,char *buffer,const int bufferlen);

//时间间隔具有随机性
class RandTimer
{
public:
#define next_time(_long) (_long / 2 + randBetween(0,_long))
	RandTimer(const float how_long,const int delay=0) : _long((int)(how_long*1000)),_timer(delay*1000)
	{

	}
	RandTimer(const float how_long,const zRTime cur) : _long((int)(how_long*1000)),_timer(cur)
	{
		_timer.addDelay(next_time(_long));
	}
	void next(const zRTime &cur)
	{
		_timer=cur;
		_timer.addDelay(next_time(_long));
	} 
	bool operator() (const zRTime& current)
	{
		if (_timer <= current) {
			_timer = current;
			_timer.addDelay(next_time(_long));
			return true;
		}

		return false;
	}
private:
	int _long;
	zRTime _timer;
};
// [ranqd] IO操作状态标志
typedef   enum   enum_IOOperationType   
{     
	IO_Write,     // 写
	IO_Read		  // 读

}IOOperationType,   *LPIOOperationType;

// [ranqd] 自定义IO操作结构，指明操作类型
typedef   struct   st_OverlappedData   
{   
	OVERLAPPED Overlapped;
	IOOperationType OperationType;

	st_OverlappedData( enum_IOOperationType type )
	{
		ZeroMemory( &Overlapped, sizeof(OVERLAPPED) );
		OperationType = type;
	}

}OverlappedData,   *LPOverlappedData;

/**
* \brief 封装套接口底层函数，提供一个比较通用的接口
*/
class zTCPTask;

class zSocket : private zNoncopyable
{

public:
	bool				m_bUseIocp;     // [ranqd]  是否使用IOCP收发数据   

	DWORD               m_SendSize;   // [ranqd] 记录希望发送数据总长度
	DWORD               m_LastSend;   // [ranqd] 记录单次请求发送数据长度
	DWORD               m_LastSended; // [ranqd] 已发送所请求数据长度

	static const int T_RD_MSEC          =  2100;          /**< 读取超时的毫秒数 */
	static const int T_WR_MSEC          =  2100;          /**< 发送超时的毫秒数 */

	static const DWORD PH_LEN       =  sizeof(DWORD);  /**< 数据包包头大小 */
	static const DWORD PACKET_ZIP_MIN  =  32;            /**< 数据包压缩最小大小 */

	static const DWORD PACKET_ZIP    =  0x40000000;        /**< 数据包压缩标志 */
	static const DWORD INCOMPLETE_READ  =  0x00000001;        /**< 上次对套接口进行读取操作没有读取完全的标志 */
	static const DWORD INCOMPLETE_WRITE  =  0x00000002;        /**< 上次对套接口进行写入操作煤油写入完毕的标志 */

	static const DWORD PACKET_MASK      =  trunkSize - 1;  /**< 最大数据包长度掩码 */
	static const DWORD MAX_DATABUFFERSIZE  =  PACKET_MASK;            /**< 数据包最大长度，包括包头4字节 */
	static const DWORD MAX_DATASIZE      =  (MAX_DATABUFFERSIZE - PH_LEN - PACKHEADLASTSIZE);    /**< 数据包最大长度 */
	static const DWORD MAX_USERDATASIZE    =  (MAX_DATASIZE - 128);        /**< 用户数据包最大长度 */

	static const char *getIPByIfName(const char *ifName);

	zSocket(const SOCKET sock,const struct sockaddr_in *addr = NULL,const bool compress = false, const bool useIocp = USE_IOCP,zTCPTask* pTask = NULL );
	~zSocket();

	int recvToCmd(void *pstrCmd,const int nCmdLen,const bool wait);
	bool sendCmd(const void *pstrCmd,const int nCmdLen,const bool buffer = false);
	bool sendCmdNoPack(const void *pstrCmd,const int nCmdLen,const bool buffer = false);
	int  Send(const SOCKET sock, const void* pBuffer, const int nLen,int flags);
	bool sync();
	void force_sync();

	int checkIOForRead();
	int checkIOForWrite();
	int recvToBuf_NoPoll();
	int recvToCmd_NoPoll(void *pstrCmd,const int nCmdLen);

	/**
	* \brief 获取套接口对方的地址
	* \return IP地址
	*/
	inline const char *getIP() const { return inet_ntoa(addr.sin_addr); }
	inline const DWORD getAddr() const { return addr.sin_addr.s_addr; }

	/**
	* \brief 获取套接口对方端口
	* \return 端口
	*/
	inline const WORD getPort() const { return ntohs(addr.sin_port); }

	/**
	* \brief 获取套接口本地的地址
	* \return IP地址
	*/
	inline const char *getLocalIP() const { return inet_ntoa(local_addr.sin_addr); }

	/**
	* \brief 获取套接口本地端口
	* \return 端口
	*/
	inline const WORD getLocalPort() const { return ntohs(local_addr.sin_port); }

	/**
	* \brief 设置读取超时
	* \param msec 超时，单位毫秒 
	* \return 
	*/
	inline void setReadTimeout(const int msec) { rd_msec = msec; }

	/**
	* \brief 设置写入超时
	* \param msec 超时，单位毫秒 
	* \return 
	*/
	inline void setWriteTimeout(const int msec) { wr_msec = msec; }


	/**
	* \brief 填充pollfd结构
	* \param pfd 待填充的结构
	* \param events 等待的事件参数
	*/
	inline void fillPollFD(struct mypollfd &pfd,short events)
	{
		pfd.fd = sock;
		pfd.events = events;
		pfd.revents = 0;
		pfd.pSock = this;
	}

	inline void setEncMethod(CEncrypt::encMethod m) { enc.setEncMethod(m); }
	inline void set_key_rc5(const BYTE *data,int nLen,int rounds) { enc.set_key_rc5(data,nLen,rounds); }
	inline void set_key_des(const_ZES_cblock *key) { enc.set_key_des(key); }
	inline DWORD snd_queue_size() { return _snd_queue.rd_size() + _enc_queue.rd_size(); }

	inline DWORD getBufferSize() const {return _rcv_queue.maxSize() + _snd_queue.maxSize();}

	inline zTCPTask*& GetpTask() {return pTask;} // [ranqd] 返回Task指针

	void ReadByte( DWORD size ); // [ranqd] 请求读取单个字节
	void RecvData( DWORD dwNum = 0 ); // [ranqd] 通过Iocp收取数据

	int SendData( DWORD dwNum ); // [ranqd] 通过Iocp发送数据

	int WaitRecv( bool bWait, int timeout = 0 ) ;  // [ranqd] 等待数据接收

	int WaitSend( bool bWait, int timeout = 0 ); // [ranqd] 等待数据发送

	void DisConnet()
	{
		::shutdown(sock,0x02);
		::closesocket(sock);
		sock = INVALID_SOCKET;
	}

	zMutex m_Lock;

	bool m_bIocpDeleted;   // iocp是否释放
	bool m_bTaskDeleted;   // task是否释放

	bool m_bDeleted;       // 释放标志
	bool SafeDelete( bool bFromIocp ) // [ranqd] 安全释放本类
	{
		m_Lock.lock();
		if( bFromIocp )
		{
			m_bIocpDeleted = true;
		}
		else
		{
			m_bTaskDeleted = true;
		}
		if( !m_bIocpDeleted || !m_bTaskDeleted )
		{			
			DisConnet();
			m_Lock.unlock();
			return false;
		}
		if( m_bDeleted )
		{
			m_Lock.unlock();
			return false;
		}
		m_bDeleted = true;
		m_Lock.unlock();
		Zebra::logger->debug( "释放连接 %0.8X", pTask );
		return true;
	}
private:
	SOCKET sock;                  /**< 套接口 */
	struct sockaddr_in addr;          /**< 套接口地址 */
	struct sockaddr_in local_addr;        /**< 套接口地址 */
	int rd_msec;                /**< 读取超时，毫秒 */
	int wr_msec;                /**< 写入超时，毫秒 */

	zMutex m_RecvLock;                  // [ranqd]  接收缓冲线程锁
	t_BufferCmdQueue    m_RecvBuffer;   // [ranqd]  Iocp接收数据缓冲
	OverlappedData		m_ovIn;         // [ranqd]  Io读状态记录
	OverlappedData      m_ovOut;        // [ranqd]  Io写状态记录
	//	HANDLE              m_hRecvEvent;   // [ranqd]  数据接收事件

	DWORD               m_dwMySendCount;// [ranqd]  通过封装发送的数据总长度
	DWORD               m_dwSendCount;  // [ranqd]  实际发送总数据长度
	DWORD               m_dwRecvCount;  // [ranqd]  接收总数据长度

	zTCPTask*           pTask;          // [ranqd]  该Sock对应的Task指针

	t_BufferCmdQueue tQueue;             // [ranqd]  封包缓冲队列

	t_BufferCmdQueue _rcv_queue;        /**< 接收缓冲指令队列 */
	DWORD _rcv_raw_size;          /**< 接收缓冲解密数据大小 */
	t_BufferCmdQueue _snd_queue;        /**< 加密缓冲指令队列 */
	t_BufferCmdQueue _enc_queue;        /**< 加密缓冲指令队列 */
	DWORD _current_cmd;
	zMutex mutex;                /**< 锁 */

	zTime last_check_time;            /**< 最后一次检测时间 */

	DWORD bitmask;            /**< 标志掩码 */
	CEncrypt enc;                /**< 加密方式 */

	inline void set_flag(DWORD _f) { bitmask |= _f; }
	inline bool isset_flag(DWORD _f) const { return bitmask & _f; }
	inline void clear_flag(DWORD _f) { bitmask &= ~_f; }
	inline bool need_enc() const { return CEncrypt::ENCDEC_NONE!=enc.getEncMethod(); }
	/**
	* \brief 返回数据包包头最小长度
	* \return 最小长度
	*/
	inline DWORD packetMinSize() const { return PH_LEN; }

	/**
	* \brief 返回整个数据包的长度
	* \param in 数据包
	* \return 返回整个数据包的长度
	*/
	inline DWORD packetSize(const BYTE *in) const { return PH_LEN + ((*((DWORD *)in)) & PACKET_MASK); }

	inline int sendRawData(const void *pBuffer,const int nSize);
	inline bool sendRawDataIM(const void *pBuffer,const int nSize);
	inline int sendRawData_NoPoll(const void *pBuffer,const int nSize);
	inline bool setNonblock();
	inline int waitForRead();
	inline int waitForWrite();
	inline int recvToBuf();

	inline DWORD packetUnpack(BYTE *in,const DWORD nPacketLen,BYTE *out);
	template<typename buffer_type>
	inline DWORD packetAppend(const void *pData,const DWORD nLen,buffer_type &cmd_queue);
	template<typename buffer_type>
	inline DWORD packetAppendNoEnc(const void *pData,const DWORD nLen,buffer_type &cmd_queue);
	template<typename buffer_type>
	inline DWORD packetPackEnc(buffer_type &cmd_queue,const DWORD current_cmd,DWORD offset = 0);
public:
	template<typename buffer_type>
	static inline DWORD packetPackZip(const void *pData,const DWORD nLen,buffer_type &cmd_queue,const bool _compress = true);

};
/**
* \brief 封装了线程操作，所有使用线程的基类
*
*/
class zThread : private zNoncopyable
{
public:

	/**
	* \brief 构造函数，创建一个对象
	*
	* \param name 线程名称
	* \param joinable 标明这个线程退出的时候是否保存状态，如果为true表示线程退出保存状态，否则将不保存退出状态
	*/
	zThread(const std::string &name = std::string("zThread"),const bool joinable = true) 
		: threadName(name),alive(false),complete(false),joinable(joinable) { m_hThread = NULL; };

	/**
	* \brief 析构函数，用于销毁一个对象，回收对象空间
	*
	*/
	virtual ~zThread()
	{
		if (NULL != m_hThread)
		{
			CloseHandle(m_hThread);
		}
	};

	/**
	* \brief 使当前线程睡眠指定的时间，秒
	*
	*
	* \param sec 指定的时间，秒
	*/
	static void sleep(const long sec)
	{
		::Sleep(1000 * sec);
	}

	/**
	* \brief 使当前线程睡眠指定的时间，毫秒
	*
	*
	* \param msec 指定的时间，毫秒
	*/
	static void msleep(const long msec)
	{
		::Sleep(msec);
	}

	/**
	* \brief 使当前线程睡眠指定的时间，微秒
	*
	*
	* \param usec 指定的时间，微秒
	*/
	static void usleep(const long usec)
	{
		::Sleep(usec / 1000);
	}

	/**
	* \brief 线程是否是joinable的
	*
	*
	* \return joinable
	*/
	const bool isJoinable() const
	{
		return joinable;
	}

	/**
	* \brief 检查线程是否在运行状态
	*
	* \return 线程是否在运行状态
	*/
	const bool isAlive() const
	{
		return alive;
	}

	static DWORD WINAPI threadFunc(void *arg);
	bool start();
	void join();

	/**
	* \brief 主动结束线程
	*
	* 其实只是设置标记，那么线程的run主回调循环回检查这个标记，如果这个标记已经设置，就退出循环
	*
	*/
	void final()
	{
		complete = true;
	}

	/**
	* \brief 判断线程是否继续运行下去
	*
	* 主要用在run()函数循环中，判断循环是否继续执行下去
	*
	* \return 线程主回调是否继续执行
	*/
	const bool isFinal() const 
	{
		return complete;
	}

	/**
	* \brief 纯虚构函数，线程主回调函数，每个需要实例华的派生类需要重载这个函数
	*
	* 如果是无限循环需要在每个循环检查线程退出标记isFinal()，这样能够保证线程安全退出
	* <pre>
	*   while(!isFinal())
	*   {
	*     ...
	*   }
	*   </pre>
	*
	*/
	virtual void run() = 0;

	/**
	* \brief 判断两个线程是否是同一个线程
	* \param other 待比较的线程
	* \return 是否是同一个线程
	*/
	//bool operator==(const zThread& other) const
	//{
	// return pthread_equal(thread,other.thread) != 0;
	//}

	/**
	* \brief 判断两个线程是否不是同一个线程
	* \param other 待比较的线程
	* \return 是否不是同一个线程
	*/
	//bool operator!=(const zThread& other) const
	//{
	//return !operator==(other);
	//}

	/**
	* \brief 返回线程名称
	*
	* \return 线程名称
	*/
	const std::string &getThreadName() const
	{
		return threadName;
	}

public:

	std::string threadName;      /**< 线程名称 */
	zMutex mlock;          /**< 互斥锁 */
	volatile bool alive;      /**< 线程是否在运行 */
	volatile bool complete;
	HANDLE m_hThread;        /**< 线程编号 */
	bool joinable;          /**< 线程属性，是否设置joinable标记 */

}; 

class zIocpRecvThread : public zThread
{
public:
	zIocpRecvThread():zThread("zIocpRecvThread"){};
	~zIocpRecvThread(){};

	void run();
};

class zIocp
{
public:
	zIocp();
	~zIocp();

	static zIocp &getInstance()
	{
		if (NULL == instance)
		{
			instance = new zIocp();
			instance->start();
		}

		return *instance;
	}
	static zIocp *instance;

	void start();

	DWORD m_dwThreadCount;

	std::vector<zIocpRecvThread*> m_RecvThreadList;
	HANDLE m_ComPort;
	void BindIocpPort(HANDLE hIo, zSocket* key);// 绑定IO端口
	void PostStatus(zSocket* key, LPOverlappedData lpOverLapped);
	void UpdateNetLog(); // 输出网络流量
};

/**
* \brief 对数据进行组织,需要时压缩,不加密
* \param pData 待组织的数据，输入
* \param nLen 待拆包的数据长度，输入
* \param cmd_queue 输出，存放数据
* \return 封包后的大小
*/
template<typename buffer_type>
inline DWORD zSocket::packetAppendNoEnc(const void *pData,const DWORD nLen,buffer_type &cmd_queue)
{
	//	Zebra::logger->debug("输入长度1： %d", nLen);
	int nSize = packetPackZip(pData,nLen,cmd_queue,PACKET_ZIP == (bitmask & PACKET_ZIP));
	//	Zebra::logger->debug("封包长度1： %d", nSize);	

	return nSize;

}

/**
* \brief 对数据进行组织,需要时压缩和加密
* \param pData 待组织的数据，输入
* \param nLen 待拆包的数据长度，输入
* \param cmd_queue 输出，存放数据
* \return 封包后的大小
*/
template<typename buffer_type>
inline DWORD zSocket::packetAppend(const void *pData,const DWORD nLen,buffer_type &cmd_queue)
{
	//	Zebra::logger->debug("输入长度2： %d", nLen);
	t_StackCmdQueue t_cmd_queue;
	DWORD nSize = packetPackZip( pData,nLen,t_cmd_queue,PACKET_ZIP == (bitmask & PACKET_ZIP));
	if (need_enc())
		nSize = packetPackEnc(t_cmd_queue,t_cmd_queue.rd_size());
	//	Zebra::logger->debug("封包长度2： %d", nSize);
	PACK_HEAD head;
	head.Len = t_cmd_queue.rd_size();
	cmd_queue.put((BYTE*)&head, sizeof(head));
	cmd_queue.put(t_cmd_queue.rd_buf(), t_cmd_queue.rd_size());
	return nSize;
}

/**
* \brief         对数据进行加密
* \param cmd_queue    待加密的数据，输入输出
* \param current_cmd  最后一个指令长度
* \param offset    待加密数据的偏移
* \return         返回加密以后真实数据的大小
*/
template<typename buffer_type>
inline DWORD zSocket::packetPackEnc(buffer_type &cmd_queue,const DWORD current_cmd,DWORD offset)
{
	DWORD mod = (cmd_queue.rd_size() - offset) % 8;
	if (0!=mod)
	{
		mod = 8 - mod;
		// [ranqd] 这样似乎更合理
		//(*(DWORD *)(&(cmd_queue.rd_buf()[cmd_queue.rd_size() - current_cmd - PACKLASTSIZE]))) += mod;
		(*(DWORD *)(&(cmd_queue.rd_buf()[cmd_queue.rd_size() - current_cmd]))) += mod;
		cmd_queue.wr_flip(mod);
	}

	//加密动作
	enc.encdec(&cmd_queue.rd_buf()[offset],cmd_queue.rd_size() - offset,true);

	return cmd_queue.rd_size();
}

/**
* \brief       对数据进行压缩,由上层判断是否需要加密,这里只负责加密不作判断
* \param pData   待压缩的数据，输入
* \param nLen     待压缩的数据长度，输入
* \param pBuffer   输出，存放压缩以后的数据
* \param _compress  当数据包过大时候是否压缩
* \return       返回加密以后真实数据的大小
*/
template<typename buffer_type>
inline DWORD zSocket::packetPackZip(const void *pData,const DWORD nLen,buffer_type &cmd_queue,const bool _compress)
{
	/*if (nLen > MAX_DATASIZE)
	{
	Cmd::t_NullCmd *cmd = (Cmd::t_NullCmd *)pData;
	Zebra::logger->warn("zSocket::packetPackZip: 发送的数据包过大(cmd = %u,para = %u",cmd->cmd,cmd->para);
	}*/
	DWORD nSize = nLen > MAX_DATASIZE ? MAX_DATASIZE : nLen;//nLen & PACKET_MASK;
	DWORD nMask = 0;//nLen & (~PACKET_MASK);
	if (nSize > PACKET_ZIP_MIN /*数据包过大*/ 
		&& _compress /*带压缩标记，数据包需要压缩*/
		/*&& !(nMask & PACKET_ZIP)*/ /*数据包过大可能已经是压缩过的*/ )
	{
		uLong nZipLen = unzip_size(nSize);
		cmd_queue.wr_reserve(nZipLen + PH_LEN);
		int retcode = compress(&(cmd_queue.wr_buf()[PH_LEN]),&nZipLen,(const Bytef *)pData,nSize);
		switch(retcode)
		{
		case Z_OK:
			break;
		case Z_MEM_ERROR:
			Zebra::logger->fatal("zSocket::packetPackZip Z_MEM_ERROR.");
			break;
		case Z_BUF_ERROR:
			Zebra::logger->fatal("zSocket::packetPackZip Z_BUF_ERROR.");
			break;
		}
		nSize = nZipLen;
		nMask |= PACKET_ZIP;
	}
	else
	{
		cmd_queue.wr_reserve(nSize + PH_LEN);
		bcopy((void *)pData,&(cmd_queue.wr_buf()[PH_LEN]),nSize,cmd_queue.wr_size());
	}

	(*(DWORD *)(cmd_queue.wr_buf())) = (nSize | nMask);

	cmd_queue.wr_flip(nSize + PH_LEN);

	return nSize + PH_LEN;
}

/**
* \brief 定义了消息处理接口，所有接收到的TCP数据指令需要通过这个接口来处理
*/
class zProcessor
{
public:
	virtual bool msgParse(const Cmd::t_NullCmd *,const DWORD) = 0;
};
/**
* \brief 指令流量分析
*/
struct CmdAnalysis
{
	CmdAnalysis(const char *disc,DWORD time_secs):_log_timer(time_secs)
	{
		bzero(_disc,sizeof(disc));
		strncpy(_disc,disc,sizeof(_disc)-1);
		bzero(_data,sizeof(_data));
		_switch=false;
	}
	struct
	{
		DWORD num;
		DWORD size;
	}_data[256][256] ;
	zMutex _mutex;
	Timer _log_timer;
	char _disc[256];
	bool _switch;//开关
	void add(const BYTE &cmd,const BYTE &para,const DWORD &size)
	{
		if (!_switch)
		{
			return;
		}
		_mutex.lock(); 
		_data[cmd][para].num++;
		_data[cmd][para].size +=size;
		zRTime ct;
		if (_log_timer(ct))
		{
			for(int i = 0 ; i < 256 ; i ++)
			{
				for(int j = 0 ; j < 256 ; j ++)
				{
					if (_data[i][j].num)
						Zebra::logger->debug("%s:%d,%d,%d,%d",_disc,i,j,_data[i][j].num,_data[i][j].size);
				}
			}
			bzero(_data,sizeof(_data));
		}
		_mutex.unlock(); 
	}
};

/**
* \brief zTCPServer类，封装了服务器监听模块，可以方便的创建一个服务器对象，等待客户端的连接
*
*/
class zTCPServer : private zNoncopyable
{

public:

	zTCPServer(const std::string &name);
	~zTCPServer();
	bool bind(const std::string &name,const WORD port);
	int accept(struct sockaddr_in *addr);

private:

	static const int T_MSEC =2100;      /**< 轮询超时，毫秒 */
	static const int MAX_WAITQUEUE = 2000;  /**< 最大等待队列 */

	std::string name;            /**< 服务器名称 */
	SOCKET sock;                /**< 套接口 */
}; 

/**
* \brief 定义一个任务类，是线程池的工作单元
*
*/
class zTCPTask : public zProcessor,private zNoncopyable
{

public:

	/**
	* \brief 连接断开方式
	*
	*/
	enum TerminateMethod
	{
		terminate_no,              /**< 没有结束任务 */
		terminate_active,            /**< 客户端主动断开连接，主要是由于服务器端检测到套接口关闭或者套接口异常 */
		terminate_passive,            /**< 服务器端主动断开连接 */
	};

	/**
	* \brief 构造函数，用于创建一个对象
	*
	*
	* \param pool 所属连接池指针
	* \param sock 套接口
	* \param addr 地址
	* \param compress 底层数据传输是否支持压缩
	* \param checkSignal 是否发送网络链路测试信号
	*/
	zTCPTask(
		zTCPTaskPool *pool,
		const SOCKET sock,
		const struct sockaddr_in *addr = NULL,
		const bool compress = false,
		const bool checkSignal = true,
		const bool useIocp = USE_IOCP ) :pool(pool),lifeTime(),_checkSignal(checkSignal),_ten_min(600),tick(false)
	{
		terminate = terminate_no;
		terminate_wait = false; 
		fdsradd = false; 
		buffered = false;
		state = notuse;
		mSocket = NULL;
		mSocket = new zSocket( sock,addr,compress, useIocp,this );
		if( mSocket == NULL )
		{
			Zebra::logger->error("new zSocket时内存不足！");
		}
	}

	/**
	* \brief 析构函数，用于销毁一个对象
	*
	*/
	virtual ~zTCPTask() 
	{
		if( mSocket != NULL )
		{
			if(mSocket->SafeDelete( false ))
				delete mSocket;
			mSocket = NULL;
		}
	}

	/**
	* \brief 填充pollfd结构
	* \param pfd 待填充的结构
	* \param events 等待的事件参数
	*/
	void fillPollFD(struct mypollfd &pfd,short events)
	{
		mSocket->fillPollFD(pfd,events);
	}

	/**
	* \brief 检测是否验证超时
	*
	*
	* \param ct 当前系统时间
	* \param interval 超时时间，毫秒
	* \return 检测是否成功
	*/
	bool checkVerifyTimeout(const zRTime &ct,const QWORD interval = 5000) const
	{
		return (lifeTime.elapse(ct) > interval);
	}

	/**
	* \brief 检查是否已经加入读事件
	*
	* \return 是否加入
	*/
	bool isFdsrAdd()
	{
		return fdsradd;
	}
	/**
	* \brief 设置加入读事件标志
	*
	* \return 是否加入
	*/
	bool fdsrAdd()
	{
		fdsradd=true;
		return fdsradd;
	}


	/**
	* \brief 连接验证函数
	*
	* 子类需要重载这个函数用于验证一个TCP连接，每个TCP连接必须通过验证才能进入下一步处理阶段，缺省使用一条空的指令作为验证指令
	* <pre>
	* int retcode = mSocket->recvToBuf_NoPoll();
	* if (retcode > 0)
	* {
	*     BYTE pstrCmd[zSocket::MAX_DATASIZE];
	*     int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
	*     if (nCmdLen <= 0)
	*       //这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
	*       return 0;
	*     else
	*     {
	*       zSocket::t_NullCmd *pNullCmd = (zSocket::t_NullCmd *)pstrCmd;
	*       if (zSocket::null_opcode == pNullCmd->opcode)
	*       {
	*         std::cout << "客户端连接通过验证" << std::endl;
	*         return 1;
	*       }
	*       else
	*       {
	*         return -1;
	*       }
	*     }
	* }
	* else
	*     return retcode;
	* </pre>
	*
	* \return 验证是否成功，1表示成功，可以进入下一步操作，0，表示还要继续等待验证，-1表示等待验证失败，需要断开连接
	*/
	virtual int verifyConn()
	{
		return 1;
	}

	/**
	* \brief 等待其它线程同步验证这个连接，有些线程池不需要这步，所以不用重载这个函数，缺省始终返回成功
	*
	* \return 等待是否成功，1表示成功，可以进入下一步操作，0，表示还要继续等待，-1表示等待失败或者等待超时，需要断开连接
	*/
	virtual int waitSync()
	{
		return 1;
	}

	/**
	* \brief 回收是否成功，回收成功以后，需要删除这个TCP连接相关资源
	*
	* \return 回收是否成功，1表示回收成功，0表示回收不成功
	*/
	virtual int recycleConn()
	{
		return 1;
	}

	/**
	* \brief 一个连接任务验证等步骤完成以后，需要添加到全局容器中
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void addToContainer() {}

	/**
	* \brief 连接任务退出的时候，需要从全局容器中删除
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void removeFromContainer() {}

	/**
	* \brief 添加到外部容器，这个容器需要保证这个连接的唯一性
	*
	* \return 添加是否成功
	*/
	virtual bool uniqueAdd()
	{
		return true;
	}

	/**
	* \brief 从外部容器删除，这个容器需要保证这个连接的唯一性
	*
	* \return 删除是否成功
	*/
	virtual bool uniqueRemove()
	{
		return true;
	}

	/**
	* \brief 设置唯一性验证通过标记
	*
	*/
	void setUnique()
	{
		uniqueVerified = true;
	}

	/**
	* \brief 判断是否已经通过了唯一性验证
	*
	* \return 是否已经通过了唯一性标记
	*/
	bool isUnique() const
	{
		return uniqueVerified;
	}

	/**
	* \brief 判断是否被其它线程设置为等待断开连接状态
	*
	* \return true or false
	*/
	bool isTerminateWait()
	{
		return terminate_wait; 
	}


	/**
	* \brief 判断是否被其它线程设置为等待断开连接状态
	*
	* \return true or false
	*/
	void TerminateWait()
	{
		terminate_wait=true; 
	}

	/**
	* \brief 判断是否需要关闭连接
	*
	* \return true or false
	*/
	bool isTerminate() const
	{
		return terminate_no != terminate;
	}

	/**
	* \brief 需要主动断开客户端的连接
	*
	* \param method 连接断开方式
	*/
	virtual void Terminate(const TerminateMethod method = terminate_passive)
	{
		terminate = method;
	}

	virtual bool sendCmd(const void *,int);
	bool sendCmdNoPack(const void *,int);
	virtual bool ListeningRecv(bool);
	virtual bool ListeningSend();

	/**
	* \brief 连接任务状态
	*
	*/
	enum zTCPTask_State
	{
		notuse    =  0,            /**< 连接关闭状态 */
		verify    =  1,            /**< 连接验证状态 */
		sync    =  2,            /**< 等待来自其它服务器的验证信息同步 */
		okay    =  3,            /**< 连接处理阶段，验证通过了，进入主循环 */
		recycle    =  4              /**< 连接退出状态，回收 */
	};

	/**
	* \brief 获取连接任务当前状态
	* \return 状态
	*/
	const zTCPTask_State getState() const
	{
		return state;
	}

	/**
	* \brief 设置连接任务状态
	* \param state 需要设置的状态
	*/
	void setState(const zTCPTask_State state)
	{
		this->state = state;
	}

	void getNextState();
	void resetState();

	/**
	* \brief 获得状态的字符串描述
	*
	*
	* \param state 状态
	* \return 返回状态的字符串描述
	*/
	const char *getStateString(const zTCPTask_State state) const
	{
		const char *retval = NULL;

		switch(state)
		{
		case notuse:
			retval = "notuse";
			break;
		case verify:
			retval = "verify";
			break;
		case sync:
			retval = "sync";
			break;
		case okay:
			retval = "okay";
			break;
		case recycle:
			retval = "recycle";
			break;
		default:
			retval = "none";
			break;
		}

		return retval;
	}

	/**
	* \brief 返回连接的IP地址
	* \return 连接的IP地址
	*/
	const char *getIP() const
	{
		return mSocket->getIP();
	}
	const DWORD getAddr() const
	{
		return mSocket->getAddr();
	}

	const WORD getPort()
	{
		return mSocket->getPort();
	}

	int WaitRecv( bool bWait = false, int timeout = 0 )
	{
		return mSocket->WaitRecv( bWait, timeout );
	}

	int WaitSend( bool bWait = false, int timeout = 0 )
	{
		return mSocket->WaitSend( bWait, timeout );
	}

	bool UseIocp()
	{
		return mSocket->m_bUseIocp;
	}
	/**
	* \brief 是否发送网络连接链路测试信号
	* \return true or false
	*/
	const bool ifCheckSignal() const
	{
		return _checkSignal;
	}

	/**
	* \brief 检测测试信号发送间隔
	*
	* \return 检测是否成功
	*/
	bool checkInterval(const zRTime &ct)
	{
		return _ten_min(ct);
	}

	/**
	* \brief 检查测试信号，如果测试信号在规定时间内返回，那么重新发送测试信号，没有返回的话可能TCP连接已经出错了
	*
	* \return true，表示检测成功；false，表示检测失败 
	*/
	bool checkTick() const
	{
		return tick;
	}

	/**
	* \brief 测试信号已经返回了
	*
	*/
	void clearTick()
	{
		tick = false;
	}

	/**
	* \brief 发送测试信号成功
	*
	*/
	void setTick()
	{
		tick = true;
	}
	zTCPTaskPool *getPool()
	{
		return pool; 
	}

	void checkSignal(const zRTime &ct);

	static CmdAnalysis analysis;
protected:

	bool buffered;                  /**< 发送指令是否缓冲 */
	//	zSocket mSocket;                /**< 底层套接口 */
	zSocket* mSocket;              // [ranqd] 修改为指针

	zTCPTask_State state;              /**< 连接状态 */

private:

	zTCPTaskPool *pool;                /**< 任务所属的池 */
	TerminateMethod terminate;            /**< 是否结束任务 */
	bool terminate_wait;              /**< 其它线程设置等待断开连接状态,由pool线程设置断开连接状态 */
	bool fdsradd;                  /**< 读事件添加标志 */
	zRTime lifeTime;                /**< 连接创建时间记录 */

	bool uniqueVerified;              /**< 是否通过了唯一性验证 */
	const bool _checkSignal;            /**< 是否发送链路检测信号 */
	Timer _ten_min;
	bool tick;

};

/**
* \brief 对线程进行分组管理的类
*
*/
class zThreadGroup : private zNoncopyable
{

public:

	struct Callback
	{
		virtual void exec(zThread *e)=0;
		virtual ~Callback(){};
	};

	typedef std::vector<zThread *> Container;  /**< 容器类型 */

	zThreadGroup();
	~zThreadGroup();
	void add(zThread *thread);
	zThread *getByIndex(const Container::size_type index);
	zThread *operator[] (const Container::size_type index);
	void joinAll();
	void execAll(Callback &cb);

	const Container::size_type size()
	{
		zRWLock_scope_rdlock scope_rdlock(rwlock);
		return vts.size();
	}

private:

	Container vts;                /**< 线程向量 */
	zRWLock rwlock;                /**< 读写锁 */

};

/**
* \brief 连接线程池类，封装了一个线程处理多个连接的线程池框架
*
*/
class zTCPTaskPool : private zNoncopyable
{

public:

	/**
	* \brief 构造函数
	* \param maxConns 线程池并行处理有效连接的最大数量
	* \param state 初始化的时候连接线程池的状态
	*/
	explicit zTCPTaskPool(const int maxConns,const int state,const int us=50000) : maxConns(maxConns),state(state)/*,usleep_time(us)// */
	{
		setUsleepTime(us);
		syncThread = NULL;
		recycleThread = NULL;
		maxThreadCount = minThreadCount;
	};

	/**
	* \brief 析构函数，销毁一个线程池对象
	*
	*/
	~zTCPTaskPool()
	{
		final();
	}

	/**
	* \brief 获取连接线程池当前状态
	*
	* \return 返回连接线程池的当前状态
	*/
	const int getState() const
	{
		return state;
	}

	/**
	* \brief 设置连接线程池状态
	*
	* \param state 设置的状态标记位
	*/
	void setState(const int state)
	{
		this->state |= state;
	}

	/**
	* \brief 清楚连接线程池状态
	*
	* \param state 清楚的状态标记位
	*/
	void clearState(const int state)
	{
		this->state &= ~state;
	}

	const int getSize();
	inline const int getMaxConns() const { return maxConns; }
	bool addVerify(zTCPTask *task);
	void addSync(zTCPTask *task);
	bool addOkay(zTCPTask *task);
	void addRecycle(zTCPTask *task);
	static void  setUsleepTime(int time)
	{
		usleep_time=time;
	}

	bool init();
	void final();

private:

	const int maxConns;                    /**< 线程池并行处理连接的最大数量 */

	static const int maxVerifyThreads = 4;          /**< 最大验证线程数量 */
	zThreadGroup verifyThreads;                /**< 验证线程，可以有多个 */

	zSyncThread *syncThread;                /**< 等待同步线程 */

	static const int minThreadCount = 1;          /**< 线程池中同时存在主处理线程的最少个数 */
	int maxThreadCount;                    /**< 线程池中同时存在主处理线程的最大个数 */
	zThreadGroup okayThreads;                /**< 处理主线程，多个 */

	zRecycleThread *recycleThread;              /**< 连接回收线程 */

	int state;                        /**< 连接池状态 */
public:
	static int usleep_time;                    /**< 循环等待时间 */

};

/**
* \brief 定义了服务器的框架基类
*
* 所有服务器程序实体需要继承这个类，并且不管是其有多少个子类，整个运行环境只有一个类的实例<br>
* 只要派生类使用Singleton设计模式实现就可以了
*
*/
class zService : private zNoncopyable
{

public:
	Timer  _one_sec_; // 秒定时器
	/**
	* \brief 虚析构函数
	*
	*/
	virtual ~zService() { serviceInst = NULL; };

	/**
	* \brief 重新读取配置文件，为HUP信号的处理函数
	*
	* 缺省什么事情都不干，只是简单输出一个调试信息，重载这个函数干想干的事情
	*
	*/
	virtual void reloadConfig()
	{
	}

	/**
	* \brief 判断主循环是否结束
	*
	* 如果返回true，将结束主回调
	*
	* \return 主循环是否结束
	*/
	bool isTerminate() const
	{
		return terminate;
	}

	/**
	* \brief 结束主循环，也就是结束主回调函数
	*
	*/
	void Terminate()
	{
		terminate = true;
	}

	void main();

	/**
	* \brief 返回服务的实例指针
	*
	* \return 服务的实例指针
	*/
	static zService *serviceInstance()
	{
		return serviceInst;
	}

	zProperties env;        /**< 存储当前运行系统的环境变量 */

protected:

	/**
	* \brief 构造函数
	*
	*/
	zService(const std::string &name) : name(name),_one_sec_(1)
	{
		serviceInst = this;

		terminate = false;
	}

	virtual bool init();

	/**
	* \brief 确认服务器初始化成功，即将进入主回调函数
	*
	* \return 确认是否成功
	*/
	virtual bool validate()
	{
		return true;
	}

	/**
	* \brief 服务程序的主回调函数，主要用于监听服务端口，如果返回false将结束程序，返回true继续执行服务
	*
	* \return 回调是否成功
	*/
	virtual bool serviceCallback() = 0;

	/**
	* \brief 结束服务器程序，回收资源，纯虚函数，子类需要实现这个函数
	*
	*/
	virtual void final() = 0;

private:

	static zService *serviceInst;    /**< 类的唯一实例指针，包括派生类，初始化为空指针 */

	std::string name;          /**< 服务名称 */
	bool terminate;            /**< 服务结束标记 */

};
class zAcceptThread;
/**
* \brief 网络服务器类
*
* 实现了网络服务器框架代码，这个类比较通用一点
*
*/
class zNetService : public zService
{

public:
	/**
	* \brief 虚析构函数
	*
	*/
	virtual ~zNetService() { instance = NULL; };

	/**
	* \brief 根据得到的TCP/IP连接获取一个连接任务
	*
	* \param sock TCP/IP套接口
	* \param addr 地址
	*/
	virtual void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr) = 0;

	/**
	* \brief 获取连接池中的连接数
	*
	* \return 连接数
	*/
	virtual const int getPoolSize() const
	{
		return 0;
	}

	/**
	* \brief 获取连接池状态
	*
	* \return 连接池状态
	*/
	virtual const int getPoolState() const
	{
		return 0;
	}

protected:

	/**
	* \brief 构造函数
	* 
	* 受保护的构造函数，实现了Singleton设计模式，保证了一个进程中只有一个类实例
	*
	* \param name 名称
	*/
	zNetService(const std::string &name) : zService(name)
	{
		instance = this;

		serviceName = name;
		tcpServer = NULL;
	}

	bool init(WORD port);
	bool serviceCallback();
	void final();

private:

	static zNetService *instance;    /**< 类的唯一实例指针，包括派生类，初始化为空指针 */
	std::string serviceName;      /**< 网络服务器名称 */

	zAcceptThread* pAcceptThread; // [ranqd] 接收连接线程
public:
	zTCPServer *tcpServer;        /**< TCP服务器实例指针 */
};
// [ranqd] 接收连接线程类
class zAcceptThread : public zThread
{
public:
	zAcceptThread( zNetService* p, const std::string &name ): zThread(name)
	{
		pService = p;
	}
	~zAcceptThread()
	{
		final();
		join();
	}
	zNetService* pService;

	void run()         // [ranqd] 接收连接线程函数
	{
		while(!isFinal())
		{
			//Zebra::logger->debug("接收连接线程建立！");
			struct sockaddr_in addr;
			if( pService->tcpServer != NULL )
			{
				int retcode = pService->tcpServer->accept(&addr);
				if (retcode >= 0) 
				{
					//接收连接成功，处理连接
					pService->newTCPTask(retcode,&addr);
				}
			}
		}
	}
};

class SuperClient;

/**
* \brief 网络服务器框架代码
*
* 在需要与管理服务器建立连接的网络服务器中使用
*
*/
class zSubNetService : public zNetService
{

public:

	virtual ~zSubNetService();

	/**
	* \brief 获取类的唯一实例
	*
	* 这个类实现了Singleton设计模式，保证了一个进程中只有一个类的实例
	*
	*/
	static zSubNetService *subNetServiceInstance()
	{
		return subNetServiceInst;
	}

	/**
	* \brief 解析来自管理服务器的指令
	*
	* 这些指令是与具体的服务器有关的，因为通用的指令都已经处理了
	*
	* \param pNullCmd 待处理的指令
	* \param nCmdLen 指令长度
	* \return 解析是否成功
	*/
	virtual bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen) = 0;

	bool sendCmdToSuperServer(const void *pstrCmd,const int nCmdLen);
	void setServerInfo(const Cmd::Super::t_Startup_Response *ptCmd);
	void addServerEntry(const Cmd::Super::ServerEntry &entry);
	const Cmd::Super::ServerEntry *getServerEntryById(const WORD wdServerID);
	const Cmd::Super::ServerEntry *getServerEntryByType(const WORD wdServerType);
	const Cmd::Super::ServerEntry *getNextServerEntryByType(const WORD wdServerType,const Cmd::Super::ServerEntry **prev);

	/**
	* \brief 返回服务器编号
	*
	* \return 服务器编号
	*/
	const WORD getServerID() const
	{
		return wdServerID;
	}

	/**
	* \brief 返回服务器类型
	*
	* \return 服务器类型
	*/
	const WORD getServerType() const
	{
		return wdServerType;
	}

protected:

	zSubNetService(const std::string &name,const WORD wdType);

	bool init();
	bool validate();
	void final();

	WORD wdServerID;          /**< 服务器编号，一个区唯一的 */
	WORD wdServerType;          /**< 服务器类型，创建类实例的时候已经确定 */
	char pstrIP[MAX_IP_LENGTH];      /**< 服务器内网地址 */
	WORD wdPort;            /**< 服务器内网端口 */

private:
	SuperClient *superClient;    /**< 管理服务器的客户端实例 */

	static zSubNetService *subNetServiceInst;      /**< 类的唯一实例指针，包括派生类，初始化为空指针 */
	zMutex mlock;                    /**< 关联服务器信息列表访问互斥体 */
	std::deque<Cmd::Super::ServerEntry> serverList;    /**< 关联服务器信息列表，保证服务器之间的验证关系 */

};

/**
* \brief zMTCPServer类，封装了服务器监听模块，可以方便的创建一个服务器对象，等待客户端的连接
* 可以同时监听多个端口
*/
class zMTCPServer : private zNoncopyable
{
public:
	typedef std::map<int,WORD> Sock2Port;
	typedef Sock2Port::value_type Sock2Port_value_type;
	typedef Sock2Port::iterator Sock2Port_iterator;
	typedef Sock2Port::const_iterator Sock2Port_const_iterator;

	zMTCPServer(const std::string &name);
	~zMTCPServer() ;

	bool bind(const std::string &name,const WORD port);
	int accept(Sock2Port &res);

private:

	static const int T_MSEC =2100;      /**< 轮询超时，毫秒 */
	static const int MAX_WAITQUEUE = 2000;  /**< 最大等待队列 */

	std::string name;            /**< 服务器名称 */
	Sock2Port mapper;
	zMutex mlock;
	std::vector<struct mypollfd> pfds;

}; 

/**
* \brief 网络服务器类
* 实现了网络服务器框架代码，这个类比较通用一点
*/
class zMNetService : public zService
{

public:

	/**
	* \brief 虚析构函数
	*/
	virtual ~zMNetService() { instance = NULL; };

	/**
	* \brief 根据得到的TCP/IP连接获取一个连接任务
	* \param sock TCP/IP套接口
	* \param srcPort 由于服务器绑定了多个端口，这个参数指定连接来自那个绑定端口
	* \return 新建立的连接任务
	*/
	virtual void newTCPTask(const SOCKET sock,const WORD srcPort) = 0;

	/**
	* \brief 绑定服务到某个端口
	* \param name 待绑定端口名称
	* \param port 待绑定的端口
	* \return 绑定是否成功
	*/
	bool bind(const std::string &name,const WORD port)
	{
		if (tcpServer)
			return tcpServer->bind(name,port);
		else
			return false;
	}

protected:

	/**
	* \brief 构造函数
	* 受保护的构造函数，实现了Singleton设计模式，保证了一个进程中只有一个类实例
	* \param name 名称
	*/
	zMNetService(const std::string &name) : zService(name)
	{
		instance = this;

		serviceName = name;
		tcpServer = NULL;
	}

	bool init();
	bool serviceCallback();
	void final();

private:
	static zMNetService *instance;    /**< 类的唯一实例指针，包括派生类，初始化为空指针 */
	std::string serviceName;      /**< 网络服务器名称 */
	zMTCPServer *tcpServer;        /**< TCP服务器实例指针 */
};

/**
* \brief TCP客户端
*
* 封装了一些TCP客户端的逻辑，比如建立连接等等，在实际应用中，需要派生这个类，并重载解析指令的函数msgParse
*
*/
class zTCPClient : public zThread,public zProcessor
{

public:

	/**
	* \brief 构造函数，创建实例对象，初始化对象成员
	*
	*
	* \param name 名称
	* \param ip 地址
	* \param port 端口
	* \param compress 底层数据传输是否支持压缩
	*/
	zTCPClient(
		const std::string &name,
		const std::string &ip = "127.0.0.1",
		const WORD port = 80,
		const bool compress = false) 
		: zThread(name),ip(ip),port(port),pSocket(NULL),compress(compress) {};

	/**
	* \brief 析构函数，销毁对象
	*
	*/
	~zTCPClient() 
	{
		close();
	}

	bool connect();

	/**
	* \brief 建立一个到服务器的TCP连接，指定服务器的IP地址和端口
	*
	*
	* \param ip 服务器的IP地址
	* \param port 服务器的端口
	* \return 连接是否成功
	*/
	bool connect(const char *ip,const WORD port)
	{
		this->ip = ip;
		this->port = port;
		return connect();
	}

	/**
	* \brief 关闭客户端连接
	*
	*/
	virtual void close()
	{
		//SAFE_DELETE( pSocket );
		if( pSocket != NULL )
		{
			if(pSocket->SafeDelete( false ))
				delete pSocket;
			pSocket = NULL;
		}
	}

	virtual bool sendCmd(const void *pstrCmd,const int nCmdLen);

	/**
	* \brief 设置服务器IP地址
	*
	*
	* \param ip 设置的服务器IP地址
	*/
	void setIP(const char *ip)
	{
		this->ip = ip;
	}

	/**
	* \brief 获取服务器IP地址
	*
	*
	* \return 返回地址
	*/
	const char *getIP() const
	{
		return ip.c_str();
	}

	/**
	* \brief 设置服务器端口
	*
	*
	* \param port 设置的服务器端口
	*/
	void setPort(const WORD port)
	{
		this->port = port;
	}

	/**
	* \brief 获取服务器端口
	*
	*
	* \return 返回端口
	*/
	const WORD getPort() const
	{
		return port;
	}

	virtual void run();
	//指令分析
	static CmdAnalysis analysis;

protected:

	std::string ip;                  /**< 服务器地址 */
	WORD port;              /**< 服务器端口 */
	zSocket *pSocket;                /**< 底层套接口 */

	const bool compress;              /**< 是否支持压缩 */

}; 

class zTCPBufferClient : public zTCPClient
{

public:

	zTCPBufferClient(
		const std::string &name,
		const std::string &ip = "127.0.0.1",
		const WORD port = 80,
		const bool compress = false,
		const int usleep_time = 50000) 
		: zTCPClient(name,ip,port,compress),usleep_time(usleep_time),_buffered(false) { }

	void close()
	{
		sync();
		zTCPClient::close();
	}

	void run();
	bool sendCmd(const void *pstrCmd,const int nCmdLen);
	void setUsleepTime(const int utime)
	{
		usleep_time = utime;
	}

private :

	bool ListeningRecv();
	bool ListeningSend();
	void sync();

	int usleep_time;
	volatile bool _buffered;

};

/**
* \brief TCP客户端
*
* 封装了一些TCP客户端的逻辑，比如建立连接等等
*
*/
class zTCPClientTask : public zProcessor,private zNoncopyable
{

public:

	/**
	* \brief 连接断开类型
	*
	*/
	enum TerminateMethod
	{
		TM_no,          /**< 没有结束任务 */
		TM_sock_error,      /**< 检测到套接口关闭或者套接口异常 */
		TM_service_close      /**< 服务器即将关闭 */
	};

	/**
	* \brief 连接任务状态
	*
	*/
	enum ConnState
	{
		close    =  0,            /**< 连接关闭状态 */
		sync    =  1,            /**< 等待同步状态 */
		okay    =  2,            /**< 连接处理阶段 */
		recycle    =  3              /**< 连接退出状态 */
	};

	/**
	* \brief 构造函数，创建实例对象，初始化对象成员
	* \param ip 地址
	* \param port 端口
	* \param compress 底层数据传输是否支持压缩
	*/
	zTCPClientTask(
		const std::string &ip,
		const WORD port,
		const bool compress = false) : pSocket(NULL),compress(compress),ip(ip),port(port),_ten_min(600)
	{
		state = close;
		terminate = TM_no;
		mainloop = false;
		fdsradd = false; 
	}

	/**
	* \brief 析构函数，销毁对象
	*/
	virtual ~zTCPClientTask() 
	{
		final();
	}

	/**
	* \brief 清楚数据
	*
	*/
	void final()
	{
		//		SAFE_DELETE(pSocket);
		if( pSocket != NULL )
		{
			if(pSocket->SafeDelete( false ))
				delete pSocket;
			pSocket = NULL;
		}		
		terminate = TM_no;
		mainloop = false;
	}

	/**
	* \brief 判断是否需要关闭连接
	* \return true or false
	*/
	bool isTerminate() const
	{
		return TM_no != terminate;
	}

	/**
	* \brief 需要主动断开客户端的连接
	* \param method 连接断开方式
	*/
	void Terminate(const TerminateMethod method)
	{
		terminate = method;
	}

	/**
	* \brief 如果是第一次进入主循环处理，需要先处理缓冲中的指令
	* \return 是否是第一次进入主处理循环
	*/
	bool checkFirstMainLoop()
	{
		if (mainloop)
			return false;
		else
		{
			mainloop = true;
			return true;
		}
	}

	/**
	* \brief 获取连接任务当前状态
	* \return 状态
	*/
	const ConnState getState() const
	{
		return state;
	}

	/**
	* \brief 设置连接任务下一个状态
	* \param state 需要设置的状态
	*/
	void setState(const ConnState state)
	{
		this->state = state;
	}

	/**
	* \brief 获得状态的字符串描述
	* \param state 状态
	* \return 返回状态的字符串描述
	*/
	const char *getStateString(const ConnState state)
	{
		const char *retval = NULL;

		switch(state)
		{
		case close:
			retval = "close";
			break;
		case sync:
			retval = "sync";
			break;
		case okay:
			retval = "okay";
			break;
		case recycle:
			retval = "recycle";
			break;
		}

		return retval;
	}


	/**
	* \brief 填充pollfd结构
	* \param pfd 待填充的结构
	* \param events 等待的事件参数
	*/
	void fillPollFD(struct mypollfd &pfd,short events)
	{
		if (pSocket)
			pSocket->fillPollFD(pfd,events);
	}

	/**
	* \brief 检测某种状态是否验证超时
	* \param state 待检测的状态
	* \param ct 当前系统时间
	* \param timeout 超时时间
	* \return 检测是否成功
	*/
	bool checkStateTimeout(const ConnState state,const zTime &ct,const time_t timeout) const
	{
		if (state == this->state)
			return (lifeTime.elapse(ct) >= timeout);
		else
			return false;
	}

	/**
	* \brief 连接验证函数
	*
	* 子类需要重载这个函数用于验证一个TCP连接，每个TCP连接必须通过验证才能进入下一步处理阶段，缺省使用一条空的指令作为验证指令
	* <pre>
	* int retcode = pSocket->recvToBuf_NoPoll();
	* if (retcode > 0)
	* {
	*     BYTE pstrCmd[zSocket::MAX_DATASIZE];
	*     int nCmdLen = pSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
	*     if (nCmdLen <= 0)
	*       //这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
	*       return 0;
	*     else
	*     {
	*       zSocket::t_NullCmd *pNullCmd = (zSocket::t_NullCmd *)pstrCmd;
	*       if (zSocket::null_opcode == pNullCmd->opcode)
	*       {
	*         std::cout << "客户端连接通过验证" << std::endl;
	*         return 1;
	*       }
	*       else
	*       {
	*         return -1;
	*       }
	*     }
	* }
	* else
	*     return retcode;
	* </pre>
	*
	* \return 验证是否成功，1表示成功，可以进入下一步操作，0，表示还要继续等待验证，-1表示等待验证失败，需要断开连接
	*/
	virtual int checkRebound()
	{
		return 1;
	}

	/**
	* \brief 需要删除这个TCP连接相关资源
	*/
	virtual void recycleConn() {};

	/**
	* \brief 一个连接任务验证等步骤完成以后，需要添加到全局容器中
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void addToContainer() {};

	/**
	* \brief 连接任务退出的时候，需要从全局容器中删除
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void removeFromContainer() {};

	virtual bool connect();

	void checkConn();
	bool sendCmd(const void *pstrCmd,const int nCmdLen);
	bool ListeningRecv(bool);
	bool ListeningSend();

	void getNextState();
	void resetState();
	/**
	* \brief 检查是否已经加入读事件
	*
	* \return 是否加入
	*/
	bool isFdsrAdd()
	{
		return fdsradd;
	}
	/**
	* \brief 设置加入读事件标志
	*
	* \return 是否加入
	*/
	bool fdsrAdd(bool set=true)
	{
		fdsradd=set;
		return fdsradd;
	}

	bool UseIocp()
	{
		return pSocket->m_bUseIocp;
	}

	int WaitRecv( bool bWait = false, int timeout = 0 )
	{
		return pSocket->WaitRecv( bWait, timeout );
	}

	int WaitSend( bool bWait = false, int timeout = 0 )
	{
		return pSocket->WaitSend( bWait, timeout );
	}

protected:

	zSocket *pSocket;                /**< 底层套接口 */
	volatile ConnState state;            /**< 连接状态 */

private:

	bool fdsradd;                  /**< 读事件添加标志 */
	const bool compress;              /**< 是否支持压缩 */
	const std::string ip;              /**< 服务器地址 */
	const WORD port;            /**< 服务器端口 */

	zTime lifeTime;                  /**< 生命期，记录每次状态改变的时间 */
	TerminateMethod terminate;            /**< 是否结束任务 */
	volatile bool mainloop;              /**< 是否已经进入主处理循环 */
	Timer _ten_min;

}; 

/**
* \brief 连接线程池类，封装了一个线程处理多个连接的线程池框架
*
*/
class zTCPClientTaskPool : private zNoncopyable
{

public:

	explicit zTCPClientTaskPool(const DWORD connPerThread,const int us=50000) : connPerThread(connPerThread)
	{       
		usleep_time=us;
		checkwaitThread = NULL; 
	} 
	~zTCPClientTaskPool();

	bool init();
	bool put(zTCPClientTask *task);
	void timeAction(const zTime &ct);

	void addCheckwait(zTCPClientTask *task);
	bool addMain(zTCPClientTask *task);
	void setUsleepTime(int time)
	{
		usleep_time = time;
	}

private:

	const DWORD connPerThread;
	zTCPClientTaskThread *newThread();

	/**
	* \brief 连接检测线程
	*
	*/
	zCheckconnectThread *checkconnectThread;;
	/**
	* \brief 连接等待返回信息的线程
	*
	*/
	zCheckwaitThread *checkwaitThread;;
	/**
	* \brief 所有成功连接处理的主线程
	*
	*/
	zThreadGroup taskThreads;

	/**
	* \brief 连接任务链表
	*
	*/
	//typedef std::list<zTCPClientTask *,__pool_alloc<zTCPClientTask *> > zTCPClientTaskContainer;
	typedef std::list<zTCPClientTask *> zTCPClientTaskContainer;


	/**
	* \brief 连接任务链表叠代器
	*
	*/
	typedef zTCPClientTaskContainer::iterator zTCPClientTask_IT;

	zMutex mlock;          /**< 互斥变量 */
	zTCPClientTaskContainer tasks;  /**< 任务列表 */

public:
	int usleep_time;                                        /**< 循环等待时间 */
};

typedef enum
{
	UNKNOWNSERVER  =  0, /** 未知服务器类型 */
	SUPERSERVER      =  1, /** 管理服务器 */
	LOGINSERVER     =  10, /** 登陆服务器 */
	RECORDSERVER  =  11, /** 档案服务器 */
	BILLSERVER      =  12, /** 计费服务器 */
	SESSIONSERVER  =  20, /** 会话服务器 */
	SCENESSERVER  =  21, /** 场景服务器 */
	GATEWAYSERVER  =  22, /** 网关服务器 */
	MINISERVER      =  23    /** 小游戏服务器 */
}ServerType;

template <typename T>
struct singleton_default
{
private:
	singleton_default();

public:
	typedef T object_type;

	static object_type & instance()
	{
		return obj;
	}

	static object_type obj;
};
template <typename T>
typename singleton_default<T>::object_type singleton_default<T>::obj;

//手动调用构造函数，不分配内存
template<class _T1> 
inline  void constructInPlace(_T1  *_Ptr)
{
	new (static_cast<void*>(_Ptr)) _T1();
}
/// 声明变长指令
#define BUFFER_CMD(cmd,name,len) char buffer##name[len];\
	cmd *name=(cmd *)buffer##name;constructInPlace(name);

typedef std::pair<DWORD,BYTE *> CmdPair;
template <int QueueSize=102400>
class MsgQueue
{
public:
	MsgQueue()
	{
		queueRead=0;
		queueWrite=0;
	}
	~MsgQueue()
	{
		clear();
	}
	typedef std::pair<volatile bool,CmdPair > CmdQueue;
	CmdPair *get()
	{
		CmdPair *ret=NULL;
		if (cmdQueue[queueRead].first)
		{
			ret=&cmdQueue[queueRead].second;
		}
		return ret;
	}
	void erase()
	{
		//SAFE_DELETE_VEC(cmdQueue[queueRead].second.second);
		__mt_alloc.deallocate(cmdQueue[queueRead].second.second,cmdQueue[queueRead].second.first);
		cmdQueue[queueRead].first=false;
		queueRead = (++queueRead)%QueueSize;
	}
	bool put(const void *pNullCmd,const DWORD cmdLen)
	{
		//BYTE *buf = new BYTE[cmdLen];
		BYTE *buf = (BYTE*)__mt_alloc.allocate(cmdLen);
		if (buf)
		{
			bcopy((void *)pNullCmd,buf,cmdLen,cmdLen);
			if (!putQueueToArray() && !cmdQueue[queueWrite].first)
			{
				cmdQueue[queueWrite].second.first = cmdLen;
				cmdQueue[queueWrite].second.second = buf;
				cmdQueue[queueWrite].first=true;
				queueWrite = (++queueWrite)%QueueSize;
				return true;
			}
			else
			{
				queueCmd.push(std::make_pair(cmdLen,buf));
			}
			return true;
		}
		return false;

	}
private:
	void clear()
	{
		while(putQueueToArray())
		{
			while(get())
			{
				erase();
			}
		}
		while(get())
		{
			erase();
		}
	}
	bool putQueueToArray()
	{
		bool isLeft=false;
		while(!queueCmd.empty())
		{
			if (!cmdQueue[queueWrite].first)
			{
				cmdQueue[queueWrite].second = queueCmd.front();;
				cmdQueue[queueWrite].first=true;
				queueWrite = (++queueWrite)%QueueSize;
				queueCmd.pop();
			}
			else
			{
				isLeft = true; 
				break;
			}
		}
		return isLeft;
	}
	__mt_alloc<BYTE> __mt_alloc;
	CmdQueue cmdQueue[QueueSize];
	std::queue<CmdPair> queueCmd;
	DWORD queueWrite;
	DWORD queueRead;
};

class MessageQueue
{
protected:
	virtual ~MessageQueue(){};
public:
	bool msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
	{
		return cmdQueue.put((void*)pNullCmd,cmdLen);
	}
	virtual bool cmdMsgParse(const Cmd::t_NullCmd *,const DWORD)=0;
	bool doCmd()
	{
		CmdPair *cmd = cmdQueue.get();
		while(cmd)
		{
			cmdMsgParse((const Cmd::t_NullCmd *)cmd->second,cmd->first);
			cmdQueue.erase();
			cmd = cmdQueue.get();
		}
		if (cmd)
		{
			cmdQueue.erase();
		}
		return true;
	}

private:
	MsgQueue<> cmdQueue;
};

/**
* \brief zUniqueID模板
* 本模板实现了唯一ID生成器，并保证线程安全。
* 可以用各种长度的无符号整数作为ID。
*/
template <class T>
class zUniqueID:private zNoncopyable
{
private:
	zMutex mutex;
	//std::list<T,__pool_alloc<T> > ids;
	std::list<T> ids;
	T maxID;
	T minID;
	T curMaxID;
	void init(T min,T max)
	{
		minID=min;
		maxID=max;
		curMaxID=minID;
	}

public:
	/**
	* \brief 默认构造函数 
	* 开始ID为1，最大有效ID为(T)-2,无效ID为(T)-1
	*/
	zUniqueID()
	{
		init(1,(T)-1);
	}

	/**
	* \brief 构造函数 
	* 用户自定义起始ID，最大有效ID为(T)-2,无效ID为(T)-1
	* \param startID 用户自定义的起始ID
	*/
	zUniqueID(T startID)
	{
		init(startID,(T)-1);
	}

	/**
	* \brief 构造函数 
	* 用户自定义起始ID，及最大无效ID,最大有效ID为最大无效ID-1
	* \param startID 用户自定义的起始ID
	* \param endID 用户自定义的最大无效ID
	*/
	zUniqueID(T startID,T endID)
	{
		init(startID,endID);
	}

	/**
	* \brief 析构函数 
	* 回收已分配的ID内存。
	*/
	~zUniqueID()
	{
		mutex.lock();
		ids.clear();
		mutex.unlock();
	}

	/**
	* \brief 得到最大无效ID 
	* \return 返回最大无效ID
	*/
	T invalid()
	{
		return maxID;
	}

	/**
	* \brief 测试这个ID是否被分配出去
	* \return 被分配出去返回true,无效ID和未分配ID返回false
	*/
	bool hasAssigned(T testid)
	{
		mutex.lock();
		if (testid<maxID && testid>=minID)
		{
			typename std::list<T,__pool_alloc<T> >::iterator iter = ids.begin();
			for(;iter != ids.end() ; iter ++)
			{
				if (*iter == testid)
				{
					mutex.unlock();
					return false;
				}
			}
			/*
			for(int i=0,n=ids.size() ;i<n;i++)
			{
			if (ids[i]==testid)
			{
			mutex.unlock();
			return false;
			}
			}
			// */
			mutex.unlock();
			return true;
		}
		mutex.unlock();
		return false;
	}

	/**
	* \brief 得到一个唯一ID 
	* \return 返回一个唯一ID，如果返回最大无效ID，比表示所有ID都已被用，无可用ID。
	*/
	T get()
	{
		T ret;
		mutex.lock();
		if (maxID>curMaxID)
		{
			ret=curMaxID;
			curMaxID++;
		}
		else
			ret=maxID;
		if (ret == maxID && !ids.empty())
		{
			ret=ids.back();
			ids.pop_back();
		}
		mutex.unlock();
		return ret;
	}

	/**
	* \brief 一次得到多个ID，这些ID都是相邻的,并且不回被放回去 
	* \param size 要分配的ID个数
	* \param count 实际分配ID的个数
	* \return 返回第一个ID，如果返回最大无效ID，比表示所有ID都已被用，无可用ID。
	*/
	T get(int size,int & count)
	{
		T ret;
		mutex.lock();
		if (maxID>curMaxID)
		{
			count=(maxID-curMaxID)>size?size:(maxID-curMaxID);
			ret=curMaxID;
			curMaxID+=count;
		}
		else
		{
			count=0;
			ret=maxID;
		}
		mutex.unlock();
		return ret;
	}

	/**
	* \brief 将ID放回ID池，以便下次使用。 
	* 
	* 放回的ID必须是由get函数得到的。并且不能保证放回的ID,没有被其他线程使用。
	* 所以用户要自己保证还在使用的ID不会被放回去。以免出现ID重复现象。
	* \param id 由get得到的ID.
	*/
	void put(T id)
	{
		mutex.lock();
		if (id<maxID && id>=minID)
		{
			bool hasID=false;
			typename std::list<T/*,__pool_alloc<T> */>::iterator iter = ids.begin();
			for(;iter != ids.end() ; iter ++)
			{
				if (*iter == id)
				{
					hasID=true;
					break;
				}
			}
			/*
			for(int i=0,n=ids.size() ;i<n;i++)
			{
			if (ids[i]==id)
			{
			hasID=true;
			break;
			}
			}
			// */
			if (!hasID) ids.push_front(id);
			//if (!hasID) ids.insert(ids.begin(),id);
			//if (!hasID) ids.push_back(id);
		}
		mutex.unlock();
	}
};

typedef zUniqueID<DWORD> zUniqueDWORDID;

/**
* \brief 配置文件解析器声明
*/
/**
* \brief zXMLParser定义
* 
* 主要提供了节点的浏览,和其属性的得到.
*/
class zXMLParser
{
public:
	zXMLParser();
	~zXMLParser();

	bool initFile(const std::string &xmlFile);
	bool initFile(const char *xmlFile);
	bool initStr(const std::string &xmlStr);
	bool initStr(const char *xmlStr);
	bool init();
	void final();
	std::string & dump(std::string & s,bool format=false);
	std::string & dump(xmlNodePtr dumpNode,std::string & s,bool head=true);
	xmlNodePtr getRootNode(const char *rootName);
	xmlNodePtr getChildNode(const xmlNodePtr parent,const char *childName);
	xmlNodePtr getNextNode(const xmlNodePtr node,const char *nextName);
	DWORD getChildNodeNum(const xmlNodePtr parent,const char *childName);

	xmlNodePtr newRootNode(const char *rootName);
	xmlNodePtr newChildNode(const xmlNodePtr parent,const char *childName,const char *content);
	bool newNodeProp(const xmlNodePtr node,const char *propName,const char *prop);

	bool getNodePropNum(const xmlNodePtr node,const char *propName,void *prop,int propSize);
	bool getNodePropStr(const xmlNodePtr node,const char *propName,void *prop,int propSize);
	bool getNodePropStr(const xmlNodePtr node,const char *propName,std::string &prop);
	bool getNodeContentNum(const xmlNodePtr node,void *content,int contentSize);
	bool getNodeContentStr(const xmlNodePtr node,void *content,int contentSize);
	bool getNodeContentStr(const xmlNodePtr node,std::string &content);
	bool getNodeContentStr(const xmlNodePtr node,std::string &content,bool head );
private:
	BYTE *charConv(BYTE *in,const char *fromEncoding,const char *toEncoding);
	xmlDocPtr doc;
};

/**
* \brief 配置文件解析器
*
* 此类必须继承使用。本类实现了全局参数的解析标记为\<global\>\</global\>
* 并把解析的参数保存在一个全局的参数容器global中。
*
* 如果用户有自己的配置,用户应该实现自己的参数解析。
*
*/
class zConfile
{
protected:
	/**
	* \brief xml解析器
	*/
	zXMLParser parser;
	/**
	* \brief 配置文件名称
	*
	*/
	std::string confile;

	bool globalParse(const xmlNodePtr node);
	bool parseNormal(const xmlNodePtr node);
	bool parseSuperServer(const xmlNodePtr node);
	virtual bool parseYour(const xmlNodePtr node)=0;

public:
	zConfile(const char *confile="config.xml");
	virtual ~zConfile();
	bool parse(const char *name);
};

/**
* \brief entry管理器定义文件
*/
/**
* \brief Entry基类
*/

#pragma pack(1)
struct zEntryC
{
	/**
	* \brief entry的数据ID，不同类型的Entry可能会重复,此时不能实现从ID查找entry
	*/
	DWORD id;
	/**
	* \brief entry的临时id,建议在实现EntryManager时，保证分配唯一
	*/
	DWORD tempid;
	/**
	* \brief entry的名字，不同类型的Entry可能会重复,此时不能实现从名字查找entry
	*/
	char name[MAX_NAMESIZE+1];
	zEntryC()
	{
		id=0;
		tempid=0;
		bzero(name,sizeof(name));
	};
};

/**
* \brief 回调函数类模板
*/
template <typename T,typename RTValue = bool>
struct zEntryCallback
{
	virtual RTValue exec(T *e)=0;
	virtual ~zEntryCallback(){};
};

struct zEntry:public zEntryC,private zNoncopyable
{
	virtual ~zEntry(){};
	zEntry():zEntryC()
	{
	};
};
#pragma pack()

/**
* \brief key值等值比较,目前支持 (DWORD,char *)，两种类型
*/
template <class keyT>
struct my_key_equal : public std::binary_function<keyT,keyT,bool>
{
	inline bool operator()(const keyT s1,const keyT s2) const;
};

/**
* \brief 模板偏特化
* 对字符串进行比较
*/
template<>
inline bool my_key_equal<const char *>::operator()(const char * s1,const char * s2) const
{
	return strcmp(s1,s2) == 0;
}

/**
* \brief 模板偏特化
* 对整数进行比较
*/
template<>
inline bool my_key_equal<DWORD>::operator()(const DWORD s1,const DWORD s2) const
{
	return s1  == s2;
}

/**
* \brief 有限桶Hash管理模板,非线程安全
*
* 目前支持两种key类型(DWORD,char *),value类型不作限制,但此类型要可copy的。
* \param keyT key类型(DWORD,char *)
* \param valueT value类型
*/
template <class keyT,class valueT>
class LimitHash:private zNoncopyable
{
protected:

	/**
	* \brief hash_map容器
	*/
	//typedef hash_map<keyT,valueT,hash<keyT>,my_key_equal<keyT> > hashmap;
	typedef hash_map<keyT,valueT> hashmap;
	typedef typename hashmap::iterator iter;
	typedef typename hashmap::const_iterator const_iter;
	hashmap ets;

	/**
	* \brief 插入数据，如果原来存在相同key值的数据，原来数据将会被替换
	* \param key key值
	* \param value 要插入的数据
	* \return 成功返回true，否则返回false
	*/
	inline bool insert(const keyT &key,valueT &value)
	{
		ets[key]=value;
		return true;
	}

	/**
	* \brief 根据key值查找并得到数据
	* \param key 要寻找的key值
	* \param value 返回结果将放入此处,未找到将不会改变此值
	* \return 查找到返回true，未找到返回false
	*/
	inline bool find(const keyT &key,valueT &value) const
	{
		const_iter it = ets.find(key);
		if (it != ets.end())
		{
			value = it->second;
			return true;
		}
		else
			return false;
	}

	/**
	* \brief 查找并得到一个数据
	* \param value 返回结果将放入此处,未找到将不会改变此值
	* \return 查找到返回true，未找到返回false
	*/
	inline bool findOne(valueT &value) const
	{
		if (!ets.empty())
		{
			value=ets.begin()->second;
			return true;
		}
		return false;
	}

	/**
	* \brief 构造函数
	*
	*/
	LimitHash()
	{
	}

	/**
	* \brief 析构函数,清除所有数据
	*/
	~LimitHash()
	{
		clear();
	}

	/**
	* \brief 移除数据
	* \param key 要移除的key值
	*/
	inline void remove(const keyT &key)
	{
		ets.erase(key);
	}

	/**
	* \brief 清除所有数据
	*/
	inline void clear()
	{
		ets.clear();
	}

	/**
	* \brief 统计数据个数
	*/
	inline DWORD size() const
	{
		return ets.size();
	}

	/**
	* \brief 判断容器是否为空
	*/
	inline bool empty() const
	{
		return ets.empty();
	}
};

/**
* \brief 有限桶MultiHash管理模板,非线程安全
*
* 目前支持两种key类型(DWORD,char *),value类型不作限制,但此类型要可copy的。
* \param keyT key类型(DWORD,char *)
* \param valueT value类型
*/
template <class keyT,class valueT>
class MultiHash:private zNoncopyable
{
protected:

	/**
	* \brief hash_multimap容器
	*/
	//typedef hash_multimap<keyT,valueT,hash<keyT>,my_key_equal<keyT> > hashmap;
	typedef hash_multimap<keyT,valueT> hashmap;
	typedef typename hashmap::iterator iter;
	typedef typename hashmap::const_iterator const_iter;
	hashmap ets;

	/**
	* \brief 插入数据，如果原来存在相同key值的数据，原来数据将会被替换
	* \param key key值
	* \param value 要插入的数据
	* \return 成功返回true，否则返回false
	*/
	inline bool insert(const keyT &key,valueT &value)
	{
		//if(ets.find(key) == ets.end())
		ets.insert(std::pair<keyT,valueT>(key,value));
		return true;
	}

	/**
	* \brief 构造函数
	*
	*/
	MultiHash()
	{
	}

	/**
	* \brief 析构函数,清除所有数据
	*/
	~MultiHash()
	{
		clear();
	}

	/**
	* \brief 清除所有数据
	*/
	inline void clear()
	{
		ets.clear();
	}

	/**
	* \brief 统计数据个数
	*/
	inline DWORD size() const
	{
		return ets.size();
	}

	/**
	* \brief 判断容器是否为空
	*/
	inline bool empty() const
	{
		return ets.empty();
	}
};

/**
* \brief Entry以临时ID为key值的指针容器，需要继承使用
*/
class zEntryTempID:public LimitHash<DWORD,zEntry *>
{
protected:

	zEntryTempID() {}
	virtual ~zEntryTempID() {}

	/**
	* \brief 将Entry加入容器中,tempid重复添加失败
	* \param e 要加入的Entry
	* \return 成功返回true,否则返回false
	*/
	inline bool push(zEntry * e)
	{
		if (e!=NULL && getUniqeID(e->tempid))
		{
			zEntry *temp;
			if (!find(e->tempid,temp))
			{
				if (insert(e->tempid,e))
					return true;
			}
			putUniqeID(e->tempid);
		}
		return false;
	}

	/**
	* \brief 移除Entry
	* \param e 要移除的Entry
	*/
	inline void remove(zEntry * e)
	{
		if (e!=NULL)
		{
			putUniqeID(e->tempid);
			LimitHash<DWORD,zEntry *>::remove(e->tempid);
		}
	}

	/**
	* \brief 通过临时ID得到Entry
	* \param tempid 要得到Entry的临时ID
	* \return 返回Entry指针,未找到返回NULL
	*/
	inline zEntry * getEntryByTempID(const DWORD tempid) const
	{
		zEntry *ret=NULL;
		LimitHash<DWORD,zEntry *>::find(tempid,ret);
		return ret;
	}

	/**
	* \brief 得到一个临时ID
	* \param tempid 存放要得到的临时ID
	* \return 得到返回true,否则返回false
	*/
	virtual bool getUniqeID(DWORD &tempid) =0;
	/**
	* \brief 放回一个临时ID
	* \param tempid 要放回的临时ID
	*/
	virtual void putUniqeID(const DWORD &tempid) =0;
};

/**
* \brief Entry以ID为key值的指针容器，需要继承使用
*/
class zEntryID:public LimitHash<DWORD,zEntry *>
{
protected:
	/**
	* \brief 将Entry加入容器中
	* \param e 要加入的Entry
	* \return 成功返回true,否则返回false
	*/
	inline bool push(zEntry * &e)
	{
		zEntry *temp;
		if (!find(e->id,temp))
			return insert(e->id,e);
		else
			return false;
	}

	/**
	* \brief 移除Entry
	* \param e 要移除的Entry
	*/
	inline void remove(zEntry * e)
	{
		if (e!=NULL)
		{
			LimitHash<DWORD,zEntry *>::remove(e->id);
		}
	}

	/**
	* \brief 通过ID得到Entry
	* \param id 要得到Entry的ID
	* \return 返回Entry指针,未找到返回NULL
	*/
	inline zEntry * getEntryByID(const DWORD id) const
	{
		zEntry *ret=NULL;
		LimitHash<DWORD,zEntry *>::find(id,ret);
		return ret;
	}
};

/**
* \brief Entry以名字为key值的指针容器，需要继承使用
*/
class zEntryName:public LimitHash<std::string,zEntry *>
{
protected:
	/**
	* \brief 将Entry加入容器中,如果容器中有相同key值的添加失败
	* \param e 要加入的Entry
	* \return 成功返回true,否则返回false
	*/
	inline bool push(zEntry * &e)
	{
		zEntry *temp;
		if (!find(std::string(e->name),temp))
			return insert(std::string(e->name),e);
		else
			return false;
	}

	/**
	* \brief 移除Entry
	* \param e 要移除的Entry
	*/
	inline void remove(zEntry * e)
	{
		if (e!=NULL)
		{
			LimitHash<std::string,zEntry *>::remove(std::string(e->name));
		}
	}

	/**
	* \brief 通过名字得到Entry
	* \param name 要得到Entry的名字
	* \return 返回Entry指针,未找到返回NULL
	*/
	inline zEntry * getEntryByName( const char * name) const
	{
		zEntry *ret=NULL;
		LimitHash<std::string,zEntry *>::find(std::string(name),ret);
		return ret;
	}

	/**
	* \brief 通过名字得到Entry
	* \param name 要得到Entry的名字
	* \return 返回Entry指针,未找到返回NULL
	*/
	inline zEntry * getEntryByName(const std::string  &name) const
	{
		return getEntryByName(name.c_str());
	}
};

/**
* \brief Entry以名字为key值的指针容器，需要继承使用
*/
class zMultiEntryName:public MultiHash</*const char **/std::string,zEntry *>
{
protected:
	/**
	* \brief 将Entry加入容器中,如果容器中有相同key值的添加失败
	* \param e 要加入的Entry
	* \return 成功返回true,否则返回false
	*/
	inline bool push(zEntry * &e)
	{
		return insert(std::string(e->name),e);
	}

	/**
	* \brief 将Entry从容器中移除
	* \param e 需要移除的Entry
	*/
	inline void remove(zEntry * &e)
	{
		pair<iter,iter> its = ets.equal_range(std::string(e->name));
		for(iter it = its.first; it != its.second; it++)
		{
			if (it->second == e)
			{
				ets.erase(it);
				return;
			}
		}
	}

	/**
	* \brief 根据key值查找并得到数据
	* \param name 要寻找的name值
	* \param e 返回结果将放入此处,未找到将不会改变此值
	* \param r 如果有多项匹配，是否随机选择
	* \return 查找到返回true，未找到返回false
	*/
	inline bool find(const char * &name,zEntry * &e,const bool r=false) const
	{
		int rd = ets.count(std::string(name));
		if (rd > 0)
		{
			int mrd = 0,j = 0;
			if (r)
				randBetween(0,rd - 1);
			pair<const_iter,const_iter> its = ets.equal_range(std::string(name));
			for(const_iter it = its.first; it != its.second && j < rd; it++,j++)
			{
				if (mrd == j)
				{
					e = it->second;
					return true;
				}
			}
		}
		return false;
	}

};

template<int i>
class zEntryNone
{
protected:
	inline bool push(zEntry * &e) { return true; }
	inline void remove(zEntry * &e) { }
	inline void clear(){}
};

/**
* \brief Entry处理接口,由<code>zEntryManager::execEveryEntry</code>使用
*/
template <class YourEntry>
struct execEntry
{
	virtual bool exec(YourEntry *entry) =0;
	virtual ~execEntry(){}
};

/**
* \brief Entry删除条件接口,由<code>zEntryManager::removeEntry_if</code>使用
*/
template <class YourEntry>
struct removeEntry_Pred
{
	/**
	* \brief 被删除的entry存储在这里
	*/
	std::vector<YourEntry *> removed;
	/**
	* \brief 测试是否要删除的entry,需要实现
	* \param 要被测试的entry
	*/
	virtual bool isIt(YourEntry *entry) =0;
	/**
	* \brief 析构函数
	*/
	virtual ~removeEntry_Pred(){}
};

/**
* \brief Entry管理器接口,用户应该根据不同使用情况继承它
*/

template<typename e1,typename e2=zEntryNone<1>,typename e3=zEntryNone<2> >
class zEntryManager:protected e1,protected e2,protected e3
{
protected:

	//unsigned long count;

	/**
	* \brief 添加Entry,对于重复索引的Entry添加失败
	* \param e 被添加的 Entry指针
	* \return 成功返回true，否则返回false 
	*/
	inline bool addEntry(zEntry * e)
	{

		if(NULL == e)
			return false;
		//++count;
		// unsigned long t = count;

		//if( 765 == count)
		//{
		//Zebra::logger->error("%u\n",count);
		//fprintf(stderr,"%u\n",count);
		//  }
		if (e1::push(e))
		{ 
			//zEntry *ee = e1::getEntryByName(e->name); 
			if (e2::push(e))
			{ 

				if (e3::push(e))
					return true;
				else
				{
					e2::remove(e);
					e1::remove(e);
				}
			}
			else
				e1::remove(e);
		}
		return false;
	}

	/**
	* \brief 删除Entry
	* \param e 被删除的Entry指针
	*/
	inline void removeEntry(zEntry * e)
	{
		e1::remove(e);
		e2::remove(e);
		e3::remove(e);
	}


	zEntryManager() { }
	/**
	* \brief 虚析构函数
	*/
	~zEntryManager() { };

	/**
	* \brief 统计管理器中Entry的个数
	* \return 返回Entry个数
	*/
	inline int size() const
	{
		return e1::size();
	}

	/**
	* \brief 判断容器是否为空
	*/
	inline bool empty() const
	{
		return e1::empty();
	}

	/**
	* \brief 清除所有Entry
	*/
	inline void clear()
	{
		e1::clear();
		e2::clear();
		e3::clear();
	}

	/**
	* \brief 对每个Entry进行处理
	* 当处理某个Entry返回false时立即打断处理返回
	* \param eee 处理接口
	* \return 如果全部执行完毕返回true,否则返回false
	*/
	template <class YourEntry>
	inline bool execEveryEntry(execEntry<YourEntry> &eee)
	{
		typedef typename e1::iter my_iter;
		for(my_iter it=e1::ets.begin();it!=e1::ets.end();it++)
		{
			if (!eee.exec((YourEntry *)it->second))
				return false;
		}
		return true;
	}

	/**
	* \brief 删除满足条件的Entry
	* \param pred 测试条件接口
	*/
	template <class YourEntry>
	inline void removeEntry_if (removeEntry_Pred<YourEntry> &pred)
	{
		typedef typename e1::iter my_iter;
		my_iter it=e1::ets.begin();
		while(it!=e1::ets.end())
		{
			if (pred.isIt((YourEntry *)it->second))
			{
				pred.removed.push_back((YourEntry *)it->second);
			}
			it++;
		}

		for(DWORD i=0;i<pred.removed.size();i++)
		{
			removeEntry(pred.removed[i]);
		}
	}
};

/**
* \brief 场景上物件定义
*/
#pragma pack(1)
/**
* \brief 用于偏移计算的坐标值
*/
struct zAdjust
{
	int x;    /**< 横坐标*/
	int y;    /**< 纵坐标*/
};
/**
* \brief 场景坐标
*/
struct zPos
{
	DWORD x;    /**< 横坐标*/
	DWORD y;    /**< 纵坐标*/
	/**
	* \brief 构造函数
	*
	*/
	zPos()
	{
		x = 0;
		y = 0;
	}
	/**
	* \brief 构造函数
	*
	*/
	zPos(const DWORD x,const DWORD y)
	{
		this->x = x;
		this->y = y;
	}
	/**
	* \brief 拷贝构造函数
	*
	*/
	zPos(const zPos &pos)
	{
		x = pos.x;
		y = pos.y;
	}
	/**
	* \brief 赋值操作符号
	*
	*/
	zPos & operator= (const zPos &pos)
	{
		x = pos.x;
		y = pos.y;
		return *this;
	}
	/**
	* \brief 重载+运算符号
	*
	*/
	const zPos & operator+ (const zPos &pos)
	{
		x += pos.x;
		y += pos.y;
		return *this;
	}
	/**
	* \brief 重载+运算符号
	* 对坐标进行修正
	*/
	const zPos & operator+ (const zAdjust &adjust)
	{
		x += adjust.x;
		y += adjust.y;
		return *this;
	}
	/**
	* \brief 重载+=运算符号
	*
	*/
	const zPos & operator+= (const zPos &pos)
	{
		x += pos.x;
		y += pos.y;
		return *this;
	}
	/**
	* \brief 重载+=运算符号
	* 对坐标进行修正
	*/
	const zPos & operator+= (const zAdjust &adjust)
	{
		x += adjust.x;
		y += adjust.y;
		return *this;
	}
	/**
	* \brief 重载-运算符号
	*
	*/
	const zPos & operator- (const zPos &pos)
	{
		x -= pos.x;
		y -= pos.y;
		return *this;
	}
	/**
	* \brief 重载-运算符号
	* 对坐标进行修正
	*/
	const zPos & operator- (const zAdjust &adjust)
	{
		x -= adjust.x;
		y -= adjust.y;
		return *this;
	}
	/**
	* \brief 重载-=运算符号
	*
	*/
	const zPos & operator-= (const zPos &pos)
	{
		x -= pos.x;
		y -= pos.y;
		return *this;
	}
	/**
	* \brief 重载-=运算符号
	* 对坐标进行修正
	*/
	const zPos & operator-= (const zAdjust &adjust)
	{
		x -= adjust.x;
		y -= adjust.y;
		return *this;
	}
	/**
	* \brief 重载==逻辑运算符号
	*
	*/
	const bool operator== (const zPos &pos) const
	{
		return (x == pos.x && y == pos.y);
	}
	/**
	* \brief 重载>逻辑运算符号
	*
	*/
	const bool operator> (const zPos &pos) const
	{
		return (x > pos.x && y > pos.y);
	}
	/**
	* \brief 重载>=逻辑运算符号
	*
	*/
	const bool operator>= (const zPos &pos) const
	{
		return (x >= pos.x && y >= pos.y);
	}
	/**
	* \brief 重载<逻辑运算符号
	*
	*/
	const bool operator< (const zPos &pos) const
	{
		return (x < pos.x && y < pos.y);
	}
	/**
	* \brief 重载<=逻辑运算符号
	*
	*/
	const bool operator<= (const zPos &pos) const
	{
		return (x <= pos.x && y <= pos.y);
	}
	/**
	* \brief 以自身为中心点，获取到另外一个坐标的方向
	* \param pos 另外一个坐标点
	* \return 方向
	*/
	const int getDirect(const zPos &pos) const
	{
		using namespace Cmd;
		if (x == pos.x && y > pos.y)
		{
			return _DIR_UP;
		}
		else if (x < pos.x && y > pos.y)
		{
			return _DIR_UPRIGHT;
		}
		else if (x < pos.x && y == pos.y)
		{
			return _DIR_RIGHT;
		}
		else if (x < pos.x && y < pos.y)
		{
			return _DIR_RIGHTDOWN;
		}
		else if (x == pos.x && y < pos.y)
		{
			return _DIR_DOWN;
		}
		else if (x > pos.x && y < pos.y)
		{
			return _DIR_DOWNLEFT;
		}
		else if (x > pos.x && y == pos.y)
		{
			return _DIR_LEFT;
		}
		else if (x > pos.x && y > pos.y)
		{
			return _DIR_LEFTUP;
		}

		return _DIR_WRONG;
	}
};
/**
* \brief 半屏坐标
*
*/
const zPos zPosHalfScreen(SCREEN_WIDTH / 2,SCREEN_HEIGHT / 2);
#pragma pack()

/**
* \brief 场景屏坐标
*/
typedef DWORD zPosI;

/**
* \brief 存放屏编号的向量
*
*/
typedef std::vector<zPosI> zPosIVector;
typedef std::vector<zPos> zPosVector;

typedef std::list<WORD> SceneEntryStateList;

class zSceneEntryIndex;
/**
* \brief 场景上物件，比如人物,NPC,建筑，地上物品等
*
* 作用有两个
*
* 1.建立屏索引
* 2.定义阻挡
*/
struct zSceneEntry:public zEntry
{
	friend class zSceneEntryIndex;
public:

	unsigned short dupIndex;
	/**
	* \brief 物件类型
	*/
	enum SceneEntryType
	{
		SceneEntry_Player,/**< 玩家角色*/
		SceneEntry_NPC,  /**< NPC*/
		SceneEntry_Build,/**< 建筑*/
		SceneEntry_Object,/**< 地上物品*/
		SceneEntry_Pet,  /**< 宠物*/
		SceneEntry_MAX
	};
	/**
	* \brief 物件状态
	*
	*/
	enum SceneEntryState
	{
		SceneEntry_Normal,  /**< 普通状态 */
		SceneEntry_Death,  /**< 死亡状态 */
		SceneEntry_Hide      /**< 隐藏状态 */
	};
	/**
	* \brief 坐标转化
	* \param screenWH 地图宽和高
	* \param pos 源坐标
	* \param posi 目的坐标
	*/
	static void zPos2zPosI(const zPos &screenWH,const zPos &pos,zPosI &posi)
	{
		posi=((screenWH.x+SCREEN_WIDTH-1)/SCREEN_WIDTH) * (pos.y/SCREEN_HEIGHT) + (pos.x/SCREEN_WIDTH);
	}
protected:
	/**
	* \brief 构造函数
	*/
	zSceneEntry(SceneEntryType type,const SceneEntryState state = SceneEntry_Normal):sceneentrytype(type),sceneentrystate(state)
	{
		bzero(byState,sizeof(byState));
		dir = Cmd::_DIR_DOWN;
		inserted=false;
		dupIndex = 0;
	}

	/**
	* \brief 坐标
	*/
	zPos pos;
	/**
	* \brief 屏坐标
	*/
	zPosI posi;
	/**
	* \brief 方向
	*
	*/
	BYTE dir;

	zPos lastPos1;
	zPos lastPos2;

private:
	/**
	* \brief 物件类型
	*/
	const SceneEntryType sceneentrytype;
	/**
	* \brief 物件状态
	*
	*/
	SceneEntryState sceneentrystate;
	/**
	* \brief 物件是否在场景上
	*/
	bool inserted;

	/**
	* \brief 设置物件坐标
	* \param screenWH 场景的宽高
	* \param newPos 物件的新坐标
	* \return 坐标超出场景宽高返回false,否则返回true
	*/
	bool setPos(const zPos &screenWH,const zPos &newPos)
	{
		if (screenWH.x>newPos.x && screenWH.y>newPos.y)
		{
			pos=newPos;
			zPos2zPosI(screenWH,newPos,posi);
			return true;
		}
		else
			return false;
	}

private:
	/**
	* \brief 物件状态，与魔法等相关的
	* 这种状态是外观可以表现的，带上某种状态客户端就可以以一种方式来表现
	* 详细的状态参见Command.h中
	*/
	BYTE byState[(Cmd::MAX_STATE + 7) / 8];
protected:
	SceneEntryStateList stateList;
public:
	/**
	* \brief 填充物件状态
	* \param state 填充位置
	* \return 状态个数
	*/
	inline BYTE full_UState(WORD *state)
	{
		BYTE ret = stateList.size();
		SceneEntryStateList::iterator iter = stateList.begin();
		for(int i=0 ; i < ret ; i ++)
		{
			state[i] = *iter;
			iter ++;
		}
		return ret;
	}
	/**
	* \brief 填充物件所有状态
	* \param state 填充位置
	*/
	inline void full_all_UState(void *state,DWORD maxSize )
	{
		bcopy(byState,state,sizeof(byState),maxSize);
	}

	/**
	* \brief 得到物件坐标
	* \return 物件坐标
	*/
	inline const zPos &getPos() const
	{
		return pos;
	}

	/**
	* \brief 得到物件刚才的坐标
	* \return 物件坐标
	*/
	inline const zPos &getOldPos1() const
	{
		return lastPos1;
	}

	/**
	* \brief 得到物件刚才的坐标
	* \return 物件坐标
	*/
	inline const zPos &getOldPos2() const
	{
		return lastPos2;
	}

	/**
	* \brief 得到物件屏坐标
	* \return 物件屏坐标
	*/
	inline const zPosI &getPosI() const
	{ 
		return posi;
	}
	/**
	* \brief 测试物件是否在场景中
	* \return 物件在场景中返回true,否则返回false
	*/
	inline bool hasInScene() const
	{ 
		return inserted;
	}

	/**
	* \brief 得到物件类型
	* \return 物件类型
	*/
	inline const SceneEntryType & getType() const
	{
		return sceneentrytype;
	}

	/**
	* \brief 获取物件状态
	* \return 状态
	*/
	inline const SceneEntryState & getState() const
	{
		return sceneentrystate;
	}

	/**
	* \brief 设置物件状态
	* \param state 需要设置的状态
	*/
	void setState(const SceneEntryState & state)
	{
		sceneentrystate = state;
	}

	/**
	* \brief 获取方向
	* \return 方向
	*/
	inline const BYTE getDir() const
	{
		return dir % 8;
	}

	/**
	* \brief 设置方向
	* \param dir 方向
	*/
	void setDir(const BYTE dir)
	{
		this->dir = dir % 8;
	}

	/**
	* \brief 检查某种状态是否设置
	* \param state 待检查的状态
	* \return 这种状态是否已经设置
	*/
	inline bool issetUState(const int state) const
	{
		return Cmd::isset_state(byState,state);
	}

	/**
	* \brief 设置某种状态
	* \param state 待设置的状态
	* \return 如果已经设置该状态返回false,否则返回true
	*/
	inline bool setUState(const int state)
	{
		if (!issetUState(state))
		{
			stateList.push_back(state);
			Cmd::set_state(byState,state);
			return true;
		}
		return false;
	}

	/**
	* \brief 清除某种状态
	* \param state 待清除的状态
	* \return 如果已经设置该状态返回true,否则返回false
	*/
	inline bool clearUState(const int state)
	{
		Cmd::clear_state(byState,state);
		SceneEntryStateList::iterator iter = stateList.begin();
		for( ; iter != stateList.end() ; ++iter)
		{
			if (*iter == state)
			{
				stateList.erase(iter);
				return true;
			}
		}
		return false;
	}
};

/**
* \brief 场景管理器定义
*/
enum enumSceneRunningState{
	SCENE_RUNNINGSTATE_NORMAL,//正常运行
	SCENE_RUNNINGSTATE_UNLOAD,//正在卸载
	SCENE_RUNNINGSTATE_REMOVE,//正在卸载
};
/**
* \brief 场景基本信息定义
*/
struct zScene:public zEntry
{
private:
	DWORD running_state;
public:
	zScene():running_state(SCENE_RUNNINGSTATE_NORMAL){}
	DWORD getRunningState() const
	{
		return running_state;
	}
	DWORD setRunningState(DWORD set)
	{
		running_state = set;
		return running_state;
	}
};

/**
* \brief 场景管理器
*
* 以名字和临时ID索引,没有ID索引，因为场景可能重复
*/
class zSceneManager:public zEntryManager<zEntryID,zEntryTempID,zEntryName>
{


protected:
	/**
	* \brief 访问管理器的互斥锁
	*/
	zRWLock rwlock;

	zScene * getSceneByName( const char * name)
	{
		rwlock.rdlock();
		zScene *ret =(zScene *)getEntryByName(name);
		rwlock.unlock();
		return ret;
	}



	zScene * getSceneByID(DWORD id)
	{
		rwlock.rdlock();
		zScene *ret =(zScene *)getEntryByID(id);
		rwlock.unlock();
		return ret;
	}

	zScene * getSceneByTempID( DWORD tempid)
	{
		rwlock.rdlock();
		zScene *ret =(zScene *)getEntryByTempID(tempid);
		rwlock.unlock();
		return ret;
	}

	template <class YourSceneEntry>
	bool execEveryScene(execEntry<YourSceneEntry> &exec)
	{
		rwlock.rdlock();
		bool ret=execEveryEntry<>(exec);
		rwlock.unlock();
		return ret;
	}

	/**
	* \brief 移出符合条件的角色
	* \param pred 条件断言
	*/
	template <class YourSceneEntry>
	void removeScene_if(removeEntry_Pred<YourSceneEntry> &pred)
	{
		rwlock.wrlock();
		removeEntry_if<>(pred);
		rwlock.unlock();
	}

public:
	/**
	* \brief 构造函数
	*/
	zSceneManager()
	{
	}

	/**
	* \brief 析构函数
	*/
	virtual ~zSceneManager()
	{
		clear();
	}

};

/**
* \brief 游戏基本数据管理器 声明
*/
#pragma pack(1)
//------------------------------------
// ObjectBase
//------------------------------------
struct ObjectBase{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}

	DWORD  dwField0;    // 编号
	char  strField1[64];    // 名称
	DWORD  dwField2;    // 最大数量
	DWORD  dwField3;    // 类型
	DWORD  dwField4;    // 职业限定
	DWORD  dwField5;    // 需要等级
	DWORD  dwField6;    // 道具等级
	char  strField7[256];    // 孔
	DWORD  dwField8;    // 配合物品
	char  strField9[256];    // 药品作用
	DWORD  dwField10;    // 最大生命值
	DWORD  dwField11;    // 最大法术值
	DWORD  dwField12;    // 最大体力值
	DWORD  dwField13;    // 最小物攻
	DWORD  dwField14;    // 最大物攻
	DWORD  dwField15;    // 最小魔攻
	DWORD  dwField16;    // 最大魔攻
	DWORD  dwField17;    // 物防
	DWORD  dwField18;    // 魔防
	DWORD  dwField19;    // 伤害加成

	DWORD  dwField20;    // 攻击速度
	DWORD  dwField21;    // 移动速度
	DWORD  dwField22;    // 命中率
	DWORD  dwField23;    // 躲避率
	DWORD  dwField24;    // 改造
	DWORD  dwField25;    // 合成等级
	DWORD  dwField26;    // 打造
	char  strField27[32];    // 需要技能
	char  strField28[1024];    // 需要原料
	DWORD  dwField29;    // 装备位置
	DWORD  dwField30;    // 耐久度
	DWORD  dwField31;    // 价格
	DWORD  dwField32;    // 颜色
	DWORD  dwField33;    // 格子宽
	DWORD  dwField34;    // 格子高
	DWORD  dwField35;    // 金子
	DWORD  dwField36;    // 合成单价
	DWORD  dwField37;    // 重击
	DWORD  dwField38;    // 神圣概率
	char  strField39[256];    // 神圣标识 

	//sky 新增属性
	DWORD  dwField40;    // 力量
	DWORD  dwField41;    // 智力
	DWORD  dwField42;    // 敏捷
	DWORD  dwField43;    // 精神
	DWORD  dwField44;    // 耐力
	DWORD  dwField45;    // 物理免伤
	DWORD  dwField46;    // 魔法免伤
};//导出 ObjectBase 成功，共 940 条记录

/**
* \brief 物品基本表
*/
struct zObjectB:public zEntry
{
	DWORD maxnum;        // 最大数量
	BYTE kind;          // 类型
	BYTE job;          // sky 职业限制
	WORD level;          // 道具等级
	std::vector<DWORD> hole;  //孔

	WORD needobject;      // 配合物品
	struct leechdom_t {
		WORD id; //功能标识
		WORD effect; //效果
		WORD time; //时间
		leechdom_t(const std::string& _id="",const std::string& _effect="",const std::string& _time="") 
			: id(atoi(_id.c_str())),effect(atoi(_effect.c_str())),time(atoi(_time.c_str()))
		{ }
	} leechdom ;         // 药品作用

	WORD needlevel;        // 需要等级

	DWORD maxhp;          // 最大生命值
	DWORD maxmp;          // 最大法术值
	DWORD maxsp;          // 最大体力值

	DWORD pdamage;        // 最小攻击力
	DWORD maxpdamage;      // 最大攻击力
	DWORD mdamage;        // 最小法术攻击力
	DWORD maxmdamage;      // 最大法术攻击力

	DWORD pdefence;        // 物防
	DWORD mdefence;        // 魔防

	WORD damagebonus;      //伤害加成

	WORD akspeed;        // 攻击速度
	WORD mvspeed;        // 移动速度
	WORD atrating;        // 命中率
	WORD akdodge;        // 躲避率

	DWORD color;        // 颜色  

	//struct socket
	//{
	//  WORD odds;
	//  BYTE min;
	//  BYTE max;
	//  socket(const std::string& odds_,const std::string& number_)
	//  {
	// odds=atoi(odds_.c_str());
	// min=0;
	// max=0;
	//    std::string::size_type pos = 0;
	//    if  ( (pos = number_.find("-")) != std::string::npos )
	//    {
	//      
	//      min = atoi(number_.substr(0,pos).c_str());
	//      max = atoi(number_.substr(pos+strlen("-")).c_str());
	//      //if (odds) Zebra::logger->debug("odds:%d\tmin:%d\tmax:%d",odds,min,max);
	//    }
	//  }
	//} hole;            //孔

	BYTE recast;        // 改造

	BYTE recastlevel;       // 合成等级
	WORD recastcost;      // 合成单价


	WORD make;          // 打造
	struct skills 
	{
		WORD id;
		BYTE level;
		skills(const std::string& id_="0",const std::string& level_="0") : id(atoi(id_.c_str())),level(atoi(level_.c_str()))
		{ }
	};
	skills need_skill;      // 需要技能

	struct material
	{
		WORD gold;
		struct  stuff
		{
			WORD id;
			WORD number;
			BYTE level;
			stuff(const std::string& id_,const std::string& level_,const std::string& number_) : id(atoi(id_.c_str())),number(atoi(number_.c_str())),level(atoi(level_.c_str()))
			{ }  
		};
		std::vector<stuff> stuffs;
		typedef std::vector<stuff>::iterator stuffs_iterator;
	};
	material need_material;    // 需要原料

	BYTE setpos;        // 装备位置
	WORD durability;      // 耐久度
	DWORD price;        // 价格

	BYTE width;          // 格子宽
	BYTE height;        // 格子高
	union
	{
		DWORD cardpoint;      // 金子 (已经无用)
		DWORD cointype;        // 货币类型
	};
	WORD bang;          //重击
	DWORD holyrating;      //神圣概率
	std::vector<DWORD> holys;     //神圣标识

	// sky 新增基本属性
	WORD str;				 //力量
	WORD inte;			 //智力
	WORD dex;				 //敏捷
	WORD spi;				 //精神
	WORD con;				 //耐力

	WORD atkhpp;  //魔法免伤
	WORD mtkhpp;  //魔法免伤

	int  nSuitData;

	void fill(ObjectBase &data)
	{
		nSuitData = -1;

		id = data.dwField0;
		strncpy(name,data.strField1,MAX_NAMESIZE);

		maxnum = data.dwField2;  
		kind = data.dwField3;  
		job = data.dwField4;  
		needlevel =  data.dwField5;  
		level =  data.dwField6;  

		init_identifier(hole,data.strField7);

		needobject = data.dwField8;  
		init_leechdom(data.strField9);

		maxhp = data.dwField10;  
		maxmp = data.dwField11;
		maxsp =  data.dwField12;

		pdamage = data.dwField13;
		maxpdamage = data.dwField14;
		mdamage = data.dwField15;
		maxmdamage = data.dwField16;
		pdefence = data.dwField17;
		mdefence = data.dwField18;
		damagebonus = data.dwField19;

		akspeed = data.dwField20;
		mvspeed = data.dwField21;
		atrating = data.dwField22;
		akdodge = data.dwField23;

		recast = data.dwField24;
		recastlevel = data.dwField25;

		make = data.dwField26;

		init_need_skills(data.strField27);
		init_need_material(data.strField28);

		setpos = data.dwField29;
		durability = data.dwField30;
		price =  data.dwField31;

		//sky  新游戏里已经不需要这个属性拉
		/*bluerating = data.dwField34;
		goldrating = data.dwField35;*/

		color = data.dwField32;
		width =  data.dwField33;
		height = data.dwField34;
		cardpoint = data.dwField35;
		recastcost = data.dwField36;
		bang = data.dwField37;

		holyrating = data.dwField38;
		init_identifier(holys,data.strField39);

		//sky 新增加属性
		str		= data.dwField40;	//力量
		inte	= data.dwField41;	//智力
		dex		= data.dwField42;	//敏捷
		spi		= data.dwField43;	//精神
		con		= data.dwField44;	//耐力
		atkhpp	= data.dwField45;  //魔法免伤
		mtkhpp	= data.dwField46;  //魔法免伤


	}

	zObjectB():zEntry()/*,hole("0","0-0")*/
	{
		bzero(this,sizeof(zObjectB));
	};

	void init_identifier(std::vector<DWORD>& list,const std::string& info)
	{
		list.clear();
		getAllNum(info.c_str(),list);
	}

	void init_leechdom(const std::string& info)
	{
		leechdom_t* p = Parse3<leechdom_t>()(info,":");
		if (p) {
			leechdom = *p;
			SAFE_DELETE(p);
		}  
	}  

	/*void init_socket(const std::string& socket_info)
	{
	std::string::size_type pos = socket_info.find(':');
	if (pos != std::string::npos) {
	hole = socket(socket_info.substr(0,pos),socket_info.substr(pos+1));
	}

	}*/

	void init_need_skills(const std::string& skills_list)
	{  
		std::string::size_type pos = skills_list.find(':');
		if (pos != std::string::npos) {
			need_skill = skills(skills_list.substr(0,pos),skills_list.substr(pos+1));
		}
	}

	void init_need_material(const std::string& materials)
	{
		need_material.stuffs.clear();
		Split<Parse3> p;
		std::string::size_type pos = materials.find(':');
		if (pos != std::string::npos) {
			need_material.gold = atoi(materials.substr(0,pos).c_str());
			p(materials.substr(pos+1),need_material.stuffs,";","-");
		}
	}

};

//------------------------------------
// ColorObjectBase
//------------------------------------
struct ColorObjectBase{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}

	DWORD  dwField0;    // 编号
	char  strField1[64];    // 名称
	char  strField2[32];    // 连接符
	char  strField3[128];    // 金色品质
	char  strField4[32];    // 力量
	char  strField5[32];    // 智力
	char  strField6[32];    // 敏捷
	char  strField7[32];    // 精神
	char  strField8[32];    // 体质
	char  strField9[32];    // 五行属性
	char  strField10[32];    // 最小物攻
	char  strField11[32];    // 最大物攻
	char  strField12[32];    // 最小魔攻
	char  strField13[32];    // 最大魔攻
	char  strField14[32];    // 物防
	char  strField15[32];    // 魔防
	char  strField16[32];    // 最大生命值
	char  strField17[32];    // 最大法术值
	char  strField18[32];    // 最大体力值
	char  strField19[32];    // 移动速度
	char  strField20[32];    // 生命值恢复
	char  strField21[32];    // 法术值恢复
	char  strField22[32];    // 体力值恢复
	char  strField23[32];    // 攻击速度
	char  strField24[32];    // 增加物理攻击力
	char  strField25[32];    // 增加物理防御力
	char  strField26[32];    // 增加魔法攻击力
	char  strField27[32];    // 增加魔法防御力
	char  strField28[32];    // 命中率
	char  strField29[32];    // 闪避率
	char  strField30[32];    // 抗毒增加
	char  strField31[32];    // 抗麻痹增加
	char  strField32[32];    // 抗眩晕增加
	char  strField33[32];    // 抗噬魔增加
	char  strField34[32];    // 抗噬力增加
	char  strField35[32];    // 抗混乱增加
	char  strField36[32];    // 抗冰冻增加
	char  strField37[32];    // 抗石化增加
	char  strField38[32];    // 抗失明增加
	char  strField39[32];    // 抗定身增加
	char  strField40[32];    // 抗减速增加
	char  strField41[32];    // 抗诱惑增加
	char  strField42[32];    // 恢复耐久度
	char  strField43[32];    // 重击
	DWORD  dwField44;    // 神圣装备几率
	char  strField45[1024];    // 技能加成
	char  strField46[32];    // 全系技能加成
};

//一个范围值得描述
struct rangeValue
{
	WORD min;
	WORD max;
};

struct luckRangeValue
{
	WORD per;  //是否产生本属性的几率
	rangeValue data;  //产生属性值的随机范围
	WORD sleightValue;  //根据熟练度产生的加权值

	operator int()
	{
		return per;
	}
};

static void fillRangeValue(const char *str,rangeValue &data)
{
	std::vector<DWORD> num;
	int i =getAllNum(str,num);
	if (i!=2)
	{
		data.min=0;
		data.max=0;
	}
	else
	{
		data.min=num[0];
		data.max=num[1];
	}
}

static void fillLuckRangeValue(char *str,luckRangeValue &data)
{
	std::vector<DWORD> num;
	int i =getAllNum(str,num);
	if (i<3)
	{
		if (i!=1)
		{
			Zebra::logger->debug("fillLuckRangeValue %s",str);
		}
		data.per=0;
		data.data.min=0;
		data.data.max=0;
		data.sleightValue=0;
	}
	else
	{
		data.per=num[0];
		data.data.min=num[1];
		data.data.max=num[2];
		if (i==4)
			data.sleightValue=num[3];
		else
			data.sleightValue=0;
	}
}

struct skillbonus {
	WORD odds; //几率
	WORD id; //技能 id
	WORD level; // 技能等级
	skillbonus(std::string _odds="0",std::string _id="0",std::string _level="0") : odds(atoi(_odds.c_str())),id(atoi(_id.c_str())),level(atoi(_level.c_str()))
	{ }
}; 

template <class Base,WORD tt>
struct zColorObjectB:public zEntry
{
	//std::string prefix;      // 名称
	char prefix[MAX_NAMESIZE];      // 名称
	//std::string joint;      // 连接符
	char joint[MAX_NAMESIZE];      // 连接符
	std::vector<WORD> golds;  // 金色品质

	union {
		luckRangeValue _p1[5];
		struct {
			luckRangeValue str;      // 力量
			luckRangeValue inte;    // 智力
			luckRangeValue dex;      // 敏捷
			luckRangeValue spi;      // 精神
			luckRangeValue con;      // 体质
		};  
	};
	luckRangeValue five;    // 五行属性

	rangeValue pdamage;      // 最小物攻
	rangeValue maxpdamage;    // 最大物攻
	rangeValue mdamage;      // 最小魔攻
	rangeValue maxmdamage;    // 最大魔攻
	rangeValue pdefence;      // 物防
	rangeValue mdefence;      // 魔防

	luckRangeValue maxhp;    // 最大生命值
	luckRangeValue maxmp;    // 最大法术值
	luckRangeValue maxsp;    // 最大体力值

	luckRangeValue mvspeed;    // 移动速度
	luckRangeValue hpr;      // 生命值恢复
	luckRangeValue mpr;      // 法术值恢复
	luckRangeValue spr;      // 体力值恢复
	luckRangeValue akspeed;    // 攻击速度

	union {
		luckRangeValue _p2[18];
		struct {
			luckRangeValue pdam;    // 增加物理攻击力
			luckRangeValue pdef;    // 增加物理防御力
			luckRangeValue mdam;    // 增加魔法攻击力
			luckRangeValue mdef;    // 增加魔法防御力

			luckRangeValue poisondef;  // 抗毒增加
			luckRangeValue lulldef;    // 抗麻痹增加
			luckRangeValue reeldef;    // 抗眩晕增加
			luckRangeValue evildef;    // 抗噬魔增加
			luckRangeValue bitedef;    // 抗噬力增加
			luckRangeValue chaosdef;  // 抗混乱增加
			luckRangeValue colddef;    // 抗冰冻增加
			luckRangeValue petrifydef;    // 抗石化增加
			luckRangeValue blinddef;    // 抗失明增加
			luckRangeValue stabledef;    // 抗定身增加
			luckRangeValue slowdef;    // 抗减速增加
			luckRangeValue luredef;    // 抗诱惑增加

			luckRangeValue atrating;    // 命中率
			luckRangeValue akdodge;    // 闪避率

		};
	};  

	luckRangeValue resumedur;    // 恢复耐久度
	luckRangeValue bang;    // 重击
	WORD holyrating;  //神圣装备几率

	std::vector<skillbonus> skill;  // 技能加成
	skillbonus skills;        // 全系技能加成

	WORD type;

public:
	void fill(Base &data)
	{
		id = data.dwField0;
		strncpy(name,data.strField1,MAX_NAMESIZE);

		//prefix =  data.strField1;
		//joint = data.strField2;
		strncpy(prefix,data.strField1,MAX_NAMESIZE);
		strncpy(joint,data.strField2,MAX_NAMESIZE);
		getAllNum(data.strField3,golds);
		fillLuckRangeValue(data.strField4,str);
		fillLuckRangeValue(data.strField5,inte);
		fillLuckRangeValue(data.strField6,dex);
		fillLuckRangeValue(data.strField7,spi);
		fillLuckRangeValue(data.strField8,con);

		fillLuckRangeValue(data.strField9,five);

		fillRangeValue(data.strField10,pdamage);
		fillRangeValue(data.strField11,maxpdamage);
		fillRangeValue(data.strField12,mdamage);
		fillRangeValue(data.strField13,maxmdamage);
		fillRangeValue(data.strField14,pdefence);
		fillRangeValue(data.strField15,mdefence);

		fillLuckRangeValue(data.strField16,maxhp);
		fillLuckRangeValue(data.strField17,maxmp);
		fillLuckRangeValue(data.strField18,maxsp);
		fillLuckRangeValue(data.strField19,mvspeed);
		fillLuckRangeValue(data.strField20,hpr);
		fillLuckRangeValue(data.strField21,mpr);
		fillLuckRangeValue(data.strField22,spr);
		fillLuckRangeValue(data.strField23,akspeed);
		fillLuckRangeValue(data.strField24,pdam);
		fillLuckRangeValue(data.strField25,pdef);
		fillLuckRangeValue(data.strField26,mdam);
		fillLuckRangeValue(data.strField27,mdef);
		fillLuckRangeValue(data.strField28,atrating);
		fillLuckRangeValue(data.strField29,akdodge);

		fillLuckRangeValue(data.strField30,poisondef);
		fillLuckRangeValue(data.strField31,lulldef);
		fillLuckRangeValue(data.strField32,reeldef);
		fillLuckRangeValue(data.strField33,evildef);
		fillLuckRangeValue(data.strField34,bitedef);
		fillLuckRangeValue(data.strField35,chaosdef);
		fillLuckRangeValue(data.strField36,colddef);
		fillLuckRangeValue(data.strField37,petrifydef);
		fillLuckRangeValue(data.strField38,blinddef);
		fillLuckRangeValue(data.strField39,stabledef);
		fillLuckRangeValue(data.strField40,slowdef);
		fillLuckRangeValue(data.strField41,luredef);
		fillLuckRangeValue(data.strField42,resumedur);
		//bang = data.dwField43;
		fillLuckRangeValue(data.strField43,bang);
		holyrating = data.dwField44;

		init_skill(data.strField45);
		init_skills(data.strField46);

		//Zebra::logger->debug("id:%d,name:%s",id,name);
#if 0
		//恢复耐久度格式单独处理
		{
			std::vector<DWORD> num;
			int i =getAllNum(data.strField47,num);
			if (i!=7)
			{
				bzero(&durpoint,sizeof(durpoint));
				bzero(&dursecond,sizeof(dursecond));
			}
			else
			{
				durpoint.per=num[0];
				durpoint.data.min=num[1];
				durpoint.data.max=num[2];
				durpoint.sleightValue=num[3];
				dursecond.per=0;
				dursecond.data.min=num[4];
				dursecond.data.max=num[5];
				dursecond.sleightValue=num[6];
			}
		}
#endif

	}

	zColorObjectB():zEntry()
	{
		bzero(this,sizeof(zColorObjectB));
		type=tt;
	};

	void init_skill(const std::string& info)
	{
		skill.clear();
		Split<Parse3> p;
		p(info,skill,";",":");
	}

	void init_skills(const std::string& info)
	{
		skillbonus* p = Parse3<skillbonus>()(info,":");
		if (p) {
			skills = *p;
			SAFE_DELETE(p);
		}  
		else if (strcmp(info.c_str(),"0")!=0)
		{       
			Zebra::logger->debug("init_skills(%d),%s",id,info.c_str());
		}     
	}  

};

typedef ColorObjectBase GoldObjectBase;
typedef ColorObjectBase DropGoldObjectBase;
typedef ColorObjectBase BlueObjectBase;
typedef zColorObjectB<BlueObjectBase,1> zBlueObjectB;
typedef zColorObjectB<GoldObjectBase,2> zGoldObjectB;
typedef zColorObjectB<DropGoldObjectBase,3> zDropGoldObjectB;

//------------------------------------
// SetObjectBase
//------------------------------------
struct SetObjectBase{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 编号
	char  strField1[32];    // 名称
	char  strField2[64];    // 套装5
	char  strField3[32];    // 套装4
	char  strField4[32];    // 套装3
	char  strField5[32];    // 套装2
	char  strField6[32];    // 套装1
	DWORD  dwField7;    // 属性标识
};//导出 SetObjectBase 成功，共 532 条记录

struct zSetObjectB:public zEntry
{
	struct SET
	{
		WORD odds;
		std::vector<WORD> ids;
	};

	typedef std::vector<SET> SETS;
	typedef SETS::iterator iterator;
	SETS sets;
	DWORD mark;

	void fill(SetObjectBase& data)
	{
		id = data.dwField0;
		strncpy(name,data.strField1,MAX_NAMESIZE);
		init_set(data.strField2);
		init_set(data.strField3);
		init_set(data.strField4);
		init_set(data.strField5);
		init_set(data.strField6);
		mark = data.dwField7;
	}

	zSetObjectB():zEntry()
	{
		bzero(this,sizeof(zSetObjectB));
	};

	void init_set(const std::string& info)
	{
		sets.clear();
		std::string::size_type pos = info.find(':');
		SET set;
		if (pos != std::string::npos) {
			set.odds = atoi(info.substr(0,pos).c_str());
			getAllNum(info.substr(pos+1).c_str(),set.ids);
		}
		sets.push_back(set);
	}

};

//------------------------------------
// FiveSetBase
//------------------------------------
struct FiveSetBase{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 属性标识
	char  strField1[16];    // 物理伤害减少x%
	char  strField2[16];    // 法术伤害减少x%
	char  strField3[16];    // 增加伤害值x%
	char  strField4[16];    // 伤害反射x%
	char  strField5[16];    // x%忽视目标防御
};//导出 FiveSetBase 成功，共 4 条记录


struct zFiveSetB:public zEntry
{
	rangeValue dpdam; //物理伤害减少%x
	rangeValue dmdam; //法术伤害减少%x
	rangeValue bdam; //增加伤害x%
	rangeValue rdam; //伤害反射%x
	rangeValue ignoredef; //%x忽视目标防御

	void fill(FiveSetBase& data)
	{
		id = data.dwField0;
		fillRangeValue(data.strField1,dpdam);
		fillRangeValue(data.strField2,dmdam);
		fillRangeValue(data.strField3,bdam);    
		fillRangeValue(data.strField4,rdam);    
		fillRangeValue(data.strField5,ignoredef);    
	}

	zFiveSetB():zEntry()
	{
		bzero(this,sizeof(zFiveSetB));
	};  
};

//------------------------------------
// HolyObjectBase
//------------------------------------
struct HolyObjectBase{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 编号
	char  strField1[64];    // 名称
	char  strField2[16];    // 神圣一击
	char  strField3[16];    // 增加伤害值x％
	char  strField4[16];    // 五行属性增加
	char  strField5[16];    // 生命值恢复
	char  strField6[16];    // 法术值恢复
	char  strField7[16];    // 体力值恢复
	char  strField8[16];    // 攻击速度
	char  strField9[16];    // 移动速度
	char  strField10[16];    // 命中率
	char  strField11[16];    // 闪避率
	char  strField12[16];    // 技能加成
	char  strField13[16];    // 全系技能加成
	char  strField14[16];    // 双倍经验
	char  strField15[16];    // 增加掉宝率
};//导出 HolyObjectBase 成功，共 705 条记录

struct zHolyObjectB:public zEntry
{
	WORD  holy;        // 神圣一击
	luckRangeValue  damage;    // 增加伤害值x％
	luckRangeValue  fivepoint;    // 五行属性增加

	luckRangeValue hpr;      // 生命值恢复
	luckRangeValue mpr;      // 法术值恢复
	luckRangeValue spr;      // 体力值恢复

	luckRangeValue akspeed;    // 攻击速度
	luckRangeValue mvspeed;    // 移动速度

	luckRangeValue atrating;    // 命中率
	luckRangeValue akdodge;      // 闪避率

	std::vector<skillbonus> skill;  // 技能加成
	skillbonus skills;        // 全系技能加成

	luckRangeValue doublexp;    //%x双倍经验
	luckRangeValue mf;       //掉宝率

	void fill(HolyObjectBase &data)
	{
		id = data.dwField0;
		strncpy(name,data.strField1,MAX_NAMESIZE);
		holy = atoi(data.strField2);

		fillLuckRangeValue(data.strField3,damage);    
		fillLuckRangeValue(data.strField4,fivepoint);
		fillLuckRangeValue(data.strField5,hpr);
		fillLuckRangeValue(data.strField6,mpr);
		fillLuckRangeValue(data.strField7,spr);
		fillLuckRangeValue(data.strField8,akspeed);
		fillLuckRangeValue(data.strField9,mvspeed);
		fillLuckRangeValue(data.strField10,atrating);
		fillLuckRangeValue(data.strField11,akdodge);

		init_skill(data.strField12);
		init_skills(data.strField13);

		fillLuckRangeValue(data.strField14,doublexp);
		fillLuckRangeValue(data.strField15,mf);

	}

	zHolyObjectB():zEntry()
	{
		bzero(this,sizeof(zHolyObjectB));
	};

	void init_skill(const std::string& info)
	{
		skill.clear();
		Split<Parse3> p;
		p(info,skill,";",":");
	}

	void init_skills(const std::string& info)
	{
		skillbonus* p = Parse3<skillbonus>()(info,":");
		if (p) {
			skills = *p;
			SAFE_DELETE(p);
		}
	}  
};

//------------------------------------
// UpgradeObjectBase
//------------------------------------
struct UpgradeObjectBase{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 编号
	DWORD  dwField1;    // 物品ID
	char  strField2[64];    // 名称
	DWORD  dwField3;    // 类型
	DWORD  dwField4;    // 升级原料
	DWORD  dwField5;    // 需要银子
	DWORD  dwField6;    // 对应成功率
	DWORD  dwField7;    // 最小物攻增加
	DWORD  dwField8;    // 最大物攻增加
	DWORD  dwField9;    // 最小魔攻增加
	DWORD  dwField10;    // 最大魔攻增加
	DWORD  dwField11;    // 物防增加
	DWORD  dwField12;    // 魔防增加
	DWORD  dwField13;    // 生命值增加
};//导出 UpgradeObjectBase 成功，共 6345 条记录


struct zUpgradeObjectB:public zEntry
{
	DWORD dwObjectID;    // 物品ID
	WORD level;    // 类型

	WORD stuff;    // 升级原料

	WORD gold;    // 需要银子

	WORD odds;    // 对应成功率

	DWORD pdamage;        // 最小攻击力
	DWORD maxpdamage;      // 最大攻击力
	DWORD mdamage;        // 最小法术攻击力
	DWORD maxmdamage;      // 最大法术攻击力

	DWORD pdefence;        // 物防
	DWORD mdefence;        // 魔防
	DWORD maxhp;          // 最大生命值

	void fill(UpgradeObjectBase  &data)
	{
		id = data.dwField0;
		dwObjectID = data.dwField1;
		strncpy(name,data.strField2,MAX_NAMESIZE);
		level = data.dwField3;
		stuff = data.dwField4;
		gold = data.dwField5;
		odds = data.dwField6;

		pdamage = data.dwField7;
		maxpdamage = data.dwField8;
		mdamage = data.dwField9;
		maxmdamage = data.dwField10;

		pdefence = data.dwField11;
		mdefence = data.dwField12;

		maxhp = data.dwField13;
	}

	zUpgradeObjectB():zEntry()
	{
		bzero(this,sizeof(zUpgradeObjectB));
	}
};

//------------------------------------
// NpcBase
//------------------------------------
struct NpcBase
{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 编号
	char  strField1[32];    // 名称
	DWORD  dwField2;    // 类型
	DWORD  dwField3;    // 等级
	DWORD  dwField4;    // 生命值
	DWORD  dwField5;    // 经验值

	DWORD  dwField6;    // 力
	DWORD  dwField7;    // 智
	DWORD  dwField8;    // 敏捷
	DWORD  dwField9;    // 精神
	DWORD  dwField10;    // 体质
	DWORD  dwField11;    // 体质

	DWORD  dwField12;    // 颜色
	DWORD  dwField13;    // ai
	DWORD  dwField14;    // 移动间隔
	DWORD  dwField15;    // 攻击间隔
	DWORD  dwField16;    // 最小物理防御力
	DWORD  dwField17;    // 最大物理防御力
	DWORD  dwField18;    // 最小法术防御力
	DWORD  dwField19;    // 最大法术防御力
	DWORD  dwField20;    // 五行属性
	DWORD  dwField21;    // 五行点数
	char  strField22[1024];    // 攻击类型
	DWORD  dwField23;    // 最小法术攻击
	DWORD  dwField24;    // 最大法术攻击
	DWORD  dwField25;    // 最小攻击力
	DWORD  dwField26;    // 最大攻击力
	DWORD  dwField27;    // 技能
	char  strField28[4096];    // 携带物品
	DWORD  dwField29;    // 魂魄之石几率
	char  strField30[1024];    // 使用技能
	char  strField31[1024];    // 状态
	DWORD  dwField32;    // 躲避率
	DWORD  dwField33;    // 命中率
	DWORD  dwField34;    // 图片
	DWORD  dwField35;    // 品质
	DWORD  dwField36;    // 怪物类别
	DWORD  dwField37;    // 纸娃娃图片
	char  strField38[64];    // 回血
	DWORD  dwField39;    // 二进制标志
	DWORD  dwField40;    // 二进制标志
	DWORD  dwField41;    // sky 极品倍率
};

struct CarryObject
{
	DWORD id;
	int   rate;
	int   minnum;
	int   maxnum;
	CarryObject()
	{
		id = 0;
		rate = 0;
		minnum = 0;
		maxnum = 0;
	}
};

typedef std::vector<CarryObject> NpcLostObject;

struct NpcCarryObject : private zNoncopyable
{
	NpcCarryObject() {};
	bool set(const char *objects)
	{
		bool retval = true;
		//mlock.lock();
		cov.clear();
		if (strcmp(objects,"0"))
		{
			std::vector<std::string> obs;
			stringtok(obs,objects,";");
			for(std::vector<std::string>::const_iterator it = obs.begin(); it != obs.end(); it++)
			{
				std::vector<std::string> rt;
				stringtok(rt,*it,":");
				if (3 == rt.size())
				{
					CarryObject co;
					co.id = atoi(rt[0].c_str());
					co.rate = atoi(rt[1].c_str());
					std::vector<std::string> nu;
					stringtok(nu,rt[2],"-");
					if (2 == nu.size())
					{
						co.minnum = atoi(nu[0].c_str());
						co.maxnum = atoi(nu[1].c_str());
						cov.push_back(co);
					}
					else
						retval = false;
				}
				else
					retval = false;
			}
		}
		//mlock.unlock();
		return retval;
	}

	/**
	* \brief 物品掉落处理
	* \param nlo npc携带物品集合
	* \param value 掉落率打折比
	* \param value1 掉落率增加
	* \param value2 银子掉落率增加
	*/
	void lost(NpcLostObject &nlo,int value,int value1,int value2,int vcharm,int vlucky,int player_level,int DropRate,int DropRateLevel)
	{
		//mlock.lock();
		if (vcharm>1000) vcharm=1000;
		if (vlucky>1000) vlucky=1000;
		for(std::vector<CarryObject>::const_iterator it = cov.begin(); it != cov.end(); it++)
		{
			//Zebra::logger->debug("%u,%u,%u,%u",(*it).id,(*it).rate,(*it).minnum,(*it).maxnum);
			switch((*it).id)
			{
			case 665:
				{
					int vrate = (int)(((*it).rate/value)*(1+value1/100.0f)*(1+value2/100.0f)*(1+vcharm/1000.0f)*(1+vlucky/1000.0f));
					if (selectByTenTh(vrate))
					{
						nlo.push_back(*it);
					}
				}
				break;
			default:
				{
					int vrate = (int)(((*it).rate/value)*(1+value1/100.0f)*(1+vcharm/1000.0f)*(1+vlucky/1000.0f));
					if (player_level<= DropRateLevel)
					{
						if (selectByTenTh(vrate * DropRate))
						{
							nlo.push_back(*it);
						}
					}
					else
					{
						if (selectByTenTh(vrate))
						{
							nlo.push_back(*it);
						}
					}
				}
				break;
			}
		}
		//mlock.unlock();
	}
	/**
	* \brief 全部物品掉落处理
	* \param nlo npc携带物品集合
	* \param value 掉落率打折比
	* \param value1 掉落率增加
	* \param value2 银子掉落率增加
	*/
	void lostAll(NpcLostObject &nlo)
	{
		for(std::vector<CarryObject>::const_iterator it = cov.begin(); it != cov.end(); it++)
		{
			nlo.push_back(*it);
		}
	}

	/**
	* \brief 装备物品全部掉落处理(绿怪专用)
	* \param nlo npc携带物品集合
	* \param value 掉落率打折比
	* \param value1 掉落率增加
	* \param value2 银子掉落率增加
	*/
	void lostGreen(NpcLostObject &nlo,int value=1,int value1=0,int value2=0,int vcharm = 0,int vlucky = 0);
private:
	std::vector<CarryObject> cov;
	//zMutex mlock;
};


struct aTypeS{
	aTypeS()
	{
		byValue[0] = 0;
		byValue[1] = 0;
	}
	union {
		struct {
			BYTE byAType;
			BYTE byAction;
		};
		BYTE byValue[2];
	};
};

enum
{
	NPC_TYPE_HUMAN    = 0,///人型
	NPC_TYPE_NORMAL    = 1,/// 普通类型
	NPC_TYPE_BBOSS    = 2,/// 大Boss类型
	NPC_TYPE_LBOSS    = 3,/// 小Boss类型
	NPC_TYPE_BACKBONE  = 4,/// 精英类型
	NPC_TYPE_GOLD    = 5,/// 黄金类型
	NPC_TYPE_TRADE    = 6,/// 买卖类型
	NPC_TYPE_TASK    = 7,/// 任务类型
	NPC_TYPE_GUARD    = 8,/// 士兵类型
	NPC_TYPE_PET    = 9,/// 宠物类型
	NPC_TYPE_BACKBONEBUG= 10,/// 精怪类型
	NPC_TYPE_SUMMONS  = 11,/// 召唤类型
	NPC_TYPE_TOTEM    = 12,/// 图腾类型
	NPC_TYPE_AGGRANDIZEMENT = 13,/// 强化类型
	NPC_TYPE_ABERRANCE  = 14,/// 变异类型
	NPC_TYPE_STORAGE  = 15,/// 仓库类型
	NPC_TYPE_ROADSIGN  = 16,/// 路标类型
	NPC_TYPE_TREASURE  = 17,/// 宝箱类型
	NPC_TYPE_WILDHORSE  = 18,/// 野马类型
	NPC_TYPE_MOBILETRADE  = 19,/// 流浪小贩
	NPC_TYPE_LIVENPC  = 20,/// 生活npc（不战斗，攻城时消失）
	NPC_TYPE_DUCKHIT  = 21,/// 蹲下才能打的npc
	NPC_TYPE_BANNER    = 22,/// 旗帜类型
	NPC_TYPE_TRAP    = 23,/// 陷阱类型
	NPC_TYPE_MAILBOX  =24,///邮箱
	NPC_TYPE_AUCTION  =25,///拍卖管理员
	NPC_TYPE_UNIONGUARD  =26,///帮会守卫
	NPC_TYPE_SOLDIER  =27,///士兵，只攻击外国人
	NPC_TYPE_UNIONATTACKER  =28,///攻方士兵
	NPC_TYPE_SURFACE = 29,/// 地表类型
	NPC_TYPE_CARTOONPET = 30,/// 替身宝宝
	NPC_TYPE_PBOSS = 31,/// 紫色BOSS
	NPC_TYPE_RESOURCE = 32, /// 资源类NPC

	//sky添加
	NPC_TYPE_GHOST	= 999,  /// 元神类NPC
	NPC_TYPE_ANIMON   = 33,   /// 动物类怪物
	NPC_TYPE_GOTO	= 34,	///传送点
	NPC_TYPE_RESUR  = 35,	///复活点
	NPC_TYPE_UNFIGHTPET	= 36, ///非战斗宠物
	NPC_TYPE_FIGHTPET	= 37, ///战斗宠物
	NPC_TYPE_RIDE		= 38, ///坐骑
	NPC_TYPE_TURRET	= 39, /// 炮塔
	NPC_TYPE_BARRACKS = 40, /// 兵营
	NPC_TYPE_CAMP = 41,		/// 基地
};

enum
{
	NPC_ATYPE_NEAR    = 1,/// 近距离攻击
	NPC_ATYPE_FAR    = 2,/// 远距离攻击
	NPC_ATYPE_MFAR    = 3,/// 法术远程攻击
	NPC_ATYPE_MNEAR    = 4,/// 法术近身攻击
	NPC_ATYPE_NOACTION  = 5,    /// 无攻击动作
	NPC_ATYPE_ANIMAL    = 6  /// 动物类
};

///npc使用一个技能的描述
struct npcSkill
{
	DWORD id;///技能id
	int needLevel;///技能id
	int rate;///使用几率
	int coefficient;///升级系数

	npcSkill():id(0),needLevel(0),rate(0),coefficient(0){}
	npcSkill(const npcSkill &skill)
	{
		id = skill.id;
		needLevel = skill.needLevel;
		rate = skill.rate;
		coefficient = skill.coefficient;
	}
	npcSkill& operator = (const npcSkill &skill)
	{
		id = skill.id;
		needLevel = skill.needLevel;
		rate = skill.rate;
		coefficient = skill.coefficient;
		return *this;
	}
};

struct npcRecover
{
	DWORD start;
	BYTE type;
	DWORD num;

	npcRecover()
	{
		start = 0;
		type = 0;
		num = 0;
	}

	void parse(const char * str)
	{
		if (!str) return;

		std::vector<std::string> vec;

		vec.clear();
		stringtok(vec,str,":");
		if (3==vec.size())
		{
			start = atoi(vec[0].c_str());
			type = atoi(vec[1].c_str());
			num = atoi(vec[2].c_str());
		}
	}
};

/**
* \brief Npc基本表格数据
*
*/
struct zNpcB : public zEntry
{
	DWORD  kind;        // 类型
	DWORD  level;        // 等级
	DWORD  hp;          // 生命值
	DWORD  exp;        // 经验值
	DWORD  str;        // 力量
	DWORD   inte;        // 智力
	DWORD   dex;        // 敏捷
	DWORD   men;        // 精神
	DWORD   con;        // 体质
	DWORD   cri;        // 暴击
	DWORD  color;        // 颜色
	DWORD  ai;          // ai
	DWORD  distance;      // 移动间隔
	DWORD  adistance;      // 攻击间隔
	DWORD  pdefence;      // 最小物理防御力
	DWORD  maxpdefence;    // 最大物理防御力
	DWORD  mdefence;      // 最小法术防御力
	DWORD  maxmdefence;    // 最大法术防御力
	DWORD  five;        // 五行属性
	DWORD   fivepoint;      // 五行点数
	std::vector<aTypeS> atypelist;  // 攻击类型
	DWORD  mdamage;      // 最小法术攻击
	DWORD  maxmdamage;      // 最大法术攻击
	DWORD  damage;        // 最小攻击力
	DWORD  maxdamage;      // 最大攻击力
	DWORD  skill;        // 技能
	//char  object[1024 + 1];  // 携带物品
	NpcCarryObject nco;
	DWORD  ChangeNpcID;     //soulrate;      //sky NPC变身ID
	char  skills[1024];    // 使用技能
	char  state[1024];    // 状态
	DWORD  dodge;        // 躲避率
	DWORD  rating;        // 命中率
	DWORD  pic;        // 图片
	DWORD  trait;        //品质
	DWORD  bear_type;      //怪物类别
	DWORD  pet_pic;      //宠物图片
	npcRecover recover;
	DWORD  flags;      //二进制标志，目前有一个，可不可被外国人杀
	DWORD  allyVisit;      //可被盟国访问的等级 0：不可访问 1：1级可访问 2：2级可访问

	std::map<int,std::vector<npcSkill> > skillMap;

	DWORD  Need_Probability; //sky 极品概率

	bool parseSkills(const char * str)
	{
		skillMap.clear();
		strncpy(skills,str,sizeof(skills));

		bool ret = false;
		std::vector<std::string> type_v;
		stringtok(type_v,str,";");
		if (type_v.size()>0)
		{
			std::vector<std::string> type_sub_v,skill_v,prop_v;
			std::vector<std::string>::iterator type_it,skill_it;

			for (type_it=type_v.begin();type_it!=type_v.end();type_it++)
			{
				type_sub_v.clear();
				stringtok(type_sub_v,type_it->c_str(),":");
				if (2==type_sub_v.size())
				{
					int type = atoi(type_sub_v[0].c_str());

					std::vector<npcSkill> oneTypeSkills;
					skill_v.clear();
					stringtok(skill_v,type_sub_v[1].c_str(),",");
					for (skill_it=skill_v.begin();skill_it!=skill_v.end();skill_it++)
					{
						prop_v.clear();
						stringtok(prop_v,skill_it->c_str(),"-");
						if (4==prop_v.size())
						{
							npcSkill oneSkill;
							oneSkill.id = atoi(prop_v[0].c_str());
							oneSkill.needLevel = atoi(prop_v[1].c_str());
							oneSkill.rate = atoi(prop_v[2].c_str());
							oneSkill.coefficient = atoi(prop_v[3].c_str());

							oneTypeSkills.push_back(oneSkill);
						}
					}
					if (oneTypeSkills.size()>0)
					{
						skillMap[type] = oneTypeSkills;
						ret = true;
					}
				}
			}
		}
		return ret;
	}

	/**
	* \brief 根据类型随机取出一个npc技能的描述
	*
	* \param type 技能类型
	* \param skill 返回值，取得的技能描述
	* \return 是否取得成功
	*/
	bool getRandomSkillByType(int type,npcSkill &skill)
	{
		if (skillMap.find(type)==skillMap.end()) return false;

		skill = skillMap[type][randBetween(0,skillMap[type].size()-1)];
		return true;
	}

	/**
	* \brief 取得所有可用的技能ID
	*
	*
	* \param list 技能ID列表
	* \return bool 是否有技能
	*/
	bool getAllSkills(std::vector<DWORD> & list,WORD level)
	{
		std::map<int,std::vector<npcSkill> >::iterator type_it;
		std::vector<npcSkill>::iterator skill_it;
		for (type_it=skillMap.begin();type_it!=skillMap.end();type_it++)
		{
			for (skill_it=type_it->second.begin();skill_it!=type_it->second.end();skill_it++)
				if (level>=skill_it->needLevel)
					list.push_back(skill_it->id);
		}
		return list.size()>0;
	}

	/**
	* \brief 增加一个npc技能
	* \param type 技能分类
	* \param id 要增加的技能id
	* \param rate 施放几率
	* \param coefficient 系数
	*/
	void addSkill(int type,DWORD id,int needLevel,int rate,int coefficient = 0)
	{
		npcSkill s;
		s.id = id;
		s.needLevel = needLevel;
		s.rate = rate;
		s.coefficient = coefficient;
		skillMap[type].push_back(s);
	}

	/**
	* \brief 删除一个npc技能
	*
	*
	* \param id 要删除的技能id
	* \return npc没有该技能则返回false
	*/
	bool delSkill(DWORD id)
	{
		std::map<int,std::vector<npcSkill> >::iterator v_it;
		for (v_it=skillMap.begin();v_it!=skillMap.end();v_it++)
		{
			std::vector<npcSkill> v = v_it->second;
			std::vector<npcSkill>::iterator s_it;
			for (s_it=v.begin();s_it!=v.end();s_it++)
			{
				if (s_it->id==id)
				{
					v.erase(s_it);
					return true;
				}
			}
		}
		return false;
	}

	/**
	* \brief 设置npc的攻击类型
	*
	*
	* \param data 传入的字符串
	* \param size 字符串大小
	*/
	void setAType(const char *data,int size)
	{

		//Zebra::logger->error("address = %x",data);
		if(NULL == data)
		{
			fprintf(stderr,"data == NULL");
			return;
		}
		atypelist.clear();
		size = 1024;

		char Buf[1024];
		bzero(Buf,size);
		strncpy(Buf,data,size);
		std::vector<std::string> v_fir;
		stringtok(v_fir,Buf,":");
		for(std::vector<std::string>::iterator iter = v_fir.begin() ; iter != v_fir.end() ; iter++)
		{
			std::vector<std::string> v_sec;
			stringtok(v_sec,iter->c_str(),"-");

			if (v_sec.size() != 2)
			{
				return;
			}

			aTypeS aValue;
			std::vector<std::string>::iterator iter_1 = v_sec.begin();

			for(int i=0; i<2; i++)
			{
				aValue.byValue[i] = (BYTE)atoi(iter_1->c_str());
				iter_1 ++;
			}
			atypelist.push_back(aValue);
		}
		return;
	}

	/**
	* \brief 取得npc的攻击类型和动画类型
	*
	*
	* \param type 输出 攻击类型
	* \param action
	*/
	void getATypeAndAction(BYTE &type,BYTE &action)
	{    
		int size = atypelist.size();
		if (size == 0)
		{
			type = NPC_ATYPE_NEAR;
			action = 4 ;//Cmd::AniTypeEnum::Ani_Attack;//Cmd::Ani_Attack
			return;
		}
		int num = randBetween(0,size-1);
		type = atypelist[num].byAType;
		action = atypelist[num].byAction;
	}

	/**
	* \brief 根据表格中读出的数据填充zNpcB结构
	*
	*
	* \param npc 从表中读出的数据
	*/
	void fill(const NpcBase &npc)
	{
		setAType(npc.strField22,1024);
		id=          npc.dwField0;
		strncpy(name,npc.strField1,MAX_NAMESIZE);
		kind=        npc.dwField2;
		level=        npc.dwField3;
		hp=          npc.dwField4;
		exp=        npc.dwField5;
		str=        npc.dwField6;
		inte=        npc.dwField7;
		dex=        npc.dwField8;
		men=        npc.dwField9;
		con=        npc.dwField10;
		cri=        npc.dwField11;
		color=        npc.dwField12;
		ai=          npc.dwField13;
		distance=      (0==npc.dwField14)?640:npc.dwField14;
		adistance=       (0==npc.dwField15)?1000:npc.dwField15;
		pdefence=      npc.dwField16;
		maxpdefence=    npc.dwField17;
		mdefence=      npc.dwField18;
		maxmdefence=    npc.dwField19;
		five=        npc.dwField20;
		fivepoint=      npc.dwField21;

		mdamage=      npc.dwField23;
		maxmdamage=      npc.dwField24;
		damage=        npc.dwField25;
		maxdamage=      npc.dwField26;
		skill=        npc.dwField27;
		if (!nco.set(npc.strField28))
			Zebra::logger->error("Npc表格携带物品格式解析错误：%u,%s,\'%s\'",id,name,npc.strField28);
		ChangeNpcID=      npc.dwField29;
		parseSkills(npc.strField30);
		strncpy(state,npc.strField31,1024);
		dodge=        npc.dwField32;
		rating=        npc.dwField33;
		pic=        npc.dwField34;
		trait=        npc.dwField35;
		bear_type=      npc.dwField36;
		pet_pic=      npc.dwField37;
		recover.parse(npc.strField38);
		flags=        npc.dwField39;
		allyVisit=        npc.dwField40;
		Need_Probability = npc.dwField41; //sky 极品倍率
	}

	zNpcB() : zEntry()
	{
		id=          0;
		bzero(name,sizeof(name));
		kind=        0;
		level=        0;
		hp=        0;
		exp=        0;
		str=        0;
		inte=        0;
		dex=        0;
		men=        0;
		con=        0;
		cri=        0;
		color=        0;
		ai=        0;
		distance=      0;
		adistance=       0;
		pdefence=      0;
		maxpdefence=    0;
		mdefence=      0;
		maxmdefence=    0;
		five=        0;
		fivepoint=      0;
		atypelist.clear();
		mdamage=      0;
		maxmdamage=      0;
		damage=        0;
		maxdamage=      0;
		skill=        0;
		//bzero(object,sizeof(object));
		ChangeNpcID=      0;
		bzero(skills,sizeof(skills));
		bzero(state,sizeof(state));
		dodge=        0;
		rating=        0;
		pic=        0;
		trait=        0;
		bear_type=      0;
		pet_pic=      0;
		flags=        0;
		allyVisit=      0;
		Need_Probability = 0;
	}

};

//------------------------------------
// 人物经验Base
//------------------------------------
struct ExperienceBase
{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 等级
	DWORD  dwField1;    // 需要经验
};//导出 人物经验Base 成功，共 300 条记录

struct zExperienceB : public zEntry
{
	DWORD  level;        // 等级
	QWORD  nextexp;      //需要经验

	void fill(const ExperienceBase &data)
	{
		id = data.dwField0;
		//snprintf(name,MAX_NAMESIZE,"%u",id);
		_snprintf_s(name,MAX_NAMESIZE,"%u",id);
		nextexp = data.dwField1;
	}

	zExperienceB () : zEntry()
	{
		id = 0;
		nextexp = 0;
	}
};
//------------------------------------
// 荣誉增加表
//------------------------------------
struct HonorBase
{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 等级
	DWORD  dwField1;    // 需要经验
};//导出 人物经验Base 成功，共 300 条记录

struct zHonorB : public zEntry
{
	DWORD  level;        // 等级
	QWORD  value;      //需要经验

	void fill(const HonorBase &data)
	{
		id = data.dwField0;
		//snprintf(name,MAX_NAMESIZE,"%u",id);
		_snprintf_s(name,MAX_NAMESIZE,"%u",id);
		value = data.dwField1;
	}

	zHonorB () : zEntry()
	{
		id = 0;
		value = 0;
	}
};

//------------------------------------
// SkillBase
//------------------------------------
/**
* \brief 根据技能类型和等级计算一个临时唯一编号
*
*/
#define skill_hash(type,level) ((type - 1) * 100 + level)

struct SkillBase
{
	const DWORD getUniqueID() const
	{
		return skill_hash(dwField0,dwField2);
	}

	DWORD  dwField0;      // 技能ID
	char  strField1[32];    // 技能名称
	DWORD  dwField2;      // 技能等级
	DWORD  dwField3;      // 技能系别
	DWORD  dwField4;      // 技能树别
	DWORD  dwField5;      // 需要本线技能点数
	DWORD  dwField6;      // 前提技能一
	DWORD  dwField7;      // 前提技能一等级
	DWORD  dwField8;      // 前提技能二
	DWORD  dwField9;      // 前提技能二等级
	DWORD  dwField10;      // 前提技能三
	DWORD  dwField11;      // 前提技能三等级
	DWORD  dwField12;      // 间隔时间
	DWORD  dwField13;      // 攻击方式
	DWORD  dwField14;      // 能否骑马使用
	DWORD  dwField15;      // 需要物品
	char  strField16[128];  // 需要武器
	DWORD  dwField17;      // 消耗体力值
	DWORD  dwField18;      // 消耗法术值
	DWORD  dwField19;      // 消耗生命值
	DWORD  dwField20;      // 伤害加成
	char  strField21[1024];  // 效果
	DWORD  dwField22;      // 消耗物品类型
	DWORD  dwField23;      // 物品消耗数量
};//导出 SkillBase 成功，共 1 条记录

#define BENIGNED_SKILL_STATE 2
#define BAD_SKILL_STATE 4
#define NONE_SKILL_STATE 1 

struct SkillElement
{
	SkillElement()
	{
		id = 0;
		value = 0;
		percent = 0;
		time = 0;
		state = 0;
	}
	union {
		struct {
			DWORD id;
			DWORD percent;
			DWORD value;
			DWORD time;
			DWORD state;
		};
		DWORD element[5];
	};
	static SkillElement *create(SkillElement elem);
};
struct SkillStatus
{
	SkillStatus()
	{
		for(int i = 0 ; i < (int)(sizeof(status) / sizeof(WORD)) ; i ++)
		{
			status[i] = 0;
		}
	}
	union {
		struct {
			WORD id;//技能id
			WORD target;//目标
			WORD center;//中心点
			WORD range;//范围
			WORD mode;//飞行模式
			WORD clear;//能否清除
			WORD isInjure;//是否需要伤害计算
		};
		WORD status[7];
	};
	std::vector<SkillElement> _StatusElementList;
};
struct zSkillB : public zEntry
{
	bool has_needweapon(const WORD weapontype) const
	{
		std::vector<WORD>::const_iterator iter;
		if (weaponlist.empty()) return true;
		for(iter = weaponlist.begin(); iter != weaponlist.end(); iter++)
		{
			if (*iter == weapontype) return true;
		}
		return false;
	}

	bool set_weaponlist(const char *data)
	{
		weaponlist.clear(); 
		std::vector<std::string> v_fir;
		stringtok(v_fir,data,":");
		for(std::vector<std::string>::iterator iter = v_fir.begin() ; iter != v_fir.end() ; iter++)
		{
			WORD weaponkind = (WORD)atoi(iter->c_str());
			weaponlist.push_back(weaponkind);
		}
		return true;
	}

	bool set_skillState(const char *data)
	{
		skillStatus.clear(); 
		std::vector<std::string> v_fir;
		stringtok(v_fir,data,".");
		for(std::vector<std::string>::iterator iter = v_fir.begin() ; iter != v_fir.end() ; iter++)
		{
			//Zebra::logger->debug("%s",iter->c_str());
			std::vector<std::string> v_sec;
			stringtok(v_sec,iter->c_str(),":");
			/*
			if (v_sec.size() != 2)
			{
			return false;
			}
			// */
			SkillStatus status;
			std::vector<std::string>::iterator iter_1 = v_sec.begin() ;
			std::vector<std::string> v_thi;
			stringtok(v_thi,iter_1->c_str(),"-");
			if (v_thi.size() != 7)
			{
				//Zebra::logger->debug("操作!=7");
				continue;
				//return false;
			}
			std::vector<std::string>::iterator iter_2 = v_thi.begin() ;
			for(int i = 0 ; i < 7 ; i ++)
			{
				status.status[i] = (WORD)atoi(iter_2->c_str());
				//Zebra::logger->debug("status.status[%ld]=%ld",i,status.status[i]);
				iter_2 ++;
			}
			iter_1 ++;
			if (iter_1 == v_sec.end())
			{
				//Zebra::logger->debug("空操作");
				skillStatus.push_back(status);
				continue;
			}
			std::vector<std::string> v_fou;
			stringtok(v_fou,iter_1->c_str(),";");
			std::vector<std::string>::iterator iter_3 = v_fou.begin() ;
			for( ; iter_3 != v_fou.end() ; iter_3 ++)
			{
				std::vector<std::string> v_fiv;
				stringtok(v_fiv,iter_3->c_str(),"-");
				if (v_fiv.size() != 5)
				{
					//Zebra::logger->debug("元素个数不对");
					continue;
					//return false;
				}
				std::vector<std::string>::iterator iter_4 = v_fiv.begin() ;
				SkillElement element;
				for(int i = 0 ; i < 5 ; i ++)
				{
					element.element[i] = (DWORD)atoi(iter_4->c_str());
					//Zebra::logger->debug("element.element[%u]=%u",i,element.element[i]);
					iter_4 ++;
				}
				status._StatusElementList.push_back(element);
			}
			skillStatus.push_back(status);
		}
		return true;
	}
	DWORD  skillid;            //技能ID
	DWORD  level;              //技能等级
	DWORD  kind;              //技能系别
	DWORD  subkind;            //技能树别
	DWORD  needpoint;            //需要本线技能点数
	DWORD  preskill1;            //前提技能1
	DWORD  preskilllevel1;          //前提技能级别1
	DWORD  preskill2;            //前提技能2
	DWORD  preskilllevel2;          //前提技能级别2
	DWORD  preskill3;            //前提技能3
	DWORD  preskilllevel3;          //前提技能级别3
	DWORD  dtime;              //间隔时间
	DWORD  usetype;            //攻击方式
	DWORD  ride;              //可否骑马使用
	DWORD  useBook;            //需要物品
	DWORD  spcost;              //消耗体力值
	DWORD  mpcost;              //消耗法术值
	DWORD  hpcost;              //消耗生命值
	DWORD  damnum;              //伤害加成
	DWORD  objcost;            //消耗物品类型
	DWORD  objnum;              //消耗物品数量
	std::vector<SkillStatus> skillStatus;  //效果
	std::vector<WORD> weaponlist;      //武器列表



	void fill(const SkillBase &data)
	{
		id=skill_hash(data.dwField0,data.dwField2);
		skillid=data.dwField0;                //技能ID
		strncpy(name,data.strField1,MAX_NAMESIZE);
		level      = data.dwField2;          //技能等级
		kind      = data.dwField3;          //技能系别
		subkind      = data.dwField4;          //技能树别
		needpoint    = data.dwField5;          //需要本线技能点数
		preskill1    = data.dwField6;          //前提技能1
		preskilllevel1  = data.dwField7;;          //前提技能级别1
		preskill2    = data.dwField8;          //前提技能2
		preskilllevel2  = data.dwField9;          //前提技能级别2
		preskill3    = data.dwField10;          //前提技能3
		preskilllevel3  = data.dwField11;          //前提技能级别3
		dtime      = data.dwField12;          //间隔时间
		usetype      = data.dwField13;          //攻击方式
		ride      = data.dwField14;          //可否骑马使用
		useBook      = data.dwField15;          //学习需要物品
		set_weaponlist(data.strField16);          //需要武器
		spcost      = data.dwField17;          //消耗体力值
		mpcost      = data.dwField18;          //消耗法术值
		hpcost      = data.dwField19;          //消耗生命值
		damnum      = data.dwField20;          //伤害加成
		set_skillState(data.strField21);
		objcost      = data.dwField22;          //消耗物品类型
		objnum      = data.dwField23;          //消耗物品数量
	}


	zSkillB() : zEntry()
	{
		id = 0;
		skillid = 0;
		bzero(name,sizeof(name));        //说明
		level      = 0;          //技能等级
		kind      = 0;          //技能系别
		subkind      = 0;          //技能树别
		needpoint    = 0;          //需要本线技能点数
		preskill1    = 0;          //前提技能1
		preskilllevel1  = 0;          //前提技能级别1
		preskill2    = 0;          //前提技能2
		preskilllevel2  = 0;          //前提技能级别2
		preskill3    = 0;          //前提技能3
		preskilllevel3  = 0;          //前提技能级别3
		dtime      = 0;          //间隔时间
		usetype      = 0;          //攻击方式
		ride      = 0;          //可否骑马使用
		useBook      = 0;          //需要物品
		spcost      = 0;          //消耗体力值
		mpcost      = 0;          //消耗法术值
		hpcost      = 0;          //消耗生命值
		damnum      = 0;          //伤害加成
		objcost      = 0;          //消耗物品类型
		objnum      = 0;          //消耗物品数量
	}

};

struct LiveSkillBase{

	const DWORD getUniqueID() const
	{
		return ((0xffff & dwField11) << 16) | (0xffff & dwField0);
	}

	DWORD  dwField0;    // 技能ID
	char  strField1[64];    // 技能名称
	DWORD  dwField2;    // 需要工具
	DWORD  dwField3;    // 初始技能
	DWORD  dwField4;    // 对应图素
	DWORD  dwField5;    // 类别
	DWORD  dwField6;    // 技能升级经验
	DWORD  dwField7;    // 可否升级
	DWORD  dwField8;    // 进阶技能
	DWORD  dwField9;    // 前提技能ID
	DWORD  dwField10;    // 所需前提技能等级
	DWORD  dwField11;    // 技能等级
	char  strField12[32];    // 技能称号
	char  strField13[256];    // 获得物品
};

struct zLiveSkillB : public zEntry
{
	enum {
		MAX_EXP_BONUS = 30,
		MIN_POINT_BONUS = 1,
		MAX_POINT_BONUS = 3,
		WORKING_TIME = 6,
		MAX_LEVEL = 30,
	};

	//DWORD skill_id; //技能标识
	//DWORD level; //技能等级
	//WORD should be enough
	WORD skill_id; //技能标识
	WORD level; //技能等级
	DWORD point; //升级所需技能点
	DWORD weapon_kind; //武器种类
	//std::string name; //技能名称
	std::string title; //称号
	bool orig; //初始技能
	bool upgrade; //能否升级
	DWORD kind; //技能类别
	DWORD basic_skill_id; //前提技能id
	DWORD basic_skill_level; //前提技能等级]
	DWORD up_skill_id; //进阶技能id
	DWORD map_kind;

	class ITEM 
	{
	public:
		DWORD item; //获得物品
		DWORD odds;  //几率
		DWORD min_number; //最小数量
		DWORD max_number; //最大数量

		ITEM( const std::string& odds_,const std::string& item_,const std::string& number_) : item(atoi(item_.c_str())),odds(atoi(odds_.c_str())),min_number(0),max_number(0)
		{
			std::string::size_type pos = 0;
			if  ( (pos = number_.find("-")) != std::string::npos ) {

				min_number = atoi(number_.substr(0,pos).c_str());
				max_number = atoi(number_.substr(pos+strlen("-")).c_str());
			}
			//if (item) Zebra::logger->debug("劳动获得物品数据:ID(%d),几率(%d),个数(%d-%d)",item,odds,min_number,max_number);
		}
	}; 

	typedef std::vector<ITEM> ITEMS;
	ITEMS items;

	BYTE min_point_bonus; //最小增加技能点
	BYTE max_point_bonus; //最大增加技能点
	BYTE exp_bonus; //奖励经验
	BYTE max_level; //最大等级

	zLiveSkillB() : zEntry(),skill_id(0),level(0),point(0),weapon_kind(0),/*name("未知"),*/ title(""),orig(false),upgrade(false),
		kind(1),basic_skill_id(0),basic_skill_level(0),up_skill_id(0),map_kind(0),
		min_point_bonus(MIN_POINT_BONUS),max_point_bonus(MAX_POINT_BONUS),exp_bonus(MAX_EXP_BONUS),
		max_level(MAX_LEVEL)
	{

	}

	void fill(const LiveSkillBase& base)
	{
		skill_id = 0xffff & base.dwField0;
		//name = base.strField1;
		weapon_kind = base.dwField2;
		orig = (base.dwField3==1)?true:false;
		map_kind = base.dwField4;
		kind = base.dwField5;
		point = base.dwField6;
		upgrade = (base.dwField7==1)?true:false;
		up_skill_id = base.dwField8;
		basic_skill_id = base.dwField9;
		basic_skill_level = base.dwField10;
		level = 0xffff & base.dwField11;
		strncpy(name,base.strField1,MAX_NAMESIZE);
		title = base.strField12;
		init_items(base.strField13);

		id = (level << 16) | skill_id;
	}

	void init_items(const std::string& item_list)
	{
		items.clear();
		Split<Parse3> p;
		p(item_list,items,";",":");

	}

};

//------------------------------------
// SoulStoneBase
//------------------------------------
struct SoulStoneBase{
	const DWORD getUniqueID() const
	{
		return dwField2;
	}

	DWORD  dwField0;    // 编号
	char  strField1[32];    // 名称
	DWORD  dwField2;    // 品质
	char  strField3[16];    // x%吸收生命值y
	char  strField4[16];    // x%吸收法术值y
	char  strField5[16];    // 转换x%生命值为法术值减少
	char  strField6[16];    // 增加银子掉落x%
	char  strField7[16];    // x%双倍经验
	char  strField8[16];    // 增加掉宝率x%
	char  strField9[16];    // 抗毒增加
	char  strField10[16];    // 抗麻痹增加
	char  strField11[16];    // 抗眩晕增加
	char  strField12[16];    // 抗噬魔增加
	char  strField13[16];    // 抗噬力增加
	char  strField14[16];    // 抗混乱增加
	char  strField15[16];    // 抗冰冻增加
	char  strField16[16];    // 抗石化增加
	char  strField17[16];    // 抗失明增加
	char  strField18[16];    // 抗定身增加
	char  strField19[16];    // 抗减速增加
	char  strField20[16];    // 抗诱惑增加
	char  strField21[16];    // 中毒增加
	char  strField22[16];    // 麻痹增加
	char  strField23[16];    // 眩晕增加
	char  strField24[16];    // 噬魔增加
	char  strField25[16];    // 噬力增加
	char  strField26[16];    // 混乱增加
	char  strField27[16];    // 冰冻增加
	char  strField28[16];    // 石化增加
	char  strField29[16];    // 失明增加
	char  strField30[16];    // 定身增加
	char  strField31[16];    // 减速增加
	char  strField32[16];    // 诱惑增加
	DWORD  dwField33;    // 需求等级
	char  strField34[16];    // 力量
	char  strField35[16];    // 智力
	char  strField36[16];    // 敏捷
	char  strField37[16];    // 精神
	char  strField38[16];    // 体质   
};//导出 SoulStoneBase 成功，共 40 条记录



struct zSoulStoneB : public zEntry
{
	//DWORD id;
	//std::string name;

	struct Value
	{ 
		rangeValue odds; 
		rangeValue effect; 
	} hpleech,mpleech; ////x%吸收生命值y,x%吸收法术值y

	rangeValue hptomp; //转换生命值为法术值x％

	rangeValue incgold; //增加银子掉落x%
	rangeValue doublexp; //x%双倍经验    
	rangeValue mf; //增加掉宝率x%

	rangeValue poisondef; //抗毒增加
	rangeValue lulldef; //抗麻痹增加
	rangeValue reeldef; //抗眩晕增加
	rangeValue evildef; //抗噬魔增加
	rangeValue bitedef; //抗噬力增加
	rangeValue chaosdef; //抗混乱增加
	rangeValue colddef; //抗冰冻增加
	rangeValue petrifydef; //抗石化增加
	rangeValue blinddef; //抗失明增加
	rangeValue stabledef; //抗定身增加
	rangeValue slowdef; //抗减速增加
	rangeValue luredef; //抗诱惑增加

	rangeValue poison; //中毒增加
	rangeValue lull; //麻痹增加
	rangeValue reel; //眩晕增加
	rangeValue evil; //噬魔增加
	rangeValue bite; //噬力增加
	rangeValue chaos; //混乱增加
	rangeValue cold; //冰冻增加
	rangeValue petrify; //石化增加
	rangeValue blind; //失明增加
	rangeValue stable; //定身增加
	rangeValue slow; //减速增加
	rangeValue lure; //诱惑增加

	WORD level;   

	rangeValue str;      // 力量
	rangeValue inte;    // 智力
	rangeValue dex;      // 敏捷
	rangeValue spi;      // 精神
	rangeValue con;      // 体质

	zSoulStoneB() : zEntry()
	{

	}

	void fill(const SoulStoneBase& base)
	{
		id = base.dwField2;
		strncpy(name,base.strField1,MAX_NAMESIZE);

		init_value(base.strField3,hpleech);
		init_value(base.strField4,mpleech);

		fillRangeValue(base.strField5,hptomp);
		fillRangeValue(base.strField6,incgold);
		fillRangeValue(base.strField7,doublexp);
		fillRangeValue(base.strField8,mf);

		fillRangeValue(base.strField9,poisondef);
		fillRangeValue(base.strField10,lulldef);
		fillRangeValue(base.strField11,reeldef);  
		fillRangeValue(base.strField12,evildef);
		fillRangeValue(base.strField13,bitedef);
		fillRangeValue(base.strField14,chaosdef);
		fillRangeValue(base.strField15,colddef);
		fillRangeValue(base.strField16,petrifydef);
		fillRangeValue(base.strField17,blinddef);
		fillRangeValue(base.strField18,stabledef);
		fillRangeValue(base.strField19,slowdef);
		fillRangeValue(base.strField20,luredef);

		fillRangeValue(base.strField21,poison);
		fillRangeValue(base.strField22,lull);
		fillRangeValue(base.strField23,reel);  
		fillRangeValue(base.strField24,evil);
		fillRangeValue(base.strField25,bite);
		fillRangeValue(base.strField26,chaos);
		fillRangeValue(base.strField27,cold);
		fillRangeValue(base.strField28,petrify);
		fillRangeValue(base.strField29,blind);
		fillRangeValue(base.strField30,stable);
		fillRangeValue(base.strField31,slow);
		fillRangeValue(base.strField32,lure);

		level = base.dwField33;

		fillRangeValue(base.strField34,str);
		fillRangeValue(base.strField35,inte);
		fillRangeValue(base.strField36,dex);
		fillRangeValue(base.strField37,spi);
		fillRangeValue(base.strField38,con);    
	}

	void init_value(const std::string& src,Value& value)
	{
		std::string::size_type pos = 0;
		if  ( (pos = src.find(';')) != std::string::npos ) {
			fillRangeValue(src.substr(0,pos).c_str(),value.odds);
			fillRangeValue(src.substr(pos+1).c_str(),value.effect);
		}
	}

};



//------------------------------------
// HairStyle
//------------------------------------
struct HairStyle{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 编号
	char  strField1[32];    // 名称
	DWORD  dwField2;    // 性别
	DWORD  dwField3;    // 动作发型图片
	DWORD  dwField4;    // 纸娃娃发型图片
	DWORD  dwField5;    // 费用
};//导出 HairStyle 成功，共 10 条记录
struct zHairStyleB : public zEntry
{
	DWORD cost;
	zHairStyleB():zEntry()
	{
		cost=0;
	}
	void fill(const HairStyle& base)
	{
		id = base.dwField0;
		strncpy(name,base.strField1,MAX_NAMESIZE);
		cost=base.dwField5;
	}
};

//------------------------------------
// HairColour
//------------------------------------
struct HairColour{
	const DWORD getUniqueID() const
	{
		return dwField2 & 0x00FFFFFF;//发色做
	}
	DWORD  dwField0;    // 编号
	char  strField1[32];    // 名称
	DWORD  dwField2;    // 颜色
	DWORD  dwField3;    // 费用
};//导出 HairColour 成功，共 4 条记录
struct zHairColourB : public zEntry
{
	DWORD color;
	DWORD cost;
	zHairColourB() : zEntry()
	{
		color=0;
		cost=0;
	}
	void fill(const HairColour& base)
	{
		id = base.dwField2 & 0x00FFFFFF;//发色做
		strncpy(name,base.strField1,MAX_NAMESIZE);
		color=base.dwField2;
		cost=base.dwField3;
	}
};
//------------------------------------
// HeadList
//------------------------------------
struct HeadList{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // 编号
	char  strField1[16];    // 头像名
	DWORD  dwField2;    // 性别
	DWORD  dwField3;    // 头像编号
	DWORD  dwField4;    // 费用
};//导出 HeadList 成功，共 10 条记录
struct zHeadListB : public zEntry
{
	DWORD sex;
	DWORD icon;
	DWORD cost;
	zHeadListB() : zEntry()
	{
		sex=0;
		icon=0;
		cost=0;
	}
	void fill(const HeadList& base)
	{
		id = base.dwField0;
		strncpy(name,base.strField1,MAX_NAMESIZE);
		sex=base.dwField2;
		icon=base.dwField3;
		cost=base.dwField4;
	}
};


//------------------------------------
//// PetBase
////------------------------------------
struct PetBase{
	const DWORD getUniqueID() const
	{
		return dwField0;
	}
	DWORD  dwField0;    // id
	DWORD  dwField1;    // 等级
	DWORD  dwField2;    // 类型
	DWORD  dwField3;    // 经验值
	DWORD  dwField4;    // 生命值
	DWORD  dwField5;    // 物攻下限
	DWORD  dwField6;    // 物攻上限
	DWORD  dwField7;    // 魔攻下限
	DWORD  dwField8;    // 魔攻上限
	DWORD  dwField9;    // 物防
	DWORD  dwField10;    // 魔防
	DWORD  dwField11;    // 重击
	DWORD  dwField12;    // 力量
	DWORD  dwField13;    // 智力
	DWORD  dwField14;    // 敏捷
	DWORD  dwField15;    // 精神
	DWORD  dwField16;    // 体质    
};

struct zPetB : public zEntry
{
	DWORD base_id;    // id
	DWORD lv;         // 等级
	DWORD type;       // 类型
	DWORD exp;        // 经验值
	DWORD hp;         // 生命值
	DWORD atk;        // 物攻下限
	DWORD maxatk;     // 物攻上限
	DWORD matk;       // 魔攻下限
	DWORD maxmatk;    // 魔攻上限
	DWORD def;        // 物防
	DWORD mdef;       // 魔防
	DWORD cri;        // 重击
	DWORD str;        // 力量
	DWORD intel;      // 智力
	DWORD agi;        // 敏捷
	DWORD men;        // 精神
	DWORD vit;    // 体质    

	zPetB() : zEntry()
	{
		base_id  = 0;  
		lv  = 0;       
		type  = 0;     
		exp  = 0;      
		hp  = 0;       
		atk  = 0;      
		maxatk  = 0;   
		matk  = 0;     
		maxmatk  = 0;  
		def  = 0;      
		mdef  = 0;     
		cri  = 0;      
		str  = 0;      
		intel  = 0;    
		agi  = 0;      
		men  = 0;      
		vit  = 0;  
	}
	void fill(PetBase &base)
	{
		base_id  = base.dwField0;  
		id  = base.dwField0;  
		char buf[32];
		sprintf_s(buf,"%d",base.dwField0);
		strncpy(name,buf,MAX_NAMESIZE);
		lv  = base.dwField1;  
		type  = base.dwField2;  
		exp  = base.dwField3;  
		hp  = base.dwField4;  
		atk  = base.dwField5;  
		maxatk  = base.dwField6;    
		matk  = base.dwField7;  
		maxmatk  = base.dwField8;    
		def  = base.dwField9;  
		mdef  = base.dwField10;  
		cri  = base.dwField11;  
		str  = base.dwField12;  
		intel  = base.dwField13;  
		agi  = base.dwField14;  
		men  = base.dwField15;  
		vit  = base.dwField16;      
	}
};
//------------------------------------
// CountryMaterial
//------------------------------------
struct CountryMaterial{
	const DWORD getUniqueID() const
	{
		return dwField1+dwField3;
	}

	DWORD  dwField0;    // 编号
	DWORD  dwField1;    // 物品ID
	DWORD  dwField2;    // 材料类型
	DWORD  dwField3;    // 物品类别
};
struct zCountryMaterialB : public zEntry
{
	DWORD dwObjectID;
	DWORD dwMaterialKind;
	DWORD dwKind;

	zCountryMaterialB() : zEntry()
	{
		dwObjectID = 0;
		dwMaterialKind = 0;
		dwKind = 0;
	}
	void fill(const CountryMaterial& base)
	{
		id = base.dwField1+base.dwField3;

		dwObjectID = base.dwField1;
		dwMaterialKind = base.dwField2;
		dwKind = base.dwField3;
	}
};

#pragma pack()

template <class data>
class zDatabaseCallBack
{
public:
	virtual bool exec(data *entry)=0;
	virtual ~zDatabaseCallBack(){};
};
typedef zEntryManager<zEntryID,zMultiEntryName> zDataManager;
template <class data,class datafile>
class  zDataBM:public zDataManager
{

private:
	static zDataBM<data,datafile> *me;
	zRWLock rwlock;

	zDataBM()
	{
	}

	class deleteEvery:public zDatabaseCallBack<data>
	{
		bool exec(data *entry)
		{
			delete entry;
			return true;
		}
	};

	~zDataBM()
	{
		deleteEvery temp;
		execAll(temp);
		rwlock.wrlock();
		clear();
		rwlock.unlock();
	}

	zEntry * getEntryByID( DWORD id)
	{
		zEntry * ret=NULL;
		zEntryID::find(id,ret);
		return ret;
	}

	void removeEntryByID(DWORD id)
	{
		zEntry * ret=NULL;
		if (zEntryID::find(id,ret))
			removeEntry(ret);
	}

	zEntry * getEntryByName( const char * name)
	{
		zEntry * ret=NULL;
		zMultiEntryName::find(name,ret,true);
		return ret;
	}

	void removeEntryByName(const char * name)
	{
		zEntry * ret=NULL;
		if (zMultiEntryName::find(name,ret))
			removeEntry(ret);
	}

	bool refresh(datafile &base)
	{
		static DWORD id = base.getUniqueID();
		data *o=(data *)getEntryByID(base.getUniqueID());
		if (o==NULL)
		{
			o=new data();
			//fprintf(stderr,"%u",o->level);
			if (o==NULL)
			{
				Zebra::logger->fatal("无法分配内存");
				return false;
			}
			o->fill(base);
			if (!zDataManager::addEntry(o))
			{
				Zebra::logger->fatal("添加Entry错误(%ld)(id=%ld,name=%s)",base.dwField0,o->id,o->name);
				SAFE_DELETE(o);
				return false;
			}
		}
		else
		{
			o->fill(base);
			//重新调整名字hash中的位置，这样即使名称改变也可以查询到
			zMultiEntryName::remove((zEntry * &)o);
			zMultiEntryName::push((zEntry * &)o);
		}
		return true;
	}

public:
	static zDataBM & getMe()
	{
		if (me==NULL)
			me=new zDataBM();
		return *me;
	}

	static void delMe()
	{
		SAFE_DELETE(me);
	}

	bool refresh(const char *filename)
	{
		FILE* fp = fopen(filename,"rb");
		bool ret=false;
		if (fp)
		{
			DWORD size;
			datafile ob;
			bzero(&ob,sizeof(ob));
			if (fread(&size,sizeof(size),1,fp)==1)
			{
				rwlock.wrlock();
				for(DWORD i =0;i<size;i++)
				{
					if (fread(&ob,sizeof(ob),1,fp)==1)
					{
						refresh(ob);
						bzero(&ob,sizeof(ob));
					}
					else
					{
						Zebra::logger->error("读到未知大小结构，文件[%s]可能损坏",filename);
						break;
					}
					if (feof(fp)) break;
				}
				rwlock.unlock();
				ret=true;
			}
			else
			{
				Zebra::logger->error("读取记录个数失败");
			}
			fclose(fp);
		}
		else
		{
			Zebra::logger->error("打开文件[%s]失败",filename);
		}
		if (ret)
			Zebra::logger->info("刷新基本表[%s]成功",filename);
		else
			Zebra::logger->error("刷新基本表[%s]失败",filename);
		return ret;
	}

	data *get(DWORD dataid)
	{
		rwlock.rdlock();
		data *ret=(data *)getEntryByID(dataid);
		rwlock.unlock();
		return ret;
	}

	data *get(const char *name)
	{
		rwlock.rdlock();
		data *ret=(data *)getEntryByName(name);
		rwlock.unlock();
		return ret;
	}

	void execAll(zDatabaseCallBack<data> &base)
	{
		rwlock.rdlock();
		for(zEntryID::hashmap::iterator it=zEntryID::ets.begin();it!=zEntryID::ets.end();it++)
		{
			if (!base.exec((data *)it->second))
			{
				rwlock.unlock();
				return;
			}
		}
		rwlock.unlock();
	}

	void listAll()
	{
		class listevery:public zDatabaseCallBack<data>
		{
		public:
			int i;
			listevery()
			{
				i=0;
			}
			bool exec(data *zEntry)
			{
				i++;
				Zebra::logger->debug("%ld\t%s",zEntry->id,zEntry->name);
				return true;
			}
		};
		listevery le;
		execAll(le);
		Zebra::logger->debug("Total %d",le.i);
	}
};

extern zDataBM<zObjectB,ObjectBase> &objectbm;
extern zDataBM<zBlueObjectB,BlueObjectBase> &blueobjectbm;
extern zDataBM<zGoldObjectB,GoldObjectBase> &goldobjectbm;
extern zDataBM<zDropGoldObjectB,DropGoldObjectBase> &dropgoldobjectbm;
extern zDataBM<zSetObjectB,SetObjectBase> &setobjectbm;
extern zDataBM<zFiveSetB,FiveSetBase> &fivesetbm;
extern zDataBM<zHolyObjectB,HolyObjectBase> &holyobjectbm;
extern zDataBM<zUpgradeObjectB,UpgradeObjectBase> &upgradeobjectbm;
extern zDataBM<zNpcB,NpcBase> &npcbm;
//extern zDataBM<zCharacterB,CharacterBase> &characterbm;
extern zDataBM<zExperienceB,ExperienceBase> &experiencebm;
extern zDataBM<zHonorB,HonorBase> &honorbm;
extern zDataBM<zSkillB,SkillBase> &skillbm;
extern zDataBM<zLiveSkillB,LiveSkillBase> &liveskillbm;
extern zDataBM<zSoulStoneB,SoulStoneBase> &soulstonebm;
extern zDataBM<zHairStyleB,HairStyle> &hairstylebm;
extern zDataBM<zHairColourB,HairColour> &haircolourbm;
extern zDataBM<zCountryMaterialB,CountryMaterial> &countrymaterialbm;
extern zDataBM<zHeadListB,HeadList> &headlistbm;
extern zDataBM<zPetB,PetBase> &petbm;

extern bool loadAllBM();
extern void unloadAllBM();

/**
* \brief 角色管理器定义
*/
/**
* \brief 角色定义类,有待扩充
*/
struct zUser:public zSceneEntry
{
	zUser():zSceneEntry(SceneEntry_Player)
	{
	}
	void lock()
	{
		//Zebra::logger->debug("lockuser");
		mlock.lock();
	}

	void unlock()
	{
		//Zebra::logger->debug("unlockuser");
		mlock.unlock();
	}

private:
	zMutex mlock;
};


/**
* \brief 角色管理器
*
* 实现了ID、临时ID和名字的索引,所以这些值不能重复
*/
class zUserManager:public zEntryManager< zEntryID,zEntryTempID,zEntryName>
{
protected:
	/**
	* \brief 管理器访问互斥锁
	*/
	zRWLock rwlock;

public:
	/**
	* \brief 构造函数
	*/
	zUserManager()
	{
	}

	/**
	* \brief 析构函数
	*/
	virtual ~zUserManager()
	{
		clear();
	}

	/**
	* \brief 根据角色名字得到角色
	* \param name 角色名字
	* \return 角色指针,如果返回NULL表示没找到角色
	*/
	zUser * getUserByName( const char * name)
	{
		rwlock.rdlock();
		zUser *ret =(zUser *)getEntryByName(name);
		rwlock.unlock();
		return ret;
	}

	/**
	* \brief 根据角色ID得到角色
	* \param id 角色ID
	* \return 角色指针,如果返回NULL表示没找到角色
	*/
	zUser * getUserByID( DWORD id)
	{
		rwlock.rdlock();
		zUser *ret =(zUser *)getEntryByID(id);
		rwlock.unlock();
		return ret;
	}

	/**
	* \brief 根据角色临时ID得到角色
	* \param tempid 角色临时ID
	* \return 角色指针,如果返回NULL表示没找到角色
	*/
	zUser * getUserByTempID( DWORD tempid)
	{
		rwlock.rdlock();
		zUser *ret =(zUser *)getEntryByTempID(tempid);
		rwlock.unlock();
		return ret;
	}

	/**
	* \brief 添加角色
	* \param user 角色
	* \return 添加是否成功
	*/
	bool addUser(zSceneEntry *user)
	{
		rwlock.wrlock();
		//      Zebra::logger->debug("%s(%x) really insert into user manager",user->name,user);      
		bool ret =addEntry((zEntry *)user);
		rwlock.unlock();
		return ret;
	}

	/**
	* \brief 移出角色
	* \param user 角色
	*/
	void removeUser(zSceneEntry *user)
	{
		rwlock.wrlock();
		//      Zebra::logger->debug("%s(%x) really removed from user manager",user->name,user);
		removeEntry((zEntry *)user);
		rwlock.unlock();
	}

	/**
	* \brief 移出符合条件的角色
	* \param pred 条件断言
	*/
	template <class YourUserEntry>
	void removeUser_if(removeEntry_Pred<YourUserEntry> &pred)
	{
		rwlock.wrlock();
		removeEntry_if<>(pred);
		rwlock.unlock();
	}

	/**
	* \brief 对每个用户执行
	* \param exec 执行接口
	*/
	template <class YourUserEntry>
	bool execEveryUser(execEntry<YourUserEntry> &exec)
	{
		rwlock.rdlock();
		bool ret=execEveryEntry<>(exec);
		rwlock.unlock();
		return ret;
	}
};

/**
* \brief A*寻路算法
*/
/**
* \brief A*寻路算法模板
* 其中step表示步长，radius表示搜索半径
*/
template <int step = 1,int radius = 12>
class zAStar
{

private:

	/**
	* \brief 路径坐标点
	*/
	struct zPathPoint
	{
		/**
		* \brief 坐标
		*/
		zPos pos;
		/**
		* \brief 当前距离
		*/
		int cc;
		/**
		* \brief 路径上一个结点指针
		*/
		zPathPoint *father;
	};

	/**
	* \brief 路径头
	*/
	struct zPathQueue
	{
		/**
		* \brief 路径节点头指针
		*/
		zPathPoint *node;
		/**
		* \brief 路径消耗距离
		*/
		int cost;
		/**
		* \brief 构造函数
		* \param node 初始化的路径节点头指针
		* \param cost 当前消耗距离
		*/
		zPathQueue(zPathPoint *node,int cost)
		{
			this->node = node;
			this->cost = cost;
		}
		/**
		* \brief 拷贝构造函数
		* \param queue 待拷贝的源数据
		*/
		zPathQueue(const zPathQueue &queue)
		{
			node = queue.node;
			cost = queue.cost;
		}
		/**
		* \brief 赋值操作符号
		* \param queue 待赋值的源数据
		* \return 返回结构的引用
		*/
		zPathQueue & operator= (const zPathQueue &queue)
		{
			node = queue.node;
			cost = queue.cost;
			return *this;
		}
	};

	/**
	* \brief 定义所有路径的链表
	*/
	typedef std::list<zPathQueue> zPathQueueHead;
	typedef typename zPathQueueHead::iterator iterator;
	typedef typename zPathQueueHead::reference reference;

	/**
	* \brief 估价函数
	* \param midPos 中间临时坐标点
	* \param endPos 最终坐标点
	* \return 估算出的两点之间的距离
	*/
	int judge(const zPos &midPos,const zPos &endPos)
	{
		int distance = abs((long)(midPos.x - endPos.x)) + abs((long)(midPos.y - endPos.y));
		return distance;
	}

	/**
	* \brief 进入路径队列
	* \param queueHead 路径队列头
	* \param pPoint 把路径节点添加到路径中
	* \param currentCost 路径的估算距离
	*/
	void enter_queue(zPathQueueHead &queueHead,zPathPoint *pPoint,int currentCost)
	{
		zPathQueue pNew(pPoint,currentCost);
		if (!queueHead.empty())
		{
			for(iterator it = queueHead.begin(); it != queueHead.end(); it++)
			{
				//队列按cost由小到大的顺序排列
				if ((*it).cost > currentCost)
				{
					queueHead.insert(it,pNew);
					return;
				}
			}
		}
		queueHead.push_back(pNew);
	}

	/**
	* \brief 从路径链表中弹出最近距离
	* \param queueHead 路径队列头
	* \return 弹出的最近路径
	*/
	zPathPoint *exit_queue(zPathQueueHead &queueHead)
	{
		zPathPoint *ret = NULL;
		if (!queueHead.empty())
		{
			reference ref = queueHead.front();
			ret = ref.node;
			queueHead.pop_front();
		}
		return ret;
	}

public:

	/**
	* \brief 寻路过程中判断中间点是否可达目的地
	*
	*  return (scene->zPosShortRange(tempPos,destPos,radius)
	*      && (!scene->checkBlock(tempPos) //目标点可达，或者是最终目标点
	*        || tempPos == destPos));
	*
	* \param tempPos 寻路过程的中间点
	* \param destPos 目的点坐标
	* \param radius 寻路范围，超出范围的视为目的地不可达
	* \return 返回是否可到达目的地
	*/
	virtual bool moveable(const zPos &tempPos,const zPos &destPos,const int radius = radius) = 0;
	/**
	* \brief 物件向某一个方向移动
	* \param direct 方向
	* \param step 表示步长
	* \return 移动是否成功
	*/
	virtual bool move(const int direct,const int step = step) = 0;
	/**
	* \brief 使物件向某一个点移动
	* 带寻路算法的移动
	* \param srcPos 起点坐标
	* \param destPos 目的地坐标
	* \return 移动是否成功
	*/
	bool gotoFindPath(const zPos &srcPos,const zPos &destPos);
	/**
	* \brief Npc向某一个点移动
	* \param srcPos 起点坐标
	* \param destPos 目的地坐标
	* \return 移动是否成功
	*/
	bool goTo(const zPos &srcPos,const zPos &destPos);
	/**
	* \brief Npc随机向某一个方向移动
	* \param direct 随机方向
	* \return 移动是否成功
	*/
	bool shiftMove(const int direct);

};

template<int step,int radius>
bool zAStar<step,radius>::gotoFindPath(const zPos &srcPos,const zPos &destPos)
{
	//DisMap是以destPos为中心的边长为2 * radius + 1 的正方形
	const int width = (2 * radius + 1);
	const int height = (2 * radius + 1);
	const int MaxNum = width * height;
	//把所有路径距离初始化为最大值
	std::vector<int> pDisMap(MaxNum,MaxNum);
	std::vector<zPathPoint> stack(MaxNum * 8 + 1);//在堆栈中分配内存
	zPathQueueHead queueHead;

	//从开始坐标进行计算
	zPathPoint *root = &stack[MaxNum * 8];
	root->pos = srcPos;
	root->cc = 0;
	root->father = NULL;
	enter_queue(queueHead,root,root->cc + judge(root->pos,destPos));

	int Count = 0;
	//无论如何,循环超过MaxNum次则放弃
	while(Count < MaxNum)
	{
		root = exit_queue(queueHead);
		if (NULL == root)
		{
			//目标点不可达
			return false;
		}

		if (root->pos == destPos)
		{
			//找到到达目的地的路径
			break;
		}

		const zAdjust adjust[8] =
		{
			{  1 * step,0 * step  },
			{  0 * step,-1 * step  },
			{  0 * step,1 * step  },
			{  -1 * step,0 * step  },
			{  1 * step,-1 * step  },
			{  -1 * step,-1 * step  },
			{  -1 * step,1 * step  },
			{  1 * step,1 * step  }
		};
		for(int i = 0; i < 8; i++)
		{
			//分别对周围8个格点进行计算路径
			bool bCanWalk = true;
			zPos tempPos = root->pos;
			tempPos += adjust[i];

			if (moveable(tempPos,destPos))
			{
				//对路径进行回溯
				zPathPoint *p = root;
				while(p)
				{
					if (p->pos == tempPos)
					{
						//发现坐标点已经在回溯路径中，不能向前走
						bCanWalk = false;
						break;
					}
					p = p->father;
				}

				//如果路径回溯成功，表示这个点是可行走的
				if (bCanWalk)
				{
					int cost = root->cc + 1;
					int index = (tempPos.y - destPos.y + radius) * width + (tempPos.x - destPos.x + radius);
					if (index >= 0
						&& index < MaxNum
						&& cost < pDisMap[index])
					{
						//这条路径比上次计算的路径还要短，需要加入到最短路径队列中
						pDisMap[index] = cost;
						zPathPoint *pNewEntry = &stack[Count * 8 + i];
						pNewEntry->pos = tempPos;
						pNewEntry->cc = cost;
						pNewEntry->father = root;
						enter_queue(queueHead,pNewEntry,pNewEntry->cc + judge(pNewEntry->pos,destPos));
					}
				}
			}
		}

		Count++;
	}

	if (Count < MaxNum)
	{
		//最终路径在PointHead中,但只走一步
		while(root)
		{
			//倒数第二个节点
			if (root->father != NULL
				&& root->father->father == NULL)
			{
				return move(srcPos.getDirect(root->pos));
			}

			root = root->father;
		}
	}

	return false;
}

template<int step,int radius>
inline bool zAStar<step,radius>::goTo(const zPos &srcPos,const zPos &destPos)
{
	int direct = srcPos.getDirect(destPos);

	if (!move(direct)) {
		int r = randBetween(0,1);
		int deep = 0;
		while(deep < 3) {
			switch(r) {
case 0://顺时针
	direct++;
	break;
case 1://逆时针
	direct += 7;
	break;
			}
			direct %= 8;
			if (move(direct))
				return true;
			deep++;
		}
	}

	return false;
}

template<int step,int radius>
inline bool zAStar<step,radius>::shiftMove(const int direct)
{
	return move(direct);
}

/**
* \brief 正则表达式类声明
*/
/**
* \brief 正则表达式类，对regex进行了封装，对于正则表达式请参考man 7 regex.
*
* 本类支持子字符串匹配，但最多支持31个字串
*
* 本类非线程安全
*/
class zRegex
{

public :

	zRegex(const char *exp):_exp(exp){}

	bool match(const char * target);
	/*{

	boost::regex reg(_exp);
	if(boost::regex_search(std::string(target),reg))
	{
	size_t len = strlen(target) + 1;
	const char *ptr1 = strstr(target,"(");
	const char *ptr2 = strstr(ptr1+1,",");
	size_t l1 = ptr2 - ptr1;
	char *f1 = new char[l1];
	strncpy(f1,ptr1+1,l1);
	f1[l1-1] = '\0';
	first = atoi(f1);
	delete[] f1;

	const char *ptr3 = strstr(ptr2+1,")");
	l1 = ptr3 - ptr2 - 1;
	f1 = new char[l1];
	strncpy(f1,ptr2+1,l1);
	f1[l1-1] = '\0';
	second = atoi(f1);
	delete[] f1;

	return true;
	}
	return false;
	}*/

private:
	/**
	* \brief 错误信息存放处
	*/
	std::string errstr;
	/**
	* \brief 错误代码
	*/
	int errcode;
	/**
	* \brief 正则表达式句柄
	*/
	//regex_t preg;
	/**
	* \brief 要匹配的字符串 
	*/
	//std::string smatch;
	/**
	* \brief 表达式是否已编译 
	*/
	//bool compiled;
	/**
	* \brief 是否匹配 
	*/
	//bool matched;
	/**
	* \brief 子串匹配位置 
	*/
	//regmatch_t rgm[32];

	/**
	* \brief 自定义错误代码:标记错误 
	*/
	static const int REG_FLAGS;
	/**
	* \brief 自定义错误代码:未编译错误
	*/
	static const int REG_COMP;
	/**
	* \brief 自定义错误代码:未知错误
	*/
	static const int REG_UNKNOW;
	/**
	* \brief 自定义错误代码:未进行匹配错误 
	*/
	static const int REG_MATCH;
public:
	/**
	* \brief 自定义标记:支持多行匹配，默认不支持
	*/
	static const int REG_MULTILINE;
	/**
	* \brief 自定义标记:默认标记
	*/
	static const int REG_DEFAULT;
	//zRegex();
	//~zRegex();
	//bool compile(const char * regex,int flags=REG_DEFAULT);
	//bool match(const char *s);
	//std::string &getSub(std::string &s,int sub=0);
	const std::string & getError();


	unsigned int first;
	unsigned int second;


private:
	const char *_exp;
};

#pragma pack(1)

/**
* \brief 定义才对战的基本结构
*
*/
namespace DareDef
{
	const DWORD CREATE_DARE_NEED_PRICE_GOLD = 500; // 对战所扣金额
	const DWORD DARE_WINNER_GOLD = 800; // 对战胜者一方，所获金额
	const DWORD READYTIME  = 300; // 等待应战的时间，单位:秒
	const DWORD ACTIVETIME = 3600; // 对战进行时间，单位:秒
	const DWORD CREATE_UNION_CITY_DARE_NEED_PRICE_MONEY = 20000; //两锭
	const DWORD CREATE_UNION_KING_CITY_DARE_NEED_PRICE_MONEY = 50000; //五锭
	const DWORD CREATE_UNION_NEUTRAL_CITY_DARE_NEED_PRICE_MONEY = 50000; //五锭

	/// 状态描述
	extern char str_state[9][20];
	extern char str_type[7][30];
}

namespace QuizDef
{
	const DWORD READYTIME  = 300; // 等待应战的时间，单位:秒
	const DWORD ACTIVETIME = 3600; // 对战进行时间，单位:秒

	const DWORD PERSONAL_QUIZ_NEED_GOLD = 100; // 个人问答，所需银两
	enum
	{
		WORLD_QUIZ = 0, // 全区竞赛
		PERSONAL_QUIZ = 1 // 个人问答
	};

	/// 状态描述
	extern char str_state[9][30];
	extern char str_type[2][20];
}

/**
* \brief 定义NPC争夺的公共信息
*
*/
namespace NpcDareDef
{
	const DWORD CREATE_NPCDARE_NEED_ITEM = 738; // 发起对战需要的道具 地羽令

	struct NpcDareRecord {
		DWORD dwCountry;      /// 国家
		DWORD dwMapID;        /// 地图ID
		DWORD dwNpcID;        /// NPC id
		DWORD dwPosX;        /// npc的 x 坐标
		DWORD dwPosY;        /// npc的 y 坐标
		DWORD dwHoldSeptID;      /// 目前该npc的所有家族
		DWORD dwDareSeptID;      /// 目前该npc的挑战家族
		DWORD dwGold;        /// 结余税金
	};
}

#pragma pack()

void Zebra_Startup(void);

//filter begin
#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

	/*
	server启动时调用,其它的所有初始化已经完成.
	*/
	void filter_init(void);

	/*
	server每次接收到一个请求时调用,返回TRUE说明filter已经处理此请求.
	原则上filter可以改写server内部的处理流程,但是目前的设计是用来处理新增的功能的.
	*/
	BOOL filter_command(PBYTE pCmd,DWORD dwCmd);

	/*
	server退出时调用,其它的所有清除尚未开始.
	*/
	void filter_term(void);

	typedef void (*PFN_filter_init)(void);

	typedef BOOL (*PFN_filter_command)(PBYTE pCmd,DWORD dwCmd);

	typedef void (*PFN_filter_term)(void);

	typedef struct
	{
		HINSTANCE          hInstance;
		PFN_filter_init    filter_init;
		PFN_filter_command filter_command;
		PFN_filter_term    filter_term;
	}NFilterModule,*PFilterModule;

#ifdef __cplusplus
}

typedef std::vector<NFilterModule> NFilterModuleArray;

#endif //__cplusplus
//filter end

//service begin
void loadFilter(NFilterModuleArray & nFMA,PSTR szPattern);

int service_main( int argc,char *argv[] );
//service end

#endif //_INC_SRVENGINE_H_

