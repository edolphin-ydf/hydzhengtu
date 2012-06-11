#ifndef _INC_SOCKLIB_
#define _INC_SOCKLIB_

#include <baseLib/platForm.h>

#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif //HAVE_WINSOCK_H

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif //HAVE_SYS_SOCKET_H

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif //HAVE_NETINET_IN_H

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif //HAVE_ARPA_INET_H

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif //HAVE_NETDB_H

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif //HAVE_SYS_TIME_H

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif //HAVE_SYS_WAIT_H

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif //HAVE_SYS_STAT_H

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#ifndef WIN32
typedef int                     SOCKET;
typedef struct in_addr          IN_ADDR;
typedef struct in_addr          *PIN_ADDR;
typedef struct sockaddr         SOCKADDR;
typedef struct sockaddr         *PSOCKADDR;
typedef struct sockaddr_in      SOCKADDR_IN;
typedef struct sockaddr_in      *PSOCKADDR_IN;
typedef struct hostent          *PHOSTENT;
#define INVALID_SOCKET			-1
#endif //WIN32

#ifdef WIN32
void sockStartup(void);
void sockCleanup(void);
#else //!WIN32
#define sockStartup()
#define sockCleanup()
#endif //!WIN32

BOOL sockTranslateAddr(PSTR szHost,BOOL bAny,PIN_ADDR addr);

int sockIOCondition(SOCKET sock,DWORD dwMS);
int sockIOConditionEx(SOCKET sock,struct timeval *tv);

BOOL sockCanPeek(SOCKET sock,DWORD dwMS);

BOOL sockPokeData(SOCKET sock,PVOID ptIOB,int cbIOB);

BOOL sockPeekData(SOCKET sock,PVOID ptIOB,int cbIOB,PINT pcbIOB,DWORD dwMS);

BOOL sockPokeDataTo(SOCKET sock,PVOID pvData,int cbData,PSOCKADDR addr,int size);
BOOL sockPeekDataFrom(SOCKET sock,PVOID pvData,PINT pcbData,PSOCKADDR addr,PINT size);

BOOL sockBind(short sPort,PSTR szHost,PSOCKADDR_IN remote);
BOOL sockConnect(SOCKET *sock,short sPort,PSTR szHost,BOOL bStream);
BOOL sockListen(SOCKET *sock,short sPort,PSTR szHost,int nWait);

BOOL sockAccept(SOCKET sock,SOCKET *sockIn,DWORD dwMS,PSOCKADDR addr,PINT size);

void sockClose(SOCKET sock);

BOOL sockIsLocalAddr(PSTR szIP);

void sockGetLocalAddr(PSTR szIP,int size);
void sockFixLocalAddr(PSTR szIP,int size);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_SOCKLIB_
