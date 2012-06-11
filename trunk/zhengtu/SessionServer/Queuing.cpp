#include <zebra/SessionServer.h>

//sky CQueueManager��ʵ��
CQueueManager::CQueueManager()
{
	WaitUserID.clear();
	WaitTeamID.clear();
}

CQueueManager::~CQueueManager()
{

}

//sky ��Ҫ�Ŷӵ��û���ӵ��Ŷ�������
void CQueueManager::Queue_AddUser(DWORD UserID)
{	
	QueueUserLock.lock();
	WaitUserID.push_back(UserID);
	QueueUserLock.unlock();
}

//sky ��Ҫ�ŶӵĶ�����ӵ��Ŷ�������
void CQueueManager::Queue_AddTeam(DWORD UserID)
{
	//sky ����¶����Ƿ��Ѿ����ڶ�����
	QueueTeamData TeamData;
	TeamData.TeamID = UserID;
	zRTime ct;
	TeamData.AddTime = ct.msecs();//sky ��ȡ��ǰ����ʵʱ��
	QueueTeamLock.lock();
	WaitTeamID.push_back(TeamData);
	QueueTeamLock.unlock();
}

//sky ����num�����Բ������û�ID���Ӷ����Ŷ�������ɾ��
bool CQueueManager::Queue_RemoveTeam(DWORD * UserID, int num)
{
	bool returnData = false;
	std::vector<DWORD> DelTeamID;
	std::vector<DWORD> DelUserID;

	std::vector<QueueTeamData>::iterator iter;
	std::vector<DWORD>::iterator iter1;
	GlobalTeamIndex * TeamM = GlobalTeamIndex::getInstance()->getInstance();

	QueueTeamLock.lock();

	int i = 0;	//sky �Ѿ�����������ȡ������
	//sky �Ӷ����б���ȡ�㹻��Ա
	for(iter=WaitTeamID.begin(); iter!=WaitTeamID.end(); iter++)
	{
		Team * teamData = TeamM->GetpTeam(iter->TeamID);
		//sky ��������������Ҫ�������һ�����ҵ�ǰ����û����ȡ���û�
		if(teamData && i == 0 && teamData->GetMemberNum() == num)
		{
			//sky �����еĶ�Ա���ύ��ȥ
			for(int n=0; n<teamData->GetMemberNum(); n++)
			{
					UserID[i] = teamData->GetMemberID(n);
					i++;
			}

			//sky �Ѷ���ŵ��Ƴ�������
			DelTeamID.push_back(iter->TeamID);

			//sky ������ȡ
			returnData = true;
			break;
		}
		else if(teamData && teamData->GetMemberNum() <= num-i)
		{
			//sky ��ȡ��ǰ����ʵʱ��
			zRTime ct;
			//sky �ȴ�ʱ���Ѿ�����1���ӻ����Ѿ�ȡ��һ�Ӳ���Ķ���
			if((i!=0) || (ct.msecs() - iter->AddTime > 60000))
			{
				for(int n=0; n<teamData->GetMemberNum(); n++)
				{
					DWORD id = teamData->GetMemberID(n);
					UserID[i] = teamData->GetMemberID(n);
					i++;
				}

				//sky �Ѷ���ŵ��Ƴ�������
				DelTeamID.push_back(iter->TeamID);
			}			
		}

		if(i == num)
		{
			//sky ������ȡ
			returnData = true;
			break;
		}
	}

	//sky �޷��Ӷ������ṻ����ʹӵ�����Ҷ�������ȡ����
	if(i!=0 && i<num)
	{
		for(iter1=WaitUserID.begin(); iter1!=WaitUserID.end(); iter1++)
		{
			UserID[i] = *(iter1);
			i++;

			if(i == num)
			{
				//sky ������ȡ
				DelUserID.push_back(*(iter1));	//sky �ѽ�ɫ�ŵ��Ƴ�������ȥ
				returnData = true;
				break;
			}
		}
	}

	if(returnData)	//sky ��ȡ�ɹ��ʹ��ŶӶ������Ƴ�������û�
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

//sky ����num�����Բ������û�ID�����Ŷ�������ɾ��
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
			DelUserID.push_back(*(iter));	//sky �ѽ�ɫ�ŵ��Ƴ�������ȥ
			returnData = true;
			break;
		}
	}

	//sky ��ȡ��ϰ���ȡ�ɹ����û�ɾ����
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

