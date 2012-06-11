/**
* \brief 缓冲一些角色名称相关的指令
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
			Zebra::logger->debug("处理缓冲中的角色名称指令完成：%u,%s,%u",rd.accid,rd.name,rd.state);
			datas.pop_front();
		}
		mlock.unlock();
		actionTimer = ct;
	}
}

/**
* \brief 存放角色信息的结构体
*/
struct RoleData
{  
	char name[MAX_NAMESIZE];//角色名
	WORD game; //游戏编号
	WORD zone; //游戏区编号
	DWORD accid;  //帐号编号

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
* \brief 封装一个容器类
*/
class RoleDataContainer
{
public:

	/**
	* \brief 构造函数
	*/
	RoleDataContainer() {};

	/**
	* \brief 析构函数
	*/
	~RoleDataContainer()
	{
		mlock.lock();
		barrel.clear();
		mlock.unlock();
	}

	/**
	* \brief 添加数据到容器中
	* \param data 待添加的容器
	* \return 添加到容器是否成功
	*/
	bool add(const RoleData &data)
	{
		zMutex_scope_lock scope_lock(mlock);
		container_type::const_iterator it = barrel.find(data.name);
		if (it == barrel.end())
		{
			//没有找到
			std::pair<container_type::iterator,bool> p = barrel.insert(container_type::value_type(data.name,data));
			return p.second;
		}
		else
		{
			//找到存在
			return false;
		}
	}

	/**
	* \brief 从容器中删除指定名称的内容
	* \param name 指定删除的名称
	*/
	void remove(const char *name)
	{
		zMutex_scope_lock scope_lock(mlock);
		container_type::iterator it = barrel.find(name);
		if (it != barrel.end())
		{
			//找到存在
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
	* \brief 容器类型
	*/
	typedef hash_map<const char *,RoleData> container_type;
	/**
	* \brief 容器访问互斥变量
	*/
	zMutex mlock;
	/**
	* \brief 容器变量
	*/
	container_type barrel;

};

static RoleDataContainer SetRoleInfo; //存放角色信息的container

/**
* \brief 角色的创建与删除
* \param pNullCmd 待处理指令
* \param nCmdLen 指令长度
* \return 创建删除角色是否成功
*/
bool RoleregCache::msgParse_loginServer(WORD wdServerID,DWORD accid,char name[MAX_NAMESIZE],WORD state)
{
	using namespace Cmd::Super;
	GameZone_t gameZone;

	Zebra::logger->debug("state=%u",state);
	gameZone = SuperService::getInstance().getZoneID();
	if (state & ROLEREG_STATE_TEST)//检测角色名
	{
		RoleData tmpinfo;
		//将数据COPY一份
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
			//添加成功,表示原来没有数据
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
					//名字已存在数据库中,清除缓冲中的数据
					SetRoleInfo.remove(tmpinfo.name);
					state |= ROLEREG_STATE_HAS;
					Zebra::logger->debug("名字已存在数据库中");
				}
				SuperService::dbConnPool->putHandle(handle);
			}
			else
			{
				//获取数据库句柄出错,清除缓冲中的数据
				SetRoleInfo.remove(tmpinfo.name);
				state |= ROLEREG_STATE_HAS;
				Zebra::logger->debug("获取数据库句柄出错");
			}
		}
		else
		{
			state |= ROLEREG_STATE_HAS;
			Zebra::logger->debug("名字已存在缓存中");
		}
	}
	else if (state & ROLEREG_STATE_WRITE)//回写
	{
		RoleData Tmpinfo;
		Tmpinfo.accid = accid;
		Tmpinfo.zone= gameZone.zone;
		Tmpinfo.game=  gameZone.game;
		strncpy(Tmpinfo.name,name,sizeof(Tmpinfo.name));
		Zebra::logger->debug("##########");
		Zebra::logger->debug("name = %s ",Tmpinfo.name);
		//数据库表的属性
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
				Zebra::logger->debug("回写数据库成功");
			}
			else
			{
				state &= ~ROLEREG_STATE_OK;
				Zebra::logger->debug("回写数据库失败");
			}
			SuperService::dbConnPool->putHandle(handle);
		}
		else
		{
			state &= ~ROLEREG_STATE_OK;
			Zebra::logger->debug("获取句柄失败");
		}
		//清除结构中该角色
		SetRoleInfo.remove(name);
	}
	else if (state & ROLEREG_STATE_CLEAN)//清除
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
				Zebra::logger->debug("删除角色%s失败",name);
			}
			else
			{
				state |= ROLEREG_STATE_OK;
				Zebra::logger->debug("删除角色%s成功",name);
			}
			SuperService::dbConnPool->putHandle(handle);
		}
		else
		{
			state &= ~ROLEREG_STATE_OK;
			Zebra::logger->debug("获取句柄失败");
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
			Zebra::logger->error("回写角色成功：%u,%s",accid,name);
		else
			Zebra::logger->error("回写角色失败：%u,%s",accid,name);
	}
	if (state & ROLEREG_STATE_CLEAN)
	{
		if (state & ROLEREG_STATE_OK)
			Zebra::logger->error("删除角色成功：%u,%s",accid,name);
		else
			Zebra::logger->error("删除角色失败：%u,%s",accid,name);
	}

	return true;        
}
