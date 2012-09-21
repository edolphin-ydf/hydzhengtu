#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include <stdio.h>
#include "Connection.h"
#include <assert.h>

#define BUFFER_SIZE 16384

//接收相关函数
static void update_next_recv_pos(struct connection *c,int32_t bytestransfer)
{
	uint32_t size;		
	while(bytestransfer)
	{
		size = c->next_recv_buf->capacity - c->next_recv_pos;
		size = size > (uint32_t)bytestransfer ? (uint32_t)bytestransfer:size;
		c->next_recv_buf->size += size;
		c->next_recv_pos += size;
		bytestransfer -= size;
		if(c->next_recv_pos >= c->next_recv_buf->capacity)
		{
			if(!c->next_recv_buf->next)
				c->next_recv_buf->next = buffer_create_and_acquire(0,BUFFER_SIZE);
			c->next_recv_buf = buffer_acquire(c->next_recv_buf,c->next_recv_buf->next);
			c->next_recv_pos = 0;
		}
	}
}

//解包
static /*rpacket_t*/void unpack(struct connection *c)
{
	uint32_t pk_len = 0;
	uint32_t pk_total_size;
	rpacket_t r;
	for(;;)
	{

		if(!c->raw)
		{		
			if(c->unpack_size <= sizeof(uint32_t))
				break;//return 0;
			buffer_read(c->unpack_buf,c->unpack_pos,(int8_t*)&pk_len,sizeof(pk_len));
			pk_total_size = pk_len+sizeof(pk_len);
			if(pk_total_size > c->unpack_size)
				break;//return 0;
			r = rpacket_create(c->unpack_buf,c->unpack_pos,pk_len,c->raw);
			//调整unpack_buf和unpack_pos
			while(pk_total_size)
			{
				uint32_t size = c->unpack_buf->size - c->unpack_pos;
				size = pk_total_size > size ? size:pk_total_size;
				c->unpack_pos  += size;
				pk_total_size  -= size;
				c->unpack_size -= size;
				if(c->unpack_pos >= c->unpack_buf->capacity)
				{
					/* unpack之前先执行了update_next_recv_pos,在update_next_recv_pos中
					*  如果buffer被填满，会扩展一块新的buffer加入链表中，所以这里的
					*  c->unpack_buf->next不应该会是NULL
					*/
					assert(c->unpack_buf->next);
					c->unpack_pos = 0;
					c->unpack_buf = buffer_acquire(c->unpack_buf,c->unpack_buf->next);
				}
			}
		}
		else
		{
			pk_len = c->unpack_buf->size - c->unpack_pos;
			if(!pk_len)
				return; 
			r = rpacket_create(c->unpack_buf,c->unpack_pos,pk_len,c->raw);
			c->unpack_pos  += pk_len;
			c->unpack_size -= pk_len;
			if(c->unpack_pos >= c->unpack_buf->capacity)
			{
				/* unpack之前先执行了update_next_recv_pos,在update_next_recv_pos中
				*  如果buffer被填满，会扩展一块新的buffer加入链表中，所以这里的
				*  c->unpack_buf->next不应该会是NULL
				*/
				assert(c->unpack_buf->next);
				c->unpack_pos = 0;
				c->unpack_buf = buffer_acquire(c->unpack_buf,c->unpack_buf->next);
			}
		}
		c->_process_packet(c,r);
	}
	//return r;
}


