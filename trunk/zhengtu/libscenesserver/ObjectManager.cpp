#include <zebra/ScenesServer.h>

GlobalObjectIndex *GlobalObjectIndex::onlyme=NULL;

GlobalObjectIndex *const goi=GlobalObjectIndex::getInstance();

GlobalObjectIndex *GlobalObjectIndex::getInstance()
{
  if (onlyme==NULL)
    onlyme= new GlobalObjectIndex();
  return onlyme;
}

void GlobalObjectIndex::delInstance()
{
  SAFE_DELETE(onlyme);
}

GlobalObjectIndex::~GlobalObjectIndex()
{
  clear();
}

GlobalObjectIndex::GlobalObjectIndex()
{
}

bool GlobalObjectIndex::addObject(zObject * o)
{
  bool bret=false;
  if (o)
  {
    mlock.lock();
    zObject *ret =(zObject *)getEntryByID(o->id);
    if (ret)//��Ʒ�ظ�����
    {
      if (ret==o)
      {
        bret=true;
        Zebra::logger->debug("ȫ����Ʒ�������з������ظ������Ʒ(%s,%d)",ret->data.strName,ret->data.qwThisID);
      }
      else if (ret->createtime==o->createtime && ret->data.dwObjectID==o->data.dwObjectID && ret->data.qwThisID == o->data.qwThisID)
      {
        Zebra::logger->warn("������Ʒ:%s ����ʱ��:%I64u",ret->name,ret->createid);
        ret=false;
      }
      else
      {
        Zebra::logger->debug("id��ͻ%s(%d)",ret->name,ret->id);
        do {
          //��������ID
          o->generateThisID();
          bret = addEntry((zEntry *)o);
        } while (!bret);
        //if (!bret) Zebra::logger->fatal("�����Ʒ����Ʒ��ʧ��1");
      }
    }
    else
    {
      bret=addEntry((zEntry *)o);
      if (!bret) Zebra::logger->fatal("�����Ʒ����Ʒ��ʧ��");
    }
    mlock.unlock();
  }else {
    Zebra::logger->fatal("��ӷǷ���Ʒ");
  }
  
  return bret;
}

void GlobalObjectIndex::removeObject(DWORD thisid)
{
  mlock.lock();
  zEntry *e=getEntryByID(thisid);
  if (e)
    removeEntry(e);
  mlock.unlock();
}


zObject *GlobalObjectIndex::getObjectByThisid(DWORD thisid)
{
  mlock.lock();
  zEntry *e=getEntryByID(thisid); 
  mlock.unlock();
  return (zObject *)e;
}

UserObjectM::UserObjectM()
{
}

UserObjectM::~UserObjectM()
{
}

zObject * UserObjectM::getObjectByThisID( DWORD thisid)
{
  return (zObject *)getEntryByID(thisid);
}

class UserObjectComparePos:public UserObjectCompare 
{
  public:
    stObjectLocation dst;

    bool isIt(zObject *object)
    {
      return true;
    }
};

zObject *UserObjectM::getObjectByPos(const stObjectLocation &dst)
{
  UserObjectComparePos comp;
  comp.dst=dst;
  return getObject(comp);
}

void UserObjectM::removeObjectByThisID(DWORD thisid)
{
  zEntry *e=getEntryByID(thisid);
  if (e)
    removeEntry(e);
}

void UserObjectM::removeObject(zObject * o)
{
  removeEntry(o);
}

bool UserObjectM::addObject(zObject * o)
{
  return addEntry(o);
}

zObject *UserObjectM::getObject(UserObjectCompare &comp)
{
  for(hashmap::iterator it=ets.begin();it!=ets.end();it++)
  {
    if (((zObject *)it->second)->data.pos.loc()!=Cmd::OBJECTCELLTYPE_COMMON &&
        ((zObject *)it->second)->data.pos.loc()!=Cmd::OBJECTCELLTYPE_PACKAGE) continue;

    if (comp.isIt((zObject *)it->second))
      return (zObject *)it->second;
  }
  return NULL;
}

