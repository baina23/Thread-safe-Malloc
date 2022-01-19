#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define META_SIZE (sizeof(size_t)+sizeof(char)+2*sizeof(intptr_t))

#define FLAG_OFFSET sizeof(size_t)
#define NEXT_OFFSET (META_SIZE - 2*sizeof(intptr_t))
#define PRE_OFFSET  (META_SIZE - sizeof(intptr_t))


void* allocatehead(size_t size);
void* findfirstfit(size_t size);
void* findbestfit(size_t size);
void* allocatenew(size_t size);

void sliceblk(void* ptr, size_t size);

void mergelist(void* ptr);

//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//performance study
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes













