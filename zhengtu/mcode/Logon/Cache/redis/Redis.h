//////////////////////////////////////////////////
/// @file : Redis.h
/// @brief : Radis·â×°Àà
/// @date:  2012/10/17
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __Redis_H__
#define __Redis_H__
#include "Common.h"
#include "Singleton.h"

class Redis : public Singleton< Redis >
{
public:
	Redis();
	~Redis();
public:
	
	bool Connect(const char *ahost, int aport);
	bool DisConnect();

	const char * getValue(int key);
private:
	string host;
	int port;
};

#define sRedis Redis::getSingleton()
#endif