/**
 * \brief 定义计费客户端接口
 * <p>
 * 提供计费客户端回调函数类型定义,<br>
 * 以及计费客户端模块初始化和回收接口。
 * </p>
 */

#include "BillServer.h"

bool Bill_init(const std::string &confile,const std::string &tradelog,struct BillCallback *bc)
{
  Zebra::logger->debug("Bill_init");
  if (NULL == bc)
  {
    Zebra::logger->debug("初始化交易客户端失败");
    return false;
  }
  BillCache::newInstance();
  BillClientManager::newInstance();
  return BillClientManager::getInstance().init(confile,tradelog,*bc);
}

void Bill_final()
{
  Zebra::logger->debug("Bill_final");
  BillClientManager::delInstance();
  BillCache::delInstance();
}

void Bill_timeAction()
{
  zTime ct;
  BillClientManager::getInstance().timeAction(ct);
  BillCache::getInstance().update(ct);
}

bool Bill_action(BillData *bd)
{
  if (NULL == bd)
    return false;
  else
    return BillClientManager::getInstance().action(bd);
}

