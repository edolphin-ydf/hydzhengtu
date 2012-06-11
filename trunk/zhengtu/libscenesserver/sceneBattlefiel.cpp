#include <zebra/SceneBattlefiel.h>

/**
* \brief 保存动态场景
*
*
*/
bool GangScene::save()
{
	//TODO  保存到数据库
	return true;
}

/**
* \brief 构造函数
*
*
*/
GangScene::GangScene():Scene(),_one_min(60)
{
	GangmapID = 0;
	//sky 阵营数量
	CampNum = 2;
	//sky 阵营最大成员数
	CampUserNum = 1;
	//sky 阵营关系阵营关系(0:对抗(可以杀死对方阵营) 1:对抗(不可以杀死对方阵营) 2:合作)
	CampRel = 0;
	//sky 胜利条件类型
	VictoryType = BATTLEFIELF_WINNT_KILLALLUSER;
	//sky 胜利条件的补充数值
	VictoryData = 0;
	//sky 阵营起始传送坐标点
	CampPos.clear();
	//sky 击杀敌对阵营用户奖励战场点数
	KillPoint = 0;
	//sky 复活类型
	reliveType = 0;
	//sku 复活倒计时
	reliveTime = 30;
	//sky 杀死对方阵营成员奖励点数
	killPoint = 0;
	//sky 战场最大持续时间
	DWORD GameTime = 10;
	//sky 时间到后的判断的默认胜负条件
	defaultvictory = BATTLEFIELF_WINNT_TIME_KILLNPCMUCH;
	//sky 胜利阵营ID初始化
	WinnerCamp = 0;

	GameStart = 0;
	StartTime = 60;
	OverTime = 60;
	bVictory = true;
	bfailure = true;
	bpenalty = true;
}

/**
* \brief 析构函数
*
*
*/
GangScene::~GangScene()
{
	std::map<DWORD ,BCampData* >::iterator iter;
	for(iter=camp.begin(); iter!=camp.end(); iter++)
	{
		if(iter->second)
			delete iter->second;
	}
}


//sky 初始化配置数据
void GangScene::InitData()
{
	if(DulesFileName[0] == 0)
		return;

	zXMLParser xml;
	if (!xml.initFile(DulesFileName))
	{
		Zebra::logger->error("%s 加载战场配置文件 %s 失败",this->name, DulesFileName);
		return;
	}

	xmlNodePtr root;

	root = xml.getRootNode("event");

	if (root)
	{
		xmlNodePtr node = xml.getChildNode(root, NULL);  
		while(node) 
		{
			if(strcmp( (char *)node->name, "CampDules" ) == 0)
			{
				if(!xml.getNodePropNum( node, "CampNum", &CampNum, sizeof(CampNum)))
					Zebra::logger->error("%s 加载战场配置 %s CampNum失败!", this->name, DulesFileName);
				if(!xml.getNodePropNum( node, "CampUserNum", &CampUserNum, sizeof(CampUserNum)))
					Zebra::logger->error("%s 加载战场配置 %s CampUserNum失败!", this->name, DulesFileName);
				xml.getNodePropNum( node, "CampRel", &CampRel, sizeof(CampRel));		
			}
			else if(strcmp( (char *)node->name, "victory" ) == 0)
			{
				if(!xml.getNodePropNum( node, "victoryType", (DWORD*)&VictoryType, sizeof(DWORD)))
					Zebra::logger->error("%s 加载战场配置 %s victoryType失败!", this->name, DulesFileName);

				xml.getNodePropNum( node, "VictoryData", &VictoryData, sizeof(VictoryData));

				xmlNodePtr victorynode = xml.getChildNode(node, NULL);
				while(victorynode)
				{
					if( strcmp( (char *)victorynode->name, "victoryreward" ) == 0 )
					{
						stBattleReward stReward;

						if(!xml.getNodePropNum( victorynode, "type", &(stReward.RewardType), sizeof(stReward.RewardType)))
							Zebra::logger->error("%s 加载战场配置 %s Victory type失败!", this->name, DulesFileName);

						if(!xml.getNodePropNum( victorynode, "num1", &(stReward.RewardData1), sizeof(stReward.RewardData1)))
							Zebra::logger->error("%s 加载战场配置 %s Victory num1失败!", this->name, DulesFileName);

						if(!xml.getNodePropNum( victorynode, "num2", &(stReward.RewardData2), sizeof(stReward.RewardData2)))
							Zebra::logger->error("%s 加载战场配置 %s Victory num2失败!", this->name, DulesFileName);

						Victory.push_back(stReward);
					}
					else if(strcmp( (char *)victorynode->name, "failurereward" ) == 0)
					{
						stBattleReward stReward;
						if(!xml.getNodePropNum( victorynode, "type", &(stReward.RewardType), sizeof(stReward.RewardType)))
							Zebra::logger->error("%s 加载战场配置 %s failure type失败!", this->name, DulesFileName);

						if(!xml.getNodePropNum( victorynode, "num1", &(stReward.RewardData1), sizeof(stReward.RewardData1)))
							Zebra::logger->error("%s 加载战场配置 %s failure num1失败!", this->name, DulesFileName);

						if(!xml.getNodePropNum( victorynode, "num2", &(stReward.RewardData2), sizeof(stReward.RewardData2)))
							Zebra::logger->error("%s 加载战场配置 %s failure num2失败!", this->name, DulesFileName);

						failure.push_back(stReward);
					}
					else if( strcmp( (char *)victorynode->name, "penalty" ) == 0 )
					{
						stBattleReward stpenalty;
						if(!xml.getNodePropNum( victorynode, "type", &(stpenalty.RewardType), sizeof(stpenalty.RewardType)))
							Zebra::logger->error("%s 加载战场配置 %s penalty type失败!", this->name, DulesFileName);

						if(!xml.getNodePropNum( victorynode, "num1", &(stpenalty.RewardData1), sizeof(stpenalty.RewardData1)))
							Zebra::logger->error("%s 加载战场配置 %s penalty num1失败!", this->name, DulesFileName);

						if(!xml.getNodePropNum( victorynode, "num2", &(stpenalty.RewardData2), sizeof(stpenalty.RewardData2)))
							Zebra::logger->error("%s 加载战场配置 %s penalty num2失败!", this->name, DulesFileName);

						penalty.push_back(stpenalty);
					}

					victorynode = xml.getNextNode(victorynode,NULL);
				}
			}
			else if(strcmp( (char *)node->name, "Battlefiel" ) == 0)
			{
				xmlNodePtr Battnode = xml.getChildNode(node, NULL);
				while (Battnode)
				{
					if( strcmp( (char *)Battnode->name, "CampPos" ) == 0 )
					{
						zPos pos;
						pos.x = 0;
						pos.y = 0;
						xml.getNodePropNum( Battnode, "x", &(pos.x), sizeof(pos.x) );
						xml.getNodePropNum( Battnode, "y", &(pos.y), sizeof(pos.y) );
						CampPos.push_back(pos);
					}
					else if( strcmp( (char *)Battnode->name, "relive" ) == 0 )
					{
						xml.getNodePropNum( Battnode, "reliveType", &reliveType, sizeof(reliveType) );			
						xml.getNodePropNum( Battnode, "reliveTime", &reliveTime, sizeof(reliveTime) );
					}
					else if( strcmp( (char *)Battnode->name, "killreward" ) == 0 )
					{
						xml.getNodePropNum( Battnode, "point", &killPoint, sizeof(killPoint) );
					}

					Battnode = xml.getNextNode(Battnode,NULL);
				}	
			}
			else if(strcmp( (char *)node->name, "select" ) == 0)
			{
				xml.getNodePropNum( node, "GameTime", &GameTime, sizeof(GameTime) );
				xml.getNodePropNum( node, "defaultvictory", &defaultvictory, sizeof(defaultvictory) );
			}
			node = xml.getNextNode(node,NULL);
		}
	}

	//sky 根据配置文件初始化阵营容器
	for(int i=0; i<CampNum; i++)
	{
		DWORD ThisId = this->id+i;
		BCampData * pCamp = new BCampData;
		pCamp->CampID = ThisId;

		if(i < CampPos.size())
		{
			pCamp->CampPos.x = CampPos[i].x;
			pCamp->CampPos.y = CampPos[i].y;
		}

		CampThisID[i+1] = ThisId;
		camp[ThisId] = pCamp;
	}

	return;
}

