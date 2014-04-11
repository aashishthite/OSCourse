#include "list.h"
/*
 struct 
{
	volatile int state;
}spinlock_t;

void spinlock_acquire(spinlock_t *lock)
{

}

void spinlock_release(spinlock_t *lock);
{

}
*/
void List_Init(list_t *list)
{
	if(list)
	{
		
		spinlock_acquire(&list->lock);
		list->head = NULL;
		spinlock_release(&list->lock);
	}
}

void List_Insert(list_t *list, void *element, unsigned int key)
{
	if(list)
	{	
	
		listItem * newItem = malloc(sizeof(listItem));
		newItem->key = key;
		newItem->element = element;
		newItem->next = NULL;
		spinlock_acquire(&list->lock);
		newItem->next = list->head;
		list->head = newItem;
		
		spinlock_release(&list->lock);
	}
}

void List_Delete(list_t *list, unsigned int key)
{
	if(list)
	{
		spinlock_acquire(&list->lock);
		listItem* temp = list->head;
		
		if(!temp)
		{
			spinlock_release(&list->lock);
			 return;
		}
		if(temp && temp->key == key)
		{
			list->head = temp->next;
			free(temp);
			spinlock_release(&list->lock);
			return;
		}
		while(temp->next)
		{
			if(temp->next->key == key)
			{
				listItem * toFree = temp->next;
				temp->next = toFree->next;	
				spinlock_release(&list->lock);
				free(toFree);
				return;
			}
			temp = temp -> next;
		}
		spinlock_release(&list->lock);
	}
}

void *List_Lookup(list_t *list, unsigned int key)
{
	if(list)
	{
		listItem * temp;
		spinlock_acquire(&list->lock);
		temp = list->head;
		while(temp)
		{
			if(temp->key == key)
			{
				spinlock_release(&list->lock);
				return temp->element;
			}
			temp = temp->next;
		}
		spinlock_release(&list->lock);	
	}
	return NULL;
}
