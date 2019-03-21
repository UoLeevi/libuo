#include "uo_cb.h"
#include "uo_hashtbl.h"
#include "uo_finstack.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct uo_http_req_handler uo_http_req_handler;

extern uo_http_req_handler *uo_http_req_handler_create(
    const char *req_pattern,
    uo_cb *);

extern void uo_http_req_handler_destroy(
    uo_http_req_handler *);

extern bool uo_http_req_handler_try(
    uo_http_req_handler *,
    const char *method_sp_uri,
    uo_strhashtbl *params,
    uo_finstack *);

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    passed &= uo_cb_init();

    uo_finstack *finstack = uo_finstack_create();
    uo_finstack_add(finstack, finstack, (void (*)(void *))uo_finstack_destroy);

    uo_strhashtbl *params = uo_strhashtbl_create(0);
    uo_finstack_add(finstack, params, (void (*)(void *))uo_strhashtbl_destroy);

    uo_cb *cb = uo_cb_create();
    uo_finstack_add(finstack, cb, (void (*)(void *))uo_cb_destroy);

    uo_http_req_handler *http_req_handler1 = uo_http_req_handler_create("/asdf/{var1}/asdf", cb);
    passed &= uo_http_req_handler_try(http_req_handler1, "/asdf", params, finstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler1, "/asdf/123/asdf", params, finstack) == true;
    passed &= strcmp(uo_strhashtbl_get(params, "var1"), "123") == 0;
    uo_http_req_handler_destroy(http_req_handler1);

    uo_http_req_handler *http_req_handler2 = uo_http_req_handler_create("/qwer/{var2}", cb);
    passed &= uo_http_req_handler_try(http_req_handler2, "/qwer/test", params, finstack) == true;
    passed &= strcmp(uo_strhashtbl_get(params, "var2"), "test") == 0;
    uo_http_req_handler_destroy(http_req_handler2);

    uo_http_req_handler *http_req_handler3 = uo_http_req_handler_create("/qwer/{var3}/{var4}/asdf/{var5}", cb);
    passed &= uo_http_req_handler_try(http_req_handler2, "/qwer/val_/val_/asdf", params, finstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler2, "/qwer/val3/val4/asdf/val5", params, finstack) == true;
    passed &= strcmp(uo_strhashtbl_get(params, "var3"), "val3") == 0;
    passed &= strcmp(uo_strhashtbl_get(params, "var4"), "val4") == 0;
    passed &= strcmp(uo_strhashtbl_get(params, "var5"), "val5") == 0;
    uo_http_req_handler_destroy(http_req_handler3);

    uo_finstack_finalize(finstack);

    return passed ? 0 : 1;
}
