#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "my_malloc.h"

#define META_SIZE (sizeof(size_t)+sizeof(char)+2*sizeof(intptr_t))

#define FLAG_OFFSET sizeof(size_t)
#define NEXT_OFFSET (META_SIZE - 2*sizeof(intptr_t))
#define PRE_OFFSET  (META_SIZE - sizeof(intptr_t))

void *head = NULL;
void *tail = NULL;

//          Metadata struct:
//         | blksize (size_t) |
//         | flag (char)      |
//         | next (void*)     |
//         | pre  (void*)     |

void* allocatehead(size_t size){
    char *flag; // used or unused
    size_t *blksize; // block size
    void *next = NULL;
    void *cur = NULL;

    head = sbrk(size + META_SIZE);
    tail = head;
    next = sbrk(0);

    blksize = head;             // blksize
    *blksize = size;
    
    cur = blksize + 1;         // flag '1' used; '0' available
    flag = cur;
    *flag = '1';
    
    cur = flag + 1;             // next
    intptr_t p = (intptr_t) next;
    *(intptr_t*)cur = p; 

    cur = cur + sizeof(void*);  //pre
    *(intptr_t*)cur = 0;
    cur = cur + sizeof(void*);

    return cur;
}

void* allocatenew(size_t size){
    char *flag; // used or unused
    size_t *blksize; // block size
    void *next = NULL;
    void *pre = tail;
    void *cur = NULL;
    
    cur = sbrk(0); 
    sbrk(size + META_SIZE);
    tail = cur;

    blksize = cur;             // blksize
    *blksize = size;    
    
    cur = blksize + 1;         // flag '1' used; '0' available
    flag = cur;
    *flag = '1';
    
    cur = flag + 1;             // next
    next = sbrk(0);
    intptr_t p = (intptr_t) next;
    *(intptr_t*)cur = p;
    cur = cur + sizeof(void*);  // pre
    p = (intptr_t) pre;
    *(intptr_t*)cur = p;

    cur = cur + sizeof(void*);

    return cur;
}

void sliceblk(void* ptr, size_t size){
    size_t cursize = *(size_t*)ptr;
    assert(cursize >= size);
    if(cursize <= size + META_SIZE) return;
    *(size_t*)ptr = size;
    void* newnode = ptr + META_SIZE + size;
    *(size_t*)newnode = cursize - size - META_SIZE;
    *(char*)(newnode + FLAG_OFFSET) = '0';
    intptr_t p = *(intptr_t*)(ptr + NEXT_OFFSET);
    *(intptr_t*)(newnode + NEXT_OFFSET) = p;
    *(intptr_t*)(ptr + NEXT_OFFSET) = (intptr_t) newnode;
    *(intptr_t*)(newnode + PRE_OFFSET) = (intptr_t) ptr;
    
    if(ptr == tail) {
      tail = newnode;
      return;
    }    
    void* nextnode = (void*) p;
    *(intptr_t*)(nextnode + PRE_OFFSET) = (intptr_t) newnode;
    
    return;  
}

void* findfirstfit(size_t size){

    void* tmp = head;
    while(tmp <= tail){
        char* f = tmp + FLAG_OFFSET;
        if(*f == '1'){
            intptr_t p = *(intptr_t*)(tmp + NEXT_OFFSET); 
            tmp = (void*) p;
            continue;
        }
        size_t val = *(size_t*)tmp;  
    
        if (val >= size){            
            *f = '1';
            sliceblk(tmp,size);
            return tmp + META_SIZE;
        }            
        else {
            intptr_t p = *(intptr_t*)(tmp + NEXT_OFFSET); 
            tmp = (void*) p;  
        }      
    }
    return 0;
}

