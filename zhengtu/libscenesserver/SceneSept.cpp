/**
 * \brief 实现家族命令的处理
 *
 */
#include <zebra/ScenesServer.h>


using namespace SeptDef;

/**
  * \brief 一个比较器
  *
  *  用于查找建立家族所需要的道具是否存在
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
  * \brief 处理用户家族命令
  *
  * 处理的关系命令如下:
  *
  * Cmd::SEPT_STATUS_CHECK_PARA
  *
  * Cmd::CREATE_SEPT_PARA
  *
  *  Cmd::ADD_MEMBER_TO_SEPT_PARA
  *
  * \param rev: 家族命令
  * \param cmdLen: 命令长度
  *
  * \return 命令被处理返回TRUE,否则为FALSE
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
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"您的荣誉点不够!");
        }

        if (ptCmd->dwHonor%10!=0)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"您的荣誉点不是5的倍数");
        }

        if (this->charbase.honor>ptCmd->dwHonor)
        {
          this->charbase.honor = this->charbase.honor - ptCmd->dwHonor;
        }
        else
        {
          this->charbase.honor = 0;
        }

        //通知客户端
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
        BYTE bState = 1; //先初始化成成功状态
        if (charbase.septid != 0)  // 家族ID大于0表示加入家族		
		{
			bState = 0;
			Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"已经加入家族无法再创建家族");
		}
		else
        {
          time_t cur_time = time(NULL);
          if (/*(cur_time - charbase.levelsept) < 24*60*60*/0)
          {
            //离开时间不足24小时
            bState = 0;
           // Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,
           //     "离开家族后24小时,才能再建立家族");
          }
          else 
		  {
			  if (packs.checkMoney(CREATE_SEPT_NEED_PRICE_GOLD) /*&& packs.removeMoney(CREATE_SEPT_NEED_PRICE_GOLD)*/) {
				  //报告成功状态
				  bState = 1;
			  }
			  else
			  {
				  //报告没有足够的钱
				  bState = 0;
				  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"你需要%u文钱来创建家族,你没有足够的钱",CREATE_SEPT_NEED_PRICE_GOLD);
			  }
          }
        }


        //返回家族建立状态
        if (Cmd::CREATE_SEPT_PARA == rev->byParam)
        {
          Cmd::stCreateSeptCmd *ptCmd=(Cmd::stCreateSeptCmd *)rev;
          if (1 == bState)
          {
            Cmd::Session::t_addSept_SceneSession send;
            
            send.dwMapTempID = scene->tempid;
            send.info.dwSeptID = 0;        // 家族的ID
            strncpy(send.info.name,ptCmd->SeptName,MAX_NAMESIZE);     // 家族名称
            send.info.dwCharID = charbase.id;    // 会长的角色ID
            send.info.byVote = 0;      // 默认不进入投票期间
            send.info.dwRepute = 0;
            send.info.dwUnionID = 0;
            send.info.dwLevel = 1;
            send.info.dwSpendGold = 0;
            send.info.dwIsExp = 0;
            send.info.calltimes = 0;
            send.info.calldaytime = 0;
            send.info.normalexptime = 0;
            strncpy(send.info.masterName,charbase.name,MAX_NAMESIZE);  // 会长的名字
            bzero(send.info.note,sizeof(send.info.note));
			strncpy( send.info.ExterData, "会长,FFFFFFFF;,0;,0;,0;,0;,0;,0;,0;,0;会员,0;", 512);
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
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"你必须先创立工会才能招收成员");
                return true;
              }
              SceneUser *pUser=scene->getUserByName(ptCmd->memberName);
              if (NULL != pUser)
              {
                //if (scene->checkUserInNine(this,pUser))
                //{
                  if (!isset_state(pUser->sysSetting,Cmd::USER_SETTING_FAMILY))                                                            
                  {
                    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"玩家 %s加入工会未开启",pUser->name);                                                   
                      return true;
                  }  
                  if (pUser->charbase.level >=JOIN_SEPT_NEED_LEVEL)
                  {
                    
//Shx Modify
//if (0==pUser->charbase.septid && pUser->charbase.unionid==0)// 为0表示未加入家族
 if (0==pUser->charbase.septid )
                    {
                      time_t cur_time = time(NULL);

//[Shx Delete 加盟时间间隔限制]
//                     if (((cur_time - pUser->charbase.levelsept) < 24*60*60)                                                                       
//                       {
//                         Channel::sendSys(this,
//         Cmd::INFO_TYPE_FAIL,"现在还不能邀请该玩家,该玩家还剩 %d 秒才能再次加入家族",
//         24*60*60-(cur_time-pUser->charbase.levelsept));
//                         return true;
// 
//                       }
//                       if((cur_time - pUser->charbase.levelsept) <  24*60*60)
//                       {
//                         Channel::sendSys(this,
//                             Cmd::INFO_TYPE_FAIL,"现在还不能邀请该玩家");
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
                        Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"他和你不是同一个服务器,你不能邀请他");
                      }
                    }
                    else
                    {
                      if (pUser->charbase.septid == charbase.septid)
                        Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"他已经加入了你的工会,无法再次邀请");
                      else
                        Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"他已加入其他工会");
                    }
                  }
                  else
                  {
                    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"等级低于%d级不能加入工会！",JOIN_SEPT_NEED_LEVEL);
                  }
              }
              else
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"玩家不在跟前,无法回应邀请");
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
                //Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"%s接受了你的邀请加入了家族",name);
                pUser->removeWarRecord(Cmd::SEPT_DARE);
                pUser->removeWarRecord(Cmd::SEPT_NPC_DARE);
    
				
				pUser->sendMeToNine();
                
                Cmd::Session::t_addSeptMember_SceneSession send;
                send.dwSeptID = pUser->charbase.septid;
                send.member.dwCharID = charbase.id;            // 会员角色ID
                send.member.wdOccupation = charbase.face;
				send.member.nRight = 9;		// Shx Add 默认权限为9 : 会员.	
                bzero(send.member.aliasname,sizeof(send.member.aliasname));
                strncpy(send.member.name,charbase.name,MAX_NAMESIZE);    // 会员角色名称  
                sessionClient->sendCmd(&send,sizeof(send));
              }
              else
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"对方已经离开,他放弃了这次邀请");
              }
              return true;
            }
            break;
          case Cmd::ANSWER_NO:
            {
              SceneUser *pUser=scene->getUserByID(ptCmd->memberID);
              if (pUser)
              {
                Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"%s不愿意加入家族,拒绝了你的邀请",name);
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