//sky 根据坐标分配阵营成员
bool GangScene::AddUserToScene(DWORD UserID, const zPos & initPos)
{
	if(camp.empty())
		return false;

	if(GameStart == 0)	//sky 第一次进人
		GameStart = 2;	//sky 开始进入倒计时阶段

	//sky 战场已经开始在上到这个地图就查询下是不是属于战场的人
	if(GameStart == 1)	
	{
		bool InCamp = false;
		std::map<DWORD ,BCampData* >::iterator iter;
		for(iter=camp.begin(); iter!=camp.end();iter++)
		{
			std::vector<stCampMember>::iterator it;
			for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
			{
				//sky 的确是在阵营中的成员
				if(it->userid == UserID)
				{
					//sky 重新把用户加到阵营中
					SceneUser * user = SceneUserManager::getMe().getUserByTempID(UserID);
					if(user)
					{
						user->BattCampID = iter->second->CampID;
						Channel::sendSys(user, Cmd::INFO_TYPE_GAME, "您已加入一个战场阵营 id:%d", id);
					}
					
					InCamp = true;
				}
			}
		}

		if(!InCamp)
		{
			//sky 上线的用户是不存在任何阵营中的,把他传到默认的地图中
		}
	}
	else
	{
		for(int i=0; i<CampNum; i++)
		{
			DWORD id = this->id + i;

			if( (initPos.x < camp[id]->CampPos.x+6 && initPos.x > camp[id]->CampPos.x-6) //sky 上线的位置只要在该传送点以中心的6层范围类
				&& (initPos.y < camp[id]->CampPos.y+6 && initPos.y > camp[id]->CampPos.y-6))
			{
				
				if(camp[id]->CampMembe.size() < CampUserNum)
				{
					stCampMember member;
					member.userid = UserID;
					camp[id]->CampMembe.push_back(member);
					SceneUser * user =  SceneUserManager::getMe().getUserByTempID(UserID);
					if(user)
					{
						user->BattCampID = camp[id]->CampID;
						Channel::sendSys(user, Cmd::INFO_TYPE_GAME, "您已加入一个战场阵营 id:%d", id);
					}
					break;
				}
				else
				{

				}
			}
		}
	}
}

//sky 获取特定阵营的杀死敌对玩家数
DWORD GangScene::GetCampKillUserNum(DWORD index)
{
	std::map<DWORD ,BCampData*>::iterator iter = camp.find(index);
	if(iter != camp.end())
	{
		return iter->second->KillUserNum;
	}
}

