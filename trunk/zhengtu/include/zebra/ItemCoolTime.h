//sky ������Ʒ��ȴ������
#pragma once
#include <zebra/csCommon.h>

#include <zebra/srvEngine.h>

struct SceneUser;
class zObject;

typedef struct stCoolTimeType
{
	DWORD TimeType;  //��ȴ����
	QWORD StartTime; //��ȴ��ʼʱ��
};

class CItmeCoolTime
{
public:
	CItmeCoolTime();
	~CItmeCoolTime();

	//��ȴ������(��¼��������ȴ���͵Ŀ�ʼʱ��)
	std::vector<stCoolTimeType> vCoolTimeType;

public:
	//������Ʒ�Ƿ����ʹ��(��ȴʱ���Ѿ�����)
	bool IsCoolTimeOver(zObject* obj);
	//������������һ����ȴ���͵���ȴ��ʼ
	bool AddCoolTimeStar(zObject* obj);
	//��ʼ��������
	bool InitCoolTimeType();
};