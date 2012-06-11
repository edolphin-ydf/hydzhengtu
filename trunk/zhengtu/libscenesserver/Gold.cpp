#include <zebra/ScenesServer.h>

bool SceneUser::npcTradeGold(Cmd::stBuyObjectNpcTradeUserCmd *ptCmd,zObjectB *base,BYTE itemlevel)
{
	SceneNpc * n = SceneNpcManager::getMe().getNpcByTempID(npc_dwNpcTempID);
	if (!n)
	{
		Zebra::logger->debug("[����:���<------�̵�]%s ����ʱ���Ҳ�����npc tempID=%u",name,npc_dwNpcTempID);
		return false;
	}
	DWORD need_gold=0;
	DWORD need_ticket=0;
	if (base && (charbase.gold + charbase.ticket) >= base->price*ptCmd->dwNum)
	{
		NpcTrade::NpcItem item;
		item.id = base->id;
		item.kind = base->kind;
		item.lowLevel = 0;
		item.level = base->needlevel;
		item.itemlevel = itemlevel;
		item.action = NpcTrade::NPC_BUY_OBJECT;
		if (NpcTrade::getInstance().verifyNpcAction(npc_dwNpcDataID,item))
		{
			//zObject *o = zObject::create(base,ptCmd->dwNum);
			zObject* o = NULL;
			if (base->recast)
			{
				o = zObject::create(base,ptCmd->dwNum,0);
				if (o && itemlevel)
				{
					do
					{
						//Upgrade::upgrade(*this,o,100); //must success
					}
					while (--itemlevel>0);
				}
			}else
			{
				o = zObject::create(base,ptCmd->dwNum,itemlevel);
			}
			if (o)
			{
				Combination callback(this,o);
				packs.main.execEvery(callback);
				if (packs.equip.pack(EquipPack::L_PACK)) packs.equip.pack(EquipPack::L_PACK)->execEvery(callback);
				if (packs.equip.pack(EquipPack::R_PACK)) packs.equip.pack(EquipPack::R_PACK)->execEvery(callback);

				int free = 0;
				if (o->data.dwNum) {

					if (packs.addObject(o,true,AUTO_PACK)) {
						free = o->data.dwNum;
						//�����˫��������ߺ�����������Ҫ��
						o->checkBind();
						Cmd::stAddObjectPropertyUserCmd status;
						status.byActionType = Cmd::EQUIPACTION_OBTAIN;
						bcopy(&o->data,&status.object,sizeof(t_Object),sizeof(status.object));
						sendCmdToMe(&status,sizeof(status));
					}
				}



				if (callback.num() || free) 
				{
					//get object
					int count = callback.num() + free;
					DWORD need=base->price*count;
					if (charbase.gold >=need)
					{
						need_gold=need;
						//charbase.gold -=need_gold;
					}
					else
					{
						need_gold=charbase.gold;
						//charbase.gold=0;
						need_ticket=need;
						//charbase.ticket-=need_ticket;
					}
					if (need_gold)
					{
						std::string disc = "����:"; 
						disc += o->base->name;
						if (!packs.removeGold(need_gold,disc.c_str())) {
							Zebra::logger->fatal("[����:���<------�̵�]�û�(%s)��%sʱ��Ҽ������!",name,o->base->name);
						}
					}
					if (need_ticket)
					{
						std::string disc = "����:"; 
						disc += o->base->name;
						if (!packs.removeTicket(need_ticket,disc.c_str())) {
							Zebra::logger->fatal("[����:���<------�̵�]�û�(%s)��%sʱ��ȯ�����!",name,o->base->name);
						}
					}
					zObject::logger(o->createid,o->data.qwThisID,o->data.strName,o->data.dwNum,count,1,n->id,n->name,this->id,this->name,"buy_npc",o->base,o->data.kind,o->data.upgrade);
					if (need_gold && !need_ticket)
					{
						//Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�õ���Ʒ %s(%d)��,���ѽ���%d��,",o->name,count,need_gold);
						Channel::sendGold(this,Cmd::INFO_TYPE_GAME,need_gold,"�õ���Ʒ %s(%d)��,���ѽ���",o->name,count);
					}
					else if (need_gold && need_ticket)
					{
						Channel::sendGold(this,Cmd::INFO_TYPE_GAME,need_gold,"�õ���Ʒ %s(%d)��,���ѻ���%d,���ѽ���",o->name,count,need_ticket);
						//Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�õ���Ʒ %s(%d)��,���ѽ���%d��,���ѻ���%d��",o->name,count,need_gold);
					}
					else
					{
						Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�õ���Ʒ %s(%d)��,���ѻ���%d��",o->name,count,need_ticket);
					}
				}
				if (!free) { //package is full
					//Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��İ�������");
					zObject::destroy(o);
				}                  
			}
		}
	}
	else
	{
		Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"���Ľ��Ӳ���");
	}
	return true;
}
