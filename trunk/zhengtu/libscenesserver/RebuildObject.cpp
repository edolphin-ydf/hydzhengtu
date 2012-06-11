/**
 * \brief    ����Ʒ�ϳ�,����,����,���,��Ƕϵͳ
 * 
 */

#include <zebra/ScenesServer.h>
#include <zebra/csBox.h>
#include <math.h>

/**     
 * \brief NPC������֤
 *      
 * ��֤�û���NPC �Ķ����Ƿ�Ϸ�
 
 * \param user:�������û�
 * \param base: ��Ʒ������
 * \param action: ��������
 * \return ��֤ͨ������true,���򷵻�false
 */
bool Base::check_npc(SceneUser& user,zObjectB* base,int action)
{
  NpcTrade::NpcItem item;
  item.id = base->id;
  item.kind = base->kind;
  item.lowLevel = 0;
  item.level = base->needlevel;
  item.action = action;
  if (!NpcTrade::getInstance().verifyNpcAction(user.npc_dwNpcDataID,item) ) {
    return false;
  }
  
  return true;  
}

/**     
 * \brief �����ռ���֤
 *      
 * ��֤�û������е�ʣ��ռ��Ƿ�����Ҫ��
 
 * \param user:�������û�
 * \param w: ��Ҫ���
 * \param h: ��Ҫ�߶�
 * \return ��֤ͨ������true,���򷵻�false
 */
bool Base::check_space(SceneUser& user,DWORD w,DWORD h)
{  
  if (user.packs.main.space() < 1)  {
    Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"�����ռ䲻��");
    return false;
  }
  
  return true;  
}

/**     
 * \brief ɾ����Ʒ
 *      
 * ���û�������ɾ��һ����Ʒ
 
 * \param user:������û�
 * \param ob: �� ɾ����Ʒ
 * \return ��ǰ���Ƿ���true
 */
bool Base::remove_object(SceneUser& user,zObject* ob)
{  

  //fprintf(stderr,"����ɾ����Ʒ\n");
  Zebra::logger->error("����ɾ����Ʒ");
  user.packs.removeObject(ob); //notify and delete
  
  return true;  
}

/**     
 * \brief  ���Ͱ����������Ʒ����Ϣ���û�
 
 * \param user:������û�
 * \param ob: ��ӵ���Ʒ
 * \param add:��ӵ�����
 * \return ��ǰ���Ƿ���true
 */
bool Base::add_object(SceneUser& user,zObject* ob,bool add)
{
  //fprintf(stderr,"���������Ʒ\n");
  Zebra::logger->error("���������Ʒ");
  Cmd::stAddObjectPropertyUserCmd ret;
  if (add) {
    ret.byActionType = Cmd::EQUIPACTION_OBTAIN;
  }else {
    ret.byActionType = Cmd::EQUIPACTION_REFRESH;
  }
  bcopy(&ob->data,&ret.object,sizeof(t_Object),sizeof(ret.object));
  user.sendCmdToMe(&ret,sizeof(ret));
  
  return true;  
}

/**     
 * \brief sky ������Ʒ����Ľ�����û�
 *
 * \param user: ������û�
 * \param status: ������
 * \param type: ��������
 * \return ��ǰ���Ƿ���true
 */
bool Base::response(SceneUser& user,int status,BYTE type)
{
  //fprintf(stderr,"���ʹ�����\n");
  Zebra::logger->error("���ʹ�����");
  Cmd::stNewMakeObjectReturnUserCmd ret;
  ret.MakeType = type;
  ret.returnNum = status;
  user.sendCmdToMe(&ret,sizeof(ret));
  
  return true;
}

void Base::refresh_pack(SceneUser& user,ObjectPack* pack)
{
  zObject* tool = pack->object();
  if (!tool) return;
  --tool->data.dur;
  Cmd::stDurabilityUserCmd ret;
  ret.dwThisID = tool->data.qwThisID;
  ret.dwDur = tool->data.dur;
  ret.dwMaxDur = tool->data.maxdur;
  user.sendCmdToMe(&ret,sizeof(ret));
}

/**     
 * \brief ��֤��Ʒ�Ƿ��ܴ�� sky��ʱ����Ҫ���Կ��� ����ֱ�������ļ����úõ�
 *
 * \param ob: ����֤��Ʒ
 * \return ��֤�ɹ�����true,���򷵻�false
 */  
bool Hole::can_hole(zObject* ob)
{
	//if (ob->base->kind >= ItemType_ClothBody && ob->base->kind <=ItemType_Fing/*ItemType_Stick*/) {
	//	return true;;
	//}  

  return false;
  
}

/**     
 * \brief ȡ����Ʒ�Ͽ��ÿ� ������
 *
 * \param ob: ��Ʒ
 * \return ���ÿ׵�����
 */  
int Hole::get_empty_hole(zObject* ob)
{
  int hole = -1;

  //sky �¿��ǽṹ����ʹ���µķ�ʽ������ -1����������ǲ����Լ���
  while (hole<INVALID_INDEX && ob->data.Hsocket[++hole].GemID != INVALID_HOLE && ob->data.Hsocket[++hole].M_State);
  
  return hole;
}

/**     
 * \brief ȡ����Ʒ�����п� ������
 *
 * \param ob: ��Ʒ
 * \return ���п׵�����
 */  
int Hole::get_hole_num(zObject* ob)
{
  int hole = -1;
  //sky �¿��ǽṹ����ʹ���µķ�ʽ������
  while (hole<INVALID_INDEX && ob->data.Hsocket[++hole].GemID != INVALID_HOLE);
  if (hole==INVALID_INDEX) hole=INVALID_INDEX-1;
  return hole+1;
}

/**     
 * \brief ����Ʒ�����ӿ� sky ��ʱҲ����Ҫ��
 *
 * \param ob: ��Ʒ
 * \param num:���ӵ�����
 * \return ��Ʒ�Ͽ׵���Ŀ
 */  
int Hole::add_hole_num(zObject* ob,int num)
{
	/*int hole = Hole::get_hole_num(ob)-1;
	while (num-- > 0 && hole < Hole::INVALID_INDEX) {
	ob->data.socket[hole++] = (DWORD)-1;
	}

	return hole;*/

	return 0;
}

/**     
 * \brief ����Ʒ��ָ��λ������һ���� sky ��ʱҲ����Ҫ��
 *
 * \param ob: ��Ʒ
 * \param index:���ӿ� ��λ��
 * \return ���ӿ׳ɹ�����true,���򷵻�false
 */
bool Hole::add_hole(zObject* ob,int index)
{
	/*if (index <INVALID_INDEX && ob->data.socket[index] != (DWORD)-1) {
		ob->data.socket[index] = (DWORD) -1;
		return true;
	}*/

  return false;
}

/**     
* \brief sky ����ָ���Ŀ�
*
* \param ob: ��Ʒ
* \param index: Ҫ����׵�λ��
* \return ���óɹ�����true,���򷵻�false
*/
bool Hole::Activated_hole(zObject* ob, int index)
{
	if(ob->data.Hsocket[index].GemID != INVALID_HOLE)
	{
		ob->data.Hsocket[index].M_State = true;
		return true;
	}
	else
		return false;
}

/**     
 * \brief ����Ʒ��ָ���׷���һ������ sky ����û�л��ǵĸ��������ɱ�ʯ��
 *
 * \param ob: ��Ʒ
 * \param index:���� ��λ��
 * \param gam : ���õı�ʯ
 * \return ���óɹ�����true,���򷵻�false
 */
bool Hole::put_hole(zObject* ob,int index,zObject* gam)
{
	if (index <INVALID_INDEX && ob->data.Hsocket[index].GemID != INVALID_HOLE && ob->data.Hsocket[index].M_State )
	{
		memset( &(ob->data.Hsocket[index]), 0, sizeof(Gem_Pop) );

		ob->data.Hsocket[index].GemID	= gam->base->id;

		if(gam->data.str != 0)
		{
			ob->data.Hsocket[index].gem_str = gam->data.str;
		}

		if(gam->data.inte != 0)
		{
			ob->data.Hsocket[index].gem_inte = gam->data.inte;
		}

		if(gam->data.dex != 0)
		{
			ob->data.Hsocket[index].gem_dex = gam->data.dex;
		}

		if(gam->data.spi != 0)
		{
			ob->data.Hsocket[index].gem_spi = gam->data.spi;
		}

		if(gam->data.con != 0)
		{
			ob->data.Hsocket[index].gem_con = gam->data.con;
		}

		if(gam->data.pdefence != 0)
		{
			ob->data.Hsocket[index].gem_def = gam->data.pdefence;
		}

		if(gam->data.mdefence != 0)
		{
			ob->data.Hsocket[index].gem_mdef = gam->data.mdefence;
		}

		if(gam->data.damagebonus != 0)
		{
			ob->data.Hsocket[index].gem_dhpp = gam->data.damagebonus;
		}

		if(gam->base->maxpdamage != 0)
		{
			ob->data.Hsocket[index].gem_atk = gam->data.maxpdamage;
		}

		if(gam->data.maxmdamage != 0)
		{
			ob->data.Hsocket[index].gem_mkt = gam->data.maxmdamage;
		}
		return true;
	}
  return false;
}


const int SoulStone::_ids[] = {/*680,*/748,749,750,751};

/**     
 * \brief ѡ�����ʯ
 *
 * �����л���ʯ�����ѡ��һ��
 *
 * \return ����ʯid
 */
int SoulStone::id(DWORD trait)
{
  int index = randBetween(0,4);
  switch(trait)
  {
    case 1:
    case 2:
    case 3:
    case 11:
    case 12:
    case 23:
    case 24:
      {
        index=0;
      }
      break;
    case 4:
    case 5:
    case 13:
    case 14:
    case 19:
    case 25:
    case 26:
      {
        index=1;
      }
      break;
    case 6:
    case 7:
    case 15:
    case 16:
    case 20:
    case 21:
    case 22:
      {
        index=2;
      }
      break;
    case 8:
    case 9:
    case 10:
    case 17:
    case 18:
    case 27:
    case 28:
      {
        index=3;
      }
      break;

    default:
      break;
  };
  return _ids[index];
}

/**
 * \brief �Ƿ��������
 * \param value : ����ֵ
 * \return �������Է���true,���򷵻�false
 */
template <typename T>
bool EXIST_PROP(T& value)
{
  if (value.min || value.max) return true;

  return false;
}

#define PROCESS_PROP(x) \
        if (EXIST_PROP(soul->x)) { \
          ob->data.x += randBetween(soul->x.min,soul->x.max); \
        }

