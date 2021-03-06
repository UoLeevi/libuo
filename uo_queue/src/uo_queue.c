#include "uo_queue.h"

#include <string.h>
#include <stdlib.h>

#include <semaphore.h>

uo_queue *uo_queue_create(
    size_t capasity)
{
    uo_queue *queue = calloc(1, sizeof *queue);

    sem_init((sem_t *)&queue->enqueue_sem, 0, capasity);
    sem_init((sem_t *)&queue->dequeue_sem, 0, 0);

    atomic_init(&queue->head, 0);
    atomic_init(&queue->tail, 0);

    queue->capasity = capasity;
    queue->items = malloc(sizeof *queue->items * capasity);

    return queue;
}

void uo_queue_destroy(
    uo_queue *queue)
{
    sem_destroy((sem_t *)&queue->enqueue_sem);
    sem_destroy((sem_t *)&queue->dequeue_sem);

    free(queue->items);
    free(queue);
}

bool uo_queue_enqueue(
    uo_queue *queue, 
    void *item, 
    bool should_block)
{
    if (should_block)
        sem_wait((sem_t *)&queue->enqueue_sem);
    else if (sem_trywait((sem_t *)&queue->enqueue_sem) == -1)
        return false;

    size_t head = atomic_fetch_add(&queue->head, 1);
    queue->items[head % queue->capasity] = item;
    sem_post((sem_t *)&queue->dequeue_sem);

    return true;
}

void *uo_queue_dequeue(
    uo_queue *queue, 
    bool should_block)
{
    if (should_block)
        sem_wait((sem_t *)&queue->dequeue_sem);
    else if (sem_trywait((sem_t *)&queue->dequeue_sem) == -1)
        return NULL;

    size_t tail = atomic_fetch_add(&queue->tail, 1);
    void *item = queue->items[tail % queue->capasity];
    sem_post((sem_t *)&queue->enqueue_sem);

    return item;
}
