#ifndef UO_STRHASHTBL_H
#define UO_STRHASHTBL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_linklist.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct uo_strkvp
{
    const char *key;
    void *value;
} uo_strkvp;

uo_def_linklist(uo_strkvp); // typedef uo_strkvp_link

typedef struct uo_strhashtbl uo_strhashtbl;

/**
 * @brief create an instance of uo_strhashtbl at specific memory location
 * 
 * uo_strhashtbl is automatically resizing hash table with string keys.
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_strhashtbl is half full
 */
void uo_strhashtbl_create_at(
    uo_strhashtbl *,
    size_t initial_capacity);

/**
 * @brief create an instance of uo_strhashtbl
 * 
 * uo_strhashtbl is automatically resizing hash table with string keys.
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_strhashtbl is half full
 * @return uo_strhashtbl *  created uo_strhashtbl instance
 */
uo_strhashtbl *uo_strhashtbl_create(
    size_t initial_capacity);

/**
 * @brief free resources used by an uo_strhashtbl instance but do not free the uo_strhashtbl pointer itself
 * 
 */
void uo_strhashtbl_destroy_at(
    uo_strhashtbl *);

/**
 * @brief free resources used by an uo_strhashtbl instance
 * 
 */
void uo_strhashtbl_destroy(
    uo_strhashtbl *);

/**
 * @brief get the number of key value pairs stored to uo_strhashtbl
 * 
 * @return size_t   the number of key value pairs stored to uo_strhashtbl
 */
size_t uo_strhashtbl_count(
    uo_strhashtbl *);

/**
 * @brief get the value held by an item which was previously inserted using a key
 * 
 * @param key       null terminated string key
 * @return void *   value held by the item or NULL if no such key exists in the uo_strhashtbl 
 */
void *uo_strhashtbl_get(
    uo_strhashtbl *,
    const char *key);

/**
 * @brief insert new item to uo_strhashtbl or update an existing item
 * 
 * @param key       null terminated string key
 * @param value     pointer to value to be stored
 */
void uo_strhashtbl_set(
    uo_strhashtbl *, 
    const char *key, 
    const void *value);

/**
 * @brief remove an item and get the value held by the item
 * 
 * @param key       null terminated string key
 * @return void *   value held by the removed item or NULL if no such key existed in the uo_strhashtbl 
 */
void *uo_strhashtbl_remove(
    uo_strhashtbl *, 
    const char *key);

/**
 * @brief get the head of the linked list with key-value pairs
 * 
 */
uo_strkvp_linklist *uo_strhashtbl_list(
    uo_strhashtbl *);

#ifdef __cplusplus
}
#endif

#endif