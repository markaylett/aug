#include "augctx/errinfo.h"
#include "augtypes.h"

AUGSYS_API ssize_t
aug_freadv(aug_ctx* ctx, aug_fd fd, const struct iovec* iov, int size)
{
    aug_seterrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, "aug",
                   AUG_ESUPPORT, AUG_MSG("aug_freadv() not supported"));
    return -1;
}

AUGSYS_API ssize_t
aug_fwritev(aug_ctx* ctx, aug_fd fd, const struct iovec* iov, int size)
{
    aug_seterrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, "aug",
                   AUG_ESUPPORT, AUG_MSG("aug_fwritev() not supported"));
    return -1;
}
