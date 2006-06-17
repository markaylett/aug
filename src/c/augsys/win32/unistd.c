/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"
#include "augsys/errno.h"

#include <winsock2.h>

AUGSYS_API int
aug_close(int fd)
{
    return aug_releasefd(fd);
}

AUGSYS_API ssize_t
aug_read(int fd, void* buf, size_t size)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    int type;
    ssize_t num;

    if (-1 == (type = aug_fdtype(fd)))
        return -1;

    if (AUG_FDSOCK == type) {

        if (SOCKET_ERROR == (num = recv((SOCKET)h, buf, (int)size, 0)))  {
            aug_maperror(WSAGetLastError());
            return -1;
        }
    } else {

        if (!ReadFile(h, buf, (DWORD)size, &num, NULL)) {
            aug_maperror(GetLastError());
            return -1;
        }
    }
    return num;
}

AUGSYS_API ssize_t
aug_write(int fd, const void* buf, size_t size)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    int type;
    ssize_t num;

    if (-1 == (type = aug_fdtype(fd)))
        return -1;

    if (AUG_FDSOCK == type) {

        if (SOCKET_ERROR == (num = send((SOCKET)h, buf, (int)size, 0)))  {
            aug_maperror(WSAGetLastError());
            return -1;
        }
    } else {

        if (!WriteFile(h, buf, (DWORD)size, &num, NULL)) {
            aug_maperror(GetLastError());
            return -1;
        }
    }
    return num;
}
