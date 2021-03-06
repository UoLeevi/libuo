#include "uo_io.h"
#include "uo_cb.h"
#include "uo_err.h"

#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32

    #include <windows.h>

    static HANDLE thrd;

#else

    #include <errno.h>
    #include <unistd.h>
    #include <sys/epoll.h>
    #include <pthread.h>

    #define UO_IO_EPOLL_MAXEVENTS 0x100

    static pthread_t thrd;
    static int epfd;

#endif

_Thread_local uo_io_err uo_io_errno;

static bool is_init;
static bool is_quitting;

typedef struct uo_ioop
{
    int fd;
    void *buf;
    size_t len;
    uo_cb *cb;
} uo_ioop;

#ifdef _WIN32

    static DWORD WINAPI uo_io_accept_async_io(
        LPVOID lpParam)
    {
        uo_cb_thrd_init();

        while (!is_quitting)
            SleepEx(INFINITE, TRUE);

        uo_cb_thrd_quit();
    }

    static VOID CALLBACK uo_io_cb_invoke(
        DWORD dwErrorCode,
        DWORD dwNumberOfBytesTransfered,
        LPOVERLAPPED lpOverlapped)
    {
        uo_cb *cb = lpOverlapped->hEvent;
        free(lpOverlapped);

        if (!dwNumberOfBytesTransfered)
        {
            // TODO: specify what error occurred
            uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
        }
        
        uo_cb_stack_push(cb, (void *)(uintptr_t)dwNumberOfBytesTransfered);
        uo_cb_invoke_async(cb);
    }

    static void uo_io_noop(
        ULONG_PTR Parameter)
    {
        // noop
    }

    static void uo_io_queue_read(
        ULONG_PTR Parameter)
    {
        uo_ioop ioop = *(uo_ioop *)Parameter;
        free((void *)Parameter);

        LPOVERLAPPED lpOverlapped = calloc(1, sizeof *lpOverlapped);
        lpOverlapped->hEvent = ioop.cb;
        if (!ReadFileEx((HANDLE)(uintptr_t)ioop.fd, ioop.buf, ioop.len, lpOverlapped, uo_io_cb_invoke))
        {
            free(lpOverlapped);

            uo_cb_stack_push(ioop.cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
            uo_cb_stack_push(ioop.cb, 0);
            uo_cb_invoke_async(ioop.cb);
        }
    }

    static void uo_io_queue_write(
        ULONG_PTR Parameter)
    {
        uo_ioop ioop = *(uo_ioop *)Parameter;
        free((void *)Parameter);

        LPOVERLAPPED lpOverlapped = calloc(1, sizeof *lpOverlapped);
        lpOverlapped->hEvent = ioop.cb;
        if (!WriteFileEx((HANDLE)(uintptr_t)ioop.fd, ioop.buf, ioop.len, lpOverlapped, uo_io_cb_invoke))
        {
            free(lpOverlapped);

            uo_cb_stack_push(ioop.cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
            uo_cb_stack_push(ioop.cb, 0);
            uo_cb_invoke_async(ioop.cb);
        }
    }

#else

    static void uo_io_execute_io(
        uo_cb *cb)
    {
        struct epoll_event *epevt = uo_cb_stack_pop(cb);
        uo_ioop *ioop = epevt->data.ptr;
        uint32_t events = epevt->events;
        free(epevt);

        int fd = ioop->fd;
        void *buf = ioop->buf;
        size_t len = ioop->len;
        free(ioop);

        if (events & EPOLLIN)
        {
            ssize_t rlen = read(fd, buf, len);
            switch (rlen)
            {
                case -1:
                    // TODO: specify what error occurred
                    uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
                    uo_cb_stack_push(cb, 0);
                    break;

                case 0:
                    uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_NONE);
                    /* fall through */
                default:
                    uo_cb_stack_push(cb, (void *)(uintptr_t)rlen);
                    break;
            }
        }
        else if (events & EPOLLOUT)
        {
            ssize_t wlen = write(fd, buf, len);
            switch (wlen)
            {
                case -1:
                    // TODO: specify what error occurred
                    uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
                    uo_cb_stack_push(cb, 0);
                    break;

                case 0:
                    uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_NONE);
                    /* fall through */
                default:
                    uo_cb_stack_push(cb, (void *)(uintptr_t)wlen);
                    break;
            }
        }
        else if (events & EPOLLHUP)
        {
            uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_NONE);
            uo_cb_stack_push(cb, 0);
        }
        else
        {
            // TODO: specify what error occurred
            uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
            uo_cb_stack_push(cb, 0);
        }

        uo_cb_invoke(cb);
    }

    static void *uo_io_accept_async_io(
        void *arg)
    {
        uo_cb_thrd_init();

        struct epoll_event epevts[UO_IO_EPOLL_MAXEVENTS];

        while (true)
        {
            int nfds = epoll_wait(epfd, epevts, UO_IO_EPOLL_MAXEVENTS, -1);

            if (is_quitting)
                break;

            if (nfds == -1 && errno != EINTR)
                uo_err_exit("Error occurred while performing epoll_wait.");

            for (int i = 0; i < nfds; ++i)
            {
                uo_ioop *ioop = epevts[i].data.ptr;
                uo_cb_prepend(ioop->cb, uo_io_execute_io);
                uo_cb_invoke_async(ioop->cb);
            }
        }

        uo_cb_thrd_quit();
    }

