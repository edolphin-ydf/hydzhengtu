//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      hyd
///  Create:      2012/10/17
///
/// @file         Redis.cpp
/// @brief        Radis封装类
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

#include "Redis.h"
#include "hiredis.h"
/*
 * @defgroup 测试redis
 * @{
 */
 
//RedisClient ge("192.168.1.116");
//int re = ge.exists_key("kalle");
//
//REDIS_TYPE atype = ge.type_key("kalle");
//
//ge.get_keys("kalle");
//
//ge.random_key();
//
//ge.rename_key("kalle","kall");
//
//ge.renamenx("kall","kalle");
//
//ge.dbsize();
//
//ge.expire_key("kalle",1000);
//
//ge.ttl_key("kalle");
//
//ge.select_index(1);
//
//ge.move("kalle",2);

/** @} */ // 模块结尾

bool RedisClient::Connect()
{
	__redis = redisConnect(m_host.c_str(),m_port);
	if (__redis->err)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void RedisClient::Close()
{
	if (__redis)
	{
		redisFree(__redis);
	}
}

void RedisClient::DelReply()
{
	if (__reply)
	{ 
		freeReplyObject(__reply);
		__reply=NULL;
	}
}

int RedisClient::exists_key( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"EXISTS %s",key);

	return (int)(int)__reply->integer;
}

int RedisClient::delete_key( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"DEL %s",key);

	return (int)(int)__reply->integer;
}

REDIS_TYPE RedisClient::type_key( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"TYPE %s",key);
	REDIS_TYPE rc;
	if (__reply->len>0) {
		char *t = __reply->str;
		if (!strcmp("string", t))
			rc = REDIS_TYPE_STRING;
		else if (!strcmp("list", t))
			rc = REDIS_TYPE_LIST;
		else if (!strcmp("set", t))
			rc = REDIS_TYPE_SET;
		else
			rc = REDIS_TYPE_NONE;
	}

	return rc;
}

int RedisClient::get_keys( const char *pattern)
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"KEYS %s",pattern);

    return __reply->elements;
}

int RedisClient::random_key()
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"RANDOMKEY");

	return __reply->len;
}

int RedisClient::rename_key( const char *key, const char *new_key_name )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"RENAME %s %s",key,new_key_name);

	return __reply->len;
}

int RedisClient::renamenx( const char *key, const char *new_key_name )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"RENAMENX %s %s",key,new_key_name);

	return (int)__reply->integer;
}

int RedisClient::dbsize()
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"DBSIZE");

	return (int)__reply->integer;
}

int RedisClient::expire_key( const char *key, int secs )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"EXPIRE %s %d",key,secs);

	return (int)__reply->integer;
}

int RedisClient::ttl_key( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"TTL %s",key);

	return (int)__reply->integer;
}

int RedisClient::select_index( int index )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SELECT %d",index);
	int rc = 0;
	if (__reply->len > 0)
		rc = 1;

	return rc;
}

int RedisClient::move( const char *key, int index )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"MOVE %s %d",key,index);

	return (int)__reply->integer;
}

int RedisClient::flushdb()
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"FLUSHDB");
	int rc = 0;
	if (__reply->len>0)
		rc = 1;
	
	return rc;
}

int RedisClient::flushall()
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"FLUSHALL");
	int rc = 0;
	if (__reply->len>0)
		rc = 1;

	return rc;
}

int RedisClient::set( const char *key, const char *val )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SET %s %s",key,val);
	int rc = 0;
	if (__reply->len > 0)
		rc = 1;

	return rc;
}

int RedisClient::get( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"GET %s",key);
	int rc = 0;
	if (__reply->len > 0)
		rc = 1;

	return rc;
}

int RedisClient::getset( const char *key, const char *set_val, char **get_val )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"GETSET %s",key);
	int rc = 0;
	if (__reply->len > 0)
		rc = 1;

	return rc;
}

int RedisClient::mget( std::vector<char *> keys )
{
	DelReply();
	redisAppendCommand(__redis,"MGET");
	int size = keys.size();
	for (int i=0;i<size;i++)
	{
		redisAppendCommand(__redis," %s",keys[i]);
	}

	if (redisGetReply(__redis,(void **)&__reply) != REDIS_OK)
		return 0;
	int rc = 0;
	if (__reply->len > 0)
		rc = 1;

	return rc;
}

int RedisClient::setnx( const char *key, const char *val )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SETNX %s %s",key,val);

	return (int)__reply->integer;
}

int RedisClient::incr( const char *key )
{
	DelReply();
	__reply = cr_incr(__redis,1,0,key);

	return (int)__reply->integer;
}

int RedisClient::incrby( const char *key, int incr_val )
{
	DelReply();
	__reply = cr_incr(__redis,incr_val,0,key);

	return (int)__reply->integer;
}

int RedisClient::decr( const char *key )
{
	DelReply();
	__reply = cr_incr(__redis,0,1,key);

	return (int)__reply->integer;
}

int RedisClient::decrby( const char *key, int decr_val )
{
	DelReply();
	__reply = cr_incr(__redis,0,decr_val,key);

	return (int)__reply->integer;
}

int RedisClient::append( const char *key, const char *val )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"APPEND %s %s",key,val);

	return (int)__reply->integer;
}

int RedisClient::substr( const char *key, int start, int end)
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SUBSTR %s %d %d",key,start,end);

	return __reply->len;
}

int RedisClient::list_rpush( const char *key, const char *element )
{
	DelReply();
	__reply = cr_push(__redis, 0, key, element);
	return __reply->len;
}

