#include <zebra/SessionServer.h>

//sky CQueueManager类实现
CQueueManager::CQueueManager()
{
	WaitUserID.clear();
	WaitTeamID.clear();
}

CQueueManager::~CQueueManager()
{

}

//sky 将要排队的用户添加到排队序列中
void CQueueManager::Queue_AddUser(DWORD UserID)
{	
	QueueUserLock.lock();
	WaitUserID.push_back(UserID);
	QueueUserLock.unlock();
}

//sky 将要排队的队伍添加到排队序列中
void CQueueManager::Queue_AddTeam(DWORD UserID)
{
	//sky 检测下队伍是否已经存在队列中
	QueueTeamData TeamData;
	TeamData.TeamID = UserID;
	zRTime ct;
	TeamData.AddTime = ct.msecs();//sky 获取当前的真实时间
	QueueTeamLock.lock();
	WaitTeamID.push_back(TeamData);
	QueueTeamLock.unlock();
}

//sky 返回num个可以操作的用户ID并从队伍排队序列中删除
bool CQueueManager::Queue_RemoveTeam(DWORD * UserID, int num)
{
	bool returnData = false;
	std::vector<DWORD> DelTeamID;
	std::vector<DWORD> DelUserID;

	std::vector<QueueTeamData>::iterator iter;
	std::vector<DWORD>::iterator iter1;
	GlobalTeamIndex * TeamM = GlobalTeamIndex::getInstance()->getInstance();

	QueueTeamLock.lock();

	int i = 0;	//sky 已经从序列中提取的人数
	//sky 从队伍列表提取足够人员
	for(iter=WaitTeamID.begin(); iter!=WaitTeamID.end(); iter++)
	{
		Team * teamData = TeamM->GetpTeam(iter->TeamID);
		//sky 如果队伍的人数和要求的人数一样并且单前调用没有提取过用户
		if(teamData && i == 0 && teamData->GetMemberNum() == num)
		{
			//sky 将所有的队员都提交上去
			for(int n=0; n<teamData->GetMemberNum(); n++)
			{
					UserID[i] = teamData->GetMemberID(n);
					i++;
			}

			//sky 把队伍放到移除队列中
			DelTeamID.push_back(iter->TeamID);

			//sky 结束提取
			returnData = true;
			break;
		}
		else if(teamData && teamData->GetMemberNum() <= num-i)
		{
			//sky 获取当前的真实时间
			zRTime ct;
			//sky 等待时间已经超过1分钟或者已经取拉一队不足的队伍
			if((i!=0) || (ct.msecs() - iter->AddTime > 60000))
			{
				for(int n=0; n<teamData->GetMemberNum(); n++)
				{
					DWORD id = teamData->GetMemberID(n);
					UserID[i] = teamData->GetMemberID(n);
					i++;
				}

				//sky 把队伍放到移除队列中
				DelTeamID.push_back(iter->TeamID);
			}			
		}

		if(i == num)
		{
			//sky 结束提取
			returnData = true;
			break;
		}
	}

	//sky 无法从队伍中提够人物就从单个玩家队列中提取补足
	if(i!=0 && i<num)
	{
		for(iter1=WaitUserID.begin(); iter1!=WaitUserID.end(); iter1++)
		{
			UserID[i] = *(iter1);
			i++;

			if(i == num)
			{
				//sky 结束提取
				DelUserID.push_back(*(iter1));	//sky 把角色放到移除容器里去
				returnData = true;
				break;
			}
		}
	}

	if(returnData)	//sky 提取成功就从排队队列中移除队伍和用户
	{
		std::vector<DWORD>::iterator it;

		if(!DelTeamID.empty())
		{
			for(it=DelTeamID.begin(); it!=DelTeamID.end(); it++)
			{
				for(iter=WaitTeamID.begin(); iter!=WaitTeamID.end();)
				{
					if(iter->TeamID == *(it))
						iter = WaitTeamID.erase(iter);
					else
						iter++;
				}
			}
		}

		if(!DelUserID.empty())
		{
			for(it=DelUserID.begin(); it!=DelUserID.end(); it++)
			{
				for(iter1=WaitUserID.begin(); iter1!=WaitUserID.end();)
				{
					if(*(iter1) == *(it))
						iter1 = WaitUserID.erase(iter1);
					else
						iter1++;
				}
			}
		}
	}

	QueueTeamLock.unlock();

	return returnData;
}

