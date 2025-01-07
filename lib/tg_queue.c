#include <memory.h>
#include "tg_queue.h"

/*
 * 无锁队列，可以支持两个线程分别负责读写，不支持多线程同时读写
 *
*/
queue_t* queue_create(uint8_t cell_size, int size)
{
    if(cell_size > QUEUE_MAX_CELL_SIZE || size > QUEUE_MAX_SIZE){
        log_error("cell size or size of queue is too big\n");
        return NULL;
    }

    queue_t* q = (queue_t*)tg_malloc(sizeof(queue_t));
    if(!q){
        log_error("malloc queue failed\n");
        return NULL;
    }
    q->head = 0;
    q->next = 0;
    //留出一个空间用于分辨队列空和满的情况
    q->max_size = size+1;
    q->cell_size = sizeof(int);
    q->data = (uint8_t*)tg_malloc(q->cell_size*q->max_size);
    if(!q->data){
        log_error("malloc data failed\n");
        return NULL;
    }

    return q;
}

void queue_destroy(queue_t* q)
{
    tg_free(q->data);
    tg_free(q);
}

bool queue_is_empty(queue_t* q)
{
    return q->head == q->next;
}

bool queue_is_full(queue_t* q)
{
    return (q->next+1)%q->max_size == q->head;
}

int queue_push(queue_t* q, void* data)
{
    if(queue_is_full(q)){
        log_error("queue is full\n");
        return TG_ERROR;
    }
    memcpy(q->data+q->next*q->cell_size, data, q->cell_size);
    q->next = (q->next+1)%q->max_size;
    
    return TG_OK;
}

int queue_pop(queue_t* q, void* data)

{
    if(queue_is_empty(q)){
        log_error("queue is empty\n");
        return TG_ERROR;
    }
    memcpy(data, q->data+q->head*q->cell_size, q->cell_size);
    q->head = (q->head+1)%q->max_size;

    return TG_OK;
}

int queue_clear(queue_t* q)
{
    q->head = q->next;
    return TG_OK;
}