//sky 获取特定阵营杀死敌对NPC的数量
DWORD GangScene::GetCampKillNpcNum(DWORD index)
{
	std::map<DWORD ,BCampData*>::iterator iter = camp.find(index);
	if(iter != camp.end())
	{
		return iter->second->KillNpcNum;
	}
}

//sky 获取特定阵营获取资源的数量
DWORD GangScene::GetCampRegNum(DWORD index)
{
	std::map<DWORD ,BCampData*>::iterator iter = camp.find(index);
	if(iter != camp.end())
	{
		return iter->second->GetReg;
	}
}

//sky 获取战场已经经过的时候
DWORD GangScene::GetPassTime()
{
	return passTime;
}

//sky 增加阵营的资源
bool GangScene::AddCampReg(DWORD campID, int RegNum)
{
	std::map<DWORD ,BCampData*>::iterator iter = camp.find(campID);
	if(iter != camp.end())
	{
		//sky 计算一下防止溢出
		if(MAX_CAMPREG - iter->second->GetReg > RegNum)
		{
			iter->second->GetReg += RegNum;
		}
	}

	//sky 每次增加资源点数的时候都检测下胜利条件
	if(VictoryType == BATTLEFIELF_WINNT_REGNAMMAX)
		WinnerCamp = IfRegNumMax(campID);	//sky 如果是资源模式就检测下是否已经胜利

	return true;
}

//sky 减少阵营的资源
bool GangScene::DesCampReg(DWORD campID, int RegNum)
{
	std::map<DWORD ,BCampData*>::iterator iter = camp.find(campID);
	if(iter != camp.end())
	{
		//sky 计算一下防止溢出
		if(iter->second->GetReg < RegNum)
			iter->second->GetReg = 0;
		else
			iter->second->GetReg -= RegNum;
	}

	return true;
}

//sky 需要定时检测的胜负判断
bool GangScene::GetCampVictory()
{
	switch(VictoryType)
	{
	case BATTLEFIELF_WINNT_WATCH_NPCATT:
		break;
	case BATTLEFIELF_WINNT_TIME_GETGO:
		break;
	case BATTLEFIELF_WINNT_TIME_KILLUSERMUCH:
		{
			//sky 首先确定是否已经战场的持续时间是否已经结束
			if(IfTimeGetGo())
				WinnerCamp = IfTimeKillUserMuch();
		}
		break;
	case BATTLEFIELF_WINNT_TIME_KILLNPCMUCH:
		{
			if(IfTimeGetGo())
				WinnerCamp = IfTimeKillNpcMuch();
		}
		break;
	default:
		break;
	}

	return true;
}

//sky 发放胜利阵营奖励
bool GangScene::largessWinVictory()
{
	if(WinnerCamp != 0 && bVictory)
	{
		std::map<DWORD ,BCampData* >::iterator iter;
		iter = camp.find(WinnerCamp);
		if(iter != camp.end())
		{
			std::vector<stBattleReward>::iterator Rewardit;
			for(Rewardit=Victory.begin(); Rewardit!=Victory.end(); Rewardit++)
			{
				switch(Rewardit->RewardType)
				{
				case REWARD_MONEY:
					{
						//sky 遍历成员给予奖励金钱
						std::vector<stCampMember>::iterator it;
						for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
						{
							SceneUser * user = SceneUserManager::getMe().getUserByTempID(it->userid);
							if(user)
								user->packs.addMoney(Rewardit->RewardData1, "战场胜利");
						}
					}
					break;
				case REWARD_ITEM:
					{
						//sky 遍历成员给予奖励物品
						std::vector<stCampMember>::iterator it;
						for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
						{
							SceneUser * user = SceneUserManager::getMe().getUserByTempID(it->userid);
							if(user)
							{
								zObjectB *base = objectbm.get(Rewardit->RewardData1);

								if(base)
								{
									zObject* o = NULL;
									o = zObject::create(base);
									if(o)
									{
										user->packs.addObject( o, true, AUTO_PACK );
										//sky 通知玩家客户端跟新包袱里的物品信息
										Cmd::stAddObjectPropertyUserCmd send;
										bcopy(&o->data,&send.object,sizeof(t_Object),sizeof(send.object));
										user->sendCmdToMe(&send,sizeof(send));
									}
								}
							}
						}
					}
					break;
				case REWARD_HONOR:
					break;
				default:
					break;
				}
			}	
		}
	}

	bVictory = false;
	return true;
}

//sky 发放所有失败阵营奖励
bool GangScene::largessFailureVictory()
{
	if(!bfailure)
		return true;

	std::map<DWORD ,BCampData* >::iterator iter;
	for(iter=camp.begin(); iter!=camp.end(); iter++)
	{
		if(iter->second->CampID != WinnerCamp && WinnerCamp != 0)
		{
			std::vector<stBattleReward>::iterator Rewardit;
			for(Rewardit=failure.begin(); Rewardit!=failure.end(); Rewardit++)
			{
				switch(Rewardit->RewardType)
				{
				case REWARD_MONEY:
					{
						//sky 遍历成员给予奖励金钱
						std::vector<stCampMember>::iterator it;
						for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
						{
							SceneUser * user = SceneUserManager::getMe().getUserByTempID(it->userid);
							if(user)
								user->packs.addMoney(Rewardit->RewardData1, "战场胜利");
						}
					}
					break;
				case REWARD_ITEM:
					{
						//sky 遍历成员给予奖励物品
						std::vector<stCampMember>::iterator it;
						for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
						{
							SceneUser * user = SceneUserManager::getMe().getUserByTempID(it->userid);
							if(user)
							{
								zObjectB *base = objectbm.get(Rewardit->RewardData1);

								if(base)
								{
									zObject* o = NULL;
									o = zObject::create(base);
									if(o)
									{
										user->packs.addObject( o, true, AUTO_PACK );
										//sky 通知玩家客户端跟新包袱里的物品信息
										Cmd::stAddObjectPropertyUserCmd send;
										bcopy(&o->data,&send.object,sizeof(t_Object),sizeof(send.object));
										user->sendCmdToMe(&send,sizeof(send));
									}
								}
							}
						}
					}
					break;
				case REWARD_HONOR:
					break;
				default:
					break;
				}
			}
		}
	}

	bfailure = false;
	return true;	
}

