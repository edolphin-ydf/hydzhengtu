#include <zebra/srvEngine.h>
#include <SQLEXT.h>
typedef struct BDATA{
	DWORD len;
	char pData[5000];
};

typedef std::vector<BDATA> BINDDATA;
typedef std::vector<BDATA>::iterator BINDDATAIT;

SQLHDBC wsSQLInit();

bool wsSQLConnet(SQLHDBC hdbc, 
				 const char *host,
				 const char *user,
				 const char *passwd,
				 const char *db,
				 unsigned int port);

bool wsSQLDisConnet(SQLHDBC& hdbc);

void wsSQLRelease(SQLHDBC& hdbc);

char* wsSQLErrorMsg(SQLHANDLE hstmt);

SQLHSTMT wsSQLExeSql(SQLHDBC hdbc,const char* sql,  int sqllen, SQLHSTMT ht = 0, BINDDATA* pBind = NULL);

DWORD wsSQLGetInsertId(SQLHDBC hdbc, SQLHSTMT ht);

void wsSQLReplaceStr(char *to,const char *from,
					 unsigned long length);
