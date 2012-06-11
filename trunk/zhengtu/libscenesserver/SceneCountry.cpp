/**
 * \brief ʵ�ֹ������ָ��Ĵ���
 *
 * 
 */
#include <zebra/ScenesServer.h>
#include <math.h>

//const DWORD CHANGE_COUNTRY_NEED_MONEY = 5 * 10000; // �������,��������5��
//const DWORD CANCEL_COUNTRY_NEED_MONEY = 5 *  10000; // ȡ������,��������5��
extern DWORD cancel_country_need_money;
extern DWORD is_cancel_country;

/**
 * \brief ִ�й����������
 *
 *
 * \param rev ��սָ��
 * \param cmdLen ��Ϣ����
 * \return �Ƿ�ɹ�
 */
bool SceneUser::doCountryCmd(const Cmd::stCountryUserCmd *rev,DWORD cmdLen)
{
  Zebra::logger->debug("doCountryCmd receive byParam:[%d]",rev->byParam);

  switch (rev->byParam)
  {
    case REQUEST_COUNTRY_POWER_PARA:
      {
        Cmd::stReturnCountryPowerCmd send;
        send.country[0]=0;
        send.country[1]=0;
        for(int i=0; i<13; i++) if (ScenesService::getInstance().countryPower[i]==1) Cmd::set_state(send.country,i);
        this->sendCmdToMe(&send,sizeof(send));
        return true;
      }
      break;
    case Cmd::ANSWER_COUNTRY_DARE_PARA:
      {
        Cmd::stAnswerCountryDareUserCmd* cmd = (Cmd::stAnswerCountryDareUserCmd*)rev;
        if (cmd->byStatus == Cmd::ANSWER_COUNTRY_DARE_YES)
        {
          Cmd::Session::t_changeScene_SceneSession cmd;
          Scene* pScene= SceneManager::getInstance().
            getSceneByID(SceneManager::getInstance().
                buildMapID(this->charbase.country,137));//�߾�

          if (this->scene->getRealMapID() == 137)
          {
            return true;
          }

          this->charbase.exploit = this->charbase.exploit+(2*exploit_arg);
          this->charbase.gomaptype = ZoneTypeDef::ZONE_COUNTRY_WAR;

          if (pScene)            
          {//����                         
            zPos Pos;
            Pos.x = 0;
            Pos.y = 0;     

            this->changeMap(pScene,Pos);
          }       
          else
          {               

            cmd.id = this->id;
            cmd.temp_id = this->tempid;
            cmd.x = 0;              
            cmd.y = 0;
            cmd.map_id = SceneManager::getInstance().buildMapID
              (this->charbase.country,137);
            //�߾�

            bzero(cmd.map_file,sizeof(cmd.map_file));
            bzero(cmd.map_name,sizeof(cmd.map_file));
            sessionClient->sendCmd(&cmd,sizeof(cmd));
          }
        }
        return true;
      }
      break;
    case Cmd::CONTRIBUTE_COUNTRY_MATERIAL:
      {
        Cmd::stContributeCountryMaterialCmd* cmd = (Cmd::stContributeCountryMaterialCmd*)rev;
        if (cmd->itemID && cmd->itemID != 0xffffffff)//INVALID_THISID
        {
          zObject * srcobj=this->packs.uom.getObjectByThisID(cmd->itemID);
          if (!srcobj)
          {
            Zebra::logger->info("%s ���Ҿ���ʱδ�ҵ�����Ʒ id=%u",
                this->name,cmd->itemID);
            return true;
          }

          zCountryMaterialB* country_material = srcobj->canContribute();
          if (country_material==NULL)
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�㲻�ܾ��������Ʒ");
            return true;
          }
            
          Cmd::Session::t_ContributeCountry_SceneSession send;
          send.byType = srcobj->getMaterialKind();
          
          if (send.byType>0)
          {
            if (country_material->dwMaterialKind == 1)  
            {//��������ĸߵȼ�����
              send.dwValue = (DWORD)pow((double)5,(int)srcobj->data.upgrade+1) * srcobj->data.dwNum;  
            }
            else
            {//��ͨ����
              send.dwValue = srcobj->data.dwNum;
            }
            send.byType = 1;
          }
          else
          {
            if (country_material->dwMaterialKind == 3)
            {
              send.dwValue = srcobj->data.dwNum;
              send.byType = 3;
            }
            else
            {
              send.dwValue = (srcobj->base->price*srcobj->data.dwNum)/100;
            }
          }

          if (send.dwValue>0)
          {
            if (send.byType !=3)
            {
            DWORD add_exploit = (DWORD)((((float)(send.dwValue)/10)/5) * exploit_arg);
            
            Zebra::logger->info("[���Ҿ���]: ��ǰ��ѫ:%d ���λ�ù�ѫ:%d ��ǰ����ֵ:%d ���λ������ֵ:%d",
                this->charbase.exploit,add_exploit,
                this->charbase.honor,(send.dwValue/5));

            this->charbase.exploit = this->charbase.exploit + add_exploit;
            
            BUFFER_CMD(Cmd::stAddUserAndPosMapScreenStateUserCmd,send2,zSocket::MAX_USERDATASIZE);
            this->full_t_MapUserDataPosState(send2->data);
            sendCmdToMe(send2,send2->size());
            //�ֲ�Ҫ������ֵ��,nb�߻�
            //this->charbase.honor += send.dwValue/5;
            //this->charbase.maxhonor += send.dwValue/5;
            Cmd::stMainUserDataUserCmd send1;
            this->full_t_MainUserData(send1.data);
            this->sendCmdToMe(&send1,sizeof(send1));
            }
            else if (send.byType == 3)
            {
              this->charbase.exploit = this->charbase.exploit + 
                ((send.dwValue*2) * exploit_arg);
              /*Cmd::stMainUserDataUserCmd send1;
                this->full_t_MainUserData(send1.data);
                this->sendCmdToMe(&send1,sizeof(send1));*/

              BUFFER_CMD(Cmd::stAddUserAndPosMapScreenStateUserCmd,send2,zSocket::MAX_USERDATASIZE);
              this->full_t_MapUserDataPosState(send2->data);
              sendCmdToMe(send2,send2->size());
            }

            send.dwCountry = this->charbase.country;
            sessionClient->sendCmd(&send,sizeof(send));
          }
          
          zObject::logger(srcobj->createid,srcobj->data.qwThisID,srcobj->base->name,
              srcobj->data.dwNum,srcobj->data.dwNum,0,this->id,this->name,0,"����","���׵�����",NULL,0,0);

          this->packs.removeObject(srcobj);//�Ƴ���Ʒ

          this->save(Cmd::Record::OPERATION_WRITEBACK);
          return true;
        }
      }
      break;
    case Cmd::CANCEL_COUNTRY_PARA:
      {
        //Cmd::stCancelCountryCmd * cmd = (Cmd::stCancelCountryCmd *)rev;
        Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�ù�����ʱδ����");
        return true;

        SceneManager::CountryMap_iter src_pos = SceneManager::getInstance().
          country_info.find(this->charbase.country);

        if (this->charbase.country == PUBLIC_COUNTRY)  
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"���Ѿ����޹�����ʿ��");
          return true;
        }

        this->charbase.country = PUBLIC_COUNTRY;
        if (this->scene->getCountryID() == 6)
        {
          deathBackToMapID = (this->charbase.country << 16 ) + 
            this->scene->getCommonCountryBacktoMapID();
        }       
        else
        {
          if (this->charbase.country == 6)
          {
            deathBackToMapID = (this->charbase.country << 16 ) + 
              this->scene->getCommonUserBacktoMapID();                                             }
          else
          {
            deathBackToMapID = (this->charbase.country << 16 ) + 
              this->scene->getForeignerBacktoMapID();
          }
        }             

        // ���Ǯׯ������Ʒ
