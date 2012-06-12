/*
*\ brief:�����û�������ص���Ϣ
*\ auth: ��ε
*\ date: 2008-02-29
*/



//#include <zebra/ScenesServer.h>
#include "scriptTickTask.h"
#include "duplicateManager.h"
#include <zebra/csBox.h>
#include <zebra/csTurn.h>
#include "boxCircle.h"

//����û��ύ����Ʒ�Ƿ��Ǹ�����Ҫ��
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
		Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�㻹û����ƥ!");
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
			case 0:	//��ƥѵ��1
				{
					if (!packs.checkMoney( 1000 ) || !packs.removeMoney(1000,"������ƥѵ��"))
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"С��,û��ô��Ǯ�ͱ������ң�");
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
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�����һ��ս��ѵ������");
				}
				break;
			case 1:	//��ƥѵ��2
				{
					if (!packs.checkMoney( 2000 ) || !packs.removeMoney(2000,"������ƥѵ��"))
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"С��,û��ô��Ǯ�ͱ������ң�");
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
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"����ö���ս��ѵ������");
				}
				break;
			case 2:	//��ƥѵ��3
				{
					if (!packs.checkMoney( 5000 ) || !packs.removeMoney(5000,"������ƥѵ��"))
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"С��,û��ô��Ǯ�ͱ������ң�");
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
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"���������ս��ѵ������");
				}
				break;
			case 3:	//���ѵ��
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
					Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"����ս��ѵ�������Ѿ������");
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
  //�ͻ��˷���Կ�ױ������Ϣ
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



			if(CreateError)//�����������
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


			//���ٳ�Ĭ����Ʒ��Ŀ����Ʒ�����������Ʒ
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
  //�û�������пؼ��� ��ȡ ����
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


//[sky] װ�����ɼӵ㴦����
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
					Zebra::logger->error("��� %s ���������ɵ������װ����ʣ������ɵ���������ʧ�ܣ���\n", this->name );
					return true;
				}

				switch( pCmd->Add_Type )
				{
				case Cmd::PPT_STR:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //�Ȱ�ʣ���������
						if(0xffff - obj->data.Freedom.str_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.str_Attribute += pCmd->Add_Num;	//�ٰ����Ե���䵽�����������
					}
					break;
				case Cmd::PPT_INTE:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //�Ȱ�ʣ���������
						if(0xffff - obj->data.Freedom.inte_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.inte_Attribute += pCmd->Add_Num;	//�ٰ����Ե���䵽�����������
					}
					break;
				case Cmd::PPT_DEX:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //�Ȱ�ʣ���������
						if(0xffff - obj->data.Freedom.dex_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.dex_Attribute += pCmd->Add_Num;		//�ٰ����Ե���䵽�����������
					}
					break;
				case Cmd::PPT_SPI:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //�Ȱ�ʣ���������
						if(0xffff - obj->data.Freedom.spi_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.spi_Attribute += pCmd->Add_Num;		//�ٰ����Ե���䵽�����������
					}
					break;
				case Cmd::PPT_CON:
					{
						obj->data.Freedom.Surplus_Attribute -= pCmd->Add_Num;  //�Ȱ�ʣ���������
						if(0xffff - obj->data.Freedom.con_Attribute >= pCmd->Add_Num )
							obj->data.Freedom.con_Attribute += pCmd->Add_Num;		//�ٰ����Ե���䵽�����������
					}
					break;
				default:
					break;
				}

				//ˢ�¿ͻ��˵���Ʒ��Ϣ
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

