/**
* \brief ����һЩ��ɫ������ص�ָ��
*/

#include "SuperServer.h"

RoleregCache *RoleregCache::instance = NULL;

void RoleregCache::add(const Cmd::Super::t_Charname_Gateway &cmd)
{
	Data d(cmd);
	mlock.lock();
	datas.push_back(d);
	mlock.unlock();
}

void RoleregCache::timeAction(const zTime &ct)
{
	if (actionTimer.elapse(ct) > 10)
	{
		mlock.lock();
		while(!datas.empty())
		{
			Data &rd = datas.front();
			if (!msgParse_loginServer(rd.wdServerID,rd.accid,rd.name,rd.state)) break;
			Zebra::logger->debug("�������еĽ�ɫ����ָ����ɣ�%u,%s,%u",rd.accid,rd.name,rd.state);
			datas.pop_front();
		}
		mlock.unlock();
		actionTimer = ct;
	}
}

/**
* \brief ��Ž�ɫ��Ϣ�Ľṹ��
*/
struct RoleData
{  
	char name[MAX_NAMESIZE];//��ɫ��
	WORD game; //��Ϸ���
	WORD zone; //��Ϸ�����
	DWORD accid;  //�ʺű��

	RoleData(){accid=0;zone=0;game=0;memset(name,0,sizeof(name));}
	RoleData(const RoleData &rd)
	{
		accid    = rd.accid;
		zone     = rd.zone;
		game     = rd.game;
		strncpy(name,rd.name,sizeof(name));
	}
	const RoleData & operator= (const RoleData &rd)
	{
		accid    = rd.accid;
		zone     = rd.zone;
		game     = rd.game;
		strncpy(name,rd.name,sizeof(name));
		return *this;
	}
};

/**
* \brief ��װһ��������
*/
class RoleDataContainer
{
public:

	/**
	* \brief ���캯��
	*/
	RoleDataContainer() {};

	/**
	* \brief ��������
	*/
	~RoleDataContainer()
	{
		mlock.lock();
		barrel.clear();
		mlock.unlock();
	}

	/**
	* \brief ������ݵ�������
	* \param data ����ӵ�����
	* \return ��ӵ������Ƿ�ɹ�
	*/
	bool add(const RoleData &data)
	{
		zMutex_scope_lock scope_lock(mlock);
		container_type::const_iterator it = barrel.find(data.name);
		if (it == barrel.end())
		{
			//û���ҵ�
			std::pair<container_type::iterator,bool> p = barrel.insert(container_type::value_type(data.name,data));
			return p.second;
		}
		else
		{
			//�ҵ�����
			return false;
		}
	}

	/**
	* \brief ��������ɾ��ָ�����Ƶ�����
	* \param name ָ��ɾ��������
	*/
	void remove(const char *name)
	{
		zMutex_scope_lock scope_lock(mlock);
		container_type::iterator it = barrel.find(name);
		if (it != barrel.end())
		{
			//�ҵ�����
			barrel.erase(it);
		}
	}

private:

	struct eqstr
	{
		bool operator()(const char* s1,const char* s2) const
		{
			return strcmp(s1,s2) == 0;
		}
	};
	/**
	* \brief ��������
	*/
	typedef hash_map<const char *,RoleData> container_type;
	/**
	* \brief �������ʻ������
	*/
	zMutex mlock;
	/**
	* \brief ��������
	*/
	container_type barrel;

};

static RoleDataContainer SetRoleInfo; //��Ž�ɫ��Ϣ��container

