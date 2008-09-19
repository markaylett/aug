#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augtypes.h"

AUGSYS_API aug_rsize
aug_freadv(aug_fd fd, const struct iovec* iov, int size)
{
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                   AUG_ESUPPORT, AUG_MSG("aug_freadv() not supported"));
    return AUG_FAILERROR;
}

AUGSYS_API aug_rsize
aug_fwritev(aug_fd fd, const struct iovec* iov, int size)
{
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                   AUG_ESUPPORT, AUG_MSG("aug_fwritev() not supported"));
    return AUG_FAILERROR;
}
