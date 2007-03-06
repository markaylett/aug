/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/unistd.h"

static const char rcsid[] = "$Id$";

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
    return aug_releasefd(fd);
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
aug_write(int fd, const void* buf, size_t size)
{
    const struct aug_fdtype* fdtype = aug_getfdtype(fd);
    if (!fdtype)
        return -1;

    if (!fdtype->write_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_write() not supported"));
        return -1;
    }

    return fdtype->write_(fd, buf, size);
}

AUGSYS_API void
aug_sleep(unsigned ms)
{
#if !defined(_WIN32)
    usleep(ms * 1000);
#else /* _WIN32 */
    Sleep(ms);
#endif /* _WIN32 */
}