/**
* \brief ��ɫ�Ĵ�����ɾ��
* \param pNullCmd ������ָ��
* \param nCmdLen ָ���
* \return ����ɾ����ɫ�Ƿ�ɹ�
*/
bool RoleregCache::msgParse_loginServer(WORD wdServerID,DWORD accid,char name[MAX_NAMESIZE],WORD state)
{
	using namespace Cmd::Super;
	GameZone_t gameZone;

	Zebra::logger->debug("state=%u",state);
	gameZone = SuperService::getInstance().getZoneID();
	if (state & ROLEREG_STATE_TEST)//����ɫ��
	{
		RoleData tmpinfo;
		//������COPYһ��
		tmpinfo.accid = accid;
		tmpinfo.zone= gameZone.zone;
		tmpinfo.game=  gameZone.game;
		strncpy(tmpinfo.name,name,sizeof(tmpinfo.name));
		Zebra::logger->debug("accid=%u",  tmpinfo.accid);
		Zebra::logger->debug("zone=%u",  tmpinfo.zone);
		Zebra::logger->debug("game=%u",  tmpinfo.game);
		Zebra::logger->debug("name=%s",  tmpinfo.name);
		if (SetRoleInfo.add(tmpinfo))
		{
			//��ӳɹ�,��ʾԭ��û������
			char where[128];
			DWORD accID=0;
			static const dbCol sel_con_define[]  = {
				{"`ACCID`",zDBConnPool::DB_DWORD,sizeof(DWORD)},
				{NULL,0,0}
			};
			memset(where,0,sizeof(where));
			sprintf(where,"NAME = '%s'",tmpinfo.name);
			connHandleID handle = SuperService::dbConnPool->getHandle();
			Zebra::logger->debug("handle=%u",handle);
			if ((connHandleID)-1 != handle)
			{
				if ((DWORD)1 == SuperService::dbConnPool->exeSelectLimit(handle,
					"'ROLEREG'",
					sel_con_define,where,NULL,1,(BYTE*)&accID))
				{
					//�����Ѵ������ݿ���,��������е�����
					SetRoleInfo.remove(tmpinfo.name);
					state |= ROLEREG_STATE_HAS;
					Zebra::logger->debug("�����Ѵ������ݿ���");
				}
				SuperService::dbConnPool->putHandle(handle);
			}
			else
			{
				//��ȡ���ݿ�������,��������е�����
				SetRoleInfo.remove(tmpinfo.name);
				state |= ROLEREG_STATE_HAS;
				Zebra::logger->debug("��ȡ���ݿ�������");
			}
		}
		else
		{
			state |= ROLEREG_STATE_HAS;
			Zebra::logger->debug("�����Ѵ��ڻ�����");
		}
	}
	else if (state & ROLEREG_STATE_WRITE)//��д
	{
		RoleData Tmpinfo;
		Tmpinfo.accid = accid;
		Tmpinfo.zone= gameZone.zone;
		Tmpinfo.game=  gameZone.game;
		strncpy(Tmpinfo.name,name,sizeof(Tmpinfo.name));
		Zebra::logger->debug("##########");
		Zebra::logger->debug("name = %s ",Tmpinfo.name);
		//���ݿ�������
		static const dbCol creat_con_define[]  = {
			{"`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE])},
			{"`GAME`",zDBConnPool::DB_WORD,sizeof(WORD)},
			{"`ZONE`",zDBConnPool::DB_WORD,sizeof(WORD)},
			{"`ACCID`",zDBConnPool::DB_DWORD,sizeof(DWORD)},
			{NULL,0,0}
		};
		connHandleID handle = SuperService::dbConnPool->getHandle();
		Zebra::logger->debug("handle=%u",handle);
		if ((connHandleID)-1 != handle)
		{
			if ((DWORD)-1 != SuperService::dbConnPool->exeInsert(handle,
				"'ROLEREG'",
				creat_con_define,(BYTE*)&Tmpinfo))
			{
				state |= ROLEREG_STATE_OK;
				Zebra::logger->debug("��д���ݿ�ɹ�");
			}
			else
			{
				state &= ~ROLEREG_STATE_OK;
				Zebra::logger->debug("��д���ݿ�ʧ��");
			}
			SuperService::dbConnPool->putHandle(handle);
		}
		else
		{
			state &= ~ROLEREG_STATE_OK;
			Zebra::logger->debug("��ȡ���ʧ��");
		}
		//����ṹ�иý�ɫ
		SetRoleInfo.remove(name);
	}
	else if (state & ROLEREG_STATE_CLEAN)//���
	{
		Zebra::logger->debug("name = %s ",name);
		char cWhere[128];
		memset(cWhere,0,sizeof(cWhere));
		sprintf(cWhere,"NAME = '%s'",name);
		connHandleID handle = SuperService::dbConnPool->getHandle();
		Zebra::logger->debug("handle=%u",handle);
		if ((connHandleID) -1 != handle)
		{
			if ((DWORD)-1 == SuperService::dbConnPool->exeDelete(handle,
				"'ROLEREG'",cWhere))
			{
				state &= ~ROLEREG_STATE_OK;
				Zebra::logger->debug("ɾ����ɫ%sʧ��",name);
			}
			else
			{
				state |= ROLEREG_STATE_OK;
				Zebra::logger->debug("ɾ����ɫ%s�ɹ�",name);
			}
			SuperService::dbConnPool->putHandle(handle);
		}
		else
		{
			state &= ~ROLEREG_STATE_OK;
			Zebra::logger->debug("��ȡ���ʧ��");
		}
	}

	Zebra::logger->debug("state =%u",state);

	if (state & ROLEREG_STATE_TEST)
	{
		t_Charname_Gateway cmd;
		cmd.wdServerID = wdServerID;
		cmd.accid = accid;
		bcopy(name,cmd.name,sizeof(cmd.name),sizeof(cmd.name));
		cmd.state = state;
		ServerManager::getInstance().broadcastByID(cmd.wdServerID,&cmd,sizeof(cmd));
	}
	if (state & ROLEREG_STATE_WRITE)
	{
		if (state & ROLEREG_STATE_OK)
			Zebra::logger->error("��д��ɫ�ɹ���%u,%s",accid,name);
		else
			Zebra::logger->error("��д��ɫʧ�ܣ�%u,%s",accid,name);
	}
	if (state & ROLEREG_STATE_CLEAN)
	{
		if (state & ROLEREG_STATE_OK)
			Zebra::logger->error("ɾ����ɫ�ɹ���%u,%s",accid,name);
		else
			Zebra::logger->error("ɾ����ɫʧ�ܣ�%u,%s",accid,name);
	}

	return true;        
}
