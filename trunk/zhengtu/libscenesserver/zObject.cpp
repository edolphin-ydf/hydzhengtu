#include <zebra/ScenesServer.h>

/**
* \brief ���캯��
*/
zObject::zObject():zEntry()
{
	createtime=time(NULL);
	base=NULL;
	inserted=false;
	bzero(&data,sizeof(data));
	data.fivetype = FIVE_NONE;
	fill(data);
}

/**
* \brief ���ɶ���ID
*/
void zObject::generateThisID()
{
	id=randBetween(0,1)?randBetween(-1000,0x80000000):randBetween(1000,0x7FFFFFFE);
	data.qwThisID=id;
}

/**
* \brief ������ȫ����Ʒ������ɾ���Լ�,������������
*/
zObject::~zObject()
{
	if (inserted)
	{
		goi->removeObject(this->id);
		inserted=false;
		createtime=0;
		base=NULL;
		bzero(&data,sizeof(data));
	}
}

void zObject::fill(t_Object& d)
{
	for (int i=0; i<5; ++i) {
		_p1[i] = &d.str + i;
	}

	for(int i=0; i<16; ++i) {
		_p2[i] = &d.pdam + i;
	}

	for(int i=0; i<2; ++i) {
		_p2[i+16] = &d.atrating + i;
	}

}

void zObject::checkBind()
{
	switch(base->kind)
	{
	case ItemType_DoubleExp:
	case ItemType_Honor:
	case ItemType_ClearProperty:
	case ItemType_KING:
	case ItemType_FAMILY:
	case ItemType_TONG:
		{
			data.bind=1;
		}
		break;
	default:
		break;
	}
	if (base->id== 881 || base->id == 882)
	{
		data.bind=1;
	}
}
DWORD zObject::RepairMoney2RepairGold(DWORD dwObjectRepair)
{
	return (DWORD)((float)(dwObjectRepair/50)+0.99);
}

