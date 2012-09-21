#include <stdlib.h>
#include <stdio.h>
#include "link_list.h"

typedef struct link_list
{
	int32_t size;
	list_node *head;
	list_node *tail;
	
}link_list;

link_list *create_list()
{
	link_list *l = malloc(sizeof(link_list));
	l->size = 0;
	l->head = l->tail = 0;
	return l;
}

void destroy_list(link_list **l)
{
	free(*l);
	*l = 0;
}

void list_clear(link_list *l)
{
	l->size = 0;
	l->head = l->tail = 0;
}

void list_push_back(link_list *l,list_node *n)
{
	if(n->next)
		return;
	n->next = 0;
	if(0 == l->size)
	{
		l->head = l->tail = n;
	}
	else
	{
		l->tail->next = n;
		l->tail = n;
	}
	++l->size;
}

void list_push_front(link_list *l,list_node *n)
{
	if(n->next)
		return;
	n->next = 0;
	if(0 == l->size)
	{
		l->head = l->tail = n;
	}
	else
	{
		n->next = l->head;
		l->head = n;
	}
	++l->size;

}

list_node* list_head(link_list *l)
{
	return l->head;
}

list_node* list_pop(link_list *l)
{
	struct list_node *ret = 0;
	if(0 == l->size)
		return 0;
	ret = l->head;
	l->head = l->head->next;
	if(0 == l->head)
		l->tail = 0;
	--l->size;
	ret->next = 0;
	return ret;
}

int32_t list_is_empty(link_list *l)
{
	return l->size == 0;
}
int32_t list_size(link_list *l)
{
	return l->size;
}
