#ifndef TG_QUEUE_H
#define TG_QUEUE_H

#include <stdbool.h>

#include "common.h"

#define QUEUE_MAX_CELL_SIZE 1024
#define QUEUE_MAX_SIZE 1024

typedef struct{
    uint8_t head;
    uint8_t next;
    uint8_t max_size;
    uint8_t cell_size;
    uint8_t* data;
}queue_t;

queue_t* queue_create(uint8_t cell_size, int size);
void queue_destroy(queue_t *queue);

bool queue_is_empty(queue_t *queue);
bool queue_is_full(queue_t* q);

int queue_push(queue_t* q, void* data);
int queue_pop(queue_t* q, void* data);

int queue_clear(queue_t* q);
#endif