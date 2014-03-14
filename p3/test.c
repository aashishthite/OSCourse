/* call Mem_Init with size = 1 page */
#include "mem.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "common.h"
int main() {
int err, i, j;
    int pagesize=0;
    char *p[8192];
    int num_p;
    struct timeval start;
    struct timeval end;
    double usec=0;
    
    err = Mem_Init(36*4096, 0);
    if(err == -1)
        return -1;

    gettimeofday(&start, NULL);
    for(j=0; j<20; j++) {
    for(i=0; i<4096; i++) {
        p[i] = Mem_Alloc(12);
        if(p[i] == NULL)
             return -1;
    }
    for(i=0; i<4096; i++) {
        err = Mem_Free(p[4095-i]);
        if(err == -1)
             return -1;
    }
    }


    for(j=0; j<20; j++) { 
    num_p = 36 * 4096 / 128;
	//printf("num_p is : %d\n", num_p);
    for(i=0; i<num_p; i++) {
        p[i] = Mem_Alloc(104);
        if(p[i] == NULL)
             return -1;
    }

    for(i=0; i<num_p; i+=4) {
        err = Mem_Free(p[i]);
        if(err == -1)
             return -1;
    }
		
    for(i=3; i<num_p; i+=4) {
        err = Mem_Free(p[i]);
        if(err == -1)
             return -1;
    }
//Mem_Dump();
    for(i=1; i<num_p; i+=4) {
        err = Mem_Free(p[i]);
        if(err == -1)
             return -1;
    }
	//Mem_Dump();
    for(i=0; i<num_p; i+=4) {
        p[i] = Mem_Alloc(232); 
	if(p[i] == NULL)
	{		
             return -1;
	}
    }
    for(i=2; i<num_p; i+=4) {
        err = Mem_Free(p[i]);
        if(err == -1)
             return -1;
    }
    for(i=0; i<num_p; i+=4) {
        err = Mem_Free(p[i]);
        if(err == -1)
             return -1;
    }
    }

    gettimeofday(&end, NULL);

    usec =  (end.tv_sec-start.tv_sec)*1000000.0 + (end.tv_usec-start.tv_usec);

    printf("Time taken -> %0.2lf us\n", usec);

    return 0;
    

printf("hello\n");
    return 0;	
  
}
