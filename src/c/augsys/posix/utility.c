/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <fcntl.h>
#include <sys/stat.h>

AUGSYS_API int
aug_filesize(int fd, size_t* size)
{
    struct stat s;
    if (-1 == fstat(fd, &s))
        return -1;
    *size = s.st_size;
    return 0;
}

AUGSYS_API int
aug_setnonblock(int fd, int on)
{
    int flags = fcntl(fd, F_GETFL);
    if (-1 == flags)
        return -1;

    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    return fcntl(fd, F_SETFL, flags);
}
