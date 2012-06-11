#include <zebra/srvEngine.h>

template <class data,class datafile>
zDataBM<data,datafile> *zDataBM<data,datafile>::me(NULL);

zDataBM<zObjectB,ObjectBase> &objectbm=zDataBM<zObjectB,ObjectBase>::getMe();
zDataBM<zBlueObjectB,BlueObjectBase> &blueobjectbm=zDataBM<zBlueObjectB,BlueObjectBase>::getMe();
zDataBM<zGoldObjectB,GoldObjectBase> &goldobjectbm=zDataBM<zGoldObjectB,GoldObjectBase>::getMe();
zDataBM<zDropGoldObjectB,DropGoldObjectBase> &dropgoldobjectbm=zDataBM<zDropGoldObjectB,DropGoldObjectBase>::getMe();
zDataBM<zSetObjectB,SetObjectBase> &setobjectbm=zDataBM<zSetObjectB,SetObjectBase>::getMe();
zDataBM<zFiveSetB,FiveSetBase> &fivesetbm=zDataBM<zFiveSetB,FiveSetBase>::getMe();
zDataBM<zHolyObjectB,HolyObjectBase> &holyobjectbm=zDataBM<zHolyObjectB,HolyObjectBase>::getMe();
zDataBM<zUpgradeObjectB,UpgradeObjectBase> &upgradeobjectbm=zDataBM<zUpgradeObjectB,UpgradeObjectBase>::getMe();
zDataBM<zNpcB,NpcBase> &npcbm=zDataBM<zNpcB,NpcBase>::getMe();
//zDataBM<zCharacterB,CharacterBase> &characterbm = zDataBM<zCharacterB,CharacterBase>::getMe();
zDataBM<zExperienceB,ExperienceBase> &experiencebm = zDataBM<zExperienceB,ExperienceBase>::getMe();
zDataBM<zHonorB,HonorBase> &honorbm = zDataBM<zHonorB,HonorBase>::getMe();
zDataBM<zSkillB,SkillBase> &skillbm = zDataBM<zSkillB,SkillBase>::getMe();
zDataBM<zLiveSkillB,LiveSkillBase> &liveskillbm = zDataBM<zLiveSkillB,LiveSkillBase>::getMe();
zDataBM<zSoulStoneB,SoulStoneBase> &soulstonebm = zDataBM<zSoulStoneB,SoulStoneBase>::getMe();
zDataBM<zHairStyleB,HairStyle> &hairstylebm = zDataBM<zHairStyleB,HairStyle>::getMe();
zDataBM<zHairColourB,HairColour> &haircolourbm = zDataBM<zHairColourB,HairColour>::getMe();
zDataBM<zCountryMaterialB,CountryMaterial> &countrymaterialbm = zDataBM<zCountryMaterialB,CountryMaterial>::getMe();
zDataBM<zHeadListB,HeadList> &headlistbm = zDataBM<zHeadListB,HeadList>::getMe();
zDataBM<zPetB,PetBase> &petbm = zDataBM<zPetB,PetBase>::getMe();

bool loadAllBM()
{
  //Zebra::logger->debug("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  if (!objectbm.refresh((Zebra::global["datadir"] + "ObjectBase.tbl").c_str())) return false;
  //objectbm.listAll();
  if (!blueobjectbm.refresh((Zebra::global["datadir"] + "BlueObjectBase.tbl").c_str())) return false;
  //blueobjectbm.listAll();
  if (!goldobjectbm.refresh((Zebra::global["datadir"] + "GoldObjectBase.tbl").c_str())) return false;
  //goldobjectbm.listAll();
  if (!dropgoldobjectbm.refresh((Zebra::global["datadir"] + "DropGoldObjectBase.tbl").c_str())) return false;
  //dropgoldobjectbm.listAll();
  if (!setobjectbm.refresh((Zebra::global["datadir"] + "SetObjectBase.tbl").c_str())) return false;
  //setobjectbm.listAll();
  if (!fivesetbm.refresh((Zebra::global["datadir"] + "FiveSetBase.tbl").c_str())) return false;

  if (!holyobjectbm.refresh((Zebra::global["datadir"] + "HolyObjectBase.tbl").c_str())) return false;
  //holyobjectbm.listAll();
  if (!upgradeobjectbm.refresh((Zebra::global["datadir"] + "UpgradeObjectBase.tbl").c_str())) return false;
  //upgradeobjectbm.listAll();
  if (!npcbm.refresh((Zebra::global["datadir"] + "NpcBase.tbl").c_str())) return false;
  //npcbm.listAll();
  
  //if (!characterbm.refresh((Zebra::global["datadir"] + "CharacterBase.tbl").c_str())) return false;
  //characterbm.listAll();
  
  if (!experiencebm.refresh((Zebra::global["datadir"] + "ExperienceBase.tbl").c_str())) return false;
  if (!honorbm.refresh((Zebra::global["datadir"] + "HonorBase.tbl").c_str())) return false;

  if (!skillbm.refresh((Zebra::global["datadir"] + "SkillBase.tbl").c_str())) return false;
  
  if (!liveskillbm.refresh((Zebra::global["datadir"] + "WorkSkillBase.tbl").c_str())) return false;

  if (!soulstonebm.refresh((Zebra::global["datadir"] + "SoulStoneBase.tbl").c_str())) return false;
  if (!hairstylebm.refresh((Zebra::global["datadir"] + "HairStyle.tbl").c_str())) return false;
  if (!haircolourbm.refresh((Zebra::global["datadir"] + "HairColour.tbl").c_str())) return false;
  if (!countrymaterialbm.refresh((Zebra::global["datadir"] + "CountryMaterial.tbl").c_str())) return false;
  if (!headlistbm.refresh((Zebra::global["datadir"] + "HeadListBase.tbl").c_str())) return false;
  if (!petbm.refresh((Zebra::global["datadir"] + "PetBase.tbl").c_str())) return false;
    
  return true;
}

void unloadAllBM()
{
}

