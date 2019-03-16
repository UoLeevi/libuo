#ifndef UO_HASHTBL_H
#define UO_HASHTBL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_linklist.h"
#include "uo_util.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief uo_hashtbl is automatically resizing hash table
 * 
 */
typedef struct uo_hashtbl uo_hashtbl;
typedef struct uo_kvp_linklist uo_kvp_linklist;

/**
 * @brief create an instance of uo_hashtbl at specific memory location
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_hashtbl is half full
 */
static void uo_hashtbl_create_at(
    uo_hashtbl *,
    size_t initial_capacity);

/**
 * @brief create an instance of uo_hashtbl
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_hashtbl is half full
 * @return uo_hashtbl *  created uo_hashtbl instance
 */
static uo_hashtbl *uo_hashtbl_create(
    size_t initial_capacity);

/**
 * @brief free resources used by an uo_hashtbl instance but do not free the uo_hashtbl pointer itself
 * 
 */
static void uo_hashtbl_destroy_at(
    uo_hashtbl *);

/**
 * @brief free resources used by an uo_hashtbl instance
 * 
 */
static void uo_hashtbl_destroy(
    uo_hashtbl *);

/**
 * @brief get the number of key value pairs stored to uo_hashtbl
 * 
 * @return size_t   the number of key value pairs stored to uo_hashtbl
 */
static size_t uo_hashtbl_count(
    uo_hashtbl *);

/**
 * @brief get the value held by an item which was previously inserted using a key
 * 
 * @param key       key that was used when uo_hashtbl_set was used
 * @return void *   value held by the item or NULL if no such key exists in the uo_hashtbl 
 */
static void *uo_hashtbl_get(
    uo_hashtbl *,
    const void *key);

/**
 * @brief insert new item to uo_hashtbl or update an existing item
 * 
 * @param key       key that can be later used to retrieve the value using uo_hashtbl_get
 * @param value     pointer to value to be stored
 */
static void uo_hashtbl_set(
    uo_hashtbl *, 
    const void *key, 
    const void *value);

/**
 * @brief remove an item and get the value held by the item
 * 
 * @param key       key that was used when uo_hashtbl_set was used
 * @return void *   value held by the removed item or NULL if no such key existed in the uo_hashtbl 
 */
static void *uo_hashtbl_remove(
    uo_hashtbl *, 
    const void *key);

/**
 * @brief get the head of the linked list with key-value pairs
 * 
 */
static uo_kvp_linklist *uo_hashtbl_list(
    uo_hashtbl *);


#define UO_HASHTBL_MIN_CAPACITY 0x10
#define UO_HASHTBL_TAG_REMOVED ((uintptr_t)~0)

#define uo__hashtbl_type(prefix) \
    prefix ## hashtbl

#define uo__kvp_type(prefix) \
    prefix ## kvp

#define uo__kvp_linklist_type(prefix) \
    prefix ## kvp_linklist

#define uo__hashtbl_create_at_f(prefix) \
    prefix ## hashtbl_create_at

#define uo__hashtbl_create_f(prefix) \
    prefix ## hashtbl_create

#define uo__hashtbl_destroy_at_f(prefix) \
    prefix ## hashtbl_destroy_at

#define uo__hashtbl_destroy_f(prefix) \
    prefix ## hashtbl_destroy

#define uo__hashtbl_find_link_f(prefix) \
    prefix ## hashtbl_find_link

#define uo__hashtbl_list_f(prefix) \
    prefix ## hashtbl_list

#define uo__hashtbl_resize_f(prefix) \
    prefix ## hashtbl_resize

#define uo__hashtbl_get_f(prefix) \
    prefix ## hashtbl_get

#define uo__hashtbl_set_f(prefix) \
    prefix ## hashtbl_set

#define uo__hashtbl_remove_f(prefix) \
    prefix ## hashtbl_remove

#define uo_def_hashtbl(prefix, type, hash, equals)                                            \
                                                                                              \
typedef struct uo__kvp_type(prefix)                                                           \
{                                                                                             \
    type key;                                                                                 \
    void *value;                                                                              \
} uo__kvp_type(prefix);                                                                       \
                                                                                              \
