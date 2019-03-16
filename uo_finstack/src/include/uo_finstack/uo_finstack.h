#ifndef UO_FINSTACK_H
#define UO_FINSTACK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uo_stack uo_finstack;

/**
 * @brief create an instance of uo_finstack
 * 
 * uo_finstack is used for grouping resources that should be finalized together.
 * The order of finalization for each of the grouped resource is reverse to the order they were added.
 * 
 * @return uo_finstack *  created uo_finstack instance
 */
uo_finstack *uo_finstack_create(void);

/**
 * @brief free resources used by an uo_finstack instance itself
 * 
 */
void uo_finstack_destroy(
    uo_finstack *);

/**
 * @brief add resource to the uo_finstack stack
 * 
 * @param ptr           resource to be added to the finstack
 * @param finalizer     pointer to the finalizer function for the resource
 */
void uo_finstack_add(
    uo_finstack *,
    void *ptr,
    void (*finalizer)(void *));

/**
 * @brief finalize all the resources added to the uo_finstack instance
 * 
 */
void uo_finstack_finalize(
    uo_finstack *);

#ifdef __cplusplus
}
#endif

#endif