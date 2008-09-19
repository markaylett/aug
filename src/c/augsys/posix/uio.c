#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>

AUGSYS_API aug_rsize
aug_freadv(aug_fd fd, const struct iovec* iov, int size)
{
    ssize_t ret;
    if (-1 == (ret = readv(fd, iov, size))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API aug_rsize
aug_fwritev(aug_fd fd, const struct iovec* iov, int size)
{
    ssize_t ret;
    if (-1 == (ret = writev(fd, iov, size))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}