uo_def_linklist(uo__kvp_type(prefix));                                                        \
                                                                                              \
typedef struct uo__hashtbl_type(prefix)                                                       \
{                                                                                             \
    uo_linklist head;                                                                         \
    size_t count;                                                                             \
    size_t removed_count;                                                                     \
    size_t capacity;                                                                          \
    uo__kvp_linklist_type(prefix) *links;                                                     \
} uo__hashtbl_type(prefix);                                                                   \
                                                                                              \
static void uo__hashtbl_create_at_f(prefix)(                                                  \
    uo__hashtbl_type(prefix) *hashtbl,                                                        \
    size_t initial_capacity)                                                                  \
{                                                                                             \
    size_t capacity = initial_capacity >= UO_HASHTBL_MIN_CAPACITY                             \
        ? uo_ceil_pow2(initial_capacity)                                                      \
        : UO_HASHTBL_MIN_CAPACITY;                                                            \
                                                                                              \
    hashtbl->count = 0;                                                                       \
    hashtbl->removed_count = 0;                                                               \
    hashtbl->links = calloc(hashtbl->capacity = capacity, sizeof *hashtbl->links);            \
                                                                                              \
    uo_linklist_selflink(hashtbl);                                                            \
}                                                                                             \
                                                                                              \
static uo__hashtbl_type(prefix) *uo__hashtbl_create_f(prefix)(                                \
    size_t initial_capacity)                                                                  \
{                                                                                             \
    uo__hashtbl_type(prefix) *hashtbl = malloc(sizeof *hashtbl);                              \
    uo__hashtbl_create_at_f(prefix)(hashtbl, initial_capacity);                               \
    return hashtbl;                                                                           \
}                                                                                             \
                                                                                              \
static inline void uo__hashtbl_destroy_at_f(prefix)(                                          \
    uo__hashtbl_type(prefix) *hashtbl)                                                        \
{                                                                                             \
    free(hashtbl->links);                                                                     \
}                                                                                             \
                                                                                              \
static inline void uo__hashtbl_destroy_f(prefix)(                                             \
    uo__hashtbl_type(prefix) *hashtbl)                                                        \
{                                                                                             \
    free(hashtbl->links);                                                                     \
    free(hashtbl);                                                                            \
}                                                                                             \
                                                                                              \
static uo__kvp_linklist_type(prefix) *uo__hashtbl_find_link_f(prefix)(                        \
    uo__kvp_linklist_type(prefix) *links,                                                     \
    size_t capacity,                                                                          \
    type key)                                                                                 \
{                                                                                             \
    const uint64_t mask = capacity - 1;                                                       \
    uint64_t h = hash(key) & mask;                                                            \
    const uo__kvp_linklist_type(prefix) *link = links + h;                                    \
                                                                                              \
    while (link->item.key && !equals(link->item.key, key)                                     \
        || (uintptr_t)link->item.value == UO_HASHTBL_TAG_REMOVED)                             \
        link = links + (++h & mask);                                                          \
                                                                                              \
    return (uo__kvp_linklist_type(prefix) *)link;                                             \
}                                                                                             \
                                                                                              \
static inline uo__kvp_linklist_type(prefix) *uo__hashtbl_list_f(prefix)(                      \
    uo__hashtbl_type(prefix) *hashtbl)                                                        \
{                                                                                             \
    return (uo__kvp_linklist_type(prefix) *)&hashtbl->head;                                   \
}                                                                                             \
                                                                                              \
static void uo__hashtbl_resize_f(prefix)(                                                     \
    uo__hashtbl_type(prefix) *hashtbl,                                                        \
    size_t capacity)                                                                          \
{                                                                                             \
    hashtbl->capacity = capacity;                                                             \
    uo__kvp_linklist_type(prefix) *new_links = calloc(capacity, sizeof *hashtbl->links);      \
                                                                                              \
    uo_linklist head = hashtbl->head;                                                         \
                                                                                              \
    uo__kvp_linklist_type(prefix) *link = (uo__kvp_linklist_type(prefix) *)&head;             \
    uo__kvp_linklist_type(prefix) *new_link;                                                  \
                                                                                              \
    uo_linklist_unlink(hashtbl);                                                              \
    uo_linklist_selflink(hashtbl);                                                            \
                                                                                              \
    size_t count = hashtbl->count;                                                            \
                                                                                              \
    for (size_t i = 0; i < count; ++i)                                                        \
    {                                                                                         \
        link = (uo__kvp_linklist_type(prefix) *)link->link.next;                              \
        new_link = uo__hashtbl_find_link_f(prefix)(new_links, capacity, link->item.key);      \
        new_link->item = link->item;                                                          \
        uo_linklist_link(hashtbl, new_link);                                                  \
    }                                                                                         \
                                                                                              \
    free(hashtbl->links);                                                                     \
    hashtbl->links = new_links;                                                               \
}                                                                                             \
                                                                                              \