//sky 失败阵营惩罚
bool GangScene::givepenalty()
{
	if(!bpenalty)
		return true;

	std::map<DWORD ,BCampData* >::iterator iter;
	for(iter=camp.begin(); iter!=camp.end(); iter++)
	{
		if(iter->second->CampID != WinnerCamp && WinnerCamp != 0)
		{
			std::vector<stBattleReward>::iterator penaltyit;
			for(penaltyit=failure.begin(); penaltyit!=failure.end(); penaltyit++)
			{
				//sky 惩罚不做物品处理
				switch(penaltyit->RewardType)
				{
				case REWARD_MONEY:
					{
						//sky 遍历成员给予奖励处罚
						std::vector<stCampMember>::iterator it;
						for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
						{
							SceneUser * user = SceneUserManager::getMe().getUserByTempID(it->userid);
							if(user)
							{
								user->packs.removeMoney(penaltyit->RewardData1, "战场失败");
							}
						}
					}
					break;
				case REWARD_HONOR:
					break;
				default:
					break;
				}
			}
		}
	}

	bpenalty = false;
	return true;
}

//sky 增加成员战场点
bool GangScene::AddUserCamePoint(DWORD userid, DWORD point)
{
	SceneUser * user = SceneUserManager::getMe().getUserByTempID(userid);
	if(user)
	{
		std::map<DWORD ,BCampData*>::iterator iter = camp.find(user->BattCampID);
		if(iter != camp.end())
		{
			std::vector<stCampMember>::iterator it;
			for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
			{
				if(it->userid == userid)
				{
					//sky 计算一下防止溢出
					if(MAX_GAMEPOINT-it->GamePoint > point)
					{
						it->GamePoint += point;
						Channel::sendSys(user, Cmd::INFO_TYPE_GAME, "您增加 %d 战场点,总共战场点:%d", point, it->GamePoint);
					}

					return true;
				}
			}
		}
	}

	return false;
}

//sky 减少成员战场点
bool GangScene::DesUserCampPoint(DWORD userid,	DWORD point)
{
	SceneUser * user = SceneUserManager::getMe().getUserByTempID(userid);
	if(user)
	{
		std::map<DWORD ,BCampData*>::iterator iter = camp.find(user->BattCampID);
		if(iter != camp.end())
		{
			std::vector<stCampMember>::iterator it;
			for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
			{
				if(it->userid == userid)
				{
					//sky 计算一下防止溢出
					if(it->GamePoint < point)
						it->GamePoint = 0;
					else
						it->GamePoint -= point;

					Channel::sendSys(user, Cmd::INFO_TYPE_GAME, "您减少 %d 战场点,总共战场点:%d", point, it->GamePoint);
					return true;
				}
			}
		}
	}

	return false;
}

//sky 批量检处理用户复活
bool GangScene::reliveRun()
{
	if(reliveType != Compulsory_Resurrection)
		return true;

	std::map<DWORD ,BCampData* >::iterator iter;
	for(iter=camp.begin(); iter!=camp.end(); iter++)
	{
		std::vector<stCampMember>::iterator it;
		for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
		{
			if( it->DeathTime > 0 )
				it->DeathTime--;	//sky 递减死亡时间
			else if( it->DeathTime == 0)
			{
				SceneUser * pUser = SceneUserManager::getMe().getUserByTempID(it->userid);
				if(pUser)
				{
					//sky 先传送回复活点
					pUser->goTo(iter->second->CampPos);

					//sky 移除灵魂状态
					pUser->Soulflag = false;
					pUser->sendMeToNine();

					it->DeathTime = -1;
				}
			}
		}
	}

	return true;
}