#endif

static void uo_io_quit(void)
{
    is_quitting = true;

    #ifdef _WIN32

        QueueUserAPC(uo_io_noop, thrd, (ULONG_PTR)0);
        WaitForSingleObject(thrd, INFINITE);

    #else

        struct epoll_event epevt = {
            .events = EPOLLOUT | EPOLLONESHOT
        };

        if ((epoll_ctl(epfd, EPOLL_CTL_ADD, 1, &epevt) == 0)
            || (errno == EEXIST && epoll_ctl(epfd, EPOLL_CTL_MOD, 1, &epevt) == 0))
            pthread_join(thrd, NULL);
        else
            pthread_cancel(thrd);

        close(epfd);

    #endif
}

bool uo_io_init()
{
    if (is_init)
        return true;

    is_init = true;

    is_init &= uo_cb_init();

    uo_io_errno = UO_IO_ERR_NONE;

    #ifdef _WIN32

        thrd = CreateThread(NULL, 0, uo_io_accept_async_io, NULL, 0, NULL);
        is_init &= thrd != NULL;

    #else

        epfd = epoll_create1(0);
        is_init &= epfd != -1;

        is_init &= pthread_create(&thrd, NULL, uo_io_accept_async_io, NULL) == 0;

    #endif

    atexit(uo_io_quit);

    return is_init;
}

size_t uo_io_write(
    int wfd,
    const void *buf,
    size_t len)
{
    #ifdef _WIN32

        DWORD wlen;
        OVERLAPPED overlapped = { 0 };

        if (!WriteFile((HANDLE)(uintptr_t)wfd, buf, len, &wlen, &overlapped))
        {
            // TODO: specify what error occurred
            uo_io_errno = UO_IO_ERR_UNKNOWN;
            return 0;
        }

    #else

        ssize_t wlen = write(wfd, buf, len);
        
        if (wlen == -1)
        {
            // TODO: specify what error occurred
            uo_io_errno = UO_IO_ERR_UNKNOWN;
            return 0;
        }

    #endif

        uo_io_errno = UO_IO_ERR_NONE;

        return wlen;
}

void uo_io_read_async(
    int rfd,
    void *buf,
    size_t len,
    uo_cb *cb)
{
    if (!buf || !len)
    {
        uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_NONE);
        uo_cb_stack_push(cb, 0);
        uo_cb_invoke_async(cb);
        return;
    }

    uo_ioop *ioop = malloc(sizeof *ioop);

    ioop->fd = rfd;
    ioop->buf = buf;
    ioop->len = len;
    ioop->cb = cb;

    #ifdef _WIN32

        bool success = QueueUserAPC(uo_io_queue_read, thrd, (ULONG_PTR)ioop);

    #else

        struct epoll_event *epevt = malloc(sizeof *epevt);
        epevt->events = EPOLLIN | EPOLLONESHOT;
        epevt->data.ptr = ioop;
        uo_cb_stack_push(cb, epevt);

        bool success = (epoll_ctl(epfd, EPOLL_CTL_ADD, rfd, epevt) == 0)
            || (errno == EEXIST && epoll_ctl(epfd, EPOLL_CTL_MOD, rfd, epevt) == 0);

    #endif

    if (!success)
    {
        uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
        uo_cb_stack_push(cb, 0);
        uo_cb_invoke_async(cb);
    }
}

void uo_io_write_async(
    int wfd,
    void *buf,
    size_t len,
    uo_cb *cb)
{
    if (!buf || !len)
    {
        uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_NONE);
        uo_cb_stack_push(cb, 0);
        uo_cb_invoke_async(cb);
        return;
    }

    uo_ioop *ioop = malloc(sizeof *ioop);

    ioop->fd = wfd;
    ioop->buf = buf;
    ioop->len = len;
    ioop->cb = cb;

    #ifdef _WIN32

        bool success = QueueUserAPC(uo_io_queue_write, thrd, (ULONG_PTR)ioop);

    #else

        struct epoll_event *epevt = malloc(sizeof *epevt);
        epevt->events = EPOLLOUT | EPOLLONESHOT;
        epevt->data.ptr = ioop;
        uo_cb_stack_push(cb, epevt);

        bool success = (epoll_ctl(epfd, EPOLL_CTL_ADD, wfd, epevt) == 0)
            || (errno == EEXIST && epoll_ctl(epfd, EPOLL_CTL_MOD, wfd, epevt) == 0);

    #endif

    if (!success)
    {
        uo_cb_stack_push(cb, (void *)(uintptr_t)UO_IO_ERR_UNKNOWN);
        uo_cb_stack_push(cb, 0);
        uo_cb_invoke_async(cb);
    }
}