static inline void *uo__hashtbl_get_f(prefix)(                                                \
    uo__hashtbl_type(prefix) *hashtbl,                                                        \
    type key)                                                                                 \
{                                                                                             \
    return key                                                                                \
        ? uo__hashtbl_find_link_f(prefix)(                                                    \
            hashtbl->links, hashtbl->capacity, (const type )key)->item.value                  \
        : NULL;                                                                               \
}                                                                                             \
                                                                                              \
static void uo__hashtbl_set_f(prefix)(                                                        \
    uo__hashtbl_type(prefix) *hashtbl,                                                        \
    type key,                                                                                 \
    const void *value)                                                                        \
{                                                                                             \
    if (!key)                                                                                 \
        return;                                                                               \
                                                                                              \
    uo__kvp_linklist_type(prefix) *link = uo__hashtbl_find_link_f(prefix)(                    \
        hashtbl->links, hashtbl->capacity, key);                                              \
    link->item.value = (void *)value;                                                         \
                                                                                              \
    if (link->item.key)                                                                       \
        return;                                                                               \
                                                                                              \
    link->item.key = key;                                                                     \
                                                                                              \
    uo_linklist_link(hashtbl, link);                                                          \
                                                                                              \
    if (++hashtbl->count == hashtbl->capacity >> 1)                                           \
    {                                                                                         \
        uo__hashtbl_resize_f(prefix)(hashtbl, hashtbl->capacity << 1);                        \
        hashtbl->removed_count = 0;                                                           \
    }                                                                                         \
}                                                                                             \
                                                                                              \
static void *uo__hashtbl_remove_f(prefix)(                                                    \
    uo__hashtbl_type(prefix) *hashtbl,                                                        \
    type key)                                                                                 \
{                                                                                             \
    if (!key)                                                                                 \
        return NULL;                                                                          \
                                                                                              \
    uo__kvp_linklist_type(prefix) *link = uo__hashtbl_find_link_f(prefix)(                    \
        hashtbl->links, hashtbl->capacity, key);                                              \
                                                                                              \
    if (!link->item.key)                                                                      \
        return NULL;                                                                          \
                                                                                              \
    void *value = link->item.value;                                                           \
    link->item.key = NULL;                                                                    \
    link->item.value = (void *)UO_HASHTBL_TAG_REMOVED;                                        \
                                                                                              \
    uo_linklist_unlink(link);                                                                 \
                                                                                              \
    size_t removed_count = ++hashtbl->removed_count;                                          \
    size_t count = --hashtbl->count;                                                          \
                                                                                              \
    if (removed_count >= hashtbl->capacity >> 2)                                              \
    {                                                                                         \
        uo__hashtbl_resize_f(prefix)(hashtbl, hashtbl->capacity);                             \
        hashtbl->removed_count = 0;                                                           \
    }                                                                                         \
                                                                                              \
    if ((count == hashtbl->capacity >> 3) >= UO_HASHTBL_MIN_CAPACITY)                         \
    {                                                                                         \
        uo__hashtbl_resize_f(prefix)(hashtbl, hashtbl->capacity >> 1);                        \
        hashtbl->removed_count = 0;                                                           \
    }                                                                                         \
                                                                                              \
    return value;                                                                             \
}                                                                                             \

uo_def_hashtbl(uo_str, const char *, uo_strhash_djb2, uo_streq);
uo_def_hashtbl(uo_, const void *, (uintptr_t), uo_ptreq);

#ifdef __cplusplus
}
#endif

#endif
