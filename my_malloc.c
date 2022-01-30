#include "my_malloc.h"

void* lock_sbrk(size_t size){
    assert(pthread_mutex_lock(&lock) == 0);
    void *ret = sbrk(size);
    assert(pthread_mutex_unlock(&lock) == 0);
    return ret;
}

void init(meta_t **head, meta_t **tail){
    if(!LOCK_V){
        *head = (meta_t*)sbrk(sizeof(meta_t));
        *tail = (meta_t*)sbrk(sizeof(meta_t));
    }
    else{
        *head = (meta_t*)lock_sbrk(sizeof(meta_t));
        *tail = (meta_t*)lock_sbrk(sizeof(meta_t));
    }
    (*head)->size = -1;
    (*head)->next = *tail;
    (*head)->pre = NULL;
    (*tail)->size = -1;
    (*tail)->pre = *head;
    (*tail)->next = NULL;
    return;
}

bool checkmerge(meta_t *cur, meta_t **head, meta_t **tail){
    meta_t *tmp = (*head)->next;
    meta_t *follow = NULL;
    meta_t *ahead = NULL;
    int state = 0;
    void *cur_next = (void*)cur + cur->size + sizeof(meta_t);
    while(tmp != *tail){
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
            cur->pre->next = cur;
            cur->next->pre = cur;
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
void* bf_malloc(size_t size, meta_t **head, meta_t **tail){
    void *ret;
    if(*head == NULL)
        init(head, tail); 
    else{
        meta_t *tmp = (*head)->next;
        meta_t *best = NULL;
        size_t min = size;
        while (tmp != *tail)
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

    meta_t *cur = NULL;
    if(!LOCK_V){
        void* p = sbrk(0);
        cur = (meta_t*)sbrk(size+sizeof(meta_t));
        cur->size = size;
        cur->next = NULL;
        cur->pre = NULL;
        ret = (void*)cur + sizeof(meta_t);
    }
        
    else{
        cur = (meta_t*)lock_sbrk(size+sizeof(meta_t));
        cur->size = size;
        cur->next = NULL;
        cur->pre = NULL;
        ret = (void*)cur + sizeof(meta_t);
    }
        
    // cur->size = size;
    // cur->next = NULL;
    // cur->pre = NULL;
    // ret = (void*)cur + sizeof(meta_t);
    return ret;
}

void bf_free(void *ptr, meta_t **head, meta_t **tail){
    assert(*head != NULL);
    meta_t *cur = (meta_t*)(ptr - sizeof(meta_t));
    if(checkmerge(cur,head,tail)) return;
    cur->next = *tail;
    cur->pre = (*tail)->pre;
    (*tail)->pre = cur;
    return;
}

//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size){
    void *ret;
    LOCK_V = 0;
    assert(pthread_mutex_lock(&lock) == 0);
    ret = bf_malloc(size, &_head, &_tail);
    assert(pthread_mutex_unlock(&lock) == 0);
    return ret;
}
void ts_free_lock(void *ptr){
    LOCK_V = 0;
    assert(pthread_mutex_lock(&lock) == 0);
    bf_free(ptr, &_head, &_tail);
    assert(pthread_mutex_unlock(&lock) == 0);
    return;
};

//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size){
    LOCK_V = 1; 
    return bf_malloc(size, &head_tls, &tail_tls);
};
void ts_free_nolock(void *ptr){
    LOCK_V = 1;
    if(head_tls == NULL) init(&head_tls, &tail_tls);
    bf_free(ptr, &head_tls, &tail_tls);
    return;
};
