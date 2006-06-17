/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/file_.h"

#include "augmar/flags_.h"
#include "augmar/format_.h"

#include "augsys/log.h"
#include "augsys/string.h" /* aug_perror() */
#include "augsys/unistd.h" /* fsync() */

#include <assert.h>
#include <errno.h>
#include <stdio.h>         /* SEEK_CUR */

AUGMAR_EXTERN int
aug_closefile_(int fd)
{
    return close(fd);
}

AUGMAR_EXTERN int
aug_openfile_(const char* path, int flags, mode_t mode)
{
    int local;
    if (-1 == aug_toflags_(&local, flags))
        return -1;

    return open(path, local, mode);
}

AUGMAR_EXTERN int
aug_extendfile_(int fd, size_t size)
{
    /* The lseek function can be used to move the file pointer beyond the end
       of the file, in which case, any subsequent write operation should fill
       the gap between the current end of file and the file pointer with zeros.

       Bug: On Windows 95/98 the gap is filled with garbage instead of
       zeros. */

    static const aug_byte_t ZERO = 0;

    off_t cur;
    int err;

    if (!size)
        return 0;

    /* Store current file pointer for later restoration. */

    if (-1 == (cur = lseek(fd, 0, SEEK_CUR))) {

        aug_perror("lseek() failed");
        return -1;
    }

    if (-1 == lseek(fd, (off_t)(size - 1), SEEK_END)) {

        aug_perror("lseek() failed");
        return -1;
    }

    if (1 != write(fd, &ZERO, 1)) {

        aug_perror("write() failed");
        goto fail;
    }

    /* Ensure that gap is filled with zeros. */

    if (-1 == fsync(fd)) {

        aug_perror("fsync() failed");
        goto fail;
    }

    /* Restore original file pointer. */

    if (-1 == lseek(fd, cur, SEEK_SET)) {

        aug_perror("lseek() failed");
        return -1;
    }

    return 0;

 fail:
    err = errno;
    lseek(fd, cur, SEEK_SET);
    errno = err;
    return -1;
}

AUGMAR_EXTERN int
aug_syncfile_(int fd)
{
    if (-1 == fsync(fd)) {

        aug_perror("fsync() failed");
        return -1;
    }
    return 0;
}

AUGMAR_EXTERN int
aug_truncatefile_(int fd, off_t size)
{
    if (-1 == ftruncate(fd, size)) {

        aug_perror("ftruncate() failed");
        return -1;
    }
    return 0;
}
