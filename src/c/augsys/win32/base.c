/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <io.h>
#include <winsock2.h>

static int
close_(int fd)
{
    if (-1 == close(fd)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static ssize_t
read_(int fd, void* buf, size_t size)
{
    DWORD ret;

    if (!ReadFile((HANDLE)_get_osfhandle(fd), buf, (DWORD)size, &ret, NULL)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    DWORD ret;

    if (!WriteFile((HANDLE)_get_osfhandle(fd), buf, (DWORD)len, &ret,
                   NULL)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static struct aug_fdtype fdtype_ = {
    close_,
    read_,
    NULL,
    write_,
    NULL,
    NULL
};

AUGSYS_API const struct aug_fdtype*
aug_posixfdtype(void)
{
    return &fdtype_;
}