void UserObjectM::execEvery(UserObjectExec &exec)
{
//  Obj_vec a,b;
  Obj_vec a;
  for(hashmap::iterator it=ets.begin();it!=ets.end();it++)
  {

    zObject* tmp = (zObject *)it->second;
    //      Zebra::logger->debug("��Ʒ%s(%d,%d,%d,%d)",tmp->name,tmp->data.pos.dwLocation,tmp->data.pos.dwTableID,tmp->data.pos.x,tmp->data.pos.y);
    if (tmp->data.pos.tab()) {
      /*
         if (a.empty() || a[0]->data.pos.dwTableID == tmp->data.pos.dwTableID) {
         a.push_back(tmp);
         }else {
         b.push_back(tmp);
         }
       */
      a.push_back(tmp);
      continue;
    }

    if (!exec.exec((zObject *)it->second))
      return;
  }
  
  Obj_vec::iterator it;
  for (it=a.begin(); it!=a.end(); ++it) {
//    Zebra::logger->debug("��Ʒ%s(%d,%d,%d)",(*it)->name,(*it)->data.pos.dwLocation,(*it)->data.pos.dwTableID,(*it)->data.pos.x,(*it)->data.pos.y);    
    if (!exec.exec(*it))
      return;
  }
/*
  for (it=b.begin(); it!=b.end(); ++it) {
//    Zebra::logger->debug("��Ʒ%s(%d,%d,%d)",(*it)->name,(*it)->data.pos.dwLocation,(*it)->data.pos.dwTableID,(*it)->data.pos.x,(*it)->data.pos.y);    
    if (!exec.exec(*it))
      return;
  }
*/

}

/**
 * \brief �жϰ�����ĳ��Ʒ�Ƿ����ָ��������
 * moved from MainPack for support multi packages
 * \param id: ��Ʒ��objectid
 * \param number: Ҫ�������
 * \param upgrade: sky ��Ʒ��Ʒ�ȼ�,0��ͨ,1��ɫ,2��װ,4��װ,8��װ 
 * \param type: �Ƚ�����0Ϊ��ƷƷ�ȼ�,1Ϊ��Ʒ����
 * \return ��������Ʒ�������ڵ��ڸ�������������ĳһ����Ʒ��thisid,���򷵻�0
 */
DWORD UserObjectM::exist(DWORD id,DWORD number,BYTE upgrade,BYTE type) const
{
  DWORD num = 0;
  
  //sky ����������ʼ�Ƚϲ���
  for(hashmap::const_iterator it=ets.begin();it!=ets.end();it++)  
  {
    zObject* ob = (zObject *)it->second;

    if (ob->data.pos.loc()!=Cmd::OBJECTCELLTYPE_COMMON && ob->data.pos.loc()!=Cmd::OBJECTCELLTYPE_PACKAGE) continue;

	//sky �Ƚ���Ʒ��ID�ͼ�Ʒ�ȼ��Ƿ�����Ϸ���
    if (ob->data.dwObjectID == id && ob->data.kind == upgrade)
    {
      num +=   ob->data.dwNum;
      if (num >= number) 
		  return ob->data.qwThisID;
    }    
  }
  
  return 0;
}

int UserObjectM::space(const SceneUser* user) const
{
#if 0
  int free = 0;
  free = user->packs.main.get_width()*user->packs.main.get_height();
  if (user->packs.equip.left) {
    free += user->packs.equip.left->get_width()*user->packs.equip.left->get_height();    
  }
  if (user->packs.equip.right) {
    free += user->packs.equip.right->get_width()*user->packs.equip.right->get_height();    
  }
  
  for(hashmap::const_iterator it=ets.begin();it!=ets.end();it++) {
    zObject* ob = (zObject *)it->second;
    if (ob->data.pos.dwLocation!=Cmd::OBJECTCELLTYPE_COMMON && ob->data.pos.dwLocation!=Cmd::OBJECTCELLTYPE_PACKAGE) continue;
    if (ob->base->kind == ItemType_Money) continue;
    //Zebra::logger->debug("space: %s,%d",ob->name,ob->data.pos.dwLocation);
    --free;
  }
#endif
  int free = user->packs.main.space();
  if (user->packs.equip.pack(EquipPack::L_PACK) && user->packs.equip.pack(EquipPack::L_PACK)->can_input()) {
    free += user->packs.equip.pack(EquipPack::L_PACK)->space();    
  }
  if (user->packs.equip.pack(EquipPack::R_PACK) && user->packs.equip.pack(EquipPack::R_PACK)->can_input()) {
    free += user->packs.equip.pack(EquipPack::R_PACK)->space();    
  }

  return free;
}

