/*
 文件名 : CmdAnalysis.h
 创建时间 : 2012/9/18
 作者 : hyd
 功能 : 指令流量分析
*/
#ifndef __CmdAnalysis_H__
#define __CmdAnalysis_H__
#include "Common.h"
#include "Mutex.h"
#include "Timer.h"

struct CmdAnalysis
{
	CmdAnalysis(const char *disc,DWORD time_secs):_log_timer(time_secs)
	{
		memset(_disc,0,sizeof(disc));
		strncpy_s(_disc,disc,sizeof(_disc)-1);
		memset(_data,0,sizeof(_data));
		_switch=false;
	}
	struct
	{
		DWORD num;
		DWORD size;
	}_data[256][256] ;
	Mutex _mutex;
	Timer _log_timer;
	char _disc[256];
	bool _switch;//开关
	void add(const BYTE &cmd,const BYTE &para,const DWORD &size)
	{
		if (!_switch)
		{
			return;
		}
		_mutex.lock(); 
		_data[cmd][para].num++;
		_data[cmd][para].size +=size;
		RTime ct;
		if (_log_timer(ct))
		{
			for(int i = 0 ; i < 256 ; i ++)
			{
				for(int j = 0 ; j < 256 ; j ++)
				{
					if (_data[i][j].num)
						printf("%s:%d,%d,%d,%d",_disc,i,j,_data[i][j].num,_data[i][j].size);
				}
			}
			memset(_data,0,sizeof(_data));
		}
		_mutex.unlock(); 
	}
};
#endif
