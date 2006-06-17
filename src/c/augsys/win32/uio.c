/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errno.h"

#include <io.h>
#include <winsock2.h>

#include <malloc.h> /* _alloca() */

AUGSYS_API ssize_t
aug_readv(int fd, const struct iovec* iov, int size)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    WSABUF* buf;
    ssize_t i, num;

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
        buf = _alloca(sizeof(WSABUF) * size);
#if defined(_MSC_VER)
	}
	__except (STATUS_STACK_OVERFLOW == GetExceptionCode()) {
		aug_maperror(ERROR_NOT_ENOUGH_MEMORY);
		return -1;
	}
#endif /* _MSC_VER */

    for (i = 0; i < size; ++i) {
        buf[i].len = iov[i].iov_len;
        buf[i].buf = iov[i].iov_base;
    }

    if (SOCKET_ERROR == WSARecv((SOCKET)h, buf, size, &num, 0, NULL, NULL)) {
        aug_maperror(WSAGetLastError());
        return -1;
    }

    return num;
}

AUGSYS_API ssize_t
aug_writev(int fd, const struct iovec* iov, int size)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    WSABUF* buf;
    ssize_t i, num;

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
        buf = _alloca(sizeof(WSABUF) * size);
#if defined(_MSC_VER)
	}
	__except (STATUS_STACK_OVERFLOW == GetExceptionCode()) {
		aug_maperror(ERROR_NOT_ENOUGH_MEMORY);
		return -1;
	}
#endif /* _MSC_VER */

    for (i = 0; i < size; ++i) {
        buf[i].len = iov[i].iov_len;
        buf[i].buf = iov[i].iov_base;
    }

    if (SOCKET_ERROR == WSASend((SOCKET)h, buf, size, &num, 0, NULL, NULL)) {
        aug_maperror(WSAGetLastError());
        return -1;
    }

    return num;
}
