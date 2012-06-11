#include <zebra/ScenesServer.h>


SceneGhost::SceneGhost(Scene* scene,zNpcB *npc,const t_NpcDefine *define,const SceneNpcType type,const SceneEntryType entrytype,zNpcB *abase)
:ScenePet(scene,npc,define,type,entrytype,abase)
{

}

void SceneGhost::full_t_UserData( Cmd::t_UserData &data, SceneUser * master )
{
	data.dwUserTempID=tempid;

	data.type = master->charbase.type;  
	data.face = master->charbase.face;
	data.goodness = master->charbase.goodness;

	if (!master->mask.is_masking() ) 
	{
		strncpy( data.name, master->name, MAX_NAMESIZE );
		data.country=master->charbase.country;
	}
	else 
	{
		strncpy(data.name,"蒙面人",MAX_NAMESIZE);
	}

  data.sculpt.dwHorseID=master->horse.horse();
  if (master->king) data.sculpt.dwHorseID = KING_HORSE_ID;
  if (master->emperor) data.sculpt.dwHorseID = EMPEROR_HORSE_ID;

  if (master->packs.equip.equip(EquipPack::HANDL))
  {
    data.sculpt.dwLeftHandID = master->packs.equip.equip(EquipPack::HANDL)->base->id;
    data.dwLeftWeaponColor = master->packs.equip.equip(EquipPack::HANDL)->data.color;
  }
  else
  {
    data.sculpt.dwLeftHandID=0;
    data.dwLeftWeaponColor=0;
  }
  if (master->packs.equip.equip(EquipPack::HANDR))
  {
    data.sculpt.dwRightHandID = master->packs.equip.equip(EquipPack::HANDR)->base->id;
    data.dwRightWeaponColor = master->packs.equip.equip(EquipPack::HANDR)->data.color;
  }
  else
  {
    data.sculpt.dwRightHandID=0;
    data.dwRightWeaponColor=0;
  }
  data.sculpt.dwHairID = master->getHairType();
  data.dwHairRGB=master->getHairColor();
  //如果有装备Custom颜色取物品颜色,System取道具表中颜色,否则Custom取人物属性随机后的color,系统色取0
  if (master->packs.equip.equip(EquipPack::OTHERS2)&& 
	  (master->packs.equip.equip(EquipPack::OTHERS2)->base->kind == ItemType_FashionBody || 
	  master->packs.equip.equip(EquipPack::OTHERS2)->base->kind == ItemType_HighFashionBody) )
  {
    data.sculpt.dwBodyID =master->packs.equip.equip(EquipPack::OTHERS2)->base->id;
    data.dwBodyColorCustom = master->packs.equip.equip(EquipPack::OTHERS2)->data.color;
  }
  else if (master->packs.equip.equip(EquipPack::OTHERS3)&& 
	  (master->packs.equip.equip(EquipPack::OTHERS3)->base->kind == ItemType_FashionBody || 
	  master->packs.equip.equip(EquipPack::OTHERS3)->base->kind == ItemType_HighFashionBody) )
  {
    data.sculpt.dwBodyID = master->packs.equip.equip(EquipPack::OTHERS3)->base->id;
    data.dwBodyColorCustom = master->packs.equip.equip(EquipPack::OTHERS3)->data.color;
  }
  else if (master->packs.equip.equip(EquipPack::BODY)) 
  {
    data.sculpt.dwBodyID = master->packs.equip.equip(EquipPack::BODY)->base->id;
    data.dwBodyColorCustom = master->packs.equip.equip(EquipPack::BODY)->data.color;
  }
  else 
  {
    data.dwBodyColorCustom = master->charbase.bodyColor;
    data.sculpt.dwBodyID=0;
  }

  data.attackspeed=(WORD)((((float)master->charstate.attackspeed)/640.0f)*100.0f);

  data.movespeed = master->getMyMoveSpeed();

  data.dwChangeFaceID = master->dwChangeFaceID;
  data.level = master->charbase.level;
 
  data.useJob = master->charbase.useJob;
  data.exploit = master->charbase.exploit;
  data.dwArmyState = master->dwArmyState;


  data.dwUnionID = master->charbase.unionid;
  data.dwSeptID = master->charbase.septid;

  if (master->king && !master->emperor)
  {
    strncpy(data.caption,(master->charbase.type==1)?"国王":"女王",sizeof(data.caption));
  }
  else if (master->emperor)
  {
    strncpy(data.caption,(master->charbase.type==1)?"皇帝":"女皇",sizeof(data.caption));
  }
  else if (master->kingConsort == 1)
  {
    strncpy(data.caption,(master->charbase.type==1)?"王夫":"王后",sizeof(data.caption));
  }
  else if (master->kingConsort == 2)
  {
    strncpy(data.caption,(master->charbase.type==1)?"皇夫":"皇后",sizeof(data.caption));
  }
  else if (master->isDiplomatState() == 0)
  {
    strncpy(data.caption,"外交官",sizeof(data.caption));
  }
  else if (master->isCatcherState())
  {
    strncpy(data.caption,"捕头",sizeof(data.caption));
  }
  else
  {
    strncpy(data.caption,master->caption,sizeof(data.caption));
  }
    
  data.dwTeamState=Cmd::TEAD_STATE_NONE;
}

void SceneGhost::full_zNpcB( SceneUser * master )
{
	petData.def		= (DWORD)((master->charstate.pdefence) * (30 / 100.0f));
	petData.mdef	= (DWORD)((master->charstate.mdefence) * (30 / 100.0f));
	petData.atk		= (DWORD)((master->charstate.pdamage) * (30 / 100.0f));
	petData.matk	= (DWORD)((master->charstate.mdamage) * (30 / 100.0f));
	petData.maxatk	= (DWORD)((master->charstate.maxpdamage) * (30 / 100.0f));
	petData.maxmatk	= (DWORD)((master->charstate.maxmdamage) * (30 / 100.0f));
	petData.maxhp	= (DWORD)((master->charstate.maxhp)	* (30 / 100.0f));
	petData.hp		= petData.maxhp;
	petData.maxhp_plus	= 0;
	petData.atk_plus	= 0;
	petData.maxatk_plus	= 0;
	petData.matk		= 0;
	petData.maxmatk_plus= 0;
	petData.pdef_plus	= 0;
	petData.mdef_plus	= 0;
}

void SceneGhost::full_t_MapUserDataPosState( Cmd::t_MapUserDataPosState &data, SceneUser * master )
{
  full_t_UserData(*((Cmd::t_UserData *)&data), master);
  data.byDir=getDir();
  data.x=getPos().x;
  data.y=getPos().y;
  data.num=full_UState(data.state);
}

void SceneGhost::full_t_MapUserData(Cmd::t_MapUserData &data, SceneUser * master )
{
  bzero(&data,sizeof(data));
  full_t_UserData(*((Cmd::t_UserData *)&data), master);
  full_all_UState(data.state,sizeof(data.state));
}