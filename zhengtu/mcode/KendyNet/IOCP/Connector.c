#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include "Connector.h"
#include <stdio.h>
#include "link_list.h"
typedef struct pending_connect
{
	list_node  lnode;
	const char *ip;
	uint32_t port;
	SOCKET   sock;
	on_connect call_back;
	uint32_t timeout;
	void     *ud;
};

struct connector
{
	FD_SET Set;
	struct link_list *_pending_connect;
	uint32_t fd_seisize;
};

connector_t connector_create()
{
	connector_t c = malloc(sizeof(*c));
	c->fd_seisize = 0;
	FD_ZERO(&c->Set);
	c->_pending_connect = create_list();
	return c;
}

void connector_destroy(connector_t *c)
{
	struct pending_connect *pc;
	while(pc = LIST_POP(struct pending_connect*,(*c)->_pending_connect))
		free(pc);
	free(*c);
	*c = 0;
}

int connector_connect(connector_t c,const char *ip,uint32_t port,on_connect call_back,void *ud,uint32_t ms)
{
	struct sockaddr_in remote;
	ULONG NonBlock = 1; 
	SOCKET sock;
	struct pending_connect *pc;
	int slot = -1;
	if(c->fd_seisize >= FD_SETSIZE)
		return -1;
	sock =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("\nError occurred while opening socket: %d.", WSAGetLastError());
		return -1;
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons((u_short)port);
	remote.sin_addr.s_addr = inet_addr(ip);
	if(ms>0)
	{
		if (ioctlsocket(sock, FIONBIO, &NonBlock) == SOCKET_ERROR)
		{
			closesocket(sock);
			printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
			return -1;
		}
	}
	if(connect(sock, (struct sockaddr *)&remote, sizeof(remote)) >=0 )
	{
		//连接成功,无需要后续处理了,直接调用回调函数
		call_back(sock,ip,port,ud);
		return 0;
	}

	pc = malloc(sizeof(*pc));
	pc->lnode.next = NULL;
	pc->sock = sock;
	pc->ip = ip;
	pc->port = port;
	pc->call_back = call_back;
	pc->timeout = GetTickCount() + ms;
	pc->ud = ud;
	FD_SET(sock,&c->Set);
	LIST_PUSH_BACK(c->_pending_connect,pc);
	++c->fd_seisize;
	return 0;
}

void connector_run(connector_t c,uint32_t ms)
{
	int32_t i = 0;
	DWORD tick,_timeout,_ms;
	int32_t size;
	int32_t total;
	struct pending_connect *pc;
	struct timeval timeout;
	tick = GetTickCount();
	_timeout = tick + ms;
	do{
		_ms = _timeout - tick;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000*_ms;
		size = list_size(c->_pending_connect);
		if(size == 0)
			return;
		if((total = select(0, NULL,&c->Set, NULL, &timeout)) >0 )
		{
			for(; i < size; ++i)
			{
				pc = LIST_POP(struct pending_connect*,c->_pending_connect);
				if(pc)
				{
					if(FD_ISSET(pc->sock, &c->Set))
					{
						pc->call_back(pc->sock,pc->ip,pc->port,pc->ud);
						free(pc);
						--c->fd_seisize;
					}
					else
						LIST_PUSH_BACK(c->_pending_connect,pc);
				}
			}
		}
		FD_ZERO(&c->Set);
		tick = GetTickCount();
		size = list_size(c->_pending_connect);
		i = 0;
		for(; i < (int32_t)size; ++i)
		{
			pc = LIST_POP(struct pending_connect*,c->_pending_connect);
			if(tick >= pc->timeout)
			{
				//超时了
				pc->call_back(INVALID_SOCKET,pc->ip,pc->port,pc->ud);
				free(pc);
				--c->fd_seisize;
			}
			else
			{
				LIST_PUSH_BACK(c->_pending_connect,pc);
				FD_SET(pc->sock,&c->Set);
			}
		}
		tick = GetTickCount();
	}while(tick < _timeout);
}