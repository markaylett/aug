/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errno.h"

#include <io.h>
#include <winsock2.h>

AUGSYS_API int
aug_filesize(int fd, size_t* size)
{
    DWORD low, high;
    intptr_t file;

    if (-1 == (file = _get_osfhandle(fd))) {
        errno = EINVAL;
        return -1;
    }

    low = GetFileSize((HANDLE)file, &high);

    if (-1 == low && NO_ERROR != GetLastError()) {
        aug_maperror(GetLastError());
        return -1;
    }
    if (high) {
        errno = EINVAL;
        return -1;
    }
    *size = low;
    return 0;
}

AUGSYS_API int
aug_setnonblock(int fd, int on)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    unsigned long arg = (unsigned long)on;

    if (SOCKET_ERROR == ioctlsocket((SOCKET)h, FIONBIO, &arg)) {
        aug_maperror(WSAGetLastError());
        return -1;
    }

    return 0;
}