bool SoulStone::assign(zObject* ob,int monster)
{
  zSoulStoneB *soul = soulstonebm.get(monster);
  if (soul==NULL) return false;

  ob->data.needlevel = soul->level;
  if (EXIST_PROP(soul->hpleech.odds))  {
    ob->data.hpleech.odds += randBetween(soul->hpleech.odds.min,soul->hpleech.odds.max);
    ob->data.hpleech.effect += randBetween(soul->hpleech.effect.min,soul->hpleech.effect.max);
  }
  if (EXIST_PROP(soul->mpleech.odds))  {
    ob->data.mpleech.odds += randBetween(soul->mpleech.odds.min,soul->mpleech.odds.max);
    ob->data.mpleech.effect += randBetween(soul->mpleech.effect.min,soul->mpleech.effect.max);
  }
  PROCESS_PROP( hptomp ) //ת������ֵΪ����ֵx��
    PROCESS_PROP( incgold ) //���ӽ�Ǯ����x%
    PROCESS_PROP( doublexp ) //x%˫������    
    PROCESS_PROP( mf ) //���ӵ�����x%
    PROCESS_PROP( poisondef ) //��������
    PROCESS_PROP( lulldef ) //���������
    PROCESS_PROP( reeldef ) //��ѣ������
    PROCESS_PROP( evildef ) //����ħ����
    PROCESS_PROP( bitedef ) //����������
    PROCESS_PROP( chaosdef ) //����������
    PROCESS_PROP( colddef ) //����������
    PROCESS_PROP( petrifydef ) //��ʯ������
    PROCESS_PROP( blinddef ) //��ʧ������
    PROCESS_PROP( stabledef ) //����������
    PROCESS_PROP( slowdef ) //����������
    PROCESS_PROP( luredef ) //���ջ�����
    PROCESS_PROP( poison ) //�ж�����
    PROCESS_PROP( lull ) //�������
    PROCESS_PROP( reel ) //ѣ������
    PROCESS_PROP( evil ) //��ħ����
    PROCESS_PROP( bite ) //��������
    PROCESS_PROP( chaos ) //��������
    PROCESS_PROP( cold ) //��������
    PROCESS_PROP( petrify ) //ʯ������
    PROCESS_PROP( blind ) //ʧ������
    PROCESS_PROP( stable ) //��������
    PROCESS_PROP( slow ) //��������
    PROCESS_PROP( lure ) //�ջ�����
    PROCESS_PROP( str ) 
    PROCESS_PROP( inte ) 
    PROCESS_PROP( dex ) 
    PROCESS_PROP( spi ) 
    PROCESS_PROP( con ) 


    /**
     * \brief �����������ְ�,��Ĵ��붼�����˰� nnd
     */
    /*
       do {
       switch (prop % (30+5) ) 
       {
       case 0:
       if (EXIST_PROP(soul->hpleech.odds))  {
       ob->data.hpleech.odds += randBetween(soul->hpleech.odds.min,soul->hpleech.odds.max);
       ob->data.hpleech.effect += randBetween(soul->hpleech.effect.min,soul->hpleech.effect.max);
       done = true;
       }
       break;
       case 1:
       if (EXIST_PROP(soul->hpleech.odds))  {
       ob->data.mpleech.odds += randBetween(soul->mpleech.odds.min,soul->mpleech.odds.max);
       ob->data.mpleech.effect += randBetween(soul->mpleech.effect.min,soul->mpleech.effect.max);
       done = true;
       }
       break;

       case 2:
       PROCESS_PROP( hptomp ) //ת������ֵΪ����ֵx��

       case 3:
       PROCESS_PROP( incgold ) //���ӽ�Ǯ����x%
       case 4:
       PROCESS_PROP( doublexp ) //x%˫������    
       case 5:
       PROCESS_PROP( mf ) //���ӵ�����x%

       case 6:
       PROCESS_PROP( poisondef ) //��������
       case 7:
       PROCESS_PROP( lulldef ) //���������
       case 8:
       PROCESS_PROP( reeldef ) //��ѣ������
       case 9:
       PROCESS_PROP( evildef ) //����ħ����
       case 10:
       PROCESS_PROP( bitedef ) //����������
       case 11:
       PROCESS_PROP( chaosdef ) //����������
       case 12:
       PROCESS_PROP( colddef ) //����������
       case 13:
       PROCESS_PROP( petrifydef ) //��ʯ������
       case 14:
       PROCESS_PROP( blinddef ) //��ʧ������
       case 15:
       PROCESS_PROP( stabledef ) //����������
       case 16:
       PROCESS_PROP( slowdef ) //����������
       case 17:
       PROCESS_PROP( luredef ) //���ջ�����

       case 18:
       PROCESS_PROP( poison ) //�ж�����
       case 19:
       PROCESS_PROP( lull ) //�������
       case 20:
       PROCESS_PROP( reel ) //ѣ������
       case 21:
       PROCESS_PROP( evil ) //��ħ����
       case 22:
       PROCESS_PROP( bite ) //��������
       case 23:
       PROCESS_PROP( chaos ) //��������
       case 24:
       PROCESS_PROP( cold ) //��������
       case 25:
       PROCESS_PROP( petrify ) //ʯ������
       case 26:
       PROCESS_PROP( blind ) //ʧ������
  case 27:
    PROCESS_PROP( stable ) //��������
  case 28:
      PROCESS_PROP( slow ) //��������
  case 29:
        PROCESS_PROP( lure ) //�ջ�����
  case 30:
          PROCESS_PROP( str ) 
  case 31:
            PROCESS_PROP( inte ) 
  case 32:
              PROCESS_PROP( dex ) 
  case 33:
                PROCESS_PROP( spi ) 
  case 34:
                  PROCESS_PROP( con ) 

}

}while (!done && ((++prop % (30+5) ) != old));
// */

return true;
}

/**     
 * \brief ����ʯ�ϳ�
 *
 * \param user: ������û�
 * \param first: ��һ�����ʯ
 * \param second:�ڶ������ʯ
 * \param odds:�ϳɼ���
 * \return �ϳɳɹ�����true,���򷵻�false
 */
zObject* SoulStone::compose(SceneUser& user,zObject* first,zObject* second,int odds)
{  
  if (!selectByPercent(odds) ) return false;
  
  /**
   * \brief �ı���ǶͼƬѡ��ʽ
   * whj
   *
   */
  zObjectB *base = objectbm.get(id(1));
  if (!base) return false;
  
  int level = max(first->data.upgrade,second->data.upgrade);
  
  zObject *ob = zObject::create(base,1,++level);
  if (ob) {
    zObject::logger(ob->createid,ob->data.qwThisID,ob->data.strName,ob->data.dwNum,ob->data.dwNum,1,0,NULL,user.id,user.name,"���Ǻϳ�",ob->base,ob->data.kind,ob->data.upgrade);
    do_compose(first,second,ob);
    
    return ob;
  }

  return NULL;  
}

/**     
 * \brief ����ʯ��Ƕ
 *
 * \param user: ������û�
 * \param dest: ����Ƕ��Ʒ
 * \param src:������Ƕ�Ļ���ʯ
 * \return ��ǰ���Ƿ���true
 */
bool SoulStone::enchance(SceneUser& user,zObject* dest,zObject* src)
{
  do_enchance(dest,src);

  return true;
}

#define COMPUTE(x) dest->data.x += src->data.x;

/**     
 * \brief ����ʯ��Ƕ
 *
 * \param dest: ����Ƕ��Ʒ
 * \param src:������Ƕ�Ļ���ʯ
 * \return ��ǰ���Ƿ���true
 */
bool SoulStone::do_enchance(zObject* dest,zObject* src)
{
  dest->data.needlevel = max(dest->data.needlevel,src->data.needlevel);
  
  //sprintf(dest->data.strName,"��Ƕ�˻��ǵ�%s",dest->base->name);
  COMPUTE( hpleech.odds )
  COMPUTE( hpleech.effect ) //x%��������ֵy,
  COMPUTE( mpleech.odds )
  COMPUTE( mpleech.effect ) // x%���շ���ֵy
  
  COMPUTE( hptomp ) //ת������ֵΪ����ֵx��

  COMPUTE( incgold ) //���ӽ�Ǯ����x%
  COMPUTE( doublexp ) //x%˫������    
  COMPUTE( mf ) //���ӵ�����x%
  
  COMPUTE( poisondef ) //��������
  COMPUTE( lulldef ) //���������
  COMPUTE( reeldef ) //��ѣ������
  COMPUTE( evildef ) //����ħ����
  COMPUTE( bitedef ) //����������
  COMPUTE( chaosdef ) //����������
  COMPUTE( colddef ) //����������
  COMPUTE( petrifydef ) //��ʯ������
  COMPUTE( blinddef ) //��ʧ������
  COMPUTE( stabledef ) //����������
  COMPUTE( slowdef ) //����������
  COMPUTE( luredef ) //���ջ�����

  COMPUTE( poison ) //�ж�����
  COMPUTE( lull ) //�������
  COMPUTE( reel ) //ѣ������
  COMPUTE( evil ) //��ħ����
  COMPUTE( bite ) //��������
  COMPUTE( chaos ) //��������
  COMPUTE( cold ) //��������
  COMPUTE( petrify ) //ʯ������
  COMPUTE( blind ) //ʧ������
  COMPUTE( stable ) //��������
  COMPUTE( slow ) //��������
  COMPUTE( lure ) //�ջ�����

  COMPUTE( str )
  COMPUTE( inte ) 
  COMPUTE( dex ) 
  COMPUTE( spi )
  COMPUTE( con )
  
  return true;
}

#undef COMPUTE
#define COMPUTE(x) additive(first->data.x,second->data.x,dest->data.x,level);
/**     
 * \brief ����ʯ�ϳ�
 *
 * \param first: ��һ�����ʯ
 * \param second:�ڶ������ʯ
 * \param dest:�����ɵĻ���ʯ
 * \param odds:�ϳɼ���
 * \return �ϳɳɹ�����true,���򷵻�false
 */
