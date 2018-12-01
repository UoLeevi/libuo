#ifndef UO_IO_H
#define UO_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool uo_io_init(void);

ssize_t uo_io_write(
	int wfd,
	const void *buf, 
	size_t len);

bool uo_io_read_async(
	int rfd,
	void *buf, 
	size_t len,
	uo_cb *);

bool uo_io_write_async(
	int wfd,
	void *buf, 
	size_t len,
	uo_cb *);

#ifdef __cplusplus
}
#endif

#endif