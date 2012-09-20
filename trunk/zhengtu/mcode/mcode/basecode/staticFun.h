/*
 文件名 : staticFun.h
 创建时间 : 2012/9/20
 作者 : hyd
 功能 : 一些静态函数
*/
#ifndef __staticFun_H__
#define __staticFun_H__
#include <windows.h>

void sockStartup(void);
void sockCleanup(void);

int gettimeofday(struct timeval *tp,void *tzp);

#endif
