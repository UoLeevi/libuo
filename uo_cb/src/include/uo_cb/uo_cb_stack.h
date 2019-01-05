#ifndef UO_CB_STACK_H
#define UO_CB_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define UO_CB_STACK_MIN_ALLOC 2

typedef struct uo_cb_stack
{
    void **items;
    size_t top;
} uo_cb_stack;

void uo_cb_stack_push_stack(
    uo_cb_stack *,
    void *);

void *uo_cb_stack_pop_stack(
    uo_cb_stack *);

void *uo_cb_stack_peek_stack(
    uo_cb_stack *);

void *uo_cb_stack_index_stack(
    uo_cb_stack *,
    int index);

#ifdef __cplusplus
}
#endif

#endif