//sky 特殊处理一个用户的死亡事件
bool GangScene::UserDeathRun(DWORD DeathID, DWORD MurderID)
{
	SceneUser * Duser = SceneUserManager::getMe().getUserByTempID(DeathID);
	if(!Duser)
		return true;

	SceneEntryPk * Muser; 
	Muser = (SceneEntryPk *)SceneUserManager::getMe().getUserByTempID(MurderID);
	if(!Muser)
		Muser = (SceneEntryPk *)SceneNpcManager::getMe().getNpcByTempID(MurderID);

	//sky  先复活死亡的玩家(战场的死亡状态是以灵魂形式存在的)
	Duser->relive(Cmd::ReliveBattle,0,0);

	//sky 给予用户灵魂状态
	Cmd::stAttackMagicUserCmd cmd;
	zSkill * s = zSkill::create(Duser,88,1);
	cmd.byAttackType = Cmd::ATTACKTYPE_U2U;
	cmd.dwDefenceTempID = Duser->tempid;
	cmd.dwUserTempID = Duser->tempid;
	cmd.wdMagicType = 88;
	cmd.byAction = Cmd::Ani_Die;
	cmd.xDes = Duser->getPos().x;
	cmd.yDes = Duser->getPos().y;
	cmd.byDirect = Duser->getDir();

	if (s)
	{
		zSkill * useSkill = NULL;
		useSkill = zSkill::createTempSkill(Duser,s->data.skillid,1);
		if(useSkill)
		{
			useSkill->action(&cmd,sizeof(cmd));
			SAFE_DELETE(useSkill);
		}
	}

	//sky 当复活类型是强制复活 设置30秒复活时间
	if(reliveType != Compulsory_Resurrection)
	{
		std::map<DWORD ,BCampData*>::iterator iter = camp.find(Duser->BattCampID);
		if(iter != camp.end())
		{
			std::vector<stCampMember>::iterator it;
			for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
			{
				if(it->userid == DeathID)
					it->DeathTime = 30;
			}

			//sky 阵营灵魂人数自加
			iter->second->SoulUserNum++;
		}
	}

	if(Muser)
	{
		//sky 把阵营的杀人数字增加
		std::map<DWORD ,BCampData*>::iterator iter = camp.find(Muser->BattCampID);
		if(iter != camp.end())
		{
			iter->second->KillUserNum++;
		}
		//sky 先检测下关于成员死亡的胜利条件是否满足
		switch(VictoryType)
		{
		case BATTLEFIELF_WINNT_KILLUSERNUM:
			{
				WinnerCamp = IfKillUserNum(Muser->BattCampID);
				if(WinnerCamp != 0)
					return true;
			}
			break;
		case BATTLEFIELF_WINNT_KILLALLUSER:
			{
				WinnerCamp = IfKillAllUser();
				if(WinnerCamp != 0)
					return true;
			}
			break;
		default:
			break;
		}

		//sky 以上胜利条件都不成立的时候
		//sky 增加杀死对方阵营的玩家的战场点
		if(Muser->getType()==zSceneEntry::SceneEntry_Player)
			AddUserCamePoint(MurderID, killPoint);

		//sky 减少被杀玩家的战场点
		DesUserCampPoint(DeathID, killPoint);
	}
	
	return false;
}

//**** sky 胜负处理函数 ****
//sky 杀够对方阵营成员特定数量胜利检测(有人死亡的时候就检测下)
DWORD GangScene::IfKillUserNum(DWORD CampID)
{
	if(GetCampKillUserNum(CampID) >= VictoryData)
		return CampID;

	return 0;
}

//sky 杀死全部其他阵营成员胜利检测(警告:可以复活的战场该胜利条件将无法成立)
DWORD GangScene::IfKillAllUser()
{
	return 0;
}

//sky 摧毁对方阵营的基地检测
//sky 被摧毁的NPC的ID
bool GangScene::IfWreckBase(DWORD npcid)
{
	SceneNpc * BaseNpc = SceneNpcManager::getMe().getNpcByTempID(npcid);
	if(BaseNpc && BaseNpc->npc->kind == NPC_TYPE_CAMP)
	{
		std::map< DWORD ,BCampData* >::iterator iter;
		iter = camp.find(BaseNpc->BattCampID);
		//sky 首先确定这个阵营是真实存在的
		if(iter != camp.end() && camp.size() == 2)
		{
			std::map< DWORD ,BCampData* >::iterator it;
			for(it=camp.begin(); it!=camp.end(); it++)
			{
				if(it->second->CampID != BaseNpc->BattCampID)
				{
					WinnerCamp = it->second->CampID;
				}
			}
		}
	}

	return false;
}

//sky 坚持完特定NPC攻击,基地没被摧毁就算胜利检测
DWORD GangScene::IfWatchNpcAtt()
{
	return 0;
}

//sky 资源最先达标的阵营胜利检测
DWORD GangScene::IfRegNumMax(DWORD CampID)
{
	if(GetCampRegNum(CampID) >= VictoryData)
		return CampID;

	return 0;	
}

//sky 以持续时间为前提的胜利条件
//sky 坚持到时间结束检测
bool GangScene::IfTimeGetGo()
{
	if( passTime == GameTime )
		return true;

	return false;
}

//sky 时间结束时候杀死用户最多的阵营胜利
DWORD GangScene::IfTimeKillUserMuch()
{
	//sky 当前杀人数最大的阵营ID
	DWORD id = 0;
	//sky 杀人数
	DWORD num = 0;

	//sky 遍历阵营取出杀人最多的阵营
	std::map<DWORD ,BCampData*>::iterator iter;
	for(iter=camp.begin(); iter!=camp.end(); iter++)
	{
		if(iter->second->KillUserNum > num)
		{
			num = iter->second->KillUserNum;
			id = iter->second->CampID;
		}
	}

	return id;
}

//sky 时间结束时候杀死NPC最多的阵营胜利
DWORD GangScene::IfTimeKillNpcMuch()
{
	//sky 当前杀NPC数最大的阵营ID
	DWORD id = 0;
	//sky 杀NPC数
	DWORD num = 0;

	//sky 遍历阵营取出杀NPC最多的阵营
	std::map<DWORD ,BCampData*>::iterator iter;
	for(iter=camp.begin(); iter!=camp.end(); iter++)
	{
		if(iter->second->KillNpcNum > num)
		{
			num = iter->second->KillNpcNum;
			id = iter->second->CampID;
		}
	}

	return id;	
}

