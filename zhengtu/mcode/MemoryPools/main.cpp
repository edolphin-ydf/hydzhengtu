

#include <stdio.h>
#include "define.h"

//#ifdef _DEBUG
//#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
//#else
//#define DEBUG_CLIENTBLOCK
//#endif
//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
//#ifdef _DEBUG
//#define new DEBUG_CLIENTBLOCK
//#endif


class test
{
public:
	int a;
	float b;
};

int main()
{
	test * a = new test();
	//CMemoryPools::Instance().DisplayMemoryList();
	delete a;
	//CMemoryPools::Instance().DisplayMemoryList();
	//CMemoryPools::release();
	getchar();
	/** @brief ¼ì²éÄÚ´æÐ¹Â© */
	_CrtDumpMemoryLeaks();
	return 0;
}