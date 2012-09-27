#include "rpacket.h"
#include "wpacket.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static struct link_list *g_rpacket_pool;

void init_rpacket_pool(uint32_t pool_size)
{
	uint32_t i = 0;
	rpacket_t r;// = calloc(1,sizeof(*w));
	g_rpacket_pool = LIST_CREATE();
	for( ; i < pool_size; ++i)
	{
		r = calloc(1,sizeof(*r));
		LIST_PUSH_BACK(g_rpacket_pool,r);
	}
}

rpacket_t rpacket_create(buffer_t b,uint32_t pos,uint32_t pk_len,uint8_t is_raw)
{

	rpacket_t r = LIST_POP(rpacket_t,g_rpacket_pool);//�ӻ�����ȡ��һ��packet,����calloc(1,sizeof(*r));
	if(!r)
	{
		printf("r���治����\n");
		getchar();
		exit(0);
	}

	//rpacket_t r = calloc(1,sizeof(*r));
	
	r->binbuf = 0;
	r->binbufpos = 0;
	r->buf = buffer_acquire(0,b);
	r->readbuf = buffer_acquire(0,b);
	r->len = pk_len;
	r->data_remain = r->len;
	r->begin_pos = pos;
	if(is_raw)
		r->rpos = pos;
	else
		r->rpos = pos + sizeof(r->len);
	r->raw = is_raw;
	return r;
}

rpacket_t rpacket_create_by_wpacket(struct wpacket *w)
{
	rpacket_t r = LIST_POP(rpacket_t,g_rpacket_pool);//calloc(1,sizeof(*r));
	if(!r)
	{
		printf("r���治����\n");
		getchar();
		exit(0);
	}
	//rpacket_t r = calloc(1,sizeof(*r));
	
	r->binbuf = 0;
	r->binbufpos = 0;
	r->buf = buffer_acquire(0,w->buf);
	r->readbuf = buffer_acquire(0,w->buf);
	r->raw = w->raw;
	r->begin_pos = w->begin_pos;	
	if(r->raw)
	{
		r->len = w->data_size;
		r->rpos =w->begin_pos;
	}
	else
	{
		//�����lenֻ��¼����ʱwpacket��len,֮��wpacket��д�벻��Ӱ�쵽rpacket��len
		r->len = w->data_size - sizeof(r->len);
		if(r->readbuf->size - r->begin_pos > sizeof(r->len))
			r->rpos = w->begin_pos + sizeof(r->len);
		else
		{
			r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
			r->rpos = 0 + sizeof(r->len);
		}
	}
	r->data_remain = r->len;
	return r;
}

void      rpacket_destroy(rpacket_t *r)
{
	//�ͷ����ж�buffer_t������
	
	buffer_release(&(*r)->buf);
	buffer_release(&(*r)->readbuf);
	buffer_release(&(*r)->binbuf);
	//free(*r);
	LIST_PUSH_BACK(g_rpacket_pool,*r);
	*r = 0;
}

uint32_t  rpacket_len(rpacket_t r)
{
	return r->len;
}

uint32_t  rpacket_data_remain(rpacket_t r)
{
	return r->data_remain;
}

static int rpacket_read(rpacket_t r,int8_t *out,uint32_t size)
{
	buffer_t _next = 0;
	if(r->data_remain < size)
		return -1;
	while(size>0)
	{
		uint32_t copy_size = r->readbuf->size - r->rpos;
		copy_size = copy_size >= size ? size:copy_size;
		memcpy(out,r->readbuf->buf + r->rpos,copy_size);
		size -= copy_size;
		r->rpos += copy_size;
		r->data_remain -= copy_size;
		out += copy_size;
		if(r->rpos >= r->readbuf->size && r->data_remain)
		{
			//��ǰbuffer�����Ѿ�������,�л�����һ��buffer
			r->rpos = 0;
			r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
		}
	}
	return 0;
}

uint8_t rpacket_read_uint8(rpacket_t r)
{
	uint8_t value = 0;
	rpacket_read(r,(int8_t*)&value,sizeof(value));
	return value;
}

