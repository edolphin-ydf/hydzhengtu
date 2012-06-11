/**
* \brief ������Ϣ�б�
*
* ��½��������Ҫ�������µ��������ص���Ϣ�б�,���ڷ�������
* 
*/

#include "FLServer.h"

GYListManager *GYListManager::instance = NULL;

/**
* \brief ���������Ϣ
* ����Ѿ�����,ֱ�Ӹ�����Ϣ,û����Ҫ�½�����¼
* \param gameZone ��Ϸ����Ϣ
* \param gy ������Ϣ
* \return ����Ƿ�ɹ�
*/
bool GYListManager::put(const GameZone_t &gameZone,const GYList &gy)
{
	Zebra::logger->debug("GYListManager::put");
	zMutex_scope_lock scope_lock(mlock);
	pair<GYListContainer_iterator,GYListContainer_iterator> hps = gyData.equal_range(gameZone);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		if (it->second.wdServerID == gy.wdServerID)
		{
			//�ҵ���,ֻ�Ǹ���,ֻ��������������������״̬
			bcopy(gy.pstrIP,it->second.pstrIP,MAX_IP_LENGTH,sizeof(it->second.pstrIP));
			it->second.wdPort = gy.wdPort;
			it->second.wdNumOnline = gy.wdNumOnline;
			it->second.state = gy.state;
			return true;
		}
	}

	//û���ҵ�,��Ҫ�����µļ�¼
	gyData.insert(GYListContainer_value_type(gameZone,gy));
	return true;
}

void GYListManager::disableAll(const GameZone_t &gameZone)
{
	Zebra::logger->debug("GYListManager::disableAll");
	zMutex_scope_lock scope_lock(mlock);
	pair<GYListContainer_iterator,GYListContainer_iterator> hps = gyData.equal_range(gameZone);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		it->second.wdPort = 0;
		it->second.wdNumOnline = 0;
		it->second.state = state_maintain;
	}
}

/**
* \brief �����ȡһ��������С��������Ϣ
* \return ������Ϣ
*/
GYList *GYListManager::getAvl(const GameZone_t &gameZone)
{
	Zebra::logger->debug("GYListManager::getAvl");
	zMutex_scope_lock scope_lock(mlock);
	GYList *ret = NULL,*tmp = NULL;

	Zebra::logger->debug("GYListSize = %d",gyData.size());
	pair<GYListContainer_iterator,GYListContainer_iterator> hps = gyData.equal_range(gameZone);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		tmp = &(it->second);
		if (state_none == tmp->state
			&& (NULL == ret
			|| ret->wdNumOnline >= tmp->wdNumOnline))
		{
			ret = tmp;
		}
	}
	return ret;
}

DWORD GYListManager::getOnline(void)
{
	DWORD                    dwCount;
	GYList                   *pGYL;
	GYListContainer_iterator pGYLCI;

	Zebra::logger->debug("GYListManager::getOnline");
	zMutex_scope_lock scope_lock(mlock);

	dwCount = 0;
	for(pGYLCI=gyData.begin();pGYLCI != gyData.end();pGYLCI++)
	{
		pGYL = &(pGYLCI->second);
		dwCount += pGYL->wdNumOnline;
	}

	return dwCount;
}

/**
* \brief ��ȡ�����б�
* \return ������Ϣ
*/
void GYListManager::full_ping_list(Cmd::stPingList* cmd,const GameZone_t& gameZone)
{
	Zebra::logger->debug("GYListManager::full_ping_list");
	zMutex_scope_lock scope_lock(mlock);
	GYList *ret = NULL;
	const int per_num = 5;   // ����
	int server_num = gyData.count(gameZone);
	int max_per = server_num * 2000;  // �������
	int per_per = max_per/per_num; // �ֳ��嵵,ÿһ��������
	int total_personal = 0; // ����������
	int i=0;

	pair<GYListContainer_iterator,GYListContainer_iterator> hps = gyData.equal_range(gameZone);

	cmd->zone_id = gameZone.id;
	//      Cmd::ping_element* tempElement = cmd->ping_list;

	for (GYListContainer_iterator it = hps.first; it != hps.second; ++it,i++)
	{
		ret = &(it->second);
		if (state_none == ret->state)
		{
			if (i<server_num)
			{
				strncpy((char*)cmd->ping_list.gateway_ip,(char*)ret->pstrIP,15);
				total_personal += ret->wdNumOnline;
			}
			else
			{
				break;
			}
		}
	}

	for (int i=0; i<5; i++)
	{
		if (total_personal>=per_per*i && total_personal<(per_per*(i+1)-1))
		{
			cmd->ping_list.state = i;
			break;
		}
	}
}

bool GYListManager::verifyVer(const GameZone_t &gameZone,DWORD verify_client_version,BYTE &retcode)
{
	Zebra::logger->debug("GYListManager::verifyVer");
	zMutex_scope_lock scope_lock(mlock);
	bool retval = false;
	GYList *ret = NULL,*tmp = NULL;
	pair<GYListContainer_iterator,GYListContainer_iterator> hps = gyData.equal_range(gameZone);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		tmp = &(it->second);
		if (state_none == tmp->state
			&& (NULL == ret
			|| ret->wdNumOnline >= tmp->wdNumOnline))
		{
			ret = tmp;
		}
	}
	if (NULL == ret)
	{
		retcode = Cmd::LOGIN_RETURN_GATEWAYNOTAVAILABLE;
	}
	else if (ret->zoneGameVersion && ret->zoneGameVersion != verify_client_version)
	{
		retcode = Cmd::LOGIN_RETURN_VERSIONERROR;
	}
	else
	{
		retval = true;
	}
	return retval;
}

