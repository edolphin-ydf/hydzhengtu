#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"

extern uint32_t bf_count;

static struct link_list *g_buffer_max;
static struct link_list *g_buffer_64;

void     buffer_init_maxbuffer_size(uint32_t pool_size)
{
	uint32_t i = 0;
	uint32_t size = sizeof(struct buffer) + 16384;
	buffer_t b;
	g_buffer_max = LIST_CREATE();
	for( ; i < pool_size; ++i)
	{
		b = calloc(1,size);
		LIST_PUSH_BACK(g_buffer_max,b);
	}
}

void     buffer_init_64(uint32_t pool_size)
{
	uint32_t i = 0;
	uint32_t size = sizeof(struct buffer) + 64;
	buffer_t b;
	g_buffer_64 = LIST_CREATE();
	for( ; i < pool_size; ++i)
	{
		b = calloc(1,size);
		LIST_PUSH_BACK(g_buffer_64,b);
	}

}




static buffer_t buffer_create(uint32_t capacity)
{
	uint32_t size = sizeof(struct buffer) + capacity;
	//buffer_t b = calloc(1,size);
	
	buffer_t b;
	if(capacity == 16384)
		b = LIST_POP(buffer_t,g_buffer_max);
	else if(capacity == 64)
		b = LIST_POP(buffer_t,g_buffer_64);
	else
		b = calloc(1,size);
	
	if(b)
	{
		//printf("buffer create\n");
		b->ref_count = 0;
		b->size = 0;
		b->capacity = capacity;
		++bf_count;
	}
	return b;
}

static void     buffer_destroy(buffer_t *b)
{
	//printf("buffer destroy\n");
	if((*b)->next)
		buffer_release(&(*b)->next);
	
	if((*b)->capacity == 16384)
		LIST_PUSH_BACK(g_buffer_max,*b);
	else if((*b)->capacity == 64)
		LIST_PUSH_BACK(g_buffer_64,*b);
	else
		free(*b);
	//free(*b);
	*b = 0;
	--bf_count;
}

buffer_t buffer_create_and_acquire(buffer_t b,uint32_t capacity)
{
	buffer_t nb = buffer_create(capacity);
	return buffer_acquire(b,nb);
}

buffer_t buffer_acquire(buffer_t b1,buffer_t b2)
{
	if(b1 == b2)
		return b1;	
	if(b2)
		++b2->ref_count;
	if(b1)
		buffer_release(&b1);

	return b2;
}

void buffer_release(buffer_t *b)
{
	if(*b)
	{
		if(--(*b)->ref_count <= 0)
			buffer_destroy(b);
		*b = 0;
	}
}

int buffer_read(buffer_t b,uint32_t pos,int8_t *out,uint32_t size)
{
	uint32_t copy_size;
	while(size)
	{
		if(!b)
			return -1;
		copy_size = b->size - pos;
		copy_size = copy_size > size ? size : copy_size;
		memcpy(out,b->buf + pos,copy_size);
		size -= copy_size;
		pos += copy_size;
		out += copy_size;
		if(pos >= b->size)
		{
			pos = 0;
			b = b->next;
		}
	}
	return 0;
}