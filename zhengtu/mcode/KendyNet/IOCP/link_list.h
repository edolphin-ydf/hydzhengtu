#ifndef _LINK_LIST_H
#define _LINK_LIST_H
#include <stdint.h>

typedef struct list_node
{
	struct list_node *next;
}list_node;

struct link_list;

void list_push_back(struct link_list*,list_node*);

void list_push_front(struct link_list*,list_node*);

list_node* list_head(struct link_list*);

list_node* list_pop(struct link_list*);

int32_t list_is_empty(struct link_list*);

int32_t list_size(struct link_list*);

struct link_list *create_list();

void destroy_list(struct link_list**);

void list_clear(struct link_list*);

#define LIST_PUSH_FRONT(L,N) list_push_front(L,(list_node*)N)//加到头部

#define LIST_PUSH_BACK(L,N) list_push_back(L,(list_node*)N)  //加到尾部

#define LIST_POP(T,L) (T)list_pop(L)      //推出头项

#define LIST_IS_EMPTY(L) list_is_empty(L) //判断链表是否为空

#define LIST_CREATE() create_list()       //创建链表

#define LIST_DESTROY(L) destroy_list(L)   //销毁链表

#define LIST_CLEAR(L) list_clear(L)       //清空链表

#endif
