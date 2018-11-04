#include "uo_ipc.h"
#include "uo_cb.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <time.h>

static bool is_init;

bool uo_ipc_init(void)
{
    if (is_init)
        return is_init;
    
    is_init = true;

    srand(time(NULL));
    is_init &= uo_sock_init();
    is_init &= uo_cb_init(2);

    return is_init;

}
