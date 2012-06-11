#include "wsSQL.h"
#include "stdio.h"
#include <zebra/srvEngine.h>
using namespace Zebra;
/*
SQL句柄申请函数
*/
SQLHDBC wsSQLInit()
{
	SQLHENV henv;
	SQLHDBC hdbc;

	if ( SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv ) != SQL_SUCCESS )
		return NULL;

	if ( SQLSetEnvAttr( henv, SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER ) != SQL_SUCCESS ) 
	{
		SQLFreeHandle( SQL_HANDLE_ENV, henv );
		return NULL;
	}
	if ( SQLAllocHandle( SQL_HANDLE_DBC, henv, (SQLHDBC FAR *)&hdbc ) != SQL_SUCCESS )
	{
		SQLFreeHandle( SQL_HANDLE_ENV, henv );
		SQLFreeHandle( SQL_HANDLE_DBC, hdbc );
		return NULL;
	}

	return hdbc;
}
/*
SQL释放句柄
*/
void wsSQLRelease(SQLHDBC& hdbc)
{
	if(hdbc == NULL) return;

	SQLFreeHandle( SQL_HANDLE_DBC, hdbc );
	delete hdbc;
	hdbc = NULL;
}
/*
SQL连接数据库函数
*/
bool wsSQLConnet(SQLHDBC hdbc, 
				 const char *host,
				 const char *user,
				 const char *passwd,
				 const char *db,
				 unsigned int port)
{
	SQLRETURN nResult = SQLConnect( hdbc, (SQLCHAR*)db, SQL_NTS, (SQLCHAR*)user, SQL_NTS,(SQLCHAR*)passwd, SQL_NTS );
	if ( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO ) 
	{
		SQLFreeHandle( SQL_HANDLE_DBC, hdbc );
		return false;
	}
	return true;
}
/*
SQL断开连接函数
*/
bool wsSQLDisConnet(SQLHDBC& hdbc)
{
	SQLRETURN ret =	SQLDisconnect(hdbc);
	if( ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		return false;
	}
	return true;
}

/*
SQL错误信息输出函数
*/
char* wsSQLErrorMsg(SQLHANDLE hstmt)
{
	SQLCHAR			SqlState[256], Msg[1024];
	SQLINTEGER		NativeError;
	SQLSMALLINT		i, MsgLen;
	SQLRETURN		rc2;
	i = 1;
	char* tmp = new char[1024];
	int len = 0;
	memset(tmp, 0, 1024);
	while ((rc2 = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, i++, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen)) != SQL_NO_DATA)
	{
		if( MsgLen > 256 ) Msg[256] = '\0';		
		if( len + strlen((const char*)Msg ) >= 1024) break;
		if( i > 10 ) break;
		if( strlen((const char*)Msg ) < 1) continue;
		strcat( tmp, (const char*)Msg );
		strcat( tmp,"\n" );
		len += strlen( (const char*)Msg ) + 1;
		Msg[0] = '\0';
	}
	return tmp;
}

SQLHSTMT wsSQLExeSql(SQLHDBC hdbc, const char* sql,  int sqllen, SQLHSTMT ht, BINDDATA* pBind)
{
	char* p = NULL;
	while((p = (char*)strstr((char*)sql, "`")) != NULL)
	{
		for(int i = 0;i < strlen(p); i++)
		{
			p[i] = p[i + 1];
		}
	}
	SQLHSTMT hstmt;
	SQLRETURN retcode;

	if(ht == 0)
		retcode = SQLAllocHandle((SQLSMALLINT)SQL_HANDLE_STMT, hdbc, &hstmt);
	else
	{
		retcode = SQLFreeStmt(ht, SQL_CLOSE);
		hstmt = ht;
	}
	if (retcode == SQL_SUCCESS)
	{
		SQLPrepare( hstmt, (SQLCHAR*)sql, SQL_NTS );
		// [ranqd] 如果存在绑定数据
		if( pBind )
		{
			int i = 1;
			for(BINDDATAIT it = pBind->begin();it != pBind->end(); it++)
			{
				SDWORD maxLen = (*it).len;
				SQLRETURN ret = SQLBindParameter( hstmt, (SQLUSMALLINT)i ++, SQL_PARAM_INPUT, 
					SQL_C_BINARY, SQL_VARBINARY, sizeof((*it).pData), 0, (*it).pData, 0, &maxLen );

				if( ret != SQL_SUCCESS)
				{
					logger->error("绑定二进制数据失败！\n %s", sql, wsSQLErrorMsg(hstmt));
				}
			}
		}
		retcode = SQLExecute( hstmt );
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA) 
		{
			return hstmt;
		}
		else
		{
			Zebra::logger->error(wsSQLErrorMsg(hstmt));
			SQLFreeStmt( hstmt, SQL_DROP );
			SQLFreeHandle( SQL_HANDLE_STMT, &hstmt );
		}
	}
	return NULL;
}

/*
SQL获取上次插入操作的ID
*/
DWORD wsSQLGetInsertId(SQLHDBC hdbc, SQLHSTMT ht)
{
	DWORD ret = (DWORD)1;
	SQLRETURN retcode;
	DWORD IndexInd;
	char sql[1024];
	sprintf(sql, "SELECT @@IDENTITY AS [INDEX]");
	SQLHSTMT hstmt = wsSQLExeSql( hdbc, sql, sizeof(sql), ht);
	if( hstmt == NULL)
	{
		return -1;
	}
	else
	{
		if( (retcode = SQLFetch(hstmt)) != SQL_NO_DATA)
		{
			retcode = SQLGetData( hstmt, 1, SQL_C_ULONG, (SQLPOINTER)&ret, 0,(SQLLEN*)&IndexInd);
		}
	}
	SQLFreeStmt( hstmt, SQL_DROP );
	SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
	if( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		return -1;
	return ret;
}
/*
SQL替换字符串，确保数据库安全
*/
void wsSQLReplaceStr(char *to,const char *from,
					 unsigned long length)
{
	// 暂时还不知道该替换些啥，先原样返回就好了
	to[0] = '\0';
	//memcpy( to, from, length,length );
	to[length] = '\0';
}