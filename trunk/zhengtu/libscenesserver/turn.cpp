
/*
*\ brief:�����û�ת����ص���Ϣ
*\ auth: ��ε
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

			//���װ���Ƿ�ȥ����

			//fprintf(stderr,"level = %d",charbase.level);
			//fprintf(stderr,"maxlevel = %d",max_level);


			if(charbase.level < max_level)
			{
			    Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"�㻹û�дﵽ����ת���ĵȼ�!");
				return true;

			}

			if(!packs.equip.isEmpty())
			{
			    Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"��ȷ�����ϵ�װ���Ѿ�ȥ��!");
				//_stTurnResultCmd.turnSucceed = false;
				//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
				return true;

			}

			//����Ƿ���ְ��

			if(king || unionMaster || septMaster || emperor)
			{
				//_stTurnResultCmd.turnSucceed = false;
				//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
			Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"��ȷ�����ϵ�ְ���Ѿ��ǵ�!");
				return true;

			}

			//����session����ϵ

			Cmd::t_CheckRelationEmpty checkCmd;
			forwardSession(&checkCmd,sizeof(checkCmd));
	        //fprintf(stderr,"��ϵ�����Ϣ�Ѿ�����2\n");


			
	        //this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
		}
		break;
	case PARA_CHECKRELATION_RESULT:
		{



			Cmd::stTurnResultCmd _stTurnResultCmd;


			//������������Ĺ�ϵ�Ƿ�ȥ����
			Cmd::t_CheckRelationEmptyResult * checkCmd =  (Cmd::t_CheckRelationEmptyResult *)ptCmd;

			if(!checkCmd->isEmpty)
			{
				//_stTurnResultCmd.turnSucceed = false;
				//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
				
				Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"��ȷ������������Ĺ�ϵ�Ѿ�����ǵ�!");
				return true;

			}


			//������������

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
            
			//��������ֵ����������
			//bcopy(&horse.oData,&oH->data,sizeof(oH->data));

            //����������û�����
			/*if(!packs.addObject(oH,true,AUTO_PACK))
			{
				zObject::destroy(oH);
				fprintf(stderr,"���maʧ��\n");
				Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"��������ӵ�����ʧ��,��ȷ�������пռ���!");
				return true;
			}*/


			//���û�����
			horse.mount(false);
			//horse.horse((DWORD)0);

            //֪ͨ�û��յ�����
			//Cmd::stAddObjectPropertyUserCmd send;
            //bcopy(&oH->data,&send.object,sizeof(t_Object));
            //sendCmdToMe(&send,sizeof(send));
			//return true;

			//�ص����ִ� 
			//Gm::gomap_Gm(this,"name=���ִ� ");
			Gm::goHome(this,NULL);
			Gm::goTo_Gm(this,"120,93");

			//�������,�ظ�����ֵ,���ӵ���

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
		    Zebra::logger->error("�յ�ת����������Ϣ");

			//_stTurnResultCmd.turnSucceed = true;
			//this->sendCmdToMe(&_stTurnResultCmd,sizeof(_stTurnResultCmd));
			Channel::sendSys(this, Cmd::INFO_TYPE_FAIL,"��ϲ��ת���ɹ�!");
			return true;
		}
		break;
	default:
		break;
	}



	return true;






}



