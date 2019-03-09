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

typedef struct uo_strkvplist
{
    uo_linklist link;
    uo_strkvp strkvp;
} uo_strkvplist;

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
 * @brief insert new item to uo_strhashtbl or update an existing item
 * 
 * @param key       null terminated string key
 * @param value     pointer to value to be stored
 */
void uo_strhashtbl_insert(
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
 * @brief get the value held by an item which was previously inserted using a key
 * 
 * @param key       null terminated string key
 * @return void *   value held by the item or NULL if no such key exists in the uo_strhashtbl 
 */
void *uo_strhashtbl_find(
    uo_strhashtbl *,
    const char *key);

/**
 * @brief get linked list of key-value pairs starting from specified key
 * 
 * Pass NULL as key to get the first item in the liked list.
 * 
 * Note that returned linked list of key-value pairs is only guaranteed to point valid memory until
 * next resize occures on the link list which can happen during any insert or remove operation.
 * 
 * @param key   null terminated string key or NULL
 */
uo_strkvplist *uo_strhashtbl_list(
    uo_strhashtbl *,
    const char *key);

/**
 * @brief get next item in the linked list of key-value pairs
 * 
 * Pass the uo_strhashtbl as argument to get the first item in the liked list.
 * 
 * Note that returned linked list of key-value pairs is only guaranteed to point valid memory until
 * next resize occures on the link list which can happen during any insert or remove operation.
 */
#define uo_strkvplist_next(strkvplist) \
    ((uo_strkvplist *)uo_linklist_next(strkvplist))

#ifdef __cplusplus
}
#endif

#endif