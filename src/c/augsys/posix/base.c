/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/lock.h"
#include "augsys/log.h"

#include <fcntl.h>
#include <unistd.h>

#include <sys/uio.h>

static int
close_(int fd)
{
    if (-1 == close(fd))
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return 0;
}

static ssize_t
read_(int fd, void* buf, size_t size)
{
    ssize_t ret = read(fd, buf, size);
    if (-1 == ret)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return ret;
}

static ssize_t
readv_(int fd, const struct iovec* iov, int size)
{
    ssize_t ret = readv(fd, iov, size);
    if (-1 == ret)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return ret;
}

static ssize_t
write_(int fd, const void* buf, size_t size)
{
    ssize_t ret = write(fd, buf, size);
    if (-1 == ret)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return ret;
}

static ssize_t
writev_(int fd, const struct iovec* iov, int size)
{
    ssize_t ret = writev(fd, iov, size);
    if (-1 == ret)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return ret;
}

static int
setnonblock_(int fd, int on)
{
    int flags = fcntl(fd, F_GETFL);
    if (-1 == flags) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (-1 == fcntl(fd, F_SETFL, flags)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static struct aug_driver driver_ = {
    close_,
    read_,
    readv_,
    write_,
    writev_,
    setnonblock_
};

AUGSYS_API int
aug_init(struct aug_errinfo* errinfo)
{
    int ret = retain_();
    if (PROCEED_ != ret)
        return ret;

    if (-1 == aug_initlock_())
        return -1;

    if (-1 == aug_initerrinfo_(errinfo)) {
        aug_termlock_();
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_term(void)
{
    int ret = release_();
    if (PROCEED_ != ret)
        return ret;

    aug_setlogger(NULL);

    if (-1 == aug_termerrinfo_()) {
        aug_termlock_();
        return -1;
    }

    return aug_termlock_();
}

AUGSYS_API const struct aug_driver*
aug_posixdriver(void)
{
    return &driver_;
}
