#ifndef UO_CB_H
#define UO_CB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include <semaphore.h>

typedef struct uo_cb uo_cb;

typedef void uo_cb_func(uo_cb *);

typedef struct uo_cb 
{
    struct
    {
        uo_cb_func **items;
        size_t count;
    } func_list;
    struct
    {
        void **items;
        size_t top;
    } stack;
} uo_cb;


/**
 * @brief Initialization of uo_cb library
 * 
 * This function is required to be called before using other functions in this library.
 * After a successful call to this function, the following calls are ignored and return true.
 * 
 * @return true     on success
 * @return false    on error
 */
bool uo_cb_init(void);

/**
 * @brief Create a callback
 * 
 * Callback is a object that contains an ordered list of functions to execute and a stack 
 * containing pointers to data.
 * 
 * @return  uo_cb *  new callback instance
 */
uo_cb *uo_cb_create(void);

/**
 * @brief Create a clone of a callback
 * 
 * @return uo_cb *  new callback instance with identical stack and function list
 */
uo_cb *uo_cb_clone(
    const uo_cb *);


/**
 * @brief Free up the resources owned by the callback instance
 * 
 * This function is automatically called after the last function in the function list
 * is called.
 */
void uo_cb_destroy(
    uo_cb *);

/**
 * @brief Call the next function in the function list or free owned resources if the function list is empty
 * 
 */
void uo_cb_invoke(
    uo_cb *);

/**
 * @brief Asynchronously call the next function in the function list or free owned resources if the function list is empty
 * 
 * @param sem   NULL or a pointer to uninitialized semaphore if the completion needs to be awaited
 */
void uo_cb_invoke_async(
    uo_cb *,
    sem_t *sem);

/**
 * @brief Depending on the type of the second argument call either uo_cb_prepend_func or uo_cb_prepend_cb
 * 
 */
#define uo_cb_prepend(cb, before) _Generic((before), \
    uo_cb_func *: uo_cb_prepend_func, \
         uo_cb *: uo_cb_prepend_cb)(cb, before)

/**
 * @brief Add function to the beginning of the function list
 * 
 */
void uo_cb_prepend_func(
    uo_cb *,
    uo_cb_func *);

/**
 * @brief Add functions of a another callback to the beginning of the function list and also combine the stacks
 * 
 * @param cb_before     a callback instance to prepend
 */
void uo_cb_prepend_cb(
    uo_cb *,
    uo_cb *cb_before);

/**
 * @brief Depending on the type of the second argument call either uo_cb_append_func or uo_cb_append_cb
 * 
 */
#define uo_cb_append(cb, after) _Generic((after), \
    uo_cb_func *: uo_cb_append_func, \
         uo_cb *: uo_cb_append_cb)(cb, after)

/**
 * @brief Add function to the end of the function list
 * 
 */
void uo_cb_append_func(
    uo_cb *,
    uo_cb_func *);

/**
 * @brief Add functions of a another callback to the end of the function list and also combine the stacks
 * 
 * @param cb_after     a callback instance to append
 */
void uo_cb_append_cb(
    uo_cb *,
    uo_cb *cb_after);

/**
 * @brief Push a pointer to the top of the stack of the callback
 * 
 */
void uo_cb_stack_push(
    uo_cb *,
    void *);

/**
 * @brief Pop a pointer from the top of the stack of the callback
 * 
 */
void *uo_cb_stack_pop(
    uo_cb *);

/**
 * @brief Get a pointer from the top of the stack of the callback
 * 
 */
void *uo_cb_stack_peek(
    uo_cb *);

/**
 * @brief Get a pointer from the the stack of the callback by index
 * 
 * @param index     use negative index to index starting from the last item of the stack
 */
void *uo_cb_stack_index(
    uo_cb *,
    int index);

#ifdef __cplusplus
}
#endif

#endif
