
#ifndef  _CSBOX_H
#define  _CSBOX_H
#include "csCommon.h"

#pragma pack(1)


/////////////////////////////////////////////////////////////
/// �ݶ��������ָ��
////////////////////////////////////////////////////////////

namespace Cmd
{

struct stCowBoxUserCmd : public stNullUserCmd
{
	stCowBoxUserCmd()
	{
		byCmd = SAFETY_COWBOX;
	}
};


//�ͻ��˷���Կ�ױ������Ϣ
#define SET_COWBOX_KEY_PARAMETER 1
struct stSetCowBoxKeyCmd : public stCowBoxUserCmd
{
	stSetCowBoxKeyCmd()
	{
		byParam = SET_COWBOX_KEY_PARAMETER;
	}
	DWORD Key_id;
	DWORD qwThisID;
};
//����˷��ͱ��г�ʼ����Ϣ
#define GET_COWBOX_INIT_OPEN 2
struct stGetCowBoxInitCmd : public stCowBoxUserCmd
{
	stGetCowBoxInitCmd()
	{
		byParam = GET_COWBOX_INIT_OPEN;
	}
	//DWORD itemID[17];  //��ʱ��ID����ȡObjectBase_t���ݣ����ڸ��ݷ���˷��͵���Ʒ��������������
	t_Object objects[17];
	int item_Ti;       //����һ�ΰ�������ת����Ʒ�ĸ��������±��ʶ
};

//�û�������пؼ��� ��ȡ ����
#define SET_COWBOX_TIQU_ITEM 3
struct stSetCowBoxTiquCmd : public stCowBoxUserCmd
{
	stSetCowBoxTiquCmd()
	{
		byParam = SET_COWBOX_TIQU_ITEM;
	}
	bool item_id;
}; 


/////////////////////////////////////////////////////////////
/// ���屦�����ָ�����
//////////////////////////////////////////////////////////// 


struct stHotspringUserCmd : public stNullUserCmd    //��Ȫ��Ϣ
{
	stHotspringUserCmd()
	{
		byCmd = HOTSPRING_USERCMD;
	}
};

#define SET_TRUN_GOTOMALE_PARAMETER 1
struct stGoToMaleCmd : public stHotspringUserCmd   //��������Ȫ
{
	stGoToMaleCmd()
	{
		byParam = SET_TRUN_GOTOMALE_PARAMETER;
	}
};

#define SET_TRUN_GOTOFEMALE_PARAMETER 2
struct  stGoToFemaleCmd : public stHotspringUserCmd  //����Ů��Ȫ
{
	stGoToFemaleCmd()
	{
		byParam = SET_TRUN_GOTOFEMALE_PARAMETER;
	}
};

#define SET_TRUN_EXITHOTSPRING_PARAMETER 3
struct  stExitHotspringCmd : public stHotspringUserCmd  //�뿪��Ȫ
{
	stExitHotspringCmd()
	{
		byParam = SET_TRUN_EXITHOTSPRING_PARAMETER;
	}
};

/************************************************************************/
/* sky �´�����Ϣ���� begin                                               */
/************************************************************************/
// sky װ��������Ϣ
struct stReMakUserCmd : public stNullUserCmd
{
	stReMakUserCmd()
	{
		byCmd = REMAKEOBJECT_USERCMD;
	}
};

//sky �ͻ���֪ͨ����˴�����Ʒ��Ϣ
#define SET_NEW_MAKEOBJECT_USERCMD_ITEM 1
struct stNewMakeObjectUserCmd : public stReMakUserCmd
{
	stNewMakeObjectUserCmd()
	{
		byParam = SET_NEW_MAKEOBJECT_USERCMD_ITEM;
		dwID = 0;
		MakeLevel = 0;
		num = 0;
	}
	DWORD dwID;		//sky Ҫ�������ƷID
	BYTE MakeLevel; //sky ������Ʒ�ļ���(0��ͨ,1��ɫ,2��װ,4��װ,8��װ)
	BYTE num;		//sky ��������
};

//sky �����֪ͨ�ͻ��˴�����
#define SET_NEW_MAKEOBJECT_RETURN_USERCMD_ITEM 2
struct stNewMakeObjectReturnUserCmd : public stReMakUserCmd
{
	stNewMakeObjectReturnUserCmd()
	{
		byParam = SET_NEW_MAKEOBJECT_RETURN_USERCMD_ITEM;
		MakeType = 1;
	}
	BYTE MakeType;	//sky ��������(1:����)
	WORD returnNum; //sky ���ش�����(0:�ɹ� 1:Ǯ���� 2:���ϲ��� 3:�����ռ䲻�� 4:δ֪����)
};
/************************************************************************/
/*   sky ������Ϣ���� end                                               */
/************************************************************************/

////////////////////////////////////////////////////////////
/// ѵ��ָ��
////////////////////////////////////////////////////////////
struct stHorseUserCmd : public stNullUserCmd
{
	stHorseUserCmd()
	{
		byCmd = HORSETRAINING_USERCMD;
	}
};

//ս��ѵ��1		0
//ս��ѵ��2		1
//ս��ѵ��3		2
//���ѵ��		3

#define HORSETRAINING_USERCMD_TEY 1
struct stHorseTrainingUserCmd : public stHorseUserCmd
{
	stHorseTrainingUserCmd()
	{
		byParam = HORSETRAINING_USERCMD_TEY;
	}
	int CmdTey;
};
/////////////////////////////////////////////////////////////
////  ѵ��ָ��END
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
////  SKY ���ɼӵ�ָ��
/////////////////////////////////////////////////////////////
struct stAttruByteUserCmd : public stNullUserCmd
{
	stAttruByteUserCmd()
	{
		byCmd = SURPLUS_ATTRIBUTE_USERCMD;
	}
};


//�ӵ�
enum ATTRIBUTETYPE{
  PPT_STR,	// ����
  PPT_INTE,	// ����
  PPT_DEX,	// ����
  PPT_SPI,	// ����
  PPT_CON,	// ����
};

#define SURPLUS_ATTRIBUTE_USERCMD_ADD 1
struct stAddAttruByteUserCmd : public stAttruByteUserCmd
{
	stAddAttruByteUserCmd()
	{
		byParam = SURPLUS_ATTRIBUTE_USERCMD_ADD;
	}

