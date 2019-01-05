#include "uo_cb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>
#include <assert.h>

#include <semaphore.h>

#define STR_TEST "TEST TEST TEST TEST"

atomic_uint counter;

void first(
    uo_cb_stack *stack)
{
    for (int i = 0; i < 0x8; ++i)
        atomic_fetch_add(&counter, (uintptr_t)uo_cb_stack_pop(stack));

    assert(stack->top == 0x0 || stack->top == 0x8);
}

void second(
    uo_cb_stack *stack)
{
    for (int i = 0; i < 0x8; ++i)
        atomic_fetch_add(&counter, (uintptr_t)uo_cb_stack_pop(stack));

    assert(stack->top == 0x0);
}

int main(
    int argc, 
    char **argv)
{
    bool pass = true;

    atomic_init(&counter, 0);

    pass &= uo_cb_init();

    uo_cb *cb = uo_cb_create();

    uo_cb_stack_push(cb, (void *)(uintptr_t)0x0);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x1);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x2);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x3);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x4);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x5);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x6);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x7);

    pass &= cb->stack.top == 0x8;

    uo_cb_append(cb, first);

    uo_cb *cb_clone = uo_cb_clone(cb);

    uo_cb_stack_push(cb, (void *)(uintptr_t)0x8);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0x9);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0xA);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0xB);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0xC);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0xD);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0xE);
    uo_cb_stack_push(cb, (void *)(uintptr_t)0xF);

    pass &= cb_clone->stack.top == 0x8;
    pass &= cb->stack.top == 0x10;

    uo_cb_append(cb, second);

    sem_t sem, sem_clone;

    uo_cb_invoke_async(cb, &sem);
    uo_cb_invoke_async(cb_clone, &sem_clone);

    sem_wait(&sem);
    sem_wait(&sem_clone);

    pass &= counter == 148;

    return pass ? 0 : 1;
}

