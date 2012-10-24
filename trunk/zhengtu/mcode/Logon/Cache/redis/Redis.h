//////////////////////////////////////////////////
/// @file : Redis.h
/// @brief : Radis封装类
/// @date:  2012/10/17
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __Redis_H__
#define __Redis_H__

#include <string>
#include <vector>
typedef enum _cr_aggregate {
	NONE,
	SUM, 
	MIN,
	MAX
} REDIS_AGGREGATE;

typedef enum REDIS_TYPE{
 REDIS_TYPE_NONE,
 REDIS_TYPE_STRING = 1,
 REDIS_TYPE_LIST = 2,
 REDIS_TYPE_SET = 3,
};

struct redisContext;
struct redisReply;
////////////////////////////////////////////////////////////////
/// @class RedisClient
/// @brief redis客户端连接操作类
///
/// @note 处理lists的命令的接口开始，没有经过测试，可能还有BUG！！！
class RedisClient
{
private:
	redisContext* __redis;
protected:
	redisReply* __reply;
	size_t __current;
	std::string m_host;
	int m_port;
public:
	static RedisClient* CreateRedisInterface(const char *ip="127.0.0.1", int port=6379);

	RedisClient(const char *ip, int port)
	{
		__redis = NULL;
		__reply = NULL;
		m_host = ip;
		m_port = port;
		Connect();
		__current =0;
	}
	virtual ~RedisClient()
	{
		DelReply();
		Close();
	}

	inline redisContext* context() { return __redis;}

	bool Connect();
	void Close();
	void DelReply();
	/** @brief 取得错误信息 */
	char* errorreply();
	////////////////////////////适合全体类型的命令//////////////////////////////////////////////
	/** @brief 判断一个键是否存在;存在返回 1;否则返回0; */
	int exists_key(const char *key);
	
	/** @brief 删除某个key,或是一系列key; */
	int delete_key(const char *key);
	
	/** @brief 返回某个key元素的数据类型 ( none:不存在,string:字符,list,set,zset,hash) */
	REDIS_TYPE type_key(const char *key);
	
	/** @brief 返回匹配的key列表 (KEYS foo*:查找foo开头的keys) */
	int get_keys(const char *pattern);
	
	/** @brief 随机获得一个已经存在的key，如果当前数据库为空，则返回空字符串 */
	int random_key();
	
	/** @brief 更改key的名字，新键如果存在将被覆盖 */
	int rename_key(const char *key, const char *new_key_name);
	
	/** @brief 更改key的名字，如果名字存在则更改失败 */
	int renamenx(const char *key, const char *new_key_name);
	
	/** @brief 返回当前数据库的key的总数 */
	int dbsize();
	
	/** @brief 设置某个key的过期时间（秒） */
	int expire_key(const char *key, int secs);
	
	/** @brief 查找某个key还有多长时间过期,返回时间秒*/
	int ttl_key(const char *key);
	
	/** @brief 选择数据库 */
	int select_index(int index);
	
	/** @brief 将指定键从当前数据库移到目标数据库 dbindex。成功返回 1;否则返回0 */
	int move(const char *key, int index);
	
	/** @brief 清空当前数据库中的所有键 */
	int flushdb();
	
	/** @brief 清空所有数据库中的所有键 */
	int flushall();
	
	///////////////////////////处理字符串的命令///////////////////////////////////////////////
	
	/** @brief 给一个键设置字符串值。SET keyname datalength data (SET bruce 10 paitoubing:保存key为burce,字符串长度为10的一个字符串paitoubing到数据库)*/
	int set(const char *key, const char *val);
	
	/** @brief 获取某个key 的value值。如key不存在，则返回字符串"nil"；如key的值不为字符串类型，则返回一个错误。 */
	int get(const char *key);
	
	/** @brief 可以理解成获得的key的值然后SET这个值，更加方便的操作 (SET bruce 10 paitoubing,
	这个时候需要修改bruce变成1234567890并获取这个以前的数据paitoubing,GETSET bruce 10 1234567890)*/
	int getset(const char *key, const char *set_val, char **get_val);
	
	/** @brief 一次性返回多个键的值 */
	int mget(std::vector<char *> keys);
	
	/** @brief 与SET的区别是SET可以创建与更新key的value，而SETNX是如果key不存在，则创建key与value数据 */
	int setnx(const char *key, const char *val);
	
	//TODO MSET 在一次原子操作下一次性设置多个键和值
	//TODO MSETNX 在一次原子操作下一次性设置多个键和值（目标键不存在情况下，如果有一个以上的key已存在，则失败）
	
	/** @brief 自增键值 */
	int incr(const char *key);
	
	/** @brief 令键值自增指定数值 */
	int incrby(const char *key, int incr_val);
	
	/** @brief 自减键值 */
	int decr(const char *key);
	
	/** @brief 令键值自减指定数值 */
	int decrby(const char *key, int decr_val);

	int append(const char *key, const char *val);
	int substr(const char *key, int start, int end);
	
	///////////////////////////处理 lists 的命令///////////////////////////////////////////////
	
	/** @brief 从 List 尾部添加一个元素（如序列不存在，则先创建，如已存在同名Key而非序列，则返回错误）*/
	int list_rpush(const char *key, const char *element);
	
	/** @brief 从 List 头部添加一个元素 */
	int list_lpush(const char *key, const char *element);
	
