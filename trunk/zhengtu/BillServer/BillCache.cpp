/**
 * \brief �������������к�
 * ��������������,�Ա�ȷ��һ�������Ľ�������
 */

#include "BillServer.h"

BillCache::BillCache() : SerialNumber(0)
{
}

BillCache::~BillCache()
{
}

BillData *BillCache::add(BillData *bd,const char *gameZone_str)
{
  if (bd)
  {
    zMutex_scope_lock scope_lock(mlock);
    BillData *new_bd = new BillData;
    if (new_bd)
    {
      bcopy(bd,new_bd,sizeof(BillData),sizeof(BillData));
      sprintf_s(new_bd->tid,"%5s%08lx%08x",gameZone_str,time(NULL),++SerialNumber);
      new_bd->timeout = time(NULL);
      strcpy_s(bd->tid,new_bd->tid);
      //Zebra::logger->debug("tradeSN: %s",new_bd->tid);
      std::pair<iter,bool> res = cache.insert(value_type(new_bd->tid,new_bd));
      if (res.second)
        return new_bd;
      else
        SAFE_DELETE(new_bd);
    }
  }

  return NULL;
}

BillData *BillCache::get(const char *tid)
{
  BillData *retval = NULL;

  if (tid)
  {
    zMutex_scope_lock scope_lock(mlock);
    iter it = cache.find(tid);
    if (it != cache.end())
    {
      retval = it->second;
      cache.erase(it);
    }
  }

  return retval;
}

void BillCache::remove(const char *tid)
{
  if (tid)
  {
    zMutex_scope_lock scope_lock(mlock);
    iter it = cache.find(tid);
    if (it != cache.end())
    {
      BillData *retval = it->second;
      cache.erase(it);
      SAFE_DELETE(retval);
    }
  }
}

void BillCache::update(const zTime &ct)
{
  zMutex_scope_lock scope_lock(mlock);
  for(iter it = cache.begin(); it != cache.end();)
  {
    if (ct.sec() > it->second->timeout
        && (ct.sec() - it->second->timeout) >= DEFAULT_BILL_TIMEOUT)
    {
      //TODO FIXME ������׳�ʱ,����д��־
      //Zebra::logger->debug("tradeSN timeout: %s",it->second->tid);
      iter tmp = it;
      BillData *bd = tmp->second;
      ++it;
      cache.erase(tmp);
      SAFE_DELETE(bd);
    }
    else
      ++it;
  }
}

