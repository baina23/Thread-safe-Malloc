#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void* allocatehead(size_t size);
void* findfirstfit(size_t size);
void* findbestfit(size_t size);
void* allocatenew(size_t size);

void mergelist(void* ptr, int dir);

//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//performance study
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes













