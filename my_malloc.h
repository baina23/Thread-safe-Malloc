#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>


typedef struct metadata{
    size_t size;
    struct metadata *next;
    struct metadata *pre;
}meta_t;

pthread_mutex_t lock;
int LOCK_V = 0; // lock version = 0; non-lock version = 1;
__thread meta_t *head_tls = NULL;
__thread meta_t *tail_tls = NULL;
meta_t *_head = NULL;
meta_t *_tail = NULL;

void init(meta_t **head, meta_t **tail);
bool checkmerge(meta_t* cur, meta_t **head, meta_t **tail);

//Best Fit malloc/free
void *bf_malloc(size_t size, meta_t **head, meta_t **tail);
void bf_free(void *ptr, meta_t **head, meta_t **tail);

//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