bool SoulStone::do_compose(zObject* first,zObject* second,zObject* dest)
{
  int level = ( (first->data.upgrade & 0xff) << 8 ) | (second->data.upgrade & 0xff);

  dest->data.needlevel = max(first->data.needlevel,second->data.needlevel);
  
  COMPUTE( hpleech.odds )
  COMPUTE( hpleech.effect ) //x%��������ֵy,
  COMPUTE( mpleech.odds )
  COMPUTE( mpleech.effect ) // x%���շ���ֵy
  
  COMPUTE( hptomp ) //ת������ֵΪ����ֵx��

  COMPUTE( incgold ) //���ӽ�Ǯ����x%
  COMPUTE( doublexp ) //x%˫������    
  COMPUTE( mf ) //���ӵ�����x%
  
  COMPUTE( poisondef ) //��������
  COMPUTE( lulldef ) //���������
  COMPUTE( reeldef ) //��ѣ������
  COMPUTE( evildef ) //����ħ����
  COMPUTE( bitedef ) //����������
  COMPUTE( chaosdef ) //����������
  COMPUTE( colddef ) //����������
  COMPUTE( petrifydef ) //��ʯ������
  COMPUTE( blinddef ) //��ʧ������
  COMPUTE( stabledef ) //����������
  COMPUTE( slowdef ) //����������
  COMPUTE( luredef ) //���ջ�����

  COMPUTE( poison ) //�ж�����
  COMPUTE( lull ) //�������
  COMPUTE( reel ) //ѣ������
  COMPUTE( evil ) //��ħ����
  COMPUTE( bite ) //��������
  COMPUTE( chaos ) //��������
  COMPUTE( cold ) //��������
  COMPUTE( petrify ) //ʯ������
  COMPUTE( blind ) //ʧ������
  COMPUTE( stable ) //��������
  COMPUTE( slow ) //��������
  COMPUTE( lure ) //�ջ�����

  COMPUTE( str )
  COMPUTE( inte ) 
  COMPUTE( dex ) 
  COMPUTE( spi )
  COMPUTE( con )
  
  return true;
}

/**     
 * \brief ������Ʒ
 *
 * \param user:������Ʒ���û�
 * \param ob: ������Ʒ
 * \param extra_odds: ����ɹ���
 * \return �����ɹ�����true,ʧ�ܷ���false
 */
bool Upgrade::upgrade(SceneUser& user,zObject* ob,int extra_odds)
{
  //free ��ֹ��������
//  Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"��������ϵͳ���ڿ� ���У�");
//  return true;

//#if 0\
  
  //fprintf(stderr,"������Ʒ\n");
   
  zUpgradeObjectB *base = upgradeobjectbm.get(ob->data.dwObjectID+100000*(ob->data.upgrade+1));
  if (!base) return false;
  
  Zebra::logger->error("�����ɹ�����Ϊ%d",base->odds + extra_odds);
  if (selectByPercent(base->odds + extra_odds)) {
	Zebra::logger->error("׼������\n");
    do_upgrade(ob,base);
    return true;
  }else {
    //8����9��ʧ�ܽ���0��,����������ǽ�2��
    int down = ob->data.upgrade >= 8?ob->data.upgrade:2;
    for( int i=0;i<down;i++)
    {
      base = upgradeobjectbm.get(ob->data.dwObjectID+100000*(ob->data.upgrade));
      if (base) do_downgrade(ob,base);
    }
    return false;
  }
//#endif
}

#undef COMPUTE
#define COMPUTE(x) ob->data.x += base->x;
/**     
 * \brief ������Ʒ�ȼ�
 *
 * \param ob: ������Ʒ
 * \param base: ��������
 * \return ��ǰ���Ƿ���true
 */
bool Upgrade::do_upgrade(zObject* ob,zUpgradeObjectB* base)
{
  ++ob->data.upgrade;
  ++ob->data.needlevel;
  
  COMPUTE( pdamage )        // ��С������
  COMPUTE( maxpdamage )      // ��󹥻���
  COMPUTE( mdamage )      // ��С����������
  COMPUTE( maxmdamage )      // �����������

  COMPUTE( pdefence )      // ���
  COMPUTE( mdefence )        // ħ��
  COMPUTE( maxhp )      // �������ֵ
  
  return true;
}

#undef COMPUTE
#define COMPUTE(x) ob->data.x -= base->x; \
if ((SWORD)ob->data.x < 0) ob->data.x = 0; //������,����Ҫ��Ԥ��,�����Ĵ���,�׳յĲ߻�,���εĳ���


/**     
 * \brief ������Ʒ�ȼ�
 *
 * \param ob: ������Ʒ
 * \param base: ��������
 * \return ��ǰ���Ƿ���true
 */
bool Upgrade::do_downgrade(zObject* ob,zUpgradeObjectB* base)
{

  if (ob->data.upgrade > 0) {
    --ob->data.upgrade;  
    --ob->data.needlevel;
    
    COMPUTE( pdamage )        // ��С������
    COMPUTE( maxpdamage )      // ��󹥻���
    COMPUTE( mdamage )      // ��С����������
    COMPUTE( maxmdamage )      // �����������
  
    COMPUTE( pdefence )      // ���
    COMPUTE( mdefence )        // ħ��
    COMPUTE( maxhp )      // �������ֵ
  }
  
  return true;  
}



const int Decompose::_odds[] = 
      {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,//white 0
           100, 80, 50, 2,   1,   1,   0,   0, 0,   0,//blue  1
           100, 80, 50, 5,   2,   1,   1,   1, 0,   0,//gold  2
           100, 100, 60, 30,   10,   5,   2,   1, 1,   1  //holy  3
         };

const int Decompose::_items[] = {540,548,556,562,576,577,662,663,664,666,667,668,669,/*670,671,672,673,674,675,676,*/ /*removed (TASK 731)*/677,678,679,681,683,655,685,686,0};

/**     
 * \brief ȡ�÷ֽ����Ʒ����
 *
 * \return �ֽ����Ʒ����
 */
int Decompose::index() const
{
  if (_ob->data.kind & 4) { //holy
     return 3;
  }
  if (_ob->data.kind & 2) { //gold
     return 2;
  }
  if (_ob->data.kind & 1) { //blue
     return 1;
  }

  return 0;
}

/**     
 * \brief ȡ�÷ֽ���Ʒ���ԭ�ϵĸ���
 *
 * \return ���ԭ�ϸ���
 */
int Decompose::chance() const
{
  double coefficient = 0;
  
  if (_ob->data.kind & 4) { //holy
    coefficient = 10;
  }else if (_ob->data.kind & 2) { //gold
     coefficient = 1;
  }else if (_ob->data.kind & 1) { //blue
     coefficient = 0.5;
  }
  
  int level = _ob->data.needlevel/20 + 1;
  return static_cast<int>(coefficient*level);
}

/**     
 * \brief �ֽ���Ʒ��Ҫ�Ľ�Ǯ����
 *
 * \return ��Ҫ��Ǯ����
 */
int Decompose::gold() const
{
  double coefficient = 0;
  
  if (_ob->data.kind & 4) { //holy
    coefficient = 20;
  }else if (_ob->data.kind & 2) { //gold
     coefficient = 10;
  }else if (_ob->data.kind & 1) { //blue
     coefficient = 5;
  }
  
  int level = _ob->data.needlevel/20 + 1;
  return static_cast<int>(coefficient*level);
}

/**     
 * \brief ����������ֽ���Ʒ���û�
 *
 * \param user: �ֽ���Ʒ���û�
 * \return ��ǰ���Ƿ���true
 */
bool Decompose::bonus_exp(SceneUser& user)
{
  int coefficient = 0;
  
  if (_ob->data.kind & 4) { //holy
    coefficient = 200;
  }else if (_ob->data.kind & 2) { //gold
     coefficient = 50;
  }else if (_ob->data.kind & 1) { //blue
     coefficient = 20;
  }
  
  int level = _ob->data.needlevel/20 + 1;
  int exp = 2*coefficient*level;

  if (_ob->data.exp) exp += _ob->data.exp;

  //add exp bonus  
  user.addExp(exp);
  /*
  user.charbase.exp += exp;
  ScenePk::attackRTExp(&user,exp);
  if (user.charbase.exp >= user.charstate.nextexp) { 
    user.upgrade();
  }
  */
  
  return true;
}

/**     
 * \brief ����ԭ�ϸ��ֽ���Ʒ���û�
 *
 * \param user: �ֽ���Ʒ���û�
 * \return ��ǰ���Ƿ���true
 */
bool Decompose::bonus_items(SceneUser& user)
{
/*
  zObjectB::material::stuffs_iterator it = _ob->base->need_material.stuffs.begin();
  int kind = index();
  
  int count = 0;  
  int items[_ob->base->need_material.stuffs.size()*(1+10)];
  memset(items,0,sizeof(items));
  
  int i = 0;
  int max_level = 0;
  while (it != _ob->base->need_material.stuffs.end()) {
    count += it->number;
    items[i*(10+1)] = it->id;
    if (!max_level) max_level = it->level;
    if (max_level > it->level) max_level = it->level;
    ++i;
    ++it;
  }

  count = randBetween(1,count/2);
  
  do {
    int which = randBetween(0,_ob->base->need_material.stuffs.size()-1);
    
//    for (int level=9; level>=0; --level) {        
    for (int level=max_level; level>=0; --level) {        
      if (selectByPercent(_odds[kind*10+level])) {
        ++items[which*(10+1)+1+level];
        break;
      }
    }
    
  }while (--count>0);

  for (int which=0; which<(int)_ob->base->need_material.stuffs.size(); ++which) {
    for (int level=0; level<10; ++level) {
      if (items[which*(10+1)+1+level] && user.addObjectNum(items[which*(10+1)],items[which*(10+1)+1+level],level,Cmd::INFO_TYPE_GAME) == 1) {
        //package full
        return true;
      }      
    }
  }
*/
  int kind = index();
  if (kind) {
    zObjectB::material::stuffs_iterator it = _ob->base->need_material.stuffs.begin();
    
    while (it != _ob->base->need_material.stuffs.end()) {
      int count =  randBetween(1,it->number / 2);
      if (index() == 3 && _ob->data.maker[0] && _ob->data.bind)
      {
        count =  randBetween(1,it->number);
        if (strstr(_ob->data.strName,"������") != NULL)
        {
          count = it->number/2;
        }
      }
      while (count -- > 0) {
        int level = 0;
        if (index() >= 2 && selectByPercent(20))
        {
          level = randBetween(1,index()-1);
        }
        if (index() == 3 && _ob->data.maker[0] && _ob->data.bind)
        {
          level = 2;
          if (strstr(_ob->data.strName,"������") != NULL)
          {
            level = 3;
          }
        }

        
        /*
        if (level > 1) {
          //CPUӦ�ÿ���,NB CEHUA
          zObjectB* ob = objectbm.get(it->id);
          if (ob && ob->level == 2) level = 1;
        }
        // */
        if (user.addObjectNum(it->id,1,level,Cmd::INFO_TYPE_GAME) == 1) {
          //package full
          return true;
        }      
      }
      ++it;
    }
  }

/*
  int i = 0;
  int odds = chance();
  do {
    if (selectByTenTh(odds)) {
      zObjectB* o = objectbm.get(_items[i]);
      int upgrade = 0;
      if (o && o->recastlevel) {
        for (int level=9; level>=0; --level) {        
          if (selectByPercent(_odds[kind*10+level])) {
            upgrade = level;
            break;    
          }
        }
      }
      
      if (user.addObjectNum(_items[i],1,upgrade,Cmd::INFO_TYPE_GAME) == 1) {
        //package is ful
        return true;
      }
    }
    
    
  }while (_items[++i] != 0);
*/
  return true;  
}

