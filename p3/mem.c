#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "mem.h"

#define PADDING 0xABCDDCBA
#define FREESPACE 0xDEADBEEF
#define PADDINGSIZE 64

int m_error;

//double linked list
typedef struct memNode
{
	unsigned int size;
	struct memNode* next;
	struct memNode* prev;
}memNode;

typedef struct allocHeader
{
	unsigned int size;
	unsigned long long gap;
	unsigned int redundancy;
}allocHeader;

memNode* freeList = NULL;
int debugMode = 0;
void * startMem = NULL;
int sizeMem=-1; 
void removeMemNode(memNode* node)
{
	if(node == NULL)
		return;
	
	if(node->next ==NULL && node->prev == NULL)
	{
		freeList = NULL;
		return;
	}
	
	if(node == freeList)
	{
		
		freeList = node->next;
		freeList->prev = NULL;		
		return;
	}
	
	if(node->next != NULL)
		node->next->prev = node->prev;
	
	if(node->prev != NULL)
	{
		node->prev->next = node->next;
	}
	
}

void fillMemWithPattern(int* ptr, int length, int pattern)
{
	while(length>=4)
	{
		length = length - 4;
		*ptr = pattern;
		ptr++; 
	}
}
int Mem_Init(int sizeOfRegion, int debug)
{
	
	if(sizeOfRegion <= 0 || freeList != NULL || sizeMem>=0 || (debug!=0 && debug!=1))
	{
		m_error = E_BAD_ARGS;
		return -1;
	}
	int fd = open("/dev/zero", O_RDWR);
	if(sizeOfRegion%getpagesize())
		sizeOfRegion = sizeOfRegion + getpagesize()-(sizeOfRegion%getpagesize());
	void* ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	sizeMem = sizeOfRegion;
	startMem = ptr;
	if(ptr == MAP_FAILED)
	{
		perror("mmap");
		return -1;
	}
	
	freeList = ptr;
	freeList->size = sizeOfRegion - sizeof(memNode);
	freeList->next = NULL;
	freeList->prev = NULL;
	debugMode = debug;
	if(debugMode)
	{
		fillMemWithPattern((void*)ptr+sizeof(memNode), freeList->size,FREESPACE);
	}
	close(fd);
	return 0;
}

void* Mem_Alloc(int size)
{
	if(size <= 0) 
		return NULL;

	if(!freeList)
	{
		//printf("freelist is empty\n");
		m_error = E_NO_SPACE;
		return NULL;
	}
	size_t memNodeSize = sizeof(memNode);
	
	size_t allocHeaderSize = sizeof(allocHeader);
	
	if(size%4)
	{
		size = size + 4 - (size%4);
	}
	
	if(debugMode)
		size = size + PADDINGSIZE *2;
	size_t sizeplusheader = size + allocHeaderSize;
	//first-fit
	memNode * firstFit = NULL;
	memNode * temp = freeList;
	
	while(temp)
	{
		
		if(debugMode)
		{
			size_t len = temp->size;
			unsigned int* tempptr = (void*)temp+memNodeSize;
			while(len>4)
			{
				len = len - 4;
				if(*tempptr != FREESPACE)
				{
					m_error = E_CORRUPT_FREESPACE;
					//printf("Corrupt Address: %p\t temp->size: %d len: %d\n",(void*)tempptr-(void*)freeList, temp->size, len);
					return NULL;
				}
				++tempptr;
			}
		}
		
		if(temp->size >= size)//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  sizeplusheader
		{
			firstFit = temp;
			//	printf("size node: %d\n",firstFit->size);
			break;
		}		
		temp = temp->next;
	}
	
	if(firstFit == NULL)
	{
		m_error = E_NO_SPACE;
		//printf("first fit not found for size: %d\n", size);
		return NULL;
	}

	if(firstFit->size == size)
	{
		removeMemNode(firstFit);
	}
	else
	{
		//printf("size plus header: %d\n",sizeplusheader);	
		//create a new node
		memNode* newNode = (memNode*)((void*)firstFit + sizeplusheader);
		//update free list
		
		newNode->size = firstFit->size - sizeplusheader; 
		
		newNode->next = firstFit->next;
		//printf("size newNode: %d\n",newNode->size);
		newNode->prev = firstFit;		
		firstFit->next = newNode;		
		if(newNode->next!=NULL)
		{						
			newNode->next->prev = newNode;
		}
		removeMemNode(firstFit);
		
				
	}
	
	if(debugMode)
	{
		//add padding and return
		allocHeader* ff = (allocHeader*)firstFit;
		ff->size = size -PADDINGSIZE*2; 
		ff->redundancy = 0xAAAAAAAA;
		fillMemWithPattern((void*)firstFit+allocHeaderSize, PADDINGSIZE,PADDING);
		fillMemWithPattern((void*)firstFit+sizeplusheader-PADDINGSIZE, PADDINGSIZE,PADDING);
		return (void*)firstFit + allocHeaderSize + PADDINGSIZE;
	}
	
	allocHeader* ff = (allocHeader*)firstFit;
	ff->size = size; 
	ff->redundancy = 0xAAAAAAAA;
	return (void*)firstFit + allocHeaderSize;
	
}

