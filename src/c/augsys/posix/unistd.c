/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"

AUGSYS_API int
aug_close(int fd)
{
    return aug_releasefd(fd);
}


AUGSYS_API int
aug_pipe(int fds[2])
{
    if (-1 == pipe(fds))
        return -1;

    if (-1 == aug_openfds(fds, AUG_FDPIPE)) {
        close(fds[0]);
        close(fds[1]);
        return -1;
    }

    return 0;
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