/**
* \brief ��Ʒlog
* \param createid   ��Ʒ����id
* \param objid   ��ƷThisid
* \param objname   ��Ʒ����
* \param num     ��Ʒ����
* \param change  ��Ʒ����
* \param type     �仯����(2��ʾ���߼���,1��ʾ��,0��ʾ��)
* \param srcid   Դid
* \param srcname    Դ����
* \param dstid   Ŀ��id
* \param dstname   Ŀ������
* \param action   ����
*
void zObject::logger(QWORD createid,DWORD objid,char *objname,DWORD num,DWORD change,DWORD type,DWORD srcid,char *srcname,DWORD dstid,char *dstname,const char *action)
{
*
char _objname[MAX_NAMESIZE + 1];
char _srcname[MAX_NAMESIZE + 1];
char _dstname[MAX_NAMESIZE + 1];
char _action[MAX_NAMESIZE + 1];
bzero(_objname,sizeof(_objname));
bzero(_srcname,sizeof(_srcname));
bzero(_dstname,sizeof(_dstname));
bzero(_action,sizeof(_action));
if (objname)
{
strncpy(_objname,objname,MAX_NAMESIZE +1);
}
else
{
strncpy(_objname,"NULL",MAX_NAMESIZE +1);
}
if (srcname)
{
strncpy(_srcname,srcname,MAX_NAMESIZE +1);
}
else
{
strncpy(_srcname,"NULL",MAX_NAMESIZE +1);
}
if (dstname)
{
strncpy(_dstname,dstname,MAX_NAMESIZE +1);
}
else
{
strncpy(_dstname,"NULL",MAX_NAMESIZE +1);
}
if (action)
{
strncpy(_action,action,MAX_NAMESIZE +1);
}
else
{
strncpy(_action,"NULL",MAX_NAMESIZE +1);
}
// /
ScenesService::objlogger->debug("%I64u,%u,%s,%u,%u,%u,%u,%s,%u,%s,%s",createid,objid,objname,num,change,type,srcid,srcname,dstid,dstname,action);
}*/
/**
* \brief ��Ʒlog
* \param createid      ��Ʒ����id
* \param objid         ��ƷThisid
* \param objname       ��Ʒ����
* \param num           ��Ʒ����
* \param change        ��Ʒ����
* \param type          �仯����(2��ʾ���߼���,1��ʾ��,0��ʾ��)
* \param srcid         Դid
* \param srcname       Դ����
* \param dstid         Ŀ��id
* \param dstname       Ŀ������
* \param action        ����
* \param base          ��Ʒ���������ָ��
* \param kind          ��Ʒ������
* \param upgrade       ��Ʒ�ĵȼ��������Ĵ�����
* \brief    ���к�����������������ӡ,׷�ӵ���־��,����װ������ɫ,�Ǽ�,���ϵĵȼ�,��ʯ�ĵȼ�
*/
void zObject::logger(QWORD createid,DWORD objid,char *objname,DWORD num,DWORD change,DWORD type,DWORD srcid,char *srcname,DWORD dstid,char *dstname,const char *action,zObjectB *base,BYTE kind,BYTE upgrade)
{
	char tmpInfo[60] = {0};
	char *p = tmpInfo;

	if (srcname != NULL)
		strcat(tmpInfo,srcname);

	if (base != NULL)
	{
		switch (base->kind)
		{
		case ItemType_ClothBody:                //101�����ʼ��������װ
		case ItemType_FellBody:                 //102����Ƥ�׼�ħ�����װ
		case ItemType_MetalBody:                //103����������׼�������װ
		case ItemType_Blade:                    //104����������������
		case ItemType_Sword :                   //105����������������
		case ItemType_Axe:                      //106����������������
		case ItemType_Hammer:                   //107����������������
		case ItemType_Staff:                    //108��������������
		case ItemType_Crossbow:                 //109���������������
		case ItemType_Fan:                      //110������Ů����
		case ItemType_Stick:                    //111�����ٻ���������
		case ItemType_Shield:                   //112���������
		case ItemType_Helm:                     //113�����ɫͷ����
		case ItemType_Caestus:                  //114�����ɫ������
		case ItemType_Cuff:                     //115�����ɫ������
		case ItemType_Shoes:                    //116�����ɫЬ����
		case ItemType_Necklace:                 //117�����ɫ������
		case ItemType_Fing:                     //118�����ɫ��ָ��
			{
				switch(kind)
				{
				case 0:
					strcat(p + strlen(tmpInfo),":");
					strcat(tmpInfo,"��ɫ:");
					break;
				case 1:
					strcat(p + strlen(tmpInfo),":");
					strcat(tmpInfo,"��ɫ:");
					break;
				case 2:
					strcat(p + strlen(tmpInfo),":");
					strcat(tmpInfo,"��ɫ:");
					break;
				case 4:
					strcat(p + strlen(tmpInfo),":");
					strcat(tmpInfo,"��ɫ:");
					break;
				default:
					break;
				}
				sprintf(p + strlen(tmpInfo),"%d",upgrade);
			}
			break;

		case ItemType_Resource: //16����ԭ���� 
			{
				switch(base->id)
				{
					//��������Ҫ��ʾ�ȼ�����ƷID
				case 506:
				case 507:
				case 516:
				case 517:
				case 526:
				case 527:
				case 536:
				case 537:
				case 546:
				case 547:
				case 877:
					{
						strcat(p + strlen(tmpInfo),":");
						strcat(tmpInfo,"����:");
						sprintf(p + strlen(tmpInfo),"%d",(upgrade+1));

					}
				default:
					break;

				}
			}
			break;
		case ItemType_LevelUp:       //27�������������Ҫ�Ĳ�����
			{
				switch(base->id)
				{
				case 678:
				case 679:
					{
						strcat(p + strlen(tmpInfo),":");
						strcat(tmpInfo,"��ʯ:");
						sprintf(p + strlen(tmpInfo),"%d",(upgrade+1));
					}
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}
	ScenesService::objlogger->debug("%I64u,%u,%s,%u,%u,%u,%u,%s,%u,%s,%s",createid,objid,objname,num,change,type,srcid,tmpInfo,dstid,dstname,action);
}
/**
* \brief ������Ʒ������һ���µ���Ʒ����
* \param objsrc ���ն���
* \return ʧ�ܷ���NULL ���򷵻����ɵĶ���
*/
zObject *zObject::create(zObject *objsrc)
{
	if (objsrc==NULL) return NULL;
	zObject *ret=new zObject();
	if (ret)
	{
		strncpy(ret->name,objsrc->name,MAX_NAMESIZE);
		ret->tempid=objsrc->id;
		ret->base=objsrc->base;
		bcopy(&objsrc->data,&ret->data,sizeof(ret->data),sizeof(ret->data));
		ret->generateThisID();
		ret->free(true);

		if (!goi->addObject(ret))
		{
			SAFE_DELETE(ret);
		}
		else
		{
			ret->dwCreateThisID=ret->data.qwThisID;
			ret->inserted=true;
		}	
	}
	return ret;
}

//[Shx ���������װ����,] 
void zObject::MakeSuit(zObjectB *objbase)
{
	FillSuit();
	if(objbase->nSuitData > -1)//�Ƿ���װ
	{
		FillSuitPPT(objbase->nSuitData);
	}
}


//��ʼ���
void zObject::FillSuit()
{
	ZeroMemory( &data.SuitAttribute, sizeof(data.SuitAttribute));
}
//�������
void zObject::FillSuitPPT(int nIndex)
{
	_Object::Suit_Attribute* ppt = &(data.SuitAttribute);
	stxml_SuitAttribute& rList = vXmlSuitAttribute[nIndex];
	
	ppt->Suit_ID = rList.id;
	strncpy( ppt->Suit_name, rList.Name, sizeof(ppt->Suit_name));
	memcpy( ppt->PartList, rList.MemberList, sizeof(ppt->PartList), sizeof( rList.MemberList));
	ppt->nPart = rList.count;
	ppt->nEffect = rList.eCount;
	int ii = 0;
	for( ;ii < rList.eCount && ii < MAX_SUIT_NUM ;  ii ++)
	{
		st_SuitEffect * peList = &(rList.EffectList[ii]);

		ppt->EffectList[ii].eKey = peList->eKey;
		ppt->EffectList[ii].eValue = peList->eValue;
		ppt->EffectList[ii].eRequire = peList->eRequire;
	}
	ppt->nEffect = ii;
}
//[End Shx]

void  zObject::destroy(zObject* ob)
{
	assert(!ob || ob->free());
	SAFE_DELETE(ob);
}

const stObjectLocation &zObject::reserve() const
{
	return data.pos;
}

void zObject::restore(const stObjectLocation &loc)
{
	data.pos = loc;
}

bool zObject::free() const
{
	return data.pos == Object::INVALID_POS || data.pos.loc() == Cmd::OBJECTCELLTYPE_NONE;
}

void zObject::free(bool flag)
{
	data.pos = Object::INVALID_POS;
}

//sky ������ƷTBL����������ʵ����Ʒ����

/**
* \brief ������Ʒ�ֵ䴴��һ����Ʒ����  
* \param objbase ��Ʒ�ֵ�
* \param num ��Ʒ������
* \param level ��Ʒ�ļ���
* \return ��Ʒ����
*/
zObject *zObject::create(zObjectB *objbase,DWORD num,BYTE level)
{
	if (objbase==NULL) return NULL;

	//only money can have 0
	if ((num == 0 && objbase->id != 665) || num > objbase->maxnum) {
		return NULL;
	}

	zObject *ret=new zObject();
	if (ret)
	{
		ret->base = objbase;
		ret->generateThisID();
		strncpy(ret->name,objbase->name,MAX_NAMESIZE);
		strncpy(ret->data.strName,objbase->name,MAX_NAMESIZE);
		/*    
		ret->data.pos.dwLocation = Cmd::OBJECTCELLTYPE_COMMON;
		*/
		ret->data.dwObjectID = objbase->id;

		ret->free(true);
		ret->tempid = objbase->id;
		ret->data.dwNum = num;
		ret->data.color = objbase->color;
		switch (objbase->kind)
		{
		case ItemType_ClothBody:    //���������װ
		case ItemType_FellBody:        //����Ƥ�����װ
		case ItemType_MetalBody:    //��������������װ
		case ItemType_Blade:        //����������������
		case ItemType_Sword :          //����������������
		case ItemType_Axe:             //����������������
		case ItemType_Hammer:          //����������������
		case ItemType_Staff:        //��������������
		case ItemType_Crossbow:          //���������������
		case ItemType_Fan:             //������Ů����
		case ItemType_Stick:          //�����ٻ���������
		case ItemType_Shield:  //���������
		case ItemType_Helm:    //�����ɫͷ����
		case ItemType_Caestus:  //�����ɫ������
		case ItemType_Cuff:    //�����ɫ����
		case ItemType_Shoes:    //�����ɫЬ�Ӳ�
		case ItemType_Necklace:  //�����ɫ������
		case ItemType_Fing:    //�����ɫ��ָ��
			/*sky �������Ƥ���ͷ���֧��*/
		case ItemType_Helm_Paper: //ͷ��Ƥ
		case ItemType_Helm_Plate: //ͷ����
		case ItemType_Cuff_Paper: //����Ƥ
		case ItemType_Cuff_Plate: //�����
		case ItemType_Caestus_Paper: //����Ƥ
		case ItemType_Caestus_Plate: //������
		case ItemType_Shoes_Paper: //ѥ��Ƥ
		case ItemType_Shoes_Plate: //ѥ�Ӱ�

		//sky ������� ���� ������
		case tyItemType_Shoulder:
		case tyItemType_Gloves:
		case tyItemType_Pants:
		case ItemType_Shoulder_Paper:
		case ItemType_Gloves_Paper:
		case ItemType_Pants_Paper:
		case ItemType_Shoulder_Plate:
		case ItemType_Gloves_Plate:
		case ItemType_Pants_Plate:

		case ItemType_FashionBody:    //����ʱװ
			ret->data.color = randBetween(0xFF000000,0xFFFFFFFF);
			break;
		case ItemType_HighFashionBody:  //124����߼�ʱװ
			ret->data.color = randBetween(0xFF000000,0xFFFFFFFF);
			break;
		}

		ret->data.needlevel = objbase->needlevel;        // ��Ҫ�ȼ�

		ret->data.maxhp = objbase->maxhp;          // �������ֵ
		ret->data.maxmp = objbase->maxmp;          // �����ֵ
		ret->data.maxsp = objbase->maxsp;          // �������ֵ

		ret->data.pdamage = objbase->pdamage;        // ��С������
		ret->data.maxpdamage = objbase->maxpdamage;      // ��󹥻���
		ret->data.mdamage = objbase->mdamage;        // ��С����������
		ret->data.maxmdamage = objbase->maxmdamage;      // �����������

		ret->data.pdefence = objbase->pdefence;        // ���
		ret->data.mdefence = objbase->mdefence;        // ħ��
		ret->data.damagebonus = objbase->damagebonus;      // �˺��ӳ�

		ret->data.akspeed = objbase->akspeed;        // �����ٶ�
		ret->data.mvspeed = objbase->mvspeed;        // �ƶ��ٶ�
		ret->data.atrating = objbase->atrating;        // ������
		ret->data.akdodge = objbase->akdodge;        // �����
		ret->data.bang = objbase->bang;
		ret->data.dur = objbase->durability;
		ret->data.maxdur = objbase->durability;
		ret->data.price = objbase->price;
		ret->data.cardpoint = objbase->cardpoint;

		ret->data.upgrade = level;                // ��ʼ�ȼ�

		//sky ���ɶ�����ǰ��¼ӵ�����Ҳ����ȥ
		ret->data.str	= objbase->str;
		ret->data.inte	= objbase->inte;
		ret->data.dex	= objbase->dex;
		ret->data.spi	= objbase->spi;
		ret->data.con	= objbase->con;

		//sky ֱ�����ɿ�
		ret->foundSocket();

		//Shx Add�����װ����(�����)
		ret->MakeSuit(objbase);	

		if (!goi->addObject(ret))
		{
			SAFE_DELETE(ret);
		}
		else
		{
			ret->dwCreateThisID=ret->data.qwThisID;
			ret->inserted = true;
		}
	}else {
		Zebra::logger->debug("������Ʒ%sʧ��",objbase->name);
	}

	return ret;
}

/**
* \brief �ӵ�������������Ʒ
*
* \param o �ӵ����������ж�������Ʒ
*
* \return load�ɹ����ظ���Ʒ,���򷵻�NULL
*/
zObject *zObject::load(const SaveObject *o)
{
	if (o==NULL) return NULL;
	zObjectB *objbase = objectbm.get(o->object.dwObjectID);
	if (objbase==NULL) 
	{
		Zebra::logger->error("������Ʒʧ��,���߻������в�����:%d",o->object.dwObjectID);
		return NULL;
	}
	int i=0;
	zObject *ret=NULL; 
	while(!ret && i < 100)
	{
		ret=new zObject();
		if (i > 1)
		{
			Zebra::logger->error("����new���µ���Ʒ�������:%d",i);
		}
		i ++;
	}
	if (ret == NULL) 
	{
		Zebra::logger->error("������Ʒʧ��,new��Ʒ����ʧ��:%d",o->object.dwObjectID);
		return ret; 
	}
	bcopy(&o->object,&ret->data,sizeof(t_Object),sizeof(ret->data));
	ret->createid = o->createid;
	ret->id = ret->data.qwThisID;
	ret->tempid = ret->data.dwObjectID;
	strncpy(ret->name,ret->data.strName,MAX_NAMESIZE);
	ret->base=objbase;

	if (!goi->addObject(ret))
	{
		SAFE_DELETE(ret);
	}
	else
		ret->inserted=true;
	return ret;
}
/**
* \brief �õ���Ʒ����ʱ��,�浵ʱʹ��
*
* \return ��Ʒ����ʱ��
*/
bool zObject::getSaveData(SaveObject *save)
{
	bcopy(&data,&save->object,sizeof(t_Object),sizeof(save->object));

	save->createid =  createid;
	//Zebra::logger->error("[����] 1 %s = %s,createid = %ld",save->object.strName,data.strName,createid);
	return true;
}

/**
* \brief ������Ʒ��

* \return �׵���Ŀ
*/
int zObject::foundSocket()
{
	WORD socket = 0;

	//sky�Ȱѿ׽ṹȫ����0��������Ϊ���ɼ�
	for(int i=0; i<SOCKET_MAXNUM; i++)
	{
		memset(&(data.Hsocket[i]), 0, sizeof(GemPop) );
		data.Hsocket[i].GemID = INVALID_HOLE;
		data.Hsocket[i].M_State = true; //sky ����ȫ���׶�Ϊ����
	}

	//sky�ٸ��ݼ������ɿ��׵���Ŀ
	socket = base->hole.size();

	if(socket > SOCKET_MAXNUM)
		socket = SOCKET_MAXNUM;

	for( int k=(socket-1); k>=0; k--)
	{
		if (selectByTenTh(base->hole[k])) 
		{
			for( int j=0; j<k+1; j++)
			{
				data.Hsocket[j].GemID = EMPTY_HOLE; //sky���ݿ�����������Ϊ�ɼ�
			}

			break;
		}
	}

	return socket;
}

/**
* \brief ������Ʒ�Ƿ�����ʼ�

* \return �Ƿ����
*/
bool zObject::canMail()
{
	if (data.bind || data.dwObjectID == 800 || base->kind == ItemType_Quest || data.dwObjectID == 734 )
		return false;
	if (data.pos.loc() !=Cmd::OBJECTCELLTYPE_COMMON 
		&& data.pos.loc() !=Cmd::OBJECTCELLTYPE_PACKAGE
		&& data.pos.loc() !=Cmd::OBJECTCELLTYPE_PET)
		return false;
	return true;
}

/**
* \brief ������Ʒ�Ƿ���Ա�����

* \return �Ƿ����
*/
zCountryMaterialB* zObject::canContribute()
{
	zCountryMaterialB* country_material = NULL;
	country_material = countrymaterialbm.get(data.dwObjectID+base->kind);
	return country_material;
}

/**
* \brief ������Ʒԭ�����

* \return ���
*/
DWORD zObject::getMaterialKind()
{
	DWORD ret = 0;

	if (base->kind == ItemType_Resource)
	{
		if (data.dwObjectID == 501 || data.dwObjectID == 502 || data.dwObjectID == 506 || data.dwObjectID == 507)
		{
			ret = 1;
		}
		else if (data.dwObjectID == 511 || data.dwObjectID == 512 
			|| data.dwObjectID == 516 || data.dwObjectID == 517)
		{
			ret = 2;
		}
		else if (data.dwObjectID == 521 || data.dwObjectID == 522 
			|| data.dwObjectID == 526 || data.dwObjectID == 527)
		{
			ret = 3;
		}
		else if (data.dwObjectID == 531 || data.dwObjectID == 532 
			|| data.dwObjectID == 536 || data.dwObjectID == 537)
		{
			ret = 4;
		}
		else if (data.dwObjectID == 541 || data.dwObjectID == 542
			|| data.dwObjectID == 546 || data.dwObjectID == 547)
		{
			ret = 5;
		}
		else if (data.dwObjectID == 554 || data.dwObjectID == 555
			|| data.dwObjectID == 559 || data.dwObjectID == 560)
		{
			ret = 6;
		}
	}

	return ret;
}


/**
* \brief ��������
*/
Package::Package(WORD type,DWORD id,WORD w,WORD h):_type(type),_id(id),_width(w),_height(h),_space(w*h),_size(_space)

{
	WORD cap = _size;
	if (cap == 0) cap = 1;
	container = new zObject* [cap];
	memset(container,0,sizeof(zObject*)*cap);
}

Package::~Package()
{
	removeAll();
	SAFE_DELETE_VEC(container);
}

bool Package::checkAdd(SceneUser *pUser,zObject *object,WORD x,WORD y)
{
	if (object==NULL) return true;
	zObject *temp;
	return getObjectByZone(&temp,x,y);
}

bool Package::add(zObject* object,bool find)
{
	if (!object  || object->base->kind==ItemType_Money)
		return false;

	if (find)  {
		if (find_space(object->data.pos.x,object->data.pos.y))  {
			return add(object,false);
		}else {
			//Zebra::logger->warn("����[%d:%d]���Ҳ����ռ�����Ʒ[%x]",_type,_id,object);
			return false;
		}  
	}

	if (!find)  {
		int pos = position(object->data.pos.xpos(),object->data.pos.ypos());
		if (pos == -1 || pos >= size()) {
			Zebra::logger->warn("����[%d:%d]�������Ʒ[%x]ʱ����[%d]����",_type,_id,object,pos);
			return false;
		}
		if (container[pos]) { 
			//shouldn't be reached at all
			Zebra::logger->warn("����[%d]��[%d,%d]������Ʒ%x,���ܴ����Ʒ%x",_id,object->data.pos.x,object->data.pos.y,container[pos],object);
			return false;
		}else {

			//  assert(!container[pos]);
			container[pos] = object;
			--_space;
			return true;
		}
	}

	//never reach
	return false;

}

bool Package::find_space(WORD &x,WORD &y) const
{
//[Shx Modify ��Ϊ������]
// 	for (int i=0; i<_width; ++i)
// 		for(int j=0; j<_height; ++j) 	
	for(int j=0; j<_height; ++j) 
		for (int i=0; i<_width; ++i)	
			if (!container[j*_width+i]) {
				x = i;
				y = j;
				return true;
			}
//End

			return false;
}

int Package::position(WORD x,WORD y) const
{
	return y*_width + x;
}


bool Package::remove(zObject *object)
{
	if (object)
	{
		int pos = position(object->data.pos.xpos(),object->data.pos.ypos());
		if (pos == -1 || pos >= size() || container[pos] != object) {
			Zebra::logger->warn("����[%d:%d]��ɾ����Ʒ[%x]ʱ����[%d]����",_type,_id,object,pos);
			return false;
		}

		object->free(true);
		container[pos] = NULL;
		++_space;
		return true;
	}

	return false;
}

void Package::removeAll()
{
	for (int i=0; i<size(); ++i) {
		SAFE_DELETE(container[i]);
	};
}

bool Package::getObjectByZone(zObject **ret,WORD x,WORD y)
{
	int pos = position(x,y);
	if (pos >= size() || pos == -1) return false;

	*ret = container[pos];
	return true;
}

bool Package::getObjectByID(zObject **ret,DWORD id)
{
	for (int i=0; i<size(); ++i)
	{
		if (!container[i]) continue;

		if (container[i]->base->id==id)
		{
			*ret = container[i];
			return true;
		}
	}
	return false;
}

void Package::execEvery(PackageCallback &callback)
{
	for (int i=0; i<size(); ++i) {
		if (!container[i]) continue;
		if (!callback.exec(container[i])) break;
	}
}

WORD Package::space() const
{
	return _space;
}

WORD Package::size() const
{
	return _size;
}

/**
* \brief ��ȡ��������
* \return ��������
*/
WORD Package::type() const
{
	return _type;
}

/**
* \brief ��ȡ����ID
* \return ����ID
*/
DWORD Package::id() const
{
	return _id;
}

void Package::setSpace(WORD s)
{
	_space = s;
}
#if 0
/**
* \brief ����
* \param type ����
* \param id ���
* \param width ���
* \param height �߶�
*/
MultiPack::MultiPack(WORD type,DWORD id,WORD width,WORD height):Package(type,id)
{
	this->width=width;
	this->height=height;
	grid.resize(height);
	for(int i = 0; i < height; i++)
	{
		grid[i].resize(width);
		for(int j=0;j<width;j++)
			grid[i][j]=NULL;
	}
}

/**
* \brief ��������
*/
MultiPack::~MultiPack()
{
	for(std::set<zObject *>::iterator it=allset.begin();it!=allset.end();it++)
	{
		zObject *o = *it;
		SAFE_DELETE(o);
	}
	allset.clear();
	for(int i = 0; i < height; i++)
		for(int j=0;j<width;j++)
			grid[i][j]=NULL;
}

/**
* \brief ���װ��
* \param object ��Ʒ����
* \param find �Ƿ��Զ�Ѱ��λ��
* \return ��ӳɹ�����true ���򷵻�false
*/
bool MultiPack::add(zObject *object,bool find)
{ 
	if (object==NULL || object->base->kind==ItemType_Money)
		return false;
	if (find)
	{
		if (findSpace(object->base->width,object->base->height,object->data.pos.x,object->data.pos.y))
		{
			return add(object,false);
		}
	}
	else if (object && object->data.pos.x+object->base->width<=width  && object->data.pos.y+object->base->height <=height)
	{
		object->data.pos.dwLocation=type;
		object->data.pos.dwTableID=id;
		WORD i = 0;
		WORD j = 0;
		for(i=object->data.pos.x;i<object->data.pos.x + object->base->width;i++)
			for(j=object->data.pos.y;j<object->data.pos.y + object->base->height;j++) 
				grid[j][i]=object;
		allset.insert(object); 
		/*
		if (!allset.insert(object).second && object->base->width == 1 && object->base->height == 1)
		{
		Zebra::logger->debug("�ظ�������Ʒ");
		grid[j][i]=0;
		}
		// */
		/*
		for(WORD j=0;j<height;j++)
		{
		printf("\n");
		for(WORD i=0;i<width;i++)
		if (grid[j][i]==NULL)
		printf("0");
		else
		printf("*");
		}
		printf("\n");
		// */
		return true;
	}
	return false;
}

/**
* \brief �����Ʒ
* \param pUser ��ɫ
* \param object ��Ʒ����
* \param x ������
* \param y ������
* \return true װ���д���Ʒ
*/
bool MultiPack::checkAdd(SceneUser *pUser,zObject *object,WORD x,WORD y)
{
	if (object==NULL) return true;
	zObject *temp;
	return getObjectByZone(&temp,x,y,object->base-> width,object->base-> height);
}

/**
* \brief �Ӱ�����ɾ����Ʒ
* \param object Ҫ����Ķ���
*/
void MultiPack::remove(zObject *object)
{
	if (object)
	{
		object->data.pos.dwLocation=Cmd::OBJECTCELLTYPE_NONE;
		object->data.pos.dwTableID=0;
		allset.erase(object);
		int count=0;
		for(WORD i=0;i<width;i++)
			for(WORD j=0;j<height;j++) {
				if (grid[j][i]==object)
				{
					count++;
					grid[j][i]=NULL;
				}
			}
			if (count>1) {
				Zebra::logger->fatal("��Ʒ(%s)����(%d,%d)�����������о���",object->name,object->data.pos.x,object->data.pos.y);
				return;
			}
			/*
			if (i>=width || j >=height) {
			Zebra::logger->fatal("��Ʒ(%s)�������(%d,d)",object->name,object->data.pos.x,object->data.pos.y);
			return;
			}
			for(WORD i=object->data.pos.x;i<object->data.pos.x+object->base->width;i++)
			for(WORD j=object->data.pos.y;j<object->data.pos.y+object->base->height;j++) {
			if (i>=width || j >=height) {
			Zebra::logger->fatal("��Ʒ(%s)�������(%d,d)",object->name,object->data.pos.x,object->data.pos.y);
			return;
			}
			if (grid[j][i]==object) grid[j][i]=NULL;
			}
			*/
	}
}

/**
* \brief ��հ���
* \param object Ҫ����Ķ���
*/
void MultiPack::removeAll()
{
	for (std::set<zObject *>::iterator it=allset.begin(); it!=allset.end(); ++it) 
	{
		remove(*it);
	}
	allset.clear();
}

/**
* \brief  ����λ���õ�����
* \param ret �ش��õ��Ķ���
* \param x   ������
* \param y   ������
* \param w   ���
* \param h   �߶�
* \return true �õ���Ʒ,false ������ܸ��Ӳ�ֻһ����Ʒ
*/
bool MultiPack::getObjectByZone(zObject **ret,WORD x,WORD y,WORD w,WORD h)
{
	*ret=NULL;
	if (x+w>width || y+h>height) return false;
	int count=0;
	for(WORD i=x;i<x+w;i++)
		for(WORD j=y;j<y+h;j++)
			if (grid[j][i]!=*ret && grid[j][i]!=NULL)
			{
				count++;
				*ret=grid[j][i];
			}
			return (count <=1);
}

/**
* \brief ���ұ����ռ�
* \param owidth ��Ʒ��
* \param oheight ��Ʒ��
* \param ���ҵ���x����
* \param ���ҵ���y����
* \return true �ҵ����ʵ�λ��,false û�ҵ��ʺϵ�λ��
*/
bool MultiPack::findSpace(WORD owidth,WORD oheight,WORD &x,WORD &y)
{
	bool finded=false;
	WORD i=0,j=0;
	for(i=0;i<=width-owidth;i++)
	{
		for(j=0;j<=height-oheight;j++)
		{
			bool space=true;
			for(WORD w=0;w<owidth;w++)
			{
				for(WORD h=0;h<oheight;h++)
					if (grid[j+h][i+w]!=NULL)
					{
						space=false;
						break;
					}
					if (!space) break;
			}
			if (space)
			{
				finded=true;
				break;
			}
		}
		if (finded) break;
	}

	if (finded)
	{
		x=i;
		y=j;
	}
	return finded;
}

/**
* \brief ����,�շ���
* \param callback �ص�
*/
void MultiPack::execEvery(PackageCallback &callback)
{
	for(std::set<zObject *>::iterator it=allset.begin(); it!=allset.end(); ++it) {
		if (!callback.exec(*it)) break;
	}
}
#endif

/**
* \brief װ����������
* \param user ����������
*/
EquipPack::EquipPack(SceneUser* user):Package(Cmd::OBJECTCELLTYPE_EQUIP,0,1,20),owner(user)
{
	needRecalc=true;
	packs = new ObjectPack*[4];
	memset(packs,0,4*sizeof(ObjectPack*));
	doubleexp_obj=0; 
	doubleexp_obj_time=0; 
	family_obj_times=5;
	family_obj_time=0;
	tong_obj_times=2;
	tong_obj_time=0;
	king_obj_times=2;
	king_obj_time=0;
	effectCount=0;
}

/**
* \brief ��������
*/
EquipPack::~EquipPack()
{
	for (int i=0; i<4; ++i) {
		SAFE_DELETE(packs[i]);
	}
	SAFE_DELETE_VEC(packs);
}

bool EquipPack::isTonic()
{
	if (container[13] && container[13]->base->kind == ItemType_Tonic)
	{
		return true;
	}
	return false;
}
/**
* \brief ����װ�����;ö�
* \param pThis ��������
* \param value װ���;�ֵ
*/
void EquipPack::updateDurability(SceneUser *pThis,DWORD value)
{
	for (int i=0;i<16;i++)
	{
		if (container[i]!=NULL)
		{
			container[i]->data.dur = value>container[i]->data.maxdur?container[i]->data.maxdur:value;

			// [ranqd] ���ݲ߻�Ҫ���;ö�Ҫ���Խ���0
			//if (container[i]->data.dur == 0)
			//{
			//	container[i]->data.dur = 1;
			//}

			freshDurability(pThis,container[i]);

			/*
			if (equip[i]!=NULL && equip[i]->data.dur < equip[i]->data.maxdur)
			{
			equip[i]->data.dur = equip[i]->data.maxdur;
			}
			*/
		}
	}
}

/**
* \brief ��װ����Ϣ�buf��
* \param buf ���ؿռ�
* \return װ������
*/
DWORD EquipPack::fullAllEquiped(char *buf)
{
	using namespace Cmd;
	EquipedObject *data = (EquipedObject*)buf;
	int num = 0;
	for (int i =0;i<16;i++)
	{
		if (container[i]!=NULL)
		{
			bcopy(&container[i]->data,&data[num].object,sizeof(t_Object),sizeof(data[num].object));
			data[i].byWhere = i;
			num++;
		}
	}
	return num;
}

/**
* \brief �ָ��;ö�
* \param pThis ��ɫ
* \param ct ʱ��
*/
void EquipPack::restituteDurability(SceneUser *pThis,const zRTime &ct)
{

	for (int i=0;i<12;i++)
	{
		if (container[i]!=NULL)
		{
			if (container[i]!=NULL && container[i]->data.dur < container[i]->data.maxdur)
			{
				if (container[i]->data.dursecond > 0 && container[i]->data.durpoint > 0)
				{
					if (!(ct.sec() % container[i]->data.dursecond))
					{
						bool need = container[i]->data.dur == 0 ? true : false;
						container[i]->data.dur +=container[i]->data.durpoint;
						if (container[i]->data.dur > container[i]->data.maxdur)
						{
							container[i]->data.dur = container[i]->data.maxdur;
						}
						if (need)
						{
							freshDurability(pThis,container[i]);
							calcAll();
							freshUserData(pThis);
						}
					}
				}
			}
		}
	}

}

/**
* \brief ����װ���������Ӱ�첢֪ͨ�ͻ���
* \param 
* \return 
*/
void EquipPack::freshUserData(SceneUser *pThis)
{
	pThis->setupCharBase();
	Cmd::stMainUserDataUserCmd  userinfo;
	pThis->full_t_MainUserData(userinfo.data);
	pThis->sendCmdToMe(&userinfo,sizeof(userinfo));
}

/**
* \brief ˢ���;ö�,֪ͨ�ͻ����;öȱ仯
* \param pThis ��ɫ
* \param o ��Ʒ
*/
void EquipPack::freshDurability(SceneUser *pThis,zObject *o)
{
	Cmd::stDurabilityUserCmd std;
	std.dwThisID = o->data.qwThisID;
	std.dwDur = o->data.dur;
	std.dwMaxDur = o->data.maxdur;
	pThis->sendCmdToMe(&std,sizeof(std));
}

/**
* \brief �����;ö� 
* \param pThis ��ɫ
* \param which ����Ŀ��
*/
bool EquipPack::reduceDur(SceneUser *pThis,DWORD which)
{
	if (which > 15 || container[which] == NULL)
	{
		return false;
	}
	WORD olddur = (WORD)(container[which]->data.dur / 50) + 1;
	container[which]->data.dur --;
	if ((olddur != (WORD)(container[which]->data.dur / 50) + 1) || (container[which]->data.dur == 0))
	{
		freshDurability(pThis,container[which]);
	}
	if (container[which]->data.dur == 0)
	{
		calcAll();

		// [ranqd] ���ݲ߻�Ҫ��װ���;�Ϊ0ʱ����ʧ��ֻ�ǲ�������
		//if (container[which]->base->kind <= 118 && container[which]->base->kind >= 101 && container[which]->data.bind)
		//{
		//	return false;
		//}
		//else
		//{
		//	//��ʱ����,����QA����
		//	//�ٴθ��ݲ߻��ĵ��޸�
		//	zObject::logger(container[which]->createid,container[which]->data.qwThisID,container[which]->data.strName,container[which]->data.dwNum,1,0,pThis->id,pThis->name,0,NULL,"�;�����ɾ��",NULL,0,0);
		//	if (pThis->packs.removeObject(container[which]))
		//	{
		//		container[which]=NULL; 
		//		pThis->setupCharBase();
		//		Cmd::stMainUserDataUserCmd  userinfo;
		//		pThis->full_t_MainUserData(userinfo.data);
		//		pThis->sendCmdToMe(&userinfo,sizeof(userinfo));
		//		pThis->sendMeToNine();
		//	}

		//	return false;
		//}
	}
	return true;
}
zObject *EquipPack::getObjectByEquipPos(DWORD no)
{
	if (no < 16)
	{
		return container[no];
	}
	else
	{
		return NULL;
	}
}
zObject *EquipPack::getObjectByEquipNo(EQUIPNO no)
{
	return container[no];
}
/**
* \brief �����;�ֵ 
* \param pThis ��ɫ
* \param which ����Ŀ��
*
* \return ʵ�����ĵ��;�ֵ
*/
int EquipPack::reduceDur(SceneUser *pThis,DWORD which,DWORD type,DWORD num,bool needFresh,bool needCal)
{
	int ret = 0;
	if(which == 15)
	{
		which = 13;
	}
	if (which > 15 || container[which] == NULL)
	{
		return 0;
	}
	if (container[which]->base->kind != type)
	{
		return 0;
	}
	WORD olddur = (WORD)(container[which]->data.dur / 50) + 1;
	ret = (int)(container[which]->data.dur - num);
	if (ret > 0)
	{
		container[which]->data.dur = ret;
		ret = num;
	}
	else
	{
		ret = container[which]->data.dur;
		container[which]->data.dur = 0;
	}
	if (needFresh)
	{
		freshDurability(pThis,container[which]);
	}
	else
	{
		if (olddur != (WORD)(container[which]->data.dur / 50) + 1)
		{
			freshDurability(pThis,container[which]);
		}
	}
	if (container[which]->data.dur == 0)
	{
		if (needCal)
		{
			calcAll();
		}

		//��ʱ����,����QA����
		//�ٴθ��ݲ߻��ĵ��޸�
		if (container[which]->base->kind <= 118 && container[which]->base->kind >= 101 && container[which]->data.bind)
		{
			return false;
		}
		else
		{
			zObject::logger(container[which]->createid,container[which]->data.qwThisID,container[which]->data.strName,container[which]->data.dwNum,1,0,pThis->id,pThis->name,0,NULL,"�;�����ɾ��",NULL,0,0);
			if (pThis->packs.removeObject(container[which])) //notify and delete
			{
				container[which]=NULL; 
				pThis->setupCharBase();
				Cmd::stMainUserDataUserCmd  userinfo;
				pThis->full_t_MainUserData(userinfo.data);
				pThis->sendCmdToMe(&userinfo,sizeof(userinfo));
				pThis->sendMeToNine();
			}
		}

	}
	return ret;
}

/**
* \brief ���Ĺ�����װ���;ö�
* \param pThis ��ɫ
*/
bool EquipPack::costAttackDur(SceneUser *pThis)
{
	bool bret = false;
	//int jewelry = 0;
	//int which[3] ={0};
	//
	for (int i=0;i<16;i++)
	{
		if (container[i]!=NULL && container[i]->data.dur != 0)
		{
			switch(container[i]->base->kind)
			{
			case ItemType_Blade:        //104����������������
			case ItemType_Sword :          //105����������������
			case ItemType_Axe:            //106����������������
			case ItemType_Hammer:          //107����������������
			case ItemType_Staff:        //108��������������
			case ItemType_Crossbow:          //109���������������
			case ItemType_Fan:               //110������Ů����
			case ItemType_Stick:          //111�����ٻ���������
			case ItemType_Flower:      //120�����ʻ�,�ɼ�����...
				{
					if (selectByPercent(10))
					{
						if (container[i] && container[i]->base->recast && container[i]->data.dur<=50)
							pThis->petAutoRepair(container[i]);

						if (!reduceDur(pThis,i))
						{
							bret = true;
						}
					}
				}
				break;
				/*
				case ItemType_Necklace:
				case ItemType_Fing:
				{
				jewelry ++;
				if (jewelry <= 3)
				{
				which[jewelry - 1] = i;
				}
				}
				break;
				// */
			default:
				break;
			}
		}
	}

	/*
	for(int ii = 0 ; ii < jewelry ; ii ++)
	{
	if (selectByPercent(60))
	{
	if (!reduceDur(pThis,which[ii]))
	{
	bret = true;
	}
	}
	}
	// */
	/*
	switch(jewelry)
	{
	case 1:
	{
	if (!reduceDur(pThis,which[jewelry -1]))
	{
	bret = true;
	}
	}
	break;
	case 2:
	{
	jewelry -= randBetween(0,1);
	if (!reduceDur(pThis,which[jewelry - 1]))
	{
	bret = true;
	}
	}
	break;
	case 3:
	{
	jewelry -= randBetween(0,2);
	if (!reduceDur(pThis,which[jewelry - 1]))
	{
	bret = true;
	}
	}
	break;
	default:
	break;
	}
	// */
	if (bret)
	{
		freshUserData(pThis);
	}
	return bret;
}

/**
* \brief ���﹥�����Ĺ�����װ���;ö�
* \param pThis ��ɫ
*/
bool EquipPack::costAttackDurByPet(SceneUser *pThis)
{
	bool bret = false;
	//int jewelry = 0;
	//int which[3] ={0};
	//
	for (int i=0;i<16;i++)
	{
		if (container[i]!=NULL && container[i]->data.dur != 0)
		{
			switch(container[i]->base->kind)
			{
			case ItemType_Stick:          //111�����ٻ���������
				{
					if (selectByPercent(10))
					{
						if (container[i])
						{
							if (container[i]->base->level <= 70) continue;
							if (container[i]->data.dur<=50 && container[i]->base->recast)
								pThis->petAutoRepair(container[i]);
						}

						if (!reduceDur(pThis,i))
						{
							bret = true;
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (bret)
	{
		freshUserData(pThis);
	}
	return bret;
}

/**
* \brief ���ķ�����װ���;ö�
* \param pThis ��ɫ
*/
bool EquipPack::costDefenceDur(SceneUser *pThis)
{
	bool bret = false;
	int clothing = 0;
	int which[5] = {0};
	for (int i=0;i<16;i++)
	{
		if (container[i]!=NULL && container[i]->data.dur != 0)
		{
			switch(container[i]->base->kind)
			{
			case ItemType_ClothBody:    //101�����ʷ�װ
			case ItemType_FellBody:        //102����Ƥ�����װ
			case ItemType_MetalBody:    //103��������������װ
			case ItemType_Helm:    //113�����ɫͷ����
			case ItemType_Caestus:  //114�����ɫ������
			case ItemType_Cuff:    //115�����ɫ������
			case ItemType_Shoes:    //116�����ɫЬ���� 
				/*sky �������Ƥ���ͷ���֧��*/
			case ItemType_Helm_Paper: //ͷ��Ƥ
			case ItemType_Helm_Plate: //ͷ����
			case ItemType_Cuff_Paper: //����Ƥ
			case ItemType_Cuff_Plate: //�����
			case ItemType_Caestus_Paper: //����Ƥ
			case ItemType_Caestus_Plate: //������
			case ItemType_Shoes_Paper: //ѥ��Ƥ
			case ItemType_Shoes_Plate: //ѥ�Ӱ�

			//sky ������� ���� ������
			case tyItemType_Shoulder:
			case tyItemType_Gloves:
			case tyItemType_Pants:
			case ItemType_Shoulder_Paper:
			case ItemType_Gloves_Paper:
			case ItemType_Pants_Paper:
			case ItemType_Shoulder_Plate:
			case ItemType_Gloves_Plate:
			case ItemType_Pants_Plate:
				{
					/*
					if (clothing < 5)
					{
					which[clothing] = i;
					}
					// */
					which[clothing] = i;
					clothing ++;
				}
				break;
			case ItemType_Shield:
			case ItemType_Necklace:
			case ItemType_Fing:
				{
					if (selectByPercent(3))
					{
						if (container[i] && container[i]->base->recast && container[i]->data.dur<=50)
							pThis->petAutoRepair(container[i]);
						if (!reduceDur(pThis,i))
						{
							bret = true;
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
	for(int ii=0 ; ii < clothing ; ii ++)
	{
		if (selectByPercent(3))
		{
			if (container[which[ii]] && container[which[ii]]->base->recast && container[which[ii]]->data.dur<=50)
				pThis->petAutoRepair(container[which[ii]]);

			if (!reduceDur(pThis,which[ii]))
			{
				bret = true;
			}
		}
	}
	/*
	switch(clothing)
	{
	case 5:
	{
	int except_1 = randBetween(0,4);
	int except_2 = randBetween(0,4);
	while(except_2 == except_1)
	{
	except_2 = randBetween(0,4);
	}
	for(int i = 0 ; i < clothing ; i ++)
	{
	if (i != except_1 && i != except_2)
	{
	if (!reduceDur(pThis,which[i]))
	{
	bret = true;
	}
	}
	}
	}
	break;
	case 4:
	{
	int except = randBetween(0,3);
	for(int i = 0 ; i < clothing ; i ++)
	{
	if (i != except)
	{
	if (!reduceDur(pThis,which[i]))
	{
	bret = true;
	}
	}
	}
	}
	break;
	case 3:
	case 2:
	case 1:
	{
	for(int i = 0 ; i < clothing ; i ++)
	{
	if (!reduceDur(pThis,which[i]))
	{
	bret = true;
	}
	}
	}
	break;
	default:
	break;
	}
	// */
	if (bret)
	{
		freshUserData(pThis);
	}
	return bret;
}

/**
* \brief ���﹥�����ķ�����װ���;ö�
* \param pThis ��ɫ
*/
bool EquipPack::costDefenceDurByPet(SceneUser *pThis)
{
	bool bret = false;

	for (int i=0;i<16;i++)
	{
		if (container[i]!=NULL && container[i]->data.dur != 0)
		{
			switch(container[i]->base->kind)
			{
			case ItemType_Necklace:
			case ItemType_Fing:
				{
					if (selectByPercent(3))
					{
						if (container[i])
						{
							if (container[i]->base->level <=70) continue;
							if (container[i]->data.dur<=50 && container[i]->base->recast)
								pThis->petAutoRepair(container[i]);
						}
						if (!reduceDur(pThis,i))
						{
							bret = true;
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (bret)
	{
		freshUserData(pThis);
	}
	return bret;
}

/**
* \brief ��ȡ����װ��
* \return ����װ������
*/
const Equips& EquipPack::getEquips() const
{
	return equips;
}

#define CALCUTE(prop) equips.prop += container[i]->data.prop;

/**
* \brief ��������װ����ֵ
*/
void EquipPack::calcAll() 
{
	bool calcmdamage;
	equips.reset();
	//  Zebra::logger->debug("reset");
	typedef std::map<std::string,int> SuitNmaeMap;
	typedef SuitNmaeMap::iterator SuitNameMap_iter;
	SuitNmaeMap suitmap[3];
	std::pair<int,int> suitnum;
	suitnum.first=-1;
	suitnum.second=0;

	calcmdamage = true;
	if (container[position(0,Cmd::EQUIPCELLTYPE_HANDR)])
	{
		if (container[position(0,Cmd::EQUIPCELLTYPE_HANDR)]->base->kind == ItemType_Stick)
		{
			calcmdamage = false;
		}
	}
	for (int i=0;i<16;i++)
	{
		if (container[i]!=NULL && container[i]->data.dur != 0)
		{
			//�����ʱװ,����ֵ����5%
			if (container[i]->base->kind == ItemType_FashionBody || container[i]->base->kind == ItemType_HighFashionBody)
			{
				equips.maxhprate +=10;
				equips.maxmprate +=10;
			}
			if (container[i]->data.bind && container[i]->data.maker[0] && container[i]->base->kind != ItemType_Shield)
			{
				int color = -1;
				if (container[i]->data.kind & 1)
				{
					color = 0;
				}
				if (container[i]->data.kind & 2)
				{
					color = 1;
				}
				if (container[i]->data.kind & 4)
				{
					color = 2;
				}
				if (color != -1)
				{
					SuitNameMap_iter iter = suitmap[color].find(container[i]->data.maker);
					if (iter != suitmap[color].end())
					{
						iter->second ++;
						if (iter->second >=6 )
						{
							suitnum.first = color;
							suitnum.second = iter->second;
						}
					}
					else
					{
						suitmap[color].insert(std::make_pair(container[i]->data.maker,1));
					}
				}
			}
			CALCUTE(maxhp)          // �������ֵ
				CALCUTE(maxmp)          // �����ֵ
				CALCUTE(maxsp)          // �������ֵ

				if (container[i]->base->kind != ItemType_Crossbow || (container[i]->base->kind == ItemType_Crossbow && equip(HANDR)/*arrow must*/ )) {
					CALCUTE(pdamage)        // ��С������
						CALCUTE(maxpdamage)        // ��󹥻���
						CALCUTE(damagebonus)      // �˺��ӳ�
				}

				if (calcmdamage)
				{
					CALCUTE(mdamage)        // ��С����������
						CALCUTE(maxmdamage)        // �����������
				}
				else
				{
					equips.appendminpet+=container[i]->data.mdamage;
					equips.appendmaxpet+=container[i]->data.maxmdamage;
				}

				CALCUTE(pdefence)        // ���
					CALCUTE(mdefence)        // ħ��
					CALCUTE(damage)          // �����˺�ֵx��

					CALCUTE(akspeed)        // �����ٶ�
					CALCUTE(mvspeed)        // �ƶ��ٶ�
					CALCUTE(atrating)        // ������
					CALCUTE(akdodge)        // �����

					CALCUTE(str)            // ����
					CALCUTE(inte)            // ����
					CALCUTE(dex)            // ����
					CALCUTE(spi)            // ����
					CALCUTE(con)            // ����

					CALCUTE(hpr)          // ����ֵ�ָ�
					CALCUTE(mpr)            // ����ֵ�ָ�
					CALCUTE(spr)            // ����ֵ�ָ�

					CALCUTE(holy)           //��ʥһ��
					CALCUTE(bang)           //�ػ�
					CALCUTE(pdam)           // ������������
					CALCUTE(pdef)            // �������������
					CALCUTE(mdam)            // ����ħ��������
					CALCUTE(mdef)            // ����ħ��������

					CALCUTE(poisondef)         //��������
					CALCUTE(lulldef)         //���������
					CALCUTE(reeldef)         //��ѣ������
					CALCUTE(evildef)         //����ħ����
					CALCUTE(bitedef)         //����������
					CALCUTE(chaosdef)         //����������
					CALCUTE(colddef)         //����������
					CALCUTE(petrifydef)       //��ʯ������
					CALCUTE(blinddef)         //��ʧ������
					CALCUTE(stabledef)         //����������
					CALCUTE(slowdef)         //����������
					CALCUTE(luredef)         //���ջ�����

					CALCUTE(poison)         //�ж�����
					CALCUTE(lull)           //�������
					CALCUTE(reel)           //ѣ������
					CALCUTE(evil)           //��ħ����
					CALCUTE(bite)          //��������
					CALCUTE(chaos)           //��������
					CALCUTE(cold)           //��������
					CALCUTE(petrify)         //ʯ������
					CALCUTE(blind)           //ʧ������
					CALCUTE(stable)         //��������
					CALCUTE(slow)           //��������
					CALCUTE(lure)           //�ջ�����
					CALCUTE(hpleech.odds) 
					CALCUTE(hpleech.effect) 
					CALCUTE(mpleech.odds)       //x%��������ֵy
					CALCUTE(mpleech.effect)     //x%���շ���ֵy

					CALCUTE(hptomp)          //ת������ֵΪ����ֵx��
					CALCUTE(dhpp)           //�����˺�����x%  
					CALCUTE(dmpp)          //�����˺�ֵ����x%    

					CALCUTE(incgold)        //�������ӵ���x%
					CALCUTE(doublexp)        //x%˫������    
					if (container[i]->base->kind == ItemType_DoubleExp) {
						equips.doublexp=100;        //x%˫������    
			  }
					CALCUTE(mf)             //���ӵ�����x%

						//sky ���ɼӵ�ĵ���
						CALCUTE(Freedom.str_Attribute)
						CALCUTE(Freedom.inte_Attribute)
						CALCUTE(Freedom.dex_Attribute)
						CALCUTE(Freedom.spi_Attribute)
						CALCUTE(Freedom.con_Attribute)

						switch ( container[i]->base->kind) 
					{
						case ItemType_Blade:        //104����������������
						case ItemType_Sword :          //105����������������
						case ItemType_Axe:             //106����������������
						case ItemType_Hammer:          //107����������������
						case ItemType_Staff:        //108��������������
						case ItemType_Crossbow:          //109���������������
						case ItemType_Fan:             //110������Ů����
						case ItemType_Stick:          //111�����ٻ���������      
							equips.aftype = container[i]->data.fivetype;
							equips.afpoint = container[i]->data.fivepoint;

							break;
						case ItemType_Necklace:  //117�����ɫ������
						case ItemType_Fing:    //118�����ɫ��ָ��
							//note: be careful,weapon must be computed before this
							if (container[i]->data.fivetype == equips.aftype) {
								equips.afpoint += container[i]->data.fivepoint;
							}
							break;
						case ItemType_ClothBody:    //101���������װ
						case ItemType_FellBody:        //102����Ƥ�����װ
						case ItemType_MetalBody:    //103��������������װ
							equips.dftype = container[i]->data.fivetype;
							equips.dfpoint = container[i]->data.fivepoint;

							break;
						case ItemType_Shield:  //112�������
						case ItemType_Helm:    //113�����ɫͷ����
						case ItemType_Caestus:  //114�����ɫ������
						case ItemType_Cuff:    //115�����ɫ����
						case ItemType_Shoes:    //116�����ɫЬ�Ӳ�
							/*sky �������Ƥ���ͷ���֧��*/
						case ItemType_Helm_Paper: //ͷ��Ƥ
						case ItemType_Helm_Plate: //ͷ����
						case ItemType_Cuff_Paper: //����Ƥ
						case ItemType_Cuff_Plate: //�����
						case ItemType_Caestus_Paper: //����Ƥ
						case ItemType_Caestus_Plate: //������
						case ItemType_Shoes_Paper: //ѥ��Ƥ
						case ItemType_Shoes_Plate: //ѥ�Ӱ�

						//sky ������� ���� ������
						case tyItemType_Shoulder:
						case tyItemType_Gloves:
						case tyItemType_Pants:
						case ItemType_Shoulder_Paper:
						case ItemType_Gloves_Paper:
						case ItemType_Pants_Paper:
						case ItemType_Shoulder_Plate:
						case ItemType_Gloves_Plate:
						case ItemType_Pants_Plate:
							//note: be careful,armor must be computed before this
							if (container[i]->data.fivetype == equips.dftype) {
								equips.dfpoint += container[i]->data.fivepoint;
							}
							break;
					}

					int j = 0;

					while (j<10 && container[i]->data.skill[j].id && equips.getMaxSkill(container[i]->data.skill[j].id) < container[i]->data.skill[j].point) 
					{
						equips.skill[container[i]->data.skill[j].id] = container[i]->data.skill[j].point;
						++j;
					}

					if (container[i]->data.skills.id && equips.getMaxSkill(container[i]->data.skills.id) < container[i]->data.skills.point) 
					{
						equips.skills[container[i]->data.skills.id] = container[i]->data.skills.point;
					}
					j = 0;

					//sky ��װ���ṹ��ı�ʯ���ݿ���
					while( j<SOCKET_MAXNUM )
					{
						equips.gempop.push_back( container[i]->data.Hsocket[j] );
						++j;
					}

					int index = 0;
					int found[16];
					memset(found,0,sizeof(int)*16);

					if (container[i]->data.fivetype != FIVE_NONE) {
						for (int j=0; j<16; ++j)  {
							/*
							if (container[j] && container[j]->data.fivetype != FIVE_NONE) {
							Zebra::logger->debug("!!![%s]\t[%d]",container[j]->data.strName,j);
							}
							*/
							if (i!=j && container[j]!=NULL && container[j]->data.dur != 0 && container[i]->data.fiveset[index]==container[j]->data.dwObjectID  && !found[j] &&
								( ((container[i]->data.fivetype+3*(index+1))%5) == container[j]->data.fivetype) ) {        
									/*            
									switch (index) 
									{
									case 0:
									CALCUTE(dpdam)      //�����˺�����%x
									break;
									case 1:
									CALCUTE(dmdam)      //�����˺�����%x
									break;
									case 2:
									CALCUTE(bdam)        //�����˺�x%
									break;
									case 3:
									CALCUTE(rdam)        //�˺�����%x
									break;
									case 4:
									CALCUTE(ignoredef)    //%x����Ŀ�����
									break;
									}
									*/
									int k = 0;
									while (k < 5) {
										if (container[i]->data._five_props[k] ) {
											if (k>=index) break;
										}
										++k;
									}
									equips._five_props[k] += container[i]->data._five_props[k];
									//              Zebra::logger->debug("found");                                
									++index;
									found[j] = 1;
									j = 0; //loop again
							}
						}        
					}
		}
	}
	switch(suitnum.first)
	{
	case 0:
		{
			if (suitnum.second >= 10)
			{
				equips.pdam+=5;           // ������������
				equips.pdef+=5;            // �������������
				equips.mdam+=5;            // ����ħ��������
				equips.mdef+=5;            // ����ħ��������
				equips.maxhprate +=8;
				equips.maxmprate +=8;
			}
			else
			{
				equips.pdam+=2;           // ������������
				equips.pdef+=2;            // �������������
				equips.mdam+=2;            // ����ħ��������
				equips.mdef+=2;            // ����ħ��������
				equips.maxhprate +=4;
				equips.maxmprate +=4;
			}
		}
		break;
	case 1:
		{
			if (suitnum.second >= 10)
			{
				equips.pdam+=5;           // ������������
				equips.pdef+=5;            // �������������
				equips.mdam+=5;            // ����ħ��������
				equips.mdef+=5;            // ����ħ��������
				equips.maxhprate +=8;
				equips.maxmprate +=8;
			}
			else
			{
				equips.pdam+=2;           // ������������
				equips.pdef+=2;            // �������������
				equips.mdam+=2;            // ����ħ��������
				equips.mdef+=2;            // ����ħ��������
				equips.maxhprate +=4;
				equips.maxmprate +=4;
			}
		}
		break;
	case 2:
		{
			if (suitnum.second >= 10)
			{
				equips.pdam+=5;           // ������������
				equips.pdef+=5;            // �������������
				equips.mdam+=5;            // ����ħ��������
				equips.mdef+=5;            // ����ħ��������
				equips.maxhprate +=8;
				equips.maxmprate +=8;
			}
			else
			{
				equips.pdam+=2;           // ������������
				equips.pdef+=2;            // �������������
				equips.mdam+=2;            // ����ħ��������
				equips.mdef+=2;            // ����ħ��������
				equips.maxhprate +=4;
				equips.maxmprate +=4;
			}
		}
		break;
	default:
		break;
	}
	//Zebra::logger->debug("װ��������������(%d:%d),��������(%d:%d)",equips.aftype,equips.afpoint,equips.dftype,equips.dfpoint);
}

int EquipPack::position(WORD x,WORD y) const
{
	int pos = x*20 + y;
	if (pos < (int)sizeof(_poses) && _poses[pos] != -1) {
		return _poses[pos];
	}

	return -1;
}    

bool EquipPack::process_extra_add(zObject* ob)
{
	if (ob && (ob->data.pos.ypos() == Cmd::EQUIPCELLTYPE_MAKE || ob->data.pos.ypos() == Cmd::EQUIPCELLTYPE_PACKAGE)) {
		int pos = ((ob->data.pos.ypos() - Cmd::EQUIPCELLTYPE_PACKAGE) << 1)  + ob->data.pos.xpos();
		assert(pos >= 0 && pos <4);
		SAFE_DELETE(packs[pos]);
		packs[pos] = new ObjectPack(ob,ob->data.pos.ypos() -1 );
		return true;
	}

	return false;
}

bool EquipPack::process_extra_remove(zObject* ob)
{
	if (ob && (ob->data.pos.ypos() == Cmd::EQUIPCELLTYPE_MAKE || ob->data.pos.ypos() == Cmd::EQUIPCELLTYPE_PACKAGE)) {
		int pos = ((ob->data.pos.ypos() - Cmd::EQUIPCELLTYPE_PACKAGE) << 1)  + ob->data.pos.xpos();
		assert(pos >= 0 && pos <4);
		SAFE_DELETE(packs[pos]);
		return true;
	}

	return false;
}

bool EquipPack::process_extra_get(zObject** ob,WORD x,WORD y)
{
	if (y == Cmd::EQUIPCELLTYPE_MAKE ||y == Cmd::EQUIPCELLTYPE_PACKAGE) {
		int pos = ((y - Cmd::EQUIPCELLTYPE_PACKAGE) << 1)  + x;
		if (pos<4) {
			if (packs[pos] && packs[pos]->object() ) *ob = packs[pos]->object();
			return true;
		}
	}

	return false;
}

ObjectPack* EquipPack::pack(PACKNO no) const
{
	return packs[no];
}

zObject* EquipPack::equip(EQUIPNO no) const
{
	return container[no];
}

//const int EquipPack::_poses[] = {   -1,  1,  0,  3,  2,  5,  8,  6,  4,  10,
//11,  16,  18,  13,  -1,  -1,  9,  7,  -1,  -1,
//12,  17,  19,  14,  -1,  -1,  -1,  -1,  -1,  -1,
//13,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
//14,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
//15,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1};


//Shx Modified
// const int EquipPack::_poses[] = {   -1,  1,  0,  3,  2,  5,  8,  6,  4,  10,
// 11,  16,  18,  13,  14,  11,  9,  7,  18,  19,
// 12,  17,  19,  14,  -1,  -1,  -1,  -1,  -1,  -1,
// 13,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
// 14,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
// 15,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1};

const int EquipPack::_poses[] = {   -1,  1,  2,  3,  4,  5,  6,  7,  8,  9,
-1,  -1,  -1,  13,  14,  15,  16,  17,  18,  19,
20,  21,  -1,  -1,  -1,  -1,  10,  11,  -1,  -1,
-1,  -1,  -1,  12,  -1,  -1,  -1,  -1,  -1,  -1,
-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1};

/**
* \brief ���װ��
* \param object ��Ʒ����
* \param find �Ƿ��Զ�Ѱ��λ��
* \return ��ӳɹ�����true ���򷵻�false
*/
bool EquipPack::add(zObject *object,bool find)
{

	//fprintf(stderr,"װ����ƷEquipPack");
	//isEmpty();
	if (process_extra_add(object)) return Package::add(object,false);

	if (Package::add(object,false))
	{
		if (owner&&object &&(11 <= object->data.upgrade))
		{
			effectCount++;
			if (1==effectCount)  owner->showCurrentEffect(Cmd::USTATE_ULTRA_EQUIPMENT,true); // ���¿ͻ���״̬
		}
		calcAll();
		needRecalc=true;
		return true;
	}
	return false;
}

/**
* \brief ɾ��װ��
* \param object ��Ʒ����
*/
bool EquipPack::remove(zObject *object)
{
	if (process_extra_remove(object)) return Package::remove(object);

	bool ret = Package::remove(object);
	calcAll();
	if (owner&&object &&(11 <= object->data.upgrade))
	{
		if (effectCount>0)
		{
			effectCount--;
			if (0>=effectCount)  owner->showCurrentEffect(Cmd::USTATE_ULTRA_EQUIPMENT,false); // ���¿ͻ���״̬
		}
	}
	/*if (ret && object && object->base->kind == ItemType_Amulet && owner->scene->isIncScene())
	{	//sky �����Ѿ�û���շѵ�ͼ�ĸ�������`���԰���ȥ���� ^_^
		std::ostringstream os;
		os << "name=" << object->data.maker;
		os << " pos=" << object->data.durpoint << "," << object->data.dursecond;
		bzero(object->data.maker,sizeof(object->data.maker));
		object->data.durpoint=0;
		object->data.dursecond=0;
		Cmd::stAddObjectPropertyUserCmd ret;
		ret.byActionType = Cmd::EQUIPACTION_REFRESH;
		bcopy(&object->data,&ret.object,sizeof(t_Object));
		owner->sendCmdToMe(&ret,sizeof(ret));        
		Gm::gomap(owner,os.str().c_str());
	}*/
	needRecalc=true;

	return ret;
}

/************************************************************************/
/* \brief sky �����Ʒ��װ��ְҵ�Ƿ���ϵ�ǰ��ҵ�ְҵ
useJob ���ְҵ
EquipType ��Ʒ����*/
/************************************************************************/
bool EquipPack::IsJobEquip(DWORD useJob, DWORD EquipType)
{
	//sky ��ָ�����������κ�ְҵ������װ����
	if(EquipType == ItemType_Necklace || EquipType == ItemType_Fing || 
		EquipType == ItemType_Manteau || EquipType == ItemType_Bangle || EquipType == ItemType_Jade || ItemType_Earrings) //Shx Add���� , ����, ����û��ְҵ����;
		return true;

	if(useJob>JOB_NULL && useJob<=JOB_PASTOR)
	{
		switch(useJob)
		{
		case JOB_FIGHTER:		//սʿ
			{
				if( EquipType == ItemType_Blade ||
					EquipType == ItemType_Sword ||
					EquipType == ItemType_Axe ||
					EquipType == ItemType_Fan ||
					EquipType == ItemType_MetalBody ||
					EquipType == ItemType_Helm_Plate ||
					EquipType == ItemType_Caestus_Plate ||
					EquipType == ItemType_Cuff_Plate ||
					EquipType == ItemType_Shoes_Plate ||
					EquipType == ItemType_Shoulder_Plate ||
					EquipType == ItemType_Gloves_Plate ||
					EquipType == ItemType_Pants_Plate)
				{

					return true;
				}
			}
			break;
		case JOB_THIEVES:		//����
			{
				if( EquipType == ItemType_Hammer ||
					EquipType == ItemType_Staff ||
					EquipType == ItemType_FellBody ||
					EquipType == ItemType_Helm_Paper ||
					EquipType == ItemType_Caestus_Paper ||
					EquipType == ItemType_Cuff_Paper ||
					EquipType == ItemType_Shoes_Paper ||
					EquipType == ItemType_Shoulder_Paper ||
					EquipType == ItemType_Gloves_Paper	||
					EquipType == ItemType_Pants_Paper)
				{
					return true;
				}
			}
			break;
		case JOB_MASTER:		//��ʦ
		case JOB_PASTOR:		//��ʦ
			{
				if( EquipType == ItemType_Crossbow ||
					EquipType == ItemType_ClothBody ||
					EquipType == ItemType_Helm ||
					EquipType == ItemType_Caestus ||
					EquipType == ItemType_Cuff ||
					EquipType == ItemType_Shoes ||
					EquipType == tyItemType_Shoulder ||
					EquipType == tyItemType_Gloves ||
					EquipType == tyItemType_Pants)
				{
					return true;
				}
			}
			break;
		default:
			break;
		}
	}

	return false;
}

/**
* \brief �����Ʒ
* \param pUser ��ɫ
* \param object ��Ʒ����
* \param x ������
* \param y ������
* \return true װ���д���Ʒ
*/
bool EquipPack::checkAdd(SceneUser *pUser,zObject *ob,WORD x,WORD y)
{
 	if (ob && ob->base->setpos == y && (y == Cmd::EQUIPCELLTYPE_MAKE || y == Cmd::EQUIPCELLTYPE_PACKAGE)) {
		int pos = ((y- Cmd::EQUIPCELLTYPE_PACKAGE) << 1)  + x;
		if (pos >= 0 && pos < 4 && packs[pos] && !packs[pos]->empty()) return false;
 	}

	zObject* tmp;
	bool ret = getObjectByZone(&tmp,x,y);
	if (ob==NULL) return true;

	//sky װ��ǰ�ȼ����ְҵ�Ƿ���ϸ�װ����ְҵ�޶�
	if(!IsJobEquip(pUser->charbase.useJob, ob->base->kind))
	{
		Zebra::logger->debug("ID:%u �û�:%s ��ͼװ���Լ�ְҵ�޷�װ������Ʒ:%s" ,pUser->charbase.id ,pUser->charbase.name, ob->data.strName);
		return false;
	}

	using namespace Cmd;
	if (pUser->charbase.level < ob->data.needlevel)
		return false;

	if (ob->base->setpos && ob->base->setpos == y) 
	{
		switch(ob->base->kind)
		{
		case ItemType_DoubleExp:
			{
				if (ob->data.dur<= 60)
				{
					Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"ʣ��ʱ��̫��,����װ��"); 
					return false;
				}
				if (doubleexp_obj_time/86400 == SceneTimeTick::currentTime.sec()/86400)
				{
					if (doubleexp_obj != ob->data.qwThisID)
					{
						Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"һ��ֻ��ʹ��һ��˫��������Ʒ��");
						return false;
					}
				}
				else
				{
					doubleexp_obj = ob->data.qwThisID;
					doubleexp_obj_time = SceneTimeTick::currentTime.sec(); 
				}
			}
			break;
		case ItemType_FashionBody:
			{
				if (ob->data.dur<= 1)
				{
					Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"ʣ��ʱ��̫��,����װ��"); 
					return false;
				}
			}
			break;
		case ItemType_HighFashionBody:
			{
				if(ob->data.dur <= 1)
				{
					Channel::sendSys( pUser,Cmd::INFO_TYPE_FAIL,"ʣ��ʱ��̫��,����װ��");
					return false;
				}
			}
		case ItemType_Tonic:
		case ItemType_Amulet:
			{
				if (ob->data.dur<= 60)
				{
					Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"ʣ��ʱ��̫��,����װ��"); 
					return false;
				}
			}
			break;
		default:
			break;
		}
		
		// [ranqd] ������������Ҫ�����ֵ�����
		if( (ob->base->setpos == EQUIPCELLTYPE_HANDL && container[HANDR] && container[HANDR]->base->kind == ItemType_Blade) ||
			( ob->base->setpos == EQUIPCELLTYPE_HANDR && ob->base->kind == ItemType_Blade && container[HANDL] && container[HANDL]->base->kind == ItemType_Fan) )
			return ret;

		// [ranqd] װ����Ʒʱ�����Լ��
		if ((ob->base->setpos == EQUIPCELLTYPE_HANDL && 
			container[HANDR] && 
			container[HANDR]->base->kind != ob->base->needobject && 
			ob->base->kind != container[HANDR]->base->needobject) || // [ranqd] ���Ҫװ�������֣���������ƷҪ�������
			(ob->base->setpos == EQUIPCELLTYPE_HANDR && 
			container[HANDL]  && 
			container[HANDL]->base->kind  != ob->base->needobject && 
			ob->base->kind != container[HANDL]->base->needobject ) ) // [ranqd] ���Ҫװ�������֣���������ƷҪ�������
		{
			// [ranqd] �Բ���������Ĵ���
			zObject* obj = container[HANDR] ;
			if (ob->base->setpos == EQUIPCELLTYPE_HANDR) obj = container[HANDL] ;

			if (!obj) return ret;

			if (pUser->packs.uom.space(pUser) > 0 ) {
				pUser->packs.removeObject(obj,true,false); //notify but not delete

				/*        
				obj->data.pos.dwLocation = Cmd::OBJECTCELLTYPE_COMMON;
				pUser->packsaddObject(obj,true);
				*/
				pUser->packs.addObject(obj,true,AUTO_PACK);

				Cmd::stAddObjectPropertyUserCmd ret1;
				ret1.byActionType = Cmd::EQUIPACTION_OBTAIN;
				bcopy(&obj->data,&ret1.object,sizeof(t_Object),sizeof(ret1.object));
				pUser->sendCmdToMe(&ret1,sizeof(ret1));

				/*      
				if (ob->base->setpos == EQUIPCELLTYPE_HANDL) {
				container[HANDR] = NULL;
				}else {
				container[HANDL] = NULL;
				}
				*/
				return ret;
			}
			return false;
		}

		return ret;
	}

	return false;
}

/**
* \brief ����λ�ô�С��ȡ��Ʒ
* \param ret ���ص���Ʒ����
* \param x ������
* \param y ������
* \param width ���
* \param height �߶�
* \return true �ɹ�����
*/
bool EquipPack::getObjectByZone(zObject **ret,WORD x,WORD y)
{
	if (process_extra_get(ret,x,y)) return true;

	return Package::getObjectByZone(ret,x,y);
}

/**
* \brief �����
* \param user ��ɫ
* \param exp �󶨵�װ�����ϵľ�������
* \param force ���жϼ���ǿ�ư�
*/
void EquipPack::obtain_exp(SceneUser* user,DWORD exp,bool force)
{
	if (force || selectByPercent(20)) {
		int index = randBetween(0,15);
		int current = index;
		do {
			if (container[current] != NULL)
			{
				switch ( container[current]->base->kind)
				{
				case ItemType_Blade:        //104����������������
				case ItemType_Sword :          //105����������������
				case ItemType_Axe:             //106����������������
				case ItemType_Hammer:          //107����������������
				case ItemType_Staff:        //108��������������
				case ItemType_Crossbow:          //109���������������
				case ItemType_Fan:             //110������Ů����
				case ItemType_Stick:          //111�����ٻ���������      
				case ItemType_Necklace:  //117�����ɫ������
				case ItemType_Fing:    //118�����ɫ��ָ��
				case ItemType_ClothBody:    //101���������װ
				case ItemType_FellBody:        //102����Ƥ�����װ
				case ItemType_MetalBody:    //103��������������װ
				case ItemType_Shield:  //112���������
				case ItemType_Helm:    //113�����ɫͷ����
				case ItemType_Caestus:  //114�����ɫ������
				case ItemType_Cuff:    //115�����ɫ����
				case ItemType_Shoes:    //116�����ɫЬ�Ӳ�
					/*sky �������Ƥ���ͷ���֧��*/
				case ItemType_Helm_Paper: //ͷ��Ƥ
				case ItemType_Helm_Plate: //ͷ����
				case ItemType_Cuff_Paper: //����Ƥ
				case ItemType_Cuff_Plate: //�����
				case ItemType_Caestus_Paper: //����Ƥ
				case ItemType_Caestus_Plate: //������
				case ItemType_Shoes_Paper: //ѥ��Ƥ
				case ItemType_Shoes_Plate: //ѥ�Ӱ�
				//sky ������� ���� ������
				case tyItemType_Shoulder:
				case tyItemType_Gloves:
				case tyItemType_Pants:
				case ItemType_Shoulder_Paper:
				case ItemType_Gloves_Paper:
				case ItemType_Pants_Paper:
				case ItemType_Shoulder_Plate:
				case ItemType_Gloves_Plate:
				case ItemType_Pants_Plate:
					{
						container[current] ->data.exp += static_cast<int>(exp*0.05);
						Cmd::stObjectExpUserCmd cmd;
						cmd.id = container[current]->data.qwThisID;
						cmd.exp = container[current]->data.exp;
						user->sendCmdToMe(&cmd,sizeof(cmd));
					}
					break;
				default:
					break;
				}
				break;
			}

		}while ( ( current = (++current % 16) ) != index ) ;
	}    
}

/**
* \brief ��֧�������������;öȼ����,���;�Ϊ0ʱɾ����Ͱ
* \param pThis ����
* \param kind ��Ʒ����
* \param num ��������
* \return �����Ƿ�ɹ�
*/
bool EquipPack::skillCheckReduceObject(SceneUser* pThis,DWORD kind,WORD num)
{
	/*
	std::set<zObject *>::iterator it = allset.begin();
	int count=0;
	while (it != allset.end()) 
	*/
	int count=0;  

	for (int i=0; i<16; ++i) 
	{
		if (!container[i]) continue;

		switch(kind)
		{
		case BOW_ARROW_ITEM_TYPE:
			{
				if (container[i]->base->kind == kind) 
				{
					if ((short)container[i]->data.dur < num)
					{
						return false;
					}
					else
					{
						return true;
					}
				}
				break;
			}
		default:
			{
				if (container[i]->base->kind == kind) 
				{
					count++;
				}
			}
		}
		//    ++it;    
	}
	if (count>=num) return true;
	return false;
}

/**
* \brief ��Ʒ���Ĵ������ڼ��ܵ���Ʒ���ģ�
* \param pThis ����
* \param kind ��Ʒ������
* \param num ������Ʒ����Ŀ
* \return �����Ƿ�ɹ�
*/
bool EquipPack::skillReduceObject(SceneUser* pThis,DWORD kind,WORD num)
{
	//  std::set<zObject *>::iterator it = allset.begin();
	int count =0;
	//  while (it != allset.end()) 
	for (int i=0; i<16; ++i) 
	{
		if (!container[i]) continue;

		switch(kind)
		{
		case BOW_ARROW_ITEM_TYPE:
			{
				if (container[i]->base->kind == kind) 
				{
					if ((short)container[i]->data.dur <= num)
					{
						/*            
						zObject * obj = *it;
						Cmd::stRemoveObjectPropertyUserCmd rm;
						rm.qwThisID=obj->data.qwThisID;
						pThis->sendCmdToMe(&rm,sizeof(rm));
						zObject::logger((*it)->createid,(*it)->data.qwThisID,(*it)->data.strName,(*it)->data.dwNum,pThis->id,pThis->name,0,NULL,"��������");
						pThis->packs.rmObject(obj);
						SAFE_DELETE(obj);
						*/
						zObject::logger(container[i]->createid,container[i]->data.qwThisID,container[i]->data.strName,container[i]->data.dwNum,1,0,pThis->id,pThis->name,0,NULL,"��������",NULL,0,0);
						pThis->packs.removeObject(container[i]);
						return true;
					}
					else
					{
						container[i]->data.dur -=num;
						Cmd::stDurabilityUserCmd std;
						std.dwThisID = container[i]->data.qwThisID;
						std.dwDur = container[i]->data.dur;
						pThis->sendCmdToMe(&std,sizeof(std));
						//Zebra::logger->debug("���ļ�֧id=%u,thisid=%u,dur=%u",id,std.dwThisID,std.dwDur);
						return true;
					}
				}
				break;
			}
		default:
			{
				if (container[i]->base->kind == kind) 
				{
					/*
					std::set<zObject *>::iterator temp = it;
					it++;
					zObject * obj = *temp;
					Cmd::stRemoveObjectPropertyUserCmd rm;
					rm.qwThisID=obj->data.qwThisID;
					pThis->sendCmdToMe(&rm,sizeof(rm));
					zObject::logger((*it)->createid,(*it)->data.qwThisID,(*it)->data.strName,(*it)->data.dwNum,pThis->id,pThis->name,0,NULL,"��������");
					count++;
					pThis->packs.rmObject(obj);
					SAFE_DELETE(obj);
					*/            
					zObject::logger(container[i]->createid,container[i]->data.qwThisID,container[i]->data.strName,container[i]->data.dwNum,1,0,pThis->id,pThis->name,0,NULL,"��������",NULL,0,0);
					pThis->packs.removeObject(container[i]);
					if (count >= num) return true;
					continue;
				}
			}
		}
		//++it;    
	}
	if (count>=num) return true;

	return false;
}


/**
* \brief ���캯��
* \param ob ��Ʒ����
* \param type ����
*/
ObjectPack::ObjectPack(zObject* ob,int type,bool consume)  :
Package(type,ob->data.qwThisID,ob->data.maxpdamage,ob->data.maxmdamage),
_ob(ob),_cosume_by_time(consume),_one_min(60,SceneTimeTick::currentTime)
{
	if (_ob && _ob->data.maxdur ==0) //Ӧ��֮ǰû���;õ���Ʒ
	{
		_ob->data.maxdur = 43200;
		_ob->data.dur = 43200;
	}
	if (ob->base->setpos == Cmd::EQUIPCELLTYPE_PACKAGE  && !_ob->data.bind) {
		_ob->data.bind = 1;
		_ob->data.color = time(NULL);
	}
}

/**
* \brief ��������
*/
ObjectPack::~ObjectPack()
{
	//SAFE_DELETE(_ob);
}

/*
bool ObjectPack::add(zObject *object,bool find)
{
if (find && (!_ob || _ob->data.dur == 0)) {
//only can takeout
return false;
}
return Package::add(object,find);
}
// */
bool ObjectPack::checkAdd(SceneUser *pUser,zObject *object,WORD x,WORD y)
{
	//fprintf(stderr,"װ����ƷObjectPack");
	//isEmpty(); 
	if (!_ob || _ob->data.dur == 0) {
		//only can takeout
		return object == NULL;
	}
	if (object && _ob->data.qwThisID == object->data.qwThisID)
	{
		Zebra::logger->debug("���Լ���ӵ��Լ��İ���%s(%d),%s(%u)",pUser->name,pUser->id,object->data.strName,object->data.qwThisID);
		return false;
	}

	return Package::checkAdd(pUser,object,x,y);
}

void ObjectPack::consume_dur_by(SceneUser* user,const zRTime& current)
{
	/*
	if (_cosume_by_time && _ob && _ob->data.dur > 0 && _one_min(current) ) {

	--_ob->data.dur;
	Cmd::stDurabilityUserCmd cmd;
	cmd.dwThisID = _ob->data.qwThisID;
	cmd.dwDur = _ob->data.dur;
	user->sendCmdToMe(&cmd,sizeof(cmd));
	}
	*/
}

/**
* \brief ���캯����ʼ������������
*/
MainPack::MainPack():Package(Cmd::OBJECTCELLTYPE_COMMON,0,MainPack::WIDTH,MainPack::HEIGHT)
{
	TabNum = MIN_TAB_NUM; //sky ��ʼ������ҳ��
	gold=NULL;
}

/**
* \brief ��������
*/
MainPack::~MainPack()
{
	SAFE_DELETE(gold);
}

/**
* \brief ���������Ӷ���
* \param object ��Ʒ����
* \param find Ѱ�ұ�־
* \return true ��ӳɹ�,false ���ʧ��
*/
bool MainPack::add(zObject *object,bool find)
{
	//fprintf(stderr,"װ����ƷMainPack");
	// isEmpty();
	if (object && object->base->kind==ItemType_Money)
	{
		if (gold)
		{
			return false;
		}
		else
		{
			gold=object;
			/*
			gold->data.pos.dwLocation=Cmd::OBJECTCELLTYPE_COMMON;
			gold->data.pos.dwTableID=0;
			gold->data.pos.x=(WORD)-1;
			gold->data.pos.y=(WORD)-1;
			*/
			gold->data.pos = stObjectLocation(Cmd::OBJECTCELLTYPE_COMMON,0,(WORD)-1,(WORD)-1);
			return true;
		}

	}
	else
		return Package::add(object,find);
}

/**
* \brief �Ӱ�����ɾ����Ʒ
* \param object  ��Ʒ����
*/
bool MainPack::remove(zObject *object)
{
	if (object && object->base->kind==ItemType_Money)
	{
		if (gold==object) gold=NULL;
		return true;
	}
	else
		return Package::remove(object);
}

/**
* \brief ��鲢���
* \param pUser ��ɫ
* \param object ��Ʒ
* \param x,y ����
* \return true �и���Ʒ,false û��
*/
bool MainPack::checkAdd(SceneUser *pUser,zObject *object,WORD x,WORD y)
{
	if (object && object->base->kind==ItemType_Money)
		return (gold==NULL);
	else
		return  Package::checkAdd(pUser,object,x,y);
}

/**
* \brief ������Ʒλ�úʹ�С��ȡ��Ʒ
* \param ret �����ҵ�����Ʒ����
* \param x ������
* \param y ������
* \param width ���
* \param height �߶�
* \return true �д���Ʒ false �޴���Ʒ
*/
bool MainPack::getObjectByZone(zObject **ret,WORD x,WORD y)
{
	if (x==(WORD)-1 && y==(WORD)-1)
	{
		*ret=gold;
		return true;
	}
	else
		return  Package::getObjectByZone(ret,x,y);
}

/**
* \brief ��ý�����Ŀ
* \return ������Ŀ
*/
DWORD MainPack::getGoldNum()
{
	if (gold)
	{
		return gold->data.dwNum;
	}
	else
	{
		return 0;
	}
}

/**
* \brief ��ý��Ӷ���
* \return  ��Ʒ�����NULL
*/
zObject * MainPack::getGold()
{
	return gold;
}

/**
* \brief ��֧�������������;öȼ����,���;�Ϊ0ʱɾ����Ͱ
* \param pThis ����
* \param id ��Ʒ��objectid
* \param num������������Ʒ������
* \return �����Ƿ�ɹ�
*/
bool MainPack::skillReduceObject(SceneUser* pThis,DWORD id,DWORD num)
{
	/*
	std::set<zObject *>::iterator it = allset.begin();
	while (it != allset.end()) 
	*/
	for (int i=0; i<16; ++i)
	{
		if (!container[i]) continue;

		if (container[i]->data.dwObjectID == id) 
		{
			if ((short)container[i]->data.dur <= 1)
			{
				/*
				pThis->packs.rmObject(*it);
				Cmd::stRemoveObjectPropertyUserCmd rm;
				rm.qwThisID=(*it)->data.qwThisID;
				pThis->sendCmdToMe(&rm,sizeof(rm));
				*/
				pThis->packs.removeObject(container[i],true,false/*?*/);
				return true;
			}
			else
			{
				container[i]->data.dur -=1;
				Cmd::stDurabilityUserCmd std;
				std.dwThisID = container[i]->data.qwThisID;
				std.dwDur = container[i]->data.dur;
				std.dwMaxDur = container[i]->data.maxdur;
				pThis->sendCmdToMe(&std,sizeof(std));
				//Zebra::logger->debug("���ļ�֧id=%u,thisid=%u,dur=%u",id,std.dwThisID,std.dwDur);
				return true;
			}
			break;
		}
		///++it;    
	}

	return false;
}


/**
* \brief �ֿ⹹�캯��,��ʼ���ֿ����Ͳ�����ֿ��С
*/
StorePack::StorePack() : Package(Cmd::OBJECTCELLTYPE_STORE,0,W,SAVEBOX_HEIGHT)
{
	days = MIN_TAB_NUM;
}

/**
* \brief �ֿ⹹�캯�����գ�
*/
StorePack::~StorePack()
{

}

/**
* \brief ���ָ��λ�����Ƿ���ָ���Ķ���
* \param pUser �ֿ�ӵ����
* \param object ��Ʒ����
* \param x ������
* \param y ������
* \return true ��ָ��λ������ָ������ false ���ʧ��
*/
bool StorePack::checkAdd(SceneUser * pUser,zObject * object,WORD x,WORD y)
{
	if (!object) return true;

	//check if npc can do this?
	NpcTrade::NpcItem item;
	item.id = object->data.dwObjectID;
	item.kind = object->base->kind;
	item.lowLevel = 0;
	item.level = object->data.needlevel;
	item.action = NpcTrade::NPC_STORE_OBJECT;
	//if (!NpcTrade::getInstance().verifyNpcAction(pUser->npc_dwNpcDataID,item) ) {
	//  return false;
	//}

	////check if npc near by user?
	//SceneNpc * npc = SceneNpcManager::getMe().getNpcByTempID(pUser->npc_dwNpcTempID);
	//if (!npc  || !npc->scene || npc->scene != pUser->scene 
	//    || !pUser->scene->zPosShortRange(pUser->getPos(),npc->getPos(),SCREEN_WIDTH,SCREEN_HEIGHT)
	//     )   
	//{
	//  return false;
	//}

	BYTE page = y / (SAVEBOX_HEIGHT/5);
	if (page < days) 
	{
		return Package::checkAdd(pUser,object,x,y);
	}
	return false;
}

void StorePack::goldstore(SceneUser *pThis,bool notify)
{
	//����Ѿ�����������������vip�û������͵�����
	/*if (pThis->packs.store.days.size()<=2 && (pThis->charbase.bitmask & CHARBASE_VIP))
	{
	pThis->packs.store.days.push_back(1);
	if (notify)
	{
	pThis->packs.store.notify(pThis);
	Channel::sendSys(pThis,Cmd::INFO_TYPE_GAME,"��ϲ,�����ϵͳ���͵�һ���ֿ�"); 
	}
	}*/
}

/**sky �޸Ĳֿ�ļ��ط�ʽ
* \brief �ֿ����
* \param dest Ŀ������
* \return ����С
*/
int StorePack::load(BYTE* dest)
{
	//for(int i = 19;i >= 0;i--)
	//	printf("%2.2X ", *(dest - i));
	//printf("\n");
	days = *dest;

	return sizeof(BYTE);
}

/** sky �޸Ĳֿ�Ĵ洢��ʽ
* \brief �ֿ�洢
* \param dest Ŀ������
* \return ����С
*/
int StorePack::save(BYTE* dest)
{
	//for(int i = 19;i >= 0;i--)
	//	printf("%2.2X ", *(dest - i));
	//printf("\n");
	*dest = days;
	//memcpy(dest, &days, sizeof(BYTE));

	return sizeof(BYTE);
}

/**
* \brief ���Ͳֿ����
* \param user ��ɫ
*/
void StorePack::notify(SceneUser* user)
{
	/*BYTE buf[zSocket::MAX_DATASIZE];
	Cmd::stStoreInfoNpcTradeUserCmd* info = (Cmd::stStoreInfoNpcTradeUserCmd*)buf;

	info->page = days;

	user->sendCmdToMe(info,sizeof(Cmd::stStoreInfoNpcTradeUserCmd));*/

	Cmd::stPackBuyTanbNumUserCmd info;
	info.TabNum = user->packs.store.days;
	info.PackType = SAVEBOX_TYPE;
	user->sendCmdToMe(&info,sizeof(Cmd::stPackBuyTanbNumUserCmd));
}

/**
* \brief �ܷ�ɫ
* \return true �� false ��
*/
bool EquipPack::canChangeColor()
{
	/// ����Ҫ��,Ŀ����Ϊ�˷���ͨ����ɫ�ֶ���װ����Ч
	/*
	if (equip(BODY) && (equip(BODY)->base->color == 0XFFFFFFFF))
	{
	return true;
	}
	else
	{
	return false;
	}
	// */
	return true;
}

/**
* \brief ���캯��
*/
Packages::Packages(SceneUser* user) : owner(user),equip(user)
{

}

/**
* \brief  ��������
*/
Packages::~Packages()
{

}

/**
* \brief �������ͻ�ȡ����
* \param type ��������
* \param id Ŀǰδʹ��
* \return ��������
*/
Package * Packages::getPackage(DWORD type,DWORD id)
{
	switch(type)
	{
	case Cmd::OBJECTCELLTYPE_COMMON:
		return (Package *)&main;
	case Cmd::OBJECTCELLTYPE_EQUIP:
		return (Package *)&equip;
	case Cmd::OBJECTCELLTYPE_STORE:
		return (Package *)&store;
	case Cmd::OBJECTCELLTYPE_PACKAGE:
		if (equip.pack(EquipPack::L_PACK) && equip.pack(EquipPack::L_PACK)->object()->data.qwThisID == id) {
			return (Package *)equip.pack(EquipPack::L_PACK);
		}
		if (equip.pack(EquipPack::R_PACK) && equip.pack(EquipPack::R_PACK)->object()->data.qwThisID == id) {
			return (Package *)equip.pack(EquipPack::R_PACK);
		}
	case Cmd::OBJECTCELLTYPE_MAKE:
		if (equip.pack(EquipPack::L_MAKE) && equip.pack(EquipPack::L_MAKE)->object()->data.qwThisID == id) {
			return (Package *)equip.pack(EquipPack::L_MAKE);
		}
		if (equip.pack(EquipPack::R_MAKE) && equip.pack(EquipPack::R_MAKE)->object()->data.qwThisID == id) {
			return (Package *)equip.pack(EquipPack::R_MAKE);
		}
		break;
	case Cmd::OBJECTCELLTYPE_PET:
		return (Package *)&petPack;
		break;
	default:
		break;
	}
	return NULL;
}

/*
void Packages::clearPackage(Package* pack)
{
ClearPack cp(this);
pack->execEvery(cp);
}
*/

Package** Packages::getPackage(int packs)
{
	Package** p = new Package*[8];
	memset(p,0,8*sizeof(Package *));

	//notice the sequence
	int i = 0;
	if (packs & MAIN_PACK) p[i++] = (Package *)&main;
	if ((packs & LEFT_PACK) && equip.pack(EquipPack::L_PACK) ) p[i++] =  (Package *)equip.pack(EquipPack::L_PACK);  
	if ((packs & RIGHT_PACK) && equip.pack(EquipPack::R_PACK) ) p[i++] = (Package *)equip.pack(EquipPack::R_PACK);  

	if (packs & EQUIP_PACK) p[i++] = (Package *)&equip;
	if (packs & STORE_PACK) p[i++] = (Package *)&store;

	if ((packs & LM_PACK) && equip.pack(EquipPack::L_MAKE) ) p[i++] = (Package *)equip.pack(EquipPack::L_MAKE);  
	if ((packs & RM_PACK) && equip.pack(EquipPack::R_MAKE) ) p[i++] = (Package *)equip.pack(EquipPack::R_MAKE);

	if (packs & PET_PACK) p[i++] = (Package *)&petPack;

	return p;
}

/**
* \brief ��õ�ǰ��������
* \return ��������
*/
DWORD Packages::getGoldNum()
{
	return main.getGoldNum();
}

/**
* \brief ��ȡ���ϵĽ���
* \return ��Ʒ����,��NULL
*/
zObject *Packages::getGold()
{
	return main.getGold();
}

/**
* \brief ����Ʒ��������
* \param o Ŀ����Ʒ 
* \param pos λ��
* \return true ���ĵķ���ֵ
*/
bool Packages::moveObjectToScene(zObject *o,const zPos &pos,DWORD overdue_msecs,const unsigned long dwID)
{
	removeObject(o,true,false); //notify but not delete
	zObject::logger(o->createid,o->data.qwThisID,o->data.strName,o->data.dwNum,o->data.dwNum,0,owner->id,owner->name,owner->scene->id,owner->scene->name,"����",o->base,o->data.kind,o->data.upgrade);
	//������
	if (!owner->scene->addObject(owner->dupIndex,o,pos,overdue_msecs,dwID))
	{
		Zebra::logger->info("%s(%ld)�����װ����ӵ�����ʧ��",
			owner->name,owner->id);
		//zObject::destroy(o);
	}
	return true;
}

/**
* \brief �ƶ���Ʒ
* \param pUser ��ɫ����
* \param srcObj ���ƶ�����
* \param dst �����Ŀ��λ��
* \return true �ƶ��ɹ�,false �ƶ�ʧ��
*/
bool Packages::moveObject(SceneUser *pUser,zObject *srcObj,stObjectLocation &dst)
{
	using namespace Cmd;

	Package *srcpack = getPackage(srcObj->data.pos.loc(),srcObj->data.pos.tab());
	if (!srcpack)  {
		Zebra::logger->warn("��Ʒ%s[%x]��λ����,�����ƶ�",srcObj->name,srcObj);    
		return false;
	}
	if (srcpack->type() == Cmd::OBJECTCELLTYPE_EQUIP)
	{
		if (
			(equip.pack(EquipPack::L_PACK) && 
			equip.pack(EquipPack::L_PACK)->object()->data.qwThisID == srcObj->data.qwThisID && 
			!equip.pack(EquipPack::L_PACK)->empty()) ||
			(equip.pack(EquipPack::R_PACK) && 
			equip.pack(EquipPack::R_PACK)->object()->data.qwThisID == srcObj->data.qwThisID &&
			!equip.pack(EquipPack::R_PACK)->empty()) ||
			(equip.pack(EquipPack::L_MAKE) && 
			equip.pack(EquipPack::L_MAKE)->object()->data.qwThisID == srcObj->data.qwThisID &&
			!equip.pack(EquipPack::L_MAKE)->empty()) ||
			(equip.pack(EquipPack::R_MAKE) && 
			equip.pack(EquipPack::R_MAKE)->object()->data.qwThisID == srcObj->data.qwThisID &&
			!equip.pack(EquipPack::R_MAKE)->empty())
			)
		{
			Zebra::logger->warn("�����ƶ�װ����Ʒ�İ���%s(%u),%s(%u)",pUser->name,pUser->id,srcObj->data.strName,srcObj->data.qwThisID);
			return false;
		}
	}

	if (dst.loc() == Cmd::OBJECTCELLTYPE_NONE) {
		zObject::logger(srcObj->createid,srcObj->data.qwThisID,srcObj->base->name,srcObj->data.dwNum,srcObj->data.dwNum,0,0,NULL,pUser->id,pUser->name,"�Ӷ���",srcObj->base,srcObj->data.kind,srcObj->data.upgrade);
		removeObject(srcObj);
		return true;
	}

	Package *destpack = getPackage(dst.loc(),dst.tab());
	if (!destpack) return false;

	if (srcpack->type() == Cmd::OBJECTCELLTYPE_STORE && destpack->type() != Cmd::OBJECTCELLTYPE_STORE)
	{
		if (srcObj->data.pos.ypos()>=6 && pUser->isSafety(Cmd::SAFE_THIRD_PACK))
		{// �ֿ���,�������Ժ�,���ǵڶ�.���ֿ�
			return false;
		}
	}

	if (destpack->type() == Cmd::OBJECTCELLTYPE_EQUIP)
	{
		if (
			(equip.pack(EquipPack::L_PACK) && 
			equip.pack(EquipPack::L_PACK)->object()->data.qwThisID == srcObj->data.qwThisID && 
			!equip.pack(EquipPack::L_PACK)->empty()) ||
			(equip.pack(EquipPack::R_PACK) && 
			equip.pack(EquipPack::R_PACK)->object()->data.qwThisID == srcObj->data.qwThisID &&
			!equip.pack(EquipPack::R_PACK)->empty()) ||
			(equip.pack(EquipPack::L_MAKE) && 
			equip.pack(EquipPack::L_MAKE)->object()->data.qwThisID == srcObj->data.qwThisID &&
			!equip.pack(EquipPack::L_MAKE)->empty()) ||
			(equip.pack(EquipPack::R_MAKE) && 
			equip.pack(EquipPack::R_MAKE)->object()->data.qwThisID == srcObj->data.qwThisID &&
			!equip.pack(EquipPack::R_MAKE)->empty())
			)
		{
			Zebra::logger->warn("�����ƶ�װ����Ʒ�İ���%s(%u),%s(%u)",pUser->name,pUser->id,srcObj->data.strName,srcObj->data.qwThisID);
			return false;
		}
	}

	zObject *destObj = NULL;
	if (destpack->getObjectByZone(&destObj,dst.xpos(),dst.ypos()) && destObj != srcObj)
	{
 		if (destpack->checkAdd(pUser,srcObj,dst.xpos(),dst.ypos()))
		{
			if (srcpack->checkAdd(pUser,destObj,srcObj->data.pos.xpos(),srcObj->data.pos.ypos()))
			{
				//�������ֻ���ó����ܷ���
				if (dst.loc()==Cmd::OBJECTCELLTYPE_PET
					|| (srcObj->data.pos.loc()==Cmd::OBJECTCELLTYPE_PET && destObj))
					return false;

				//sky �ж����û�Ҫ���µİ���λ���Ƿ񱻼�����
				if(destpack->type() == OBJECTCELLTYPE_COMMON && (dst.ypos() >= ((MainPack*)destpack)->TabNum * (PACK_HEIGHT/MAX_TAB_NUM)))
				{
					return false;
				}

				destpack->remove(destObj);
				if (destObj) destObj->data.pos=srcObj->data.pos;

				srcpack->remove(srcObj);
				srcpack->add(destObj,false);

				srcObj->data.pos=dst;
				destpack->add(srcObj,false);

				if (dst.loc() ==Cmd::OBJECTCELLTYPE_EQUIP)//װ��ʱ�����;�
				{
					switch(srcObj->base->kind)
					{
					case ItemType_DoubleExp:
						{
							if (srcObj->data.dur> 60)
							{
								if (!pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN,ItemType_DoubleExp,60,true,true))
								{
									pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN + 1,ItemType_DoubleExp,60,true,true);
								}
							}
							/*
							else
							{
							Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"�;ö�̫��,����װ��"); 
							return false;
							}
							// */
						}
						break;
					case ItemType_Tonic:
						{
							if (srcObj->data.dur> 60)
							{
								if (!pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN,ItemType_Tonic,60,true,false))
								{
									pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN + 1,ItemType_Tonic,60,true,false);
								}
							}
							/*
							else
							{
							Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"�;ö�̫��,����װ��"); 
							return false;
							}
							// */
						}
						break;
					case ItemType_Amulet:
						{
							if (srcObj->data.dur> 60)
							{
								if (!pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN,ItemType_Amulet,60,true,false))
								{
									pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN + 1,ItemType_Amulet,60,true,false);
								}
							}
							/*
							else
							{
							Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"�;ö�̫��,����װ��"); 
							return false;
							}
							// */
						}
						break;
					case ItemType_FashionBody:
						{
							if (!pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN,ItemType_FashionBody,1,true,false))
							{
								pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN + 1,ItemType_FashionBody,1,true,false);
							}
						}
						break;
					case ItemType_HighFashionBody:
						{
							if (!pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN,ItemType_HighFashionBody,1,true,false))
							{
								pUser->packs.equip.reduceDur(pUser,Cmd::EQUIPCELLTYPE_ADORN + 1,ItemType_HighFashionBody,1,true,false);
							}
						}
						break;
					case ItemType_GreatLeechdomMp:
						{
							//����Ƿ����Զ���ħ����
							pUser->checkAutoMP();
						}
						break;
					default:
						break;
					}
				}

				return true;
			}
		}
	}
	else
	{
		if (destObj == srcObj )
		{
			Zebra::logger->debug("�Լ��ƶ����Լ���λ��bug");
		}
	}

	return false;
}

/**
* \brief  ɾ�������еĶ���
* \param   srcObj Ŀ����Ʒ
*/
bool Packages::removeObject(zObject *srcObj,bool notify,bool del)
{
	if (!srcObj) return false;

	if (notify) {
		Cmd::stRemoveObjectPropertyUserCmd send;
		send.qwThisID=srcObj->data.qwThisID;
		owner->sendCmdToMe(&send,sizeof(send));
	}

	uom.removeObject(srcObj);
	Package *p=getPackage(srcObj->data.pos.loc(),srcObj->data.pos.tab());
	if (p && p->remove(srcObj)) {
		//only delete when remove from package and del is true
		if (del) zObject::destroy(srcObj);
		return true;
	}

	Zebra::logger->warn("��Ʒ%s[%x]��λ����,����ɾ��",srcObj->name,srcObj);    
	return false;
}

/**
* \brief ������Ʒ
* \param srcObj ��Ʒ����
* \param needFind �Ƿ�Ҫ����λ��
* \param from_record �Ƿ����Լ�¼
* \return false ���ʧ�� true ��ӳɹ�
*/
bool Packages::addObject(zObject *srcObj,bool needFind,int packs)
{
	if (srcObj)
	{
		switch (srcObj->base->kind)
		{
		case ItemType_Blade:                //104����������������
		case ItemType_Sword :           //105����������������
		case ItemType_Axe:                 //106����������������
		case ItemType_Hammer:           //107����������������
		case ItemType_Staff:                //108��������������
		case ItemType_Crossbow:         //109���������������
		case ItemType_Fan:                 //110������Ů����
		case ItemType_Stick:            //111�����ٻ���������                   
		case ItemType_Necklace: //117�����ɫ������
		case ItemType_Fing:             //118�����ɫ��ָ��
		case ItemType_ClothBody:                //101�����ʼ��������װ
		case ItemType_FellBody:             //102����Ƥ�׼�ħ�����װ
		case ItemType_MetalBody:                //103����������׼�������װ
		case ItemType_Shield:   //112���������
		case ItemType_Helm:    //113�����ɫͷ����
		case ItemType_Caestus:  //114�����ɫ������
		case ItemType_Cuff:    //115�����ɫ����
		case ItemType_Shoes:    //116�����ɫЬ�Ӳ�
			/*sky �������Ƥ���ͷ���֧��*/
		case ItemType_Helm_Paper: //ͷ��Ƥ
		case ItemType_Helm_Plate: //ͷ����
		case ItemType_Cuff_Paper: //����Ƥ
		case ItemType_Cuff_Plate: //�����
		case ItemType_Caestus_Paper: //����Ƥ
		case ItemType_Caestus_Plate: //������
		case ItemType_Shoes_Paper: //ѥ��Ƥ
		case ItemType_Shoes_Plate: //ѥ�Ӱ�
		//sky ������� ���� ������
		case tyItemType_Shoulder:
		case tyItemType_Gloves:
		case tyItemType_Pants:
		case ItemType_Shoulder_Paper:
		case ItemType_Gloves_Paper:
		case ItemType_Pants_Paper:
		case ItemType_Shoulder_Plate:
		case ItemType_Gloves_Plate:
		case ItemType_Pants_Plate:
			break;
		default:
			srcObj->data.exp = 0;
			break;
		}

		assert( !needFind || srcObj->free() ); //be sure a object do not be added into 2 diff package
		if (uom.addObject(srcObj))  {
			if (!packs) {
				Package *p = getPackage(srcObj->data.pos.loc(),srcObj->data.pos.tab());
				if (p && p->add(srcObj,needFind)) return true;
			}

			if (packs) {
				//save location infomation
				stObjectLocation loc = srcObj->reserve();

				Package** p = getPackage(packs);
				int i = 0;
				while (p && p[i]) {
					srcObj->data.pos = stObjectLocation(p[i]->type(),p[i]->id(),(WORD)-1,(WORD)-1);
					if (p[i]->add(srcObj,needFind)) {
						SAFE_DELETE_VEC(p);
						return true;
					}
					++i;
				}
				SAFE_DELETE_VEC(p);

				//can not be add,resotre the location
				srcObj->restore(loc);
			}
			//Zebra::logger->warn("��Ʒ%s[%x]��λ����,�������",srcObj->name,srcObj);
			uom.removeObject(srcObj);

			return false;
		}else {
			//Zebra::logger->warn("��Ʒ%s[%x]�����ظ�,�������",srcObj->name,srcObj);
		}
	}
	return false;
}

/**     
* \brief ����������֤
*      
* ��֤�û������еĻ����Ƿ�����Ҫ��

* \param need: ��Ҫ����
* \return ��֤ͨ������true,���򷵻�false
*/
bool Packages::checkTicket(DWORD need)
{
	if (owner->charbase.ticket >= need)
	{
		return true;
	}

	return false;  
}
/**     
* \brief ���������֤
*      
* ��֤�û������еĽ���Ƿ�����Ҫ��

* \param need: ��Ҫ���
* \return ��֤ͨ������true,���򷵻�false
*/
bool Packages::checkGold(DWORD need)
{
	if (owner->charbase.gold >= need)
	{
		return true;
	}

	return false;  
}
/**     
* \brief ����������֤
*      
* ��֤�û������е������Ƿ�����Ҫ��

* \param need: ��Ҫ����
* \return ��֤ͨ������true,���򷵻�false
*/
bool Packages::checkMoney(DWORD need)
{
	if (!need) return true;

	zObject* gold = getGold();
	//temp solution,just for record before
	if (!gold) {
		gold = zObject::create(objectbm.get(665),0);
		if (gold) {
			owner->packs.addObject(gold,true,MAIN_PACK);
		}else {
			Zebra::logger->error("��������ʧ��");
		}

	}

	if (!gold) return false;
	if (gold->data.dwNum < need ) {
		//Channel::sendSys(&user,Cmd::INFO_TYPE_FAIL,"���Ӳ���"); 
		return false;
	}

	return true;  
}

/**     
* \brief �۳�����
*      
* ���û������۳�����,�ú���������۳��Ľ��,ע���ֹ���

* \param num: �۳�����������
* \return �ɹ�����true,���򷵻�false
*/
bool Packages::removeMoney(DWORD num,const char *disc)
{  
#ifdef _DEBUG
	Zebra::logger->warn("�û�(%s)���ӿ۳���������,����(%d)",owner->name,num);
#endif
	if (!num) return true;

	zObject* gold = getGold();
	//temp solution,just for record before
	if (!gold) {
		gold = zObject::create(objectbm.get(665),0);
		if (gold) {
			owner->packs.addObject(gold,true,MAIN_PACK);
		}else {
			Zebra::logger->error("��������ʧ��");
		}
	}

	if (!gold) return false;

#ifdef _DEBUG
	Zebra::logger->warn("�û�(%s)���ӿ۳�����,����(%d),����(%d)",owner->name,num,gold->data.dwNum);
#endif

	if (gold->data.dwNum < num) {
		Zebra::logger->warn("�û�(%s)���ӿ۳�ʧ��,����(%d),����(%d)",owner->name,num,gold->data.dwNum);
		return false;
	}

	gold->data.dwNum -= num;
	zObject::logger(owner->charbase.accid,gold->data.qwThisID,gold->data.strName,gold->data.dwNum,num,0,0,NULL,owner->id,owner->name,disc,NULL,0,0);

	Cmd::stRefCountObjectPropertyUserCmd ret;
	ret.qwThisID = gold->data.qwThisID;
	ret.dwNum = gold->data.dwNum;
	owner->sendCmdToMe(&ret,sizeof(ret));

	return true;
}

/**     
* \brief �۳�����
*      

* \param num: �۳��Ļ�������
* \return �ɹ�����true,���򷵻�false
*/
bool Packages::removeTicket(DWORD num,const char *disc)
{
	if (owner->charbase.ticket < num)
	{
		Zebra::logger->debug("%s(%d)���ֲ���,�۳�ʧ��,��Ҫ%d,����%d,����:%s",owner->name,owner->id,num,owner->charbase.ticket,disc);
		return false;
	}
	owner->charbase.ticket-=num;
	zObject::logger(0,0,"����",owner->charbase.ticket,num,0,owner->id,owner->name,0,NULL,disc,NULL,0,0);
	Cmd::stMainUserDataUserCmd send;
	owner->full_t_MainUserData(send.data);
	owner->sendCmdToMe(&send,sizeof(send));

	return true;
}
/**
* \brief ��ӻ���
* \param num ��������
* \param note ��ʾ��Ϣ
*/
void Packages::addTicket(DWORD num,const char *disc,const char *note,bool notify)
{       
	using namespace Cmd;
	owner->charbase.ticket+=num;
	zObject::logger(0,0,"����",owner->charbase.ticket,num,1,0,NULL,owner->id,owner->name,disc,NULL,0,0);
	Cmd::stMainUserDataUserCmd send;
	owner->full_t_MainUserData(send.data);
	owner->sendCmdToMe(&send,sizeof(send));
	if (notify)
	{
		if (note == NULL)
		{
			Channel::sendSys(owner,Cmd::INFO_TYPE_GAME,"��õ�����%d",num);
		}       
		else                    
		{       
			Channel::sendSys(owner,Cmd::INFO_TYPE_GAME,"%s����ʹ��õ�����%d",note,num);
		}
	}
}
/**     
* \brief �۳����
*      

* \param num: �۳��Ľ�������
* \param need: �Ƿ���Ҫ�ӳ�(��Ʊ��Ǯ�������κμӳ�)
* \return �ɹ�����true,���򷵻�false
*/
bool Packages::removeGold(DWORD num,const char *disc,bool need)
{
	if (owner->charbase.gold < num)
	{
		Zebra::logger->debug("%s(%d)��Ҳ���,�۳�ʧ��,��Ҫ%d,����%d,����:%s",owner->name,owner->id,num,owner->charbase.gold,disc);
		return false;
	}
	owner->charbase.gold-=num;
	zObject::logger(owner->charbase.accid,owner->charbase.level,"���",owner->charbase.gold,num,0,owner->id,owner->name,0,NULL,disc,NULL,0,0);
	Cmd::stMainUserDataUserCmd send;
	owner->full_t_MainUserData(send.data);
	owner->sendCmdToMe(&send,sizeof(send));
	//����ǹ�Ʊ��Ǯ��洢,������
	if (!need)
		owner->save(Cmd::Record::OPERATION_WRITEBACK);

	return true;
}
/**
* \brief ��ӽ��
* \param num �������
* \param note ��ʾ��Ϣ
*/
void Packages::addGold(DWORD num,const char *disc,const char *note,bool notify,bool pack)
{       
	using namespace Cmd;
	owner->charbase.gold+=num;
	zObject::logger(owner->charbase.accid,owner->charbase.level,"���",owner->charbase.gold,num,1,0,NULL,owner->id,owner->name,disc,NULL,0,0);
	Cmd::stMainUserDataUserCmd send;
	owner->full_t_MainUserData(send.data);
	owner->sendCmdToMe(&send,sizeof(send));
	owner->save(Cmd::Record::OPERATION_WRITEBACK);
	if (notify)
	{
		if (note == NULL)
		{
			//Channel::sendSys(owner,Cmd::INFO_TYPE_GAME,"��õ�����%d",num);
			Channel::sendGold(owner,Cmd::INFO_TYPE_GAME,num,"��õ�����");
		}       
		else                    
		{       
			Channel::sendGold(owner,Cmd::INFO_TYPE_GAME,num,disc);
		}
	}
	if (pack && !(owner->charbase.bitmask & CHARBASE_VIP))
	{
		owner->charbase.bitmask |= CHARBASE_VIP;
		//vip�û�����һ������
		owner->packs.store.goldstore(owner);
	}
}
/**
* \brief �������
* \param num ��������
* \param note ��ʾ��Ϣ
*/
void Packages::addMoney(DWORD num,const char *disc,const char *note,bool notify)
{       
	using namespace Cmd;
	DWORD realGet = 0;
	zObject *gold = getGold();
	//temp solution,just for record before
	if (!gold) {
		gold = zObject::create(objectbm.get(665),0);
		if (gold) {
			owner->packs.addObject(gold,true,MAIN_PACK);
		}else {
			Zebra::logger->error("��������ʧ��");
		}
	}

	if (!gold) return;
	if (gold->base->maxnum - gold->data.dwNum < num)
	{
		realGet = gold->base->maxnum - gold->data.dwNum;
		gold->data.dwNum = gold->base->maxnum;
	}
	else
	{
		gold->data.dwNum += num;
		realGet = num;
	}
	zObject::logger(owner->charbase.accid,gold->data.qwThisID,gold->data.strName,gold->data.dwNum,realGet,1,0,NULL,owner->id,owner->name,disc,NULL,0,0);
	stRefCountObjectPropertyUserCmd setgold;
	setgold.qwThisID=gold->data.qwThisID;
	setgold.dwNum=gold->data.dwNum;
	owner->sendCmdToMe(&setgold,sizeof(setgold));

	if (notify)
	{
		if (note == NULL)
		{
			Channel::sendMoney(owner,Cmd::INFO_TYPE_GAME,realGet,"��õ�����");
		}       
		else                    
		{       
			Channel::sendMoney(owner,Cmd::INFO_TYPE_GAME,realGet,"%s",note);
		}
	}
}

/**
* \brief �������Ҳ��ϲ���Ʒ,�����֪ͨ�ͻ���
* \param o ��Ʒ����
* \return true ��������,false ��ֹ����
*/
bool Combination::exec(zObject* o)
{
	if (o->data.dwObjectID == _get->data.dwObjectID && o->data.upgrade == _get->data.upgrade && o->data.dwNum < o->base->maxnum) {
		o->data.dwNum += _get->data.dwNum;
		if (o->data.dwNum > o->base->maxnum) {
			_num += (o->base->maxnum - (o->data.dwNum - _get->data.dwNum) );
			_get->data.dwNum = o->data.dwNum - o->base->maxnum;
			o->data.dwNum = o->base->maxnum;

			Cmd::stRefCountObjectPropertyUserCmd status;
			status.qwThisID = o->data.qwThisID;
			status.dwNum = o->data.dwNum;
			_user->sendCmdToMe(&status,sizeof(status));  
			return true;
		}


		_num += _get->data.dwNum;
		_get->data.dwNum = 0;

		Cmd::stRefCountObjectPropertyUserCmd status;
		status.qwThisID = o->data.qwThisID;
		status.dwNum = o->data.dwNum;
		_user->sendCmdToMe(&status,sizeof(status));  

		return false;
	}

	return true;
}

/*
* \param kind    ˵��ɾ����Ʒ��ԭ��
*      �磺����,����,etc
*
*
bool ClearPack::exec(zObject* ob,char *kind)
{
char event[128]={0};

if (kind == NULL)
strcpy(event,"������Ʒɾ��");
else
{
strcpy(event,kind);
strcat(event,"��Ʒɾ��");
}

zObject::logger(ob->createid,ob->data.qwThisID,ob->data.strName,ob->data.dwNum,ob->data.dwNum,0,const_cast<Packages* >(_ps)->getOwner()->id,const_cast<Packages* >(_ps)->getOwner()->name,0,NULL,event,ob->base,ob->data.kind,ob->data.upgrade);
const_cast<Packages* >(_ps)->removeObject(ob); //notify and delete
return true;
}*/
bool ClearPack::exec(zObject* ob)
{
	zObject::logger(ob->createid,ob->data.qwThisID,ob->data.strName,ob->data.dwNum,ob->data.dwNum,0,const_cast<Packages* >(_ps)->getOwner()->id,const_cast<Packages* >(_ps)->getOwner()->name,0,NULL,"������Ʒɾ��",ob->base,ob->data.kind,ob->data.upgrade);
	const_cast<Packages* >(_ps)->removeObject(ob); //notify and delete
	return true;
}


DropFromPack::DropFromPack(const Packages* ps,const Param& p) : _ps(ps),_p(p)
{ 
	int num = p.pack->size() -p.pack->space();
	_begin = (num < p.drop_num)?0:randBetween(0,num - p.drop_num);
	_pos = 0;
}


bool DropFromPack::exec(zObject* ob)
{
	/*
	//TODOBYLQY
	std::vector<zObject *> temp_vec;
	int begin = 0;
	std::set<zObject *>::iterator iter;
	if (mainpack > 0)
	{
	for(iter = packs.main.getAllset().begin(); iter != packs.main.getAllset().end() ; iter ++)
	{
	//TODO �������ɵ�����Ʒ
	if ((*iter)->data.upgrade > 5 || (*iter)->data.bind || (*iter)->data.pos.y == Cmd::EQUIPCELLTYPE_PACKAGE || (*iter)->data.pos.y == Cmd::EQUIPCELLTYPE_MAKE || (*iter)->base->kind==ItemType_MASK)
	{
	continue;
	}
	temp_vec.push_back(*iter);
	}
	if (mainpack < (int)temp_vec.size())
	{
	begin = randBetween(0,temp_vec.size() - mainpack);
	}
	else
	{
	mainpack = temp_vec.size();
	}
	for(int i = begin; i < mainpack ;  i ++)
	{
	this->packs.moveObjectToScene(&*temp_vec[i],this->getPos());
	}
	}
	*/  
	if (/*ob->data.upgrade > 5 || */ob->data.bind || 
		ob->data.pos.ypos() == Cmd::EQUIPCELLTYPE_PACKAGE || 
		ob->data.pos.ypos() == Cmd::EQUIPCELLTYPE_MAKE || ob->base->kind==ItemType_MASK ||
		ob->base->kind==ItemType_Quest) {
			//didn't drop
			return true;
	}
	++_pos;
	if (_pos >= _begin  && _p.drop_num > 0) {
		const_cast<Packages*>(_ps)->moveObjectToScene(ob,_p.pos);
		--_p.drop_num;
	}

	return true;
}

//////////////�������//////////////
PetPack::PetPack()
:Package(Cmd::OBJECTCELLTYPE_PET,0,8,10),available(0)
{
	setSpace(0);
}
PetPack::~PetPack() {}

bool PetPack::checkAdd(SceneUser* pUser,zObject* object,WORD x,WORD y)
{
	//fprintf(stderr,"װ����ƷPetPack");
	//isEmpty();
	if (x>W || y>H) return false;

	int pos = W*y + x;
	if (pos>available) return false;

	return Package::checkAdd(pUser,object,x,y);
}

void PetPack::setAvailable(WORD s)
{
	WORD old = available;
	available = s;

	WORD n = space();
	if (s>old)
		n += s-old;
	else if (old>s)
		n -= old-s;

	setSpace(n);
}

WORD PetPack::size() const
{
	return available;
}

bool PetPack::isEmpty() const
{
	return space()==size();
}


bool EquipPack::isEmpty()
{
	//fprintf(stderr,"�㵱ǰװ����%d����Ʒ\n",size());
	//return size() == 0;
	for(int i = 0; i < 16;++i)
		if(getObjectByEquipNo((EquipPack::EQUIPNO)i))
			return false;
	return true;
}
