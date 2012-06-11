/**
 * \brief ���嵵�����������ӿͻ���
 *
 * �����뵵������������,��ȡ����
 * 
 */

#include <zebra/ScenesServer.h>

///RecordClient��Ψһʵ��
RecordClient *recordClient = NULL;
/**
 * \brief ��ѹ��ɫ��Ҫ���������
 *
 * \param pUser ��ɫָ��
 * \param data ���룺ѹ�������� / �������ѹ������� 
 * \param dataSize ���� ���ݴ�С
 * \return ��ѹ������ݴ�С
 */
bool uncompressSaveData(SceneUser *pUser,const BYTE *data,const DWORD dataSize,BYTE * petData); 

/**
 * \brief ����������������������
 *
 * \return �����Ƿ�ɹ�
 */
bool RecordClient::connectToRecordServer()
{
  if (!connect())
  {
    Zebra::logger->error("���ӵ���������ʧ��");
    return false;
  }

  Cmd::Record::t_LoginRecord tCmd;

  tCmd.wdServerID   = ScenesService::getInstance().getServerID();
  tCmd.wdServerType = ScenesService::getInstance().getServerType();

  return sendCmd(&tCmd,sizeof(tCmd));
}

/**
 * \brief ����zTCPBufferClient��run����
 *
 *
 */
void RecordClient::run()
{
  zTCPBufferClient::run();

  //�뵵�������������ӶϿ�,�رշ�����
  ScenesService::getInstance().Terminate();
  while (sessionClient)
  {
    zThread::msleep(10);
  }
}

/**
 * \brief �������Ե�����������ָ��
 *
 * \param pNullCmd ��������ָ��
 * \param nCmdLen ��������ָ���
 * \return ����ָ���Ƿ�ɹ�
 */
