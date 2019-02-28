#ifndef UO_STRHASHTBL_H
#define UO_STRHASHTBL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

typedef struct uo_strkvp
{
    const char *key;
    void *value;
} uo_strkvp;

typedef struct uo_strhashtbl uo_strhashtbl;

/**
 * @brief create an instance of uo_strhashtbl
 * 
 * uo_strhashtbl is thread-safe, automatically resizing hash table with string keys.
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_strhashtbl is half full
 * @return uo_strhashtbl *  created uo_strhashtbl instance
 */
uo_strhashtbl *uo_strhashtbl_create(
    size_t initial_capacity);

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
size_t uo_strhashtbl_get_count(
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
 * @brief get next key-value pair in the uo_strhashtbl using a key of the previous key-value pair
 * 
 * Use this function for looping through the items in the uo_strhashtbl.
 * This function always returns a key-value pair if the key exists in the uo_strhashtbl.
 * Internally the key-value pairs are linked using circular linked list.
 * 
 * @param key           null terminated string key or NULL to get the first key-value pair
 * @return uo_strkvp    next key-value pair
 */
uo_strkvp uo_strhashtbl_find_next_strkvp(
    uo_strhashtbl *,
    const char *key);

#ifdef __cplusplus
}
#endif

#endif