void RecvFinish(struct Socket *s,struct OverLapContext *o,int32_t bytestransfer,uint32_t err_code)
{
	struct OVERLAPCONTEXT *OVERLAP = (struct OVERLAPCONTEXT *)o;
	struct connection *c = (struct connection *)s;
	uint32_t recv_size;
	uint32_t free_buffer_size;
	buffer_t buf;
	uint32_t pos;
	int32_t i = 0;
	for(;;)
	{
		if(bytestransfer == 0 || bytestransfer < 0 && err_code != WSA_IO_PENDING)
		{
			c->recv_overlap.isUsed = 0;
			if(!c->send_overlap.isUsed)
			{
				printf("断开:%d\n",err_code);
				closesocket(c->socket.sock);
				connection_destroy(&c);
			}
			break;
		}
		else if(bytestransfer < 0 && err_code == WSA_IO_PENDING)
		{
			break;
		}
		else
		{
			while(bytestransfer > 0)
			{
				update_next_recv_pos(c,bytestransfer);
				c->unpack_size += bytestransfer;
				unpack(c);
				//while(r = unpack(c))
				//	c->_process_packet(c,r);
				//发起另一次读操作
				buf = c->next_recv_buf;
				pos = c->next_recv_pos;
				recv_size = BUFFER_SIZE;
				i = 0;
				while(recv_size)
				{
					free_buffer_size = buf->capacity - pos;
					free_buffer_size = recv_size > free_buffer_size ? free_buffer_size:recv_size;
					c->wrecvbuf[i].len = free_buffer_size;
					c->wrecvbuf[i].buf = buf->buf + pos;
					recv_size -= free_buffer_size;
					pos += free_buffer_size;
					if(recv_size && pos >= buf->capacity)
					{
						pos = 0;
						if(!buf->next)
							buf->next = buffer_create_and_acquire(0,BUFFER_SIZE);
						buf = buf->next;
					}
					++i;
				}

				c->recv_overlap.isUsed = 1;
				c->recv_overlap.m_super.buf_count = i;
				c->recv_overlap.m_super.wbuf = c->wrecvbuf;
				bytestransfer = WSA_Recv(s,(struct OverLapContext*)&c->recv_overlap,1,&err_code);
			}
		}
	}
}

//发送相关函数
static  struct OverLapContext *prepare_send(struct connection *c)
{
	int32_t i = 0;
	wpacket_t w = (wpacket_t)list_head(c->send_list);
	buffer_t b;
	uint32_t pos;
	struct OverLapContext *O = 0;
	uint32_t buffer_size = 0;
	uint32_t size = 0;
	while(w && i < MAX_WBAF)
	{
		pos = w->begin_pos;
		b = w->buf;
		buffer_size = w->data_size;
		while(i < MAX_WBAF && b && buffer_size)
		{
			c->wsendbuf[i].buf = b->buf + pos;
			size = b->size - pos;
			size = size > buffer_size ? buffer_size:size;
			buffer_size -= size;
			c->wsendbuf[i].len = size;
			++i;
			b = b->next;
			pos = 0;
		}
		w = (wpacket_t)w->next.next;
	}
	if(i)
	{
		c->send_overlap.m_super.buf_count = i;
		c->send_overlap.m_super.wbuf = c->wsendbuf;
		O = (struct OverLapContext *)&c->send_overlap;
	}
	return O;

}
extern DWORD packet_send;
static void update_send_list(struct connection *c,int32_t bytestransfer)
{
	wpacket_t w;
	uint32_t size;
	while(bytestransfer)
	{
		w = LIST_POP(wpacket_t,c->send_list);
		if(!w)
		{
			//记录错误
			break;
		}
		if((uint32_t)bytestransfer >= w->data_size)
		{
			//一个包发完
			bytestransfer -= w->data_size;
			wpacket_destroy(&w);
			++packet_send;
		}
		else
		{
			while(bytestransfer)
			{
				size = w->buf->size - w->begin_pos;
				size = size > (uint32_t)bytestransfer ? (uint32_t)bytestransfer:size;
				bytestransfer -= size;
				w->begin_pos += size;
				w->data_size -= size;
				if(w->begin_pos >= w->buf->size)
				{
					w->begin_pos = 0;
					w->buf = buffer_acquire(w->buf,w->buf->next);
				}
			}
			LIST_PUSH_FRONT(c->send_list,w);
		}
	}
}

