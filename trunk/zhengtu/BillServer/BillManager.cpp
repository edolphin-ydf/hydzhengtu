/**
 * \brief �˺Ź���,��¼һ�����������Ѿ���½���˺�
 *
 * 
 */

#include "BillServer.h"

BillManager *BillManager::instance = NULL;

/**
 * \brief ��֤�ʺ��Ƿ��Ѿ���½
 * ����Ѿ���½��Ҫ���ش�����Ϣ����½������
 * \param session ����֤�ĻỰ��Ϣ
 * \return ��֤�Ƿ�ɹ�
 */
bool BillManager::verify(const t_NewLoginSession &session)
{
  mlock.lock();
  BillInfoMap_iterator it = infoMap.find(session.accid);
  if (it != infoMap.end())
  {
    mlock.unlock();
    return false;
  }

  BillInfo info;
  info.accid = session.accid;
  info.loginTempID = session.loginTempID;
  info.wdGatewayID = session.wdGatewayID;
  strncpy(info.client_ip,session.client_ip,sizeof(info.client_ip));
  infoMap.insert(BillInfoMap_pair(info.accid,info));

  mlock.unlock();
  return true;
}

/**
 * \brief ���µ�½��ʱ��Ϣ
 * ��½��������֤ͨ���Ժ�,�����һ��ʱ��û�е�½����,��ô��ɾ������Ự����
 */
void BillManager::update()
{
  mlock.lock();
  if (!infoMap.empty())
  {
    zTime current;
    for(BillInfoMap_iterator it = infoMap.begin(); it != infoMap.end();)
    {
      //��½��ʱ,ɾ��
      if (BillInfo::WAIT_LOGIN == it->second.state
          && it->second.timestamp.elapse(current) >= session_timeout_value)
      {
        Zebra::logger->debug("�ȴ����ص�½��Ϣ��ʱ��%u,%u,%u",it->second.accid,it->second.loginTempID,it->second.wdGatewayID);
        BillInfoMap_iterator tmp = it;
        it++;
        infoMap.erase(tmp);
      }
      else
        it++;
    }
  }
  mlock.unlock();
}

/**
 * \brief �����عرյ�ʱ��,��Ҫ�����������������½�����лỰ��Ϣ
 * \param wdGatewayID ���ر��
 */
void BillManager::updateByGatewayID(const WORD wdGatewayID)
{
  mlock.lock();
  if (!infoMap.empty())
  {
    for(BillInfoMap_iterator it = infoMap.begin(); it != infoMap.end();)
    {
      if (wdGatewayID == it->second.wdGatewayID)
      {
        Zebra::logger->debug("���عر�,�����½���ݣ�%u,%u,%u",it->second.accid,it->second.loginTempID,it->second.wdGatewayID);
        BillInfoMap_iterator tmp = it;
        it++;
        infoMap.erase(tmp);
      }
      else
        it++;
    }
  }
  mlock.unlock();
}

/**
 * \brief ��֤�ͻ��˵�½���ص���Ϣ
 * ���ͻ��˵�½���ص��Ƿ�,��Ҫ��֤�����½�Ƿ��ǺϷ���
 * \param accid �ʺű��
 * \param loginTempID ��½��ʱ���
 * \return ��֤�Ƿ�ɹ�
 */
bool BillManager::login(const DWORD accid,const DWORD loginTempID)
{
  mlock.lock();
  BillInfoMap_iterator it = infoMap.find(accid);
  if (it != infoMap.end()
      && loginTempID == it->second.loginTempID
      && BillInfo::WAIT_LOGIN == it->second.state)
  {
    it->second.state = BillInfo::CONF_LOGIN;
    mlock.unlock();
    return true;
  }
  mlock.unlock();
  return false;
}

/**
 * \brief ���˳���ʱ��,��Ҫ�����ҵĵ�½�Ự����
 * ����������ɾ����¼,����������˺�����һ��ʱ��,������Ч�����ظ���½
 * \param accid �ʺű��
 * \param loginTempID ��½��ʱ���
 * \return �˳���½�Ƿ�ɹ�
 */
bool BillManager::logout(const DWORD accid,const DWORD loginTempID)
{
  mlock.lock();
  BillInfoMap_iterator it = infoMap.find(accid);
  if (it != infoMap.end()
      && loginTempID == it->second.loginTempID
      && BillInfo::CONF_LOGIN == it->second.state)
  {
    //�˳���½��ʱ��,��Ҫ�ȴ�2���ӵ�ʱ��,����������һ�ε�½
    it->second.state = BillInfo::WAIT_LOGIN;
    it->second.timestamp.now();
    it->second.timestamp -= (session_timeout_value - 2);
    mlock.unlock();
    return true;
  }
  mlock.unlock();
  return false;
}

bool BillManager::updateGold(DWORD acc,double gold)
{
  mlock.lock();
  BillInfoMap_iterator it = infoMap.find(acc);
  if (it != infoMap.end()
      && BillInfo::CONF_LOGIN == it->second.state)
  {
    it->second.gold = gold;
    mlock.unlock();
    return true;
  }
  mlock.unlock();
  return false;
}
bool BillManager::updateVipTime(DWORD acc,DWORD vip)
{
  mlock.lock();
  BillInfoMap_iterator it = infoMap.find(acc);
  if (it != infoMap.end()
      && BillInfo::CONF_LOGIN == it->second.state)
  {
    it->second.vip_time = vip;
    mlock.unlock();
    return true;
  }
  mlock.unlock();
  return false;
}
DWORD BillManager::getVipTime(DWORD acc)
{
  DWORD ret_vip = 0;
  mlock.lock();
  BillInfoMap_iterator it = infoMap.find(acc);
  if (it != infoMap.end()
      && BillInfo::CONF_LOGIN == it->second.state)
  {
    ret_vip = it->second.vip_time;
  }
  mlock.unlock();
  return ret_vip;
}
double BillManager::getGold(DWORD acc)
{
  double ret_gold = 0.0;
  mlock.lock();
  BillInfoMap_iterator it = infoMap.find(acc);
  if (it != infoMap.end()
      && BillInfo::CONF_LOGIN == it->second.state)
  {
    ret_gold = it->second.gold;
  }
  mlock.unlock();
  return ret_gold;
}
