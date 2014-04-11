#ifndef __COUNTER_H
#define __COUNTER_H
#include "stdlib.h"
#include "spin.h"

typedef struct counter_type{
  //TODO: put your extra fields here
  int value;
}counter_t;

void Counter_Init(counter_t *c, int value);
int Counter_GetValue(counter_t *c);
void Counter_Increment(counter_t *c);
void Counter_Decrement(counter_t *c);
#endif
