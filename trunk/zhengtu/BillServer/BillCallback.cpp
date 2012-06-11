/**
 * \brief ����Ʒѿͻ��˽ӿ�
 * <p>
 * �ṩ�Ʒѿͻ��˻ص��������Ͷ���,<br>
 * �Լ��Ʒѿͻ���ģ���ʼ���ͻ��սӿڡ�
 * </p>
 */

#include "BillServer.h"

bool Bill_init(const std::string &confile,const std::string &tradelog,struct BillCallback *bc)
{
  Zebra::logger->debug("Bill_init");
  if (NULL == bc)
  {
    Zebra::logger->debug("��ʼ�����׿ͻ���ʧ��");
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

