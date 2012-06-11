#include <zebra/ScenesServer.h>

#include <math.h>
#include <zebra/csBox.h>

/**     
 * \brief  ����stPropertyUserCmdָ��
 *
 *
 * ����stPropertyUserCmdָ��
 *      
 * \param rev: ���ܵ���ָ������
 * \param cmdLen: ���ܵ���ָ���
 * \return ����ָ��ɹ�������,���򷵻�false
 */       
bool SceneUser::doPropertyCmd(const Cmd::stPropertyUserCmd *rev,DWORD cmdLen)
{
  switch(rev->byParam)
  {
  case SWAPUSEROBJECT_PROPERTY_USERCMD_PARAMETER:
      {
        Cmd::stSwapObjectPropertyUserCmd *swap=(Cmd::stSwapObjectPropertyUserCmd *)rev;
        //�õ�ԭ��Ʒ
        zObject *srcobj=packs.uom.getObjectByThisID(swap->qwThisID);
        if (srcobj)
        {
          //���״���
          if (tradeorder.hasBegin() && tradeorder.in_trade(srcobj))
          {
            return true;
          }
        
          if (mask.is_use(srcobj)) {
            return Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"���Ƚ���������!");
          }
          stObjectLocation org=srcobj->data.pos;
          Zebra::logger->info("[�ƶ���Ʒ]%s�ƶ���Ʒ%s(%d,%d,%d,%d)->(%d,%u,%d,%d)",name,srcobj->data.strName,
              org.loc(),org.tab(),org.xpos(),org.ypos(),
              swap->dst.loc(),swap->dst.tab(),swap->dst.xpos(),swap->dst.ypos());
          if (org != swap->dst && packs.moveObject(this,srcobj,swap->dst))
          {
            //��ͻ��˷���ָ��ָ���µ������ط�����Ʒ
            if (swap->dst.loc()!=Cmd::OBJECTCELLTYPE_NONE)
              sendCmdToMe(swap,sizeof(Cmd::stSwapObjectPropertyUserCmd));

            if (packs.equip.needRecalc/* || recalcBySword(false)*/)
            {
              notifyEquipChange();
              setupCharBase();
              Cmd::stMainUserDataUserCmd  userinfo;
              full_t_MainUserData(userinfo.data);
              sendCmdToMe(&userinfo,sizeof(userinfo));

              sendMeToNine();
              this->reSendData = false;
#ifdef _DEBUG
              Zebra::logger->info("[�ƶ���Ʒ]���......");
#endif
            }
          }
          else
            Zebra::logger->info("[�ƶ���Ʒ]%s�ƶ���Ʒ%s(%d,%d,%d,%d)->(%d,%u,%d,%d)ʧ��",name,srcobj->data.strName,
                org.loc(),org.tab(),org.xpos(),org.ypos(),
                swap->dst.loc(),swap->dst.tab(),swap->dst.xpos(),swap->dst.ypos());
        }
        else
          Zebra::logger->warn("[�ƶ���Ʒ]%sδ�ҵ��ƶ���Ʒ %ld",name,swap->qwThisID);
        return true;
      }
      break;
    case SPLITUSEROBJECT_PROPERTY_USERCMD_PARAMETER:
      {
        Cmd::stSplitObjectPropertyUserCmd *split=(Cmd::stSplitObjectPropertyUserCmd *)rev;
        zObject *srcobj=packs.uom.getObjectByThisID(split->qwThisID);

        if (!srcobj) return true;


        //fix objects which have num can be equiped via this,it's ugly and should be checked at client too
        //NOTICE: this will lead to theses objects can not be equiped,client shouldn't send this cmd,use 
        //stSwapObjectPropertyUserCmd instead when needed.
        if (split->dst.loc() == Cmd::OBJECTCELLTYPE_EQUIP
            || split->dst.loc() == Cmd::OBJECTCELLTYPE_PET) return true;
        
        //���״���
        if (tradeorder.hasBegin() && tradeorder.in_trade(srcobj))
        {
          return true;
        }

        if (split->dwNum > srcobj->data.dwNum) {
          return true;
        }

        bool notify = false;

        //������Ʒ 
        Package *destpack=packs.getPackage(split->dst.loc(),split->dst.tab());
        zObject *destobj=NULL;
        if (destpack && 
          destpack->getObjectByZone(&destobj,split->dst.xpos(),split->dst.ypos()) )
        {          
          if (!destobj) { //split
            destobj = zObject::create(srcobj);
            if (destobj)   {
              destobj->data.dwNum = split->dwNum;
              destobj->data.pos = split->dst;

              if (packs.addObject(destobj,false)) {
                srcobj->data.dwNum -= split->dwNum;

                //֪ͨ�ͻ���
                Cmd::stSplitObjectPropertyUserCmd ret;
                bcopy(split,&ret,sizeof(ret),sizeof(ret));
                ret.qwNewThisID=destobj->data.qwThisID;
                sendCmdToMe(&ret,sizeof(ret));
                notify = true;
                zObject::logger(destobj->createid,destobj->data.qwThisID,destobj->data.strName,destobj->data.dwNum,destobj->data.dwNum,1,0,NULL,this->id,this->name,"�������",destobj->base,destobj->data.kind,destobj->data.upgrade);
              }
              else {
                zObject::destroy(destobj);
                return true;
              }
              
            }
          } else if (destobj->data.dwObjectID == srcobj->data.dwObjectID && 
            srcobj->base->maxnum>1 &&
            srcobj->data.dwObjectID==destobj->data.dwObjectID &&
            srcobj->data.upgrade==destobj->data.upgrade ) {

            srcobj->data.dwNum -= split->dwNum;
            destobj->data.dwNum += split->dwNum;

            if (destobj->data.dwNum>destobj->base->maxnum)
            {
              destobj->data.dwNum=destobj->base->maxnum;
            }

            notify = true;
            
            //֪ͨ�ͻ���
          
            Cmd::stSplitObjectPropertyUserCmd ret;
            bcopy(split,&ret,sizeof(ret),sizeof(ret));
            ret.qwNewThisID=destobj->data.qwThisID;
            sendCmdToMe(&ret,sizeof(ret));


          }
           else {
          }


          if (srcobj->data.dwNum==0)
          {
            zObject::logger(srcobj->createid,srcobj->data.qwThisID,srcobj->data.strName,srcobj->data.dwNum,destobj->data.dwNum,0,0,NULL,this->id,this->name,"���ɾ��",srcobj->base,srcobj->data.kind,srcobj->data.upgrade);
            packs.removeObject(srcobj,false,true);
          }
          else
          {
            zObject::logger(srcobj->createid,srcobj->data.qwThisID,srcobj->data.strName,srcobj->data.dwNum,destobj->data.dwNum,0,0,NULL,this->id,this->name,"�����",srcobj->base,srcobj->data.kind,srcobj->data.upgrade);
          }

        }

        return true;
      }
      break;
#if 0
    case UNIONUSEROBJECT_PROPERTY_USERCMD_PARAMETER:
      {
        Cmd::stUnionObjectPropertyUserCmd *uobj=(Cmd::stUnionObjectPropertyUserCmd *)rev;
        zObject *srcobj=packs.uom.getObjectByThisID(uobj->qwSrcThisID);
        zObject *destobj=packs.uom.getObjectByThisID(uobj->qwDstThisID);
        if (srcobj && destobj && srcobj->base->maxnum>1 &&
            srcobj->data.dwObjectID==destobj->data.dwObjectID &&
            srcobj->data.upgrade==destobj->data.upgrade /*&&
            srcobj->data.pos.dwLocation==Cmd::OBJECTCELLTYPE_MOUSE*/)
        {
          if (srcobj->data.dwNum+destobj->data.dwNum>srcobj->base->maxnum)
          {
            srcobj->data.dwNum=srcobj->data.dwNum+destobj->data.dwNum-srcobj->base->maxnum;
            destobj->data.dwNum=srcobj->base->maxnum;
          }
          else
          {
            destobj->data.dwNum+=srcobj->data.dwNum;
            srcobj->data.dwNum=0;
          }
          //���״���
          if (destobj->data.pos.dwLocation==Cmd::OBJECTCELLTYPE_TRADE && tradeorder.hasBegin())
          {
            tradeorder.rollback();
            SceneUser *an=tradeorder.getAnother();
            if (an)
            {
              an->lock();
              an->tradeorder.rollback();
              Cmd::stRefCountObjectPropertyUserCmd ret;
              ret.qwThisID=destobj->data.qwThisID;
              ret.dwNum=destobj->data.dwNum;
              an->sendCmdToMe(&ret,sizeof(ret));
              an->unlock();
            }
          }

          //֪ͨ�ͻ���
          Cmd::stRefCountObjectPropertyUserCmd ret;
          ret.qwThisID=destobj->data.qwThisID;
          ret.dwNum=destobj->data.dwNum;
          sendCmdToMe(&ret,sizeof(ret));
          if (srcobj->data.dwNum==0)
          {
            packs.rmObject(srcobj);
            Cmd::stRemoveObjectPropertyUserCmd rm;
            rm.qwThisID=srcobj->data.qwThisID;
            sendCmdToMe(&rm,sizeof(rm));
            SAFE_DELETE(srcobj);
          }
          else
          {
            ret.qwThisID=srcobj->data.qwThisID;
            ret.dwNum=srcobj->data.dwNum;
            sendCmdToMe(&ret,sizeof(ret));
          }
        }
        return true;
      }
      break;
#endif
    case USEUSEROBJECT_PROPERTY_USERCMD_PARAMETER:
      {
        Cmd::stUseObjectPropertyUserCmd *use=(Cmd::stUseObjectPropertyUserCmd *)rev;
        zObject *srcobj=packs.uom.getObjectByThisID(use->qwThisID);
        if (srcobj && srcobj->data.pos.loc() ==Cmd::OBJECTCELLTYPE_COMMON)
        {
          useObject(srcobj, use->preUseUserID);
        }
        return true;
      }
      break;
	case SET_COWBOX_KEY_PARAMETER:
	{
		//fprintf(stderr,"�û�ʹ��Կ��\n");
		Cmd::stSetCowBoxKeyCmd *use = (Cmd::stSetCowBoxKeyCmd*)rev;
		zObject *srcobj=packs.uom.getObjectByThisID(use->qwThisID);


			//if(use->qwThisID)

			Zebra::logger->error("key id = %u\n",use->qwThisID);
			fprintf(stderr,"key id = %u\n",use->qwThisID);


			if(srcobj == NULL)
			{
				return false;
				Zebra::logger->error("%s�û���ʹ��Կ�׵�ʱ�������û��Կ��\n", this->name);
			}

			zObject* ob;
			if(use->Key_id == 964)
			{
				//Ѱ�Ұ������Ƿ��н���
				
				if(!packs.main.getObjectByID(&ob,962))
				{
					ob = packs.uom.getObjectByID(962,0,true);
					if(NULL == ob)
					{
						//fprintf(stderr,"�������Ҳ�������\n");
						return false;
					}
				}
				//
			}
			else
			{
				if(!packs.main.getObjectByID(&ob,961))
				{
					ob = packs.uom.getObjectByID(961,0,true);
					if(NULL == ob)
					{
						//fprintf(stderr,"�������Ҳ�������\n");
						return false;
					}
				}
			}

            
			if(NULL == ob)
				return false;
	
			if(--srcobj->data.dwNum)
				{
					Cmd::stRefCountObjectPropertyUserCmd send;
					send.qwThisID=srcobj->data.qwThisID;
					send.dwNum=srcobj->data.dwNum;
					sendCmdToMe(&send,sizeof(send));
				}
				else
				{
					packs.removeObject(srcobj);
				}

				if(--ob->data.dwNum)
				{
					Cmd::stRefCountObjectPropertyUserCmd send;
					send.qwThisID=ob->data.qwThisID;
					send.dwNum=ob->data.dwNum;
					sendCmdToMe(&send,sizeof(send));
					//fprintf(stderr,"Կ��count=%d\n",send.dwNum);
					//return true;
				}
				else
				{
					packs.removeObject(ob);
				}

				//fprintf(stderr,"Կ��,����ɾ��\n");
				return true;
			} 
		break;

    case Cmd::SCROLL_MAP_PROPERTY_USERCMD_PARA:
      {
        Cmd::stScrollMapPropertyUserCmd * smp = (Cmd::stScrollMapPropertyUserCmd*)rev;
        zObject *srcobj=packs.uom.getObjectByThisID(smp->qwThisID);
        if (srcobj && (srcobj->base->id==675 || srcobj->base->id==676) && srcobj->data.pos.loc()==Cmd::OBJECTCELLTYPE_COMMON)
        {
          strncpy(srcobj->data.maker,smp->mapname[0].strMapName,MAX_NAMESIZE);
          useObject(srcobj);
        }
        else if (srcobj && srcobj->data.pos.loc()==Cmd::OBJECTCELLTYPE_EQUIP&&srcobj->base->kind == ItemType_Amulet)
        {
          strncpy(srcobj->data.maker,smp->mapname[0].strMapName,MAX_NAMESIZE);
          useAmulet(srcobj);
        }
        return true;
      }
      break;
      //������ֵ�ع��б�
    case Cmd::REQUEST_INC_MAP_PROPERTY_USERCMD_PARA:
      {
        zObject *obj = this->packs.equip.getObjectByEquipPos(Cmd::EQUIPCELLTYPE_ADORN);
        if (obj && obj->base->kind == ItemType_Amulet)
        {
          char buf[1024];
          bzero(buf,sizeof(buf));
          Cmd::stScrollMapPropertyUserCmd *smp = (Cmd::stScrollMapPropertyUserCmd*)buf;
          smp->qwThisID = obj->data.qwThisID;
          constructInPlace(smp);
          smp->size = this->scene->getIncCity((const char*)smp->mapname);
          sendCmdToMe(smp,sizeof(Cmd::stScrollMapPropertyUserCmd) + MAX_NAMESIZE * smp->size);        
        }
        return true;
      }
      break;
    case PICKUPITEM_PROPERTY_USERCMD_PARA:
      {
		  Cmd::stPickUpItemPropertyUserCmd *pick=(Cmd::stPickUpItemPropertyUserCmd *)rev;

		  TeamManager * teamMan = SceneManager::getInstance().GetMapTeam(TeamThisID);

		  zPos p;
		  zSceneObject *ret = NULL;
		  p.x=pick->x;
		  p.y=pick->y;
		  ret = scene->getSceneObjectByPos(p);
		  if (ret)
		  {
			  //int ok = 0;
			  zObject *o=ret->getObject();
			  if (!o)
			  {
				  return false;
			  }
			  //������Ʒ������û���������µ���Ʒ
			  if ((o->base->id!=673 || o->base->id!=674) && (!ret->getOwner() || ret->getOwner() == this->id || (teamMan && teamMan->IsOurTeam(ret->getOwner()))))
			  {
				  //bool bret = false;
				  if (o->base->id==673)//�����
				  {
					  if (guard)
					  {
						  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���������ڣ�����������ڳ���");
						  return true;
					  }
					  if (o->data.maker[0]=='\0')
					  {
						  zPos newPos;
						  if (!this->scene->backtoCityMap())
						  {
							  //�������������
							  if (!this->scene->randzPosByZoneType(ZoneTypeDef::ZONE_RELIVE,newPos)) return true;
							  if (!this->goTo(newPos)) return true;
						  }
						  else
						  {
							  //��������ͼ��Ѱ�ҳ���
							  this->scene->changeMap(this,false);
						  }
					  }
					  else
					  {
						  zPos newPos;
						  newPos.x=o->data.durpoint;
						  newPos.y=o->data.dursecond;


						  SceneManager::CountryMap_iter country_iter = SceneManager::getInstance().country_info.find(scene->getCountryID());
						  if (country_iter == SceneManager::getInstance().country_info.end()) {
							  //unknow country
							  return true;
						  }


						  std::ostringstream os;
						  os << "name=" << o->data.maker;
						  os << " pos=" << o->data.durpoint << "," << o->data.dursecond;
						  Gm::gomap(this,os.str().c_str());
					  }
					  return true;
				  }
				  else if (o->base->id==674)//��ԯ��
				  {
					  if (ret->getOwner() == this->id)
					  {
						  if (guard)
						  {
							  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���������ڣ�����������ڳ���");
							  return true;
						  }
						  if (o->data.maker[0]=='\0')
						  {
							  zPos newPos;
							  if (!this->scene->backtoCityMap())
							  {
								  //�������������
								  if (!this->scene->randzPosByZoneType(ZoneTypeDef::ZONE_RELIVE,newPos)) return true;
								  if (!this->goTo(newPos)) return true;
							  }
							  else
							  {
								  //��������ͼ��Ѱ�ҳ���
								  this->scene->changeMap(this,false);
							  }
						  }
						  else
						  {
							  zPos newPos;
							  newPos.x=o->data.durpoint;
							  newPos.y=o->data.dursecond;


							  SceneManager::CountryMap_iter country_iter = SceneManager::getInstance().country_info.find(scene->getCountryID());
							  if (country_iter == SceneManager::getInstance().country_info.end()) {
								  //unknow country
								  return true;
							  }


							  std::ostringstream os;
							  os << "name=" << o->data.maker;
							  os << " pos=" << o->data.durpoint << "," << o->data.dursecond;
							  Gm::gomap(this,os.str().c_str());
						  }
					  }
					  return true;
				  }
			  }

			  if (!ret->getOwner() || ret->getOwner() == this->id ||(teamMan->IsOurTeam(ret->getOwner()) && (teamMan && teamMan->isNormalObj())))
			  {
				  Combination callback(this,o);
				  packs.main.execEvery(callback);
				  if (packs.equip.pack(EquipPack::L_PACK) && packs.equip.pack(EquipPack::L_PACK)->can_input()) packs.equip.pack(EquipPack::L_PACK)->execEvery(callback);
				  if (packs.equip.pack(EquipPack::R_PACK) && packs.equip.pack(EquipPack::R_PACK)->can_input()) packs.equip.pack(EquipPack::R_PACK)->execEvery(callback);

				  bool added = false;
				  if (o->data.dwNum) {

					  if (this->packs.uom.space(this) >= 1 && packs.addObject(o,true,AUTO_PACK)) {
						  //�����˫��������ߺ�����������Ҫ��
						  if (o->base->kind == ItemType_DoubleExp || o->base->kind == ItemType_Honor || o->base->kind == ItemType_ClearProperty)
						  {
							  o->data.bind=1;
						  }
						  added = true;
						  Cmd::stAddObjectPropertyUserCmd status;
						  status.byActionType = Cmd::EQUIPACTION_OBTAIN;
						  bcopy(&o->data,&status.object,sizeof(t_Object),sizeof(status.object));
						  sendCmdToMe(&status,sizeof(status));
					  }
					  else
					  {
						  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��İ�������");
						  Cmd::stAddMapObjectMapScreenUserCmd status;
						  status.action = Cmd::OBJECTACTION_DROP;
						  status.data.dwMapObjectTempID = o->data.qwThisID;
						  status.data.dwObjectID = o->data.dwObjectID;
						  strncpy(status.data.pstrName,o->data.strName,MAX_NAMESIZE);
						  status.data.x = p.x;
						  status.data.y = p.y;
						  status.data.wdNumber = o->data.dwNum;
						  status.data.wdLevel = o->base->level;
						  status.data.upgrade = o->data.upgrade;
						  status.data.kind = o->data.kind;
						  scene->sendCmdToNine(ret->getPosI(),&status,sizeof(status),this->dupIndex);  
					  }
				  }

				  if (callback.num() || added) {
					  OnGet event(o->data.dwObjectID);
					  EventTable::instance().execute(*this,event);
					  zObject::logger(o->createid,o->data.qwThisID,o->data.strName,o->data.dwNum,o->data.dwNum,1,this->scene->id,this->scene->name,this->id,this->name,"����",o->base,o->data.kind,o->data.upgrade);
					  if (ScriptQuest::get_instance().has(ScriptQuest::OBJ_GET,o->data.dwObjectID)) { 
						  char func_name[32];
						  sprintf(func_name,"%s_%d","get",o->data.dwObjectID);
						  execute_script_event(this,func_name,o);
					  }                
					  /*
					  Zebra::logger->debug("%s(%u)��%s(%u)��(%u,%u)",
					  name,id,o->name,o->id,ret->getPos().x,ret->getPos().y);
					  // */
				  }

				  if (added) {
					  ret->clear();
				  }
				  if (!o->data.dwNum || added) {
					  scene->removeObject(ret);

					  Cmd::stRemoveMapObjectMapScreenUserCmd re;
					  re.dwMapObjectTempID=ret->id;
					  scene->sendCmdToNine(getPosI(),&re,sizeof(re),this->dupIndex);

					  SAFE_DELETE(ret);
				  }

				  return true;
				  // }
			  }
			  else
			  {
				  zObject *o=ret->getObject();
				  if (o && o->base->id!=673 && o->base->id!=674)//��ԯ��
				  {
					  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�Բ���,����Ʒ��������");
				  }
			  }
		  }
	  }
	  break;

	  //sky �ϵĴ�����Ϣ���ڴ�����
    /*case FOUNDITEM_PROPERTY_USERCMD_PARA:
      if (!RebuildObject::instance().make(*this,rev))
      {
        RebuildObject::instance().response(*this,1,Base::MAKE);
      }
      break;*/
    case UPGRADEITEM_PROPERTY_USERCMD_PARA:
      if (!RebuildObject::instance().upgrade(*this,rev))
      {
        RebuildObject::instance().response(*this,1,Base::UPGRADE);
      }
      break;
    case COMPOSEITEM_PROPERTY_USERCMD_PARA:
      if (!RebuildObject::instance().compose(*this,rev))
      {
        RebuildObject::instance().response(*this,1,Base::COMPOSE);
      }
      break;
    case COMPOSE_SOUL_STONE_PROPERTY_USERCMD_PARA:
      if (!RebuildObject::instance().compose_soul_stone(*this,rev))
      {
        RebuildObject::instance().response(*this,1,Base::COMPOSE);
      }
      break;
    case HOLE_PROPERTY_USERCMD_PARA:
      if (!RebuildObject::instance().hole(*this,rev))
      {
        RebuildObject::instance().response(*this,1,Base::HOLE);
      }
      break;
    case ENCHASEL_PROPERTY_USERCMD_PARA:
      if (!RebuildObject::instance().enchance(*this,rev))
      {
        RebuildObject::instance().response(*this,1,Base::ENCHANCE);
      }
      break;
    case DECOMPOSE_PROPERTY_USERCMD_PARA:
      if (!RebuildObject::instance().decompose(*this,rev))
      {
        RebuildObject::instance().response(*this,1,Base::DECOMPOSE);
      }
      break;


    //�����û���ݼ�  
    case Cmd::ACCELKEY_PROPERTY_USERCMD_PARA:
      {
        Cmd::stAccekKeyPropertyUserCmd *acc = (Cmd::stAccekKeyPropertyUserCmd *)rev;
        int len = sizeof(Cmd::stAccekKeyPropertyUserCmd) + acc->accelNum * sizeof(Cmd::stGameAccelKey);
        if (acc->accelNum > 0 && len < 1024)
        {
          memcpy(accelData,rev,len,sizeof(accelData),sizeof(accelData));
        }
        else
        {
          bzero(accelData,sizeof(accelData));
        }
        //Zebra::logger->debug("(%s,%ld)�����û���ݼ��ɹ�",this->name,this->tempid);
        return true;
      }
      break;

    //����û���������
    case ADDUSERSKILL_PROPERTY_USERCMD_PARA:
      {
        if (!addSkillData((Cmd::stAddUserSkillPropertyUserCmd *)rev))
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����ٴ�ѧϰ�Ѿ�ѧϰ������");
        }
        return true;
      }
      break;
    
    //ɾ���û�����
    case REMOVEUSERSKILL_PROPERTY_USERCMD_PARAMETER:
      {
        if (removeSkill((Cmd::stRemoveUserSkillPropertyUserCmd *)rev))
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"ɾ������ʧ��");
        }
        return true;
      }
      break;

    //��������  
    case ADDSKILLPOINT_PROPERTY_USERCMD:
      {
        if (!upgradeSkill(((Cmd::stAddSkillPointPropertyUserCmd *)rev)->dwSkillID))
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���������ü���");
        }
        return true;
      }
      break;
    case SYSTEMSETTINGS_PROPERTY_USERCMD_PARA:
      {
		  Cmd::stSystemSettingsUserCmd *st = (Cmd::stSystemSettingsUserCmd *)rev;


		  //�������
		  if (isset_state(st->data.bySettings,Cmd::USER_SETTING_TEAM))
		  {
			  set_state(sysSetting,Cmd::USER_SETTING_TEAM);
			  setOpen(true);
		  }
		  else
		  {
			  clear_state(sysSetting,Cmd::USER_SETTING_TEAM);
			  setOpen(false);
		  }

		  bcopy(&st->data.bySettings[1],setting,sizeof(setting),sizeof(setting));
		  bcopy(st->data.dwChatColor,chatColor,sizeof(chatColor),sizeof(chatColor));


#ifdef _DEBUG
		  Zebra::logger->debug("�յ�ϵͳ������Ϣ:%x %x %x %x,%u %u %u %u %u %u %u %u",st->data.bySettings[0],st->data.bySettings[1],st->data.bySettings[2],st->data.bySettings[3],st->data.dwChatColor[0],st->data.dwChatColor[1],st->data.dwChatColor[2],st->data.dwChatColor[3],st->data.dwChatColor[4],st->data.dwChatColor[5],st->data.dwChatColor[6],st->data.dwChatColor[7]);
#endif

		  //֪ͨsession
		  Cmd::Session::t_sysSetting_SceneSession send;
		  bcopy(sysSetting,send.sysSetting,sizeof(send.sysSetting),sizeof(send.sysSetting));
		  strncpy((char *)send.name,name,MAX_NAMESIZE-1);
		  sessionClient->sendCmd(&send,sizeof(send));

		  //֪ͨ����
		  Cmd::Scene::t_sysSetting_GateScene gate_send;
		  bcopy(sysSetting,gate_send.sysSetting,sizeof(gate_send.sysSetting),sizeof(gate_send.sysSetting));
		  gate_send.id=this->id;
		  this->gatetask->sendCmd(&gate_send,sizeof(gate_send));


		  return true;
	  }
	  break;
