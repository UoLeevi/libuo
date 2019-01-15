#ifndef UO_TCP_H
#define UO_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Initialization of uo_tcp library
 * 
 * This function is required to be called before using other functions in this library.
 * After a successful call to this function, the following calls are ignored and return true.
 * 
 * @return true     on success
 * @return false    on error
 */
bool uo_tcp_init(void);

#ifdef __cplusplus
}
#endif

#endif
