#include "my_malloc.h"


meta_t *head = NULL;
meta_t *tail = NULL;

void init(){
    head = (meta_t*)sbrk(sizeof(meta_t));
    tail = (meta_t*)sbrk(sizeof(meta_t));
    head->size = -1;
    head->next = tail;
    head->pre = NULL;
    tail->size = -1;
    tail->pre = head;
    tail->next = NULL;
    return;
}

bool checkmerge(meta_t *cur){
    meta_t *tmp = head;
    meta_t *follow = NULL;
    meta_t *ahead = NULL;
    int state = 0;
    void *cur_next = (void*)cur + cur->size + sizeof(meta_t);
    while(tmp != tail){
        if(cur_next == (void*)tmp && follow == NULL) {
            state++;
            follow = tmp;
        }
        void *tmp_next = (void*)tmp + tmp->size + sizeof(meta_t);
        if(tmp_next == (void*)cur && ahead == NULL) {
            state++;
            ahead = tmp;
        }
        if(state == 2) break;
        tmp = tmp->next;
    }

    switch (state)
    {
    case 0: return false;
    case 1:{
        if(follow != NULL){
            cur->size = follow->size + cur->size + sizeof(meta_t);
            cur->pre = follow->pre;
            cur->next = follow->next;
            follow->pre->next = cur;
            follow->next->pre = cur;
        }
        if(ahead != NULL){
            ahead->size = ahead->size + cur->size + sizeof(meta_t);
        }
        return true;
    }
    case 2:{
        ahead->size = ahead->size + cur->size + follow->size + 2*sizeof(meta_t);
        follow->pre->next = follow->next;
        follow->next->pre = follow->pre;
        return true;
    }   
    default:
        return false;
    }
}


//Best Fit malloc/free
void *bf_malloc(size_t size){
        void *ret;
    if(head == NULL)
        init(); 
    else{
        meta_t *tmp = head->next;
        meta_t *best = NULL;
        size_t min = size;
        while (tmp != tail)
        {
            if(tmp->size >= size && tmp->size <= size + sizeof(meta_t)){
                ret = (void*)tmp + sizeof(meta_t);
                tmp->pre->next = tmp->next;
                tmp->next->pre = tmp->pre;    
                tmp->next = NULL;
                tmp->pre = NULL;            
                return ret;
            }
            
            if(tmp->size > size + sizeof(meta_t)){
                size_t d = tmp->size - size - sizeof(meta_t);
                if(d < min){
                    min = d;
                    best = tmp;
                }
            }
            tmp = tmp->next;
        }

            if(best != NULL){
                ret = (void*)best + sizeof(meta_t);
                meta_t *new = (meta_t*)((void*)tmp + size + sizeof(meta_t));
                new->size = tmp->size - size;
                new->next = tmp->next;
                new->pre = tmp->pre;
                new->pre->next = new;
                best->size = size;
                best->next = NULL;
                best->pre = NULL;
                return ret;
            }
            
        } 

    meta_t *cur = (meta_t*)sbrk(size+sizeof(meta_t));
    cur->size = size;
    cur->next = NULL;
    cur->pre = NULL;
    ret = (void*)cur + sizeof(meta_t);
    return ret;
}

void bf_free(void *ptr){
    assert(head != NULL);
    meta_t *cur = (meta_t*)(ptr - sizeof(meta_t));
    if(checkmerge(cur)) return;
    cur->next = tail;
    cur->pre = tail->pre;
    tail->pre = cur;
    return;
}

//performance study
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes