/**
 * \brief ʵ�ְ������Ĵ���
 *
 */
#include <zebra/ScenesServer.h>

using namespace UnionDef;
/**
  * \brief һ���Ƚ���
  *
  *  ���ڲ��ҽ����������Ҫ�ĵ����Ƿ����
  *  
  *
  */
class UnionObjectCompare:public UserObjectCompare 
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
  * \brief �����û��������
  *
  * ����İ����������:
  *
  * Cmd::UNION_STATUS_CHECK_PARA
  *
  * Cmd::CREATE_UNION_PARA
  *
  * Cmd::ADD_MEMBER_TO_UNION_PARA
  *
  * \param rev: �������
  * \param cmdLen: �����
  *
  * \return ���������TRUE,����ΪFALSE
  *
  *
  */
bool SceneUser::doUnionCmd(const Cmd::stUnionUserCmd *rev,DWORD cmdLen)
{
//[Shx Delete ԭ���,ɾ��];
	return  TRUE;


  switch(rev->byParam)
  {
    case Cmd::CONTRIBUTE_UNION_PARA:
      {
        Cmd::stContributeUnion* ptCmd = (Cmd::stContributeUnion*)rev;

        if (ptCmd->dwMoney>0 && this->charbase.unionid>0) {
          if (packs.checkMoney(ptCmd->dwMoney) 
              && packs.removeMoney(ptCmd->dwMoney,"������")) {  
            Cmd::Session::t_OpUnionMoney_SceneSession send;
            send.dwUnionID = this->charbase.unionid;
            send.dwMoney = ptCmd->dwMoney;
            sessionClient->sendCmd(&send,sizeof(send));

            this->charbase.exploit += (ptCmd->dwMoney/2000 * exploit_arg);
            
            Cmd::stAddUserMapScreenUserCmd update;
            full_t_MapUserData(update.data);
            sendCmdToMe(&update,sizeof(update));
          }
          else
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"ʣ����������,���ܾ���");
          }
        }
                
        return true;
      }
      break;
    case Cmd::QUESTION_UNION_CITY_INFO_PARA:
      {
        Cmd::Session::t_questionUnionCity_SceneSession send;
        Cmd::stQuestionUnionCityInfo* ptCmd = (Cmd::stQuestionUnionCityInfo*)rev;
          
        send.dwUserID = this->id;
        send.dwCountryID = this->scene->getCountryID();
        send.dwCityID = this->scene->getRealMapID();
        send.byType = ptCmd->byType;
        sessionClient->sendCmd(&send,sizeof(send));
        return true;
      }
      break;
    case Cmd::ENTER_UNION_CITY_AREA_PARA:
      {
        Cmd::Session::t_enterUnionManor_SceneSession send;
        send.dwUserID = this->id;
        send.dwCountryID = this->scene->getCountryID();
        send.dwCityID = this->scene->getRealMapID();

        if (this->charbase.unionid>0)
        {
          sessionClient->sendCmd(&send,sizeof(send));
        }
        else
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"����δ�����ᡣ���ܽ��������");
          return true;
        }
        
        return true;
      }
      break;
     case Cmd::UNION_STATUS_CHECK_PARA:
     case Cmd::CREATE_UNION_PARA:
	 //[Shx Delete ]