/**     
 * \brief ɾ�����ֽ���Ʒ
 *
 * ���û�������ɾ�����ֽ����Ʒ
 *
 * \param user: �ֽ���Ʒ���û�
 * \return ��ǰ���Ƿ���true
 */
bool Decompose::remove_from(SceneUser& user)
{
  return Base::remove_object(user,_ob);
}

/**     
 * \brief ���Ӽ��ܵ�
 *
 * ����Ʒ��Ҫ�ȼ������û����켼�ܵļ��ܵ�
 *
 * \param user: ������û�
 * \param ob: �������Ʒ
 * \return ��ǰ���Ƿ���true
 */
bool EquipMaker::add_skill(SceneUser& user,zObjectB* ob)
{
  return true;
}

/**     
 * \brief ���Ӵ����û��ľ���ֵ
 *
 * \param user: ������û�
 * \param coefficient: ����ϵ��
 * \return ��ǰ���Ƿ���true
 */
bool EquipMaker::add_exp(SceneUser& user,DWORD coefficient)
{
  DWORD exp = (DWORD)_odds.material_level*coefficient*_need;
  if (exp < 10) exp = 10;
  //add exp bonus  
  user.addExp(exp);

  return true;
}

/**     
 * \brief ���ܼ��
 *
 * ����û��ļ����Ƿ�����������ƷҪ��
 *
 * \param user: ������û�
 * \param ob: �������Ʒ
 * \return ����Ҫ�󷵻�true,���򷵻�false
 */
bool EquipMaker::check_skill(SceneUser& user,zObjectB* ob)
{
  int id = ob->need_skill.id;
  if (!id) return false;
  int level = ob->need_skill.level;
  
  _odds.skill_level = level>1?-1:1;

  return true;
}

/**     
 * \brief ���ϼ��
 *
 * ����û������еĲ����Ƿ�����������ƷҪ��
 *
 * \param ob: �������Ʒ
 * \param list:�����б�
 * \param is_resource: NOT USED YET
 * \return ����Ҫ�󷵻�true,���򷵻�false
 */
bool EquipMaker::check_material(zObjectB* ob,const std::map<DWORD,DWORD>& list,bool is_resource)
{
  /*
  for (zObjectB::material::stuffs_iterator it= ob->need_material.stuffs.begin(); it!=ob->need_material.stuffs.end(); ++it) 
  {
    Zebra::logger->debug("����%s��Ҫ����%d",ob->name,it->id);
  }
  // */
  
  if (list.size() != ob->need_material.stuffs.size()) return false;
  
  zObjectB::material::stuffs_iterator it = ob->need_material.stuffs.begin();
  while (it != ob->need_material.stuffs.end()) {
    std::map<DWORD,DWORD>::const_iterator l_it = list.find(it->id);
    if (l_it == list.end() || l_it->second != it->number) {
      return false;
    }      

    ++it;
  }
  
  return true;
}


bool EquipMaker::check_material1(zObjectB* ob,const std::map<DWORD,DWORD>& list)
{ 
  if (list.size() != ob->need_material.stuffs.size()) return false;
  
  return true;
}

#if 0
void  EquipMaker::pre_level_of_material(zObjectB* base)
{
  int max_level = 0,min_level = 0xffff;
  zObjectB::material::stuffs_iterator it = base->need_material.stuffs.begin();
  for ( ; it!=base->need_material.stuffs.end(); ++it) {
    zObjectB* ob = objectbm.get(it->id);
    if (!ob /*|| ob->level == 1*/) continue;
    if (ob->level > max_level) {
      max_level = ob->level;
      _max_level_id = it->id;
    }
    if (ob->level < min_level) {
      min_level = ob->level;
      _min_level_id = it->id;
    }
  }
}
#endif

void  EquipMaker::pre_level_of_material(int id,int level)
{
  if (!_1_id) {
    _1_id = id;
    _1_level = level;
  }

  if (_1_id == id && level > _1_level) _1_level = level;

  if (!_2_id && id != _1_id) {
    _2_id = id;
    _2_level = level;
  }
  if (_2_id == id && level > _2_level) _2_level = level;  
}

/**     
 * \brief ������ϵȼ�
 *
 * ���������ϵ����յȼ�
 *
 * \param id: ����id
 * \param num: ��������
 * \param level:���ϵȼ�
 * \param base: ������Ϣ
 * \return �ɹ�����true,���򷵻�false
 */
bool EquipMaker::level_of_material(DWORD id,DWORD num,DWORD level,zObjectB* base)
{
/*
  zObjectB::material::stuffs_iterator it = base->need_material.stuffs.begin();
  while ( it!= base->need_material.stuffs.end() && (it->id != id) ) ++it;
  if (it ==  base->need_material.stuffs.end() ) return false;

  zObjectB* ob = objectbm.get(id);
  if (!ob || ob->kind != ItemType_Resource || level < it->level) return false;
  
  _current += static_cast<double>( (1+it->level/8.0)*num*ob->level*(level-it->level+1) );
  _base += static_cast<double>(ob->level*num);
*/
  zObjectB::material::stuffs_iterator it = base->need_material.stuffs.begin();
  while ( it!= base->need_material.stuffs.end() && (it->id != id) ) ++it;
  if (it ==  base->need_material.stuffs.end() ) return false;

  float coff = 1.0;
  zObjectB* ob = objectbm.get(it->id);
  //if (ob && ob->level == 1) coff = 0.01;
  //���칫ʽ�޸�
  if (ob && ob->level == 1) coff = 0.1;
  
  if (((int)id == _1_id && (int)level == _1_level) || ((int)id == _2_id && (int)level == _2_level) ) {
    _current += static_cast<double>( coff*5.0f*num*(level-it->level+1) );
    _base += static_cast<double>(coff*5.0f*num);
    //Zebra::logger->debug("current:%f\tbase:%f",_current,_base);
  }else {
    _current += static_cast<double>( coff*5.0f*num*(level-it->level+1));
    _base += static_cast<double>(coff*5.0f*num);
    //Zebra::logger->debug("current:%f\tbase:%f",_current,_base);
  }
  
  return true;
}

/**     
 * \brief �������
 *
 * \param base: ������Ʒ
 * \return �������Ʒ
 */
zObject* EquipMaker::make_material(zObjectB* base)
{  
  zObject *ob = zObject::create(base,1);
  return ob;
}

/**     
 * \brief ���캯��
 *
 * ��ʼ����ر���
 *
 * param user : ���������û�
 *
 */   
EquipMaker::EquipMaker(SceneUser* user) : _current(0),_base(0),_make(user?true:false),_need(0),_1_id(0),_1_level(0),_2_id(0),_2_level(0)
{
  /*_odds.per = user?user->charstate.charm:0;
  _odds.luck = user?user->charstate.lucky:0;*/
}

/**     
 * \brief ����Hole
 *
 * \param ob: ������Ʒ
 * \return ��
 */
void EquipMaker::bonus_hole(zObject* ob)
{

  int bonus_hole = static_cast<int>(_odds.material_level -6);
  if (bonus_hole>0) {
    Hole::add_hole_num(ob,bonus_hole);
    Zebra::logger->debug("���ϵȼ�(%f),������(%d)",_odds.material_level,bonus_hole);
  }
}


/**     
 * \brief ����װ��
 *
 * \param user: �����û�
 * \param base: ������Ʒ
 * \param flag: �Ƿ�ǿ������
 * \return �������Ʒ
 */
zObject* EquipMaker::make(SceneUser* user,zObjectB* base, BYTE kind, int flag)
{  
  _odds.material_level = _base?(_current/_base):0;  
  /*if (user)
    Zebra::logger->debug("[����]�û�(%s)���ϵȼ�(%s:%.10f)",user?user->name:"",base->name,_odds.material_level);

  int white = odds_of_white(base);

  if (user)
    Zebra::logger->debug("[����]�û�(%s)���ɰ�ɫװ������(%s:%f\%)",user?user->name:"",base->name,white*1.0);
  
  if (selectByPercent(white) || flag ) {*/
    zObject *ob = zObject::create(base,1);
    if (!ob) return NULL;
    //_need = ob->base->needlevel;
    
    //zObject::logger(ob->createid,ob->data.qwThisID,ob->data.strName,ob->data.dwNum,ob->data.dwNum,1,0,NULL,user->id,user->name,"����װ��");
    assign(user, ob, base, kind);
    
    return ob;
  //}  

  //return NULL;
}

