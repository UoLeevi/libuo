#include "uo_cb.h"
#include "uo_hashtbl.h"
#include "uo_refstack.h"
#include "uo_util.h"

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
    uo_refstack *);

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    passed &= uo_cb_init();

    uo_refstack *refstack = uo_refstack_create();
    uo_refstack_push(refstack, refstack, (void (*)(void *))uo_refstack_destroy);

    uo_strhashtbl *params = uo_strhashtbl_create(0);
    uo_refstack_push(refstack, params, (void (*)(void *))uo_strhashtbl_destroy);

    uo_cb *cb = uo_cb_create();
    uo_refstack_push(refstack, cb, (void (*)(void *))uo_cb_destroy);

    uo_http_req_handler *http_req_handler1 = uo_http_req_handler_create("/asdf/{var1}/asdf", cb);
    passed &= uo_http_req_handler_try(http_req_handler1, "/asdf", params, refstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler1, "/asdf/123/asdf", params, refstack) == true;
    passed &= strcmp(uo_strhashtbl_get(params, "var1"), "123") == 0;
    uo_http_req_handler_destroy(http_req_handler1);

    uo_http_req_handler *http_req_handler2 = uo_http_req_handler_create("/qwer/{var2}/", cb);
    passed &= uo_http_req_handler_try(http_req_handler2, "/qwer/test", params, refstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler2, "/qwer/test/asdf", params, refstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler2, "/qwer/test/", params, refstack) == true;
    passed &= strcmp(uo_strhashtbl_get(params, "var2"), "test") == 0;
    uo_http_req_handler_destroy(http_req_handler2);

    uo_http_req_handler *http_req_handler3 = uo_http_req_handler_create("/qwer/{var3}/{var4}/asdf/{var5}", cb);
    passed &= uo_http_req_handler_try(http_req_handler3, "/qwer/val_/val_/asdf", params, refstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler3, "/qwer/val3/val4/asdf/val5", params, refstack) == true;
    passed &= strcmp(uo_strhashtbl_get(params, "var3"), "val3") == 0;
    passed &= strcmp(uo_strhashtbl_get(params, "var4"), "val4") == 0;
    passed &= strcmp(uo_strhashtbl_get(params, "var5"), "val5") == 0;
    uo_http_req_handler_destroy(http_req_handler3);

    uo_http_req_handler *http_req_handler4 = uo_http_req_handler_create("/asdf/qwer/*", cb);
    passed &= uo_http_req_handler_try(http_req_handler4, "/asdf/qwer/zxcv", params, refstack) == true;
    passed &= uo_http_req_handler_try(http_req_handler4, "/asdf/qwer/", params, refstack) == true;
    passed &= uo_http_req_handler_try(http_req_handler4, "/asdf/qwer", params, refstack) == false;
    uo_http_req_handler_destroy(http_req_handler4);

    uo_http_req_handler *http_req_handler5 = uo_http_req_handler_create("/asdf/qwer/", cb);
    passed &= uo_http_req_handler_try(http_req_handler5, "/asdf/qwer/zxcv", params, refstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler5, "/asdf/qwer/", params, refstack) == true;
    passed &= uo_http_req_handler_try(http_req_handler5, "/asdf/qwer", params, refstack) == false;
    uo_http_req_handler_destroy(http_req_handler5);

    uo_http_req_handler *http_req_handler6 = uo_http_req_handler_create("/asdf/{var6}/qwer/*", cb);
    passed &= uo_http_req_handler_try(http_req_handler6, "/asdf/qwer/zxcv", params, refstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler6, "/asdf/qwer/", params, refstack) == false;
    passed &= uo_http_req_handler_try(http_req_handler6, "/asdf/qwer/qwer/", params, refstack) == true;
    passed &= uo_http_req_handler_try(http_req_handler6, "/asdf/qwer/qwer/asdf", params, refstack) == true;
    uo_http_req_handler_destroy(http_req_handler6);

    uo_refstack_finalize(refstack);

    return passed ? 0 : 1;
}
