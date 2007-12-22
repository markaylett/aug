/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/unistd.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/base.h"
#include "augsys/errinfo.h"
#if defined(_WIN32)
# include "augsys/windows.h" /* Sleep() */
#endif /* _WIN32 */

#include <errno.h>
#include <fcntl.h>           /* O_BINARY */

AUGSYS_API int
aug_close(int fd)
{
    return aub_releasefd(fd);
}

AUGSYS_API int
aug_open(const char* path, int flags, ...)
{
    int fd;
    mode_t mode;

    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
    } else
        mode = 0;

#if defined(_WIN32)
    flags |= O_BINARY;
#endif /* _WIN32 */

    if (-1 == (fd = open(path, flags, mode))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (-1 == aug_openfd(fd, aug_posixfdtype())) {
        close(fd);
        return -1;
    }

    return fd;
}

AUGSYS_API int
aug_pipe(int fds[2])
{
#if !defined(_WIN32)
    if (-1 == pipe(fds)) {
#else /* _WIN32 */
    if (-1 == _pipe(fds, 8192, O_BINARY)) {
#endif /* _WIN32 */
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (-1 == aug_openfds(fds, aug_posixfdtype())) {
        close(fds[0]);
        close(fds[1]);
        return -1;
    }

    return 0;
}

AUGSYS_API ssize_t
aug_read(int fd, void* buf, size_t size)
{
    const struct aug_fdtype* fdtype = aug_getfdtype(fd);
    if (!fdtype)
        return -1;

    if (!fdtype->read_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_read() not supported"));
        return -1;
    }

    return fdtype->read_(fd, buf, size);
}

AUGSYS_API ssize_t
aug_write(int fd, const void* buf, size_t len)
{
    const struct aug_fdtype* fdtype = aug_getfdtype(fd);
    if (!fdtype)
        return -1;

    if (!fdtype->write_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_write() not supported"));
        return -1;
    }

    return fdtype->write_(fd, buf, len);
}

AUGSYS_API void
aug_msleep(unsigned ms)
{
#if !defined(_WIN32)
    usleep(ms * 1000);
#else /* _WIN32 */
    Sleep(ms);
#endif /* _WIN32 */
}