//sky�¼Ӽ�Ʒ���㺯��(ֱ�Ӱ���������)
/**     
* \brief ���ɼ�Ʒװ��
*
* \param user: �����û�
* \param base: ������Ʒ
* \param kind: ��Ʒ����
*/
void EquipMaker::assign(SceneUser* user,zObject* ob,zObjectB* base,BYTE kind)
{
	if( ob->base->kind == ItemType_SOULSTONE )			//sky ��ʯ�����ɼ�Ʒ
	{
		int Add_att = 0;	//��Ʒ����������ֵ

		switch(kind)
		{
		case 8: //��ɫװ��
			{
				ob->data.kind = 8;
				Add_att = 4;
			}
			break;
		case 4: //��ɫװ��
			{
				ob->data.kind = 4;
				Add_att = 3;
			}
			break;
		case 2: //��ɫװ��
			{
				ob->data.kind = 2;
				Add_att = 2;
			}
			break;
		case 1: //��ɫװ��
			{
				ob->data.kind = 1;
				Add_att = 1;
			}
			break;
		default:
			break;
		}

		if( ob->base->str != 0 )
		{
			ob->data.str += Add_att;
		}

		if( ob->base->inte != 0 )
		{
			ob->data.inte += Add_att;
		}

		if( ob->base->dex != 0 )
		{
			ob->data.dex += Add_att;
		}

		if( ob->base->spi != 0 )
		{
			ob->data.spi += Add_att;
		}

		if( ob->base->con != 0 )
		{
			ob->data.con += Add_att;
		}

		if( ob->base->pdefence != 0 )
		{
			ob->data.pdefence += Add_att;
		}

		if( ob->base->mdefence != 0 )
		{
			ob->data.mdefence += Add_att;
		}

		if( ob->base->maxpdamage != 0 )
		{
			ob->data.maxpdamage += Add_att;
		}

		if( ob->base->maxmdamage != 0 )
		{
			ob->data.maxmdamage += Add_att;
		}
	}
	else if( ob->base->kind == ItemType_ClothBody	||
		ob->base->kind == ItemType_FellBody		||
		ob->base->kind == ItemType_MetalBody	||
		ob->base->kind == ItemType_Blade		||
		ob->base->kind == ItemType_Axe			||
		ob->base->kind == ItemType_Hammer		||
		ob->base->kind == ItemType_Staff		||
		ob->base->kind == ItemType_Crossbow		||
		ob->base->kind == ItemType_Fan			||
		ob->base->kind == ItemType_Stick		||
		ob->base->kind == ItemType_Shield		||
		ob->base->kind == ItemType_Helm			||
		ob->base->kind == ItemType_Caestus		||
		ob->base->kind == ItemType_Cuff			||
		ob->base->kind == ItemType_Shoes		||
		ob->base->kind == ItemType_Necklace		||
		ob->base->kind == ItemType_Fing			||
		/*sky �������Ƥ���ͷ���֧��*/
		ob->base->kind == ItemType_Helm_Paper	||
		ob->base->kind == ItemType_Caestus_Paper||
		ob->base->kind == ItemType_Cuff_Paper	||
		ob->base->kind == ItemType_Shoes_Paper	||
		ob->base->kind == ItemType_Helm_Plate	||
		ob->base->kind == ItemType_Caestus_Plate||
		ob->base->kind == ItemType_Cuff_Plate	||
		ob->base->kind == ItemType_Shoes_Plate	||
		ob->base->kind == tyItemType_Shoulder	||
		ob->base->kind == tyItemType_Gloves		||
		ob->base->kind == tyItemType_Pants		||
		ob->base->kind == ItemType_Shoulder_Paper ||
		ob->base->kind == ItemType_Gloves_Paper	||
		ob->base->kind == ItemType_Pants_Paper	||
		ob->base->kind == ItemType_Shoulder_Plate ||
		ob->base->kind == ItemType_Gloves_Plate	||
		ob->base->kind == ItemType_Pants_Plate)				//sky װ�������ɼ�Ʒ			
	{
		//����װ���������Ե�
		DWORD All_pop = base->str + base->inte + base->dex + base->spi + base->con;

		switch(kind)
		{
		case 8: //��ɫװ��
			{

				ob->data.kind = 8;
				ob->data.Freedom.Surplus_Attribute = All_pop * 0.2f; //�����������Ե�
			}
			break;
		case 4: //��ɫװ��
			{
				ob->data.kind = 4;
				ob->data.Freedom.Surplus_Attribute = All_pop * 0.15f;
			}
			break;
		case 2: //��ɫװ��
			{
				ob->data.kind = 2;
				ob->data.Freedom.Surplus_Attribute = All_pop * 0.1f;
			}
			break;
		case 1: //��ɫװ��
			{
				ob->data.kind = 1;
				ob->data.Freedom.Surplus_Attribute = All_pop * 0.5f;
			}
			break;
		default:
			break;
		}

		fix(ob);
	}
	else if(ob->base->kind == ItemType_LevelUp)			//sky ���������ɼ�Ʒ
	{
		switch(kind)
		{
		case 8: //�жϳ�ɫװ������
			{
				ob->data.kind = 8;
			}
			break;
		case 4: //�ж���ɫװ������
			{
				ob->data.kind = 4;
			}
			break;
		case 2: //�ж���ɫװ������
			{
				ob->data.kind = 2;
			}
			break;
		case 1: //�ж���ɫװ������
			{
				ob->data.kind = 1;
			}
			break;
		default:
			break;
		}
	}
}

//sky�¼Ӽ�Ʒ���㺯��(��������)
/**     
 * \brief ���ɼ�Ʒװ��
 *
 * \param user: �����û�
 * \param base: ������Ʒ
 * \param npc_mul: ��Ʒ����(NPC)
 * \return �������Ʒ
 */
