#include "augctx/errinfo.h"

#include <errno.h>

AUGSYS_API ssize_t
aug_freadv(aug_ctx* ctx, aug_fd fd, const struct iovec* iov, int size)
{
    ssize_t ret;
    if (-1 == (ret = readv(fd, iov, size))) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}

AUGSYS_API ssize_t
aug_fwritev(aug_ctx* ctx, aug_fd fd, const struct iovec* iov, int size)
{
    ssize_t ret;
    if (-1 == (ret = writev(fd, iov, size))) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        return -1;
    }
    return ret;
}