int Mem_Free(void* ptr)
{
	if(ptr == NULL)
		return 0;
	size_t memNodeSize = sizeof(memNode);
	size_t allocHeaderSize = sizeof(allocHeader);
	memNode* toFree;
	
	void* start;
	if(debugMode)
	{
		start = ptr - allocHeaderSize - PADDINGSIZE;
	
	}
	else
		start  = (ptr - allocHeaderSize);

	allocHeader* alloc  = (allocHeader*) start;
	if(start< startMem || start >= startMem+sizeMem)
	{
		m_error = E_BAD_POINTER;
		//printf("out of address space\n");
		return -1;	
	}
	
	if(alloc->redundancy!= 0xAAAAAAAA)
	{
		m_error = E_BAD_POINTER;
		//printf("redundancy did not match\n");
		return -1;
	}
	//check if padding was overwritten
	int tempPadding = 60; 
	while(tempPadding>=0 && debugMode)
	{
		if(*(int*)(start+allocHeaderSize+tempPadding) != PADDING || *(int*)(start+allocHeaderSize+alloc->size+PADDINGSIZE+tempPadding) !=PADDING)
		{
			m_error = E_PADDING_OVERWRITTEN;
			//printf("Padding overwritten\n");
			return -1;
		}
		tempPadding-=4;
	}
	size_t sizeOfChunk = alloc->size;
		
	toFree = start;
	if(debugMode)
		toFree->size = sizeOfChunk + allocHeaderSize + 2*PADDINGSIZE -memNodeSize;
	else
		toFree->size = sizeOfChunk + allocHeaderSize - memNodeSize;  

	toFree->next = NULL;
	toFree->prev = NULL;
/*
	if(debugMode)
	{
		fillMemWithPattern((void*)toFree + memNodeSize, toFree->size,FREESPACE);
	} */
	//printf("toFreeSize %d\n",toFree->size);
	if(freeList ==NULL)
	{	
		freeList = toFree;
		if(debugMode)
		{
			fillMemWithPattern((void*)toFree + memNodeSize, toFree->size,FREESPACE);
		}
		return 0;
	}
	
	//Before first node
	if((void*)toFree<(void*)freeList)
	{
		int diff = (void*)toFree + toFree->size + memNodeSize - (void*)freeList;
		
		if(diff > 0)
		{
			m_error = E_BAD_POINTER;
			//printf("aloha\n");
			return -1;
		}
		else if(diff<0)
		{
			
			toFree ->next = freeList;
			freeList->prev = toFree;
			freeList = toFree;
			if(debugMode)
			{
				fillMemWithPattern((void*)toFree + memNodeSize, toFree->size,FREESPACE);
			}
			
		}
		else if(diff ==0)//contiguous with first node
		{
			
			
			int lengthToFree = toFree->size + memNodeSize;	
			toFree->size = toFree->size + freeList->size + memNodeSize;
			toFree->next = freeList->next;
			if(freeList->next)
				freeList->next->prev = toFree;
			freeList = toFree;				
			if(debugMode)
			{
				fillMemWithPattern((void*)toFree + memNodeSize, lengthToFree,FREESPACE);
			}
		}
		
	}
	else
	{
		memNode* fit = freeList;
		while((fit->next) && (void*)fit <= (void*)toFree)
		{
			fit = fit -> next;
		}
		//after last node
		if(fit->next == NULL && (void*)fit <= (void*)toFree)
		{
			//place toFree after the fit
			int diff = (void*) toFree - fit->size -memNodeSize -(void*) fit;
			if(diff==0)//contiguous with the last node
			{
				fit->size = fit->size + toFree->size + memNodeSize;
				if(debugMode)
				{
					fillMemWithPattern((void*)toFree, toFree->size + memNodeSize,FREESPACE);
				}
			}
			else
			{
				fit->next = toFree;
				toFree->prev = fit;
				if(debugMode)
				{
					fillMemWithPattern((void*)toFree + memNodeSize, toFree->size ,FREESPACE);
				}
			}
		}
		else//between fit and fit->prev
		{
			int diff1 = (void*)toFree - ((void*) fit->prev + fit->prev->size + memNodeSize);
			int diff2 = (void*) toFree + toFree->size + memNodeSize - (void*)fit;
			//printf("diff1 is: %d\n",diff1);
			//printf("diff2 is: %d\n",diff2);
			if( diff1 == 0 || diff2 == 0)
			{
				if(diff1==0 && diff2==0)
				{
					fit->prev->size = fit->prev->size + toFree->size + fit->size + 2* memNodeSize;
					fit->prev->next = fit->next;
					if(fit->next)
						fit->next->prev = fit->prev;
					if(debugMode)
					{
						fillMemWithPattern((void*)toFree, toFree->size + 2*memNodeSize,FREESPACE);
					}
				}
				else if(diff1==0)
				{
					fit->prev->size = fit->prev->size + toFree->size+ memNodeSize;
					if(debugMode)
					{
						fillMemWithPattern((void*)toFree, toFree->size + memNodeSize,FREESPACE);
					}
				}
				else
				{					
					int lenghtToFree = toFree->size +memNodeSize;
					toFree->size = toFree->size + fit->size + memNodeSize;					
					toFree->next = fit->next;
					toFree -> prev = fit-> prev;
					if(fit->next)
						fit->next->prev = toFree;
					if(fit -> prev)					
						fit->prev->next = toFree;
					if(debugMode)
					{
						fillMemWithPattern((void*)toFree + memNodeSize, lenghtToFree,FREESPACE);
					}										
				}
			}
			else
			{
				//printf("dsgsdgasdgZie is: %d\n", toFree->size);
				toFree->prev = fit->prev;
				toFree->next = fit;
				toFree->prev->next = toFree;
				fit->prev = toFree;
				if(debugMode)
				{
					fillMemWithPattern((void*)toFree + memNodeSize, toFree->size,FREESPACE);
				}
			}
		}
	}	
	return 0;
}

void Mem_Dump()
{
	printf("\nMemory Dump:\n");
	memNode* temp = freeList;
	while(temp)
	{
		printf("Start Address: %p\t Available space: %d bytes\n",(void*)temp+sizeof(memNode), temp->size);
		temp = temp->next;
	}
	printf("End of memdump\n");
}
