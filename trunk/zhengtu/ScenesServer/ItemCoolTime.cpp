#include "zebra/ItemCoolTime.h"
#include "zebra/ScenesServer.h"

CItmeCoolTime::CItmeCoolTime()
{
	vCoolTimeType.clear();
	InitCoolTimeType();
}

CItmeCoolTime::~CItmeCoolTime()
{

}

//检测该物品是否可以使用(冷却时间已经结束)
bool CItmeCoolTime::IsCoolTimeOver(zObject* obj)
{
	if(vXmlItemCoolTime.empty())
		return true;

	DWORD coolTimeType = 0;
	DWORD coolTime = 0;

	DWORD dwID = obj->base->id;
	DWORD dwType = obj->base->kind;

	std::vector<stCoolTimeType>::iterator iter;
	std::vector<stXmlItemCoolTime>::iterator iter1;
	std::vector<stItemIdCoolTime>::iterator iter2;
	std::vector<stItemTypeCoolTiem>::iterator iter3;

	for(iter1=vXmlItemCoolTime.begin(); iter1!=vXmlItemCoolTime.end(); iter1++)
	{
		//sky 物品ID的优先级别高于类别,所以先检测ID是否和冷却配置一致
		for(iter2=iter1->IdCoolTime.begin(); iter2!=iter1->IdCoolTime.end(); iter2++)
		{
			if(dwID == iter2->ItemID)
			{
				coolTimeType = iter1->CoolTimeType;

				if(iter2->CoolTime > 0)
					coolTime = iter2->CoolTime;
				else
					coolTime = iter1->nCoolTime;

				goto IsCoolParser;
			}

		}

		//sky ID无法匹配的时候在检测下类型
		for(iter3=iter1->TypeCoolTime.begin(); iter3!=iter1->TypeCoolTime.end(); iter3++)
		{
			if(dwType == iter3->ItemType)
			{
				coolTimeType = iter1->CoolTimeType;

				if(iter3->CoolTime > 0)
					coolTime = iter3->CoolTime;
				else
					coolTime = iter1->nCoolTime;

				goto IsCoolParser;
			}
		}
	}

IsCoolParser:
	for(iter=vCoolTimeType.begin(); iter!=vCoolTimeType.end(); iter++)
	{
		if(coolTimeType == iter->TimeType)
		{
			zRTime ct; //sky 获取当前的真实时间
			DWORD OldTime = (DWORD)(ct.msecs() - iter->StartTime);
			if (coolTime > OldTime)
			{
				if ((float)(OldTime)/(float)coolTime < 0.0f)
					return true;
				else
					return false;
			}
			iter->StartTime = ct.msecs();
			return true;
		}
	}

	return true;
}

//向管理器里添加一个冷却类型的冷却开始
bool CItmeCoolTime::AddCoolTimeStar(zObject* obj)
{

//	if(vXmlItemCoolTime.empty())
//		return true;
//
//	DWORD coolTimeType = 0;
//
//	DWORD dwID = obj->base->id;
//	DWORD dwType = obj->base->kind;
//
//	std::vector<stCoolTimeType>::iterator iter;
//	std::vector<stXmlItemCoolTime>::iterator iter1;
//	std::vector<stItemIdCoolTime>::iterator iter2;
//	std::vector<stItemTypeCoolTiem>::iterator iter3;
//
//	for(iter1=vXmlItemCoolTime.begin(); iter1!=vXmlItemCoolTime.end(); iter1++)
//	{
//		//sky 物品ID的优先级别高于类别,所以先检测ID是否和冷却配置一致
//		for(iter2=iter1->IdCoolTime.begin(); iter2!=iter1->IdCoolTime.end(); iter2++)
//		{
//			if(dwID == iter2->ItemID)
//			{
//				coolTimeType = iter1->CoolTimeType;
//
//				goto AddCoolParser;
//			}
//
//		}
//
//		//sky ID无法匹配的时候在检测下类型
//		for(iter3=iter1->TypeCoolTime.begin(); iter3!=iter1->TypeCoolTime.end(); iter3++)
//		{
//			if(dwType == iter3->ItemType)
//			{
//				coolTimeType = iter1->CoolTimeType;
//
//				goto AddCoolParser;
//			}
//		}
//	}
//
//AddCoolParser:
//	for(iter=vCoolTimeType.begin(); iter!=vCoolTimeType.end(); iter++)
//	{
//		if(coolTimeType == iter->TimeType)
//		{
//			zRTime ct;						//sky 获取当前的真实时间
//			iter->StartTime = ct.msecs();	//sky 把当前时间放到物品的开始时间中
//			break;
//		}
//	}
//
	return true;

}

//初始化管理器
bool CItmeCoolTime::InitCoolTimeType()
{
	std::vector<stXmlItemCoolTime>::iterator iter;

	vCoolTimeType.resize(vXmlItemCoolTime.size());

	int i = 0;
	if(!vCoolTimeType.empty())
	{
		for(iter=vXmlItemCoolTime.begin(); iter!=vXmlItemCoolTime.end(); iter++)
		{
			vCoolTimeType[i].TimeType = iter->CoolTimeType;
			vCoolTimeType[i].StartTime = 0;
			i++;
		}

		return true;
	}

	return false;	
}