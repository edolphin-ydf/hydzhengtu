/*
 �ļ��� : Processor.h
 ����ʱ�� : 2012/9/18
 ���� : hyd
 ���� : ��������Ϣ����ӿڣ����н��յ���TCP����ָ����Ҫͨ������ӿ�������
*/
#ifndef __Processor_H__
#define __Processor_H__

#include "Common.h"
class Processor
{
public:
	virtual bool msgParse(const Cmd::Cmd_NULL *,const DWORD) = 0;
};
#endif
