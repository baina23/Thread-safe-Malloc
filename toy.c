#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define META_SIZE 4*sizeof(void*)

struct node{
    int val;
    struct node *next;
};


void* find(size_t size){
    return 0;
}

int main(){
    size_t blk_size = 2;
    size_t *p =(size_t*)sbrk(blk_size);

    printf("size_t is %ld\n", blk_size);
    printf("current address is %p\n", p);
    p = sbrk(sizeof(void*));
    
    //struct node *t = malloc(sizeof(struct node)); 
    void *h = sbrk(0);   
    size_t addr = 0;
    *(void**)& *p = h;
    //*p = addr; 
    void *a = (void*)*(p+2);
    printf("current address is (p) %p\n", p);
    printf("current address is (h) %p\n", h);
    printf("saved address is (*p) %p\n", a);
    printf("*p = %lx\n",*p);
    printf("META_SIZE=%ld\n", META_SIZE);



    void* start = h;
    intptr_t g = (intptr_t) start;
    *(intptr_t*)h = g;
    printf("start address = %p\n", start);
    printf("head = intptr_t %lx\n", *(intptr_t*)h);



    char *s ="sdfgh";
    void *pp;
    pp = s;
    s = pp;
    printf("pp = %p\n", pp);
    pp = (int*)pp + 1;
    printf("pp + (int*)1 = %p\n", pp);
    pp = pp + 1;
    printf("pp + (void*)1 = %p\n", pp);
    assert(find(5) == NULL );
    pp = sbrk(3*sizeof(int));
    int size;
    int *num = pp;
    *num = 10;
    void *kk = num + 1;
    char *c = kk - sizeof(void*); 
    *c = '0';
    printf("num = %d\n",*num);
    printf("num = %p\n",num);
    printf("kk = num + 1 = %p\n", kk);
    printf("c address = %p, c = %c\n",c,*c);
    

    int aa , bb;
    int* head = &aa;
    int* tail = &bb;
    *head = 13;
    *tail = 18;
    unsigned long mean = (unsigned long)((void*)tail-(void*)head);
    printf("aa address = %p, bb address = %p\n", head, tail);
    printf(" pointers' mean = %ld\n", mean);

    printf("sizeof(intptr_t) = %ld\n", sizeof(intptr_t));
    
    

    return 0;
}