void EquipMaker::NewAssign(SceneUser* user,zObject* ob,zObjectB* base,DWORD npc_mul)
{
	if( npc_mul == 0)
		return;

	if( ob->base->kind == ItemType_SOULSTONE )				//sky ��ʯ�����ɼ�Ʒ
	{
		int Add_att = 0;	//��Ʒ����������ֵ

		if( selectByTenTh( g_orange_changce * npc_mul ) ) //�жϳ�ɫװ������
		{
			ob->data.kind = 8;
			Add_att = 4;
		}
		else if( selectByTenTh(g_purple_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 4;
			Add_att = 3;
		}
		else if( selectByTenTh(g_green_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 2;
			Add_att = 2;
		}
		else if( selectByTenTh(g_blue_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 1;
			Add_att = 1;
		}

		if( ob->base->str != 0 )
		{
			ob->data.str += Add_att;
		}

		if( ob->base->inte != 0 )
		{
			ob->data.inte += Add_att;
		}

		if( ob->base->dex != 0 )
		{
			ob->data.dex += Add_att;
		}

		if( ob->base->spi != 0 )
		{
			ob->data.spi += Add_att;
		}

		if( ob->base->con != 0 )
		{
			ob->data.con += Add_att;
		}

		if( ob->base->pdefence != 0 )
		{
			ob->data.pdefence += Add_att;
		}

		if( ob->base->mdefence != 0 )
		{
			ob->data.mdefence += Add_att;
		}

		if( ob->base->maxpdamage != 0 )
		{
			ob->data.maxpdamage += Add_att;
		}

		if( ob->base->maxmdamage != 0 )
		{
			ob->data.maxmdamage += Add_att;
		}
	}
	else if( ob->base->kind == ItemType_ClothBody	||
			ob->base->kind == ItemType_FellBody		||
			ob->base->kind == ItemType_MetalBody	||
			ob->base->kind == ItemType_Blade		||
			ob->base->kind == ItemType_Axe			||
			ob->base->kind == ItemType_Hammer		||
			ob->base->kind == ItemType_Staff		||
			ob->base->kind == ItemType_Crossbow		||
			ob->base->kind == ItemType_Fan			||
			ob->base->kind == ItemType_Stick		||
			ob->base->kind == ItemType_Shield		||
			ob->base->kind == ItemType_Helm			||
			ob->base->kind == ItemType_Caestus		||
			ob->base->kind == ItemType_Cuff			||
			ob->base->kind == ItemType_Shoes		||
			ob->base->kind == ItemType_Necklace		||
			ob->base->kind == ItemType_Fing			||
			/*sky �������Ƥ���ͷ���֧��*/
			ob->base->kind == ItemType_Helm_Paper	||
			ob->base->kind == ItemType_Caestus_Paper||
			ob->base->kind == ItemType_Cuff_Paper	||
			ob->base->kind == ItemType_Shoes_Paper	||
			ob->base->kind == ItemType_Helm_Plate	||
			ob->base->kind == ItemType_Caestus_Plate||
			ob->base->kind == ItemType_Cuff_Plate	||
			ob->base->kind == ItemType_Shoes_Plate	||
			ob->base->kind == tyItemType_Shoulder	||
			ob->base->kind == tyItemType_Gloves		||
			ob->base->kind == tyItemType_Pants		||
			ob->base->kind == ItemType_Shoulder_Paper ||
			ob->base->kind == ItemType_Gloves_Paper	||
			ob->base->kind == ItemType_Pants_Paper	||
			ob->base->kind == ItemType_Shoulder_Plate ||
			ob->base->kind == ItemType_Gloves_Plate	||
			ob->base->kind == ItemType_Pants_Plate)			//sky װ�������ɼ�Ʒ
	{
		//����װ���������Ե�
		DWORD All_pop = base->str + base->inte + base->dex + base->spi + base->con;

		if( selectByTenTh( g_orange_changce * npc_mul ) ) //�жϳ�ɫװ������
		{
			ob->data.kind = 8;
			ob->data.Freedom.Surplus_Attribute = All_pop * 0.2f; //�����������Ե�
		}
		else if( selectByTenTh(g_purple_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 4;
			ob->data.Freedom.Surplus_Attribute = All_pop * 0.15f;
		}
		else if( selectByTenTh(g_green_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 2;
			ob->data.Freedom.Surplus_Attribute = All_pop * 0.1f;
		}
		else if( selectByTenTh(g_blue_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 1;
			ob->data.Freedom.Surplus_Attribute = All_pop * 0.5f;
		}

		fix(ob);
	}
	else if(ob->base->kind == ItemType_LevelUp)			//sky ���������ɼ�Ʒ
	{
		if( selectByTenTh( g_orange_changce * npc_mul ) ) //�жϳ�ɫװ������
		{
			ob->data.kind = 8;
		}
		else if( selectByTenTh(g_purple_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 4;
		}
		else if( selectByTenTh(g_green_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 2;
		}
		else if( selectByTenTh(g_blue_changce * npc_mul) ) //�ж���ɫװ������
		{
			ob->data.kind = 1;
		}
	}
}

void EquipMaker::fix(zObject* ob)
{
  if (ob->data.pdamage > ob->data.maxpdamage) {
    std::swap(ob->data.pdamage,ob->data.maxpdamage);
  }

  if (ob->data.mdamage > ob->data.maxmdamage) {
    std::swap(ob->data.mdamage,ob->data.maxmdamage);
  }


}

#undef COMPUTE
#define COMPUTE(x) ob->data.x = static_cast<WORD>(ob->data.x*1.4);
/**     
 * \brief ������ʥװ��
 *
 * \param base: ������Ʒ
 * \return �ɹ�����true,���򷵻�false
 */
bool EquipMaker::assign_holy(zObject* ob,int holy)
{
//  zHolyObjectB *bob = holyobjectbm.get(ob->data.dwObjectID);
  //int index = randBetween(0,ob->base->holys.size()-1);      
  zHolyObjectB *bob = holyobjectbm.get(1001/*ob->base->holys[index]*/);
  if (bob==NULL) return false;
  
  ob->data.kind |= 4;//��ʥװ��,�ο�Object.hע��
  ob->data.holy = bob->holy;
  
  int property = 1;

  COMPUTE( maxhp )          // �������ֵ
  COMPUTE( maxmp )          // �����ֵ
  COMPUTE( maxsp )          // �������ֵ

  COMPUTE( pdamage )        // ��С������
  COMPUTE( maxpdamage )      // ��󹥻���
  COMPUTE( mdamage )        // ��С����������
  COMPUTE( maxmdamage )      // �����������

  COMPUTE( pdefence )        // ���
  COMPUTE( mdefence )        // ħ��

  COMPUTE_L( damage )    // �����˺�ֵx��
  COMPUTE_L(  fivepoint )    // ������������

  COMPUTE_L( hpr )    // ����ֵ�ָ�
  COMPUTE_L( mpr )    // ����ֵ�ָ�
  COMPUTE_L( spr )    // ����ֵ�ָ�

  COMPUTE_L( akspeed )  // �����ٶ�
  COMPUTE_L( mvspeed )  // �ƶ��ٶ�
  
  COMPUTE_L( atrating )  // ������
  COMPUTE_L( akdodge )    // ������

  COMPUTE_L( doublexp )  // %x˫������
  COMPUTE_L( mf )    //������
  
  BONUS_SKILL  
  BONUS_SKILLS
  
  return true;
}

#undef COMPUTE
#define COMPUTE(x) ob->data.x = randBetween(prop->x.min,prop->x.max); \
  set_prop[index] = 1; \
  --i; \
  break;
  
/**     
 * \brief ����������װ
 *
 * \param base: ������Ʒ
 * \return �ɹ�����true,���򷵻�false
 */
bool EquipMaker::assign_set(zObject* ob)
{
	zSetObjectB *base = setobjectbm.get(ob->data.dwObjectID);
	if (base==NULL)
	{
		//fprintf(stderr,"�������ʧ�� %s dwId = %u \n",ob->name,ob->data.dwObjectID);
		return false;
	}

	zFiveSetB* prop = fivesetbm.get(base->mark);
	if (!prop) return false;

	ob->data.kind |= 8;//������װ,�ο�Object.hע��

	for (zSetObjectB::iterator it=base->sets.begin(); it!=base->sets.end(); ++it) {
		if (selectByTenTh(it->odds)) {
			int i = 0;
			for (std::vector<WORD>::iterator s_it=(*it).ids.begin(); s_it!=(*it).ids.end(); ++s_it) {
				ob->data.fiveset[i++] = *s_it;
			}

			int set_prop[] = { 0,0,0,0,0};      
			do {
				int index = randBetween(0,4);
				if (set_prop[index]) continue;
				switch (index)
				{
				case 0:
					COMPUTE( dpdam ) //�����˺�����%x
				case 1:
					COMPUTE( dmdam ) //�����˺�����%x
				case 2:
					COMPUTE( bdam ) //�����˺�x%
				case 3:
					COMPUTE( rdam ) //�˺�����%x
				case 4:
					COMPUTE( ignoredef ) //%x����Ŀ�����
				default:
					break;    
				}

			}while (i>0);

			break;  
		}  
	}

	//fprintf(stderr,"�������ɹ� %s dwId = %u \n",ob->name,ob->data.dwObjectID);
	return true;
}

/**     
 * \brief ��������Ƿ���ļ��ʱ�ʯ�Ƿ�Ϸ�����Ӧ�Ķ���ɹ�����
 *
 * \param kind: ����װ������
 * \param id: ����ļ��ʱ�ʯ
 * \return �ɹ�����true,���򷵻�false
 */
bool EquipMaker::is_odds_gem(DWORD kind,DWORD id)
{

  if (id == 730 && kind >= ItemType_ClothBody && kind <= ItemType_MetalBody ) {
    _odds.odds_gem = 20;
    return true;
  }
    
  if (id == 731 && (kind >= ItemType_Blade && kind <= ItemType_Stick)) {
    _odds.odds_gem = 20;
    return true;
  }
    
  if (id == 732 && (kind == ItemType_Helm || kind == ItemType_Shield || kind == ItemType_Shoes) ) {
    _odds.odds_gem = 20;
    return true;
  }
    
  if (id == 733 && (kind == ItemType_Caestus || kind == ItemType_Caestus || kind == ItemType_Necklace || kind == ItemType_Fing) ) {
    _odds.odds_gem = 20;
    return true;
  }
  
  return false;
}

/**     
 * \brief ���ɰ�ɫװ������
 *
 * \param ob: ������Ʒ
 * \return ���ɸ���
 */
int EquipMaker::odds_of_white(const zObjectB* ob)
{
//  return static_cast<int>((20*_odds.per/(_odds.per+4) + 20*_odds.luck/(_odds.luck+10) + 70*_odds.material_level + 10*_odds.skill_level));    
  return static_cast<int>((20*_odds.per/(_odds.per+4) + 20*_odds.luck/(_odds.luck+10) + 100*_odds.material_level + 10*_odds.skill_level));    

}

/**     
 * \brief ������ɫװ������
 *
 * \param ob: ������Ʒ
 * \return ���ɸ���
 */
int EquipMaker::odds_of_blue(const zObjectB* ob)
{
  int odds =  g_blue_changce;

  //if (_make) odds = static_cast<int>( pow(_odds.material_level,5) + 0.1*ob->bluerating );
  if (_make) {
    odds = static_cast<int>( 100*pow(_odds.material_level+3,3) /*+ 0.1*ob->bluerating*/ );
    //�и��ԵĲ߻���ʽ������
    //if (odds >= 9800) odds = 10000;
    odds += 200;
  }

  return odds;
}

/**     
 * \brief ���ɽ�ɫװ������
 *
 * \param ob: ������Ʒ
 * \return ���ɸ���
 */
int EquipMaker::odds_of_gold(const zObjectB* ob)
{
  int odds =  g_green_changce;
;
  if (_make) {
    odds = static_cast<int>((4*pow(_odds.material_level,7) /* + 0.1*ob->goldrating*/ ) );
    //�и��ԵĲ߻���ʽ������
    //if (odds >= 9800) odds = 10000;
    odds += 200;
  }

  return odds;
}

/**     
 * \brief ������ʥװ������
 *
 * \param ob: ������Ʒ
 * \return ���ɸ���
 */
int EquipMaker::odds_of_holy(int object)
{
  int odds = object;
  if (_make) {
    odds = static_cast<int>( 0.01*pow(_odds.material_level,10) /* + object + 120 */ );
    //�и��ԵĲ߻���ʽ������
    //if (odds >= 9800) odds = 10000;
    odds += 200;
  }  

  return odds;
}

/**     
 * \brief װ���������Ը���
 *
 * \param ob: ������Ʒ
 * \return ���ɸ���
 */
int EquipMaker::odds_of_property(int object,int property)
{
  return static_cast<int>(( (  3*_odds.per/(_odds.per+4) + 3*_odds.luck/(_odds.luck+10) + _odds.odds_gem + 5*(_odds.material_level) )/property) + object);
}


RebuildObject* RebuildObject::_instance = NULL;

/**     
 * \brief  ����ģʽ,��֤��Ʒ�������Ψһʵ��
 *
 * \return ��Ʒ�������Ψһʵ��
 */  
RebuildObject& RebuildObject::instance()
{
  static RebuildObject new_instance;
  return new_instance;
  
}  

/**     
 * \brief �ϳ���ͨ��Ʒ
 *
 * \param user: ����ϳɵ��û�
 * \param cmd: �ϳ�ָ��
 * \return �ϳɳɹ�����true,���򷵻�false
 */
bool RebuildObject::compose(SceneUser& user,const Cmd::stPropertyUserCmd* cmd)
{
  //by RAY
//  Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"��Ʒ�ϳ�ϵͳ���ڿ� ���У�");
//  return false;

//#if 0
   Cmd::stComposeItemPropertyUserCmd * command = (Cmd::stComposeItemPropertyUserCmd *)cmd;

/*  
  if (!check_space(user,1,1)) {
    response(user,1,COMPOSE);
    return false;
  }
*/
  ObjectPack* pack = user.packs.equip.pack(EquipPack::R_MAKE);
  if (command->location == 1) pack = user.packs.equip.pack(EquipPack::L_MAKE);
  if (!pack || pack->object()->data.dur < 1 || user.tradeorder.in_trade(pack->object()) ) {
    Zebra::logger->warn("�û�(%d:%d:%s)����ϳɵİ���������,ʹ�ô����������ڽ�����",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;
  }

  struct do_compose : public PackageCallback
  {
    do_compose() : id(0),level(0),num(0),max_num(0),can_compose(true),compose_id(0)
    { }
    
    bool exec(zObject* ob)
    {
      if (! ( (ob->base->recastlevel && ob->data.upgrade < ob->base->recastlevel) ||(ob->base->make && ob->base->make != 1) )) {
        can_compose = false;
        return false;  
      }
      if (id && (ob->base->id != id || ob->data.upgrade != level)) {
        can_compose = false;  
        return false;
      }
      
      if (!id) {
        id = ob->base->id;
        level = ob->data.upgrade;
        max_num = ob->base->maxnum;
        compose_id = ob->base->make;
      }
      
      num += ob->data.dwNum;    
      
      return true;  
    }
    
    DWORD id;
    BYTE level;
    DWORD num;
    DWORD max_num;
    bool can_compose;
    WORD compose_id;
    
  };
  
  do_compose dc;
  pack->execEvery(dc);
  
  if (!dc.can_compose || !dc.num ) {
    Zebra::logger->warn("�û�(%d:%d:%s)����ϳɲ��ɺϳ���Ʒ",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;      
  }

  if (command->per_num <3 || command->per_num > 5 || (dc.num % command->per_num) || (dc.num > command->per_num*dc.max_num) ) {
    Zebra::logger->warn("�û�(%d:%d:%s)����ϳ���Ʒ��������",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;
  }

  int expect_count = dc.num / command->per_num;

  zObjectB *base = objectbm.get(dc.id);
  
  DWORD gold = base->recastcost*expect_count; //base must exist
  if (!user.packs.checkMoney(gold)) {
    return false;
  }
  
  if (user.packs.removeMoney(gold,"��ͨ�ϳ�") ) {
    Base::refresh_pack(user,pack);
    
//    pack->clear(&user);
    user.packs.execEvery(pack,Type2Type<ClearPack>());
    
    int real_count = 0;

    int odds = 100;
    if (command->per_num == 3) odds=50;
    else if (command->per_num == 4) odds=75;
    
    do {
      if (selectByPercent(odds)) {
        ++real_count;
      }
    }while (--expect_count>0);

    if (!real_count) {
      response(user,1,COMPOSE);  
      return true;
    }

    zObject* o = NULL;
    if (dc.compose_id) {
      base = objectbm.get(dc.compose_id);
      o = zObject::create(base,real_count,0);
    }else {
      o = zObject::create(base,real_count,dc.level+1);
    }
    
    if (o) {
      o->data.pos = stObjectLocation(pack->type(),pack->id(),0,Cmd::MAKECELLTYPE_EQUIP);
      if (user.packs.addObject(o,false) ) {
        Base::add_object(user,o);
        zObject::logger(o->createid,o->data.qwThisID,o->base->name,o->data.dwNum,o->data.dwNum,1,0,NULL,user.id,user.name,"��ͨ�ϳ�",o->base,o->data.kind,o->data.upgrade);
        response(user,0,COMPOSE);
        return true;
      }
    }
    
    response(user,1,COMPOSE);      
    Zebra::logger->error("Ϊ�û�(%d:%d:%s)��Ӻϳ���Ʒʱʧ��",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;
  }
  
  return false;
//#endif
}

/**     
 * \brief �ϳɻ���ʯ sky û�л���ʯ�ĸ�����
 *
 * \param user: ����ϳɵ��û�
 * \param cmd: �ϳ�ָ��
 * \return �ϳɳɹ�����true,���򷵻�false
 */
bool RebuildObject::compose_soul_stone(SceneUser& user,const Cmd::stPropertyUserCmd* cmd)
{
  return false;
}

/**     
 * \brief ������Ʒ
 *
 * \param user: �����������û�
 * \param cmd: ����ָ��
 * \return �����ɹ�����true,���򷵻�false
 */
bool RebuildObject::upgrade(SceneUser& user,const Cmd::stPropertyUserCmd* cmd)
{
  Cmd::stUpgradeItemPropertyUserCmd * command = (Cmd::stUpgradeItemPropertyUserCmd *)cmd;

  ObjectPack* pack = user.packs.equip.pack(EquipPack::R_MAKE);
  if (command->location == 1) pack = user.packs.equip.pack(EquipPack::L_MAKE);
  if (!pack || pack->object()->data.dur < 1 || user.tradeorder.in_trade(pack->object()) ) {
    Zebra::logger->warn("�û�(%d:%d:%s)����ϳɵİ���������,ʹ�ô����������ڽ�����",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;
  }

  struct do_compose : public PackageCallback
  {
    do_compose() : up_ob(NULL),c_ob(NULL),odds_ob(NULL),count(0),can(true),dwObjectID(0)
    { }

/*    
    bool exec(zObject* ob)
    {
      if (!up_ob && ob->base->recast) {
        up_ob = ob;
      }else if (!c_ob && ob->data.dwObjectID == LEVELUP_STONE_ID) {
        c_ob = ob;  
      //}else if (!odds_ob && ob->base->kind==ItemType_AddLevelUpPercent) {
      //  odds_ob = ob;  
      }else {
        can = false;
        return false;
      }
      
      return true;  
    }
*/
    bool exec(zObject* ob)
    {
      if (!up_ob && ob->base->recast) {
        up_ob = ob;
/*
      }else if (ob->data.dwObjectID == LEVELUP_STONE_ID) {
        c_ob = ob;  
        ++count;
*/
      }else {
          
		  //if(dwObjectID != 0)
		  //	  return true;

		  if(ob->data.dwObjectID == 799 || 
			  ob->data.dwObjectID == 1170 || 
			  ob->data.dwObjectID == 4300)
			  {
				dwObjectID = ob->data.dwObjectID;
				return true;
			  }
        if (!c_ob) {
          c_ob = ob;
          ++count;
          return true;
        }

        if (ob->data.dwObjectID == c_ob->data.dwObjectID) {
          ++count;
          return true;
        }
        
        can=false;
        return false;
      }
      
      return true;  
    }

    int odds() const
    {
      return 10*(count-1);
    }
    
    zObject* up_ob;
    zObject* c_ob;
    zObject* odds_ob;
    int count;
    
    bool can;

	DWORD dwObjectID;//���Ϊ,799,1170,4300,��ʹ��Ĭ�ϵ���������

    bool is_stuff()
    {
      zUpgradeObjectB *base = upgradeobjectbm.get(up_ob->data.dwObjectID+100000*(up_ob->data.upgrade+1));
      if (!base) return false;
  
      return c_ob->data.dwObjectID == base->stuff;
    }
    
    bool can_compose()
    {
      return count>=1 && up_ob && is_stuff() && can;  
    }
  };
  
  do_compose dc;
  pack->execEvery(dc);


  if(dc.dwObjectID != 0)
	  {
		//ʹ���µ���������
		goto _up;
	  }

  if (!dc.can_compose()) {
    Zebra::logger->warn("�û�(%d:%d:%s)������������������Ʒ",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;      
  }

_up:

  DWORD _odds = dc.odds();
   switch(dc.dwObjectID)
  {
  case 799:
	  _odds += 20;
  case 1170:
	  _odds += 40;
  case 4300:
	  _odds += 100;
  }

  DWORD need = 0;

  zUpgradeObjectB *uob = upgradeobjectbm.get(dc.up_ob->data.dwObjectID + 100000*(dc.up_ob->data.upgrade+1));
  if (uob)  {
    need = uob->gold;
  }else {
    Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"����Ʒ��������");  
    return false;
  }

  if (!user.packs.checkMoney(need)) {
    return false;
  }
  
  if (user.packs.removeMoney(need,"����") ) {
    Base::refresh_pack(user,pack);
    
    bool ret = Upgrade::upgrade(user,dc.up_ob,_odds);//dc.odds());
/*    
    pack->clear(&user,dc.up_ob);
*/
    zObject* ob = zObject::create(dc.up_ob);
//    user.packs.clearPackage(pack);
    user.packs.execEvery(pack,Type2Type<ClearPack>());
    if (ob ) {  
      ob->data.pos = stObjectLocation(pack->type(),pack->id(),0,Cmd::MAKECELLTYPE_EQUIP);
      
      if (user.packs.addObject(ob,false)) {      
        zObject::logger(ob->createid,ob->data.qwThisID,ob->data.strName,ob->data.dwNum,ob->data.dwNum,1,0,NULL,user.id,user.name,"��������",ob->base,ob->data.kind,ob->data.upgrade);
        Base::add_object(user,ob,false);
        response(user,ret?0:1,UPGRADE);
      }
    }
    
    return true;
  }

  return false;
}


/**     
 * \brief װ�����
 *
 * \param user: �����׵��û�
 * \param cmd: ���ָ��
 * \return ��׳ɹ�����true,���򷵻�false
 */
bool RebuildObject::hole(SceneUser& user,const Cmd::stPropertyUserCmd* cmd)
{
  Cmd::stHolePropertyUserCmd * command = (Cmd::stHolePropertyUserCmd *)cmd;
    
  zObject *up_ob = user.packs.uom.getObjectByThisID(command->up_id);
  if (!up_ob || !Hole::can_hole(up_ob)) {
    Zebra::logger->warn("�û�(%d:%d:%s)����Բ��ܴ��װ�����",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;      
  }
  if (!check_npc(user,up_ob->base,NpcTrade::NPC_HOLE_OBJECT)) {
    return false;
  }
  
  zObject* ob = user.packs.uom.getObjectByID(command->gem_id,command->gem_level,true);
  if (!ob || ob->data.dwObjectID != HOLE_SONTE_ID || Hole::get_hole_num(up_ob) == Hole::INVALID_NUM || ob->data.upgrade != (Hole::get_hole_num(up_ob)-1) ) {
    Zebra::logger->warn("�û�(%d:%d:%s)��װ�����ʱȱ�ٱ�ʯ",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;      
  }

  DWORD gold = HOLE_MONEY*(1 << ob->data.upgrade);

  DWORD taxMoney = (DWORD)((gold*(user.scene->getTax())/100.0f)+0.5f); // ������˰
  gold = gold + taxMoney;
  Cmd::Session::t_taxAddCountry_SceneSession send;
  send.dwCountryID = user.scene->getCountryID();
  send.qwTaxMoney = taxMoney;
  sessionClient->sendCmd(&send,sizeof(send));

  if (!user.packs.checkMoney(gold)) {
    return false;
  }
  
  if (user.packs.removeMoney(gold,"���") ) {
    Hole::add_hole(up_ob,ob->data.upgrade);
    user.reduceObjectNum(command->gem_id,1,command->gem_level);
    add_object(user,up_ob,false);
    response(user,0,HOLE);
    return true;

  }

  return false;
  
}

/**     
 * \brief ��Ƕ��Ʒ
 *
 * \param user: ������Ƕ���û�
 * \param cmd: ��Ƕָ��
 * \return ��Ƕ�ɹ�����true,���򷵻�false
 */
bool RebuildObject::enchance(SceneUser& user,const Cmd::stPropertyUserCmd* cmd)
{
  Cmd::stEnchasePropertyUserCmd * command = (Cmd::stEnchasePropertyUserCmd *)cmd;

  ObjectPack* pack = user.packs.equip.pack(EquipPack::R_MAKE);
  if (command->location == 1) pack = user.packs.equip.pack(EquipPack::L_MAKE);
  if (!pack || pack->object()->data.dur < 1 || user.tradeorder.in_trade(pack->object()) ) {
    Zebra::logger->warn("�û�(%d:%d:%s)����ϳɵİ���������,ʹ�ô����������ڽ�����",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;
  }

  //static _obj_need_store[][]={{101,748},{101,748},{101,748},{101,748},
  struct do_compose : public PackageCallback
  {
    do_compose() : up_ob(NULL),c_ob(NULL),s_ob(NULL),can(true)
    { }
    
    bool exec(zObject* ob)
    {
      if (!c_ob && ob->data.dwObjectID == ENCHANCE_SONTE_ID) {
        c_ob = ob;  
      }else if (!s_ob && ob->base->kind==ItemType_SOULSTONE) {
        if (up_ob){
          switch(ob->base->id){
            case 748:
              {
                if ((101 <=up_ob->base->kind) && (up_ob->base->kind <= 110)){
                  s_ob = ob;
                }
              }
              break;
            case 749:
              {
                if (113 ==up_ob->base->kind || up_ob->base->kind == 116){
                  s_ob = ob;
                }
              }
              break;
            case 680:
            case 750:
              {
                if (114 == up_ob->base->kind || up_ob->base->kind == 115){
                  s_ob = ob;
                }
              }
              break;
            case 751:
              {
                if (117 == up_ob->base->kind || up_ob->base->kind == 118){
                  s_ob = ob;
                }
              }
              break;
            default:
              {
                can = false;
                return false;
              }
              break;
          }
        }else{
          s_ob = ob;  
        }
      }else if (!up_ob) {
        if (s_ob){
          switch(ob->base->kind){
           // case 101 ... 111: 
			case 102:
			case 103:
			case 104:
			case 105:
			case 106:
			case 107:
			case 108:
			case 109:
			case 110:
			case 111:
              {
                if (s_ob->base->id == 748){
                  up_ob = ob;
                }
              }
              break;
            case 113:
            case 116:
              {
                if (s_ob->base->id == 749){
                  up_ob = ob;
                }
              }
              break;
            case 114:
            case 115:
              {
                if (s_ob->base->id == 680 || s_ob->base->id == 751){
                  up_ob = ob;
                }
              }
              break;
            case 117:
            case 118:
              {
                if (s_ob->base->id == 751){
                  up_ob = ob;
                }
              }
              break;
            default:
              {
                can = false;
                return false;
              }
              break;
          }
        }else{
          up_ob = ob;
        }
      }else {
        can = false;
        return false;
      }
      
      return true;  
    }
    
    zObject* up_ob;
    zObject* c_ob;
    zObject* s_ob;
    
    bool can;
    
    bool can_compose()
    {
      return can && up_ob && c_ob && s_ob && 
        Hole::get_hole_num(up_ob) != Hole::INVALID_NUM &&  Hole::get_empty_hole(up_ob) != Hole::INVALID_INDEX &&
        c_ob->data.upgrade == Hole::get_empty_hole(up_ob);
    }
  };
  
  do_compose dc;
  pack->execEvery(dc);

  if (!dc.can_compose()) {
    Zebra::logger->warn("�û�(%d:%d:%s)������Ƕ������Ƕ��Ʒ",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;      
  }

  DWORD gold = ENCHANCE_MONEY*(1 << dc.c_ob->data.upgrade);

  if (!user.packs.checkMoney(gold)) {
    return false;
  }
  
  if (user.packs.removeMoney(gold,"��Ƕ") ) {
    Base::refresh_pack(user,pack);
    
    Hole::put_hole(dc.up_ob,dc.c_ob->data.upgrade,dc.s_ob);
    
    SoulStone::enchance(user,dc.up_ob,dc.s_ob);  
//  pack->clear(&user,dc.up_ob);
    zObject* ob = zObject::create(dc.up_ob);
//    user.packs.clearPackage(pack);
    user.packs.execEvery(pack,Type2Type<ClearPack>());
    if (ob) {
      ob->data.pos = stObjectLocation(pack->type(),pack->id(),0,Cmd::MAKECELLTYPE_EQUIP);
      if (user.packs.addObject(ob,false)) {
        Base::add_object(user,ob,false);
        response(user,0,ENCHANCE);
      }
    }    
    return true;
  }

  return false;
}

/**     
 * \brief �ֽ���Ʒ
 *
 * \param user: ����ֽ���û�
 * \param cmd: �ֽ�ָ��
 * \return �ֽ�ɹ�����true,���򷵻�false
 */
bool RebuildObject::decompose(SceneUser& user,const Cmd::stPropertyUserCmd* cmd)
{
  Cmd::stDecomposePropertyUserCmd * command = (Cmd::stDecomposePropertyUserCmd *)cmd;
    
  zObject *up_ob = user.packs.uom.getObjectByThisID(command->up_id);
  if (!up_ob || up_ob->base->make != 1) {
    Zebra::logger->warn("�û�(%d:%d:%s)����ֽⲻ�ֽܷ�װ��",user.charbase.accid,user.charbase.id,user.charbase.name);
    return false;
  }
  

  if (!check_space(user,1,1)) {
    response(user,1,DECOMPOSE);
    return false;
  }

  if (!check_npc(user,up_ob->base,NpcTrade::NPC_DECOMPOSE_OBJECT)) {
    return false;
  }

  // �Ѱ󶨵���Ʒ��Ҫ��֤�Ƿ����ܱ���
  if (up_ob->data.bind && user.isSafety(Cmd::SAFE_SPLIT_OBJECT)) return false;

  Decompose decompose(up_ob);

  DWORD gold = decompose.gold();

  DWORD taxMoney = (DWORD)((gold*(user.scene->getTax())/100.0f)+0.5f); // ������˰
  gold = gold + taxMoney;
  Cmd::Session::t_taxAddCountry_SceneSession send;
  send.dwCountryID = user.scene->getCountryID();
  send.qwTaxMoney = taxMoney;
  sessionClient->sendCmd(&send,sizeof(send));

  if (!user.packs.checkMoney(gold)) {
    return false;
  }
  
  if (user.packs.removeMoney(gold,"�ֽ�") ) {
    decompose.bonus_exp(user);
  //  if (up_ob->data.maker[0] ) {
      decompose.bonus_items(user);
  //  }
    decompose.remove_from(user);
    response(user,0,DECOMPOSE);  
    return true;  
  }
  
  response(user,1,DECOMPOSE);
  return false;
}


/*zObject *RebuildObject::remake(SceneUser& user,const Cmd::stReMakObjectUserCmd* cmd,DWORD id)
{
  zObjectB *up_ob = objectbm.get(id);

  if(NULL == up_ob)
  {
	return NULL;
  }
 
  if (!up_ob || up_ob->make != 1) {
    Zebra::logger->warn("�û�(%d:%d:%s)������첻�ɴ�����Ʒ",user.charbase.accid,user.charbase.id,user.charbase.name);
    return NULL;  
  }
  
  DWORD dwGold = up_ob->need_material.gold;

  if (!user.packs.checkMoney(dwGold)) {
    return NULL;
  }

  EquipMaker make(&user);
  std::map<DWORD,DWORD> list;
    
  int count = -1;
  int level = 0;
  while (++count < (int)cmd->count) make.pre_level_of_material(cmd->list[count].gem_id,cmd->list[count].gem_level);

  count = -1;  
  while ( ++count < (int)cmd->count) {
    if (!user.packs.uom.exist(cmd->list[count].gem_id,cmd->list[count].gem_num,cmd->list[count].gem_level)) {
      return NULL;
    }
    if (make.is_odds_gem(up_ob->kind,cmd->list[count].gem_id) && cmd->list[count].gem_num == 1 ) continue;
    
    if (!level) level = cmd->list[count].gem_level;
    if (up_ob->kind == ItemType_Resource && level != cmd->list[count].gem_level ) return false;
    
    if (! make.level_of_material(cmd->list[count].gem_id,cmd->list[count].gem_num,cmd->list[count].gem_level,up_ob) ) {
      return NULL;
    }
    list[cmd->list[count].gem_id] += cmd->list[count].gem_num;
  }
    
  if (!make.check_skill(user,up_ob) ) {
    return NULL;
  }
  
  if (!make.check_material1(up_ob,list)) {
    return NULL;
  }
  
  count = -1;
  while ( ++count < (int)cmd->count) {
    user.reduceObjectNum(cmd->list[count].gem_id,cmd->list[count].gem_num,cmd->list[count].gem_level);
  }  

  user.packs.removeMoney(dwGold,"����");

  zObject *ob = NULL;
  if (up_ob->kind == ItemType_Resource) {
    ob = make.make_material(up_ob);
  }else {
    ob = make.make(&user,up_ob);
  }

  return ob;
}*/


/**     
 * \brief ������Ʒ
 *
 * \param user: ���������û�
 * \param cmd: ����ָ��
 * \return ����ɹ�����true,���򷵻�false
 */
int RebuildObject::make(SceneUser& user, DWORD dwID, int num, BYTE kind)
{

	int MakeNum = 0;
	zObjectB *up_ob = objectbm.get(dwID);

	//sky ���Ȳ鿴Ҫ�������Ʒ�Ƿ�Ϸ�
	if(NULL == up_ob)
	{
		Channel::sendSys(&user, Cmd::INFO_TYPE_FAIL,"�������Ҳ�����Ʒ!");
		Zebra::logger->debug("�û�%s������ͨװ��id = %uʧ��",user.name,dwID);
		return 0;
	}

	//sky Ч�����������Ƿ�����Ʒ���������
	if(num < 1)
		return 0;

	if(num > up_ob->maxnum)
		MakeNum = up_ob->maxnum;
	else
		MakeNum = num;

	//sky Ч��NPC�����Ƿ�Ϸ�
	if (!check_npc(user,up_ob,NpcTrade::NPC_MAKE_OBJECT)) 
	{
		Zebra::logger->debug("�û�%s������ͨװ��%sʧ��,������npctrade��д������",user.name,up_ob->name);
		return 0;
	}

	//sky Ч����Ʒ�Ƿ���Ա�����
	if (!up_ob || up_ob->make != 1) {
		Zebra::logger->warn("�û�(%d:%d:%s)������첻�ɴ�����Ʒ",user.charbase.accid,user.charbase.id,user.charbase.name);
		return 0;  
	}

	//sky ���û������Ƿ���ʣ��ռ�
	if (!check_space(user,up_ob->width,up_ob->height)) {
		response(user,3,1);
		return 0;
	}

	DWORD dwGold = up_ob->need_material.gold;
	dwGold = dwGold * MakeNum;		//sky ǮҲ�ǰ����۳���Ҫ���������

	//sky ��֤�û��Ľ�Ǯ�Ƿ��������Ҫ��
	if (!user.packs.checkMoney(dwGold)) {
		response(user,1,1);
		return 0;
	}

	EquipMaker make(&user);

	//sky ��֤�û��Ĳ��ϺͲ��ϵļ�Ʒ�����Ƿ���ϴ��������
	for(int i=0; i<up_ob->need_material.stuffs.size(); i++)
	{
		if( !user.packs.uom.exist(up_ob->need_material.stuffs[i].id, 
			up_ob->need_material.stuffs[i].number * MakeNum, //sky һ���Ĳ��ϵ���������Ҫ���������
			kind))
		{
			response(user,2,1);
			return 0;
		}
	}

	//sky �����㹻����ɾ�����û��Ĳ����ڴ���
	for(int i=0; i<up_ob->need_material.stuffs.size(); i++)
	{
		user.reduceObjectNum(up_ob->need_material.stuffs[i].id,
			up_ob->need_material.stuffs[i].number * MakeNum, //sky һ���Ĳ��ϵ���������Ҫ���������
			kind);
	}

	//sky ��Ǯ
	user.packs.removeMoney(dwGold,"����");

	zObject *ob = NULL;

	ob = make.make(&user,up_ob, kind);

	//sky ���ݴ�������������Ʒ������
	if(MakeNum > 1)
		ob->data.dwNum = MakeNum;

	if (ob && user.packs.addObject(ob,true,AUTO_PACK)) {
		zObject::logger(ob->createid,ob->data.qwThisID,ob->data.strName,ob->data.dwNum,ob->data.dwNum,1,0,NULL,user.id,user.name,"��������",ob->base,ob->data.kind,ob->data.upgrade);

		add_object(user,ob);

		//response(user,0,1);
		return MakeNum;
	}

	response(user,4,1);

	return 0;
}

