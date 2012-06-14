/**
 * \brief ʵ�ֶԹ�ϵ����Ĵ���
 *
 */

#include <zebra/ScenesServer.h>

#define MARRY_REQUEST_LEVEL 40
#define MARRY_REQUEST_M_ITEM_ID 2226
#define MARRY_REQUEST_W_ITEM_ID 2227
#define MARRY_REQUEST_MONEY 3000
#define MARRY_REQUEST_MONEY1 5000
#define MARRY_REQUEST_MONEY2 10000
#define MARRY_PRESENT_MALE 875
#define MARRY_PRESENT_FEME 875


/**
  * \brief һ���Ƚ���
  *
  *  �����Ƚ�����zObjectʵ���Ƿ�����ȵ�
  *  (��ʱδʹ��)
  *
  */
class ItemObjectCompare:public UserObjectCompare 
{
  public:
    DWORD  dwObjectID;

    bool isIt(zObject *object)
    {
      if (object->data.dwObjectID == dwObjectID) return true;
      return false;
    }
};

/**
  * \brief ���л�����ƥ��,���ҷ���������һ��
  * 
  *
  */
struct FindConsort : public TeamMemExec
{
  SceneUser *feme;
  SceneUser *male;
  SceneUser *me;
  FindConsort(SceneUser *pUser)
  {
    feme = NULL;
    male = NULL;
    me = pUser;
  }
  virtual bool exec(TeamMember &member)
  {
       SceneUser *pUser = NULL;
     if (me->id == member.id)
     {
       pUser = me;
     }
     else
     {
       pUser = SceneUserManager::getMe().getUserByID(member.id);
     }
    if (pUser)
    {
      switch(pUser->charbase.type)
      {
        case PROFESSION_1:    //����
        case PROFESSION_3:    //����
        case PROFESSION_5:    //��ʦ
        case PROFESSION_7:    //��ʦ
          male = pUser;
          break;
        case PROFESSION_2:    //��Ů
        case PROFESSION_4:    //����
        case PROFESSION_6:    //��Ů
        case PROFESSION_8:    //��Ů
          feme = pUser;
          break;
        case PROFESSION_NONE:  //��ҵ
        default:
          Zebra::logger->error("�����ְҵ����");
          break;
      }
    }
    return true;
  }
};

void SceneUser::addObjectToUserPacket(int objectid,SceneUser *pUser)
{
  zObjectB *base = objectbm.get(objectid);
  if (base)
  {
    zObject *o=zObject::create(base,1);
    if (!pUser->packs.addObject(o,true,AUTO_PACK))
    {
      pUser->scene->addObject(dupIndex,o,pUser->getPos());
    }
    else
    {
      Cmd::stAddObjectPropertyUserCmd item;
      item.byActionType=Cmd::EQUIPACTION_OBTAIN;
      bcopy(&o->data,&item.object,sizeof(item.object),sizeof(item.object));
      pUser->sendCmdToMe(&item,sizeof(item));
    }
  }
}

/**
  * \brief �����ϵ����
  *
  * ����Ĺ�ϵ��������:
  *
  * Cmd::MARRY_STATUS_CHECK_PARA
  *
  * \param rev: ��ϵ����
  * \param cmdLen: �����
  *
  * \return ���������TRUE,����ΪFALSE
  *
  *
  */
