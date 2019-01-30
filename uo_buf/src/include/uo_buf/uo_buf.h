#ifndef UO_BUF_H
#define UO_BUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>

/**
 * @brief buffer type, that has a position and a size and is capaple of automatically resizing
 * 
 * The buffer maintains two integer values in addition to the actual data:
 *  - the allocated buffer size in bytes
 *  - an arbitrary byte offset from the beginning of the buffer
 * Having this information the buffer can:
 *  - automatically resize when more data is written to buffer than there is allocated memory for the buffer
 *  - hold information about how much data is written to or read from the buffer
 */
typedef unsigned char *uo_buf;

/**
 * @brief allocate a buffer
 * 
 * @param size      number of bytes to allocate for the data
 * @return uo_buf   a buffer that is also a pointer to the beginning of the data
 */
uo_buf uo_buf_alloc(
    size_t size);

/**
 * @brief expands or shrinks previously allocated buffer
 * 
 * @param size      number of bytes to allocate for the data
 * @return uo_buf   a buffer that is also a pointer to the beginning of the data
 */
uo_buf uo_buf_realloc(
    uo_buf,
    size_t size);

/**
 * @brief double the size of a buffer
 * 
 * @return uo_buf   a buffer that is also a pointer to the beginning of the data
 */
uo_buf uo_buf_realloc_2x(
    uo_buf);

/**
 * @brief free the buffer
 * 
 */
void uo_buf_free(
    uo_buf);

/**
 * @brief write a null byte to current position of the buffer
 * 
 * Writes a null byte at offset specified by the position and resize buffer if necessary.
 */
void uo_buf_null_terminate(
    uo_buf *);

/**
 * @brief copy memory to buffer at the current position of the buffer
 * 
 * Copies number of bytes from src to buffer starting from at offset specified by the position and 
 * resize buffer if necessary. Moves the current position forward by the number of bytes copied.
 */
void *uo_buf_memcpy_append(
    uo_buf *restrict buf,
    const void *restrict src, 
    size_t size);

/**
 * @brief print to buffer at the current position of the buffer
 * 
 * Writes the results to the buffer, resizes if necessary. Moves the current position forward 
 * by the number of bytes copied.
 */
int uo_buf_printf_append(
    uo_buf *,
    const char *format,
    ...);

/**
 * @brief gets a pointer to the current position of the buffer
 * 
 */
unsigned char *uo_buf_get_ptr(
    uo_buf);

/**
 * @brief move the position indicator by number of bytes
 * 
 */
void uo_buf_set_ptr_rel(
    uo_buf,
    ptrdiff_t);

/**
 * @brief set the position indicator to a offset from the beginning of the buffer
 * 
 */
void uo_buf_set_ptr_abs(
    uo_buf,
    ptrdiff_t);

/**
 * @brief get the number of bytes from the beginning of the buffer to the current position
 * 
 */
ptrdiff_t uo_buf_get_len_before_ptr(
    uo_buf);

/**
 * @brief get the number of bytes from the current position to the end of the buffer
 * 
 */
ptrdiff_t uo_buf_get_len_after_ptr(
    uo_buf);

/**
 * @brief get the number of bytes that are owned by the buffer starting from the buffer start
 * 
 */
size_t uo_buf_get_size(
    uo_buf);

/**
 * @brief get a pointer to the end of the buffer
 * 
 */
unsigned char *uo_buf_get_end(
    uo_buf);

#ifdef __cplusplus
}
#endif

#endif