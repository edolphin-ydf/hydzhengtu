
/*
*\ brief:处理用户转生相关的消息
*\ auth: 黄蔚
*\ date: 2008-02-29
*/


#include <zebra/ScenesServer.h>
#include <zebra/csBox.h>
#include <zebra/csTurn.h>
#include "boxCircle.h"


bool SceneUser::doTurnCmd(const Cmd::stTurnUserCmd *ptCmd,DWORD cmdLen)
{
    
    
	using namespace Cmd;
	switch(ptCmd->byParam)
	{
	case SET_TRUN_REQUEST_PARAMETER:
		{

			//Cmd::stTurnResultCmd _stTurnResultCmd;

			//检查装备是否都去掉了

			//fprintf(stderr,"level = %d",charbase.level);
			//fprintf(stderr,"maxlevel = %d",max_level);


			if(charbase.level < max_level)
			{
			    Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"你还没有达到可以转生的等级!");
				return true;

			}

			if(!packs.equip.isEmpty())
			{
			    Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"请确定身上的装备已经去掉!");
				//_stTurnResultCmd.turnSucceed = false;
				//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
				return true;

			}

			//检查是否有职务

			if(king || unionMaster || septMaster || emperor)
			{
				//_stTurnResultCmd.turnSucceed = false;
				//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
			Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"请确定身上的职务已经辞掉!");
				return true;

			}

			//请求session检查关系

			Cmd::t_CheckRelationEmpty checkCmd;
			forwardSession(&checkCmd,sizeof(checkCmd));
	        //fprintf(stderr,"关系检查消息已经发出2\n");


			
	        //this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
		}
		break;
	case PARA_CHECKRELATION_RESULT:
		{



			Cmd::stTurnResultCmd _stTurnResultCmd;


			//检查除好友以外的关系是否都去掉了
			Cmd::t_CheckRelationEmptyResult * checkCmd =  (Cmd::t_CheckRelationEmptyResult *)ptCmd;

			if(!checkCmd->isEmpty)
			{
				//_stTurnResultCmd.turnSucceed = false;
				//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
				
				Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"请确定除好友以外的关系已经清掉辞掉!");
				return true;

			}


			//生成马铠对象

		   /* horse.dwID;
			zObjectB *baseH = objectbm.get(horse.dwID);

			if(NULL == baseH)
			{
					return true;
			}

			zObject* oH = NULL;
			oH = zObject::create(baseH,1,1);

			if(NULL == oH)
			{
					return true;
			}*/
            
			//拷贝属性值到马铠对象
			//bcopy(&horse.oData,&oH->data,sizeof(oH->data));

            //添加马铠到用户包裹
			/*if(!packs.addObject(oH,true,AUTO_PACK))
			{
				zObject::destroy(oH);
				fprintf(stderr,"添加ma失败\n");
				Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"将马铠添加到包袱失败,请确保包袱有空间存放!");
				return true;
			}*/


			//让用户下马
			horse.mount(false);
			//horse.horse((DWORD)0);

            //通知用户收到马铠
			//Cmd::stAddObjectPropertyUserCmd send;
            //bcopy(&oH->data,&send.object,sizeof(t_Object));
            //sendCmdToMe(&send,sizeof(send));
			//return true;

			//回到新手村 
			//Gm::gomap_Gm(this,"name=新手村 ");
			Gm::goHome(this,NULL);
			Gm::goTo_Gm(this,"120,93");

			//清除技能,回复基础值,增加点数

			charbase.level = 1;

			

			int zs = charbase.zs;
			zs += 1;

			charbase.points = zs*trun_point_rate;

		    charbase.wdProperty[0]=0;
		    charbase.wdProperty[1]=0;
			charbase.wdProperty[2]=0;
			charbase.wdProperty[3]=0;
			charbase.wdProperty[4]=0;
			//this->setupCharBase();


			usm.clear();
			charbase.skillpoint = trun_skill_rate*zs;
            Cmd::stClearSkillUserCmd send1;
            sendCmdToMe(&send1,sizeof(send1));
            skillValue.init();

			setupCharBase();

			charbase.zs = zs;

			Cmd::stMainUserDataUserCmd  userinfo;
			full_t_MainUserData(userinfo.data);
			sendCmdToMe(&userinfo,sizeof(userinfo));







			Zebra::logger->error("zs = %d",this->charbase.zs);
		    Zebra::logger->error("收到转生结果检查消息");

			//_stTurnResultCmd.turnSucceed = true;
			//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
			Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"恭喜你转生成功!");
			return true;
		}
		break;
	default:
		break;
	}



	return true;






}



