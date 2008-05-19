/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsys/posix/base.c"
#else /* _WIN32 */
# include "augsys/win32/base.c"
#endif /* _WIN32 */

#include "augctx/lock.h"

#include <limits.h>

AUGSYS_API unsigned
aug_nextid(void)
{
    static unsigned id_ = 1;
    unsigned id;

    aug_lock();
    if (id_ == INT_MAX) {
        id_ = 1;
        id = INT_MAX;
    } else
        id = id_++;
    aug_unlock();

    return id;
}

AUGSYS_API const struct aug_fdtype*
aug_setfdtype(int fd, const struct aug_fdtype* fdtype)
{
    return NULL;
}

AUGSYS_API const struct aug_fdtype*
aug_getfdtype(int fd)
{
    return NULL;
}

AUGSYS_API struct aug_fdtype*
aug_extfdtype(struct aug_fdtype* derived, const struct aug_fdtype* base)
{
    return NULL;
}
