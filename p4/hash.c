#include "hash.h"

void Hash_Init(hash_t *hash, int buckets)
{
	if(hash)
	{
		buckets=buckets>0?buckets:1;
		hash->numBuckets = buckets;
		hash->buckets = malloc(buckets* sizeof(list_t*));
		int i;
		for(i =0; i < buckets; ++i)
		{
			hash->buckets[i]=malloc(sizeof(list_t));
			List_Init(hash->buckets[i]);// = NULL;
		}
	}
}

void Hash_Insert(hash_t *hash, void *element, unsigned int key)
{
	if(hash)
	{
		List_Insert(getBucket(hash,key), element, key);		
	}

}

void Hash_Delete(hash_t *hash, unsigned int key)
{
	if(hash)
	{
		List_Delete(getBucket(hash,key), key);
	}
}

void *Hash_Lookup(hash_t *hash, unsigned int key)
{
	if(hash)
	{
		return  List_Lookup(getBucket(hash,key),key);
	}
	return NULL;
}

list_t* getBucket(hash_t * hash, unsigned int key)
{
	
	return hash->buckets[key%(hash->numBuckets)];
}