uint16_t rpacket_read_uint16(rpacket_t r)
{
	uint16_t value = 0;
	rpacket_read(r,(int8_t*)&value,sizeof(value));
	return value;
}

uint32_t rpacket_read_uint32(rpacket_t r)
{
	uint32_t value = 0;
	rpacket_read(r,(int8_t*)&value,sizeof(value));
	return value;
}

uint64_t rpacket_read_uint64(rpacket_t r)
{
	uint64_t value = 0;
	rpacket_read(r,(int8_t*)&value,sizeof(value));
	return value;
}

double   rpacket_read_double(rpacket_t r)
{
	double value = 0;
	rpacket_read(r,(int8_t*)&value,sizeof(value));
	return value;
}

const char* rpacket_read_string(rpacket_t r)
{
	uint32_t len = 0;
	if(r->raw)//raw���͵�rpacket��֧�ֶ�ȡ�ַ���
		return 0;
	return (const char *)rpacket_read_binary(r,&len);
}

static const void* rpacket_raw_read_binary(rpacket_t r,uint32_t *len)
{
	void *addr = 0;
	uint32_t size = 0;
	if(!r->data_remain)
		return addr;
	if(r->readbuf->size - r->rpos >= r->data_remain)
	{
		*len = r->data_remain;
		r->data_remain = 0;
		addr = &r->readbuf->buf[r->rpos];
		r->rpos += r->data_remain;
	}
	else
	{
		if(!r->binbuf)
		{
			r->binbufpos = 0;
			r->binbuf = buffer_create_and_acquire(0,r->len);
		}
		addr = r->binbuf->buf + r->binbufpos;
		size = r->data_remain;
		while(size)
		{
			uint32_t copy_size = r->readbuf->size - r->rpos;
			copy_size = copy_size >= size ? size:copy_size;
			memcpy(r->binbuf->buf + r->binbufpos,r->readbuf->buf + r->rpos,copy_size);
			size -= copy_size;
			r->rpos += copy_size;
			r->data_remain -= copy_size;
			r->binbufpos += copy_size;		
			if(r->rpos >= r->readbuf->size && r->data_remain)
			{
				//��ǰbuffer�����Ѿ�������,�л�����һ��buffer
				r->rpos = 0;
				r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
			}
		}
	}
	return addr;
}

const void* rpacket_read_binary(rpacket_t r,uint32_t *len)
{
	void *addr = 0;
	uint32_t size = 0;
	if(r->raw)
		return rpacket_raw_read_binary(r,len);
	size = rpacket_read_uint32(r);
	*len = size;
	if(!r->data_remain || r->data_remain < size)
		return addr;
	if(r->readbuf->size - r->rpos >= size)
	{
		addr = &r->readbuf->buf[r->rpos];
		r->rpos += size;
		r->data_remain -= size;
		if(r->rpos >= r->readbuf->size && r->data_remain)
		{
			//��ǰbuffer�����Ѿ�������,�л�����һ��buffer
			r->rpos = 0;
			r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
		}
	}
	else
	{
		//���ݿ�Խ��buffer�߽�,����binbuf,�����ݿ�����binbuf��
		if(!r->binbuf)
		{
			r->binbufpos = 0;
			r->binbuf = buffer_create_and_acquire(0,r->len);
		}
		addr = r->binbuf->buf + r->binbufpos;
		while(size)
		{
			uint32_t copy_size = r->readbuf->size - r->rpos;
			copy_size = copy_size >= size ? size:copy_size;
			memcpy(r->binbuf->buf + r->binbufpos,r->readbuf->buf + r->rpos,copy_size);
			size -= copy_size;
			r->rpos += copy_size;
			r->data_remain -= copy_size;
			r->binbufpos += copy_size;		
			if(r->rpos >= r->readbuf->size && r->data_remain)
			{
				//��ǰbuffer�����Ѿ�������,�л�����һ��buffer
				r->rpos = 0;
				r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
			}
		}

	}
	return addr;
}


