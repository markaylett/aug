/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"

AUGSYS_API int
aug_close(int fd)
{
    return aug_releasefd(fd);
}

AUGSYS_API ssize_t
aug_read(int fd, void* buf, size_t size)
{
    return read(fd, buf, size);
}

AUGSYS_API ssize_t
aug_write(int fd, const void* buf, size_t size)
{
    return write(fd, buf, size);
}
