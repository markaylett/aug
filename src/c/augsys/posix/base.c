/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>

static int
close_(int fd)
{
    if (-1 == close(fd))
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    return 0;
}

static ssize_t
read_(int fd, void* buf, size_t size)
{
    ssize_t ret = read(fd, buf, size);
    if (-1 == ret)
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    return ret;
}

static ssize_t
readv_(int fd, const struct iovec* iov, int size)
{
    ssize_t ret = readv(fd, iov, size);
    if (-1 == ret)
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    return ret;
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    ssize_t ret = write(fd, buf, len);
    if (-1 == ret)
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    return ret;
}

static ssize_t
writev_(int fd, const struct iovec* iov, int size)
{
    ssize_t ret = writev(fd, iov, size);
    if (-1 == ret)
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    return ret;
}

static int
setnonblock_(int fd, int on)
{
    int flags = fcntl(fd, F_GETFL);
    if (-1 == flags) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }

    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (-1 == fcntl(fd, F_SETFL, flags)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static struct aug_fdtype fdtype_ = {
    close_,
    read_,
    readv_,
    write_,
    writev_,
    setnonblock_
};

AUGSYS_API const struct aug_fdtype*
aug_posixfdtype(void)
{
    return &fdtype_;
}
