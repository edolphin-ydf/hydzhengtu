/*
*\ brief:处理用户宝箱相关的消息
*\ auth: 黄蔚
*\ date: 2008-02-29
*/



//#include <zebra/ScenesServer.h>
#include "scriptTickTask.h"
#include "duplicateManager.h"
#include <zebra/csBox.h>
#include <zebra/csTurn.h>
#include "boxCircle.h"

//检查用户提交的物品是否是改造需要的
//bool check(zObject *obj,Cmd::stReMakObjectUserCmd *cmd,DWORD &money)
//{
	/*zObjectB *base = objectbm.get(obj->data.dwObjectID);
	if(base)
	{
		DWORD size = base->need_material.stuffs.size();
		money = base->need_material.gold;

		DWORD size1 = cmd->count;
		for( int i =0; i < size; ++i)
		{
			int j = 0;
			for(; j < size1; ++j)
			{

				if((base->need_material.stuffs)[i].id == (cmd->list)[j].gem_id)
					break;

			}

			if(j == size1)
				return false;

		}

		return true;
	}*/

	//return false;

//}

bool SceneUser::doHorseTrainingCmd(const Cmd::stReMakUserCmd *ptCmd,DWORD cmdLen)
{
	if (!horse.horse())
	{
		Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"你还没有马匹!");
		return true;
	}

	using namespace Cmd;
	switch(ptCmd->byParam)
	{
	case HORSETRAINING_USERCMD_TEY:
		{
			stHorseTrainingUserCmd * pCmd = (stHorseTrainingUserCmd*)ptCmd;

			switch(pCmd->CmdTey)
			{
			case 0:	//马匹训练1
				{
					if (!packs.checkMoney( 1000 ) || !packs.removeMoney(1000,"二级马匹训练"))
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"小样,没那么多钱就别来找我！");
						return true;
					}

					if( horse.data.horseXLlevel > 0 )
					{
						if( horse.data.horseXLlevel == 1 )
						{
							horse.data.speed	-= 10;
						}
						else if( horse.data.horseXLlevel == 2 )
						{
							horse.data.speed	-= 20;
						}
						else if( horse.data.horseXLlevel == 3 )
						{
							horse.data.speed	-= 30;
						}
						horse.data.horseXLlevel = 0;
					}

					horse.data.horseXLlevel = 1;
					horse.data.horseXLtime	+= 86400;
					horse.data.speed		+= 10;
					horse.mount(false);
					horse.sendData();
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"您获得一级战马训练能力");
				}
				break;
			case 1:	//马匹训练2
				{
					if (!packs.checkMoney( 2000 ) || !packs.removeMoney(2000,"二级马匹训练"))
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"小样,没那么多钱就别来找我！");
						return true;
					}

					if( horse.data.horseXLlevel > 0 )
					{
						if( horse.data.horseXLlevel == 1 )
						{
							horse.data.speed	-= 10;
						}
						else if( horse.data.horseXLlevel == 2 )
						{
							horse.data.speed	-= 20;
						}
						else if( horse.data.horseXLlevel == 3 )
						{
							horse.data.speed	-= 30;
						}
						horse.data.horseXLlevel = 0;
					}

					horse.data.horseXLlevel = 2;
					horse.data.horseXLtime	+= 86400;
					horse.data.speed		+= 20;
					horse.mount(false);
					horse.sendData();
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"您获得二级战马训练能力");
				}
				break;
			case 2:	//马匹训练3
				{
					if (!packs.checkMoney( 5000 ) || !packs.removeMoney(5000,"三级马匹训练"))
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"小样,没那么多钱就别来找我！");
						return true;
					}

					if( horse.data.horseXLlevel > 0 )
					{
						if( horse.data.horseXLlevel == 1 )
						{
							horse.data.speed	-= 10;
						}
						else if( horse.data.horseXLlevel == 2 )
						{
							horse.data.speed	-= 20;
						}
						else if( horse.data.horseXLlevel == 3 )
						{
							horse.data.speed	-= 30;
						}
						horse.data.horseXLlevel = 0;
					}

					horse.data.horseXLlevel = 3;
					horse.data.horseXLtime	+= 86400;
					horse.data.speed		+= 30;
					horse.mount(false);
					horse.sendData();
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"您获得三级战马训练能力");
				}
				break;
			case 3:	//清除训练
				{
					horse.data.horseXLtime	= 0;
					if( horse.data.horseXLlevel == 1 )
					{
						horse.data.speed	-= 10;
					}
					else if( horse.data.horseXLlevel == 2 )
					{
						horse.data.speed	-= 20;
					}
					else if( horse.data.horseXLlevel == 3 )
					{
						horse.data.speed	-= 30;
					}
					horse.data.horseXLlevel = 0;
					horse.mount(false);
					horse.sendData();
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"您的战马训练能力已经被清空");
				}
				break;
			case 4:
				{
					DWORD Ret	= atoi(Zebra::global["ExploitRate"].c_str());
					DWORD dwExp;
					if( Ret > 0 )
					{
						dwExp = ( charbase.exploit/100 )* Ret;

					}
					else
					{
						dwExp = ( charbase.exploit/100 )* 10;
					}
					charbase.exploit = 0;
					Cmd::stAddUserMapScreenUserCmd cmd;
					full_t_MapUserData(cmd.data);
					sendCmdToMe(&cmd,sizeof(cmd));
					addExp(dwExp,true,0,0,true);
				}
				break;
			default:
				{
					return true;
				}
				break;
			}
		}
	}
}

