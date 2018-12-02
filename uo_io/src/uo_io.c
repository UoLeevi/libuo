#include "uo_io.h"
#include "uo_cb.h"
#include "uo_err.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>

#define UO_IO_EPOLL_MAXEVENTS 0x100
#endif

#ifdef _WIN32
static HANDLE thrd;
#else
static pthread_t thrd;
static int epfd;
#endif

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
	while (!is_quitting)
		SleepEx(INFINITE, TRUE);
}

static VOID CALLBACK uo_io_cb_invoke(
	DWORD dwErrorCode,
	DWORD dwNumberOfBytesTransfered,
	LPOVERLAPPED lpOverlapped)
{
	uo_cb *cb = lpOverlapped->hEvent;
	free(lpOverlapped);
	uo_cb_invoke_async(cb, (void *)(uintptr_t)dwNumberOfBytesTransfered, NULL);
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
	ReadFileEx((HANDLE)(uintptr_t)ioop.fd, ioop.buf, ioop.len, lpOverlapped, uo_io_cb_invoke);
}

static void uo_io_queue_write(
	ULONG_PTR Parameter)
{
	uo_ioop ioop = *(uo_ioop *)Parameter;
	free((void *)Parameter);

	LPOVERLAPPED lpOverlapped = calloc(1, sizeof *lpOverlapped);
	lpOverlapped->hEvent = ioop.cb;
	WriteFileEx((HANDLE)(uintptr_t)ioop.fd, ioop.buf, ioop.len, lpOverlapped, uo_io_cb_invoke);
}
#else
static void *uo_io_execute_io(
	void *arg,
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

	if (!buf || !len)
		return (void *)(uintptr_t)-1;
	else if (events & EPOLLIN)
		return (void *)(uintptr_t)read(fd, buf, len);
	else if (events & EPOLLOUT)
		return (void *)(uintptr_t)write(fd, buf, len);
	else
		return (void *)(uintptr_t)-1;
}

static void *uo_io_accept_async_io(
	void *arg)
{
	struct epoll_event epevts[UO_IO_EPOLL_MAXEVENTS];

	while (!is_quitting)
	{
		int nfds = epoll_wait(epfd, epevts, UO_IO_EPOLL_MAXEVENTS, -1);
		if (nfds == -1)
			uo_err_exit("Error occurred while performing epoll_wait.");

		for (int i = 0; i < nfds; ++i) 
		{
			uo_ioop *ioop = epevts[i].data.ptr;
			uo_cb_prepend(ioop->cb, (void *(*)(void *, uo_cb *))uo_io_execute_io);
			uo_cb_invoke_async(ioop->cb, NULL, NULL);
		}
	}
}
#endif

static void uo_io_quit(void)
{
	is_quitting = true;
#ifdef _WIN32
	QueueUserAPC(uo_io_noop, thrd, (ULONG_PTR)0);
	WaitForSingleObject(thrd, INFINITE);
#else
	uo_cb *noop_cb = uo_cb_create(UO_CB_OPT_DESTROY);
	uo_io_write_async(1, NULL, 0, noop_cb);
	pthread_join(thrd, NULL);
	close(epfd);
#endif
}

bool uo_io_init()
{
	if (is_init)
		return true;

	is_init = true;

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

ssize_t uo_io_write(
	int wfd,
	const void *buf, 
	size_t len)
{
#ifdef _WIN32
	DWORD wlen;
	OVERLAPPED overlapped = { 0 };
	if (!WriteFile((HANDLE)(uintptr_t)wfd, buf, len, &wlen, &overlapped))
		return -1;
	return wlen;
#else
	return write(wfd, buf, len);
#endif
}

bool uo_io_read_async(
	int rfd,
	void *buf,
	size_t len,
	uo_cb *cb)
{
	uo_ioop *ioop = malloc(sizeof *ioop);

	ioop->fd = rfd;
	ioop->buf = buf;
	ioop->len = len;
	ioop->cb = cb;

#ifdef _WIN32
	return QueueUserAPC(uo_io_queue_read, thrd, (ULONG_PTR)ioop);
#else
	struct epoll_event *epevt = malloc(sizeof *epevt);
	epevt->events = EPOLLIN | EPOLLONESHOT;
	epevt->data.ptr = ioop;
	uo_cb_stack_push(cb, epevt);
	return epoll_ctl(epfd, EPOLL_CTL_ADD, rfd, epevt) == 0;
#endif
}

bool uo_io_write_async(
	int wfd,
	void *buf,
	size_t len,
	uo_cb *cb)
{
	uo_ioop *ioop = malloc(sizeof *ioop);

	ioop->fd = wfd;
	ioop->buf = buf;
	ioop->len = len;
	ioop->cb = cb;

#ifdef _WIN32
	return QueueUserAPC(uo_io_queue_write, thrd, (ULONG_PTR)ioop);
#else
	struct epoll_event *epevt = malloc(sizeof *epevt);
	epevt->events = EPOLLOUT | EPOLLONESHOT;
	epevt->data.ptr = ioop;
	uo_cb_stack_push(cb, epevt);
	return epoll_ctl(epfd, EPOLL_CTL_ADD, wfd, epevt) == 0;
#endif
}
