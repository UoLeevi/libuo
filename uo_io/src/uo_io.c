#include "uo_io.h"
#include "uo_cb.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>
#endif

#ifdef _WIN32
static HANDLE thrd;
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

static VOID CALLBACK uo_io_invoke_cb(
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

}

static void uo_io_queue_read(
	ULONG_PTR Parameter)
{
	uo_ioop ioop = *(uo_ioop *)Parameter;
	free((void *)Parameter);

	LPOVERLAPPED lpOverlapped = calloc(1, sizeof *lpOverlapped);
	lpOverlapped->hEvent = ioop.cb;
	ReadFileEx((HANDLE)(uintptr_t)ioop.fd, ioop.buf, ioop.len, lpOverlapped, uo_io_invoke_cb);
}

static void uo_io_queue_write(
	ULONG_PTR Parameter)
{
	uo_ioop ioop = *(uo_ioop *)Parameter;
	free((void *)Parameter);

	LPOVERLAPPED lpOverlapped = calloc(1, sizeof *lpOverlapped);
	lpOverlapped->hEvent = ioop.cb;
	WriteFileEx((HANDLE)(uintptr_t)ioop.fd, ioop.buf, ioop.len, lpOverlapped, uo_io_invoke_cb);
}
#endif

static void uo_io_quit(void)
{
	is_quitting = true;
#ifdef _WIN32
	QueueUserAPC(uo_io_noop, thrd, (ULONG_PTR)0);
	WaitForSingleObject(thrd, INFINITE);
#endif
}

bool uo_io_init()
{
	if (is_init)
		return true;

	is_init = true;

#ifdef _WIN32
	thrd = CreateThread(NULL, 0, uo_io_accept_async_io, NULL, 0, NULL);
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
#endif
}
