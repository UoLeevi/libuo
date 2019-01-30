#ifndef UO_PROG_H
#define UO_PROG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief wait for SIGINT or similar interruption message such as CTRL+C
 * 
 */
void uo_prog_wait_for_sigint(void);

/**
 * @brief Initialization of uo_prog library
 * 
 * This function is required to be called before using other functions in this library.
 * After a successful call to this function, the following calls are ignored and return true.
 * 
 * @return true     on success
 * @return false    on error
 */
bool uo_prog_init(void);

#ifdef __cplusplus
}
#endif

#endif