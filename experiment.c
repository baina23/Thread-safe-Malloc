#include "my_malloc.h"

int main(){
    meta_t *new = (meta_t*)sbrk(256);
    new->size = 10;
    new->pre = NULL;
    //void *t = ;
    meta_t *new2 = (meta_t*) ((void*)new + sizeof(meta_t));
    new->next = new2;
    new2->size = 5;
    new2->pre = new;
    void *tail = sbrk(0);
    printf("sizeof meta_t=%ld\n", sizeof(meta_t));
    printf("%p, %p, %p, %ld, %ld\n", new, new2, tail,new->size, new2->size);
    return EXIT_SUCCESS;
}