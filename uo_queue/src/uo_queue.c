#include "uo_queue.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

uo_queue *uo_queue_create(
	size_t capasity)
{
	uo_queue *queue = calloc(1, sizeof *queue);
	sem_init(&queue->enqueue_sem, 0, capasity);
	sem_init(&queue->dequeue_sem, 0, 0);
	pthread_mutex_init(&queue->enqueue_mtx, NULL);
	pthread_mutex_init(&queue->dequeue_mtx, NULL);
	queue->capasity = capasity;
	queue->items = malloc(sizeof *queue->items * capasity);
	return queue;
}

void *uo_queue_destroy(
	uo_queue *queue)
{
	free(queue->items);
	free(queue);
}

bool uo_queue_enqueue(
	uo_queue *queue, 
	void *item, 
	const int blocking)
{
	if (blocking)
		sem_wait(&queue->enqueue_sem);
	else if (sem_trywait(&queue->enqueue_sem) == -1)
		return false;

	pthread_mutex_lock(&queue->enqueue_mtx);
	queue->items[queue->head] = item;
	++queue->head;
	queue->head %= queue->capasity;
	pthread_mutex_unlock(&queue->enqueue_mtx);
	sem_post(&queue->dequeue_sem);

	return true;
}

void *uo_queue_dequeue(
	uo_queue *queue, 
	const int blocking)
{
	if (blocking)
		sem_wait(&queue->dequeue_sem);
	else if (sem_trywait(&queue->dequeue_sem) == -1)
		return NULL;

	pthread_mutex_lock(&queue->dequeue_mtx);
	void *item = queue->items[queue->tail];
	++queue->tail;
	queue->tail %= queue->capasity;
	pthread_mutex_unlock(&queue->dequeue_mtx);
	sem_post(&queue->enqueue_sem);

	return item;
}
