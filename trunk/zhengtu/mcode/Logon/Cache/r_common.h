//////////////////////////////////////////////////
/// @file : r_common.h
/// @brief : 
/// @date:  2012/10/17
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __r_common_H__
#define __r_common_H__

#include "Common.h"
#include <Network/Network.h>
#include "shared/Log.h"
#include "shared/Config/ConfigEnv.h"
#include "shared/Database/DatabaseEnv.h"

#include "Redis.h"

#include "CacheSocket.h"

#include "global.h"

#include "pbPsrser.h"

extern Database* sCacheSQL;
extern RedisClient* sCacheRedis;
extern pbPsrser* sCachepb;
extern set<CacheSocket*> _cacheSockets;
extern Mutex _cacheSocketLock;

#endif