//       {
//         DWORD dwItemID = 0; // ���ڱ�����߶���id
//         bool bState = true; //�ȳ�ʼ���ɳɹ�״̬
// 
//         if (charbase.unionid == 0 && charbase.septid>0 && this->septMaster) 
//         {
//              if (charbase.level >= CREATE_UNION_NEED_LEVEL)
//               {
//                  UnionObjectCompare found;
//                  found.dwObjectID = CREATE_UNION_NEED_ITEM_ID;
//                  zObject *itemobj = packs.uom.getObject(found);// ����������
//                  
//                  if (itemobj)
//                  {
//                      if (packs.checkMoney(CREATE_UNION_NEED_PRICE_GOLD) /*&& packs.removeMoney(CREATE_UNION_NEED_PRICE_GOLD)*/) {
//                        //����ɹ�״̬
//                        bState = true;
//                        dwItemID = itemobj->data.qwThisID;  // ��¼������ߵĶ���id
//                      }
//                      else
//                      {
//                        //����û���㹻��Ǯ
//                        bState = false;
//                        Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"����Ҫ%u��Ǯ���������,��û���㹻��Ǯ",CREATE_UNION_NEED_PRICE_GOLD);
//                      }//dwItemID = itemobj->data.qwThisID;  // test
//                    }
//                    else 
//                    {
//                      //����û�е���
//                      bState = false;
//                      Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"ȱ��������� %s �޷��������","������");
//                    }
//              }
//  				else
//  				{
//  					//����ȼ�����
//  					bState = false;
//  					Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�ȼ�����%u���޷��������",CREATE_UNION_NEED_LEVEL);
//  				}
// 		}
// 
// 		//DWORD dwItemID = 0; // ���ڱ�����߶���id
// 		bool bState = true; //�ȳ�ʼ���ɳɹ�״̬
// 		if (charbase.unionid == 0 )		//���������Ǯ���ܵ��ϴ�. ^_^,û��ɶ��Ҫ���.
// 		{
// 			UnionObjectCompare found;
// 			found.dwObjectID = CREATE_UNION_NEED_ITEM_ID;	
// 
// 			if (packs.checkMoney(CREATE_UNION_NEED_PRICE_GOLD))
// 			{
// 				bState = true;	//����ɹ�״̬	
// 			}
// 			else
// 			{				
// 				bState = false;//����û���㹻��Ǯ
// 				Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"����Ҫ%u��Ǯ���������,��û���㹻��Ǯ",CREATE_UNION_NEED_PRICE_GOLD);
// 			}
//         }
//         else
//         {
//             bState = false;
//             Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,
//               "�Ѿ��������޷��ٴ����������峤");
//         }
//         
//         //���ذ�Ὠ��״̬
//         if (Cmd::CREATE_UNION_PARA == rev->byParam)
//         {
//           Cmd::stCreateUnionCmd *ptCmd=(Cmd::stCreateUnionCmd *)rev;
//           if (bState)
//           {
//             Cmd::Session::t_addUnion_SceneSession send;
//             
//             send.dwMapTempID = scene->tempid;
//             send.dwItemID    = dwItemID;
//             send.info.dwCountryID = this->charbase.country;
//             send.info.dwUnionID = 0;          // ����ID
//             send.info.dwMana = 0;
//             strncpy(send.info.name,ptCmd->UnionName,MAX_NAMESIZE);       // �������
//             send.info.dwCharID = charbase.id;                            // �᳤�Ľ�ɫID
//             strncpy(send.info.masterName,charbase.name,MAX_NAMESIZE);    // �᳤������
//             send.info.wdLevel = 1;                                        // ��ἶ��
//             send.info.qwExp = 0;                                        // ��ᾭ��
//             send.info.byVote = 0; //���ͶƱ��־
//             send.info.dwActionPoint = 0;
//             send.info.dwMoney = 0;
//             send.info.calltimes = 0;
//             sessionClient->sendCmd(&send,sizeof(send));
//             return true;
//           }
//         }
//         ////////////////////////////////////////
//         if (bState)
//         {
//             Cmd::stUnionStatusCheckCmd retCmd;
//             sendCmdToMe(&retCmd,sizeof(retCmd));
//         }
//         ///////////////////////////////////////
//         
//         return true;
//       }
       break;
    case Cmd::ADD_MEMBER_TO_UNION_PARA:
      {
        Cmd::stAddMemberToUnionCmd *ptCmd=(Cmd::stAddMemberToUnionCmd *)rev;
        switch(ptCmd->byState)
        {
          case Cmd::QUESTION:
            {
              if (0 == charbase.unionid)
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"������ȴ������������ճ�Ա");
                return true;
              }

              SceneUser *pUser=scene->getUserByName(ptCmd->memberName);
              if (NULL != pUser)
              {
                //if (scene->checkUserInNine(this,pUser))
                //{
                  if (!isset_state(pUser->sysSetting,Cmd::USER_SETTING_UNION))                                                            
                  {
                    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��� %s������δ����",pUser->name);                                                   
                      return true;
                  }  
                  
                  if (0==pUser->charbase.unionid
                     && pUser->charbase.septid>0
                     && pUser->septMaster
                      ) // Ϊ0��ʾδ������,�������峤
                  {
                    if (pUser->charbase.country == charbase.country)
                    {
                      strncpy(ptCmd->memberName,name,MAX_NAMESIZE);
                      ptCmd->memberID = id;
                      pUser->sendCmdToMe(ptCmd,sizeof(Cmd::stAddMemberToUnionCmd));
                      return true;
                    }
                    else
                    {
                      Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����㲻��һ�����Ҳ���������");
                    }
                  }
                  else
                  {
                    if (!pUser->septMaster)
                    {
                      Channel::sendSys(this,
                      Cmd::INFO_TYPE_FAIL,"ֻ�������峤");
                    }
                    else
                    {
                    if (pUser->charbase.unionid == charbase.unionid)
                      Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���Ѿ����뱾����,�����ٴ�����");
                    else
                      Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"���Ѿ������Ҫ���˻���ܼ�����İ��");
                    }
                  }
                //}
                //else
                //{
                //  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"����̫Զ�޷�����");
                //}
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
                //charbase.unionid = pUser->charbase.unionid;
                
              //  Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"%s�����������������˰��",name);
                pUser->removeWarRecord(Cmd::UNION_DARE);
                pUser->removeWarRecord(Cmd::UNION_CITY_DARE);
                pUser->sendNineToMe();
                pUser->sendMeToNine();

                Cmd::Session::t_addUnionMember_SceneSession send;
                send.member.dwSeptID = pUser->charbase.septid;
                send.dwUnionID = pUser->charbase.unionid;
                send.member.dwCharID = charbase.id;                                // ��Ա��ɫID
                send.member.wdOccupation = charbase.face;
                send.member.wdPower = 0;
                strncpy(send.member.name,charbase.name,MAX_NAMESIZE);             // ��Ա��ɫ����  
                strncpy(send.member.aliasname,DEFAULTMEMBERALIAS,MAX_NAMESIZE);   // ��Ա����
                SETMEMBERPOWER_WD(send.member.wdPower);                            // ��ԱȨ��
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
                Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"%s��Ը�������,�ܾ����������",name);
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

