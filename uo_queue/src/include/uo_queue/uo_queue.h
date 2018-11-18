#ifndef UO_QUEUE_H
#define UO_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <semaphore.h>

#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct uo_queue {
	atomic_size_t head;
	atomic_size_t tail;
    size_t capasity;
	sem_t enqueue_sem;
	sem_t dequeue_sem;
	void **items;
} uo_queue;

uo_queue *uo_queue_create(
	size_t capasity);

void uo_queue_destroy(
	uo_queue *);

bool uo_queue_enqueue(
	uo_queue *, 
	void *item, 
	const int blocking);

void *uo_queue_dequeue(
	uo_queue *, 
	const int blocking);

#ifdef __cplusplus
}
#endif

#endif