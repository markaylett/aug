/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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
    const struct aug_driver* driver = aug_getdriver(fd);
    if (!driver)
        return -1;

    if (!driver->readv_) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_readv() not supported"));
        return -1;
    }

    return driver->readv_(fd, iov, size);
}

AUGSYS_API ssize_t
aug_writev(int fd, const struct iovec* iov, int size)
{
    const struct aug_driver* driver = aug_getdriver(fd);
    if (!driver)
        return -1;

    if (!driver->writev_) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_writev() not supported"));
        return -1;
    }

    return driver->writev_(fd, iov, size);
}
