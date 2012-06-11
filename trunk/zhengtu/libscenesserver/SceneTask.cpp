/**
* \brief �����½��������
*
*/
//#include <zebra/ScenesServer.h>
#include "scriptTickTask.h"
#include <zebra/csTurn.h>
//#include "csBox.h"

SceneTask::~SceneTask()
{
	Zebra::logger->info("����(%u)�ر�",wdServerID);
}

/**
* \brief ��֤��½����������������ָ��
*
* �����֤��ͨ��ֱ�ӶϿ�����
*
* \param ptCmd ��½ָ��
* \return ��֤�Ƿ�ɹ�
*/
bool SceneTask::verifyLogin(const Cmd::Scene::t_LoginScene *ptCmd)
{
	using namespace Cmd::Scene;

	if (CMD_LOGIN == ptCmd->cmd
		&& PARA_LOGIN == ptCmd->para)
	{
		const Cmd::Super::ServerEntry *entry = ScenesService::getInstance().getServerEntryById(ptCmd->wdServerID);
		char strIP[32];
		strncpy(strIP,getIP(),sizeof(strIP));
		if (entry
			&& ptCmd->wdServerType == entry->wdServerType
			&& 0 == strcmp(strIP,entry->pstrIP))
		{
			wdServerID   = ptCmd->wdServerID;
			wdServerType = ptCmd->wdServerType;
			return true;
		}
	}

	return false;
}
/**
* \brief ��֤����
*
* \return ��֤�Ƿ�ɹ�
*/
int SceneTask::verifyConn()
{
	int retcode = mSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
		if (nCmdLen <= 0)
			//����ֻ�Ǵӻ���ȡ���ݰ�,���Բ������,û������ֱ�ӷ���
			return 0;
		else
		{
			using namespace Cmd::Scene;
			if (verifyLogin((t_LoginScene *)pstrCmd))
			{
				Zebra::logger->debug("�ͻ�������ͨ����֤");
				veriry_ok=true; 
				return 1;
			}
			else
			{
				Zebra::logger->error("�ͻ���������֤ʧ��");
				return -1;
			}
		}
	}
	else
		return retcode;
}

int SceneTask::waitSync()
{
	return 1;
}
bool SceneTask::checkRecycle()
{
	if (recycle_state == 0)
	{
		return false;
	}
	if (recycle_state == 1)
	{
		SceneUserManager::getMe().removeUserByTask(this);
		char Buf[zSocket::MAX_DATASIZE];
		bzero(Buf,sizeof(Buf));
		Cmd::Scene::t_Remove_MapIndex *map = (Cmd::Scene::t_Remove_MapIndex *)Buf;
		constructInPlace(map);
		struct GateMapRemoveExec : public SceneCallBack
		{
			Cmd::Scene::t_Remove_MapIndex *_data;
			GateMapRemoveExec(Cmd::Scene::t_Remove_MapIndex *data):_data(data){_data->dwSize=0;} 
			bool exec(Scene *scene)
			{
				if (scene)
				{
					_data->dwMapTempID[_data->dwSize]=scene->tempid;
					_data->dwSize++; 
				}
				return true; 
			}
		};
		GateMapRemoveExec exec(map);
		SceneManager::getInstance().execEveryScene(exec);
		this->sendCmd(map,sizeof(Cmd::Scene::t_Remove_MapIndex) + map->dwSize * sizeof(DWORD));
		recycle_state=2;
		return true;
	}
	return true;
}

int SceneTask::recycleConn()
{
	if (veriry_ok)
	{
		switch(recycle_state)
		{
		case 0:
			{
				recycle_state=1;
				return 0;
			}
			break;
		case 1:
			{
				return 0;
			}
			break;
		case 2:
			{
				return 1;
			}
			break;
		}
	}
	Zebra::logger->debug("����%sʹ���ڴ�:%d",getIP(),mSocket->getBufferSize());
	return 1;
}

bool SceneTask::uniqueAdd()
{
	return SceneTaskManager::getInstance().uniqueAdd(this);
}

bool SceneTask::uniqueRemove()
{
	return SceneTaskManager::getInstance().uniqueRemove(this);
}

