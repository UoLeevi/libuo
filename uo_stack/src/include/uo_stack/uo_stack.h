#ifndef UO_STACK_H
#define UO_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

typedef struct uo_stack 
{
    void **items;
    size_t top;
    void *push_sem;
    void *pop_sem;
    void *mtx;
} uo_stack;

uo_stack *uo_stack_create(
    size_t capasity);

void uo_stack_destroy(
    uo_stack *);

bool uo_stack_push(
    uo_stack *,
    void *item,
    bool should_block);

void *uo_stack_pop(
    uo_stack *, 
    bool should_block);

#ifdef __cplusplus
}
#endif

#endif