extern uint32_t s_p;
int32_t connection_send(struct connection *c,wpacket_t w,int32_t send)
{
	int32_t bytestransfer = 0;
	uint32_t err_code = 0;
	struct OverLapContext *O;
	int32_t ret = 1;
	if(w)
		LIST_PUSH_BACK(c->send_list,w);
	if(!c->send_overlap.isUsed)
	{
		c->send_overlap.isUsed = 1;
		while(O = prepare_send(c))
		{
			bytestransfer = WSA_Send(&c->socket,O,send,&err_code);			
			if(bytestransfer == 0 || (bytestransfer < 0 && err_code != WSA_IO_PENDING))
			{
				c->send_overlap.isUsed = 0;
				return 0;
			}
			else if(bytestransfer > 0)
			{
				update_send_list(c,bytestransfer);
			}
			else 
			{
				++s_p;
				return 1;
			}
		}
		c->send_overlap.isUsed = 0;
	}
	return 1;
}

void connection_push_packet(struct connection *c,wpacket_t w)
{
	if(w)
	{
		LIST_PUSH_BACK(c->send_list,w);
		//printf("list size:%d\n",list_size(c->send_list));
	}
}

void SendFinish(struct Socket *s,struct OverLapContext *o,int32_t bytestransfer,uint32_t err_code)
{
	struct OVERLAPCONTEXT *OVERLAP = (struct OVERLAPCONTEXT *)o;
	struct connection *c = (struct connection *)s;
	for(;;)
	{
		if(bytestransfer == 0 || bytestransfer < 0 && err_code != WSA_IO_PENDING)
		{
			c->send_overlap.isUsed = 0;
			if(!c->recv_overlap.isUsed)
			{
				printf("断开:%d\n",err_code);
				closesocket(c->socket.sock);
				connection_destroy(&c);
			}
			break;
		}
		else if(bytestransfer < 0 && err_code == WSA_IO_PENDING)
		{
			break;
		}
		else
		{
			while(bytestransfer > 0)
			{
				update_send_list(c,bytestransfer);
				o = prepare_send(c);
				if(!o)
				{
					//没有数据需要发送了
					c->send_overlap.isUsed = 0;
					return;
				}
				bytestransfer = WSA_Send(&c->socket,o,1,&err_code);
			}
		}
	}
}

struct connection *connection_create(SOCKET s,uint8_t is_raw,process_packet _process_packet,on_connection_destroy on_destroy)
{
	struct connection *c = calloc(1,sizeof(*c));
	c->socket.sock = s;
	c->socket.RecvFinish = RecvFinish;
	c->socket.SendFinish = SendFinish;
	c->send_list = LIST_CREATE();
	c->_process_packet = _process_packet;
	c->_on_destroy = on_destroy;
	c->next_recv_buf = 0;
	c->next_recv_pos = 0;
	c->unpack_buf = 0;
	c->unpack_pos = 0;
	c->unpack_size = 0;
	c->raw = is_raw;
	return c;
}

void connection_destroy(struct connection** c)
{
	wpacket_t w;
	(*c)->_on_destroy(*c);

	while(w = LIST_POP(wpacket_t,(*c)->send_list))
		wpacket_destroy(&w);

	LIST_DESTROY(&(*c)->send_list);
	buffer_release(&(*c)->unpack_buf);
	buffer_release(&(*c)->next_recv_buf);
	free(*c);
	*c = 0;
}

int connection_recv(struct connection *c)
{
	DWORD err_code;
	c->unpack_buf = buffer_create_and_acquire(0,BUFFER_SIZE);
	c->next_recv_buf = buffer_acquire(0,c->unpack_buf);
	c->next_recv_pos = c->unpack_pos = c->unpack_size = 0;
	c->wrecvbuf[0].len = BUFFER_SIZE;
	c->wrecvbuf[0].buf = c->next_recv_buf->buf;
	c->recv_overlap.m_super.buf_count = 1;
	c->recv_overlap.isUsed = 1;
	c->recv_overlap.m_super.wbuf = c->wrecvbuf;
	return WSA_Recv(&c->socket,&c->recv_overlap.m_super,0,&err_code);
}
