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

#define LIST_PUSH_FRONT(L,N) list_push_front(L,(list_node*)N)

#define LIST_PUSH_BACK(L,N) list_push_back(L,(list_node*)N)

#define LIST_POP(T,L) (T)list_pop(L)

#define LIST_IS_EMPTY(L) list_is_empty(L)

#define LIST_CREATE() create_list()

#define LIST_DESTROY(L) destroy_list(L)

#define LIST_CLEAR(L) list_clear(L)

#endif