	stObjectLocation pos;   //��Ʒ�ڰ����е�λ��
	BYTE Add_Type;  //��Ҫ�ӵ����������
	WORD Add_Num;	//�����������ֵ

};

//ϴ�� �ݲ�ʹ��

/////////////////////////////////////////////////////////////
////  ���ɼӵ�ָ�� END
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
////  sky ����Ƕָ��
/////////////////////////////////////////////////////////////

struct stMakeObjectUserCmd : public stNullUserCmd
{
	stMakeObjectUserCmd()
	{
		byCmd = MAKEOBJECT_USERCMD;
	}
};

#define SURPLUS_MOSAIGEM_USERCMD_ADD 1
struct stMosaicGemUserCmd : public stMakeObjectUserCmd
{
	stMosaicGemUserCmd()
	{
		byParam = SURPLUS_MOSAIGEM_USERCMD_ADD;
	}
	stObjectLocation	Epos;	//����Ƕ��װ���ڰ����е�λ��
	stObjectLocation	Gpos;	//Ҫ��Ƕ�ı�ʯ�ڰ����е�λ��
	int					index;	//Ҫ��Ƕ�Ŀ׵��±�	
};

/////////////////////////////////////////////////////////////
//// ����Ƕָ�� end
/////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//// sky NPC��ͼ�������Ϣ
////////////////////////////////////////////////////////////

//skyֱ�Ӵ���Ƕָ��������������ĸ�ͷ�ļ��ܱ�����鷳
#define RT_NPC_DIRITEM_USERCMD_PARA 2
struct stNpcDirItemUserCmd : public stMakeObjectUserCmd{

  stNpcDirItemUserCmd()
  {
    byParam = RT_NPC_DIRITEM_USERCMD_PARA;
  }

  DWORD dwNpcTempID;      /**< ʬ����ʱ��� */
};

#define RT_NPC_DIRITEM_DATA_USERCMD_PARA 3
struct stNpcDirItemDataUserCmd : public stMakeObjectUserCmd
{
	stNpcDirItemDataUserCmd()
	{
		byParam = RT_NPC_DIRITEM_DATA_USERCMD_PARA;
	}
	struct team
	{
		DWORD tempid;
		char name[MAX_NAMESIZE + 1];
	}fen_team[MAX_TEAM_NUM];

	DWORD		count; //��Ʒ����
	t_Object	objects[0];
}; 

//sky �ӳ��������Ա��Ʒ
#define RT_NPC_GIVEITEM_USERCMD 4
struct stNpcGiveItemUserCmd : public stMakeObjectUserCmd
{
	stNpcGiveItemUserCmd()
	{
		byParam = RT_NPC_GIVEITEM_USERCMD;
	}
	DWORD	UserTemID;	//�������˵�ΨһID
	DWORD	qwThisID;	//��������Ʒ��ΨһID
};

//sky �ͻ��˸�֪�������ҪROLL����Ʒ��ʰȡ����Ϣ
#define RT_TEAM_ROLL_ITEM_START 5
struct stTeamRollItemStartUserCmd : public stMakeObjectUserCmd
{
	stTeamRollItemStartUserCmd()
	{
		byParam = RT_TEAM_ROLL_ITEM_START;
	}
	DWORD itemX;	//Ҫ��ROLL����Ʒ�������X
	DWORD itemY;	//Ҫ��ROLL����Ʒ�������Y
};

//sky �����֪ͨ�ͻ��˶���������ԱROLL��Ʒ����Ϣ
#define RT_TEAM_ROLL_ITEM_NOTICE 6
struct stTeamRollItemNoticeUserCmd : public stMakeObjectUserCmd
{
	stTeamRollItemNoticeUserCmd()
	{
		byParam = RT_TEAM_ROLL_ITEM_NOTICE;
	}
	t_Object object;	//��ROLL����Ʒ������
};

//sky �ͻ��˸�֪��������ѡ���ROLL��ʽ��Ϣ
#define RT_TEAM_ROLL_ITEM_USERTYPE 7
struct stTeamRollItemTypeUserCmd : public stMakeObjectUserCmd
{
	stTeamRollItemTypeUserCmd()
	{
		byParam = RT_TEAM_ROLL_ITEM_USERTYPE;
	}
	BYTE	Rolltype;	//��ѡ�е�ROLLѡ��
};

//sky ������������Ϣ
#define RT_MAKE_TURRET_USERCMD 8
struct stMakeTurretUserCmd : public stMakeObjectUserCmd
{
	stMakeTurretUserCmd()
	{
		byParam = RT_MAKE_TURRET_USERCMD;
		dwThisID = 0;
		pos.x = 0;
		pos.y = 0;
	}

	DWORD dwThisID;		//��ʹ����Ʒ��ΨһID
	POINT pos;			//�����
};

//sky ����������Ϣ

//sky �ͻ���֪ͨ�����NPC������Ϣ
#define RT_NPC_START_CHANGE_USERCMD 9
struct stNpcStartChangeUserCmd : public stMakeObjectUserCmd
{
	stNpcStartChangeUserCmd()
	{
		byParam = RT_NPC_START_CHANGE_USERCMD;
		npcid = 0;
	}

	DWORD npcid;
};


//sky �����֪ͨ�ͻ���NPC����ɹ����¿ͻ��˸�NPC����Ϣ
#define RT_NPC_CHANGE_USERCMD 10
struct stNpcChangeUserCmd : public stMakeObjectUserCmd
{
	stNpcChangeUserCmd()
	{
		byParam = RT_NPC_CHANGE_USERCMD;
	}