/**
* \brief Bill�û��������
*
* \param pUser ���
* \param pNullCmd �û�����
* \param cmdLen �����
*
* \return ����õ�����,����TRUE,����FALSE
*/
bool SceneTask::usermsgParseBill(SceneUser *pUser,const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
	using namespace Cmd::Bill;

	switch(pNullCmd->cmd)
	{
	case CMD_STOCK:
		{
			switch(pNullCmd->para)
			{
			case PARA_STOCK_FETCH:
				{
					t_Stock_Fetch *ret = (t_Stock_Fetch *)pNullCmd;
					if (ret->dwMoney)
					{
						pUser->packs.addMoney(ret->dwMoney,"��Ʊ����");
					}
					if (ret->dwGold)
					{
						pUser->packs.addGold(ret->dwGold,"��Ʊ����");
					}
					Zebra::logger->debug("%s(%d)��Ʊ����,���:%d,����:%d",pUser->name,pUser->id,ret->dwGold,ret->dwMoney);
				}
				break;
			}
			return true;
		}
		break;
	}
	Zebra::logger->error("SceneTask::usermsgParseBill(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,cmdLen);
	return false;
}
/**
* \brief �û��������
*
* \param pUser ���
* \param pNullCmd �û�����
* \param cmdLen �����
*
* \return ����õ�����,����TRUE,����FALSE
*/
bool SceneTask::usermsgParse(SceneUser *pUser,const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
	using namespace Cmd;

	//fprintf(stderr,"�յ��û���Ϣ\n");
	//fprintf(stderr,"cmd:%u, para:%u\n",pNullCmd->cmd,pNullCmd->para);	



	scriptMessageFilter::exeScript(pUser,const_cast<Cmd::t_NullCmd*>(pNullCmd));/*->cmd,pNullCmd->para);*/
	if (pUser->getState() == SceneUser::SceneEntry_Death)
	{
		switch(pNullCmd->cmd)
		{
		case CHAT_USERCMD:
		case RELIVE_USERCMD:
		case GOLD_USERCMD:  
			break;
		case MOVE_USERCMD:
			{
				/*
				Cmd::stUserMoveMoveUserCmd ret;
				Cmd::stUserMoveMoveUserCmd *pret = (Cmd::stUserMoveMoveUserCmd *)pNullCmd;
				ret.dwUserTempID = pret->dwUserTempID;
				ret.byDirect = pret->byDirect;
				ret.bySpeed = 0;
				ret.x = 0;
				ret.y = 0;
				pUser->sendCmdToMe(&ret,sizeof(ret));
				*/
				return true;
			}
			break;
		case MAGIC_USERCMD:
			{
				ScenePk::attackFailToMe((Cmd::stAttackMagicUserCmd *)pNullCmd,pUser);
				return true;
			}
			break;
			/*
			case PROPERTY_USERCMD:
			case MAPSCREEN_USERCMD:
			case TRADE_USERCMD:
			case MAIL_USERCMD:
			// */
		default:
			{
				//Zebra::logger->debug("(%s,%ld)����״̬����ָ�� (%ld,%ld)",pUser->name,pUser->id,pNullCmd->cmd,pNullCmd->para);
				return true;
			}
			break;
		}
	}

	if (pUser->tradeorder.hasBegin()) {

		switch (pNullCmd->cmd)
		{
		case CHAT_USERCMD:
		case MOVE_USERCMD:
		case GOLD_USERCMD:  
			break;
		case TRADE_USERCMD:
			{
				switch(pNullCmd->para)
				{
				case REQUEST_TRADE_USERCMD_PARAMETER:
				case ANSWER_TRADE_USERCMD_PARAMETER:
				case BEGIN_TRADE_USERCMD_PARAMETER:
				case COMMIT_TRADE_USERCMD_PARAMETER:
				case FINISH_TRADE_USERCMD_PARAMETER:
				case CANCEL_TRADE_USERCMD_PARAMETER:
				case ADD_OBJECT_TRADE_USERCMD_PARAMETER:
				case REMOVE_OBJECT_TRADE_USERCMD_PARAMETER:
				case UPDATE_TRADE_MONEY_USERCMD:	//Shx Add;
					break;
				default:
					return true;
				}
			}
			break;
		case MAPSCREEN_USERCMD:
			{
				switch(pNullCmd->para)
				{
				case REQUESTMAPNPCDATA_MAPSCREEN_USERCMD_PARA: //����npc����
				case REQUESTUSERDATA_MAPSCREEN_USERCMD_PARA: //������������
					break;
				default:
					{
						Zebra::logger->debug("(%s,%ld)����״̬����ָ�� (%ld,%ld)",pUser->name,pUser->id,pNullCmd->cmd,pNullCmd->para);
						return true;
					}
					break;
				}
			}
			break;
		default:
			{
				Zebra::logger->debug("(%s,%ld)����״̬����ָ�� (%ld,%ld)",pUser->name,pUser->id,pNullCmd->cmd,pNullCmd->para);
				return true;
			}
			break;

		}

	}

	if (pUser->privatestore.step() != PrivateStore::NONE) {
		switch (pNullCmd->cmd)
		{
		case CHAT_USERCMD:
		case GOLD_USERCMD:  
			break;
		case TRADE_USERCMD:
			{
				switch(pNullCmd->para)
				{
				case START_SELL_USERCMD_PARAMETER:
				case FINISH_SELL_USERCMD_PARAMETER:
				case ADD_OBJECT_SELL_USERCMD_PARAMETER:
				case REQUEST_ADD_OBJECT_SELL_USERCMD_PARAMETER:
				case REMOVE_OBJECT_SELL_USERCMD_PARAMETER:
				case REQUEST_SELL_INFO_USERCMD_PARAMETER:
				case UPDATE_SHOP_ADV_USERCMD_PARAMETER:
					//case REQUEST_SELL_BUY_USERCMD_PARAMETER:
					break;
				default:
					return true;
				}
			}
			break;
		case MAPSCREEN_USERCMD:
			{
				switch(pNullCmd->para)
				{
				case REQUESTMAPNPCDATA_MAPSCREEN_USERCMD_PARA: //����npc����
				case REQUESTUSERDATA_MAPSCREEN_USERCMD_PARA: //������������
					break;
				default:
					{
						Zebra::logger->debug("(%s,%ld)��̯״̬����ָ�� (%ld,%ld)",pUser->name,pUser->id,pNullCmd->cmd,pNullCmd->para);
						return true;
					}
					break;
				}
			}
			break;
		default:
			{
				Zebra::logger->debug("(%s,%ld)��̯״̬����ָ�� (%ld,%ld)",pUser->name,pUser->id,pNullCmd->cmd,pNullCmd->para);
				return true;
			}
			break;
		}
	}

	//Zebra::logger->debug("�û�ָ��(cmd=%u,para=%u,rev->size=%d",pNullCmd->cmd,pNullCmd->para,cmdLen);
	switch(pNullCmd->cmd)
	{
	case DATA_USERCMD:
		{
			switch(pNullCmd->para)
			{
			case LOADMAPOK_DATA_USERCMD_PARA:
				{
					//pUser->loadMapOK();
					//Zebra::logger->debug("%s(%d)���ص�ͼ���",pUser->name,pUser->id);
				}
				break;
			}
			return true;
		}
		break;
	case MAPSCREEN_USERCMD:
		{
			switch(pNullCmd->para)
			{
			case REQUESTUSERDATA_MAPSCREEN_USERCMD_PARA:
				{
					stRequestUserDataMapScreenUserCmd *rev=(stRequestUserDataMapScreenUserCmd *)pNullCmd;
					bool ret=pUser->requestUser(rev);
					return ret;
				}
				break;
			case REQUESTMAPNPCDATA_MAPSCREEN_USERCMD_PARA:
				{
					stRequestMapNpcDataMapScreenUserCmd *rev=(stRequestMapNpcDataMapScreenUserCmd *)pNullCmd;
					bool ret= pUser->requestNpc(rev);
					return ret;
				}
				break;
			case RIDE_MAPSCREEN_USERCMD_PARA:
				{
					stRideMapScreenUserCmd *rev=(stRideMapScreenUserCmd *)pNullCmd;
					bool ret= pUser->ride(rev);
					return ret;
				}
				break;
			case CHANGEFACE_MAPSCREEN_USERCMD_PARA:
				{
					Cmd::stChangeFaceMapScreenUserCmd *cmd = (Cmd::stChangeFaceMapScreenUserCmd *)pNullCmd;
					bool ret= pUser->changeFace(cmd,cmdLen);
					return ret;
				}
			default:
				break;
			}
		}
		break;
	case MOVE_USERCMD:
		{
			switch(pNullCmd->para)
			{
			case USERMOVE_MOVE_USERCMD_PARA:
				{
					Zebra::logger->debug("%s �յ��ƶ���Ϣ",pUser->name);
					Cmd::stUserMoveMoveUserCmd *rev=(Cmd::stUserMoveMoveUserCmd *)pNullCmd;
					/// �˳�״̬�������κ�ָ��
					if (pUser->unReging)
					{
						return true;
					}
					//������С��Ϸ,�����ƶ�
					if (pUser->miniGame) return true;
					//sky �־��ʧ��״̬�����ƶ�
					if (pUser->blind || pUser->dread) return true;
					bool ret= pUser->move(rev);
					Zebra::logger->debug("%s �ƶ��������",pUser->name);
					return ret;
				}
				break;
			case SITDOWN_MOVE_USERCMD_PARA:
				{
					/// �˳�״̬�������κ�ָ��
					if (pUser->unReging)
					{
						return true;
					}
					if (!pUser->horse.mount()) pUser->sitdown();
					return true;
				}
				break;
			default:
				break;
			}
		}
		break;
	case MAGIC_USERCMD:
		{
			switch( pNullCmd->para)
			{
			case MAGIC_USERCMD_PARA:
				{
					Cmd::stAttackMagicUserCmd *rev = (Cmd::stAttackMagicUserCmd *)pNullCmd;
					/// �˳�״̬�������κ�ָ��
					if (pUser->unReging) return true;
#ifdef _DEBUG
					Zebra::logger->error("!!!----�յ�������Ϣ,������Ϣ�ĳ���Ϊ[%u]",cmdLen);
#endif
 					return pUser->attackMagic( rev,cmdLen);
				}
				break;

			case BACKOFF_USERCMD_PARA:
				{
					//TODO
				}
				break;

			case PKMODE_USERCMD_PARA:
				{
					Cmd::stPKModeUserCmd *rev = (Cmd::stPKModeUserCmd *)pNullCmd;
					bool bret =  pUser->switchPKMode(rev);
					return bret;
				}
				break;
			case UNCOMBIN_USERCMD_PARA:
				{
					Cmd::stUnCombinUserCmd *rev = (Cmd::stUnCombinUserCmd *)pNullCmd;
					bool bret =  pUser->unCombin(rev);
					return bret;
				}
				break;
			case FIREWORK_USERCMD_PARA:
				{
					Cmd::stFireWorkUserCmd *rev = (Cmd::stFireWorkUserCmd *)pNullCmd;
					if (rev->qwObjectTempID>0)
					{
						zObject *srcobj=pUser->packs.uom.getObjectByThisID(rev->qwObjectTempID);
						if (srcobj && srcobj->data.pos.loc() ==Cmd::OBJECTCELLTYPE_COMMON)
						{
							zObject::logger(srcobj->createid,srcobj->data.qwThisID,srcobj->data.strName,srcobj->data.dwNum,srcobj->data.dwNum,0,pUser->id,pUser->name,0,NULL,"ʩ���̻�������Ʒ",NULL,0,0);
							pUser->packs.removeObject(srcobj); //notify and delete
						}
						else
						{
							return true;
						}
					}
					if (pUser->scene) pUser->scene->sendCmdToNine(pUser->getPosI(),rev,cmdLen,pUser->dupIndex);
					return true;
				}
				break;
			default:
				break;
			}  
		}
		break;
	case RELIVE_USERCMD:
		switch(pNullCmd->para)
		{
		case OK_RELIVE_USERCMD_PARA:
			{
				Cmd::stOKReliveUserCmd *rev = (Cmd::stOKReliveUserCmd *)pNullCmd;
				return pUser->relive(rev->byType,0,100);
			}
			break;
		}
		break;
	case PROPERTY_USERCMD:
		return pUser->doPropertyCmd((Cmd::stPropertyUserCmd *)pNullCmd,cmdLen);
	case CHAT_USERCMD:
		return pUser->doChatCmd((Cmd::stChatUserCmd *)pNullCmd,cmdLen);
	case TRADE_USERCMD:
		{
			bool ret= pUser->doTradeCmd((Cmd::stTradeUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case TASK_USERCMD:
		{
			//bool ret = pUser->doTaskCmd((Cmd::stQuestUserCmd *)pNullCmd,cmdLen);
			bool ret = Quest::execute(*pUser,(Cmd::stQuestUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case UNION_USERCMD: // ��������
		{
			bool ret = pUser->doUnionCmd((Cmd::stUnionUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case SAFETY_USERCMD:
		{
			bool ret = pUser->doSafetyCmd((Cmd::stSafetyUserCmd*)pNullCmd,cmdLen);
			return ret;
		}
	case SEPT_USERCMD: // ���������
		{
			bool ret = pUser->doSeptCmd((Cmd::stSeptUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case SCHOOL_USERCMD:
		{
			bool ret = pUser->doSchoolCmd((Cmd::stSchoolUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case RELATION_USERCMD:
		{
			bool ret = pUser->doRelationCmd((Cmd::stRelationUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case DARE_USERCMD:
		{
			bool ret = pUser->doDareCmd((Cmd::stDareUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case NPCDARE_USERCMD:
		{
			bool ret = pUser->doNpcDareCmd((Cmd::stDareUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
	case PET_USERCMD:
		{
			bool ret = pUser->doPetCmd((Cmd::stPetUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case QUIZ_USERCMD:
		{
			bool ret = pUser->doQuizCmd((Cmd::stQuizUserCmd*)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case COUNTRY_USERCMD:
		{
			bool ret = pUser->doCountryCmd((Cmd::stCountryUserCmd*)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case ARMY_USERCMD:
		{
			bool ret = pUser->doArmyCmd((Cmd::stArmyUserCmd*)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case MAIL_USERCMD:
		{
			bool ret = pUser->doMailCmd((Cmd::stMailUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case AUCTION_USERCMD:
		{
			bool ret = pUser->doAuctionCmd((Cmd::stAuctionUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case CARTOON_USERCMD:
		{
			bool ret = pUser->doCartoonCmd((Cmd::stCartoonUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case PRISON_USERCMD:
		{
			bool ret = pUser->doPrisonCmd((Cmd::stPrisonUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case MINIGAME_USERCMD:
		{
			bool ret = pUser->doMiniGameCmd((Cmd::stMiniGameUserCmd *)pNullCmd,cmdLen);
			return ret;
		}
		break;
	case STOCK_SCENE_USERCMD:
		{
			return pUser->doStockCmd((Cmd::stStockSceneUserCmd*)pNullCmd,cmdLen);
		}
		break;
	case SAFETY_COWBOX:
		{
			//Zebra::logger->error("�յ�������Ϣ\n");
			return pUser->doBoxCmd((Cmd::stCowBoxUserCmd *)pNullCmd,cmdLen);
		}
		break;
	case TURN_USERCMD:
		{
			//fprintf(stderr,"�յ�ת����Ϣ\n");
			return pUser->doTurnCmd((Cmd::stTurnUserCmd *)pNullCmd,cmdLen);
		}
		break;
	case REMAKEOBJECT_USERCMD:
		{
			return pUser->doReMakeObjCmd((Cmd::stReMakUserCmd *)pNullCmd,cmdLen);
		}
		break;
	case HORSETRAINING_USERCMD:
		{
			return pUser->doHorseTrainingCmd( (Cmd::stReMakUserCmd *)pNullCmd,cmdLen );
		}
		break;
	case SURPLUS_ATTRIBUTE_USERCMD:   //װ�����ɼӵ���Ϣ sky
		{
			return pUser->doAddItemAttCmd( (Cmd::stAttruByteUserCmd *)pNullCmd,cmdLen );
		}
		break;
	case MAKEOBJECT_USERCMD:
		{
			return pUser->doMosaicGenCmd( (Cmd::stMakeObjectUserCmd *)pNullCmd,cmdLen );
		}
		break;
	case ARENA_USERCMD:				//ս��-����-���������ָ�� sky 
		{
			return pUser->doArenaCmd( (Cmd::stArenaQueuingUserCmd *)pNullCmd, cmdLen );
		}
		break;
	case GOLD_USERCMD:
		{
			switch(pNullCmd->para)
			{
			case REDEEM_MONTH_CARD_PARA:
				{
					Cmd::stRedeemMonthCard *rev = (Cmd::stRedeemMonthCard *)pNullCmd;
					pUser->sendCmdToMe(rev,sizeof(Cmd::stRedeemMonthCard));
					/*
					Zebra::logger->debug("%s(%u)�һ��¿�",pUser->name,pUser->id);
					Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"�һ��¿�,����ʱ��%d,ʣ��㿨%d,��������%d",rev->dwNum,rev->dwBalance,rev->byReturn);
					// */
					return true;
				}
				break;
			case REDEEM_GOLD_PARA:
				{

					Cmd::stRedeemGold *rev = (Cmd::stRedeemGold *)pNullCmd;
					if (rev->byReturn == Cmd::REDEEM_SUCCESS && (int)rev->dwNum >= 0)
					{
						DWORD temp=rev->dwNum/100;
						if ((int)temp>0)
						{
							pUser->charbase.goldgive +=temp;
							Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"��õ���Ʒ����������ʯ%d��,����Ե�����������Ŷ",temp);
						}
						pUser->packs.addGold(rev->dwNum,"�㿨�����",NULL,true,true);
						pUser->sendCmdToMe(rev,sizeof(Cmd::stRedeemGold));
					}
					else
					{
						Zebra::logger->debug("�㿨����ҳ��ָ���,��Ҫƽ̨ȷ����ֵ(%d,%d)",rev->dwNum,rev->byReturn);
					}
					/*
					Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"�һ����,��ǰ���%d,ʣ��㿨%d,��������%d",rev->dwNum,rev->dwBalance,rev->byReturn);
					Zebra::logger->debug("%s(%u)�һ����",pUser->name,pUser->id);
					// */
					return true;
				}
				break;
			case RETURN_CARD_AND_GOLD_PARA:
				{
					Cmd::stReturnCardAndGold *rev = (Cmd::stReturnCardAndGold *)pNullCmd;
					if (rev->byReturn == Cmd::REDEEM_SUCCESS)
					{
					}
					//pUser->sendCmdToMe(rev,sizeof(Cmd::stReturnCardAndGold));
					/*
					Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"��ѯ��Һ��¿�,���%d,�¿�%d,��������%d",rev->dwGold,rev->dwMonthCard,rev->byReturn);
					Zebra::logger->debug("%s(%u)��ѯ����¿�,���%d,�¿�%u",pUser->name,pUser->id,rev->dwGold,rev->dwMonthCard);
					// */
					return true;
				}
				break;
			case RETURN_CONSUME_CARD_PARA:
				{
					Cmd::stReturnConSumeCardCard *rev = (Cmd::stReturnConSumeCardCard *)pNullCmd;
					if (rev->byReturn == Cmd::REDEEM_SUCCESS)
					{
						switch(rev->byType)
						{
						case OBJ_GOLD_STONE://�;���������ʯ
							{
								//  pUser->charbase.goldgive += 50;
								//  Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"��õ���Ʒ����������ʯ50��,����Ե�����������Ŷ");
								pUser->Card_num += 1;

								Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"��õ���Ʒ����������ʯ�Ͳ���,����Ե�����������Ŷ");
							}
							break;
						case OBJ_GOLD_OBJECT://�ͽ�ɫװ��
							{
								pUser->charbase.accPriv |= SceneUser::ACCPRIV_GOLD_EQUIP_AT_5_15;
								if (pUser->charbase.level >= 15)
								{
									pUser->sendGiftEquip(15);
								}
								else if (pUser->charbase.level >= 5)
								{
									pUser->sendGiftEquip(5);
								}
								Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"�һ��ɹ�,ϵͳ������5����15��ʱ����һ�׻ƽ�װ��");
							}
							break;
						case OBJ_GREEN_OBJECT://����ʥװ��
							{
								pUser->charbase.accPriv |= SceneUser::ACCPRIV_GREEN_EQUIP_AT_5_25_50;
								if (pUser->charbase.level >= 50)
								{
									pUser->sendGiftEquip(50);
								}
								else if (pUser->charbase.level >= 25)
								{
									pUser->sendGiftEquip(25);
								}
								else if (pUser->charbase.level >= 5)
								{
									pUser->sendGiftEquip(5);
								}

								Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"�һ��ɹ�,ϵͳ������5����25����50��ʱ����һ����ɫװ��");
							}
							break;
						case SUBAT_GOLD://����ʥװ��
							{
								DWORD temp=rev->balance/100;
								if ((int)temp>0)
								{
									pUser->charbase.goldgive +=temp;
									Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"��õ���Ʒ����������ʯ%d��,����Ե�����������Ŷ",temp);
								}
								pUser->packs.addGold(rev->balance,"ר������ֵ",NULL,true,true);
								Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"����ר�����ɹ�");
							}
							break;
						}
					}
					pUser->sendCmdToMe(rev,sizeof(Cmd::stReturnConSumeCardCard));
					return true;
				}
				break;
			case REQUEST_CARD_AND_GOLD_PARA:
				{
					/// ���ڽ���ڽ�ɫ����,�����ѯ
					/*
					// �����Һ��¿�ʱ��
					Cmd::Scene::t_Request_Bill rb;
					pUser->gatetask->sendCmd(&rb,sizeof(rb));
					Zebra::logger->debug("%s(%u)�����ѯ��Һ��¿�",pUser->name,pUser->id);
					return true;
					// */
				}
				break;
			case REQUEST_POINT_PARA:
				{
					// ����ʣ��㿨
					Cmd::Scene::t_Request_Point rb;
					pUser->gatetask->sendCmd(&rb,sizeof(rb));
					//Zebra::logger->debug("%s(%u)�����ѯʣ��㿨",pUser->name,pUser->id);
					return true;
				}
				break;
			case RETURN_REQUEST_POINT_PARA:
				{
					Cmd::stReturnRequestPoint *rev = (Cmd::stReturnRequestPoint *)pNullCmd;
					pUser->sendCmdToMe(rev,sizeof(Cmd::stReturnRequestPoint));
					//Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"ʣ�����%d,��������%d",rev->dwPoint,rev->byReturn);
					//Zebra::logger->debug("%s(%u)�����ѯ�㿨",pUser->name,pUser->id);
					return true;
				}
				break;
			default:
				break;
			}
			return true;
		}
		break;
	case GMTOOL_USERCMD:
		{
			if (pNullCmd->para==Cmd::MSG_GMTOOL_PARA)
			{
				Cmd::stMsgGmTool * rev = (Cmd::stMsgGmTool *)pNullCmd;
				Zebra::logger->debug("[GM����]�յ�GM������Ϣ %s",pUser->name);
				if (rev->type==6)
				{
					ScenesService::wglogger->info("[���]%s,%u,%u,%s,%u,%s,",pUser->name,pUser->accid,pUser->id,rev->content,pUser->charbase.level,SceneManager::getInstance().getCountryNameByCountryID(pUser->charbase.country));
					return true;
				}

				if (pUser->charbase.msgTime>SceneTimeTick::currentTime.sec())
				{
					Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"�Բ���,��Сʱ��ֻ�ܸ�GM����һ��");
					return true;
				}

				Cmd::GmTool::t_NewMsg_GmTool send;
				strncpy(send.userName,pUser->name,MAX_NAMESIZE);
				send.accid = pUser->accid;
				send.userID = pUser->id;
				strncpy(send.userCountry,SceneManager::getInstance().getCountryNameByCountryID(pUser->charbase.country),MAX_NAMESIZE);
				send.type = rev->type;
				strncpy(send.content,rev->content,512);
				send.contact = rev->contact;
				strncpy(send.tele,rev->tele,64);
				strncpy(send.hisName,rev->hisName,64);
				strncpy(send.bugCountry,rev->bugCountry,8);
				strncpy(send.bugMap,rev->bugMap,16);
				strncpy(send.bugPos,rev->bugPos,8);
				strncpy(send.bugTime,rev->bugTime,64);
				send.behavior = rev->behavior;
				strncpy(send.hisName,rev->hisName,MAX_NAMESIZE);
				strncpy(send.bugCountry,rev->bugCountry,8);
				strncpy(send.bugMap,rev->bugMap,16);
				strncpy(send.bugPos,rev->bugPos,8);
				strncpy(send.bugTime,rev->bugTime,64);
				strncpy(send.progName,rev->progName,64);
				send.behavior = rev->behavior;

				ScenesService::getInstance().sendCmdToSuperServer(&send,sizeof(send));

				pUser->charbase.msgTime = SceneTimeTick::currentTime.sec()+1800;
				Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"��Ϣ���ύ��GM,�����ĵȴ��ʼ��ظ�");
				return true;
			}
		}
		break;
	default:
		break;
	}

	Zebra::logger->error("SceneTask::usermsgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,cmdLen);
	return false;
}

bool SceneTask::loginmsgParse(const Cmd::t_NullCmd *pNullCmd,DWORD cmdLen)
{
	if (pNullCmd->para==Cmd::Scene::PARA_LOGIN_UNREG)
	{
		//TODO ע���û�
		Cmd::Scene::t_Unreg_LoginScene *rev=(Cmd::Scene::t_Unreg_LoginScene *)pNullCmd;
		//Scene *scene=SceneManager::getInstance().getSceneByTempID(rev->dwSceneTempID);
		//if (scene)
		{
			SceneUser *pUser=SceneUserManager::getMe().getUserByIDOut(rev->dwUserID);
			if (pUser)
			{
				Cmd::Record::t_RemoveUser_SceneRecord rec_ret;
				rec_ret.accid = pUser->accid;
				rec_ret.id = pUser->id;
				recordClient->sendCmd(&rec_ret,sizeof(rec_ret));

				pUser->unreg();
				Zebra::logger->debug("�û�%ldע��ʱ��û�н��볡��",rev->dwUserID);
			}
			else
			{
				pUser=SceneUserManager::getMe().getUserByID(rev->dwUserID);
				if (pUser)
				{
					//pUser->skillStatusM.clearRecoveryElement(230);//�������
					OnQuit event(1);
					EventTable::instance().execute(*pUser,event);
					execute_script_event(pUser,"quit");

					pUser->save(Cmd::Record::LOGOUT_WRITEBACK);
					//pUser->killAllPets();
					pUser->unreg();
					Zebra::logger->debug("����ע��(%s,%u)ָ��:t_Unreg_LoginScene",pUser->name,pUser->id);
				}
				else
					Zebra::logger->error("ע��ʱ,δ�ҵ���ʱ���Ϊ%ld���û�",rev->dwUserID);
			}
		}
		return true;
	}
	// */
	return false;
}
/**
* \brief �����û�����,��������Ӧ�ķ��ɴ�����
*
* \param pNullCmd �û�����
* \param cmdLen �����
*
* \return ����õ�������TRUE,����FALSE
*
*/
bool SceneTask::msgParse(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
	return MessageQueue::msgParse(cmd,cmdLen);
}

bool SceneTask::cmdMsgParse(const Cmd::t_NullCmd *cmd,const DWORD cmdLen)
{
	NFilterModuleArray::const_iterator pIterator;

	using namespace Cmd;

	//command
	for(pIterator=g_nFMA.begin(); pIterator != g_nFMA.end();pIterator++)
	{
		if (pIterator->filter_command((PBYTE)cmd,cmdLen)) 
		{
			//fprintf(stderr,"eturn here");
			//Zebra::logger->error("return here");
			return true;
		}
	}

	if (cmd->cmd==Cmd::Scene::CMD_FORWARD && cmd->para==Cmd::Scene::PARA_FORWARD_SCENE)
	{

		Cmd::Scene::t_Scene_ForwardScene *rev=(Cmd::Scene::t_Scene_ForwardScene *)cmd;
		//Scene *scene=SceneManager::getInstance().getSceneByTempID(rev->dwSceneTempID);
		//if (scene)
		{
			//      SceneUserManager::getMe().lock();
			SceneUser *pUser=SceneUserManager::getMe().getUserByID(rev->dwUserID);
			if (pUser)
			{
				set_me(pUser);
				if (usermsgParse(pUser,(Cmd::t_NullCmd *)rev->data,rev->size))
				{
					//          SceneUserManager::getMe().unlock();
					return true;
				}
				else 
				{
					return true;
					//          SceneUserManager::getMe().unlock();
				}
			}
			else
			{
				Zebra::logger->error("�����û�ָ��(%d,%d)ʱ,δ�ҵ���ʱ���Ϊ%ld���û�",cmd->cmd,cmd->para,rev->dwUserID);
				//        SceneUserManager::getMe().unlock();
				return true;
			}
		}
	}
	else if (cmd->cmd==Cmd::Scene::CMD_LOGIN)
	{
		return loginmsgParse(cmd,cmdLen);
	}
	else if (cmd->cmd==Cmd::Scene::CMD_FORWARD && cmd->para==Cmd::Scene::PARA_BILL_FORWARD_SCENE)
	{
		Cmd::Scene::t_Bill_ForwardScene *rev=(Cmd::Scene::t_Bill_ForwardScene*)cmd;
		SceneUser *pUser=SceneUserManager::getMe().getUserByID(rev->dwUserID);
		if (pUser)
		{
			return usermsgParseBill(pUser,(Cmd::t_NullCmd *)rev->data,rev->size);
		}
	}

	Zebra::logger->error("SceneTask::cmdMsgParse(%u,%u,%u)",cmd->cmd,cmd->para,cmdLen);
	return false;
}

