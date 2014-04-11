#include "counter.h"
#include "assert.h"

spinlock_t slock;

void Counter_Init(counter_t *c, int value)
{
  spinlock_init(&slock);
  c->value = value;
  return;
}

int Counter_GetValue(counter_t *c)
{
  return c->value;
}

void Counter_Increment(counter_t *c)
{
  spinlock_acquire(&slock);
  c->value += 1;
  spinlock_release(&slock);
}

void Counter_Decrement(counter_t *c)
{
  spinlock_acquire(&slock);
  c->value -= 1;
  spinlock_release(&slock);
}
