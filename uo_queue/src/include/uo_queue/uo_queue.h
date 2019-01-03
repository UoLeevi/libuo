#ifndef UO_QUEUE_H
#define UO_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct uo_queue 
{
    void **items;
    atomic_size_t head;
    atomic_size_t tail;
    size_t capasity;
    void *enqueue_sem;
    void *dequeue_sem;
} uo_queue;

uo_queue *uo_queue_create(
    size_t capasity);

void uo_queue_destroy(
    uo_queue *);

bool uo_queue_enqueue(
    uo_queue *,
    void *item,
    bool should_block);

void *uo_queue_dequeue(
    uo_queue *,
    bool should_block);

#ifdef __cplusplus
}
#endif

#endif