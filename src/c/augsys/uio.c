/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/uio.h"

static const char rcsid[] = "$Id$";

#include "augsys/base.h"
#include "augsys/errinfo.h"

AUGSYS_API ssize_t
aug_readv(int fd, const struct iovec* iov, int size)
{
    const struct aug_fdtype* fdtype = aug_getfdtype(fd);
    if (!fdtype)
        return -1;

    if (!fdtype->readv_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_readv() not supported"));
        return -1;
    }

    return fdtype->readv_(fd, iov, size);
}

AUGSYS_API ssize_t
aug_writev(int fd, const struct iovec* iov, int size)
{
    const struct aug_fdtype* fdtype = aug_getfdtype(fd);
    if (!fdtype)
        return -1;

    if (!fdtype->writev_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_writev() not supported"));
        return -1;
    }

    return fdtype->writev_(fd, iov, size);
}
