/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsys/posix/utility.c"
#else /* _WIN32 */
# include "augsys/win32/utility.c"
#endif /* _WIN32 */

#include "augsys/base.h"
#include "augsys/errinfo.h"
#include "augsys/log.h"
#include "augsys/string.h" /* aug_strerror() */

AUGSYS_API int
aug_perrinfo(const struct aug_errinfo* errinfo, const char* s)
{
    const char* file;
    if (!errinfo)
        errinfo = aug_geterrinfo();

    if (0 == errinfo->num_) {
        aug_error("%s: no description available", s);
        return 0;
    }

    for (file = errinfo->file_;; ++file)
        switch (*file) {
        case '.':
        case '/':
        case '\\':
            break;
        default:
            goto done;
        }
 done:
    return aug_error("%s: [src=%d, num=0x%.8x (%d)] %s at %s line %d.", s,
                     errinfo->src_, (int)errinfo->num_, (int)errinfo->num_,
                     errinfo->desc_, file, (int)errinfo->line_);
}

AUGSYS_API int
aug_perror(const char* s)
{
    return aug_error("%s: %s", s, aug_strerror(errno));
}

AUGSYS_API int
aug_setnonblock(int fd, int on)
{
    const struct aug_fdtype* fdtype = aug_getfdtype(fd);
    if (!fdtype)
        return -1;

    if (!fdtype->setnonblock_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_setnonblock() not supported"));
        return -1;
    }

    return fdtype->setnonblock_(fd, on);
}
