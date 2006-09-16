/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#undef __STRICT_ANSI__ /* random(), srandom() */
#define AUGSYS_BUILD
#include "augsys/utility.h"

static const char rcsid[] = "$Id$";

#if !defined(_WIN32)
# include "augsys/posix/utility.c"
#else /* _WIN32 */
# include "augsys/win32/utility.c"
#endif /* _WIN32 */

#include "augsys/base.h"

AUGSYS_API int
aug_setnonblock(int fd, int on)
{
    const struct aug_driver* driver = aug_getdriver(fd);
    if (!driver)
        return -1;

    if (!driver->setnonblock_) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_setnonblock() not supported"));
        return -1;
    }

    return driver->setnonblock_(fd, on);
}
