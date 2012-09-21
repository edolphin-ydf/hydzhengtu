#ifndef _CONNECTION_H
#define _CONNECTION_H
#include "KendyNet.h"
#include "wpacket.h"
#include "rpacket.h"
#include <stdint.h>
struct OVERLAPCONTEXT
{
	struct OverLapContext m_super;
	int8_t   isUsed;
};

typedef void (*process_packet)(struct connection*,rpacket_t);
typedef void (*on_connection_destroy)(struct connection*);

#define MAX_WBAF 1024

struct connection
{
	struct Socket socket;
	WSABUF wsendbuf[MAX_WBAF];
	WSABUF wrecvbuf[2];
	struct OVERLAPCONTEXT send_overlap;
	struct OVERLAPCONTEXT recv_overlap;

	uint32_t unpack_size; //��δ��������ݴ�С

	uint32_t unpack_pos;
	uint32_t next_recv_pos;

	buffer_t next_recv_buf;
	buffer_t unpack_buf; 
	
	struct link_list *send_list;//�����͵İ�
	process_packet _process_packet;
	on_connection_destroy _on_destroy;
	uint8_t raw;
};

struct connection *connection_create(SOCKET s,uint8_t is_raw,process_packet,on_connection_destroy);
void connection_destroy(struct connection**);

//�����Ѱ����뷢�Ͷ���
void connection_push_packet(struct connection*,wpacket_t);

//����ֵ:0,���ӶϿ�;��������
int32_t connection_send(struct connection*,wpacket_t,int32_t);

int32_t connection_recv(struct connection*);

#endif