#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define META_SIZE sizeof(size_t)+sizeof(char)+2*sizeof(intptr_t)
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

    blksize = head;             // blksize
    *blksize = size;
    
    cur = blksize + 1;         // flag '1' used; '0' available
    flag = cur + 1;
    *flag = '1';
    
    cur = flag + 1;             // next
    next = sbrk(0) + 1;
    intptr_t p = (intptr_t) next;
    *(intptr_t*)cur = p; 
    //*(void**)& *cur = next;
    
    cur = cur + sizeof(void*);  //pre
    *(intptr_t*)cur = 0;
    //*(void**)& *cur = NULL;

    cur = cur + sizeof(void*);
    return cur;
}

void* findfirstfit(size_t size){
    //void* tail = sbrk(0);
    void* tmp = head;
    while(tmp < tail){
        if(*(char*)(tmp+sizeof(size_t)) == '1'){
            intptr_t p = (intptr_t)(tmp + META_SIZE - 2*sizeof(void*)); 
            tmp = (void*) p;
            //tmp = (void*)*(tmp + META_SIZE - 2*sizeof(void*));   // next pointer  
            continue;
        }
        size_t val = *(size_t*)tmp;
        if (val >= size)
            return tmp + META_SIZE;
        else {
            intptr_t p = (intptr_t)(tmp + META_SIZE - 2*sizeof(void*)); 
            tmp = (void*) p;
            //tmp = (void*)*(tmp + META_SIZE - 2*sizeof(void*));   // next pointer      
        }      
    }
    return 0;
}

void* findbestfit(size_t size){
    //void* tail = sbrk(0);
    void* tmp = head;
    size_t mean = 0xffffffffff;
    void* best = NULL;
    while(tmp < tail){
        if(*(char*)(tmp+sizeof(size_t)) == '1'){
            intptr_t p = (intptr_t)(tmp + META_SIZE - 2*sizeof(void*)); 
            tmp = (void*) p;
            //tmp = (void*)*(tmp + META_SIZE - 2*sizeof(void*));   // next pointer  
            continue;
        }
        size_t val = *(size_t*)tmp;
        if (val >= size && mean > val - size){
            best = tmp + META_SIZE;
            mean = val - size;
        }
        intptr_t p = (intptr_t)(tmp + META_SIZE - 2*sizeof(void*)); 
        tmp = (void*) p;
        //tmp = (void*)*(tmp + META_SIZE - 2*sizeof(void*));      // next pointer 
    }
    return best;
}


void* allocatenew(size_t size){
    char *flag; // used or unused
    size_t *blksize; // block size
    void *next = NULL;
    void *pre = tail;
    void *cur = NULL;
    
    cur = sbrk(size + META_SIZE);
    tail = cur;

    blksize = cur;             // blksize
    *blksize = size;
    
    cur = blksize + 1;         // flag '1' used; '0' available
    flag = cur + 1;
    *flag = '1';
    
    cur = flag + 1;             // next
    next = sbrk(0) + 1;
    intptr_t p = (intptr_t) next;
    *(intptr_t*)cur = p;
    //*(void**)& *cur = next;
    
    cur = cur + sizeof(void*);  // pre
    p = (intptr_t) pre;
    *(intptr_t*)cur = p;
    //*(void**)& *cur = pre;

    cur = cur + sizeof(void*);
    return cur;
}

void mergelist(void* ptr, int dir){ // dir = 0: merge ahead list; dir = 1: merge followed list
    void* cur = ptr - META_SIZE;
    int OFFSET = dir ? META_SIZE-2*sizeof(void*) : META_SIZE-sizeof(void*); //next or pre
    int _OFFSET = !dir ? META_SIZE-2*sizeof(void*) : META_SIZE-sizeof(void*);
    intptr_t p = *(intptr_t*)(cur+OFFSET);    // neighbor in current dir
    void* neighbor = (void*) p;
    //void* neighbor = (void*)*(cur+OFFSET); 
    p = *(intptr_t*)(cur+_OFFSET);
    void* neighbor2 = (void*) p;            // neighbor in opposite dir
    //void* neighbor2 = (void*)*(cur+_OFFSET); 
    char* f = cur + sizeof(size_t);
    assert(*f == '1');
    *f = '0';

    while(*(neighbor+sizeof(size_t)) == '0'){       // neighbor flag = 0 -> can merge
        *(size_t*)neighbor = *(size_t*)neighbor + *(size_t*)cur + META_SIZE;  //neighbor blksize
        intptr_t p = *(intptr_t*)(cur+_OFFSET);     //neighbor->next(pre) = cur->next(pre)
        *(intptr_t*)(neighbor+_OFFSET) = p;
        //*(void**)& *(neighbor+_OFFSET) = (void*)*(cur+_OFFSET);  
        p = *(intptr_t*)(cur+OFFSET);              
        *(intptr_t*)(neighbor2+OFFSET) = p;
        //*(void**)& *(neighbor2+OFFSET) = (void*)*(cur+OFFSET);                 //neighbor2->pre(next) = cur->pre(next)
        cur = neighbor;
        p = *(intptr_t*)(cur+OFFSET);
        neighbor = (void*) p;
        //neighbor = (void*)*(cur+OFFSET);
        p = *(intptr_t*)(cur+_OFFSET);
        neighbor2 = (void*) p;
        //neighbor2 = (void*)*(cur+_OFFSET);
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
    else return res;  
}


void ff_free(void *ptr){
    mergelist(*ptr, 0); //merge ahead list
    mergelist(*ptr, 1); //merge followed list
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
    mergelist(ptr, 0);
    mergelist(ptr, 1);
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
        if(*(char*)(tmp+sizeof(size_t)) == '1'){
            intptr_t p = (intptr_t)(tmp + META_SIZE - 2*sizeof(void*)); 
            tmp = (void*) p;
            //tmp = (void*)*(tmp + META_SIZE - 2*sizeof(void*));   // next pointer  
            continue;
        }
        size_t val = *(size_t*)tmp;
        res = res + val + META_SIZE;        
        intptr_t p = (intptr_t)(tmp + META_SIZE - 2*sizeof(void*)); 
        tmp = (void*) p;
        //tmp = (void*)*(tmp + META_SIZE - 2*sizeof(void*));   // next pointer            
    }
    return res;
} 