bool SceneUser::doReMakeObjCmd(const Cmd::stReMakUserCmd *ptCmd,DWORD cmdLen)
 {
	using namespace Cmd;
	switch(ptCmd->byParam)
	{
	case SET_NEW_MAKEOBJECT_USERCMD_ITEM:
		{
			stNewMakeObjectUserCmd * pCmd = (stNewMakeObjectUserCmd *)ptCmd;
			int num = pCmd->num;

			if(num < 1)
				return true;

			for(int i=0; i<=num;)
			{
				int MakeNum = RebuildObject::instance().make( *this, pCmd->dwID, (num-i) , pCmd->MakeLevel );

				if(MakeNum == 0)
					return true;

				i += MakeNum;
			}
		}
		break;
	default:
		break;
	}
	return true;
}


bool SceneUser::doBoxCmd(const Cmd::stCowBoxUserCmd *ptCmd,DWORD cmdLen)
{
    using namespace Cmd;
	switch(ptCmd->byParam)
	{
  //客户端发送钥匙被点击消息
	case SET_COWBOX_KEY_PARAMETER:
		{	
		    boxCircle::getInstance().createBoxCircle();


			if(!doPropertyCmd((stPropertyUserCmd *)ptCmd,cmdLen))
				return true;



			if( box_item.targetO != NULL)
			{
				zObject::destroy(box_item.targetO);
				box_item.targetO = NULL;
			}

			if( box_item.defaultO != NULL)
			{
				zObject::destroy(box_item.defaultO);
				box_item.defaultO = NULL;
			}

			box *_box = boxCircle::getInstance().generateOneBox(((stSetCowBoxKeyCmd *)ptCmd)->Key_id == 963,this->charbase.face == 1 ? 1 : 2,this->charbase.level);


			stGetCowBoxInitCmd initBoxCmd;
			initBoxCmd.item_Ti =  boxCircle::getInstance().getRandomIndex(16,time(NULL)) + 1;

			zObject* tempOs[17];
			bzero(tempOs,sizeof(zObject*)*17);

			bool CreateError = false;

			for(int i = 0; i < 17; ++i)
			{

				zObjectB *base = objectbm.get(_box->_boxObjects[i].obj_id);

				if(NULL == base)
				{
					CreateError = true;
					break;
				}

				zObject* o = NULL;
				o = zObject::create(base,1,_box->_boxObjects[i].itemlevel);

				if(NULL == o)
				{
					CreateError = true;
					break;
				}


				o->data.kind = _box->_boxObjects[i].kind;

				//if(_box->_boxObjects[i].itemkind == 3)
				//	_box->_boxObjects[i].itemkind = 4;

				if(base->kind >= 101 && base->kind <=118)
				{

					EquipMaker maker(NULL);

					maker.NewAssign(this,o,base,1);

				}


				bcopy(&o->data,&initBoxCmd.objects[i],sizeof(t_Object),sizeof(t_Object));

				if(i == 0)
				{
				    box_item.defaultO = o;
					//box_item->default_id = _box->_boxObjects[i].obj_id;
					//box_item->default_level = _box->_boxObjects[i].itemlevel;
					//box_item->default_kind = _box->_boxObjects[i].itemkind;
				}
				if(i == initBoxCmd.item_Ti)
				{
					box_item.targetO = o;
					//box_item->target_id = _box->_boxObjects[i].obj_id;
					//box_item->target_level = _box->_boxObjects[i].itemlevel;
					//box_item->target_kind = _box->_boxObjects[i].itemkind;
				}

				tempOs[i] = o;

			}



			if(CreateError)//创建对象出错
			{
				//
				for(int i = 0; i < 17; ++i)
				{
					if(tempOs[i] != NULL)
					zObject::destroy(tempOs[i]);
				}
				//delete box_item;
				//box_item = NULL;
				box_item.defaultO = NULL;
				box_item.targetO = NULL;
				return true;

			}


			this->sendCmdToMe(&initBoxCmd,sizeof(initBoxCmd));


			//销毁除默认物品和目标物品以外的所有物品
			for(int i = 0; i < 17; ++i)
			{
				if(tempOs[i] != box_item.targetO && tempOs[i] != box_item.defaultO)
				{
					if(tempOs[i] != NULL)
					zObject::destroy(tempOs[i]);
				}
			}


	//		execute_script_event(this,"test");

			

			/*if(this->dupIndex == 0)
			{
				userQuestEnterDup(SceneManager::getInstance().getMapId(charbase.country,1391));

			}
			else
			{
				userLeaveDup();
			}*/
			return true;
		}
		break;
  //用户点击宝盒控件的 提取 按键
	case SET_COWBOX_TIQU_ITEM:
		{


		    //if(NULL == box_item)
			//	return true;
		
		Cmd::stSetCowBoxTiquCmd *fetchCmd = (Cmd::stSetCowBoxTiquCmd *)ptCmd;

			zObject *targetO;

			//DWORD id;
			//DWORD level;
			//DWORD kind;

			if(false == fetchCmd->item_id)
			{
				if(box_item.defaultO != NULL)
					zObject::destroy(box_item.defaultO);
				targetO = box_item.targetO;
				box_item.targetO = NULL;
				box_item.defaultO = NULL;
			    //id = box_item->target_id;
				//level = box_item->target_level;
				//kind =  box_item->target_kind;
			}
			else
			{
				if(box_item.targetO != NULL)
					zObject::destroy(box_item.targetO);
				targetO = box_item.defaultO;
				box_item.defaultO = NULL;
				box_item.targetO = NULL;
			}
		

			if(targetO != NULL)
			{

				if(!packs.addObject(targetO,true,AUTO_PACK))
				{
					zObject::destroy(targetO);
					return true;
				}
			}
			else
			{
				return true;
			}

			Cmd::stAddObjectPropertyUserCmd send;
            bcopy(&targetO->data,&send.object,sizeof(t_Object),sizeof(t_Object));
            sendCmdToMe(&send,sizeof(send));

			return true;
		}
		break;
    default:
      break;
  }
  return true;
}


//[sky] 装备自由加点处理函数
bool SceneUser::doAddItemAttCmd(const Cmd::stAttruByteUserCmd *ptCmd,DWORD cmdLen)
{
	using namespace Cmd;

	switch(ptCmd->byParam)
	{
	case SURPLUS_ATTRIBUTE_USERCMD_ADD:
		{
			stAddAttruByteUserCmd * pCmd = (stAddAttruByteUserCmd *)ptCmd;

			zObject * obj = GetObjectBydst( &(pCmd->pos) );

			if( obj != NULL )
			{
				if(pCmd->Add_Num > obj->data.Freedom.Surplus_Attribute )
				{
					Zebra::logger->error("玩家 %s 想分配的自由点数大过装备所剩余的自由点数！分配失败！！\n", this->name );
					return true;
				}

				switch( pCmd->Add_Type )
				{
				case Cmd::PPT_STR:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //先把剩余点数减掉
						if(0xffff - obj->data.Freedom.str_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.str_Attribute += pCmd->Add_Num;	//再把属性点分配到额外的属性中
					}
					break;
				case Cmd::PPT_INTE:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //先把剩余点数减掉
						if(0xffff - obj->data.Freedom.inte_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.inte_Attribute += pCmd->Add_Num;	//再把属性点分配到额外的属性中
					}
					break;
				case Cmd::PPT_DEX:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //先把剩余点数减掉
						if(0xffff - obj->data.Freedom.dex_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.dex_Attribute += pCmd->Add_Num;		//再把属性点分配到额外的属性中
					}
					break;
				case Cmd::PPT_SPI:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //先把剩余点数减掉
						if(0xffff - obj->data.Freedom.spi_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.spi_Attribute += pCmd->Add_Num;		//再把属性点分配到额外的属性中
					}
					break;
				case Cmd::PPT_CON:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //先把剩余点数减掉
						if(0xffff - obj->data.Freedom.con_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.con_Attribute += pCmd->Add_Num;		//再把属性点分配到额外的属性中
					}
					break;
				default:
					break;
				}

				//刷新客户端的物品信息
				Cmd::stAddObjectPropertyUserCmd ret;
				ret.byActionType = Cmd::EQUIPACTION_REFRESH;
				bcopy(&obj->data,&ret.object,sizeof(t_Object),sizeof(t_Object));
				sendCmdToMe(&ret,sizeof(ret));
			}
		}
		break;
	default:
		break;
	}

	return true;
}

