#ifndef __SPIN_H
#define __SPIN_H
#include "stdio.h"
#include "stdlib.h"

typedef struct{
 	volatile int state; 
} spinlock_t;

int spinlock_init(spinlock_t *lock);
int spinlock_acquire(spinlock_t *lock);
int spinlock_release(spinlock_t *lock);
#endif
