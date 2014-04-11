#ifndef LIST_H
#define LIST_H

#include "spin.h"

typedef struct list_t{
	spinlock_t lock;
	struct listItem *head;
}list_t;

//using double linked list
typedef struct listItem
{
	struct listItem *next;
	void* element;
	unsigned int key;
}listItem;

void List_Init(list_t *);
void List_Insert(list_t *, void *, unsigned int);
void List_Delete(list_t *, unsigned int);
void* List_Lookup(list_t *, unsigned int);

#endif