//sky 战场场景定时处理函数
void GangScene::GangSceneTime(const zRTime& ctv)
{
	//sky 倒计时状态
	if(GameStart == 2)
	{
		StartTime -= 1;
		//sky 时间结束
		if(StartTime == 0)
			GameStart = 1; //sky 战场正式开始
	}

	//sky 只有战场开始状态下才执行以下的处理
	if(GameStart == 1)
	{
		//sky 处理用户的复活状况
		reliveRun();

		if(_one_min(ctv) && GameStart == 1)	//sky 每过一分钟把时间流逝+1
			passTime++;

		//sky 检测胜负状况
		GetCampVictory();

		//sky 处理胜负
		CampVictoryRun();
	}

	if(GameStart == 3)
	{
		//sky 开始战场结束倒计时
		OverTime -= 1;
		if(OverTime == 0)
		{
			//sky 结束战场处理
			OverBattGame();
		}
	}
}


//sky 战场胜负处理函数
bool GangScene::CampVictoryRun()
{
	//sky 必须已经存在胜利阵营
	if(WinnerCamp != 0)
	{
		//sky 发送胜利阵营奖励
		largessWinVictory();
		//sky 发送失败阵营奖励
		largessFailureVictory();
		//sky 失败阵营惩罚
		givepenalty();
	}

	//sky 已经处理完胜负把战场状况致为结束状态
	GameStart = 3;

	return true;
}

DWORD GangScene::ReCampThisID(BYTE index)
{
	if(index <= CampNum)
	{
		std::map<BYTE, DWORD>::iterator iter;
		iter = CampThisID.find(index);

		if(iter != CampThisID.end())
			return iter->second;
	}

	return 0;
}

//sky 结束战场处理
bool GangScene::OverBattGame()
{
	std::map<DWORD ,BCampData* >::iterator iter;
	for(iter=camp.begin(); iter!=camp.end(); iter++)
	{
		std::vector<stCampMember>::iterator it;
		for(it=iter->second->CampMembe.begin(); it!=iter->second->CampMembe.end(); it++)
		{
			SceneUser * pUser = SceneUserManager::getMe().getUserByTempID(it->userid);
			if(pUser)
			{
				//sky 如果用户是死亡状态先设置为复活状态
				if(pUser->Soulflag)
				{
					//sky 移除灵魂状态
					pUser->Soulflag = false;
					pUser->sendMeToNine();
				}

				//sky 把用户传到排队前的地图位置
				char mapname[MAX_NAMESIZE+1];
				DWORD oldx;
				DWORD oldy;
				sscanf(pUser->charbase.OldMap, "%[^-]-%d-%d", mapname, &oldx, &oldy);
				pUser->charbase.x = oldx;
				pUser->charbase.y = oldy;
				Cmd::Session::t_changeScene_SceneSession cmd;
				cmd.id = pUser->id;
				cmd.temp_id = pUser->tempid;
				cmd.x = oldx;
				cmd.y = oldy;
				cmd.map_id = 0;
				cmd.map_file[0] = '\0';
				strncpy((char *)cmd.map_name, mapname, MAX_NAMESIZE);
				sessionClient->sendCmd(&cmd,sizeof(cmd));
			}
		}
	}
	return true;
}


//***********************************************************************
//*********** sky SceneArchitecture类实现 *******************************
//***********************************************************************

SceneArchitecture::SceneArchitecture(Scene* scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype,zNpcB *abase)
:SceneNpc(scene,npc,define,type,entrytype,abase)
,_one_min(60)
{
	SummonNpc.clear();	//sky 召唤的NPC信息(ID, 数量)
	SummonTime = 45;					//sky 召唤间隔(毫秒)
	LevelUpTime = 2;					//sky 召唤的兵升级时间(分钟)
	SummonLevel = 0;							//sky 召唤的士兵当前等级
	bOutcome = false;					//sky 死亡是否影响胜负
	SummonAI = define->scriptID;		//sky 刷出来的士兵使用的移动AI_ID

	//sky 刷兵倒计时
	SummonCountdown = 0;
	//sky 升级倒计时
	LevelUpCountdown = 0;
}

//[sky] 召唤士兵(每个士兵都是一个独立的个体不同以召唤兽或者宠物)
int SceneArchitecture::summonSoldiers(DWORD id, Cmd::petType type, WORD num, DWORD sid, const char * name, DWORD anpcid,zPos pos,BYTE dir)
{
	zNpcB *base = npcbm.get(id);
	zNpcB *abase = NULL;
	if (anpcid>0) abase = npcbm.get(anpcid);
	if (NULL == base) return false;

	t_NpcDefine define;
	define.id = base->id;
	strncpy(define.name,base->name,MAX_NAMESIZE-1);
	if (pos.x != 0 && pos.y !=0)
		define.pos = pos;
	else
		define.pos = getPos();
	define.num = 1;
	define.interval = 5;
	define.initstate = zSceneEntry::SceneEntry_Normal;
	define.width = 6;
	define.height = 6;
	define.pos -= zPos(define.width/2,define.height/2);
	define.scriptID = sid;
	scene->initRegion(define.region,define.pos,define.width,define.height);

	int NewNum = 0;
	SceneNpc * newPet = NULL;

	for( int i=0; i< num; i++ )
	{
		newPet = scene->summonOneNpc<SceneNpc>(define,pos,base,dupIndex,0,abase,dir,this);

		if (newPet)
		{
			//sky 设置召唤出来的士兵的阵营和召唤者一样
			newPet->BattCampID = this->BattCampID;

			if (name&&(0!=strncmp(name,"",MAX_NAMESIZE)))
				strncpy(newPet->name,name,MAX_NAMESIZE-1);

			SceneNpcManager::getMe().addSpecialNpc(newPet);

			newPet->aif = newPet->aif | AIF_ACTIVE_MODE;
			newPet->setDir(dir);
			newPet->setSpeedRate( 2.0 );
			NewNum++;
		}
	}

	return NewNum;
}