/**
 * \brief ������Ʒid�ڰ����в���û�е��������������Ʒ,����������ж����ͬID��Ʒ,���ڰ����е�˳�򷵻�
 * \param id: ��Ʒ��objectid
 * \param upgrade: ��Ʒ�ȼ�
 * \param not_need_space: �Ƿ���Ҫ��ʣ��ռ�
 * \return �ҵ��򷵻ظ���Ʒʵ��,���򷵻�NULL
 */
zObject* UserObjectM::getObjectByID(DWORD id,BYTE upgrade,bool not_need_space) const
{
  for(hashmap::const_iterator it=ets.begin();it!=ets.end();it++)  {

    zObject* ob = (zObject *)it->second;
    if (ob->data.pos.loc()!=Cmd::OBJECTCELLTYPE_COMMON && ob->data.pos.loc()!=Cmd::OBJECTCELLTYPE_PACKAGE) continue;

    if ((ob->data.dwObjectID == id) && (ob->data.upgrade== upgrade) && ( not_need_space || ob->data.dwNum < ob->base->maxnum) ) {
      return ob;  
    }    
  }

  return NULL;
}

/**
 * \brief ������Ʒid������Ʒ�ڰ����е�����,������ڸ���Ʒ��������򴴽�һ������Ʒ
 * \param user: Ҫ�ı�������û�
 * \param id: ��Ʒ��objectid
 * \param number: ���ӵ�����
 * \param orig_ob: �������ı����Ʒ,���û����Ʒ�������ı�,�ò���������
 * \param new_obs: �´�������Ʒ���б�,���û����Ʒ������,�ò���������
 * \param upgrade: ��Ʒ�ȼ�
 * \return ʧ�ܷ���-1,û����Ʒ����������0,����������1,�ɹ�������д�������Ʒ����2
 */
int UserObjectM::addObjectNum(SceneUser* user,DWORD id,DWORD number,zObject* & orig_ob,Obj_vec& new_obs,BYTE upgrade)
{
  //check if exist this object id
  zObjectB* o = objectbm.get(id);
  if (!o) {
    return -1;
  }
  
  DWORD capacity = o->maxnum;
  
  //check if we have a instance of this object which havn't reached max number
  zObject* ob = getObjectByID(id,upgrade);
  if (ob) {
    number += ob->data.dwNum;
    if (capacity >= number) { //have enough space to put this 
      ob->data.dwNum = number;      
      orig_ob = ob;
      return 0;
    }else { //space isn't enough,set max number first,then create new object
      ob->data.dwNum = capacity;
      number -= capacity;
      orig_ob = ob;
    }
  }
  
  while ((int)number > 0) {
    zObject *tmp = zObject::create(o,(number>=capacity)?capacity:number,upgrade);
    if (tmp) {
      EquipMaker maker(NULL);
      maker.NewAssign(NULL,tmp,tmp->base,1);
      
      if (user->packs.addObject(tmp,true,AUTO_PACK)) {
        //�����˫��������ߺ�����������Ҫ��
        if (tmp->base->kind == ItemType_DoubleExp || tmp->base->kind == ItemType_Honor || tmp->base->kind == ItemType_ClearProperty)
        {
          tmp->data.bind=1;
        }
      zObject::logger(tmp->createid,tmp->data.qwThisID,tmp->data.strName,tmp->data.dwNum,tmp->data.dwNum,1,0,NULL,user->id,user->name,"����",tmp->base,tmp->data.kind,tmp->data.upgrade);
        new_obs.push_back(tmp);    
      }else {
        //maybe package is full,stop process,but still return true
        zObject::destroy(tmp);  
        return 1;
      }
    }
    number -= capacity;
  }
  
  return 2;
}

