#include <baseLib/sockLib.h>
#include <baseLib/logLib.h>
#include <stdio.h>

#ifdef WIN32
#pragma comment(lib,"ws2_32")
#endif //WIN32

#ifdef WIN32
void sockStartup(void)
{
  WSADATA WSAData;

  WSAStartup(0x0202,&WSAData);
}

void sockCleanup(void)
{
  WSACleanup();
}
#endif //WIN32

BOOL sockTranslateAddr(PSTR szHost,BOOL bAny,PIN_ADDR addr)
{
  int            i,j;
  char           szLocal[32];
  struct hostent *phe;

  if (NULL == szHost){
    gethostname(szLocal,sizeof(szLocal));
    szHost = szLocal;
  }
  for(i=j=0;0!=szHost[i];i++){
    if ('.' == szHost[i]){
	  j++;
	  continue;
	}
	if (!isdigit(szHost[i])) break;
  }
  if (0==szHost[i] && 3==j){
    addr->s_addr = inet_addr(szHost);
	return TRUE;
  }
  else{
  	if (NULL == (phe=gethostbyname(szHost))) return FALSE;
	for(i=0;;i++){
	  if (NULL == phe->h_addr_list[i]) return FALSE;
	  if (!bAny){
	    if (*(PBYTE)phe->h_addr_list[i] == 127) continue;
	  }
      memcpy(addr,phe->h_addr_list[i],phe->h_length);
	  break;
	}
  }
  return TRUE;
}

int sockIOConditionEx(SOCKET sock,struct timeval *tv)
{
  fd_set fds;
  
  FD_ZERO(&fds);
  FD_SET(sock,&fds);
#ifdef WIN32
  return select(0,&fds,NULL,NULL,tv);
#else //!WIN32
  return select(sock+1,&fds,NULL,NULL,tv);
#endif //!WIN32
}

int sockIOCondition(SOCKET sock,DWORD dwMS)
{
  struct timeval tv;

  if (-1 == dwMS) return sockIOConditionEx(sock,NULL);
  memset(&tv,0,sizeof(tv));
  tv.tv_sec  = dwMS / 1000;
  tv.tv_usec = dwMS % 1000;
  return sockIOConditionEx(sock,&tv);
}

BOOL sockCanPeek(SOCKET sock,DWORD dwMS)
{
  return 0 != sockIOCondition(sock,dwMS);
}

BOOL sockPokeData(SOCKET sock,PVOID pIOB,int cbIOB)
{
  int size;

  if (-1 == cbIOB) cbIOB = strlen(pIOB); 
  for(;0!=cbIOB;){
    if ((size=send(sock,pIOB,cbIOB,0)) <= 0){
#ifdef WIN32
	  logMessage1("!send %d",WSAGetLastError());
#else //!WIN32
      logError0("!send");
#endif //!WIN32
	  return FALSE;
	}
	((PSTR)pIOB) += size;
	cbIOB -= size;
  }
  return TRUE;
}

BOOL sockPokeDataTo(SOCKET sock,PVOID pIOB,int cbIOB,PSOCKADDR addr,int size)
{
  if (-1 == cbIOB) cbIOB = strlen(pIOB); 
  return cbIOB == sendto(sock,pIOB,cbIOB,0,addr,size);
}

BOOL sockPeekData(SOCKET sock,PVOID pIOBi,int cbIOB,PINT pcbIOB,DWORD dwMS)
{
  PBYTE pIOB;
  int   size;

  pIOB = pIOBi;
  if (NULL != pcbIOB) *pcbIOB = 0;
  for(;0!=cbIOB;){
    if (!sockCanPeek(sock,dwMS)){
#ifdef WIN32
	  logMessage1("!sockCanPeek %d",WSAGetLastError());
#else //!WIN32
      logError0("!sockCanPeek");
#endif //!WIN32
	  return FALSE;
    }
    if ((size=recv(sock,pIOB,cbIOB,0)) <= 0){
	  if (0 != size)
#ifdef WIN32
	  logMessage2("!recv %d %d",size,WSAGetLastError());
#else //!WIN32
      logError0("!recv");
#endif //WIN32
	  return FALSE;
	}
	pIOB  += size;
	cbIOB -= size;
    if (NULL != pcbIOB){
      *pcbIOB = +size;
      break;
    }
  }
  return TRUE;
}