int RedisClient::list_lpush( const char *key, const char *element )
{
	DelReply();
	__reply = cr_push(__redis, 1, key, element);
	return __reply->len;
}

int RedisClient::list_llen( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"LLEN %s",key);

	return (int)__reply->integer;
}

int RedisClient::list_lrange( const char *key, int start, int range )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"LRANGE %s %d %d",key, start, range);

	return __reply->elements;
}

int RedisClient::list_ltrim( const char *key, int start, int end )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"LTRIM %s %d %d",key, start, end);

	return (int)__reply->integer;
}

int RedisClient::list_lindex( const char *key, int index )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"LINDEX %s %d",key, index);

	return __reply->elements;
}

int RedisClient::list_lset( const char *key, int index, const char *element )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"LSET %s %d %s",key, index, element);

	return __reply->len;
}

int RedisClient::list_lrem( const char *key, int count, const char *element )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"LREM %s %d %s",key, count, element);

	return (int)__reply->integer;
}

int RedisClient::list_lpop( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"LPOP %s",key);

	return (int)__reply->integer;
}

int RedisClient::list_rpop( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"RPOP %s",key);

	return (int)__reply->integer;
}

int RedisClient::set_sadd( const char *key, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SADD %s %s", key, member);

	return (int)__reply->integer;
}

int RedisClient::set_srem( const char *key, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SREM %s %s",key , member);

	return (int)__reply->integer;
}

int RedisClient::set_spop( const char *key, char **member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SPOP %s", key);

	return __reply->elements;
}

int RedisClient::set_smove( const char *sourcekey, const char *destkey, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SMOVE %s %s %s", sourcekey, destkey, member);

	return (int)__reply->integer;
}

int RedisClient::set_scard( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SCARD %s",key);

	return (int)__reply->integer;
}

int RedisClient::set_sismember( const char *key, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SISMEMBER %s %d %s",key, member);

	return (int)__reply->integer;
}

int RedisClient::set_sinter( int keyc, const char **keyv, char ***members )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"SINTER %s %d %s",key, index, element);

	return __reply->len;
}

int RedisClient::set_sinterstore( const char *destkey, int keyc, const char **keyv )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"SINTERSTORE %s %d %s",key, index, element);

	return __reply->len;
}

int RedisClient::set_sunion( int keyc, const char **keyv, char ***members )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"SUNION %s %d %s",key, index, element);

	return __reply->len;
}

int RedisClient::set_sunionstore( const char *destkey, int keyc, const char **keyv )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"SUNIONSTORE %s %d %s",key, index, element);

	return __reply->len;
}

int RedisClient::set_sdiff( int keyc, const char **keyv, char ***members )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"SDIFF %s %d %s",key, index, element);

	return __reply->len;
}

int RedisClient::set_sdiffstore( const char *destkey, int keyc, const char **keyv )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"SDIFFSTORE %s %d %s",key, index, element);

	return __reply->len;
}

int RedisClient::set_smembers( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SMEMBERS %s",key);

	return __reply->elements;
}

int RedisClient::zsets_zadd( const char *key, double score, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZADD %f %s",key, score, member);

	return (int)__reply->integer;
}

int RedisClient::zsets_zrem( const char *key, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZREM %s %d %s",key, member);

	return (int)__reply->integer;
}

int RedisClient::zsets_zincrby( const char *key, double incr_score, const char *member, double *new_score )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZINCRBY %s %f %s",key, incr_score, member);

	return (int)__reply->integer;
}

int RedisClient::zsets_zrank( const char *key, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZRANK %s %s",key, member);

	return (int)__reply->integer;
}

int RedisClient::zsets_zrevrank( const char *key, const char *member )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZREVRANK %s %s",key, member);

	return (int)__reply->integer;
}

int RedisClient::zsets_zrange( const char *key, int start, int end )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZRANGE %s %d %d",key, start, end);

	return __reply->elements;
}

int RedisClient::zsets_zrevrange( const char *key, int start, int end )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZREVRANGE %s %d %d",key, start, end);

	return __reply->elements;
}

int RedisClient::zsets_zcard( const char *key )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZCARD %s",key);

	return (int)__reply->integer;
}

int RedisClient::zsets_zscore( const char *key, const char *member, double *score )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZSCORE %s %s",key, member);

	return (int)__reply->integer;
}

int RedisClient::zsets_zremrangebyscore( const char *key, double min, double max )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZREMRANGEBYSCORE %s %f %f",key, min, max);

	return (int)__reply->integer;
}

int RedisClient::zsets_zremrangebyrank( const char *key, int start, int end )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"ZREMRANGEBYRANK %s %d %d",key, start, end);

	return (int)__reply->integer;
}

int RedisClient::zsets_zinterstore( const char *destkey, int keyc, const char **keyv, const int *weightv, REDIS_AGGREGATE aggregate )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"ZINTERSTORE %s %d %s",key, index, element);

	return (int)__reply->integer;
}

int RedisClient::zsets_zunionstore( const char *destkey, int keyc, const char **keyv, const int *weightv, REDIS_AGGREGATE aggregate )
{
	DelReply();
	//__reply = (redisReply *)redisCommand(__redis,"ZUNIONSTORE %s %d %s",key, index, element);

	return (int)__reply->integer;
}

int RedisClient::sort( const char *query )
{
	DelReply();
	__reply = (redisReply *)redisCommand(__redis,"SORT %s",query);

	return __reply->elements;
}

RedisClient* RedisClient::CreateRedisInterface(const char *ip, int port)
{
	return new RedisClient(ip,port);
}

char* RedisClient::errorreply()
{
	if(__redis)
		return __redis->errstr;
	else 
		return NULL;
}

