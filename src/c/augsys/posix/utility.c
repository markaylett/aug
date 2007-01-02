/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"

#include <errno.h>
#include <stdlib.h> /* random(), srandom() */
#include <sys/stat.h>

#if defined(_MT)
# include <pthread.h>
#endif /* _MT */

AUGSYS_API int
aug_filesize(int fd, size_t* size)
{
    struct stat s;
    if (-1 == fstat(fd, &s)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
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
aug_srand(unsigned seed)
{
    srandom(seed);
}

AUGSYS_API unsigned
aug_threadid(void)
{
#if !defined(_MT)
    return 0;
#else /* _MT */
    return (unsigned)pthread_self();
#endif /* _MT */
}