BOOL sockListen(SOCKET *psock,short sPort,PSTR szHost,int nWait)
{
  SOCKADDR_IN remote;
  
  if (!sockBind(sPort,szHost,&remote)) return FALSE;
  *psock = socket(AF_INET,-1 != nWait ? SOCK_STREAM : SOCK_DGRAM,0);
#ifdef WIN32
  if (INVALID_SOCKET == *psock) return FALSE;
#else //!WIN32
  if (*psock < 0) return FALSE;
#endif //!WIN32
  if (0 != bind(*psock,(PSOCKADDR)&remote,sizeof(remote))){
#ifdef WIN32
    logMessage1("!bind %d",WSAGetLastError());
#else //!WIN32
    logError0("!bind");
#endif //!WIN32
	sockClose(*psock);
	return FALSE;
  }
  if (-1 != nWait){
    if (0 != listen(*psock,nWait)){
#ifdef WIN32
      logMessage1("!listen %d",WSAGetLastError());
#else //!WIN32
      logError0("!listen");
#endif //!WIN32
      sockClose(*psock);
	  return FALSE;
    }
  }
  return TRUE;
}

BOOL sockConnect(SOCKET *psock,short sPort,PSTR szHost,BOOL bStream)
{
  SOCKADDR_IN remote;

  if (!sockBind(sPort,szHost,&remote)) return FALSE;
  *psock = socket(AF_INET,bStream?SOCK_STREAM:SOCK_DGRAM,0);
#ifdef WIN32
  if (INVALID_SOCKET == *psock) return FALSE;
#else //!WIN32
  if (*psock < 0) return FALSE;
#endif //!WIN32
  if (0 != connect(*psock,(PSOCKADDR)&remote,sizeof(remote))){
#ifdef WIN32
    logMessage3("!sockConnect(%s,%d) %d",szHost,sPort,WSAGetLastError());
#else //!WIN32
    logError2("!sockConnect(%s,%d)",szHost,sPort);
#endif //!WIN32
    sockClose(*psock);
	return FALSE;
  }
  return TRUE;
}

BOOL sockBind(short sPort,PSTR szHost,PSOCKADDR_IN remote)
{
  memset(remote,0,sizeof(*remote));
  remote->sin_family = PF_INET;
  remote->sin_port   = htons(sPort);
  if (NULL == szHost){
    remote->sin_addr.s_addr = INADDR_ANY;
	return TRUE;
  }
  else{
    return sockTranslateAddr(szHost,TRUE,&remote->sin_addr);
  }
}

BOOL sockAccept(SOCKET sock,SOCKET *sockIn,DWORD dwMS,PSOCKADDR addr,PINT size)
{
  if (-1 != dwMS){
    if (1 != sockIOCondition(sock,dwMS)) return FALSE;
  }
#ifdef WIN32
  if (INVALID_SOCKET == (*sockIn=accept(sock,addr,size))) return FALSE;
#else //!WIN32
  if ((*sockIn=accept(sock,addr,size)) < 0) return FALSE;
#endif //!WIN32
  return TRUE;
}

void sockClose(SOCKET sock)
{
#ifdef WIN32
  closesocket(sock);
#else //!WIN32
  close(sock);
#endif //!WIN32
}

BOOL sockIsLocalAddr(PSTR szIP)
{
  if (0 == szIP[0]) FALSE;
  return 127 == atoi(szIP);
}

void sockGetLocalAddr(PSTR szIP,int size)
{
  int      i;
  char     szHost[32];
  PHOSTENT phe;

  gethostname(szHost,sizeof(szHost));
  if (NULL == (phe=gethostbyname(szHost))) return;
  for(i=0;;i++){
    if (NULL == phe->h_addr_list[i]) return;
    if (*(PBYTE)phe->h_addr_list[i] == 127) continue;
    strncpy(szIP,inet_ntoa(*(IN_ADDR*)phe->h_addr_list[i]),sizeof(szIP));
    break;
  }
}

void sockFixLocalAddr(PSTR szIP,int size)
{
  if (!sockIsLocalAddr(szIP)) return;
  sockGetLocalAddr(szIP,size);
}