	/** @brief 返回一个 List 的长度 */
	int list_llen(const char *key);
	
	/** @brief 从自定的范围内返回序列的元素 (LRANGE testlist 0 2;返回序列testlist前0 1 2元素) */
	int list_lrange(const char *key, int start, int range);
	
	/** @brief 修剪某个范围之外的数据 (LTRIM testlist 0 2;保留0 1 2元素，其余的删除) */
	int list_ltrim(const char *key, int start, int end);
	
	/** @brief 返回某个位置的序列值(LINDEX testlist 0;返回序列testlist位置为0的元素) */
	int list_lindex(const char *key, int index);
	
	/** @brief 更新某个位置元素的值 */
	int list_lset(const char *key, int index, const char *element);
	
	/** @brief 从 List 的头部（count正数）或尾部（count负数）删除一定数量（count）匹配value的元素，返回删除的元素数量。 */
	int list_lrem(const char *key, int count, const char *element);
	
	/** @brief 弹出 List 的第一个元素 */
	int list_lpop(const char *key);
	
	/** @brief 弹出 List 的最后一个元素 */
	int list_rpop(const char *key);

	//////////////////////////处理集合(sets)的命令////////////////////////////////////////////////
	
	/** @brief 增加元素到SETS序列,如果元素（membe）不存在则添加成功 1，否则失败 0;(SADD testlist 3 \n one) */
	int set_sadd(const char *key, const char *member);
	
	/** @brief 删除SETS序列的某个元素，如果元素不存在则失败0，否则成功 1(SREM testlist 3 \N one) */
	int set_srem(const char *key, const char *member);
	
	/** @brief 从集合中随机弹出一个成员 */
	int set_spop(const char *key, char **member);
	
	/** @brief 把一个SETS序列的某个元素移动到另外一个SETS序列 (SMOVE testlist test 3\n two;
	从序列testlist移动元素two到 test中，testlist中将不存在two元素) */
	int set_smove(const char *sourcekey, const char *destkey, 
					const char *member);
	
	/** @brief y 统计某个SETS的序列的元素数量 */
	int set_scard(const char *key);
	
	/** @brief 获知指定成员是否存在于集合中 */
	int set_sismember(const char *key, const char *member);
	
	/** @brief 返回keyv 中的交集 */
	int set_sinter(int keyc, const char **keyv, char ***members);
	
	/** @brief 将 key1, key2, …, keyN 中的交集存入 dstkey */
	int set_sinterstore(const char *destkey, int keyc, const char **keyv);
	
	/** @brief 返回 key1, key2, …, keyN 的并集 */
	int set_sunion(int keyc, const char **keyv, char ***members);
	
	/** @brief 将 key1, key2, …, keyN 的并集存入 dstkey */
	int set_sunionstore(const char *destkey, int keyc, const char **keyv);
	
	/** @brief 依据 key2, …, keyN 求 key1 的差集。官方例子：
	key1 = x,a,b,c
	key2 = c
	key3 = a,d
	SDIFF key1,key2,key3 => x,b*/
	int set_sdiff(int keyc, const char **keyv, char ***members);
	
	/** @brief 依据 key2, …, keyN 求 key1 的差集并存入 dstkey */
	int set_sdiffstore(const char *destkey, int keyc, const char **keyv);
	
	/** @brief 返回某个序列的所有元素 */
	int set_smembers(const char *key); 

	/////////////////////////处理有序集合(sorted sets)的命令 (zsets)/////////////////////////////////////////////////
	
	/** @brief 添加指定成员到有序集合中，如果目标存在则更新score（分值，排序用） */
	int zsets_zadd(const char *key, double score, const char *member);
	
	/** @brief 从有序集合删除指定成员 */
	int zsets_zrem(const char *key, const char *member);
	
	/** @brief 如果成员存在则将其增加_increment_，否则将设置一个score为_increment_的成员 */
	int zsets_zincrby(const char *key, double incr_score, const char *member, double *new_score);
	
	/** @brief  */
	int zsets_zrank(const char *key, const char *member);
	
	/** @brief  */
	int zsets_zrevrank(const char *key, const char *member);
	
	/** @brief 返回升序排序后的指定范围的成员 */
	int zsets_zrange(const char *key, int start, int end);
	
	/** @brief 返回降序排序后的指定范围的成员 */
	int zsets_zrevrange(const char *key, int start, int end);
	
	/** @brief */
	int zsets_zcard(const char *key);
	
	/** @brief */
	int zsets_zscore(const char *key, const char *member, double *score);
	
	/** @brief 返回所有符合score >= min和score <= max的成员 ZCARD key 返回有序集合的元素数量 ZSCORE key element 
	返回指定成员的SCORE值 ZREMRANGEBYSCORE key min max 删除符合 score >= min 和 score <= max 条件的所有成员 */
	int zsets_zremrangebyscore(const char *key, double min, double max);
	
	/** @brief */
	int zsets_zremrangebyrank(const char *key, int start, int end);
	
	/** @brief */
	int zsets_zinterstore(const char *destkey, int keyc, const char **keyv, 
						   const int *weightv, REDIS_AGGREGATE aggregate);
	
	/** @brief */
	int zsets_zunionstore(const char *destkey, int keyc, const char **keyv, 
						   const int *weightv, REDIS_AGGREGATE aggregate);
	
	/** @brief 排序 */
	int sort(const char *query);
};

#endif