bool SceneArchitecture::laod_ai()
{
	stNewAIData_t m_AiXml;
	strncpy(m_AiXml.AiName, name,sizeof(m_AiXml.AiName));
	m_AiXml.AiId = npc->id;

	aif = 0; //sky 初始化行动模式
	npc_search_region = 5; //sky 初始化索敌范围

	char Ai_FileName[MAX_PATH];
	sprintf( Ai_FileName, "newAI/%s.xml", name );

	FILE * fp = fopen( Ai_FileName, "r");
	if( fp == NULL )
		return false;

	fclose( fp );

	zXMLParser xml;
	if (!xml.initFile(Ai_FileName))
	{
		Zebra::logger->error("加载AI文件 %s 失败",Ai_FileName);
		return false;
	}

	int ai_id = 0;
	NpcAiCs NpcAibuffe;
	xmlNodePtr root;

	root = xml.getRootNode("event");

	if (root)
	{
		xmlNodePtr node = xml.getChildNode(root,"actionai");

		if (0 == strcmp((char *)node->name,"actionai")) 
		{
			if(!xml.getNodePropNum(node,"type",&aif,sizeof(aif)))
				Zebra::logger->error("读取文件 %s 怪物行为模式失败 默认行为模式为 %u",Ai_FileName, aif);

			m_AiXml.aif = aif;
		}

		node = xml.getChildNode(root, "attRange");
		if( 0 == strcmp((char *)node->name, "attRange") )
		{
			if(!xml.getNodePropNum(node, "range", &npc_search_region, sizeof(npc_search_region)))
				Zebra::logger->error("读取文件 %s 怪物索敌范围失败 默认索敌范围为 %u",Ai_FileName, npc_search_region);

			m_AiXml.npc_search_region = npc_search_region;
		}

		node = xml.getChildNode(root,"quest");  
		while (node) 
		{
			//parse quest
			if (0 == strcmp((char *)node->name,"quest")) 
			{
				if (!xml.getNodePropNum(node,"aiid",&ai_id,sizeof(ai_id))) 
				{
					return false;
				}

				xmlNodePtr phaseNode = xml.getChildNode(node,"phase");
				while (phaseNode)
				{

					if(!xml.getNodePropNum( phaseNode, "id", &NpcAibuffe.id, sizeof(NpcAibuffe.id) ) )
					{
						return false;
					}

					if( !xml.getNodePropNum( phaseNode, "level", &NpcAibuffe.level, sizeof(NpcAibuffe.level) ) )
					{
						return false;
					}

					if( !xml.getNodePropNum( phaseNode, "target", &NpcAibuffe.target, sizeof(NpcAibuffe.target) ) )
					{
						return false;
					}

					if( !xml.getNodePropNum( phaseNode, "Rate", &NpcAibuffe.Rate, sizeof(NpcAibuffe.Rate) ) )
					{
						return false;
					}

					if( !xml.getNodePropNum( phaseNode, "cond1", &NpcAibuffe.cond1, sizeof(NpcAibuffe.cond1) ) )
					{
						return false;
					}

					if( !xml.getNodePropNum( phaseNode, "cond2", &NpcAibuffe.cond2, sizeof(NpcAibuffe.cond2) ) )
					{
						return false;
					}

					//#ifdef _DEBUG
					/*Zebra::logger->error*/
					//#endif
					if( ai_id == 1)
					{
						NpcAiTimeList.push_back(NpcAibuffe);
						m_AiXml.NpcAiTimeList.push_back( NpcAibuffe );
					}
					else if( ai_id == 2)
					{
						NpcAiList.push_back( NpcAibuffe );
						m_AiXml.NpcAiList.push_back( NpcAibuffe );
					}

					phaseNode = xml.getNextNode(phaseNode,"phase");
				}

				node = xml.getNextNode(node,"quest");
			}
		}

		//sky 新增读取刷兵配置AI
		node = xml.getChildNode(root, "summonMatter");
		while(node)
		{
			xmlNodePtr SummonNode = xml.getChildNode(node, NULL);
			while(SummonNode)
			{
				if(strcmp((char *)SummonNode->name,"SurviveTime") == 0)
				{
					if(!xml.getNodePropNum(SummonNode, "tiem", &dwStandTime, sizeof(DWORD)))
					{
						Zebra::logger->error("%s 加载NPC刷兵配置 %s SurviveTime失败!", this->name, Ai_FileName);
					}
					else
						m_AiXml.Matter.SurviveTime = dwStandTime;
				}
				else if(strcmp((char *)SummonNode->name,"summonTime") == 0)
				{
					if(!xml.getNodePropNum(SummonNode, "tiem", &SummonTime, sizeof(DWORD)))
					{
						Zebra::logger->error("%s 加载NPC刷兵配置 %s CampUserNum失败!", this->name, Ai_FileName);
					}
					else
					{
						m_AiXml.Matter.SummonTime = SummonTime;
						SummonCountdown = SummonTime;
					}
				}
				else if(strcmp((char *)SummonNode->name,"summonLevelUp") == 0)
				{
					if(!xml.getNodePropNum(SummonNode, "time", &LevelUpTime, sizeof(WORD)))
					{
						Zebra::logger->error("%s 加载NPC刷兵配置 %s LevelUpTime失败!", this->name, Ai_FileName);
					}
					else
					{
						m_AiXml.Matter.levelupTime = LevelUpTime;
						LevelUpCountdown = LevelUpTime;
					}
				}
				else if(strcmp((char *)SummonNode->name, "level") == 0)
				{
					xmlNodePtr NpcNode = xml.getChildNode(SummonNode, NULL);
					while(NpcNode)
					{
						if(strcmp((char *)NpcNode->name, "npc") == 0)
						{
							SummonNpcData data;

							if(!xml.getNodePropNum(NpcNode,"id1", data.id, sizeof(DWORD)))
								Zebra::logger->error("%s 加载NPC刷兵配置 %s npc id1失败!", this->name, Ai_FileName);
							if(!xml.getNodePropNum(NpcNode,"num1", &data.num, sizeof(DWORD)))
								Zebra::logger->error("%s 加载NPC刷兵配置 %s npc num1失败!", this->name, Ai_FileName);
							if(!xml.getNodePropNum(NpcNode,"id2", data.id+1, sizeof(DWORD)))
								Zebra::logger->error("%s 加载NPC刷兵配置 %s npc id2失败!", this->name, Ai_FileName);
							if(!xml.getNodePropNum(NpcNode,"num2", data.num+1, sizeof(DWORD)))
								Zebra::logger->error("%s 加载NPC刷兵配置 %s npc num2失败!", this->name, Ai_FileName);
							if(!xml.getNodePropNum(NpcNode,"id3", data.id+2, sizeof(DWORD)))
								Zebra::logger->error("%s 加载NPC刷兵配置 %s npc id3失败!", this->name, Ai_FileName);
							if(!xml.getNodePropNum(NpcNode,"num3", data.num+2, sizeof(DWORD)))
								Zebra::logger->error("%s 加载NPC刷兵配置 %s npc num3失败!", this->name, Ai_FileName);

							SummonNpc.push_back(data);
							m_AiXml.Matter.SummonNpc.push_back(data);
						}

						NpcNode = xml.getNextNode(NpcNode, NULL);
					}
				}

				SummonNode = xml.getNextNode(SummonNode, NULL);
			}

			node = xml.getNextNode(node,"summonMatter");
		}

		printf("加载AI文件 %s 成功\n",Ai_FileName);

		NpcAiXmlData.push_back(m_AiXml);
		return true;
	}

	return false;
}

