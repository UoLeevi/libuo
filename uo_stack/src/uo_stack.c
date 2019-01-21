#include "uo_stack.h"

#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

uo_stack *uo_stack_create(
    size_t capasity)
{
    uo_stack *stack = calloc(1, sizeof *stack);

    sem_init((sem_t *)&stack->push_sem, 0, capasity);
    sem_init((sem_t *)&stack->pop_sem, 0, 0);

    pthread_mutex_init((pthread_mutex_t *)&stack->mtx, NULL);

    stack->items = malloc(sizeof(void *) * capasity);

    return stack;
}

void uo_stack_destroy(
    uo_stack *stack)
{
    sem_destroy((sem_t *)&stack->push_sem);
    sem_destroy((sem_t *)&stack->pop_sem);

    free(stack->items);
    free(stack);
}

bool uo_stack_push(
    uo_stack *stack,
    void *item,
    bool should_block)
{
    if (should_block)
        sem_wait((sem_t *)&stack->push_sem);
    else if (sem_trywait((sem_t *)&stack->push_sem) == -1)
        return false;

    pthread_mutex_lock((pthread_mutex_t *)&stack->mtx);
    stack->items[stack->top++] = item;
    pthread_mutex_unlock((pthread_mutex_t *)&stack->mtx);
    sem_post((sem_t *)&stack->pop_sem);

    return true;
}

void *uo_stack_pop(
    uo_stack *stack,
    bool should_block)
{
    if (should_block)
        sem_wait((sem_t *)&stack->pop_sem);
    else if (sem_trywait((sem_t *)&stack->pop_sem) == -1)
        return NULL;

    pthread_mutex_lock((pthread_mutex_t *)&stack->mtx);
    void *item = stack->items[--stack->top];
    pthread_mutex_unlock((pthread_mutex_t *)&stack->mtx);
    sem_post((sem_t *)&stack->push_sem);

    return item;
}
