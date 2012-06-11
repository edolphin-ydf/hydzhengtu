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

//������Ʒ�Ƿ����ʹ��(��ȴʱ���Ѿ�����)
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
		//sky ��ƷID�����ȼ���������,�����ȼ��ID�Ƿ����ȴ����һ��
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

		//sky ID�޷�ƥ���ʱ���ڼ��������
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
			zRTime ct; //sky ��ȡ��ǰ����ʵʱ��
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

//������������һ����ȴ���͵���ȴ��ʼ
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
//		//sky ��ƷID�����ȼ���������,�����ȼ��ID�Ƿ����ȴ����һ��
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
//		//sky ID�޷�ƥ���ʱ���ڼ��������
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
//			zRTime ct;						//sky ��ȡ��ǰ����ʵʱ��
//			iter->StartTime = ct.msecs();	//sky �ѵ�ǰʱ��ŵ���Ʒ�Ŀ�ʼʱ����
//			break;
//		}
//	}
//
	return true;

}

//��ʼ��������
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