//sky 宝石镶嵌处理函数
bool SceneUser::doMosaicGenCmd(const Cmd::stMakeObjectUserCmd *ptCmd,DWORD cmdLen)
{
	using namespace Cmd;
	switch(ptCmd->byParam)
	{
	case SURPLUS_MOSAIGEM_USERCMD_ADD:		//sky 镶嵌宝石
		{
			stMosaicGemUserCmd * pCmd = (stMosaicGemUserCmd *)ptCmd;

			zObject * Eobj = GetObjectBydst( &(pCmd->Epos) );
			zObject * Gobj = GetObjectBydst( &(pCmd->Gpos) );

			if( Hole::put_hole(Eobj ,pCmd->index ,Gobj) )
			{
				//从包袱中删除宝石
				packs.removeObject(Gobj);

				//刷新客户端的物品信息
				Cmd::stAddObjectPropertyUserCmd ret;
				ret.byActionType = Cmd::EQUIPACTION_REFRESH;
				bcopy(&Eobj->data,&ret.object,sizeof(t_Object),sizeof(t_Object));
				sendCmdToMe(&ret,sizeof(ret));
			}
		}
		break;
	case RT_NPC_DIRITEM_USERCMD_PARA:		//sky 队长查询NPC尸体中的物品列表
		{
			stNpcDirItemUserCmd * pCmd = (stNpcDirItemUserCmd *)ptCmd;

			npc_dwNpcTempID = pCmd->dwNpcTempID;
			SceneNpc *sceneNpc= SceneNpcManager::getMe().getNpcByTempID( npc_dwNpcTempID );

			if( !sceneNpc )
			{
				Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"目标不存在.");
				return true;
			}

			if( !(sceneNpc->m_TemObj.empty()) )
			{
				TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(TeamThisID);

				if( this->tempid == sceneNpc->Captain_ID && teamMan->isCaptainObj() )
				{
					void * buffer = NULL;
					int nLen = sizeof(stNpcDirItemDataUserCmd) 
						+ (sizeof(stNpcDirItemDataUserCmd::team)*10)
						+ (sizeof(t_Object)*(sceneNpc->m_TemObj.size()));

					buffer = malloc( nLen );

					stNpcDirItemDataUserCmd * Cmd = (stNpcDirItemDataUserCmd *)buffer;
					Cmd->byCmd = MAKEOBJECT_USERCMD;
					Cmd->byParam = RT_NPC_DIRITEM_DATA_USERCMD_PARA;

					memset( &(Cmd->fen_team[0]), 0, sizeof(Cmd->fen_team) );
					int i = 0;

					std::vector<TeamMember>::iterator teamit;

					for( int f=0; f<teamMan->getSize(); f++ )
					{
						if( f < MAX_TEAM_NUM )
						{
							const TeamMember * ztTeam = getMember(f);
							Cmd->fen_team[f].tempid = ztTeam->tempid;
							memcpy( Cmd->fen_team[f].name, ztTeam->name, 33,sizeof(Cmd->fen_team[f].name) );
						}
					}


					Cmd->count = sceneNpc->m_TemObj.size();
					i = 0;
					std::list<zObject *>::iterator it;
					for( it = sceneNpc->m_TemObj.begin(); it != sceneNpc->m_TemObj.end(); it++ )
					{
						if( i < Cmd->count )
						{
							memcpy( &(Cmd->objects[i]), &((*it)->data), sizeof(t_Object),sizeof(Cmd->objects[i]) );
							i++;
						}
					}

					sendCmdToMe(Cmd,nLen);

					free( buffer );
				}
			}
		}
		break;
	case RT_NPC_GIVEITEM_USERCMD:		//sky 把分配过的物品添加到被分配人的包袱中
		{
			stNpcGiveItemUserCmd * pCmd = (stNpcGiveItemUserCmd *)ptCmd;

			SceneNpc *sceneNpc= SceneNpcManager::getMe().getNpcByTempID( npc_dwNpcTempID );
			if( !sceneNpc )
			{
				Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"该NPC已经不存在拉!");
				return true;
			}
			std::list<zObject *>::iterator it;
			for(it = sceneNpc->m_TemObj.begin(); it != sceneNpc->m_TemObj.end(); it++)
			{
				if( (*it)->data.qwThisID == pCmd->qwThisID )
				{
					SceneUser * pUser = SceneUserManager::getMe().getUserByTempID( pCmd->UserTemID );
					if( !pUser )
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"被分配的队员现在处于离线状态!");
					}
					else
					{
						if(pUser->packs.uom.space(pUser) >= 1)
						{
							zObject * obj = *it;
							it = sceneNpc->m_TemObj.erase(it);				//sky 先从怪物尸体中的物品列表里删除这个物品
							pUser->packs.addObject(obj, true, AUTO_PACK);	//sky 再添加到被分配人的包袱中

							char msg[MAX_PATH];
							sprintf(msg, "%s 被队长 %s 分配到物品 %s", pUser->charbase.name, this->charbase.name, obj->data.strName );
							Channel::sendTeam(TeamThisID, msg);

							Cmd::stAddObjectPropertyUserCmd send;
							bcopy(&obj->data,&send.object,sizeof(t_Object),sizeof(send.object));
							pUser->sendCmdToMe(&send,sizeof(send));
							return true;
						}
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"被分配队员包裹没有空位!");
						}
					}
				}
			}
		}
		break;
	case RT_TEAM_ROLL_ITEM_START:
		{
			stTeamRollItemStartUserCmd * pCmd = (stTeamRollItemStartUserCmd *)ptCmd;

			zPos p;
			zSceneObject *ret = NULL;
			p.x = pCmd->itemX;
			p.y = pCmd->itemY;
			ret = scene->getSceneObjectByPos( p );
			if (ret)
			{
				//int ok = 0;
				zObject *o=ret->getObject();
				if (!o)
				{
					return true;
				}

				//sky 开始ROLL的时候把地面物品的保护时间延迟90秒(以免ROLL还没结束物品就到拉保护时间被人拾取拉)
				ret->setprotectTime( 90 );

				//sky 判断物品的保护ID是否和拾取人的队伍ID一致
				if(!ret->getOwner())
				{
					TeamManager *team = SceneManager::getInstance().GetMapTeam(TeamThisID);

					if(team && ret->getOwner() == team->getTeamtempId())
					{
						if( team->GetTeamMemberNum() > 1 ) //sky 判断队伍中真实在线的人必须超过2个才ROLL
						{
					
							if( team->bRoll )
							{
								//sky 将该物品的唯一ID保存到队伍被ROLL物品中同时开始设置定时器和ROLL标志
								team->SetRollItem( p, scene->tempid );
								team->dwRollTime = ROLL_MAX_TIME;
								team->bRoll = true;

								//sky 通知所以队伍成员开始ROLL
								team->NoticeRoll( o );
							}
							else
								Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"请先等上一次的ROLL结束后在拾取!");
						
						}
						else //sky 否则直接把物品给这个拾取的玩家
						{
							if( packs.uom.space(this) >= 1)
							{
								//sky 通知客户端跟新地面物品被删除
								Cmd::stRemoveMapObjectMapScreenUserCmd re;
								re.dwMapObjectTempID=ret->id;
								scene->sendCmdToNine(ret->getPosI(),&re,sizeof(re),ret->dupIndex);

								//sky 先冲地面上删除这个物品
								scene->removeObject(ret);
								SAFE_DELETE(ret);

								//sky 在把他添加到玩家的包袱中
								packs.addObject( o, true, AUTO_PACK );

								//sky 通知玩家客户端跟新包袱里的物品信息
								Cmd::stAddObjectPropertyUserCmd send;
								bcopy(&o->data,&send.object,sizeof(t_Object),sizeof(send.object));
								sendCmdToMe(&send,sizeof(send));
							}
							else
							{
								Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"包裹里没有空位置拉!");
							}
						}

						return true;
					}
				}
				else
				{
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"这件物品不属于你！");
					return true;
				}
			}
		}
		break;
	case RT_TEAM_ROLL_ITEM_USERTYPE:
		{
			stTeamRollItemTypeUserCmd * pCmd = (stTeamRollItemTypeUserCmd *)ptCmd;

			TeamManager * team = SceneManager::getInstance().GetMapTeam(TeamThisID);

			if( team )
			{
				if(!team->bRoll )
				{
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"非法ROLL消息！");
					return true;
				}

				char msg[MAX_PATH];
				int num;

				num = team->SetMemberRoll( this->tempid, pCmd->Rolltype );

				if( pCmd->Rolltype == Roll_GiveUp )
				{
					sprintf(msg, "ROLL信息:%s放弃", charbase.name );
				}
				else if( pCmd->Rolltype == Roll_Greed )
				{
					sprintf(msg, "ROLL信息:%s 贪婪 点数为 %d", charbase.name, num );
				}
				else if( pCmd->Rolltype == Roll_Need )
				{
					sprintf(msg, "ROLL信息:%s 需求 点数为 %d", charbase.name, num );
				}

				Channel::sendTeam(team->getTeamtempId(), msg);
			}
			return true;
		}
		break;
	case RT_MAKE_TURRET_USERCMD:	//sky 这个是制造炮台消息和上面的分配消息已经完全没关系拉^_^
		{
			stMakeTurretUserCmd * pCmd = (stMakeTurretUserCmd*)ptCmd;

			zObject *srcobj=packs.uom.getObjectByThisID(pCmd->dwThisID);

			//sky 如果建筑类的物品不存在或者传递过来的物品类型不是建筑类的物品
			if(!srcobj || srcobj->base->kind != ItemType_Building)
			{
				Zebra::logger->debug("建造:物品已经不存在或者物品类型不是建筑类物品,用户:%s 物品ID:%u", name, pCmd->dwThisID);
				return true;
			}

			zNpcB *base = npcbm.get( srcobj->base->maxhp );

			if(base)
			{
				zPos pos;
				pos.x = pCmd->pos.x;
				pos.y = pCmd->pos.y;

				if(!scene->checkBlock( pos ) && !scene->checkBlock(pos, TILE_NOCREATE) )
				{
					ScenePet *npc = summonPet(base->id ,Cmd::PET_TYPE_TURRET,0,0,"",0,pos,4);

					if(npc)
					{
						//sky 把该NPC设置为建造状态
						npc->showCurrentEffect(Cmd::NPCSTATE_MAKE, true);
						npc->MakeTime = srcobj->base->maxsp;

						if(--srcobj->data.dwNum)
						{
							Cmd::stRefCountObjectPropertyUserCmd send;
							send.qwThisID=srcobj->data.qwThisID;
							send.dwNum=srcobj->data.dwNum;
							sendCmdToMe(&send,sizeof(send));
						}
						else
						{
							return packs.removeObject(srcobj);
						}
					}
					else
						Zebra::logger->debug("用户:%s 企图建造一个不存在的NPC！", name);
				}
				else
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"这里不允许建造！");
			}
		}
		break;
	case RT_NPC_START_CHANGE_USERCMD:	//sky  这个又变成NPC变身的处理拉，我承认我懒的加父消息拉otz
		{
			stNpcStartChangeUserCmd * pCmd = (stNpcStartChangeUserCmd*)ptCmd;
			SceneNpc *sceneNpc= SceneNpcManager::getMe().getNpcByTempID( pCmd->npcid );

			if(sceneNpc)
			{
				if(!sceneNpc->ChangeNpc())
					Zebra::logger->debug("NPC %s 变身失败!",sceneNpc->define->name);
			}
			else
				Zebra::logger->debug("无法找到唯一ID为 %u 的NPC",pCmd->npcid);
		}
		break;
	case PACK_BUYTAB_NUM_USERCMD:	//sky 购买包袱页的处理,反正都已经做拉怎么多无关的消息拉,再加一个也没关系吧 ^_^
		{
			stPackBuyTanbNumUserCmd * pCmd = (stPackBuyTanbNumUserCmd*)ptCmd;
			if(pCmd->PackType == PACKE_TYPE)
			{
				switch(packs.main.TabNum)
				{
				case 2:
					{
						if (!packs.checkMoney( 5000 ) || !packs.removeMoney(5000,"购买包袱页3"))
						{
							Channel::sendSys(this, Cmd::INFO_TYPE_FAIL, "50个银币！钱都没有！还想要包袱！！先去赚钱吧");
							return true;
						}
						else
							packs.main.TabNum += 1;
					}
					break;
				case 3:
					{
						if(packs.removeGold( 50, "购买包袱页4" ))
							packs.main.TabNum += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL, "50点游戏点卷！钱都没有！还想要包袱！！先去赚钱吧");
							return true;
						}
					}
					break;
				case 4:
					{
						if(packs.removeGold( 200, "购买包袱页5" ))
							packs.main.TabNum += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"200点游戏点卷！钱都没有！还想要包袱！！先去赚钱吧");
							return true;
						}
					}
					break;
				default:
					{
						Zebra::logger->error("玩家:%s 购买包袱的时候 发现错误的可用页数量%d", name, packs.main.TabNum);
						return true;
					}
					break;
				}

				stPackBuyTanbNumUserCmd Cmd;
				Cmd.TabNum = packs.main.TabNum;
				Cmd.PackType = PACKE_TYPE;
				sendCmdToMe(&Cmd,sizeof(stPackBuyTanbNumUserCmd));
			}
			else if(pCmd->PackType == SAVEBOX_TYPE)
			{
				switch(packs.store.days)
				{
				case 2:
					{
						/*if (!packs.checkMoney( 5000 ) || !packs.removeMoney(5000,"购买仓库页3"))
						{
							Channel::sendSys(this, Cmd::INFO_TYPE_FAIL, "50个银币！钱都没有！还想要仓库！！先去赚钱吧");
							return true;
						}
						else*/
							packs.store.days += 1;
					}
					break;
				case 3:
					{
						if(packs.removeGold( 50, "购买仓库页4" ))
							packs.store.days += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL, "50点游戏点卷！钱都没有！还想要仓库！！先去赚钱吧");
							return true;
						}
					}
					break;
				case 4:
					{
						if(packs.removeGold( 200, "购买仓库页5" ))
							packs.store.days += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"200点游戏点卷！钱都没有！还想要仓库！！先去赚钱吧");
							return true;
						}
					}
					break;
				default:
					{
						Zebra::logger->error("玩家:%s 购买仓库的时候 发现错误的可用页数量%d", name, packs.main.TabNum);
						return true;
					}
					break;
				}

				packs.store.notify(this);
			}		
		}
		break;
	default:
		break;
	}

	return true;
}