/**
 * \brief ������Ʒid������Ʒ�ڰ����е�����,������ڸ���Ʒ��������򴴽�һ������Ʒ���Ұ�(ֻ��������ӿ�����������)
 * \param user: Ҫ�ı�������û�
 * \param id: ��Ʒ��objectid
 * \param number: ���ӵ�����
 * \param orig_ob: �������ı����Ʒ,���û����Ʒ�������ı�,�ò���������
 * \param new_obs: �´�������Ʒ���б�,���û����Ʒ������,�ò���������
 * \param upgrade: ��Ʒ�ȼ�
 * \return ʧ�ܷ���-1,û����Ʒ����������0,����������1,�ɹ�������д�������Ʒ����2
 */
int UserObjectM::addGreenObjectNum(SceneUser* user,DWORD id,DWORD number,zObject* & orig_ob,Obj_vec& new_obs,BYTE upgrade)
{
  //check if exist this object id
  zObjectB* o = objectbm.get(id);
  if (!o) {
    return -1;
  }
  
  DWORD capacity = o->maxnum;
  
  //check if we have a instance of this object which havn't reached max number
  zObject* ob = getObjectByID(id,upgrade);
  if (ob) {
    number += ob->data.dwNum;
    if (capacity >= number) { //have enough space to put this 
      ob->data.dwNum = number;      
      orig_ob = ob;
      return 0;
    }else { //space isn't enough,set max number first,then create new object
      ob->data.dwNum = capacity;
      number -= capacity;
      orig_ob = ob;
    }
  }
  
  while ((int)number > 0) {
    zObject *tmp = zObject::create(o,(number>=capacity)?capacity:number,upgrade);
    if (tmp) {
      tmp->data.bind=1;
      EquipMaker maker(NULL);
      maker.NewAssign(NULL,tmp,tmp->base,1);
      
      if (user->packs.addObject(tmp,true,AUTO_PACK)) {
        zObject::logger(tmp->createid,tmp->data.qwThisID,tmp->data.strName,tmp->data.dwNum,tmp->data.dwNum,1,0,NULL,user->id,user->name,"����",tmp->base,tmp->data.kind,tmp->data.upgrade);
        new_obs.push_back(tmp);    
      }else {
        //maybe package is full,stop process,but still return true
        zObject::destroy(tmp);  
        return 1;
      }
    }
    number -= capacity;
  }
  
  return 2;
}

/**
 * \brief ������Ʒid������Ʒ�ڰ����е�����
 * \param user: Ҫ�ı�������û�
 * \param id: ��Ʒ��objectid
 * \param number: ���ٵ�����
 * \param update_ob: �������ı����Ʒ,���û����Ʒ�������ı�,�ò���������
 * \param del_obs: ��ɾ������Ʒ�б�
 * \param upgrade: ��Ʒ�ȼ�
 * \return ʧ�ܷ���-1,���򷵻�0
 */
int UserObjectM::reduceObjectNum(SceneUser* user,DWORD id,DWORD number,zObject*& update_ob,ObjID_vec& del_obs,BYTE upgrade)
{
  //check if exist this object id
	zObjectB* o = objectbm.get(id);
	if (!o) {
		return -1;
	}

	for(hashmap::iterator it=ets.begin();it!=ets.end();)
	{
		zObject* ob = (zObject *)it->second;
		if (ob->data.pos.loc()!=Cmd::OBJECTCELLTYPE_COMMON && ob->data.pos.loc()!=Cmd::OBJECTCELLTYPE_PACKAGE) {
			++it;
			continue;
		}
		if (ob->data.dwObjectID == id && ob->data.kind == upgrade ) 
		{
			if (number >= ob->data.dwNum) {
				hashmap::iterator tmp = it;
				++tmp;
				number -= ob->data.dwNum;
				del_obs.push_back(ob->data.qwThisID);
				user->packs.removeObject(ob,false,true); //delete but not notify
				it = tmp;
				continue;
			}
			else 
			{
				ob->data.dwNum -= number;
				number = 0;
				update_ob = ob;
				return 0;
			}
		}
		++it;
	}
  
  return number?-1:0;  
}
