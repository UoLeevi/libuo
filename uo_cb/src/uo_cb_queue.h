#ifndef UO_CB_QUEUE_H
#define UO_CB_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"

bool uo_cb_queue_init(void);

void uo_cb_queue_enqueue(
    uo_cb *);

uo_cb *uo_cb_queue_try_dequeue(void);

#ifdef __cplusplus
}
#endif

#endif