#ifndef UO_IO_H
#define UO_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum uo_io_err
{
    UO_IO_ERR_NONE,
    UO_IO_ERR_UNKNOWN
} uo_io_err;

extern _Thread_local uo_io_err uo_io_errno;

/**
 * @brief Initialization of uo_io library
 * 
 * This function is required to be called before using other functions in this library.
 * After a successful call to this function, the following calls are ignored and return true.
 * 
 * @return true     on success
 * @return false    on error
 */
bool uo_io_init(void);

/**
 * @brief Write to a file descriptor
 * 
 * Writes up to len bytes from the buffer pointed buf to the file referred to by the file descriptor wfd.
 * 
 * @param wfd   a file descriptor to write to
 * @param buf   a pointer to the buffer containing the data to be written to the file or device
 * @param len   the number of bytes to be written to the file or device
 * @return      the number of bytes written or if nothing is written or there is 
 *              an error 0 is returned and uo_io_errno is set appropriately
 */
size_t uo_io_write(
    int wfd,
    const void *buf,
    size_t len);

/**
 * @brief Asynchronously read from file descriptor
 * 
 * Asynchronously reads up to len bytes from the file referred to by the file descriptor rfd into the buffer starting at buf.
 * Invokes a callback after operation is finished with the count of transferred bytes and possible error code pushed on the stack of the uo_cb callback.
 * 
 * @param rfd       a file descriptor to read from
 * @param buf       a pointer to a buffer that receives the data read from the file or device
 * @param len       the number of bytes to be read
 * @param cb        The callback that will be invoked after the completion of the asynchronous read operation.
 *                  The result of the read operation will be pushed on the stack of the uo_cb callback.
 *                  On success the result is the number of bytes read from the file descriptor.
 *                  On error two values are pushed on to the stack: an error code of type uo_io_err and a zero.
 */
void uo_io_read_async(
    int rfd,
    void *buf,
    size_t len,
    uo_cb *);

/**
 * @brief Asynchronously write to file descriptor
 * 
 * Asynchronously writes up to len bytes from the buffer pointed buf to the file referred to by the file descriptor wfd.
 * Invokes a callback after operation is finished with the count of transferred bytes and possible error code pushed on the stack of the uo_cb callback.
 * 
 * @param wfd       a file descriptor to write to
 * @param buf       a pointer to the buffer containing the data to be written to the file or device
 * @param len       the number of bytes to be written to the file or device
 * @param cb        The callback that will be invoked after the completion of the asynchronous write operation.
 *                  The result of the write operation will be pushed on the stack of the uo_cb callback.
 *                  On success the result is the number of bytes written to the file descriptor.
 *                  On error two values are pushed on to the stack: an error code of type uo_io_err and a zero.
 */
void uo_io_write_async(
    int wfd,
    void *buf,
    size_t len,
    uo_cb *);

#ifdef __cplusplus
}
#endif

#endif