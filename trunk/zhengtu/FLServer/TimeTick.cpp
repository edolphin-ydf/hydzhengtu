/**
* \brief 时间回调函数
*
* 
*/

#include "FLServer.h"

FLTimeTick *FLTimeTick::instance = NULL;

struct LoginTimeout : public LoginManager::LoginTaskCallback
{  
	const zTime &ct;
	LoginTimeout(const zTime &ct) : ct(ct) {}
	void exec(LoginTask *lt)
	{
		if (lt->timeout(ct))
			lt->Terminate();
	}
};

void FLTimeTick::run()
{
	Zebra::logger->debug("FLTimeTick::run()");
	while(!isFinal())
	{
		zThread::sleep(1);

		zTime ct;
		InfoClientManager::getInstance().timeAction(ct);
		if (ct.sec() % 10 == 0)
		{
			LoginTimeout cb(ct);
			LoginManager::getInstance().execAll(cb);
		}
	}
}