bool SceneUser::doRelationCmd(const Cmd::stRelationUserCmd *rev,DWORD cmdLen)
{
	switch(rev->byParam)
	{
	case Cmd::MARRY_STATUS_CHECK_PARA:
		{
			Cmd::stMarryStatusCheckCmd *ptCmd=(Cmd::stMarryStatusCheckCmd *)rev;

			if (ptCmd->byStep == Cmd::MARRY_REFUSE)
			{
				SceneUser *pUser = NULL;
				pUser = SceneUserManager::getMe().getUserByID(this->friendID);
				if (pUser)
				{
					this->answerMarry=false;
					this->friendID=0;
					Channel::sendSys(pUser,Cmd::INFO_TYPE_MSG,"�ǳ��ź�,�Է��ܾ����㣡");
				}
				return true;
			}

			if (ptCmd->byStep == Cmd::MARRY_AGREE)
			{
				SceneUser *pUser = NULL;
				this->answerMarry = true;
				pUser = SceneUserManager::getMe().getUserByID(this->friendID);
				if (pUser)
				{
					ptCmd->byStep = Cmd::MARRY_ITEM_CHECK;
					pUser->doRelationCmd(rev,cmdLen);
					return true;
				}
				return true;
			}

			SceneUser *pUser = NULL;

			TeamManager * team = SceneManager::getInstance().GetMapTeam(TeamThisID);

			if (team && team->getSize()==2)
			{
				pUser = SceneUserManager::getMe().getUserByTempID(team->getLeader());
				if (!pUser) pUser= this;

				FindConsort callback(this);
				team->execEveryOne(callback);

				if (callback.feme&&callback.male)
				{
					if (pUser->scene->checkTwoPosIInNine(callback.feme->getPosI(),callback.male->getPosI()))
					{
						if (callback.feme->charbase.country == callback.male->charbase.country)
						{
							if (callback.feme->charbase.level >=MARRY_REQUEST_LEVEL &&
								callback.male->charbase.level >=MARRY_REQUEST_LEVEL)
							{
								if (callback.feme->charbase.consort ==0 &&
									callback.male->charbase.consort ==0)
								{
									if (ptCmd->byStep == Cmd::MARRY_AHEAD_CHECK)
									{
										callback.feme->friendID=0;
										callback.feme->answerMarry=false;
										callback.male->friendID=0;
										callback.male->answerMarry=false;
										sendCmdToMe(rev,cmdLen);
										return true;
									}

									SceneUser *pOther = NULL;
									if (this == callback.feme)
									{
										pOther = callback.male;
									}
									else
									{
										pOther = callback.feme;
									}

									if (ptCmd->byStep == Cmd::MARRY_ITEM_CHECK && !pOther->answerMarry)
									{
										pOther->friendID = this->id;
										ptCmd->byStep = Cmd::MARRY_ANSWER;
										pOther->sendCmdToMe(rev,cmdLen);
										return true;
									}
									else
									{
										pOther->answerMarry = false;
										pOther->friendID = 0;
									}

									ItemObjectCompare found;
									found.dwObjectID = MARRY_REQUEST_M_ITEM_ID;
									zObject *mitemobj = callback.male->packs.uom.getObject(found);
									found.dwObjectID = MARRY_REQUEST_W_ITEM_ID;
									zObject *witemobj = callback.feme->packs.uom.getObject(found);

									if (mitemobj && witemobj)
									{
										if (ptCmd->byStep == Cmd::MARRY_ITEM_CHECK)
										{
											sendCmdToMe(rev,cmdLen);
											return true;
										}
										if (ptCmd->byStep != Cmd::MARRY_PAY_MONEY  &&
											ptCmd->byStep != Cmd::MARRY_PAY_MONEY1 &&
											ptCmd->byStep != Cmd::MARRY_PAY_MONEY2) return true;

										switch(ptCmd->byStep)
										{
										case Cmd::MARRY_PAY_MONEY:
											{
												if (packs.checkMoney(MARRY_REQUEST_MONEY) && packs.removeMoney(MARRY_REQUEST_MONEY,"���0"))
												{
													addObjectToUserPacket(MARRY_PRESENT_MALE,callback.male);
													addObjectToUserPacket(MARRY_PRESENT_FEME,callback.feme);
												}
												else
												{
													ptCmd->byStep = Cmd::MARRY_NO_MONEY;
													sendCmdToMe(ptCmd,cmdLen);
													Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��û���㹻��Ǯ�����ɽ�����");
													return true;
												}
											}
											break;
										case Cmd::MARRY_PAY_MONEY1:
											{
												if (packs.checkMoney(MARRY_REQUEST_MONEY1) && packs.removeMoney(MARRY_REQUEST_MONEY1,"���1"))
												{
													addObjectToUserPacket(MARRY_PRESENT_MALE,callback.male);
													addObjectToUserPacket(1723,callback.male);
													addObjectToUserPacket(1723,callback.male);
													addObjectToUserPacket(MARRY_PRESENT_FEME,callback.feme);
													addObjectToUserPacket(1723,callback.feme);
													addObjectToUserPacket(1723,callback.feme);
												}
												else
												{
													ptCmd->byStep = Cmd::MARRY_NO_MONEY;
													sendCmdToMe(ptCmd,cmdLen);
													Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��û���㹻��Ǯ�����ɽ�����");
													return true;
												}
											}
											break;
										case Cmd::MARRY_PAY_MONEY2:
											{
												if (packs.checkMoney(MARRY_REQUEST_MONEY2) && packs.removeMoney(MARRY_REQUEST_MONEY2,"���2"))
												{
													addObjectToUserPacket(MARRY_PRESENT_MALE,callback.male);
													addObjectToUserPacket(1723,callback.male);
													addObjectToUserPacket(1723,callback.male);
													addObjectToUserPacket(1723,callback.male);
													addObjectToUserPacket(1723,callback.male);
													addObjectToUserPacket(MARRY_PRESENT_FEME,callback.feme);
													addObjectToUserPacket(1723,callback.feme);
													addObjectToUserPacket(1723,callback.feme);
													addObjectToUserPacket(1723,callback.feme);
													addObjectToUserPacket(1723,callback.feme);

													callback.male->summonPet(60001,Cmd::PET_TYPE_TOTEM,180);
													callback.male->summonPet(60003,Cmd::PET_TYPE_TOTEM,180);
													callback.male->summonPet(60003,Cmd::PET_TYPE_TOTEM,180);
													callback.male->summonPet(60003,Cmd::PET_TYPE_TOTEM,180);

													callback.feme->summonPet(60002,Cmd::PET_TYPE_TOTEM,180);
													callback.feme->summonPet(60004,Cmd::PET_TYPE_TOTEM,180);
													callback.feme->summonPet(60004,Cmd::PET_TYPE_TOTEM,180);
													callback.feme->summonPet(60004,Cmd::PET_TYPE_TOTEM,180);
												}
												else
												{
													ptCmd->byStep = Cmd::MARRY_NO_MONEY;
													sendCmdToMe(ptCmd,cmdLen);
													Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��û���㹻��Ǯ�����ɽ�����");
													return true;
												}
											}
											break;
										default:
											break;
										}
										callback.male->packs.removeObject(mitemobj);
										callback.feme->packs.removeObject(witemobj);


										callback.feme->charbase.consort = callback.male->charbase.id;
										callback.male->charbase.consort = callback.feme->charbase.id;

										Cmd::stRelationStatusCmd send;
										send.byState = Cmd::RELATION_ANSWER_YES;
										send.type = Cmd::RELATION_TYPE_LOVE;
										if (callback.feme == this)
										{
											send.userid = callback.male->charbase.id;
										}
										else
										{
											send.userid = callback.feme->charbase.id;
										}
										this->forwardSession(&send,sizeof(send));

										sendCmdToMe(ptCmd,cmdLen);
										//Channel::sendSys(callback.feme,Cmd::INFO_TYPE_GAME,"��ϲ���ǽ��ɹ�");
										//Channel::sendSys(callback.male,Cmd::INFO_TYPE_GAME,"��ϲ���ǽ��ɹ�");


										char buf[MAX_CHATINFO];
										sprintf(buf,"HydTest��ף%s��%sϲ������,��ͷ���ϣ�",callback.feme->name,callback.male->name);
										zRTime ctv;
										Cmd::stChannelChatUserCmd sendMsg;
										sendMsg.dwType=Cmd::CHAT_TYPE_COUNTRY_MARRY;
										switch(ptCmd->byStep)
										{
										case Cmd::MARRY_PAY_MONEY:
											sendMsg.dwSysInfoType = Cmd::INFO_TYPE_SYS;//ϵͳ��Ϣ������
											break;
										case Cmd::MARRY_PAY_MONEY1:
											sendMsg.dwSysInfoType = Cmd::INFO_TYPE_SKYROCKET1;//ϵͳ��Ϣ������
											break;
										case Cmd::MARRY_PAY_MONEY2:
											sendMsg.dwSysInfoType = Cmd::INFO_TYPE_SKYROCKET2;//ϵͳ��Ϣ������
											break;
										default:
											sendMsg.dwSysInfoType = Cmd::INFO_TYPE_SYS;//ϵͳ��Ϣ������
											break;
										}

										//sendMsg.dwSysInfoType = Cmd::INFO_TYPE_EXP;//ϵͳ��Ϣ������
										sendMsg.dwChatTime = ctv.sec();
										bzero(sendMsg.pstrName,sizeof(sendMsg.pstrName));
										bzero(sendMsg.pstrChat,sizeof(sendMsg.pstrChat));
										strncpy((char *)sendMsg.pstrChat,buf,MAX_CHATINFO-1);
										strncpy((char *)sendMsg.pstrName,callback.feme->name,MAX_NAMESIZE);
										Channel::sendCountry(callback.feme,&sendMsg,sizeof(sendMsg));
										return true;
									}
									else
									{
										if (ptCmd->byStep == Cmd::MARRY_ITEM_CHECK)
										{
											ptCmd->byStep = Cmd::MARRY_NO_ITEM;
											sendCmdToMe(ptCmd,cmdLen);
											if (!mitemobj)
											{
												Channel::sendSys(callback.male,Cmd::INFO_TYPE_FAIL,"��û��׼����䣿��ȥ�Һ����ñ������");
											}
											if (!witemobj)
											{
												Channel::sendSys(callback.feme,Cmd::INFO_TYPE_FAIL,"��ļ�ױ��û�кã���ȥ�Һ������ʰ�");
											}
											return true;
										}
										else
										{
											Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"û�ж����");
											return true;
										}
									}

								}
								else
								{
									Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����м������Ѿ�����ż,�ػ��ǲ�����ģ�");
								}
							}
							else
							{
								Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���ǵȼ�������,����������ɣ�");
							}
						}
						else
						{
							Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�������Ĺ�����һ��,���ⲻ������������Ǽ�,Ҫô�Ĺ����Ժ��������ң�");
						}
					}
					else
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���Ǳ��붼���Ҹ�ǰ��,�Ҳ��ܰ����ǰ�����������");
					}
				}
				else
				{
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��....ͬ���ǲ��ܽ��ģ�");
				}
			}
			else
			{
				if (team->getSize()<2)
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�ø��������������öӣ�");
				else
					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��Ķ�������ֻ����2����,�޹���Ա���������˳���");
			}
			ptCmd->byStep = Cmd::MARRY_AHEAD_CHECK_FAIL;
			sendCmdToMe(ptCmd,cmdLen);
		}
	break;
default:
	break;
}
return false;
}