bool SceneUser::doArenaCmd(const Cmd::stArenaUserCmd *ptCmd, DWORD cmdLen)
{
	using namespace Cmd;
	switch(ptCmd->byParam)
	{
	case SURPLUS_MOSAIGEM_USERCMD_ADD:
		{
			stArenaQueuingUserCmd * pCmd = (stArenaQueuingUserCmd*)ptCmd;
			switch(pCmd->Type)
			{
			case 0:		//sky 用户单人排队
				{
					if(pCmd->UserID == this->tempid)
					{
						Cmd::Session::t_Sports_AddMeToQueuing Cmd;
						Cmd.AddMeType = pCmd->AddMeType;
						Cmd.UserID = this->id;
						Cmd.Type = pCmd->Type;
						sessionClient->sendCmd(&Cmd, sizeof(Cmd::Session::t_Sports_AddMeToQueuing));
					}
				}
				break;
			case 1:		//sky 队伍排队形式
				{
					TeamManager * teamM = SceneManager::getInstance().GetMapTeam(TeamThisID);
					if(teamM)
					{
						//sky 必须自己是队长才可以队排战场
						if(this->tempid == teamM->getLeader())
						{
							Cmd::Session::t_Sports_AddMeToQueuing Cmd;
							Cmd.AddMeType = pCmd->AddMeType;
							Cmd.Type = pCmd->Type;
							Cmd.UserID = teamM->getTeamtempId();
							sessionClient->sendCmd(&Cmd, sizeof(Cmd::Session::t_Sports_AddMeToQueuing));
						}
					}
				}
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return true;
}


//[sky] 根据格子位置获取格子里的物品指针
zObject * SceneUser::GetObjectBydst( stObjectLocation * dst )
{
	Package *destpack = packs.getPackage(dst->loc(),dst->tab());
	zObject *destObj = NULL;
	destpack->getObjectByZone(&destObj,dst->xpos(),dst->ypos());

	return destObj;
}

