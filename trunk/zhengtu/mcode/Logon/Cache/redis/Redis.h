//////////////////////////////////////////////////
/// @file : Redis.h
/// @brief : Radis��װ��
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
/// @brief redis�ͻ������Ӳ�����
///
/// @note ����lists������Ľӿڿ�ʼ��û�о������ԣ����ܻ���BUG������
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
	/** @brief ȡ�ô�����Ϣ */
	char* errorreply();
	////////////////////////////�ʺ�ȫ�����͵�����//////////////////////////////////////////////
	/** @brief �ж�һ�����Ƿ����;���ڷ��� 1;���򷵻�0; */
	int exists_key(const char *key);
	
	/** @brief ɾ��ĳ��key,����һϵ��key; */
	int delete_key(const char *key);
	
	/** @brief ����ĳ��keyԪ�ص��������� ( none:������,string:�ַ�,list,set,zset,hash) */
	REDIS_TYPE type_key(const char *key);
	
	/** @brief ����ƥ���key�б� (KEYS foo*:����foo��ͷ��keys) */
	int get_keys(const char *pattern);
	
	/** @brief ������һ���Ѿ����ڵ�key�������ǰ���ݿ�Ϊ�գ��򷵻ؿ��ַ��� */
	int random_key();
	
	/** @brief ����key�����֣��¼�������ڽ������� */
	int rename_key(const char *key, const char *new_key_name);
	
	/** @brief ����key�����֣�������ִ��������ʧ�� */
	int renamenx(const char *key, const char *new_key_name);
	
	/** @brief ���ص�ǰ���ݿ��key������ */
	int dbsize();
	
	/** @brief ����ĳ��key�Ĺ���ʱ�䣨�룩 */
	int expire_key(const char *key, int secs);
	
	/** @brief ����ĳ��key���ж೤ʱ�����,����ʱ����*/
	int ttl_key(const char *key);
	
	/** @brief ѡ�����ݿ� */
	int select_index(int index);
	
	/** @brief ��ָ�����ӵ�ǰ���ݿ��Ƶ�Ŀ�����ݿ� dbindex���ɹ����� 1;���򷵻�0 */
	int move(const char *key, int index);
	
	/** @brief ��յ�ǰ���ݿ��е����м� */
	int flushdb();
	
	/** @brief ����������ݿ��е����м� */
	int flushall();
	
	///////////////////////////�����ַ���������///////////////////////////////////////////////
	
	/** @brief ��һ���������ַ���ֵ��SET keyname datalength data (SET bruce 10 paitoubing:����keyΪburce,�ַ�������Ϊ10��һ���ַ���paitoubing�����ݿ�)*/
	int set(const char *key, const char *val);
	
	/** @brief ��ȡĳ��key ��valueֵ����key�����ڣ��򷵻��ַ���"nil"����key��ֵ��Ϊ�ַ������ͣ��򷵻�һ������ */
	int get(const char *key);
	
	/** @brief �������ɻ�õ�key��ֵȻ��SET���ֵ�����ӷ���Ĳ��� (SET bruce 10 paitoubing,
	���ʱ����Ҫ�޸�bruce���1234567890����ȡ�����ǰ������paitoubing,GETSET bruce 10 1234567890)*/
	int getset(const char *key, const char *set_val, char **get_val);
	
	/** @brief һ���Է��ض������ֵ */
	int mget(std::vector<char *> keys);
	
	/** @brief ��SET��������SET���Դ��������key��value����SETNX�����key�����ڣ��򴴽�key��value���� */
	int setnx(const char *key, const char *val);
	
	//TODO MSET ��һ��ԭ�Ӳ�����һ�������ö������ֵ
	//TODO MSETNX ��һ��ԭ�Ӳ�����һ�������ö������ֵ��Ŀ�������������£������һ�����ϵ�key�Ѵ��ڣ���ʧ�ܣ�
	
	/** @brief ������ֵ */
	int incr(const char *key);
	
	/** @brief ���ֵ����ָ����ֵ */
	int incrby(const char *key, int incr_val);
	
	/** @brief �Լ���ֵ */
	int decr(const char *key);
	
	/** @brief ���ֵ�Լ�ָ����ֵ */
	int decrby(const char *key, int decr_val);

	int append(const char *key, const char *val);
	int substr(const char *key, int start, int end);
	
	///////////////////////////���� lists ������///////////////////////////////////////////////
	
	/** @brief �� List β�����һ��Ԫ�أ������в����ڣ����ȴ��������Ѵ���ͬ��Key�������У��򷵻ش���*/
	int list_rpush(const char *key, const char *element);
	
	/** @brief �� List ͷ�����һ��Ԫ�� */
	int list_lpush(const char *key, const char *element);
	
	/** @brief ����һ�� List �ĳ��� */
	int list_llen(const char *key);
	
	/** @brief ���Զ��ķ�Χ�ڷ������е�Ԫ�� (LRANGE testlist 0 2;��������testlistǰ0 1 2Ԫ��) */
	int list_lrange(const char *key, int start, int range);
	
	/** @brief �޼�ĳ����Χ֮������� (LTRIM testlist 0 2;����0 1 2Ԫ�أ������ɾ��) */
	int list_ltrim(const char *key, int start, int end);
	
	/** @brief ����ĳ��λ�õ�����ֵ(LINDEX testlist 0;��������testlistλ��Ϊ0��Ԫ��) */
	int list_lindex(const char *key, int index);
	
	/** @brief ����ĳ��λ��Ԫ�ص�ֵ */
	int list_lset(const char *key, int index, const char *element);
	
	/** @brief �� List ��ͷ����count��������β����count������ɾ��һ��������count��ƥ��value��Ԫ�أ�����ɾ����Ԫ�������� */
	int list_lrem(const char *key, int count, const char *element);
	
	/** @brief ���� List �ĵ�һ��Ԫ�� */
	int list_lpop(const char *key);
	
	/** @brief ���� List �����һ��Ԫ�� */
	int list_rpop(const char *key);

	//////////////////////////������(sets)������////////////////////////////////////////////////
	
	/** @brief ����Ԫ�ص�SETS����,���Ԫ�أ�membe������������ӳɹ� 1������ʧ�� 0;(SADD testlist 3 \n one) */
	int set_sadd(const char *key, const char *member);
	
	/** @brief ɾ��SETS���е�ĳ��Ԫ�أ����Ԫ�ز�������ʧ��0������ɹ� 1(SREM testlist 3 \N one) */
	int set_srem(const char *key, const char *member);
	
	/** @brief �Ӽ������������һ����Ա */
	int set_spop(const char *key, char **member);
	
	/** @brief ��һ��SETS���е�ĳ��Ԫ���ƶ�������һ��SETS���� (SMOVE testlist test 3\n two;
	������testlist�ƶ�Ԫ��two�� test�У�testlist�н�������twoԪ��) */
	int set_smove(const char *sourcekey, const char *destkey, 
					const char *member);
	
	/** @brief y ͳ��ĳ��SETS�����е�Ԫ������ */
	int set_scard(const char *key);
	
	/** @brief ��ָ֪����Ա�Ƿ�����ڼ����� */
	int set_sismember(const char *key, const char *member);
	
	/** @brief ����keyv �еĽ��� */
	int set_sinter(int keyc, const char **keyv, char ***members);
	
	/** @brief �� key1, key2, ��, keyN �еĽ������� dstkey */
	int set_sinterstore(const char *destkey, int keyc, const char **keyv);
	
	/** @brief ���� key1, key2, ��, keyN �Ĳ��� */
	int set_sunion(int keyc, const char **keyv, char ***members);
	
	/** @brief �� key1, key2, ��, keyN �Ĳ������� dstkey */
	int set_sunionstore(const char *destkey, int keyc, const char **keyv);
	
	/** @brief ���� key2, ��, keyN �� key1 �Ĳ���ٷ����ӣ�
	key1 = x,a,b,c
	key2 = c
	key3 = a,d
	SDIFF key1,key2,key3 => x,b*/
	int set_sdiff(int keyc, const char **keyv, char ***members);
	
	/** @brief ���� key2, ��, keyN �� key1 �Ĳ������ dstkey */
	int set_sdiffstore(const char *destkey, int keyc, const char **keyv);
	
	/** @brief ����ĳ�����е�����Ԫ�� */
	int set_smembers(const char *key); 

	/////////////////////////�������򼯺�(sorted sets)������ (zsets)/////////////////////////////////////////////////
	
	/** @brief ���ָ����Ա�����򼯺��У����Ŀ����������score����ֵ�������ã� */
	int zsets_zadd(const char *key, double score, const char *member);
	
	/** @brief �����򼯺�ɾ��ָ����Ա */
	int zsets_zrem(const char *key, const char *member);
	
	/** @brief �����Ա������������_increment_����������һ��scoreΪ_increment_�ĳ�Ա */
	int zsets_zincrby(const char *key, double incr_score, const char *member, double *new_score);
	
	/** @brief  */
	int zsets_zrank(const char *key, const char *member);
	
	/** @brief  */
	int zsets_zrevrank(const char *key, const char *member);
	
	/** @brief ��������������ָ����Χ�ĳ�Ա */
	int zsets_zrange(const char *key, int start, int end);
	
	/** @brief ���ؽ���������ָ����Χ�ĳ�Ա */
	int zsets_zrevrange(const char *key, int start, int end);
	
	/** @brief */
	int zsets_zcard(const char *key);
	
	/** @brief */
	int zsets_zscore(const char *key, const char *member, double *score);
	
	/** @brief �������з���score >= min��score <= max�ĳ�Ա ZCARD key �������򼯺ϵ�Ԫ������ ZSCORE key element 
	����ָ����Ա��SCOREֵ ZREMRANGEBYSCORE key min max ɾ������ score >= min �� score <= max ���������г�Ա */
	int zsets_zremrangebyscore(const char *key, double min, double max);
	
	/** @brief */
	int zsets_zremrangebyrank(const char *key, int start, int end);
	
	/** @brief */
	int zsets_zinterstore(const char *destkey, int keyc, const char **keyv, 
						   const int *weightv, REDIS_AGGREGATE aggregate);
	
	/** @brief */
	int zsets_zunionstore(const char *destkey, int keyc, const char **keyv, 
						   const int *weightv, REDIS_AGGREGATE aggregate);
	
	/** @brief ���� */
	int sort(const char *query);
};

#endif