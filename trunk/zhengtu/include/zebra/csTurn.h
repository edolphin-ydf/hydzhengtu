
#ifndef _CSTURN_H
#define _CSTURN_H
#include "csCommon.h"

#pragma pack(1)


/////////////////////////////////////////////////////////////
/// �ݶ�ת�����ָ��
////////////////////////////////////////////////////////////

namespace Cmd
{

	struct stTurnUserCmd : public stNullUserCmd
	{
		stTurnUserCmd()
		{
			byCmd = TURN_USERCMD;
		}
	};
    
	//����ת��
	#define SET_TRUN_REQUEST_PARAMETER 1
	struct stTurnRequestCmd : public stTurnUserCmd
	{
		stTurnRequestCmd()
		{
			byParam = SET_TRUN_REQUEST_PARAMETER;
		}
	};


	//enum stata{
	//	relationClear;


	//};
    
	//ת��������
	#define SET_TRUN_RESULT_PARAMETER 2
	struct  stTurnResultCmd : public stTurnUserCmd
	{
		//true�ɹ�;

		bool turnSucceed;

		stTurnResultCmd()
		{
			byParam = SET_TRUN_RESULT_PARAMETER;
		}
	};


		//�Ự�������򳡾����������ع�ϵ�����
	const BYTE PARA_CHECKRELATION_RESULT = 248;// 3
	struct t_CheckRelationEmptyResult : public t_NullCmd
	{
		DWORD dwUserID;//�û�ID
		bool isEmpty;
		//t_CheckRelationEmptyResult()
		//{
		//	byParam = PARA_CHECKRELATION_RESULT;
		//}
		t_CheckRelationEmptyResult()
			: t_NullCmd(Session::CMD_SCENE,PARA_CHECKRELATION_RESULT) {}
	};


    //������������Ự�������������û������ѹ�ϵ����Ĺ�ϵ�Ƿ�Ϊ��

	#define PARA_CHECKRELATION_EMPTY 4
	struct t_CheckRelationEmpty : public stTurnUserCmd
	{
		t_CheckRelationEmpty()
		{
			byParam = PARA_CHECKRELATION_EMPTY;
		}
			//: t_NullCmd(CMD_SCENE,PARA_CHECKRELATION_EMPTY) {}
	};


    //������������Ự�������������û������ѹ�ϵ����Ĺ�ϵ�Ƿ�Ϊ��

    //const BYTE PARA_CHECKRELATION_EMPTY 248;
	//struct t_CheckRelationEmpty : t_NullCmd
	//{

		//t_CheckRelationEmpty()
		//{
		//	byParam = PARA_CHECKRELATION_EMPTY;
		//}

	//	DWORD dwUserID;//�û�ID
	//	t_CheckRelationEmpty_SceneSession()
	//		: t_NullCmd(Cmd::Session::CMD_SCENE,PARA_CHECKRELATION_EMPTY) {}
	//};


}

#pragma pack()


#endif