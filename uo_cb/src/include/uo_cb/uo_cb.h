#ifndef UO_CB_H
#define UO_CB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

typedef struct uo_cb uo_cb;
typedef void (*uo_cb_func)(uo_cb *);

/**
 * @brief initialization of uo_cb library
 * 
 * This function is required to be called before using other functions in this library.
 * After a successful call to this function, the following calls are ignored and return true.
 * 
 * @return true     on success
 * @return false    on error
 */
bool uo_cb_init(void);

/**
 * @brief initialization of uo_cb library for current thread
 * 
 * This function is required to be called from each additional thread before using other 
 * functions in this library. After a successful call to this function, the following 
 * calls are ignored and return true.
 * 
 * @return true     on success
 * @return false    on error
 */
bool uo_cb_thrd_init(void);

/**
 * @brief release of thread specific resources held uo_cb library
 * 
 * This function is should to be called from each additional thread before the thread exits 
 * to release the thread specific resources held by uo_cb library.
 */
void uo_cb_thrd_quit(void);

/**
 * @brief create a callback
 * 
 * Callback is a object that contains an ordered list of functions to execute and a stack 
 * containing pointers to data.
 * 
 * @return  uo_cb *  new callback instance
 */
uo_cb *uo_cb_create(void);

/**
 * @brief create a clone of a callback
 * 
 * @return uo_cb *  new callback instance with identical stack and function list
 */
uo_cb *uo_cb_clone(
    const uo_cb *);

/**
 * @brief free up the resources owned by the callback instance
 * 
 * This function is automatically called after the last function in the function list
 * is called.
 */
void uo_cb_destroy(
    uo_cb *);

/**
 * @brief call the next function in the function list or free owned resources if the function list is empty
 * 
 */
void uo_cb_invoke(
    uo_cb *);

/**
 * @brief asynchronously call the next function in the function list or free owned resources if the function list is empty
 * 
 */
void uo_cb_invoke_async(
    uo_cb *);

/**
 * @brief depending on the type of the second argument call either uo_cb_prepend_func or uo_cb_prepend_cb
 * 
 */
#define uo_cb_prepend(cb, before) _Generic((before), \
    uo_cb_func: uo_cb_prepend_func, \
       uo_cb *: uo_cb_prepend_cb)(cb, before)

/**
 * @brief add function to the beginning of the function list
 * 
 */
void uo_cb_prepend_func(
    uo_cb *,
    uo_cb_func);

/**
 * @brief add functions of a another callback to the beginning of the function list and also combine the stacks
 * 
 * The stack items of the callback that is prepended are copied to the top of the stack of the callback
 * that is being prepended to. This means that:
 *  - the positive indexes of the stack items of the first argument are unchanged
 *  - the negative indexes of the stack items of the second argument are unchanged 
 * 
 * @param cb_before     a callback instance to prepend
 */
void uo_cb_prepend_cb(
    uo_cb *,
    uo_cb *cb_before);

/**
 * @brief depending on the type of the second argument call either uo_cb_append_func or uo_cb_append_cb
 * 
 */
#define uo_cb_append(cb, after) _Generic((after), \
    uo_cb_func: uo_cb_append_func, \
       uo_cb *: uo_cb_append_cb)(cb, after)

/**
 * @brief add function to the end of the function list
 * 
 */
void uo_cb_append_func(
    uo_cb *,
    uo_cb_func);

/**
 * @brief add functions of a another callback to the end of the function list and also combine the stacks
 * 
 * The stack items of the callback that is appended are copied to the bottom of the stack of the callback
 * that is being appended to. This means that:
 *  - the negatice indexes of the stack items of the first argument are unchanged
 *  - the positive indexes of the stack items of the second argument are unchanged 
 * 
 * @param cb_after     a callback instance to append
 */
void uo_cb_append_cb(
    uo_cb *,
    uo_cb *cb_after);

/**
 * @brief push a pointer to the top of the stack of the callback
 * 
 */
void uo_cb_stack_push(
    uo_cb *,
    void *item);

/**
 * @brief pop a pointer from the top of the stack of the callback
 * 
 */
void *uo_cb_stack_pop(
    uo_cb *);

/**
 * @brief get a pointer from the top of the stack of the callback
 * 
 */
void *uo_cb_stack_peek(
    uo_cb *);

/**
 * @brief get a pointer from the the stack of the callback by index
 * 
 * @param index     use negative index to index starting from one past the last item of the stack
 */
void *uo_cb_stack_index(
    uo_cb *,
    int index);

#ifdef __cplusplus
}
#endif

#endif
