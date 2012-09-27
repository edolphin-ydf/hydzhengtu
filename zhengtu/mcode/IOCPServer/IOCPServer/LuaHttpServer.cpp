#include "LuaHttpServer.h"

#define HEADER_MAX_LEN	1024 * 1	// 1K
#define FILE_MAX_LEN	1024 * 100	// 100K

CLuaHttpServer::CLuaHttpServer()
{
	// init virtual path
	m_strVirtualPath = "./";

	// --------- init file type map ------
	// text file
	m_FileTypeMap["htm"] = "text/html";
	// image file
	m_FileTypeMap["html"] = "text/html";
	m_FileTypeMap["jpg"] = "image/jpeg";
	m_FileTypeMap["gif"] = "image/gif";
	m_FileTypeMap["png"] = "image/png";
}

CLuaHttpServer::~CLuaHttpServer()
{

}


void CLuaHttpServer::SetVirtualPath(string strVirtualPath)
{
	m_strVirtualPath = strVirtualPath;
}


BOOL CLuaHttpServer::LoadLocationFile(char* pBuffer, DWORD& dwSize, const char* strFilePath)
{
	assert(strFilePath != NULL);
	FILE* pFile = fopen(strFilePath, "r+b");
	if (pFile == NULL)
	{
		return FALSE;
	}

	fseek(pFile,0,SEEK_END);
	dwSize = ftell(pFile);
	fseek(pFile,0,SEEK_SET);

	if (dwSize >= FILE_MAX_LEN)
	{
		fclose(pFile);
		return FALSE;
	}

	fread(pBuffer, dwSize, 1, pFile);
	fclose(pFile);

	return TRUE;
}

int CLuaHttpServer::OnReceive(WORD nClient, char* pData, DWORD dwDataLen)
{
	string strMethod = GetMethod(pData);
	string strPathFile = GetPathFile(pData);
	string strVersion = GetHttpVersion(pData);

	string strFilePath = m_strVirtualPath + strPathFile;
	char szFileContent[FILE_MAX_LEN];
	DWORD dwFileLen = 0;
	LoadLocationFile(szFileContent, dwFileLen, strFilePath.c_str());

	char szHeader[HEADER_MAX_LEN];
	sprintf(szHeader, 
		"HTTP/1.1 %d %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %d\r\n"
		"Content-Type: %s\r\n"
		"Connection: close\r\n"
		"Connection: keep-alive\r\n\r\n",
		200, "OK",
		"LHS/1.0 Lua/5.1",
		dwFileLen,
		"text/html");
	size_t nHeaderSize = strlen(szHeader);

	char szSendBuffer[FILE_MAX_LEN+HEADER_MAX_LEN+HEADER_MAX_LEN];
	memcpy(szSendBuffer, szHeader, nHeaderSize);
	if (szFileContent > 0)
	{
		memcpy(szSendBuffer+nHeaderSize, szFileContent, dwFileLen);
	}

	SendData(nClient, szSendBuffer, (u_long)(nHeaderSize+dwFileLen));

	return 0;
}


int CLuaHttpServer::OnSend(WORD nClient, char* pData, DWORD dwDataLen)
{
	// DisconnectClient(nClient);
	return 0;
}


string CLuaHttpServer::GetMethod(string strData)
{
	size_t iPos = strData.find(' ');
	string strMethod = strData.substr(0, iPos);
	return strMethod;
}

string CLuaHttpServer::GetPathFile(string strData)
{
	size_t iPos1 = strData.find(' ');
	size_t iPos2 = strData.find(' ', iPos1+1);
	string strPathFile = strData.substr(iPos1+1, iPos2-iPos1);
	return strPathFile;
}

string CLuaHttpServer::GetHttpVersion(string strData)
{
	size_t iPos1 = strData.find("HTTP/");
	size_t iPos2 = strData.find("\r\n");
	string strVersion = strData.substr(iPos1, iPos2-iPos1);
	return strVersion;
}