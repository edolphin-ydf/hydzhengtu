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
#ifndef _RPACKET_H
#define _RPACKET_H

#include "buffer.h"
#include "link_list.h"
#include <stdint.h>
typedef struct rpacket
{
	list_node next;
	uint32_t len;     //����(ȥ���������ֶ�)
	uint32_t rpos;    //���±�
	uint32_t data_remain;
	uint32_t binbufpos;
	uint32_t begin_pos;
	buffer_t binbuf;       //���ڴ�ſ�Խbuffer_t�߽����ݵ�buffer_t
	buffer_t buf;          //��Ŵ����ݰ����ݵ�buffer_t����
	buffer_t readbuf;      //��ǰrpos���ڵ�buffer_t
	uint8_t  raw;          //ԭʼ�ֽ������ݰ�
}*rpacket_t;

struct wpacket;

rpacket_t rpacket_create(buffer_t,uint32_t pos,uint32_t pk_len,uint8_t is_raw);
rpacket_t rpacket_create_by_wpacket(struct wpacket*);//ͨ��wpacket����
void      rpacket_destroy(rpacket_t*);

//���ݶ�ȡ�ӿ�
uint32_t  rpacket_len(rpacket_t);
uint32_t  rpacket_data_remain(rpacket_t);
uint8_t rpacket_read_uint8(rpacket_t);
uint16_t rpacket_read_uint16(rpacket_t);
uint32_t rpacket_read_uint32(rpacket_t);
uint64_t rpacket_read_uint64(rpacket_t);

double         rpacket_read_double(rpacket_t);
const char*    rpacket_read_string(rpacket_t);
const void*    rpacket_read_binary(rpacket_t,uint32_t *len);
void init_rpacket_pool(uint32_t pool_size);

#endif