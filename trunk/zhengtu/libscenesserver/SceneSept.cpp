/**
 * \brief ʵ�ּ�������Ĵ���
 *
 */
#include <zebra/ScenesServer.h>


using namespace SeptDef;

/**
  * \brief һ���Ƚ���
  *
  *  ���ڲ��ҽ�����������Ҫ�ĵ����Ƿ����
  *  
  *
  */
class SeptObjectCompare:public UserObjectCompare 
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
  * \brief �����û���������
  *
  * ����Ĺ�ϵ��������:
  *
  * Cmd::SEPT_STATUS_CHECK_PARA
  *
  * Cmd::CREATE_SEPT_PARA
  *
  *  Cmd::ADD_MEMBER_TO_SEPT_PARA
  *
  * \param rev: ��������
  * \param cmdLen: �����
  *
  * \return ���������TRUE,����ΪFALSE
  *
  *
  */
bool SceneUser::doSeptCmd(const Cmd::stSeptUserCmd *rev,DWORD cmdLen)
{
  switch(rev->byParam)
  {
    case Cmd::DONATE_HONOR_PARA:
      {
        Cmd::stDonateHonor* ptCmd = (Cmd::stDonateHonor*)rev;
        
        if (ptCmd->dwHonor>this->charbase.honor)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���������㲻��!");
        }

        if (ptCmd->dwHonor%10!=0)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���������㲻��5�ı���");
        }

        if (this->charbase.honor>ptCmd->dwHonor)
        {
          this->charbase.honor = this->charbase.honor - ptCmd->dwHonor;
        }
        else
        {
          this->charbase.honor = 0;
        }

        //֪ͨ�ͻ���
        Cmd::stMainUserDataUserCmd send;
        this->full_t_MainUserData(send.data);
        this->sendCmdToMe(&send,sizeof(send));

        Cmd::Session::t_OpRepute_SceneSession sendSession;
        sendSession.dwSeptID = this->charbase.septid;
        sendSession.dwRepute = ptCmd->dwHonor/10;
        sessionClient->sendCmd(&send,sizeof(send));
        
        return true;
      }
      break;
    case Cmd::SEPT_STATUS_CHECK_PARA:
    case Cmd::CREATE_SEPT_PARA:
      {
        BYTE bState = 1; //�ȳ�ʼ���ɳɹ�״̬
        if (charbase.septid != 0)  // ����ID����0��ʾ�������		
		{
			bState = 0;
			Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�Ѿ���������޷��ٴ�������");
		}
		else
        {
          time_t cur_time = time(NULL);
          if (/*(cur_time - charbase.levelsept) < 24*60*60*/0)
          {
            //�뿪ʱ�䲻��24Сʱ
            bState = 0;
           // Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,
           //     "�뿪�����24Сʱ,�����ٽ�������");
          }
          else 
		  {
			  if (packs.checkMoney(CREATE_SEPT_NEED_PRICE_GOLD) /*&& packs.removeMoney(CREATE_SEPT_NEED_PRICE_GOLD)*/) {
				  //����ɹ�״̬
				  bState = 1;
			  }
			  else
			  {
				  //����û���㹻��Ǯ
				  bState = 0;
				  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"����Ҫ%u��Ǯ����������,��û���㹻��Ǯ",CREATE_SEPT_NEED_PRICE_GOLD);
			  }
          }
        }


        //���ؼ��彨��״̬
        if (Cmd::CREATE_SEPT_PARA == rev->byParam)
        {
          Cmd::stCreateSeptCmd *ptCmd=(Cmd::stCreateSeptCmd *)rev;
          if (1 == bState)
          {
            Cmd::Session::t_addSept_SceneSession send;
            
            send.dwMapTempID = scene->tempid;
            send.info.dwSeptID = 0;        // �����ID
            strncpy(send.info.name,ptCmd->SeptName,MAX_NAMESIZE);     // ��������
            send.info.dwCharID = charbase.id;    // �᳤�Ľ�ɫID
            send.info.byVote = 0;      // Ĭ�ϲ�����ͶƱ�ڼ�
            send.info.dwRepute = 0;
            send.info.dwUnionID = 0;
            send.info.dwLevel = 1;
            send.info.dwSpendGold = 0;
            send.info.dwIsExp = 0;
            send.info.calltimes = 0;
            send.info.calldaytime = 0;
            send.info.normalexptime = 0;
            strncpy(send.info.masterName,charbase.name,MAX_NAMESIZE);  // �᳤������
            bzero(send.info.note,sizeof(send.info.note));
			strncpy( send.info.ExterData, "�᳤,FFFFFFFF;,0;,0;,0;,0;,0;,0;,0;,0;��Ա,0;", 512);
            sessionClient->sendCmd(&send,sizeof(send));
            return true;
          }
        }
        ////////////////////////////////////////
        if (1 == bState)
        {
            Cmd::stSeptStatusCheckCmd retCmd;
            sendCmdToMe(&retCmd,sizeof(retCmd));
        }
        ///////////////////////////////////////
        
        return true;
      }
      break;
    case Cmd::ADD_MEMBER_TO_SEPT_PARA:
      {
        Cmd::stAddMemberToSeptCmd *ptCmd=(Cmd::stAddMemberToSeptCmd *)rev;
        switch(ptCmd->byState)
        {
          case Cmd::QUESTION:
            {
              if (0 == charbase.septid)
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"������ȴ�������������ճ�Ա");
                return true;
              }
              SceneUser *pUser=scene->getUserByName(ptCmd->memberName);
              if (NULL != pUser)
              {
                //if (scene->checkUserInNine(this,pUser))
                //{
                  if (!isset_state(pUser->sysSetting,Cmd::USER_SETTING_FAMILY))                                                            
                  {
                    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��� %s���빤��δ����",pUser->name);                                                   
                      return true;
                  }  
                  if (pUser->charbase.level >=JOIN_SEPT_NEED_LEVEL)
                  {
                    
//Shx Modify
//if (0==pUser->charbase.septid && pUser->charbase.unionid==0)// Ϊ0��ʾδ�������
 if (0==pUser->charbase.septid )
                    {
                      time_t cur_time = time(NULL);

//[Shx Delete ����ʱ��������]
//                     if (((cur_time - pUser->charbase.levelsept) < 24*60*60)                                                                       
//                       {
//                         Channel::sendSys(this,
//         Cmd::INFO_TYPE_FAIL,"���ڻ�������������,����һ�ʣ %d ������ٴμ������",
//         24*60*60-(cur_time-pUser->charbase.levelsept));
//                         return true;
// 
//                       }
//                       if((cur_time - pUser->charbase.levelsept) <  24*60*60)
//                       {
//                         Channel::sendSys(this,
//                             Cmd::INFO_TYPE_FAIL,"���ڻ�������������");
//                         return true;
// 
//                       }

                      if (pUser->charbase.country == charbase.country)
                      {
                        strncpy(ptCmd->memberName,name,MAX_NAMESIZE);
                        ptCmd->memberID = id;					
                        pUser->sendCmdToMe(ptCmd,sizeof(Cmd::stAddMemberToSeptCmd));
                        return true;
                      }
                      else
                      {
                        Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����㲻��ͬһ��������,�㲻��������");
                      }
                    }
                    else
                    {
                      if (pUser->charbase.septid == charbase.septid)
                        Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���Ѿ���������Ĺ���,�޷��ٴ�����");
                      else
                        Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���Ѽ�����������");
                    }
                  }
                  else
                  {
                    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�ȼ�����%d�����ܼ��빤�ᣡ",JOIN_SEPT_NEED_LEVEL);
                  }
              }
              else
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��Ҳ��ڸ�ǰ,�޷���Ӧ����");
              }
              return true;
            }
            break;
          case Cmd::ANSWER_YES:
            {
              SceneUser *pUser=scene->getUserByID(ptCmd->memberID);
              if (pUser)
              {
                //charbase.septid = pUser->charbase.septid;
                //Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"%s�����������������˼���",name);
                pUser->removeWarRecord(Cmd::SEPT_DARE);
                pUser->removeWarRecord(Cmd::SEPT_NPC_DARE);
    
				
				pUser->sendMeToNine();
                
                Cmd::Session::t_addSeptMember_SceneSession send;
                send.dwSeptID = pUser->charbase.septid;
                send.member.dwCharID = charbase.id;            // ��Ա��ɫID
                send.member.wdOccupation = charbase.face;
				send.member.nRight = 9;		// Shx Add Ĭ��Ȩ��Ϊ9 : ��Ա.	
                bzero(send.member.aliasname,sizeof(send.member.aliasname));
                strncpy(send.member.name,charbase.name,MAX_NAMESIZE);    // ��Ա��ɫ����  
                sessionClient->sendCmd(&send,sizeof(send));
              }
              else
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�Է��Ѿ��뿪,���������������");
              }
              return true;
            }
            break;
          case Cmd::ANSWER_NO:
            {
              SceneUser *pUser=scene->getUserByID(ptCmd->memberID);
              if (pUser)
              {
                Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"%s��Ը��������,�ܾ����������",name);
              }
              return true;
            }
            break;
          default:
            break;
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