//        this->packs.store.removeAll();  
//        packs.clearPackage(&packs.store);
        packs.execEvery(&packs.store,Type2Type<ClearPack>());

        // �����������
        this->quest_list.clear(this);

        // �����ѫ���Ĳ�ֵ
        this->charbase.exploit = 0;
        this->save(Cmd::Record::OPERATION_WRITEBACK);
        Cmd::Session::t_changeCountry_SceneSession send;

        Channel::sendSys(this,Cmd::INFO_TYPE_GAME,
            "���Ѿ��뿪 %s,��Ϊ�޹�����ʿ,ԭ������ϵ��Ǯׯ��Ʒ�����,ϣ�����������������������",
            src_pos->second.name);

        send.dwUserID = this->id;
        send.dwToCountryID = this->charbase.country;
        sessionClient->sendCmd(&send,sizeof(send));

        return true;
      }
      break;
    case Cmd::APPLY_COUNTRY_PARA:
      {
        Cmd::stApplyCountryCmd* cmd = (Cmd::stApplyCountryCmd*)rev;
        
        Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�ù�����ʱδ����");
        return true;

        if (this->charbase.country != PUBLIC_COUNTRY)  
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���������ȳ�Ϊ�޹�����ʿ,������������µĹ���");
          return true;
        }

        if (cmd->dwToCountryID == PUBLIC_COUNTRY)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,
              "���������Ϊ�޹�����ʿ��");
          return true;
        }

        if (is_cancel_country)
        {       
          if (this->packs.checkMoney(cancel_country_need_money)                                                        && this->packs.removeMoney(cancel_country_need_money,"�ı����"))
          {
            this->charbase.country = cmd->dwToCountryID;

            if (this->scene->getCountryID() == 6)
            {
              deathBackToMapID = (this->charbase.country << 16 ) + 
                this->scene->getCommonCountryBacktoMapID();
            }       
            else
            {
              if (this->charbase.country == 6)
              {
                deathBackToMapID = (this->charbase.country << 16 ) + 
                  this->scene->getCommonUserBacktoMapID();                                             }
              else
              {
                deathBackToMapID = (this->charbase.country << 16 ) + 
                  this->scene->getForeignerBacktoMapID();
              }
            }            

            this->relive();  
          }
          else
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��Ǯ����5��,������������¹���");
          }
        }
        else
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��ʱδ�����ѹ����ܡ�");
        }

        return true;
      }
      break;
    case Cmd::CHANGE_COUNTRY_PARA:
      {
        Cmd::stChangeCountryCmd* cmd = (Cmd::stChangeCountryCmd*)rev;

        if (this->charbase.country == cmd->dwToCountryID)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���Ѿ��Ǹù�������������");
          return true;
        }
        time_t cur_time = time(NULL);

        if ((cur_time - this->lastChangeCountryTime)< 24*60*60*3)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�ѹ��������Ժ�����ٴ��ѹ�");
          return true;
        }
        
        if (6 == cmd->dwToCountryID)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���ܱ����������");
          return true;
        }

        SceneManager::CountryMap_iter src_pos = SceneManager::getInstance().
          country_info.find(this->charbase.country);

        SceneManager::CountryMap_iter cur_pos = SceneManager::getInstance().
          country_info.find(cmd->dwToCountryID);

        if (src_pos == SceneManager::getInstance().country_info.end() 
            || cur_pos == SceneManager::getInstance().country_info.end())
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���Ҳ�����,�޷����");
          return true;
        }

        if (this->charbase.unionid>0 || this->charbase.septid>0)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�������˳���������ٱ������");
          return true;
        }

        const DWORD change_country_need_gold = 5000;

        if (packs.checkGold(change_country_need_gold)
            && packs.removeGold(change_country_need_gold,"�ı����"))
        {       
          this->charbase.country = cmd->dwToCountryID;
          this->deathBackToMapID  = (this->charbase.country << 16 ) + 102;

          // ���Ǯׯ������Ʒ
          //          this->packs.store.removeAll();  
          //          packs.clearPackage(&packs.store);
          packs.execEvery(&packs.store,Type2Type<ClearPack>());          
          // �����������
          //this->quest_list.clear(this);

          // �����ѫ���Ĳ�ֵ
          this->charbase.exploit = 0;
          this->save(Cmd::Record::OPERATION_WRITEBACK);
          Cmd::Session::t_changeCountry_SceneSession send;

          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,
              "���Ѿ��뿪 %s ������ %s,ԭ������ϵ��Ǯׯ��Ʒ�����,ϣ�����������������������",
              src_pos->second.name,cur_pos->second.name);

          send.dwUserID = this->id;
          send.dwToCountryID = cmd->dwToCountryID;
          sessionClient->sendCmd(&send,sizeof(send));
          this->lastChangeCountryTime = cur_time;
        }
        else
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��Ҳ���50��,���ܱ������");
        }

        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

