#include "stdio.h"
#include "stdlib.h"
#include "hash.h"
#include "counter.h"
#include "pthread.h"
 
#include "sys/times.h"
 
#define MAXTHREADS 1000 
int main()
{
	
   int element = 255;
    unsigned int key = 127;
    hash_t hs;
    Hash_Init(&hs, 32);

    Hash_Insert(&hs, (void*)&element, key);
    void *val = Hash_Lookup(&hs, key);
	//printf("%d\n",*(int*)val);
    if (val)
	return 0;
    else
	{
		printf("No val returned\n");
		return -1;
	}
}

