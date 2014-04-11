#include "spin.h"
#include "stdio.h"
#include "stdlib.h"

static inline uint
xchg(volatile unsigned int *addr, unsigned int newval)
{
      uint result;
          asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
	      //xchg: exchange the two source operands
	      //two outputs: *addr and result
	      //"+m", m means the operand is directly from memory, + means read and write
	      //"=a": suggest gcc to put the operand value into eax regi  ster; `=' means write only
	      //one input: newval, "1" means it uses the same constraint as the earlier 1th, i.e., it 
	      // will be put into eax and then be overwritten
	      //"cc" means the condition register might be altered
	      return result;
}

int spinlock_init(spinlock_t *lock)
{
  xchg(&lock->state, 0);
  return 0;
}

int spinlock_acquire(spinlock_t* lock)
{

  while (xchg(&lock->state, 1) == 1)
    ; // spin
  return 0;
}

int spinlock_release(spinlock_t* lock)
{
  xchg(&lock->state, 0);
  return 0;
}
