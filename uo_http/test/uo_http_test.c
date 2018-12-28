#include "uo_http.h"
#include "uo_http_server.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    passed &= uo_http_init();
    uo_http_server *http_server = uo_http_server_create("80");
    passed &= uo_http_server_set_root_dir(http_server, "test_content");
    uo_http_server_start(http_server);

    printf("Press 'q' to quit...\n");
    while(getchar() != 'q');

    uo_http_server_destroy(http_server);

    return passed ? 0 : 1;
}
