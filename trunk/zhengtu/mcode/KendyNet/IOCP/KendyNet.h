/*	
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/	
#ifndef _KENDYNET_H
#define _KENDYNET_H
#include "Connector.h"
#include "Acceptor.h"
#include <stdint.h>

enum
{
	IO_ERROR = -1,
	UNBIND2ENGINE = -2,
};

//重叠结构
struct OverLapContext
{
	OVERLAPPED    m_overLapped;
	WSABUF*       wbuf;
	DWORD         buf_count;
	uint8_t       m_Type;
};
struct Socket;

//socket的简单封装
typedef struct Socket
{
	SOCKET sock;
	HANDLE complete_port;
	void (*RecvFinish)(struct Socket*,struct OverLapContext*,int32_t,uint32_t);
	void (*SendFinish)(struct Socket*,struct OverLapContext*,int32_t,uint32_t);
}*Socket_t;

int32_t    InitNetSystem();
void   CleanNetSystem();

HANDLE CreateNetEngine(DWORD NumberOfConcurrentThreads);
void   CloseNetEngine(HANDLE);
int32_t    RunEngine(HANDLE,DWORD timeout);
int32_t    Bind2Engine(HANDLE,Socket_t);

//now表示是否立即发起操作
int32_t    WSA_Send(Socket_t,struct OverLapContext*,int32_t now,uint32_t *err_code);
int32_t    WSA_Recv(Socket_t,struct OverLapContext*,int32_t now,uint32_t *err_code);



#endif