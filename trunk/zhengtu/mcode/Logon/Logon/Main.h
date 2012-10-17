#include "shared/Singleton.h"

enum RealmFlags
{
    REALM_FLAG_NONE         = 0x00,
    REALM_FLAG_INVALID      = 0x01,
    REALM_FLAG_OFFLINE      = 0x02,
    REALM_FLAG_SPECIFYBUILD = 0x04,                         // client will show realm version in RealmList screen in form "RealmName (major.minor.revision.build)"
    REALM_FLAG_UNK1         = 0x08,
    REALM_FLAG_UNK2         = 0x10,
    REALM_FLAG_NEW_PLAYERS  = 0x20,
    REALM_FLAG_RECOMMENDED  = 0x40,
    REALM_FLAG_FULL         = 0x80
};

extern MNet::Threading::AtomicBoolean mrunning;
class AuthSocket;
extern set<AuthSocket*> _authSockets;
extern Mutex _authSocketLock;

struct AllowedIP
{
	unsigned int IP;
	unsigned char Bytes;
};

class LogonServer;
class LogonServer : public Singleton< LogonServer >
{
	public:
		void CheckForDeadSockets();
		void Run(int argc, char** argv);
		void Stop();
		uint32 max_build;
		uint32 min_build;
		uint8 sql_hash[20];

		MNet::PerformanceCounter perfcounter;
	private:
		bool m_stopEvent;
};
