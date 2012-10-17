//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      hyd
///  Create:      2012/10/17
///
/// @file         Redis.cpp
/// @brief        Radis·â×°Àà
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

#include "Redis.h"

createFileSingleton(Redis);


Redis::Redis()
{

}

Redis::~Redis()
{

}

bool Redis::Connect( const char *ip, int port )
{
	return true;
}

bool Redis::DisConnect()
{
	return true;
}
