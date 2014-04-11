#ifndef HASH_H
#define HASH_H
#include "list.h"
typedef struct hash_t
{
	unsigned int numBuckets;
	list_t** buckets;
}hash_t;


void Hash_Init(hash_t *hash, int buckets);
void Hash_Insert(hash_t *hash, void *element, unsigned int key);
void Hash_Delete(hash_t *hash, unsigned int key);
void *Hash_Lookup(hash_t *hash, unsigned int key);
list_t* getBucket(hash_t* hash, unsigned int key);
#endif