//sky 获取AI
bool SceneArchitecture::GetNpcAi()
{
	NpcAiList.clear();
	NpcAiTimeList.clear();

	if(!NpcAiXmlData.empty())
	{
		std::list<stNewAIData_t>::iterator it;
		for(it=NpcAiXmlData.begin(); it!=NpcAiXmlData.end(); ++it)
		{
			if(npc->id == it->AiId)
			{
				aif = it->aif;
				npc_search_region = it->npc_search_region;

				std::vector<NpcAiCs>::iterator iter;
				for(iter=it->NpcAiList.begin(); iter!=it->NpcAiList.end(); ++iter)
				{
					NpcAiList.push_back((*iter));	
				}

				for(iter=it->NpcAiTimeList.begin(); iter!=it->NpcAiTimeList.end(); ++iter)
				{
					NpcAiTimeList.push_back((*iter));
				}

				SummonTime = it->Matter.SummonTime;				//sky 召唤间隔(毫秒)
				SummonCountdown = SummonTime;
				LevelUpTime = it->Matter.levelupTime;			//sky 召唤的兵升级时间(分钟)
				LevelUpCountdown = LevelUpTime;
				dwStandTime = it->Matter.SurviveTime;			//sky 生存时间(为0就是一直存活,一般用于战场大门,死亡就是开门^_^)

				std::vector<SummonNpcData>::iterator iter1;
				for(iter1=it->Matter.SummonNpc.begin(); iter1!=it->Matter.SummonNpc.end(); ++iter1)
				{
					SummonNpc.push_back(*(iter1));
				}
			}
		}
	}

	return laod_ai();
}


void SceneArchitecture::ArchitecTimer(const zRTime& ctv)
{
	if(this->scene->IsGangScene())
	{
		if(((GangScene *)scene)->GetGameStart() == 1)
		{
			//sky 刷兵倒记时间递减
			SummonCountdown++;
			if(SummonCountdown >= SummonTime)
			{
				SummonCountdown = 0;
				if(!SummonNpc.empty())
				{
					//sky 召唤
					for(int i=0; i<3; i++)
					{
						summonSoldiers(SummonNpc[SummonLevel].id[i], Cmd::PET_TYPE_NOTPET, SummonNpc[SummonLevel].num[i], SummonAI);
					}
				}
			}

			if(_one_min(ctv))
			{
				LevelUpCountdown++;
				if(LevelUpCountdown >= LevelUpTime)
				{
					LevelUpCountdown = 0;
					//sky 判断召唤等级是否是容器的极限
					if(SummonLevel < (SummonNpc.size()-1))
						SummonLevel++;	//sky 否 把召唤等级提升
				}
			}
		}
	}
}