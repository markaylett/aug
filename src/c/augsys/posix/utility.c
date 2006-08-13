/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"

#include <errno.h>
#include <sys/stat.h>

AUGSYS_API int
aug_filesize(int fd, size_t* size)
{
    struct stat s;
    if (-1 == fstat(fd, &s)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    *size = s.st_size;
    return 0;
}

AUGSYS_API long
aug_rand(void)
{
    return (long)random();
}

AUGSYS_API void
aug_srand(unsigned int seed)
{
    srandom(seed);
}