bool RecordClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  return MessageQueue::msgParse(pNullCmd,nCmdLen);
}
bool RecordClient::cmdMsgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
	using namespace Cmd::Record;

	if (pNullCmd->cmd==CMD_SCENE)
	{
		switch(pNullCmd->para)
		{
		case PARA_SCENE_USERINFO:
			{
				t_UserInfo_SceneRecord *rev=(t_UserInfo_SceneRecord *)pNullCmd;
				Scene *scene=SceneManager::getInstance().getSceneByTempID(rev->dwMapTempID);
				SceneUser *pUser=SceneUserManager::getMe().getUserByIDOut(rev->id);
				if (scene)
				{
					//Zebra::logger->debug("�õ�����%s",scene->name);
					if (pUser)
					{
						bool dataOk=false;
						//Zebra::logger->trade("��ȡ%ld�����ɹ�",rev->id);
						bcopy(&rev->charbase,&pUser->charbase,sizeof(CharBase),sizeof(pUser->charbase));

						if (pUser->charbase.reliveWeakTime>0)
						{
							if (pUser->charbase.reliveWeakTime <=450)
								pUser->charbase.reliveWeakTime = SceneTimeTick::currentTime.sec()%10000+pUser->charbase.reliveWeakTime;
							else
								pUser->charbase.reliveWeakTime=0;
						}

						//Zebra::logger->debug("%u,%u",rev->charbase.x,rev->charbase.y);
						// load Object
						//���ó�������Ĵ�С
						PetPack *pack = (PetPack *)pUser->packs.getPackage(Cmd::OBJECTCELLTYPE_PET,0);
						if (pack)
						{
							pack->setAvailable(pUser->charbase.petPack);
							//Zebra::logger->debug("%s ���������С %u",pUser->name,pUser->charbase.petPack);
						}
						if (rev->dataSize != (DWORD)PARA_SCENE_USER_READ_ERROR)
						{
							BYTE petData[sizeof(int)+sizeof(Cmd::t_PetData)*10];//���Դ��10������
							bzero(petData,sizeof(petData));

							if (rev->dataSize != 0)
							{
								dataOk =uncompressSaveData(pUser,(BYTE*)rev->data,rev->dataSize,petData);
							}
							else
							{
								dataOk=true;
								//��ʱʱ��������
								if (!pUser->isNewCharBase())
								{
									/*
									pUser->charbase.points=pUser->charbase.points
									+pUser->charbase.wdProperty[0]
									+pUser->charbase.wdProperty[1]
									+pUser->charbase.wdProperty[2]
									+pUser->charbase.wdProperty[3]
									+pUser->charbase.wdProperty[4]
									;
									pUser->charbase.wdProperty[0]=0;
									pUser->charbase.wdProperty[1]=0;
									pUser->charbase.wdProperty[2]=0;
									pUser->charbase.wdProperty[3]=0;
									pUser->charbase.wdProperty[4]=0;
									pUser->charbase.skillpoint=pUser->charbase.level;
									pUser->packs.addMoney(pUser->charbase.level * 200000,"��Ǯ");
									//Ĭ��ϵͳ����
									using namespace Cmd;
									memset(pUser->sysSetting,0xff,sizeof(pUser->sysSetting));
									pUser->sysSetting[0] = 0;//pkģʽ
									//clear_state(sysSetting,Cmd::USER_SETTING_AUTOFINDPATH);//�Զ�Ѱ·
									clear_state(pUser->sysSetting,Cmd::USER_SETTING_SHOW_PLAYERNAME);//��ʾ�������
									clear_state(pUser->sysSetting,Cmd::USER_SETTING_AUTO_KILL_SUMMON);//�Զ����
									//֪ͨsession
									Cmd::Session::t_sysSetting_SceneSession send;
									strncpy((char *)send.name,pUser->name,MAX_NAMESIZE-1);
									bcopy(pUser->sysSetting,&send.sysSetting,sizeof(send.sysSetting));
									send.face = pUser->charbase.face;
									sessionClient->sendCmd(&send,sizeof(send));

									pUser->chatColor[0] = 0xffffffff;
									pUser->chatColor[1] = 0xffffd100;//COLOR_ARGB(255,255,209,0);
									pUser->chatColor[2] = 0xff4eaa00;//COLOR_ARGB(255,78,170,0);
									pUser->chatColor[3] = 0xffff4818;//COLOR_ARGB(255,255,72,24);
									pUser->chatColor[4] = 0xff006300;//COLOR_ARGB(255,0,99,0);
									pUser->chatColor[5] = 0xff98f417;//COLOR_ARGB(255,152,244,23);
									pUser->chatColor[6] = 0xffff627c;//COLOR_ARGB(255,255,98,124);
									pUser->chatColor[7] = 0xff007fff;//COLOR_ARGB(255,0,127,255);
									//֪ͨ�ͻ���
									Cmd::stSystemSettingsUserCmd sendClient;
									bcopy(pUser->sysSetting,&sendClient.data.bySettings,sizeof(sendClient.data.bySettings));
									bcopy(pUser->chatColor,&sendClient.data.dwChatColor,sizeof(sendClient.data.dwChatColor));
									pUser->sendCmdToMe(&sendClient,sizeof(sendClient));
									// */

								}
							}
							// */
							if (pUser&&(pUser->charbase.level<scene->getLevel()))
							{       
								char map[MAX_NAMESIZE+1];
								bzero(map,sizeof(map));
								bcopy(scene->getName(),map,6,sizeof(map));
								bcopy("���ִ�",&map[6],6,sizeof(map) - 6);
								scene=SceneManager::getInstance().getSceneByName(map);
								if (!scene)
								{
									Zebra::logger->fatal("��ȡ%u����ʧ��,��ȼ����ͼ������,��δ�ҵ����ִ� ��ͼ",rev->id);
									return true;
								}
							}   
							if (dataOk)
							{
								//sky �����ȡ���ϵ�ͼ������
								if(rev->RegMapType == 1)
								{
									char mapname[MAX_NAMESIZE+1];
									DWORD oldx;
									DWORD oldy;
									sscanf(pUser->charbase.OldMap, "%[^-]-%d-%d", mapname, &oldx, &oldy);
									pUser->charbase.x = oldx;
									pUser->charbase.y = oldy;

									Zebra::logger->fatal("�����ϵ�ͼ�� name:%s x:%d y:%d RegMapType:%d",scene->name, pUser->charbase.x, pUser->charbase.y, rev->RegMapType);
								}

								Zebra::logger->fatal("�����µ�ͼ�� name:%s x:%d y:%d RegMapType:%d",scene->name, pUser->charbase.x, pUser->charbase.y, rev->RegMapType);

								pUser->initCharBase(scene);
								//�û������ͼ,���������û���Ϣ
								if (pUser->intoScene(scene,true,pUser->getPos()))
								{
									Cmd::Session::t_regUserSuccess_SceneSession ret; // ֪ͨ�Ự�������û�������Ϸ�ɹ�
									ret.dwUserID=rev->id;
									ret.dwUseJob = pUser->charbase.useJob;
									ret.dwExploit = pUser->charbase.exploit;
									ret.dwCountryID = pUser->charbase.country;
									ret.dwSchoolID = pUser->charbase.schoolid;
									ret.dwSeptID = pUser->charbase.septid;
									ret.dwUnionID = pUser->charbase.unionid;
									ret.qwExp = pUser->charbase.exp;
									sessionClient->sendCmd(&ret,sizeof(ret));
									//�������ˢ��������
									if ((int)pUser->charbase.hp <= 0)
									{
										pUser->relive(Cmd::ReliveHome,0,100);
									}
									pUser->loadPetState(petData,sizeof(petData));

									//������ʼ�
									if (Zebra::global["mail_service"]=="on")
									{
										Cmd::Session::t_checkNewMail_SceneSession cn;
										cn.userID = pUser->tempid;
										sessionClient->sendCmd(&cn,sizeof(cn));
									}

									//sky ������Ұ�����ʹ�õ�ҳ��
									Cmd::stPackBuyTanbNumUserCmd PackSend;
									PackSend.PackType = PACKE_TYPE;
									PackSend.TabNum = pUser->packs.main.TabNum;
									pUser->sendCmdToMe(&PackSend,sizeof(PackSend));

									//sky ����Ʒ��ȴ������Ϣ���͸��û�
									pUser->sendCmdToMe(&CoolTimeSendData[0], CoolTimeSendData.size());

									Cmd::stTimeSyncUserCmd send;
									send.serverTime = time(NULL);
									pUser->sendCmdToMe(&send,sizeof(send));
									//�������б�
									Cmd::Session::t_loadCartoon_SceneSession lc;
									lc.userID = pUser->id;
									sessionClient->sendCmd(&lc,sizeof(lc));

									//֪ͨmini������
									Cmd::Mini::t_Scene_SetScene ss;
									ss.userID = pUser->id;
									ss.serverID = ScenesService::getInstance().getServerID();
									miniClient->sendCmd(&ss,sizeof(ss));

									if (pUser->charbase.reliveWeakTime > 0)
									{
										if (pUser->charbase.reliveWeakTime > SceneTimeTick::currentTime.sec()%10000)
										{// ����Ԥ����,���¼�����������
											pUser->showCurrentEffect(Cmd::USTATE_RELIVEWEAK,true); // ���¿ͻ���״̬
											pUser->sendtoSelectedReliveWeakState();
										}
									}

									if (pUser->charbase.trainTime)
									{
										pUser->showCurrentEffect(Cmd::USTATE_DAOJISHI,true); // ���¿ͻ���״̬
										pUser->sendtoSelectedTrainState();
									}
									return true;
								}
							}
							else
							{
								Zebra::logger->fatal("����%u����ʧ��,�����Ǳ�����",rev->id);
							}
						}
						else
						{
							Zebra::logger->fatal("��ȡ%u����ʧ��,��Ч�û�����",rev->id);
						}
					}
					else
						Zebra::logger->fatal("��ȡ%u����ʧ��,δ�ҵ��û�����",rev->id);
				}
				else
					Zebra::logger->fatal("��ȡ%u����ʧ��,δ�ҵ���ͼ,maptempid=%u",rev->id,rev->dwMapTempID);
				/*
				// ֪ͨSession ע��ʧ��
				Cmd::Session::t_unregUser_SceneSession ret;
				ret.dwSceneTempID=rev->dwMapTempID;
				ret.dwUserID=rev->id;
				ret.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
				sessionClient->sendCmd(&ret,sizeof(ret));
				if (pUser)
				{
				// ֪ͨRecord ��ȡʧ��
				Cmd::Record::t_RemoveUser_SceneRecord rec_ret;
				rec_ret.accid = pUser->accid;
				rec_ret.id = pUser->id;
				recordClient->sendCmd(&rec_ret,sizeof(rec_ret));

				// ֪ͨGateway ע��ʧ��
				Cmd::Scene::t_Unreg_LoginScene ret;
				ret.dwUserID=pUser->id;
				ret.dwSceneTempID=rev->dwMapTempID;
				ret.retcode=Cmd::Scene::UNREGUSER_RET_ERROR;
				pUser->gatetask->sendCmd(&ret,sizeof(ret));
				SceneUserManager::getMe().removeUser(pUser);
				Zebra::logger->debug("����ʧ��ע��(%s,%u)",pUser->name,pUser->id);
				pUser->destroy();
				SAFE_DELETE(pUser);
				}
				// */
				return true;
			}
			break;
      case PARA_SCENE_USER_EXIST:
        {
          t_userExist_SceneRecord * rev = (t_userExist_SceneRecord *)pNullCmd;

          SceneUser * pUser = SceneUserManager::getMe().getUserByTempID(rev->fromID);
          if (!pUser)
          {
            Zebra::logger->warn("[�ʼ�]���Ŀ����� %s ��,�������Ѳ�����",rev->sm.toName);
            return false;
          }
          pUser->isSendingMail = false;
          if ((pUser->privatestore.step() != PrivateStore::NONE) || pUser->tradeorder.hasBegin())
          {
            Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"���׹����в��ܷ����ʼ�");
            return true;
          }

          if (0==rev->toID)
          {
            Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"�����ڸý�ɫ,��������ȷ������");
            return true;
          }

          if (!pUser->packs.checkMoney(rev->sm.sendMoney?rev->sm.sendMoney+Cmd::mail_postage:Cmd::mail_postage))                          
          {
            Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"�����������");

            return true;
          }
          if (!pUser->packs.checkGold(rev->sm.sendGold)) 
          {
            Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"��Ľ�Ҳ���");

            return true;
          }

          Cmd::Session::t_sendMail_SceneSession sm;
          sm.mail.state = Cmd::Session::MAIL_STATE_NEW;
          strncpy(sm.mail.fromName,pUser->name,MAX_NAMESIZE);
          strncpy(sm.mail.toName,rev->sm.toName,MAX_NAMESIZE);
          strncpy(sm.mail.title,rev->sm.title,MAX_NAMESIZE);
          sm.mail.type = Cmd::Session::MAIL_TYPE_MAIL;
          zRTime ct;
          sm.mail.createTime = ct.sec();
          sm.mail.delTime = sm.mail.createTime + 60*60*24*7;
          strncpy(sm.mail.text,rev->sm.text,256);
          sm.mail.sendMoney = rev->sm.sendMoney;
          sm.mail.recvMoney = rev->sm.recvMoney;
          sm.mail.sendGold = 0;//�����ͽ��
          sm.mail.recvGold = 0;//�����ͽ��
          //sm.itemID = rev->sm.itemID;
          sm.mail.itemGot = 0;
          sm.mail.fromID = pUser->id;
          sm.mail.toID = rev->toID;
          if (rev->sm.sendMoney||rev->sm.sendGold)
            sm.mail.accessory = 1;
          //bcopy(&rev->mail,&sm.mail,sizeof(mailInfo));

          if (rev->sm.itemID && rev->sm.itemID != 0xffffffff)//INVALID_THISID
          {
            zObject * srcobj=pUser->packs.uom.getObjectByThisID(rev->sm.itemID);
            if (!srcobj)
            {
              Zebra::logger->info("%s �����ʼ�ʱδ�ҵ�����Ʒ��Ʒ id=%u",pUser->name,rev->sm.itemID);
              return false;
            }
            if (!srcobj->canMail())
            {
              Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"�㲻���ʼ������Ʒ");
              return true;
            }

            pUser->packs.removeObject(srcobj,true,false); //notify but not delete
            //srcobj->getSaveData((SaveObject *)&sm.item);

			bcopy(&srcobj->data,&sm.item.object,sizeof(t_Object),sizeof(sm.item.object));
            sm.mail.itemID = srcobj->data.qwThisID;
            
            //srcobj->getSaveData((SaveObject *)&sm.item);

            zObject::logger(srcobj->createid,srcobj->data.qwThisID,srcobj->base->name,srcobj->data.dwNum,srcobj->data.dwNum,0,pUser->id,pUser->name,0,rev->sm.toName,"�ʼĵ�����",srcobj->base,srcobj->data.kind,srcobj->data.upgrade);
            //pUser->packs.removeObject(srcobj);//notify and delete

            zObject::destroy(srcobj);
            sm.mail.accessory = 1;
          }
          pUser->packs.removeMoney(rev->sm.sendMoney+Cmd::mail_postage,"�����ʼ��ʼ�");//��Ǯ
          //pUser->packs.removeGold(rev->sm.sendGold,"�����ʼ�");//��Ǯ

          if (sessionClient->sendCmd(&sm,sizeof(Cmd::Session::t_sendMail_SceneSession)))
          {
            pUser->save(OPERATION_WRITEBACK);//���̴浵
            Zebra::logger->info("�����ʼ� %s->%s",pUser->name,rev->sm.toName);
            return true;
          }
          else
          {
            Zebra::logger->error("�ʼ�����ʧ�� %s->%s money=%u gold=%u",pUser->name,rev->sm.toName,rev->sm.sendMoney,rev->sm.sendGold);
            return false;
          }
          return true;
        }
        break;
      case PARA_SCENE_USER_WRITE_OK:
        {
          t_WriteUser_SceneRecord_Ok * rev = (t_WriteUser_SceneRecord_Ok *)pNullCmd;
          Cmd::Scene::t_Unreg_LoginScene_Ok ok; 
          ok.type=rev->type;
          ok.id=rev->id;
          ok.accid=rev->accid;
          SceneTaskManager::getInstance().broadcastCmd(&ok,sizeof(ok));
          return true;
        }
        break;
      default:
        break;
    }
  }

  Zebra::logger->error("RecordClient::cmdMsgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

