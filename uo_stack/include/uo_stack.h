#ifndef UO_STACK_H
#define UO_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

typedef struct uo_stack {
	size_t top;
	sem_t push_sem;
	sem_t pop_sem;
	pthread_mutex_t mtx;
	void **items;
} uo_stack;

uo_stack *uo_stack_create(
	size_t capasity);

void *uo_stack_destroy(
	uo_stack *);

bool uo_stack_push(
	uo_stack *, 
	void *item, 
	const int blocking);

void *uo_stack_pop(
	uo_stack *, 
	const int blocking);

#ifdef __cplusplus
}
#endif

#endif