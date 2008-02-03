/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/flags_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/types.h"

#include "augsys/errinfo.h"
#include "augsys/log.h"

#include <assert.h>

#define FLAGMASK_ (AUG_RDONLY | AUG_WRONLY | AUG_RDWR \
    | AUG_APPEND | AUG_CREAT | AUG_TRUNC | AUG_EXCL)

AUG_EXTERNC int
aug_toflags_(int* to, int from)
{
#if !defined(_WIN32)
    int flags = 0;
#else /* _WIN32 */
    int flags = O_BINARY;
#endif /* _WIN32 */

    assert(to);
    if (from & ~FLAGMASK_)
        goto fail;

    switch (from & (AUG_RDONLY | AUG_WRONLY | AUG_RDWR)) {
    case AUG_RDONLY:
        flags |= O_RDONLY;
        break;
    case AUG_WRONLY:
    case AUG_RDWR:
        flags |= O_RDWR;
        break;
    default:
        goto fail;
    }
    if (from & AUG_APPEND) {
        assert(AUG_APPEND == (from & AUG_APPEND));
        flags |= O_APPEND;
    }
    if (from & AUG_CREAT) {
        assert(AUG_CREAT == (from & AUG_CREAT));
        flags |= O_CREAT;
    }
    if (from & AUG_TRUNC) {
        assert(AUG_TRUNC == (from & AUG_TRUNC));
        flags |= O_TRUNC;
    }
    if (from & AUG_EXCL) {
        assert(AUG_EXCL == (from & AUG_EXCL));
        flags |= O_EXCL;
    }
    *to = flags;
    return 0;

 fail:
    aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                   AUG_MSG("invalid open flags"));
    return -1;
}