//sky ��ʯ��Ƕ������
bool SceneUser::doMosaicGenCmd(const Cmd::stMakeObjectUserCmd *ptCmd,DWORD cmdLen)
{
	using namespace Cmd;
	switch(ptCmd->byParam)
	{
	case SURPLUS_MOSAIGEM_USERCMD_ADD:		//sky ��Ƕ��ʯ
		{
			stMosaicGemUserCmd * pCmd = (stMosaicGemUserCmd *)ptCmd;

			zObject * Eobj = GetObjectBydst( &(pCmd->Epos) );
			zObject * Gobj = GetObjectBydst( &(pCmd->Gpos) );

			if( Hole::put_hole(Eobj ,pCmd->index ,Gobj) )
			{
				//�Ӱ�����ɾ����ʯ
				packs.removeObject(Gobj);

				//ˢ�¿ͻ��˵���Ʒ��Ϣ
				Cmd::stAddObjectPropertyUserCmd ret;
				ret.byActionType = Cmd::EQUIPACTION_REFRESH;
				bcopy(&Eobj->data,&ret.object,sizeof(t_Object),sizeof(t_Object));
				sendCmdToMe(&ret,sizeof(ret));
			}
		}
		break;
	case RT_NPC_DIRITEM_USERCMD_PARA:		//sky �ӳ���ѯNPCʬ���е���Ʒ�б�
		{
			stNpcDirItemUserCmd * pCmd = (stNpcDirItemUserCmd *)ptCmd;

			npc_dwNpcTempID = pCmd->dwNpcTempID;
			SceneNpc *sceneNpc= SceneNpcManager::getMe().getNpcByTempID( npc_dwNpcTempID );

			if( !sceneNpc )
			{
				Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"Ŀ�겻����.");
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
							memccpy( Cmd->fen_team[f].name, ztTeam->name, 33,sizeof(Cmd->fen_team[f].name) );
						}
					}


					Cmd->count = sceneNpc->m_TemObj.size();
					i = 0;
					std::list<zObject *>::iterator it;
					for( it = sceneNpc->m_TemObj.begin(); it != sceneNpc->m_TemObj.end(); it++ )
					{
						if( i < Cmd->count )
						{
							memccpy( &(Cmd->objects[i]), &((*it)->data), sizeof(t_Object),sizeof(Cmd->objects[i]) );
							i++;
						}
					}

					sendCmdToMe(Cmd,nLen);

					free( buffer );
				}
			}
		}
		break;
	case RT_NPC_GIVEITEM_USERCMD:		//sky �ѷ��������Ʒ��ӵ��������˵İ�����
		{
			stNpcGiveItemUserCmd * pCmd = (stNpcGiveItemUserCmd *)ptCmd;

			SceneNpc *sceneNpc= SceneNpcManager::getMe().getNpcByTempID( npc_dwNpcTempID );
			if( !sceneNpc )
			{
				Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��NPC�Ѿ���������!");
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
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"������Ķ�Ա���ڴ�������״̬!");
					}
					else
					{
						if(pUser->packs.uom.space(pUser) >= 1)
						{
							zObject * obj = *it;
							it = sceneNpc->m_TemObj.erase(it);				//sky �ȴӹ���ʬ���е���Ʒ�б���ɾ�������Ʒ
							pUser->packs.addObject(obj, true, AUTO_PACK);	//sky ����ӵ��������˵İ�����

							char msg[MAX_PATH];
							sprintf(msg, "%s ���ӳ� %s ���䵽��Ʒ %s", pUser->charbase.name, this->charbase.name, obj->data.strName );
							Channel::sendTeam(TeamThisID, msg);

							Cmd::stAddObjectPropertyUserCmd send;
							bcopy(&obj->data,&send.object,sizeof(t_Object),sizeof(send.object));
							pUser->sendCmdToMe(&send,sizeof(send));
							return true;
						}
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�������Ա����û�п�λ!");
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

				//sky ��ʼROLL��ʱ��ѵ�����Ʒ�ı���ʱ���ӳ�90��(����ROLL��û������Ʒ�͵�������ʱ�䱻��ʰȡ��)
				ret->setprotectTime( 90 );

				//sky �ж���Ʒ�ı���ID�Ƿ��ʰȡ�˵Ķ���IDһ��
				if(!ret->getOwner())
				{
					TeamManager *team = SceneManager::getInstance().GetMapTeam(TeamThisID);

					if(team && ret->getOwner() == team->getTeamtempId())
					{
						if( team->GetTeamMemberNum() > 1 ) //sky �ж϶�������ʵ���ߵ��˱��볬��2����ROLL
						{
					
							if( team->bRoll )
							{
								//sky ������Ʒ��ΨһID���浽���鱻ROLL��Ʒ��ͬʱ��ʼ���ö�ʱ����ROLL��־
								team->SetRollItem( p, scene->tempid );
								team->dwRollTime = ROLL_MAX_TIME;
								team->bRoll = true;

								//sky ֪ͨ���Զ����Ա��ʼROLL
								team->NoticeRoll( o );
							}
							else
								Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���ȵ���һ�ε�ROLL��������ʰȡ!");
						
						}
						else //sky ����ֱ�Ӱ���Ʒ�����ʰȡ�����
						{
							if( packs.uom.space(this) >= 1)
							{
								//sky ֪ͨ�ͻ��˸��µ�����Ʒ��ɾ��
								Cmd::stRemoveMapObjectMapScreenUserCmd re;
								re.dwMapObjectTempID=ret->id;
								scene->sendCmdToNine(ret->getPosI(),&re,sizeof(re),ret->dupIndex);

								//sky �ȳ������ɾ�������Ʒ
								scene->removeObject(ret);
								SAFE_DELETE(ret);

								//sky �ڰ�����ӵ���ҵİ�����
								packs.addObject( o, true, AUTO_PACK );

								//sky ֪ͨ��ҿͻ��˸��°��������Ʒ��Ϣ
								Cmd::stAddObjectPropertyUserCmd send;
								bcopy(&o->data,&send.object,sizeof(t_Object),sizeof(send.object));
								sendCmdToMe(&send,sizeof(send));
							}
							else
							{
								Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"������û�п�λ����!");
							}
						}

						return true;
					}
				}
				else
				{
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����Ʒ�������㣡");
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
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�Ƿ�ROLL��Ϣ��");
					return true;
				}

				char msg[MAX_PATH];
				int num;

				num = team->SetMemberRoll( this->tempid, pCmd->Rolltype );

				if( pCmd->Rolltype == Roll_GiveUp )
				{
					sprintf(msg, "ROLL��Ϣ:%s����", charbase.name );
				}
				else if( pCmd->Rolltype == Roll_Greed )
				{
					sprintf(msg, "ROLL��Ϣ:%s ̰�� ����Ϊ %d", charbase.name, num );
				}
				else if( pCmd->Rolltype == Roll_Need )
				{
					sprintf(msg, "ROLL��Ϣ:%s ���� ����Ϊ %d", charbase.name, num );
				}

				Channel::sendTeam(team->getTeamtempId(), msg);
			}
			return true;
		}
		break;
	case RT_MAKE_TURRET_USERCMD:	//sky �����������̨��Ϣ������ķ�����Ϣ�Ѿ���ȫû��ϵ��^_^
		{
			stMakeTurretUserCmd * pCmd = (stMakeTurretUserCmd*)ptCmd;

			zObject *srcobj=packs.uom.getObjectByThisID(pCmd->dwThisID);

			//sky ������������Ʒ�����ڻ��ߴ��ݹ�������Ʒ���Ͳ��ǽ��������Ʒ
			if(!srcobj || srcobj->base->kind != ItemType_Building)
			{
				Zebra::logger->debug("����:��Ʒ�Ѿ������ڻ�����Ʒ���Ͳ��ǽ�������Ʒ,�û�:%s ��ƷID:%u", name, pCmd->dwThisID);
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
						//sky �Ѹ�NPC����Ϊ����״̬
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
						Zebra::logger->debug("�û�:%s ��ͼ����һ�������ڵ�NPC��", name);
				}
				else
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���ﲻ�����죡");
			}
		}
		break;
	case RT_NPC_START_CHANGE_USERCMD:	//sky  ����ֱ��NPC����Ĵ��������ҳ��������ļӸ���Ϣ��otz
		{
			stNpcStartChangeUserCmd * pCmd = (stNpcStartChangeUserCmd*)ptCmd;
			SceneNpc *sceneNpc= SceneNpcManager::getMe().getNpcByTempID( pCmd->npcid );

			if(sceneNpc)
			{
				if(!sceneNpc->ChangeNpc())
					Zebra::logger->debug("NPC %s ����ʧ��!",sceneNpc->define->name);
			}
			else
				Zebra::logger->debug("�޷��ҵ�ΨһIDΪ %u ��NPC",pCmd->npcid);
		}
		break;
	case PACK_BUYTAB_NUM_USERCMD:	//sky �������ҳ�Ĵ���,�������Ѿ�������ô���޹ص���Ϣ��,�ټ�һ��Ҳû��ϵ�� ^_^
		{
			stPackBuyTanbNumUserCmd * pCmd = (stPackBuyTanbNumUserCmd*)ptCmd;
			if(pCmd->PackType == PACKE_TYPE)
			{
				switch(packs.main.TabNum)
				{
				case 2:
					{
						if (!packs.checkMoney( 5000 ) || !packs.removeMoney(5000,"�������ҳ3"))
						{
							Channel::sendSys(this, Cmd::INFO_TYPE_FAIL, "50�����ң�Ǯ��û�У�����Ҫ����������ȥ׬Ǯ��");
							return true;
						}
						else
							packs.main.TabNum += 1;
					}
					break;
				case 3:
					{
						if(packs.removeGold( 50, "�������ҳ4" ))
							packs.main.TabNum += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL, "50����Ϸ���Ǯ��û�У�����Ҫ����������ȥ׬Ǯ��");
							return true;
						}
					}
					break;
				case 4:
					{
						if(packs.removeGold( 200, "�������ҳ5" ))
							packs.main.TabNum += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"200����Ϸ���Ǯ��û�У�����Ҫ����������ȥ׬Ǯ��");
							return true;
						}
					}
					break;
				default:
					{
						Zebra::logger->error("���:%s ���������ʱ�� ���ִ���Ŀ���ҳ����%d", name, packs.main.TabNum);
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
						/*if (!packs.checkMoney( 5000 ) || !packs.removeMoney(5000,"����ֿ�ҳ3"))
						{
							Channel::sendSys(this, Cmd::INFO_TYPE_FAIL, "50�����ң�Ǯ��û�У�����Ҫ�ֿ⣡����ȥ׬Ǯ��");
							return true;
						}
						else*/
							packs.store.days += 1;
					}
					break;
				case 3:
					{
						if(packs.removeGold( 50, "����ֿ�ҳ4" ))
							packs.store.days += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL, "50����Ϸ���Ǯ��û�У�����Ҫ�ֿ⣡����ȥ׬Ǯ��");
							return true;
						}
					}
					break;
				case 4:
					{
						if(packs.removeGold( 200, "����ֿ�ҳ5" ))
							packs.store.days += 1;
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"200����Ϸ���Ǯ��û�У�����Ҫ�ֿ⣡����ȥ׬Ǯ��");
							return true;
						}
					}
					break;
				default:
					{
						Zebra::logger->error("���:%s ����ֿ��ʱ�� ���ִ���Ŀ���ҳ����%d", name, packs.main.TabNum);
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
			case 0:		//sky �û������Ŷ�
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
			case 1:		//sky �����Ŷ���ʽ
				{
					TeamManager * teamM = SceneManager::getInstance().GetMapTeam(TeamThisID);
					if(teamM)
					{
						//sky �����Լ��Ƕӳ��ſ��Զ���ս��
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


//[sky] ���ݸ���λ�û�ȡ���������Ʒָ��
zObject * SceneUser::GetObjectBydst( stObjectLocation * dst )
{
	Package *destpack = packs.getPackage(dst->loc(),dst->tab());
	zObject *destObj = NULL;
	destpack->getObjectByZone(&destObj,dst->xpos(),dst->ypos());

	return destObj;
}

