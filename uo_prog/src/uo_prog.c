#include "uo_prog.h"

#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdatomic.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

static bool is_init;

static sem_t sem;
static atomic_size_t count_waiting;

static void uo_prog_sigint_handler(
    int signal)
{
    size_t count;
    while (count = atomic_load(&count_waiting))
    {
        sem_post(&sem);
        atomic_fetch_sub(&count_waiting, 1);
    }
}

#ifdef _WIN32
    BOOL WINAPI uo_prog_ConsoleCtrlHandler(
        DWORD dwType)
    {
        if (dwType == CTRL_C_EVENT)
        {
            uo_prog_sigint_handler(SIGINT);
            return TRUE;
        }

        return FALSE;
    }
#endif

void uo_prog_wait_for_sigint()
{
    atomic_fetch_add(&count_waiting, 1);
    sem_wait(&sem);
}

static void uo_prog_quit(void)
{
    sem_destroy(&sem);
}

bool uo_prog_init()
{
    if (is_init)
        return true;

    is_init = true;

    is_init &= sem_init(&sem, 0, 0) == 0;

    atomic_init(&count_waiting, 0);

#ifdef _WIN32
    is_init &= SetConsoleCtrlHandler((PHANDLER_ROUTINE)uo_prog_ConsoleCtrlHandler, TRUE);
#else
    is_init &= signal(SIGINT, uo_prog_sigint_handler) != SIG_ERR;
#endif

    atexit(uo_prog_quit);

    return is_init;
}
