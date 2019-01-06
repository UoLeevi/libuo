#ifndef UO_CB_H
#define UO_CB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb_stack.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include <semaphore.h>

#define UO_CB_F_MIN_ALLOC 2

typedef struct uo_cb 
{
    void (**f)(uo_cb_stack *);
    size_t count;
    uo_cb_stack stack;
} uo_cb;

bool uo_cb_init(void);

uo_cb *uo_cb_create(void);

uo_cb *uo_cb_clone(
    const uo_cb *);

void uo_cb_destroy(
    uo_cb *);

void uo_cb_append_f(
    uo_cb *,
    void (*)(uo_cb_stack *));

void uo_cb_append_cb(
    uo_cb *,
    uo_cb *);

#define uo_cb_append(cb, after) _Generic((after), \
    void (*)(uo_cb_stack *): uo_cb_append_f, \
                    uo_cb *: uo_cb_append_cb)(cb, after)

void uo_cb_prepend_f(
    uo_cb *,
    void (*)(uo_cb_stack *));

void uo_cb_prepend_cb(
    uo_cb *,
    uo_cb *);

#define uo_cb_prepend(cb, before) _Generic((before), \
    void (*)(uo_cb_stack *): uo_cb_prepend_f, \
                    uo_cb *: uo_cb_prepend_cb)(cb, before)

void uo_cb_invoke(
    uo_cb *);

void uo_cb_invoke_async(
    uo_cb *,
    sem_t *);

#define uo_cb_stack_push(cb_stack, ptr) \
    uo_cb_stack_push_stack(_Generic((cb_stack), \
        uo_cb_stack *: cb_stack, \
              uo_cb *: (uo_cb_stack *)((char *)(cb_stack) + offsetof(uo_cb, stack))), (void *)(uintptr_t)(ptr))

#define uo_cb_stack_pop(cb_stack) \
    uo_cb_stack_pop_stack(_Generic((cb_stack), \
        uo_cb_stack *: cb_stack, \
              uo_cb *: (uo_cb_stack *)((char *)(cb_stack) + offsetof(uo_cb, stack))))

#define uo_cb_stack_peek(cb_stack) \
    uo_cb_stack_peek_stack(_Generic((cb_stack), \
        uo_cb_stack *: cb_stack, \
              uo_cb *: (uo_cb_stack *)((char *)(cb_stack) + offsetof(uo_cb, stack))))

#define uo_cb_stack_index(cb_stack, index) \
    uo_cb_stack_index_stack(_Generic((cb_stack), \
        uo_cb_stack *: cb_stack, \
              uo_cb *: (uo_cb_stack *)((char *)(cb_stack) + offsetof(uo_cb, stack))), index)

#ifdef __cplusplus
}
#endif

#endif
