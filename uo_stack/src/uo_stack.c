#include "uo_stack.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

uo_stack *uo_stack_create(
	size_t capasity)
{
	uo_stack *stack = calloc(1, sizeof *stack);
	sem_init(&stack->push_sem, 0, capasity);
	sem_init(&stack->pop_sem, 0, 0);
	pthread_mutex_init(&stack->mtx, NULL);
	stack->items = malloc(sizeof(void *) * capasity);
}

void *uo_stack_destroy(
	uo_stack *stack)
{
	free(stack->items);
	free(stack);
}

bool uo_stack_push(
	uo_stack *stack, 
	void *item, 
	const int blocking)
{
	if (blocking)
		sem_wait(&stack->push_sem);
	else if (sem_trywait(&stack->push_sem) == -1)
		return false;

	pthread_mutex_lock(&stack->mtx);
	stack->items[stack->top++] = item;
	pthread_mutex_unlock(&stack->mtx);
	sem_post(&stack->pop_sem);

	return true;
}

void *uo_stack_pop(
	uo_stack *stack, 
	const int blocking)
{
	if (blocking)
		sem_wait(&stack->pop_sem);
	else if (sem_trywait(&stack->pop_sem) == -1)
		return NULL;

	pthread_mutex_lock(&stack->mtx);
	void *item = stack->items[--stack->top];
	pthread_mutex_unlock(&stack->mtx);
	sem_post(&stack->push_sem);

	return item;
}
