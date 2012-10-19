
#include "DatabaseEnv.h"
#include "../Log.h"

#if defined(ENABLE_DATABASE_MYSQL)
#include "MySQLDatabase.h"
#endif

void Database::CleanupLibs()
{
#if defined(ENABLE_DATABASE_MYSQL)
	mysql_library_end();
#endif
}


Database* Database::CreateDatabaseInterface()
{
#if defined(ENABLE_DATABASE_MYSQL)
	return new MySQLDatabase();
#endif
}
