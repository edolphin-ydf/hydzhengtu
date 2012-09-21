#ifndef _BUFFER_H
#define _BUFFER_H
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
/*
* 带引用计数的buffer
*/

#include "link_list.h"
#include <stdint.h>
typedef struct buffer
{
	list_node lnode;
	int32_t ref_count;
	uint32_t capacity;
	uint32_t size;
	struct buffer *next;
	int8_t   buf[0];
}*buffer_t;


buffer_t buffer_create_and_acquire(buffer_t,uint32_t);
buffer_t buffer_acquire(buffer_t,buffer_t);
void     buffer_release(buffer_t*);
int32_t      buffer_read(buffer_t,uint32_t,int8_t*,uint32_t);

void     buffer_init_maxbuffer_size(uint32_t);
void     buffer_init_64(uint32_t);



#endif