//sky �û��Ƿ��Ѿ����ڵ�ǰ������
bool CQueueManager::IfUserInWait(BYTE type, DWORD UserID)
{
	

	switch(type)
	{
	case 0:		//sky �û���
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
	case 1:		//sky ����
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

//sky CQueuingManager��ʵ��

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

//sky ��Ҫ�Ŷӵ��û���ӵ��Ŷ�������
void CQueuingManager::Queuing_AddUser(DWORD UserID, BYTE UserType)
{
	switch(UserType)
	{
	case 0:		//sky �û��Ŷ�
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
	case 1:		//sky �����Ŷ�
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

//sky ִ�в�ѯ��������(����������)
void CQueuingManager::Queuing_Main()
{
	//sky ���������Ӫ��Ա����
	DWORD * user = new DWORD[MaxTeamnum];
	memset(user, 0, sizeof(DWORD)*MaxTeamnum);

	int index = MIN_ARENA_USERLEVEL;
	for(int i=0; i<(MAX_ARENA_USERLEVEL/MIN_ARENA_USERLEVEL); i++)
	{
		if(Camp[index].size() < MaxCampNum) //sky ��ӪС�ڿ�һ��ս�����������ʱ
		{
			bool bReturn = false;
			//sky �ɹ�ȡ����Ӫ��
			int CampOK = Camp[index].size();
			while(CampOK < MaxCampNum)
			{
				if(bAccess)	//sky ��������ģʽ
				{
					bReturn = Queuing[index]->Queue_RemoveTeam(user, MaxTeamnum);

					if(!bReturn)	//sky ��������в�������
						bReturn = Queuing[index]->Queue_RemoveUser(user, MaxTeamnum); //sky �嵥���������ȡ

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

					if(!bReturn)	//sky ��������в�������
						bReturn = Queuing[index]->Queue_RemoveTeam(user, MaxTeamnum); //sky ������������ȡ

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
			if(!sceneMapID.empty())	//sky ������ֳɵ�ս�����Ѿ�˫����Ӫ��������ս��
			{
				DistributionUser(index);
			}
			else	//sky ֪ͨ��������ս����ͼ
			{
				SendSceneNewMap(ManagerKey);
			}

		}

		//sky ��map���±�����
		index += 10;
	}

	delete user;
}

bool CQueuingManager::DistributionUser(int index)					//sky �����û����Ѿ����ɺõĵ�ͼ��
{
	if((index>MIN_ARENA_USERLEVEL && index<MAX_ARENA_USERLEVEL) && !sceneMapID.empty())
	{
		//sky ȡ��ս��ID
		std::vector<SceneMapData>::iterator mapIdIt = sceneMapID.begin();
		SceneSession * pScene = SceneSessionManager::getInstance()->getSceneByID(mapIdIt->SceneID);
		if(pScene)
		{
			Cmd::Session::t_Sports_MoveSecen cmd;
			std::map<DWORD, tyCamp>::iterator iter;

			//sky ���ݴ�������Ӫ�����ҵ���Ӫ����
			iter = Camp.find(index);
			if(iter != Camp.end())
			{
				int i = 0;
				tyCamp::iterator it;
				//sky ��ʼ������Ӫ
				for(it=iter->second.begin(); it!=iter->second.end();)
				{
					//sky ������ȡ�û�������ת��ͼ��Ϣ
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
							n++;	//sky ������Ϣ�Ƿ��ͳɹ����Լ�(���˵����Ҳ��������)

						//sky ���һ����Ӫ����������Ѿ��㹻
						if(n == MaxTeamnum)
							break;
					}
					i++;	//sky ���һ����Ӫ�Ĵ��;��Լ�
					it = iter->second.erase(it);	//sky ͬ�ǰ������Ӫ����Ӫ������ɾ��

					//sky ����Ѿ�����һ��ս����������ս����
					if(i == MaxCampNum)
						break;
				}

				//sky ȫ��������Ͻ�����ID��������ɾ��
				sceneMapID.erase(mapIdIt);
			}
		}
	}

	return false;
}

bool CQueuingManager::SendSceneNewMap(int index)						//sky ֪ͨս�����߾���������һ����ͼ
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
			//sky �Ѿ�֪ͨһ������ȥ����ս���Ͱ�����Ϊ��
			NewMapLock = false;
			return true;
		}
	}
	else
		Zebra::logger->error("֪ͨս������ʱû�ҵ��κο��õĳ���!");

	return false;
}

bool CQueuingManager::Queuing_AddScene(Cmd::Session::t_Sports_ReturnMapID * cmd)		//sky ��һ�������õ�ս����ͼ��ID�ŵ�ս��������
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

//sky �û��Ƿ��Ѿ����ڵ�ǰս��������
bool CQueuingManager::IfUserInWaits(BYTE type, DWORD UserID)
{
	bool bReturn = false;
	int index = MIN_ARENA_USERLEVEL;
	for(int i=0; i<(MAX_ARENA_USERLEVEL/MIN_ARENA_USERLEVEL); i++)
	{
		bReturn = Queuing[index]->IfUserInWait(type, UserID);

		//sky ֻҪһ��Ϊ��ʹ�����ڼ�������
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

		//sky ���ŵĻ���ȡ�ӳ���ֵ
		id = team->GetLeader();

	}

	//sky ��Ҫ�������û��Ƿ��ڱ���ȡ�ɹ�����Ӫ��
	std::map<DWORD, tyCamp>::iterator iter;
	for(iter=Camp.begin(); iter!=Camp.end(); iter++)
	{
		tyCamp::iterator it;
		//sky ��ʼ������Ӫ
		for(it=iter->second.begin(); it!=iter->second.end(); it++)
		{
			//sky ������ȡ�û�������ת��ͼ��Ϣ
			for(int i=0; i<it->UserID.size(); i++)
			{
				//sky ���ֵ�Ƿ��Ѿ�����
				if(id == it->UserID[i])
					return true;
			}
		}
	}

	return bReturn;
}

//sky CArenaManager��ʵ��
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

//sky ��Ҫ�Ŷӵ��û���ӵ���Ӧ��ս���Ŷ�������
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
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "��û�дﵽս������С�ȼ�Ҫ��");
				//sky �ж�������ǲ�����˽�ŵ�
 				else if(user->teamid != 0)
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "��Ǹ�ж�����û������Ը����Ŷӣ����˳����������");
				//sky ���֮ǰ�ȼ�����û��Ź���û
				else if(IfUserInQueuing(cmd->Type, cmd->UserID))
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "һ���û�ֻ����һ�����У��벻�����ظ��Ŷ�");
				else
				{
					AddOk = true;
					if(user)
						user->sendSysChat(Cmd::INFO_TYPE_GAME, "���Ѽ���һ��ս���ŶӶ��У�");
				}
			}
		}
		break;
	case 1:
		{
			Team * team = GlobalTeamIndex::getInstance()->GetpTeam(cmd->UserID);

			if(!team)
				break;

			//sky ���֮ǰ�ȼ�����û��Ź���û
			if(IfUserInQueuing(cmd->Type, cmd->UserID))
			{
				 UserSession * user = UserSessionManager::getInstance()->getUserByID(team->GetLeader());
				 if(user)
					 user->sendSysChat(Cmd::INFO_TYPE_GAME, "һ������ֻ����һ�����У��벻�����ظ��Ŷ�");
			}
			else
			{
				AddOk = true;
				UserSession * user = UserSessionManager::getInstance()->getUserByID(team->GetLeader());
				if(user)
					user->sendSysChat(Cmd::INFO_TYPE_GAME, "���Ķ����Ѽ���һ��ս���ŶӶ��У�");
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

//sky ս�����й���������
void CArenaManager::Arena_timer()
{
	std::map<DWORD,CQueuingManager*>::iterator it;
	for(it=_ArenaMap.begin(); it!=_ArenaMap.end(); it++)
	{
		it->second->Queuing_Main();
	}
}

//sky ����ս�����й����������ļ����ɶ�Ӧ��ս�����й�����
bool CArenaManager::LoadXmlToArena()
{
	char CoolFileName[MAX_PATH];
	strcpy( CoolFileName, "Arena/battlequeue.xml" );

	zXMLParser xml;
	if (!xml.initFile(CoolFileName))
	{
		Zebra::logger->error("����sessionս�����й��������ļ� %s ʧ��",CoolFileName);
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

			//sky ȡ����ͼԴID
			if(!xml.getNodePropNum(node, "mapID", &BaseID, sizeof(DWORD)))
			{
				return false;
			}
			
			//sky ȡ�������Ӫ��
			if(!xml.getNodePropNum(node, "maxCamp", &maxCamp,sizeof(WORD))) 
			{
				return false;
			}

			//sky ȡ��ÿ����Ӫ����û�����
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
			printf("�ɹ����� %d ��ս�����й�����\n", _ArenaMap.size());

		return true;
	}	

	return false;
}

//sky ���������ӷŵ��б�������
void CArenaManager::InsertBattleTask(SessionTask * pTask)
{
	setBattleTask.insert(pTask).second;
	printf("ArenaManager�յ�ս������֪ͨ TaskID:%d\n", pTask->getID());
}

//sky �Ѿ���ս���ŵ�index�����Ķ��й�������
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

//sky �����ض����й�����ĳ���������
void CArenaManager::NewMap_Lock(DWORD index, bool block)
{
	if(index == 0)
		return;

	std::map<DWORD,CQueuingManager*>::iterator iter;
	iter = _ArenaMap.find(index);
	if(iter != _ArenaMap.end())
		iter->second->NewMapLock = block;
}

//sky �ƶ��ض����й�����ĳ���ƫ��
void CArenaManager::MoveSceneM(DWORD index)
{
	if(index == 0)
		return;

	std::map<DWORD,CQueuingManager*>::iterator iter;
	iter = _ArenaMap.find(index);
	if(iter != _ArenaMap.end())
		iter->second->MapIDMove++;
}

//sky �û��Ƿ��Ѿ����ڶ��й�������
bool CArenaManager::IfUserInQueuing(BYTE type, DWORD UserID)
{
	bool bReturn = false;
	std::map<DWORD,CQueuingManager*>::iterator it;
	for(it=_ArenaMap.begin(); it!=_ArenaMap.end(); it++)
	{
		bReturn = it->second->IfUserInWaits(type, UserID);

		//sky ֻҪһ��Ϊ��ȴ����Ѿ����ھ�ֱ�ӷ���
		if(bReturn)
			return bReturn;
	}

	return bReturn;
}