//sky 返回num个可以操作的用户ID并从排队序列中删除
bool CQueueManager::Queue_RemoveUser(DWORD * UserID, int num)
{
	bool returnData = false;
	if(WaitUserID.size() < num)
		return returnData;

	std::vector<DWORD> DelUserID;
	std::vector<DWORD>::iterator iter1;

	QueueUserLock.lock();

	int i = 0;
	std::vector<DWORD>::iterator iter;
	for(iter=WaitUserID.begin(); iter!=WaitUserID.end(); iter++)
	{
		DWORD id = *(iter);
		UserID[i] = id;
		i++;

		if(i == num)
		{
			DelUserID.push_back(*(iter));	//sky 把角色放到移除容器里去
			returnData = true;
			break;
		}
	}

	//sky 提取完毕把提取成功的用户删除掉
	if(returnData)
	{
		std::vector<DWORD>::iterator it;
		if(!DelUserID.empty())
		{
			for(it=DelUserID.begin(); it!=DelUserID.end(); it++)
			{
				for(iter1=WaitUserID.begin(); iter1!=WaitUserID.end();)
				{
					if(*(iter1) == *(it))
						iter1 = WaitUserID.erase(iter1);
					else
						iter1++;
				}
			}
		}
	}

	QueueUserLock.unlock();

	return returnData;
}

//sky 用户是否已经存在当前队列中
bool CQueueManager::IfUserInWait(BYTE type, DWORD UserID)
{
	

	switch(type)
	{
	case 0:		//sky 用户排
		{
			std::vector<DWORD>::iterator it;
			for(it=WaitUserID.begin(); it!=WaitUserID.end(); it++)
			{
				if( UserID == *(it) )
				{
					return true;
				}
			}
		}
		break;
	case 1:		//sky 队排
		{
			std::vector<QueueTeamData>::iterator it;
			for(it=WaitTeamID.begin(); it!=WaitTeamID.end(); it++)
			{
				if( UserID == it->TeamID )
				{
					return true;
				}
			}
		}
		break;
	default:
		break;
	}

	return false;
}

//sky CQueuingManager类实现

CQueuingManager::CQueuingManager()
{
	MapIDMove = 0;
	NewMapLock = true;
	ManagerKey = 0;
	MaxTeamnum = 0;
	MaxCampNum = 0;
	
	sceneMapID.clear();

	WORD level = MIN_ARENA_USERLEVEL;
	while(level <= MAX_ARENA_USERLEVEL)
	{
		Camp[level].clear();
		level += 10;
	}

	level = MIN_ARENA_USERLEVEL;
	while(level <= MAX_ARENA_USERLEVEL)
	{
		CQueueManager* pQueue = new CQueueManager;
		Queuing[level] = pQueue;
		level += 10;
	}

	bAccess = true;
}

CQueuingManager::~CQueuingManager()
{
	std::map<DWORD, CQueueManager*>::iterator iter;
	for(iter=Queuing.begin(); iter!=Queuing.end(); iter++)
	{
		if(iter->second)
			delete iter->second;
	}
}

//sky 将要排队的用户添加到排队序列中
void CQueuingManager::Queuing_AddUser(DWORD UserID, BYTE UserType)
{
	switch(UserType)
	{
	case 0:		//sky 用户排队
		{
			UserSession * use = UserSessionManager::getInstance()->getUserByID(UserID);
			if(use)
			{
				if(use->level >= MIN_ARENA_USERLEVEL && use->level <= MAX_ARENA_USERLEVEL)
				{
					int index = use->level / 10 * 10;

					if(index >= MIN_ARENA_USERLEVEL && index <= MAX_ARENA_USERLEVEL)
						Queuing[index]->Queue_AddUser(UserID);
				}
			}
		}
		break;
	case 1:		//sky 队伍排队
		{
			Team * pteam = GlobalTeamIndex::getInstance()->GetpTeam(UserID);

			if(pteam)
			{
				UserSession * Leader = UserSessionManager::getInstance()->getUserByID(pteam->GetLeader());
				if(Leader)
				{
					if(Leader->level >= MIN_ARENA_USERLEVEL && Leader->level <= MAX_ARENA_USERLEVEL)
					{
						int index = Leader->level / 10 * 10;

						if(index >= MIN_ARENA_USERLEVEL && Leader->level <= MAX_ARENA_USERLEVEL)
							Queuing[index]->Queue_AddTeam(UserID);
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

//sky 执行查询分配任务(主操作函数)
void CQueuingManager::Queuing_Main()
{
	//sky 用来存放阵营成员缓冲
	DWORD * user = new DWORD[MaxTeamnum];
	memset(user, 0, sizeof(DWORD)*MaxTeamnum);

	int index = MIN_ARENA_USERLEVEL;
	for(int i=0; i<(MAX_ARENA_USERLEVEL/MIN_ARENA_USERLEVEL); i++)
	{
		if(Camp[index].size() < MaxCampNum) //sky 阵营小于开一个战场需求的数量时
		{
			bool bReturn = false;
			//sky 成功取出阵营数
			int CampOK = Camp[index].size();
			while(CampOK < MaxCampNum)
			{
				if(bAccess)	//sky 队伍优先模式
				{
					bReturn = Queuing[index]->Queue_RemoveTeam(user, MaxTeamnum);

					if(!bReturn)	//sky 队伍队列中不足人数
						bReturn = Queuing[index]->Queue_RemoveUser(user, MaxTeamnum); //sky 冲单人玩家中提取

					if(bReturn)
					{
						CampData Data;
						Data.Teamnum = MaxTeamnum;
						for(int i=0; i<MaxTeamnum; i++)
						{
							DWORD id = user[i];
							Data.UserID.push_back(id);
							memset(user, 0, sizeof(DWORD)*MaxTeamnum);
						}

						Camp[index].push_back(Data);
						CampOK++;
						bAccess = false;
					}
					else
					{
						memset(user, 0, sizeof(DWORD)*MaxTeamnum);
						break;
					}
				}
				else
				{
					bReturn = Queuing[index]->Queue_RemoveUser(user, MaxTeamnum);

					if(!bReturn)	//sky 队伍队列中不足人数
						bReturn = Queuing[index]->Queue_RemoveTeam(user, MaxTeamnum); //sky 冲队伍玩家中提取

					if(bReturn)
					{
						CampData Data;
						Data.Teamnum = MaxTeamnum;
						for(int i=0; i<MaxTeamnum; i++)
						{
							DWORD id = user[i];
							Data.UserID.push_back(id);
							memset(user, 0, sizeof(DWORD)*MaxTeamnum);
						}

						Camp[index].push_back(Data);
						CampOK++;

						bAccess = true;
					}
					else
					{
						memset(user, 0, sizeof(DWORD)*MaxTeamnum);
						break;
					}
				}
			}
		}
		else
		{
			if(!sceneMapID.empty())	//sky 如果有现成的战场将已经双方阵营的人送入战场
			{
				DistributionUser(index);
			}
			else	//sky 通知场景生成战场地图
			{
				SendSceneNewMap(ManagerKey);
			}

		}

		//sky 将map的下标下移
		index += 10;
	}

	delete user;
}

bool CQueuingManager::DistributionUser(int index)					//sky 分配用户到已经生成好的地图中
{
	if((index>MIN_ARENA_USERLEVEL && index<MAX_ARENA_USERLEVEL) && !sceneMapID.empty())
	{
		//sky 取得战场ID
		std::vector<SceneMapData>::iterator mapIdIt = sceneMapID.begin();
		SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByID(mapIdIt->SceneID);
		if(pScene)
		{
			Cmd::Session::t_Sports_MoveSecen cmd;
			std::map<DWORD, tyCamp>::iterator iter;

			//sky 根据传来的阵营索引找到阵营对象
			iter = Camp.find(index);
			if(iter != Camp.end())
			{
				int i = 0;
				tyCamp::iterator it;
				//sky 开始遍历阵营
				for(it=iter->second.begin(); it!=iter->second.end();)
				{
					//sky 遍历提取用户发送跳转地图消息
					for(int n=0; n<it->UserID.size(); n++)
					{
						UserSession * use = UserSessionManager::getInstance()->getUserByID(it->UserID[n]);
						if(use)
						{
							Cmd::Session::t_changeScene_SceneSession ret;
							ret.id = use->id;
							ret.x = mapIdIt->pos[i].x;
							ret.y = mapIdIt->pos[i].y;
							ret.map_id = pScene->id;
							ret.temp_id = pScene->tempid;
							strncpy((char *)ret.map_file,pScene->file.c_str(),MAX_NAMESIZE);
							strncpy((char *)ret.map_name,pScene->name,MAX_NAMESIZE);

							use->scene->sendCmd(&ret,sizeof(ret));
						}
						else
							n++;	//sky 不管消息是否发送成功都自加(掉人的情况也这样处理)

						//sky 如果一个阵营的最大人数已经足够
						if(n == MaxTeamnum)
							break;
					}
					i++;	//sky 完成一个阵营的传送就自加
					it = iter->second.erase(it);	//sky 同是把这个阵营从阵营容器中删除

					//sky 如果已经满足一个战场需求的最大战场数
					if(i == MaxCampNum)
						break;
				}

				//sky 全部传送完毕将场景ID从容器中删除
				sceneMapID.erase(mapIdIt);
			}
		}
	}

	return false;
}

bool CQueuingManager::SendSceneNewMap(int index)						//sky 通知战场或者竞技场创建一个地图
{
	if(!setBattleTask.empty() && NewMapLock)
	{
		Cmd::Session::t_Sports_RequestMap cmd;
		cmd.MapBaseID = MapBaseID;
		cmd.AddMeType = index;

		if(MapIDMove >= setBattleTask.size())
			MapIDMove = 0;

		std::set<SessionTask *>::iterator iter = setBattleTask.begin();

		for(int i=0; i<MapIDMove; i++)
			iter++;
			
		if(iter != setBattleTask.end())
		{
			SessionTask * pTask = *iter;
			pTask->sendCmd(&cmd, sizeof(Cmd::Session::t_Sports_RequestMap));
			//sky 已经通知一个场景去申请战场就把锁设为假
			NewMapLock = false;
			return true;
		}
	}
	else
		Zebra::logger->error("通知战场场景时没找到任何可用的场景!");

	return false;
}

bool CQueuingManager::Queuing_AddScene(Cmd::Session::t_Sports_ReturnMapID * cmd)		//sky 将一个创建好的战场地图的ID放到战场容器里
{
	if(cmd->dwID == 0)
		return false;

	SceneMapData data;
	data.SceneID = cmd->dwID;
	for(int i=0; i<20; i++)
	{
		data.pos[i].x = cmd->pos[i].x;
		data.pos[i].y = cmd->pos[i].y;
	}

	sceneMapID.push_back(data);
	return true;
}

//sky 用户是否已经存在当前战场队列中
bool CQueuingManager::IfUserInWaits(BYTE type, DWORD UserID)
{
	bool bReturn = false;
	int index = MIN_ARENA_USERLEVEL;
	for(int i=0; i<(MAX_ARENA_USERLEVEL/MIN_ARENA_USERLEVEL); i++)
	{
		bReturn = Queuing[index]->IfUserInWait(type, UserID);

		//sky 只要一个为真就代表存在既利马返回
		if(bReturn)
			return bReturn;

		index += 10;
	}

	DWORD id = UserID;

	if(type == 1)
	{
		Team * team = GlobalTeamIndex::getInstance()->GetpTeam(UserID);
		if(!team)
			return true;

		//sky 队排的话就取队长的值
		id = team->GetLeader();

	}

	//sky 还要遍历下用户是否在被提取成功的阵营里
	std::map<DWORD, tyCamp>::iterator iter;
	for(iter=Camp.begin(); iter!=Camp.end(); iter++)
	{
		tyCamp::iterator it;
		//sky 开始遍历阵营
		for(it=iter->second.begin(); it!=iter->second.end(); it++)
		{
			//sky 遍历提取用户发送跳转地图消息
			for(int i=0; i<it->UserID.size(); i++)
			{
				//sky 检测值是否已经存在
				if(id == it->UserID[i])
					return true;
			}
		}
	}

	return bReturn;
}

//sky CArenaManager类实现
CArenaManager::CArenaManager()
{
	setBattleTask.clear();
	LoadXmlToArena();
}

CArenaManager::~CArenaManager()
{
	std::map<DWORD,CQueuingManager*>::iterator it;
	for(it=_ArenaMap.begin(); it!=_ArenaMap.end(); it++)
	{
		if(it->second)
			delete it->second;
	}
}

//sky 将要排队的用户添加到相应的战场排队序列中
void CArenaManager::Arena_AddUser(Cmd::Session::t_Sports_AddMeToQueuing * cmd)
{
	bool AddOk = false;
	std::map<DWORD, CQueuingManager*>::iterator iter;
	switch(cmd->Type)
	{
	case 0:
		{
			UserSession * user = UserSessionManager::getInstance()->getUserByID(cmd->UserID);
			if(user)
			{
				if(user->level < MIN_ARENA_USERLEVEL)
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "您没有达到战场的最小等级要求");
				//sky 有队伍的人是不可以私排的
 				else if(user->teamid != 0)
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "抱歉有队伍的用户不可以个人排队！请退出队伍后再来");
				//sky 添加之前先检测下用户排过队没
				else if(IfUserInQueuing(cmd->Type, cmd->UserID))
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "一个用户只能排一个队列！请不可以重复排队");
				else
				{
					AddOk = true;
					if(user)
						user->sendSysChat(Cmd::INFO_TYPE_GAME, "您已加入一个战场排队队列！");
				}
			}
		}
		break;
	case 1:
		{
			Team * team = GlobalTeamIndex::getInstance()->GetpTeam(cmd->UserID);

			if(!team)
				break;

			//sky 添加之前先检测下用户排过队没
			if(IfUserInQueuing(cmd->Type, cmd->UserID))
			{
				 UserSession * user = UserSessionManager::getInstance()->getUserByID(team->GetLeader());
				 if(user)
					 user->sendSysChat(Cmd::INFO_TYPE_GAME, "一个队伍只能排一个队列！请不可以重复排队");
			}
			else
			{
				AddOk = true;
				UserSession * user = UserSessionManager::getInstance()->getUserByID(team->GetLeader());
				if(user)
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "您的队伍已加入一个战场排队队列！");
			}
		}
		break;
	default:
		break;
	}
	
	if(AddOk)
	{
		iter = _ArenaMap.find(cmd->AddMeType);

		if(iter!=_ArenaMap.end())
		{
			iter->second->Queuing_AddUser(cmd->UserID, cmd->Type);
		}
	}
}

//sky 战场队列管理主函数
void CArenaManager::Arena_timer()
{
	std::map<DWORD,CQueuingManager*>::iterator it;
	for(it=_ArenaMap.begin(); it!=_ArenaMap.end(); it++)
	{
		it->second->Queuing_Main();
	}
}

//sky 加载战场队列管理器配置文件生成对应的战场队列管理器
bool CArenaManager::LoadXmlToArena()
{
	char CoolFileName[MAX_PATH];
	strcpy( CoolFileName, "Arena/battlequeue.xml" );

	zXMLParser xml;
	if (!xml.initFile(CoolFileName))
	{
		Zebra::logger->error("加载session战场队列管理配置文件 %s 失败",CoolFileName);
		return false;
	}

	xmlNodePtr root;

	root = xml.getRootNode("event");

	if (root)
	{
		xmlNodePtr node = xml.getChildNode(root,"battle"); 

		while (node) 
		{
			WORD keyID = 0;
			WORD maxCamp = 0;
			WORD maxUser = 0;
			DWORD BaseID = 0;

			CQueuingManager * Queuing = new CQueuingManager;

			if(!xml.getNodePropNum(node, "Key", &keyID,sizeof(WORD))) 
			{
				return false;
			}

			//sky 取出地图源ID
			if(!xml.getNodePropNum(node, "mapID", &BaseID, sizeof(DWORD)))
			{
				return false;
			}
			
			//sky 取出最大阵营数
			if(!xml.getNodePropNum(node, "maxCamp", &maxCamp,sizeof(WORD))) 
			{
				return false;
			}

			//sky 取出每个阵营最大用户数量
			if(!xml.getNodePropNum(node, "maxuser", &maxUser,sizeof(WORD))) 
			{
				return false;
			}			

			Queuing->ManagerKey = keyID; 
			Queuing->MaxCampNum = maxCamp;
			Queuing->MaxTeamnum = maxUser;
			Queuing->MapBaseID = BaseID;

			_ArenaMap[keyID] = Queuing;
			node = xml.getNextNode(node,"battle");
		}

		if(              !_ArenaMap.empty())
			printf("成功传建 %d 个战场队列管理器\n", _ArenaMap.size());

		return true;
	}	

	return false;
}

//sky 将场景连接放到列表容器中
void CArenaManager::InsertBattleTask(SessionTask * pTask)
{
	setBattleTask.insert(pTask).second;
	printf("ArenaManager收到战场场景通知 TaskID:%d\n", pTask->getID());
}

//sky 把具体战场放到index索引的队列管理类下
bool CArenaManager::AddMapToQueuing(Cmd::Session::t_Sports_ReturnMapID * cmd)
{
	if(cmd->AddMeType == 0 || cmd->dwID == 0)
		return false;

	std::map<DWORD,CQueuingManager*>::iterator iter;
	iter = _ArenaMap.find(cmd->AddMeType);
	if(iter != _ArenaMap.end())
		iter->second->Queuing_AddScene(cmd);

	return true;
}

//sky 设置特定队列管理类的场景申请锁
void CArenaManager::NewMap_Lock(DWORD index, bool block)
{
	if(index == 0)
		return;

	std::map<DWORD,CQueuingManager*>::iterator iter;
	iter = _ArenaMap.find(index);
	if(iter != _ArenaMap.end())
		iter->second->NewMapLock = block;
}

//sky 移动特定队列管理类的场景偏移
void CArenaManager::MoveSceneM(DWORD index)
{
	if(index == 0)
		return;

	std::map<DWORD,CQueuingManager*>::iterator iter;
	iter = _ArenaMap.find(index);
	if(iter != _ArenaMap.end())
		iter->second->MapIDMove++;
}

//sky 用户是否已经存在队列管理器中
bool CArenaManager::IfUserInQueuing(BYTE type, DWORD UserID)
{
	bool bReturn = false;
	std::map<DWORD,CQueuingManager*>::iterator it;
	for(it=_ArenaMap.begin(); it!=_ArenaMap.end(); it++)
	{
		bReturn = it->second->IfUserInWaits(type, UserID);

		//sky 只要一个为真既代表已经存在就直接返回
		if(bReturn)
			return bReturn;
	}

	return bReturn;
}