	t_MapNpcData data;
};

//sky �����Ͳֿ⹺��ҳ����Ϣ
#define PACKE_TYPE 1
#define SAVEBOX_TYPE 2

#define PACK_BUYTAB_NUM_USERCMD 11
struct stPackBuyTanbNumUserCmd : public stMakeObjectUserCmd
{
	stPackBuyTanbNumUserCmd() 
	{ 
		byParam = PACK_BUYTAB_NUM_USERCMD;
		TabNum = 0;
		PackType = 0;
	}
	BYTE PackType;  //sky �������İ�������(PACKE_TYPE:���� SAVEBOX_TYPE:�ֿ�)
	BYTE TabNum;
};

//sky ��Ʒ��ȴ������Ϣ
#define ITEM_COOL_TIMES_USERCMD 12
struct stItemCoolTimesUserCmd : public stMakeObjectUserCmd
{
	stItemCoolTimesUserCmd(WORD Count = 0)
	{
		byParam = ITEM_COOL_TIMES_USERCMD;
		DataCount = Count;
	}
	WORD DataCount;  //���ݸ���
};

//sky ��Ʒʹ�óɹ�
#define ITEN_USEITEM_SUCCESS_USERCMD 13
struct stItemUseItemSuccessUserCmd : public stMakeObjectUserCmd
{
	stItemUseItemSuccessUserCmd()
	{
		byParam = ITEN_USEITEM_SUCCESS_USERCMD;
		ItemID = 0;
	}
	DWORD ItemID;
};

//lxb begin 08.12.09
//��������NPC��־  �ο�   ����\ͨѶЭ�鿪��Ŀ¼\�����.doc
#define TEAM_ALL_NPC_FLAG_USERCMD_PARA 28
struct stTeamAllNPCFlagUserCmd: public stChatUserCmd
{
	stTeamAllNPCFlagUserCmd()
	{
		byParam = TEAM_ALL_NPC_FLAG_USERCMD_PARA;
		memset( dwFlag, 0xff, sizeof( DWORD ) * 10 );
	}
	DWORD dwFlag[10];  //sky ���Ա�־���NPC��ΨһID
};

//���ö���NPC��־  �ο�   ����\ͨѶЭ�鿪��Ŀ¼\�����.doc
#define TEAM_SET_NPC_FLAG_USERCMD_PARA 29
struct stTeamSetNPCFlagUserCmd: public stChatUserCmd
{
	stTeamSetNPCFlagUserCmd()
	{
		byParam = TEAM_SET_NPC_FLAG_USERCMD_PARA;
		nFlag = -1;
		dwNPCID = INVALID_THISID;
	}

	int nFlag;    //sky �����õı�־
	DWORD dwNPCID; //sky ������NPC��ΨһID
};
//lxb end

////////////////////////////////////////////////////////////
//// NPC��ͼ�������Ϣ end
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//// sky ս��-����-���������ָ��
////////////////////////////////////////////////////////////
struct stArenaUserCmd : public stNullUserCmd
{
	stArenaUserCmd()
	{
		byCmd = ARENA_USERCMD;
	}
};

//sky �Ŷ���Ϣ
#define ARENA_QUEUING_USERCMD_PARA 1;
struct stArenaQueuingUserCmd : stArenaUserCmd
{
	DWORD UserID;			//sky �û�ID(typeΪ����ʱ���IDΪ����ΨһID)
	BYTE  Type;				//sky �Ŷ�����(0:���� 1:����)
	WORD  AddMeType;		//sky �Ŷӵ�ս������

	stArenaQueuingUserCmd()
	{
		byParam = ARENA_QUEUING_USERCMD_PARA;
		UserID = 0;
		Type = 0;
		AddMeType = 0;
	}
};
////////////////////////////////////////////////////////////
//// ս��-����-���������ָ�� end
////////////////////////////////////////////////////////////

}
#pragma pack()


#endif