void* findbestfit(size_t size){

    void* tmp = head;
    size_t mean = 0xffffffffff;
    void* best_META = NULL;
    void* best = NULL;
    while(tmp <= tail){
        char* f = tmp + FLAG_OFFSET;
        if(*f == '1'){
            intptr_t p = *(intptr_t*)(tmp + NEXT_OFFSET); 
            tmp = (void*) p;
            continue;
        }
        size_t val = *(size_t*)tmp;
        if (val >= size && mean > val - size){
            best = tmp + META_SIZE;
            best_META = tmp;
            mean = val - size;
        }
        intptr_t p = *(intptr_t*)(tmp + NEXT_OFFSET); 
        tmp = (void*) p;
    }

    if(best == NULL) return best;
    char* f = best_META + FLAG_OFFSET;
    *f = '1';
    sliceblk(best_META , size);
    return best;    
}


void mergelist (void* ptr){

    void* start = ptr - META_SIZE;
    void* cur = start;

    
    char* f = start + FLAG_OFFSET;         //change current flag to 0

    assert(*f == '1');
    *f = '0';    
    
    intptr_t p = *(intptr_t*)(start+PRE_OFFSET);    // find aheader
    void* aheader = (void*) p;
    f = aheader + FLAG_OFFSET;    
    while(aheader != NULL && *f == '0'){       // neighbor flag = 0 -> can merge
        cur = aheader;
        p = *(intptr_t*)(cur+PRE_OFFSET);
        aheader = (void*) p;
        f = aheader + FLAG_OFFSET;
    }
    aheader = cur;

    cur = start;
    p = *(intptr_t*)(start+NEXT_OFFSET);           // find follower
    void* follower = (void*)p;
    f = follower + FLAG_OFFSET;
    void* tail_pre = (void*)*(intptr_t*)(tail+PRE_OFFSET);
    while(follower <= tail && *f == '0'){
        cur = follower;
        p = *(intptr_t*)(cur+NEXT_OFFSET);
        follower = (void*) p;
        f = follower + FLAG_OFFSET;
    }
    follower = cur;

    if(aheader == follower) return;
    
    *(size_t*)aheader = (size_t)(follower - aheader) + *(size_t*)follower;   // update aheader blksize
    p = *(intptr_t*)(follower+NEXT_OFFSET);
    *(intptr_t*)(aheader+NEXT_OFFSET) = p;       // aheader->next = follower->next
    if(follower == tail)
        tail = aheader;
    else {
        p = *(intptr_t*)(follower+NEXT_OFFSET);  // follower->next->pre = aheader
        cur = (void*) p;
        p = (intptr_t)aheader;
        *(intptr_t*)(cur+PRE_OFFSET) = p;
    } 
    return;
}


//First Fit malloc/free
void *ff_malloc(size_t size){
    if(head == NULL)
        return allocatehead(size);
        
    void *res = findfirstfit(size);    
    if(res == NULL)
        return allocatenew(size);
    else {        
        return res;  
    }
}


void ff_free(void *ptr){
    mergelist(ptr);
    return;
}

//Best Fit malloc/free
void *bf_malloc(size_t size){
    if(head == NULL)
        allocatehead(size);
    void* res = findbestfit(size);
    if(res == NULL)
        return allocatenew(size);
    else
        return res;
}

void bf_free(void *ptr){
    mergelist(ptr);
    return;
}

//performance study
unsigned long get_data_segment_size(){ //in bytes
    return (unsigned long)(sbrk(0) - head);
}

unsigned long get_data_segment_free_space_size(){//in bytes
    void* tmp = head;
    unsigned long res = 0;
    while(tmp < tail){
        if(*(char*)(tmp+FLAG_OFFSET) == '1'){
            intptr_t p = (intptr_t)(tmp + NEXT_OFFSET); 
            tmp = (void*) p;
            continue;
        }
        size_t val = *(size_t*)tmp;
        res = res + val + META_SIZE;        
        intptr_t p = (intptr_t)(tmp + NEXT_OFFSET); 
        tmp = (void*) p;      
    }
    return res;
} 


