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
    uo_cb *cb)
{
    for (int i = 0; i < 0x8; ++i)
        atomic_fetch_add(&counter, (uintptr_t)uo_cb_stack_pop(cb));

    uo_cb_invoke(cb);
}

void second(
    uo_cb *cb)
{
    for (int i = 0; i < 0x8; ++i)
        atomic_fetch_add(&counter, (uintptr_t)uo_cb_stack_pop(cb));

    uo_cb_invoke(cb);
}

void post_sem(
    uo_cb *cb)
{
    sem_post(uo_cb_stack_pop(cb));

    uo_cb_invoke(cb);
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

    uo_cb_append(cb, second);

    sem_t sem, sem_clone;

    sem_init(&sem, 0, 0);
    sem_init(&sem_clone, 0, 0);

    uo_cb *sem_cb = uo_cb_create();
    uo_cb_append(sem_cb, post_sem);
    uo_cb *sem_clone_cb = uo_cb_clone(sem_cb);

    uo_cb_stack_push(sem_cb, &sem);
    uo_cb_stack_push(sem_clone_cb, &sem_clone);

    uo_cb_append_cb(cb, sem_cb);
    uo_cb_append_cb(cb_clone, sem_clone_cb);

    uo_cb_destroy(sem_cb);
    uo_cb_destroy(sem_clone_cb);

    uo_cb_invoke_async(cb);
    uo_cb_invoke_async(cb_clone);

    sem_wait(&sem);
    sem_wait(&sem_clone);

    pass &= counter == 148;

    return pass ? 0 : 1;
}

