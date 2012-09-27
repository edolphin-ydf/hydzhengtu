#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_
#include "IOCPServer.h"
#include <assert.h>
#include <string>
#include <map>
using namespace std;


class CLuaHttpServer :
	public CIOCPServer
{
public:
	CLuaHttpServer(void);
public:
	virtual ~CLuaHttpServer(void);

	virtual int OnAccept(WORD nClient){return 0;};
	virtual int OnClose(WORD nClient){return 0;};
	virtual int OnError(WORD nClient, int iError){return 0;};


	virtual int OnSend(WORD nClient, char* pData, DWORD dwDataLen);
	virtual int OnReceive(WORD nClient, char* pData, DWORD dwDataLen);

public:
	void SetVirtualPath(string strVirtualPath);

private:
	string GetMethod(string strData);
	string GetPathFile(string strData);
	string GetHttpVersion(string strData);

	//BOOL LoadHtmlFile(char* pBuffer, DWORD& dwSize, string strFilePath);
	BOOL LoadLocationFile(char* pBuffer, DWORD& dwSize, const char* strFilePath);

private:
	string m_strVirtualPath;
	map <string, string>m_FileTypeMap;
};


#endif