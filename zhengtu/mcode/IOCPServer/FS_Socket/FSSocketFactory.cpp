#include "FS_DLL.h"
#include ".\fssocketfactory.h"
#include "SocketServer.h"
#include "SocketClient.h"

FSSocketFactory::FSSocketFactory(void)
{
}

FSSocketFactory::~FSSocketFactory(void)
{
}

FSServer* FSSocketFactory::CreateServer()
{
	return new CSocketServer;
}


FSClient* FSSocketFactory::CreateClient()
{
	return new CSocketClient;
}