/*
    //�������е���
    case DISTRIBUTEUFIVEPOINT_PROPERTY_USERCMD_PARA:
      {
        Zebra::logger->debug("(%s,%ld)�������е���ָ��",this->name,this->tempid);
        Cmd::stDistributeFivePointPropertyUserCmd *dis = (Cmd::stDistributeFivePointPropertyUserCmd *)rev;
        if (IsJoin(dis->fiveType) && charbase.points > 0 || (charbase.five == dis->fiveType && charbase.points > 0))
        {
          charbase.fivevalue[dis->fiveType] ++;
          charstate.fivevalue[dis->fiveType] ++;
          int type = dis->fiveType;
        charstate.fivedefence[(type + 1) % 5] = 
          (WORD)sqrt(sqrt((charbase.fivevalue[type] * charbase.fivevalue[type] * charbase.fivevalue[type])));
          if (charbase.fivevalue[type] % 4 == 0)
          {
            type =(type + 3) % 5;
            if (charbase.fivevalue[type] > 0)
            {
              charbase.fivevalue[type] ++;
              charstate.fivevalue[type] ++;
              charstate.fivedefence[(type + 1) % 5] = 
                (WORD)sqrt(sqrt((charbase.fivevalue[type] * charbase.fivevalue[type] * charbase.fivevalue[type])));
              if (charbase.fivevalue[type] % 4 == 0)
              {
                type =(type + 3) % 5;
                charbase.fivevalue[type] ++;
                charstate.fivevalue[type] ++;
                charstate.fivedefence[(type + 1) % 5] = 
                  (WORD)sqrt(sqrt((charbase.fivevalue[type] * charbase.fivevalue[type] * charbase.fivevalue[type])));
              }
            }
          }
          charbase.points --;

          //װ���ı乥����Ԥ����
          calPreValue();

          //ˢ���û�����
          Cmd::stMainUserDataUserCmd  userinfo;
          full_t_MainUserData(userinfo.data);
          sendCmdToMe(&userinfo,sizeof(userinfo));
          break;
*/
    //�������Ե���
    case DISTRIBUTEUPOINT_PROPERTY_USERCMD_PARA:
      {
        if (charbase.points>0)
        {
          Cmd::stDistributePointPropertyUserCmd *dis = (Cmd::stDistributePointPropertyUserCmd *)rev;
          switch(dis->type)
          {
            case Cmd::PROPERTY_CON:  //����
            case Cmd::PROPERTY_STR:  //����
            case Cmd::PROPERTY_DEX:  //����
            case Cmd::PROPERTY_INT:  //����
            case Cmd::PROPERTY_MEN:  //����
              {
                charbase.wdProperty[dis->type]++;
                charbase.points--;
                this->setupCharBase();
                //ˢ���û�����
                Cmd::stMainUserDataUserCmd  userinfo;
                full_t_MainUserData(userinfo.data);
                sendCmdToMe(&userinfo,sizeof(userinfo));
              }
              break;
            default:
              {
              }
              break;
          }
        }
        return true;
      }
      break;
    case CLEARPOINT_PROPERTY_USERCMD_PARA:
      {
        Cmd::stClearPointPropertyUserCmd *cmd = (Cmd::stClearPointPropertyUserCmd*)rev;
        switch (cmd->dwItemID)
        {
          case 752:
            if (this->charbase.level<40)
            {
              if (this->reduceObjectNum(752,1)==-1)
              {
                Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦��ʧ��",this->name);
                return true;
              }
            }
            else
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦��ʧ��",this->name);
              return true;
            }
            break;
          case 760:
            if (this->charbase.level>=40)
            {
              if (this->reduceObjectNum(760,1)==-1)
              {
                Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦��ʧ��",this->name);
                return true;
              }
            }
            else
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦��ʧ��",this->name);
              return true;
            }
            break;
          case 756:
            if (this->charbase.level>=40)
            {
              if (this->reduceObjectNum(756,1)==-1)
              {
                Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦��ʧ��",this->name);
                return true;
              }
            }
            else
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦��ʧ��",this->name);
              return true;
            }
            break;
          default:
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦��ʧ��",this->name);
              return true;
            }
            break;
        }
        charbase.points =charbase.points
            +charbase.wdProperty[0]
            +charbase.wdProperty[1]
            +charbase.wdProperty[2]
            +charbase.wdProperty[3]
            +charbase.wdProperty[4];
        charbase.wdProperty[0]=0;
        charbase.wdProperty[1]=0;
        charbase.wdProperty[2]=0;
        charbase.wdProperty[3]=0;
        charbase.wdProperty[4]=0;
        this->setupCharBase();
        //ˢ���û�����
        Cmd::stMainUserDataUserCmd  userinfo;
        full_t_MainUserData(userinfo.data);
        sendCmdToMe(&userinfo,sizeof(userinfo));
        Zebra::logger->info("��ɫ[%s]ʹ��ϴ���Ե㹦�ܳɹ�",this->name);
        return true;
      }
      break;
    case CLEARPOINT_LIMIT_PROPERTY_USERCMD_PARA:
      {
        Cmd::stClearPointLimitPropertyUserCmd *cmd = (Cmd::stClearPointLimitPropertyUserCmd*)rev;
        zObject *srcobj=packs.uom.getObjectByThisID(cmd->qwThisID);
        if (srcobj && srcobj->data.pos.loc() ==Cmd::OBJECTCELLTYPE_COMMON)
        {
          if (srcobj->base->kind == ItemType_ClearProperty && srcobj->base->id == 755)
          {
            BYTE num = cmd->byProperty%5;
            if (charbase.wdProperty[num]>5)
            {
              charbase.points +=5;
              charbase.wdProperty[num]=charbase.wdProperty[num]-5;
            }
            else
            {
              charbase.points += charbase.wdProperty[num];
              charbase.wdProperty[num]=0;
            }
            this->setupCharBase();
            //ˢ���û�����
            Cmd::stMainUserDataUserCmd  userinfo;
            full_t_MainUserData(userinfo.data);
            sendCmdToMe(&userinfo,sizeof(userinfo));

            zObject::logger(srcobj->createid,srcobj->data.qwThisID,srcobj->data.strName,srcobj->data.dwNum,srcobj->data.dwNum,0,this->id,this->name,0,NULL,"��ϴ5�����Ա�ʯ",NULL,0,0);
            Zebra::logger->info("��ɫ[%s]ʹ��ϴ5�����Թ��ܳɹ�",this->name);
            packs.removeObject(srcobj); //notify and delete
          }
        }
        return true;
      }
      break;
    case CLEARUSERSKILLPOINT_PROPERTY_USERCMD_PARAMETER:
      {
        Cmd::stClearUserSkillPointPropertyUserCmd *cmd = (Cmd::stClearUserSkillPointPropertyUserCmd*)rev;
        switch (cmd->dwItemID)
        {
          case 753:
            if (this->charbase.level<40)
            {
              if (this->reduceObjectNum(753,1)==-1)
              {
                Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦��ʧ��",this->name);
                return true;
              }
            }
            else
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦��ʧ��",this->name);
              return true;
            }
            break;
          case 761:
            if (this->charbase.level>=40)
            {
              if (this->reduceObjectNum(761,1)==-1)
              {
                Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦��ʧ��",this->name);
                return true;
              }
            }
            else
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦��ʧ��",this->name);
              return true;
            }
            break;
          case 757:
            if (this->charbase.level>=40)
            {
              if (this->reduceObjectNum(757,1)==-1)
              {
                Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦��ʧ��",this->name);
                return true;
              }
            }
            else
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦��ʧ��",this->name);
              return true;
            }
            break;
          default:
            {
              Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦��ʧ��",this->name);
              return true;
            }
            break;
        }
        charbase.skillpoint = charbase.level;
        usm.clear();
        Cmd::stClearSkillUserCmd send;
        sendCmdToMe(&send,sizeof(send));
        skillValue.init();
        this->setupCharBase();
        //ˢ���û�����
        Cmd::stMainUserDataUserCmd  userinfo;
        full_t_MainUserData(userinfo.data);
        sendCmdToMe(&userinfo,sizeof(userinfo));
        Zebra::logger->info("��ɫ[%s]ʹ��ϴ���ܵ㹦�ܳɹ�",this->name);
        return true;
      }
      break;
    case BODYCOLOR_PROPERTY_USERCMD_PARA:
      {
        using namespace Cmd;
        stBodyColorPropertyUserCmd *bcp = (stBodyColorPropertyUserCmd*)rev;
        if ((bcp->dwBodyColorCustom != charbase.bodyColor) && packs.equip.canChangeColor())
        {
          DWORD cost = 1000;
/*
          zObject *gold=packs.getGold();          
          if (!gold)
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�Բ���,�����Ӳ���");
            return true;
          }

          if (cost > gold->data.dwNum)
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�Բ���,�����Ӳ���");
            return true;
          }
          gold->data.dwNum -= cost;
          if (gold->data.dwNum==0)
          {
            stRemoveObjectPropertyUserCmd rmgold;
            rmgold.qwThisID=gold->data.qwThisID;
            sendCmdToMe(&rmgold,sizeof(rmgold));
            packs.rmObject(gold);
            SAFE_DELETE(gold);
          }
          else
          {
            stRefCountObjectPropertyUserCmd setgold;
            setgold.qwThisID=gold->data.qwThisID;
            setgold.dwNum=gold->data.dwNum;
            sendCmdToMe(&setgold,sizeof(setgold));
          }
*/
          if (!packs.checkMoney(cost) || !packs.removeMoney(cost,"��װȾɫ")) {
            Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�Բ���,�����Ӳ���");
            return true;
          }
          Channel::sendMoney(this,Cmd::INFO_TYPE_GAME,cost,"��װȾɫ�ɹ�,��������");
          if (packs.equip.equip(EquipPack::OTHERS3)  && ( packs.equip.equip(EquipPack::OTHERS3)->base->kind == ItemType_FashionBody || packs.equip.equip(EquipPack::OTHERS3)->base->kind == ItemType_HighFashionBody) )
          {
            packs.equip.equip(EquipPack::OTHERS3)->data.color = bcp->dwBodyColorCustom;
          }
          else if (packs.equip.equip(EquipPack::OTHERS2)  && ( packs.equip.equip(EquipPack::OTHERS2)->base->kind == ItemType_FashionBody || packs.equip.equip(EquipPack::OTHERS2)->base->kind == ItemType_HighFashionBody) )
          {
            packs.equip.equip(EquipPack::OTHERS2)->data.color = bcp->dwBodyColorCustom;
          }
          else if (packs.equip.equip(EquipPack::BODY)) 
          {
            packs.equip.equip(EquipPack::BODY)->data.color = bcp->dwBodyColorCustom;
          }
          else
          {
            charbase.bodyColor = bcp->dwBodyColorCustom;
          }
          //Cmd::stAddUserMapScreenUserCmd cmd;
          //full_t_MapUserData(cmd.data);
          //scene->sendCmdToNine(getPosI(),&cmd,sizeof(cmd),false);
          reSendMyMapData();
        }
        else
        {
          Zebra::logger->warn("�û�(%u,%s,%u)�������ɸ�����ɫ���·�",id,name,tempid);
        }
        return true;
      }
      break;
    case FACE_PROPERTY_USERCMD_PARA:
      {
        using namespace Cmd;
        stFacePropertyUserCmd *fp = (stFacePropertyUserCmd*)rev;
        if (fp->dwFace == charbase.face)
        {
          return true;
        }
        DWORD cost=0;
        zHeadListB *base=headlistbm.get(fp->dwFace);
        if (base)
        {
          cost = base->cost;
        }
        else
        {
          return true;
        }
        if (!packs.checkMoney(cost) || !packs.removeMoney(cost,"����ͷ��")) {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�Բ���,�����Ӳ���");
          return true;
        }
        
        charbase.face = fp->dwFace;
        Channel::sendMoney(this,Cmd::INFO_TYPE_GAME,cost,"ͷ������ɹ�,��������");
        //Cmd::stAddUserMapScreenUserCmd cmd;
        //full_t_MapUserData(cmd.data);
        //scene->sendCmdToNine(getPosI(),&cmd,sizeof(cmd),false);
        reSendMyMapData();
        return true;
      }
      break;
    case HAIR_PROPERTY_USERCMD_PARA:
      {
        using namespace Cmd;
        stHairPropertyUserCmd *hp = (stHairPropertyUserCmd*)rev;
        if (hp->dwHairColor == charbase.hair && hp->byHairType == getHairType())
        {
          return true;
        }
        DWORD cost = 0;
        int isChange = 0;
        if (hp->byHairType != getHairType())
        {
          zHairStyleB *base=hairstylebm.get(hp->byHairType);
          if (base)
          {
            cost = base->cost;
            isChange = 1;
          }
          /*
          switch(hp->byHairType)
          {
            case HairStype_1:
              {
                isChange = 1;
                cost = 2000;
              }
              break;
            case HairStype_2:
              {
                isChange = 1;
                cost = 2000;
              }
              break;
            case HairStype_3:
              {
                isChange = 1;
                cost = 2000;
              }
              break;
            case HairStype_4:
              {
                isChange = 1;
                cost = 2000;
              }
              break;
            case HairStype_5:
              {
                isChange = 1;
                cost = 2000;
              }
              break;
            case HairStype_6:
              {
                isChange = 1;
                cost = 2000;
              }
              break;
            default:
              break;
          }
          // */
        }
        if ((hp->dwHairColor & 0X00FFFFFF) != getHairColor())
        {
          zHairColourB *base=haircolourbm.get(hp->dwHairColor & 0X00FFFFFF);
          if (base)
          {
            /// ������ǹ�ͷ����ɫ����Ҫ����
            if (hp->byHairType != 1)
            {
              cost += base->cost;
            }
            isChange = 2;
          }
          /*
          switch(hp->dwHairColor)
          {
            case HairColor_black:
              {
                isChange = 2;
                cost += 1000;
              }
              break;
            case HairColor_drink:
              {
                isChange = 2;
                cost += 2000;
              }
              break;
            case HairColor_purple:
              {
                isChange = 2;
                cost += 3000;
              }
              break;
          }
          // */
        }
/*
        zObject *gold=packs.getGold();
        if (!gold)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�Բ���,�����Ӳ���");
          return true;
        }
        if (cost > gold->data.dwNum)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�Բ���,�����Ӳ���");
          return true;
        }
        gold->data.dwNum -= cost;
        if (gold->data.dwNum==0)
        {
          stRemoveObjectPropertyUserCmd rmgold;
          rmgold.qwThisID=gold->data.qwThisID;
          sendCmdToMe(&rmgold,sizeof(rmgold));
          packs.rmObject(gold);
          SAFE_DELETE(gold);
        }
        else
        {
          stRefCountObjectPropertyUserCmd setgold;
          setgold.qwThisID=gold->data.qwThisID;
          setgold.dwNum=gold->data.dwNum;
          sendCmdToMe(&setgold,sizeof(setgold));
        }
*/
        if (!packs.checkMoney(cost) || !packs.removeMoney(cost,"����")) {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�Բ���,�����Ӳ���");
          return true;
        }
        
        if (isChange)
        {
          Channel::sendMoney(this,Cmd::INFO_TYPE_GAME,cost,"�����ɹ�,��������");
        }
        else
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"��ѡ������ͬ����ѡ����������");
        }
        setHairType(hp->byHairType);
        setHairColor(hp->dwHairColor);
        //Cmd::stAddUserMapScreenUserCmd cmd;
        //full_t_MapUserData(cmd.data);
        //scene->sendCmdToNine(getPosI(),&cmd,sizeof(cmd),false);
        reSendMyMapData();
        return true;
      }
      break;
	case SELECT_PROPERTY_USERCMD_PARA:
		{
			using namespace Cmd;
			stSelectPropertyUserCmd * spu = (stSelectPropertyUserCmd*)rev;
			//Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�յ�ѡ��ָ��");
			if (spu->dwOldTempID)
			{
				if (spu->byOldType == MAPDATATYPE_USER)
				{
					SceneUser *pDel = SceneUserManager::getMe().getUserByTempID(spu->dwOldTempID);
					if (pDel)
					{
						//pDel->selected_lock.lock();
						pDel->selected.erase(this->tempid);
						//pDel->selected_lock.unlock();
					}
				}
				else
				{
					SceneNpc *pDel = SceneNpcManager::getMe().getNpcByTempID(spu->dwOldTempID);
					if (pDel)
					{
						//pDel->selected_lock.lock();
						pDel->selected.erase(this->tempid);
						//pDel->selected_lock.unlock();
					}
				}
			}
			//���Ŀ��Ϊ-1���ʾ����ѡ�б�Ķ���
			if (spu->dwTempID == (DWORD)-1)
			{
				break;
			}
			switch(spu->byType)
			{
			case MAPDATATYPE_NPC:
				{
					SceneNpc *pNpc = SceneNpcManager::getMe().getNpcByTempID(spu->dwTempID);
					if (pNpc)
					{
						//pNpc->selected_lock.lock();
						pNpc->selected.insert(SelectedSet_value_type(this->tempid));
						Cmd::stRTSelectedHpMpPropertyUserCmd ret;
						ret.byType = Cmd::MAPDATATYPE_NPC;
						ret.dwTempID = pNpc->tempid;//��ʱ���
						ret.dwHP = pNpc->hp;//��ǰѪ
						ret.dwMaxHp = pNpc->getMaxHP();//���hp
						ret.dwMP = 0;//this->charbase.mp;//��ǰmp
						ret.dwMaxMp = 0;//this->charstate.maxmp;//���mp
						//pNpc->selected_lock.unlock();
						this->sendCmdToMe(&ret,sizeof(ret));
						char Buf[200]; 
						bzero(Buf,sizeof(Buf));
						stSelectReturnStatesPropertyUserCmd *srs=(stSelectReturnStatesPropertyUserCmd*)Buf;
						constructInPlace(srs);
						srs->byType = MAPDATATYPE_NPC;
						srs->dwTempID = spu->dwTempID;
						pNpc->skillStatusM.getSelectStates(srs,sizeof(Buf));
						if (srs->size > 0)
						{
							this->sendCmdToMe(srs,sizeof(stSelectReturnStatesPropertyUserCmd) + 
								sizeof(srs->states[0]) * srs->size);
						}
					}
				}
				break;
			case MAPDATATYPE_USER:
				{
					using namespace Cmd;
					SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(spu->dwTempID);
					if (pUser)
					{
						//Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"%s�յ�ѡ��%sָ��",this->name,pUser->name);
						//pUser->selected_lock.lock();
						pUser->selected.insert(SelectedSet_value_type(this->tempid));
						Cmd::stRTSelectedHpMpPropertyUserCmd ret;
						ret.byType = Cmd::MAPDATATYPE_USER;
						ret.dwTempID = pUser->tempid;//��ʱ���
						ret.dwHP = pUser->charbase.hp;//��ǰѪ
						ret.dwMaxHp = pUser->charstate.maxhp;//���hp
						ret.dwMP = pUser->charbase.mp;//��ǰmp
						ret.dwMaxMp = pUser->charstate.maxmp;//���mp
						//pUser->selected_lock.unlock();
						this->sendCmdToMe(&ret,sizeof(ret));
						char Buf[200]; 
						bzero(Buf,sizeof(Buf));
						stSelectReturnStatesPropertyUserCmd *srs=(stSelectReturnStatesPropertyUserCmd*)Buf;
						constructInPlace(srs);
						srs->byType = MAPDATATYPE_USER;
						srs->dwTempID = spu->dwTempID;
						pUser->skillStatusM.getSelectStates(srs,sizeof(Buf));
						if (srs->size > 0)
						{
							this->sendCmdToMe(srs,sizeof(stSelectReturnStatesPropertyUserCmd) + 
								sizeof(srs->states[0]) * srs->size);
						}
						//��ʱ����pk����״̬,�ȴ��Ժ�߻��޸�^_^����
						pUser->sendtoSelectedPkAdditionStateToUser(this);
						pUser->sendtoSelectedReliveWeakStateToUser(this);
						pUser->sendtoSelectedTrainStateToUser(this);
					}
				}
				break;
			default:
				break;
			}
		}
      break;
      //����۲�ĳ���û�
    case SELECT_MAINUSER_PROPERTY_USERCMD_PARA:
      {
        using namespace Cmd;
        stSelectMainUserPropertyUserCmd *smu = (stSelectMainUserPropertyUserCmd*)rev;
        switch(smu->byType)
        {
          case MAPDATATYPE_USER:
            {
              SceneUser *pUser = SceneUserManager::getMe().getUserByTempID(smu->dwTempID);
              if (pUser)
              {
                if (!pUser->mask.is_masking())
                {
                  char Buf[sizeof(stSelectReturnMainUserPropertyUserCmd) + sizeof(EquipedObject) * 16];
                  bzero(Buf,sizeof(Buf));
                  stSelectReturnMainUserPropertyUserCmd *srm = (stSelectReturnMainUserPropertyUserCmd*)Buf;
                  constructInPlace(srm);
                  srm->dwTempID = pUser->tempid;
                  pUser->full_t_MainUserData(srm->mainuser_data);
                  pUser->full_t_MapUserData(srm->mapuser_data);
                
                  srm->dwSize = pUser->packs.equip.fullAllEquiped((char*)srm->object_data);
                  this->sendCmdToMe(srm,sizeof(stSelectReturnMainUserPropertyUserCmd) 
                      + sizeof(EquipedObject) * srm->dwSize);

                  if (pUser->horse.horse())
                  {
                      stSelectReturnHorsePropertyUserCmd send;
                      pUser->horse.full_HorseDataStruct(&send.data);
                      sendCmdToMe(&send,sizeof(send));
                  }
                }
                else
                {
                  Channel::sendSys(this,Cmd::INFO_TYPE_MSG,"�����������޷��۲�");
                }
              }
            }
            break;
          case MAPDATATYPE_NPC:
            {
            }
            break;
          default:
            break;
        }
      }
      break;
    case HONOR_TO_PKVALUE_PROPERTY_USERCMD_PARA:
      {
        short good = this->charbase.goodness & 0x0000FFFF;
        short old_good = good;
        DWORD old_honor = this->charbase.honor;
        if (good > 0 && good < (short)Cmd::GOODNESS_7 && this->charbase.honor > 0)
        {
          if (good <= (short)(this->charbase.honor / 5))
          {
            this->charbase.honor -= good * 5;
            good = 0;
          }
          else
          {
            good -= (short)(this->charbase.honor / 5);
            this->charbase.honor = this->charbase.honor % 5;
          }
          this->charbase.goodness = this->charbase.goodness & 0XFFFF0000 + good;
          Cmd::stMainUserDataUserCmd  userinfo;
          full_t_MainUserData(userinfo.data);
          sendCmdToMe(&userinfo,sizeof(userinfo));
          reSendMyMapData();
          zObject::logger(0,0,"����ֵ",this->charbase.honor,old_honor - this->charbase.honor,0,this->id,this->name,0,NULL,"ϴPKֵ�۳�����ֵ",NULL,0,0);
          return Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"����%u������������%u���ӵ����ʱ��",old_honor - this->charbase.honor,old_good - good);
        }
        return true;
      }
      break;
    case GO_TRAIN_PROPERTY_USERCMD_PARA:
      {
        if ((privatestore.step() != PrivateStore::NONE) || tradeorder.hasBegin())//���ڽ���
          return true;

        Cmd::stGoTrainPropertyUserCmd * cmd = (Cmd::stGoTrainPropertyUserCmd *)rev;

        char mname[MAX_NAMESIZE];
        bzero(mname,MAX_NAMESIZE);
        DWORD mid = 192+cmd->level;
        SceneManager::getInstance().buildMapName(6,mid,mname);
        zPos p;
        switch (cmd->level)
        {
          case 1:
            p = zPos(172,265);
            break;
          case 2:
            p = zPos(257,144);
            break;
          case 3:
            p = zPos(34,188);
            break;
          case 4:
            p = zPos(139,270);
            break;
          case 5:
            p = zPos(69,192);
            break;
          case 6:
            p = zPos(217,208);
            break;
          case 7:
            p = zPos(245,200);
            break;
          case 8:
            p = zPos(78,187);
            break;
          case 9:
            p = zPos(107,204);
            break;
          default:
            Zebra::logger->error("%s ����������ͼ�������� level=%u",name,cmd->level);
            return true;
            break;
        }

        zObject * o = 0;
        if (!packs.main.getObjectByID(&o,798))
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��ȱ��һ����Ҫ�ĵ���");
          return true;
        }

        packs.removeObject(o,true,true);
        charbase.trainTime = 86399;
        sendtoSelectedTrainState();

        Scene * s=SceneManager::getInstance().getSceneByName(mname);
        if (s)
        {
          if (changeMap(s,p))
            Zebra::logger->info("%s ����������ͼ %s(%u,%u)",name,s->name,getPos().x,getPos().y);
          else
            Zebra::logger->info("%s ����������ͼʧ�� %s(%u,%u)",name,s->name,p.x,p.y);
        }
        else
        {       
          Cmd::Session::t_changeScene_SceneSession cmd;
          cmd.id = id;
          cmd.temp_id = tempid;
          cmd.x = p.x;
          cmd.y = p.y;
          cmd.map_id = 0; 
          cmd.map_file[0] = '\0';
          strncpy((char *)cmd.map_name,mname,MAX_NAMESIZE);
          sessionClient->sendCmd(&cmd,sizeof(cmd));
        }
      }
      break;
    default:
      break;
  }
//  Zebra::logger->debug("SceneUser::doPropertyCmd\tparam:%d",rev->byParam);
  return false;
}

