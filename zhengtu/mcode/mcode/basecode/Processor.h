/*
 文件名 : Processor.h
 创建时间 : 2012/9/18
 作者 : hyd
 功能 : 定义了消息处理接口，所有接收到的TCP数据指令需要通过这个接口来处理
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
