#include "uo_cb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <semaphore.h>

#define STR_TEST "TEST TEST TEST TEST"

void *first(
    void *arg, 
    uo_cb *cb)
{
    uo_cb_stack_push(cb, arg);
    return arg;
}

void *second(
    void *arg, 
    uo_cb *cb)
{
    assert(arg == uo_cb_stack_pop(cb));
    assert((uintptr_t)arg + 1 == 2);
    return NULL;
}

int main(
    int argc, 
    char **argv)
{
    bool pass = true;

    pass &= uo_cb_init();

    uo_cb *cb = uo_cb_create(UO_CB_OPT_DESTROY);

    uo_cb_append(cb, first);
    uo_cb_append(cb, second);

    sem_t sem;

    uo_cb_invoke_async(cb, (void *)(uintptr_t)1, &sem);

    sem_wait(&sem);

    return pass ? 0 : 1;
}

